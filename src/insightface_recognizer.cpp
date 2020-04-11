#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "base_logger.h"
#include "face_recognizer.h"
#include "user_mgt.h"
#include "common.h"
#include "conf_mgt.h"

#include "timing.h"

using namespace cv;
using namespace std;
using namespace tensorflow;
using tensorflow::Tensor;
using tensorflow::Status;

const int height = 112;
const int width = 112;
const int depth = 3;

extern int g_mode;
extern User_Mgt *g_user_mgt;
extern Logger g_logger;
extern StModelInfo g_modelInfo;

//const string input_layer = "data:0";
//const string output_layer = "output:0";
const string input_layer_1 = "input_images:0";
const string input_layer_2 = "train_phase_bn:0";
const string input_layer_3 = "train_phase_dropout:0";
const string output_layer = "embeddings:0";

int InsightFace_Recognizer::init_session(const string &graph_path)
{
    tensorflow::GraphDef graph_def;
	tensorflow::SessionOptions opts;

	TF_CHECK_OK(ReadBinaryProto(tensorflow::Env::Default(), graph_path, &graph_def));

	//graph::SetDefaultDevice("/gpu:0", &graph_def);
	//opts.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction(0.5);
	opts.config.mutable_gpu_options()->set_allow_growth(true);

	m_session.reset(tensorflow::NewSession(opts));
	TF_CHECK_OK(m_session->Create(graph_def));

	#if 1
	// Walk around to load libcublas.so.9.0 before client request

    tensorflow::Tensor face_tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1, height, width, depth}));
    std::vector<Tensor> outputs;

	Tensor train_phase(tensorflow::DT_BOOL, tensorflow::TensorShape());
    train_phase.scalar<bool>()() = false;
	Tensor train_phase_last(tensorflow::DT_BOOL, tensorflow::TensorShape());
    train_phase_last.scalar<bool>()() = false;

    TF_CHECK_OK(m_session->Run({{input_layer_1, face_tensor},
    							{input_layer_2, train_phase},
    							{input_layer_3, train_phase_last}},
    							{output_layer},
    							{},
    							&outputs));
	#endif

    return 0;
}

InsightFace_Recognizer::InsightFace_Recognizer(const std::string &model_dir, const std::string &data_dir)
{
    init_session(model_dir + g_modelInfo._recognizer);
    m_data_dir = data_dir;
    if (g_user_mgt)
    {
    	m_users = g_user_mgt->get_users();
    }
    m_aligner = std::make_unique<Opencv_Aligner>(m_data_dir);
}


InsightFace_Recognizer::~InsightFace_Recognizer()
{
}

void InsightFace_Recognizer::get_img_tensor(cv::Mat& img, tensorflow::Tensor &input_tensor, bool mix_feature)
{
	if (img.empty())
	{
		return;
	}

	if (img.rows != height || img.cols != width)
    	cv::resize(img, img, Size(width, height));

    auto input_tensor_mapped = input_tensor.tensor<float, 4>();

	//cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	img.convertTo(img, CV_64FC3);
    img = img - cv::Mat(width, height, CV_64FC3, cv::Scalar::all(127.5));
    img = img * 0.0078125;

	std::vector<cv::Mat> imgs;
	imgs.push_back(img);

	if (mix_feature)
	{
		cv::Mat flip_img;
    	cv::flip(img, flip_img, 1);
		imgs.push_back(flip_img);
	}

	for (int i = 0; i < imgs.size(); ++i)
	{
	    // copying the data into the corresponding tensor
	    for (int y = 0; y < height; ++y)
	    {
	        const double* source_row = imgs[i].ptr<double>(y);
	        for (int x = 0; x < width; ++x)
	        {
	            const double* source_pixel = source_row + (x * depth);
	            for (int c = 0; c < depth; ++c)
	            {
	                const double* source_value = source_pixel + (2 - c); //BGR2RGB
	                input_tensor_mapped(i, y, x, c) = *source_value;
	            }
	        }
	    }
	}
}

void InsightFace_Recognizer::print_feat_vec(std::vector<float> &feat_vec)
{
	dbg("feat vector:\n");
	for (auto feat: feat_vec)
	{
		dbg("%f ", feat);
	}
	dbg("\n");
}

