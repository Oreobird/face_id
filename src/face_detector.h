#ifndef __FACE_DETECTOR_H__
#define __FACE_DETECTOR_H__

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "face_handler.h"
#include "face_aligner.h"
#include "mtcnn.h"

class Face_Detector
{
public:
    virtual int detect(cv::Mat &img, std::vector<face_box_t> &faces) = 0;
    virtual void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path) = 0;
};

// MTCNN face detector
class Mtcnn_Detector : public Face_Detector
{
public:
    Mtcnn_Detector(const std::string &model_dir, const std::string &data_dir);
    ~Mtcnn_Detector();
    int detect(cv::Mat &img, std::vector<face_box_t> &faces);
    std::vector<cv::Mat> align(cv::Mat &img, std::vector<face_box_t> &faces);
    void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path);
private:
    std::unique_ptr<MTCNN> m_net;
    std::unique_ptr<Face_Aligner> m_aligner;
};

// Other face detector
//

#endif
