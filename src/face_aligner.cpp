#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "face_handler.h"
#include "face_aligner.h"
#include "common.h"

using namespace cv;
using namespace std;

Opencv_Aligner::Opencv_Aligner(const std::string &data_dir):m_data_dir(data_dir)
{
}

Opencv_Aligner::~Opencv_Aligner()
{
}

cv::Mat Opencv_Aligner::alignment(cv::Mat &img, face_box_t &face)
{
	cv::Point2f left_eye(face.box.ppoint[0], face.box.ppoint[5]);
	cv::Point2f right_eye(face.box.ppoint[1], face.box.ppoint[6]);

    //eyes center
    cv::Point2f eyes_center = cv::Point2f((left_eye.x + right_eye.x) * 0.5f, (left_eye.y + right_eye.y) * 0.5f);

    //eyes angle
    double dy = right_eye.y - left_eye.y;
    double dx = right_eye.x - left_eye.x;
    double angle = atan2(dy, dx) * 180.0 / CV_PI;

    cv::Mat rot_mat = cv::getRotationMatrix2D(eyes_center, angle, 1.0);
    cv::Mat align_img;

    cv::warpAffine(img, align_img, rot_mat, img.size());
    std::vector<Point2f> marks;

	#if 0
    for (int n = 0; n < sizeof(face.box.ppoint) / 2; n++)
    {
        Point2f p = Point2f(0, 0);
        p.x = rot_mat.ptr<double>(0)[0] * face.box.ppoint[n] + rot_mat.ptr<double>(0)[1] * face.box.ppoint[n + 5] + rot_mat.ptr<double>(0)[2];
        p.y = rot_mat.ptr<double>(1)[0] * face.box.ppoint[n] + rot_mat.ptr<double>(1)[1] * face.box.ppoint[n + 5] + rot_mat.ptr<double>(1)[2];
        marks.push_back(p);
    }

    for (int j = 0; j < marks.size(); j++)
    {
        circle(align_img, marks[j], 2, Scalar(0, 0, 255));
    }
	#endif

	cv::Rect face_rect(face.box.x1, face.box.y1, face.box.x2 - face.box.x1, face.box.y2 - face.box.y1);
    cv::Mat face_mat = align_img(face_rect);

	cv::imwrite("../data/align_face_" + std::to_string(angle)+".jpg", face_mat);

    return face_mat;
}

void Opencv_Aligner::draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path)
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
