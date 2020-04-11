#ifndef __FACE_ALIGNER_H__
#define __FACE_ALIGNER_H__

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "face_handler.h"

class Face_Aligner
{
public:
    virtual cv::Mat alignment(cv::Mat &img, face_box_t &face) = 0;
    virtual void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path) = 0;
};


class Opencv_Aligner : public Face_Aligner
{
public:
    Opencv_Aligner(const std::string &data_dir);
    ~Opencv_Aligner();
    cv::Mat alignment(cv::Mat &img, face_box_t &face);
    void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path);

private:
    std::string m_data_dir;
};


#endif
