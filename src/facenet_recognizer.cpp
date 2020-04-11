#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "base_logger.h"
#include "face_recognizer.h"
#include "user_mgt.h"
#include "timing.h"
#include "common.h"
#include "conf_mgt.h"

using namespace cv;
using namespace std;
using namespace tensorflow;
using tensorflow::Tensor;
using tensorflow::Status;

const int height = 160;
const int width = 160;
const int depth = 3;

extern int g_mode;
extern User_Mgt *g_user_mgt;
extern Logger g_logger;
extern StModelInfo g_modelInfo;

const float margin_thresh = 0.75;
const string input_layer_1 = "input:0";
const string input_layer_2 = "phase_train:0";
const string output_layer = "embeddings:0";

int FaceNet_Recognizer::init_session(const string &graph_path)
{
    tensorflow::GraphDef graph_def;
	tensorflow::SessionOptions opts;

	TF_CHECK_OK(ReadBinaryProto(tensorflow::Env::Default(), graph_path, &graph_def));

	//graph::SetDefaultDevice("/gpu:0", &graph_def);
	opts.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction(0.5);
	opts.config.mutable_gpu_options()->set_allow_growth(true);

	m_session.reset(tensorflow::NewSession(opts));
	TF_CHECK_OK(m_session->Create(graph_def));

	#if 1
	// Walk around to load libcublas.so.9.0 before client request
    Tensor phase_train(tensorflow::DT_BOOL, tensorflow::TensorShape());
    phase_train.scalar<bool>()() = false;

    tensorflow::Tensor face_tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1, height, width, depth}));
    std::vector<Tensor> outputs;

    TF_CHECK_OK(m_session->Run({{input_layer_1, face_tensor},
    							 		{input_layer_2, phase_train}},
    							 		{output_layer},
    							 		{},
    							 		&outputs));
	#endif

    return 0;
}

FaceNet_Recognizer::FaceNet_Recognizer(const std::string &model_dir, const std::string &data_dir)
{
    init_session(model_dir + g_modelInfo._recognizer);
    m_data_dir = data_dir;
    m_users = g_user_mgt->get_users();
    m_aligner = std::make_unique<Opencv_Aligner>(m_data_dir);
}


FaceNet_Recognizer::~FaceNet_Recognizer()
{
}

void FaceNet_Recognizer::get_img_tensor(cv::Mat& img, tensorflow::Tensor &input_tensor)
{
	if (img.empty())
	{
		return;
	}
    resize(img, img, Size(width, height));

    auto input_tensor_mapped = input_tensor.tensor<float, 4>();

    //mean and std
    //c * r * 3 => c * 3r * 1
    cv::Mat temp = img.reshape(1, img.rows * 3);

    cv::Mat mean3;
    cv::Mat stddev3;
    cv::meanStdDev(temp, mean3, stddev3);

    double mean_pxl = mean3.at<double>(0);
    double stddev_pxl = stddev3.at<double>(0);

    //prewhiten
    img.convertTo(img, CV_64FC1);
    img = img - cv::Vec3d(mean_pxl, mean_pxl, mean_pxl);
    img = img / stddev_pxl;

    // copying the data into the corresponding tensor
    for (int y = 0; y < height; ++y)
    {
        const double* source_row = img.ptr<double>(y);
        for (int x = 0; x < width; ++x)
        {
            const double* source_pixel = source_row + (x * depth);
            for (int c = 0; c < depth; ++c)
            {
                const double* source_value = source_pixel + (2-c);//RGB->BGR
                input_tensor_mapped(0, y, x, c) = *source_value;
            }
        }
    }
    // cout<<"The image preprocess cost "<<1000 *(double)(getTickCount()-start)/getTickFrequency()<<" ms"<<endl;
}

void FaceNet_Recognizer::print_feat_vec(std::vector<float> &feat_vec)
{
	dbg("feat vector:\n");
	for (auto feat: feat_vec)
	{
		dbg("%f ", feat);
	}
	dbg("\n");
}

int FaceNet_Recognizer::extract_feat_vec(cv::Mat &img, face_box_t &face, bool detect_in_client)
{
	double start = 0;
	if (g_mode == 1)
		start = now();

    Tensor phase_train(tensorflow::DT_BOOL, tensorflow::TensorShape());
    phase_train.scalar<bool>()() = false;

    std::vector<Tensor> outputs;
    tensorflow::Tensor face_tensor(tensorflow::DT_FLOAT, tensorflow::TensorShape({ 1, height, width, depth}));

	dbg("extract_feat_vec start\n");

	cv::Mat img_copy = img.clone();

	if (detect_in_client)
	{
		get_img_tensor(img_copy, face_tensor);
	}
	else
	{
		cv::Mat face_mat = m_aligner->alignment(img_copy, face);
	    get_img_tensor(face_mat, face_tensor);
    }

	img_copy.release();

	dbg("session run start\n");
    Status run_status = m_session->Run({{input_layer_1, face_tensor},
    							 		{input_layer_2, phase_train}},
    							 		{output_layer},
    							 		{},
    							 		&outputs);
    if (!run_status.ok())
    {
        std::cout << "Running model failed" << run_status << std::endl;
        return -1;
    }
	dbg("session run done\n");
    auto feature = outputs[0].tensor<float, 2>();

    for (int i = 0; i < feature.size(); ++i)
    {
        face.feat_vec.push_back(feature(i));
	}

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

int FaceNet_Recognizer::regist(cv::Mat &img, face_box_t &face, bool detect_in_client, const std::string &user_name, const std::string &user_id)
{
	dbg("facenet regist start\n");
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
	dbg("face feature vector extract done\n");

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

int FaceNet_Recognizer::withdraw(const std::string &user_name, const std::string &user_id)
{
	int ret = -1;
	user_t user;
	user.name = user_name;
	user.id = user_id;
	ret = g_user_mgt->del_user(user);
	return ret;
}

float FaceNet_Recognizer::cal_similarity(vector<float> &user_feat, vector<float> &face_feat)
{
	#if 0
	double sum = 0;
    for (int i = 0; i < face_feat.size(); i++)
    {
        sum += (face_feat[i] - user_feat[i]) * (face_feat[i] - user_feat[i]);
    }

    return sqrt(sum);
    #else
	cv::Mat user_feat_mat = cv::Mat(user_feat);
	cv::Mat face_feat_mat = cv::Mat(face_feat);
	return cv::norm(user_feat_mat, face_feat_mat, cv::NORM_L2);
	#endif
}

int FaceNet_Recognizer::recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold)
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

int FaceNet_Recognizer::recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client,
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
		std::string save_path = m_data_dir + user_name + "_" + user_id + ".jpg";

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

void FaceNet_Recognizer::draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path)
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