int InsightFace_Recognizer::extract_feat_vec(cv::Mat &img, face_box_t &face, bool detect_in_client)
{
	double start = 0;
	if (g_mode == 1)
		start = now();

    std::vector<Tensor> outputs;
    bool mix_feature = true;
	int feature_num = mix_feature ? 2 : 1;

    tensorflow::Tensor face_tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({feature_num, height, width, depth}));

	dbg("get face img tensor start\n");

	cv::Mat img_copy = img.clone();
	if (detect_in_client)
	{
		get_img_tensor(img_copy, face_tensor, mix_feature);
	}
	else
	{
		cv::Mat face_mat = m_aligner->alignment(img_copy, face);
	    get_img_tensor(face_mat, face_tensor, mix_feature);
    }
	img_copy.release();

	dbg("session run start\n");

	Tensor train_phase(tensorflow::DT_BOOL, tensorflow::TensorShape());
    train_phase.scalar<bool>()() = false;
	Tensor train_phase_last(tensorflow::DT_BOOL, tensorflow::TensorShape());
    train_phase_last.scalar<bool>()() = false;

    Status run_status = m_session->Run({{input_layer_1, face_tensor},
										{input_layer_2, train_phase},
										{input_layer_3, train_phase_last}},
										{output_layer},
										{},
										&outputs);
    #if 0
    Status run_status = m_session->Run({{input_layer, face_tensor}},
    							 		{output_layer},
    							 		{},
    							 		&outputs);
    #endif
    if (!run_status.ok())
    {
        std::cout << "Running model failed" << run_status << std::endl;
        return -1;
    }

	dbg("session run done\n");

	auto feature = outputs[0].tensor<float, 2>();
	cv::Mat feat_mat(1, static_cast<int>(feature.size() / feature_num), CV_32FC1);

	if (mix_feature)
	{
		int feature_size = feature.size() / feature_num;
		cv::Mat flip_feat_mat(1, feature_size, CV_32FC1);

		for (int i = 0; i < feature_size; ++i)
		{
			feat_mat.at<float>(i) = feature(i);
			flip_feat_mat.at<float>(i) = feature(feature_size + i);
		}

		cv::normalize(feat_mat, feat_mat);
		cv::normalize(flip_feat_mat, flip_feat_mat);
		feat_mat += flip_feat_mat;
	}
	else
	{
		for (int i = 0; i < feature.size(); ++i)
		{
			feat_mat.at<float>(i) = feature(i);
		}
	}

	cv::normalize(feat_mat, feat_mat);

	face.feat_vec = (std::vector<float>)feat_mat.reshape(1, 1);

	cout << "feature vec size:" << face.feat_vec.size() << endl;

	//print_feat_vec(face.feat_vec);

	if (g_mode == 1)
	{
		double end = now();
		double elapsed = calc_elapsed(start, end);
		dbg("extract feature time: %f\n", elapsed);
		XCP_LOGGER_INFO(&g_logger, "---> extract feature time: %f\n", elapsed);
	}

    return 0;
}

int InsightFace_Recognizer::regist(cv::Mat &img, face_box_t &face, bool detect_in_client, const std::string &user_name, const std::string &user_id)
{
	dbg("InsightFace regist start\n");
	if (g_user_mgt == NULL)
	{
		dbg("user mgmt init error\n");
		return -1;
	}

	if (g_user_mgt->user_exist(user_id))
	{
		dbg("user_id:%s %s already registed\n", user_id.c_str(), user_name.c_str());
		return -1;
	}

	int ret = extract_feat_vec(img, face, detect_in_client);
	if (ret < 0)
	{
		std::cout << "registe failed" << std::endl;
		return -1;
	}

	face.name = user_name;

	user_t user;
	user.name = user_name;
	user.id = user_id;
	user.feat_vec.assign(face.feat_vec.begin(), face.feat_vec.end());

    std::string save_path = m_data_dir + user_name + "_" + user_id + ".jpg";

	ret = g_user_mgt->add_user(user, save_path);
	if (ret < 0)
	{
		std::cout << "add user failed" << std::endl;
		return -1;
	}

	cv::Rect face_rect(face.box.x1, face.box.y1, face.box.x2 - face.box.x1, face.box.y2 - face.box.y1);
    cv::imwrite(save_path, img(face_rect));

	return 0;
}

int InsightFace_Recognizer::withdraw(const std::string &user_name, const std::string &user_id)
{
	if (g_user_mgt == NULL)
	{
		dbg("user mgmt init error\n");
		return -1;
	}

	int ret = -1;
	user_t user;
	user.name = user_name;
	user.id = user_id;
	ret = g_user_mgt->del_user(user);
	return ret;
}

