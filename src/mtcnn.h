
#pragma once

#ifndef __MTCNN_NCNN_H__
#define __MTCNN_NCNN_H__


#include "ncnn/net.h"
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>
#include <map>
#include <iostream>

#include "face_handler.h"

using namespace std;

class MTCNN {

public:
    MTCNN(const string &model_path);

    MTCNN(std::vector<std::string> param_files, std::vector<std::string> bin_files);

    ~MTCNN();

    void SetMinFace(int minSize);

    void detect_face(cv::Mat &img_, std::vector<face_box_t> &faces);

private:
    void generateBbox(ncnn::Mat score, ncnn::Mat location, vector<Bbox> &boundingBox_, float scale);

    void nms(vector<Bbox> &boundingBox_, const float overlap_threshold, string modelname = "Union");

    void refine(vector<Bbox> &vecBbox, const int &height, const int &width, bool square);

    void PNet();

    void RNet();

    void ONet();

    ncnn::Net Pnet, Rnet, Onet;
    ncnn::Mat img;
    const float nms_threshold[3] = {0.5f, 0.7f, 0.7f};

    const float mean_vals[3] = {127.5, 127.5, 127.5};
    const float norm_vals[3] = {0.0078125, 0.0078125, 0.0078125};
    const int MIN_DET_SIZE = 12;
    std::vector<Bbox> firstBbox_, secondBbox_, thirdBbox_;
    int img_w, img_h;

private:
    const float threshold[3] = {0.8f, 0.8f, 0.6f};
    int minsize = 40;
    const float pre_facetor = 0.709f;

};


#endif //__MTCNN_NCNN_H__
