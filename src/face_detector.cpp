#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "face_detector.h"
#include "mtcnn.h"

using namespace cv;
using namespace std;

Mtcnn_Detector::Mtcnn_Detector(const string &model_dir, const string &data_dir)
				:m_net(std::make_unique<MTCNN>(model_dir)),
				m_aligner(std::make_unique<Opencv_Aligner>(data_dir))
{
}

Mtcnn_Detector::~Mtcnn_Detector()
{
}

int Mtcnn_Detector::detect(cv::Mat &img, std::vector<face_box_t> &faces)
{
	if (m_net)
	{
		m_net->detect_face(img, faces);
	}

	return 0;
}

std::vector<cv::Mat> Mtcnn_Detector::align(cv::Mat &img, std::vector<face_box_t> &faces)
{
	std::vector<cv::Mat> faces_mat;
	if (m_aligner)
	{
		for (auto face : faces)
		{
			cv::Mat face_mat = m_aligner->alignment(img, face);
			faces_mat.push_back(face_mat);
		}
	}

	return faces_mat;
}

void Mtcnn_Detector::draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path)
{
	for (std::vector<face_box_t>::iterator it = faces.begin(); it != faces.end(); it++)
	{
		cv::rectangle(img,
						cv::Point((*it).box.x1, (*it).box.y1),
						cv::Point((*it).box.x2, (*it).box.y2),
						color, 4);
    }

	if (!img_save_path.empty())
    {
    	cv::imwrite(img_save_path, img);
	}

	#ifdef HAS_GUI
	if (show)
	{
    	cv::imshow("mtcnn detected face", img);
    	cv::waitKey(0);
    }
    #endif
}