float InsightFace_Recognizer::cal_similarity(vector<float> &user_feat, vector<float> &face_feat)
{
	cv::Mat uf_mat = cv::Mat(user_feat);
	cv::Mat ff_mat = cv::Mat(face_feat);
	#if 0
	cv::normalize(uf_mat, uf_mat, 1, 0, cv::NORM_L2);
	cv::normalize(ff_mat, ff_mat, 1, 0, cv::NORM_L2);
	return uf_mat.dot(ff_mat) / (sqrt(uf_mat.dot(uf_mat)) * sqrt(ff_mat.dot(ff_mat)));
	#else
	return cv::norm(uf_mat, ff_mat, cv::NORM_L2);
	#endif
}

int InsightFace_Recognizer::recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold)
{
	if (faces.empty())
	{
		std::cout << "No face" << std::endl;
		return -1;
	}

	for (std::vector<face_box_t>::iterator iter = faces.begin(); iter != faces.end(); ++iter)
	{
		int ret = extract_feat_vec(img, *iter, detect_in_client);
		if (ret < 0)
		{
			std::cout << "extract face feature vector failed" << std::endl;
		}

		float min_diff = 10;
		int idx = -1;

		dbg("m_users size:%ld\n", m_users->size());

		for (int i = 0; i < m_users->size(); ++i)
		{
			float diff = cal_similarity(m_users->at(i).feat_vec, (*iter).feat_vec);

			if (diff < threshhold && diff < min_diff)
			{
				dbg("diff: %f, %s\n", diff, m_users->at(i).name.c_str());
				min_diff = diff;
				idx = i;
			}
		}

		if (idx >= 0)
		{
			(*iter).name = m_users->at(idx).name;
			dbg("match user: %s\n", (*iter).name.c_str());
		}
		else
		{
			(*iter).name = "unknown";
			dbg("no match user\n");
		}
	}

	return 0;
}


int InsightFace_Recognizer::recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client,
								const std::string &user_name, const std::string &user_id, const float threshhold)
{
	if (faces.empty())
	{
		std::cout << "No face" << std::endl;
		return -1;
	}

	for (std::vector<face_box_t>::iterator iter = faces.begin(); iter != faces.end(); ++iter)
	{
		int ret = extract_feat_vec(img, *iter, detect_in_client);
		if (ret < 0)
		{
			std::cout << "extract face feature vector failed" << std::endl;
		}

		cv::Rect face_rect((*iter).box.x1, (*iter).box.y1,
						(*iter).box.x2 - (*iter).box.x1, (*iter).box.y2 - (*iter).box.y1);
		std::string save_path = m_data_dir + user_name + "_" + user_id + "_recognized.jpg";

    	cv::imwrite(save_path, img(face_rect));

		float min_diff = 10;
		int idx = -1;

		dbg("m_users size:%ld\n", m_users->size());

		for (int i = 0; i < m_users->size(); ++i)
		{
			float diff = cal_similarity(m_users->at(i).feat_vec, (*iter).feat_vec);

			if (diff < threshhold && diff < min_diff)
			{
				dbg("diff: %f, %s\n", diff, m_users->at(i).name.c_str());
				min_diff = diff;
				idx = i;
			}

			#ifdef DEBUG
			if (m_users->at(i).name == user_name)
			{
				dbg("--->%s diff: [%f]\n", user_name.c_str(), diff);
			}
			#endif
		}

		if (idx >= 0)
		{
			(*iter).name = m_users->at(idx).name;
			dbg("match user: %s\n", (*iter).name.c_str());
		}
		else
		{
			(*iter).name = "unknown";
			dbg("no match user\n");
		}
	}

	return 0;
}

void InsightFace_Recognizer::draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path)
{
	for (std::vector<face_box_t>::iterator it = faces.begin(); it != faces.end(); it++)
	{
		cv::rectangle(img,
					  cv::Point((*it).box.x1, (*it).box.y1),
				      cv::Point((*it).box.x2, (*it).box.y2),
					  color, 4);

		cv::putText(img,
					(*it).name, cv::Point((*it).box.x1, (*it).box.y1 - 5),
					cv::FONT_HERSHEY_COMPLEX,
					1.5,
					color,
					2);
    }

	if (!img_save_path.empty())
    {
    	cv::imwrite(img_save_path, img);
	}

	#ifdef HAS_GUI
	if (show)
	{
    	cv::imshow("recognize face", img);
    	cv::waitKey(0);
    }
    #endif

}
