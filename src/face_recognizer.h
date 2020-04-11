#ifndef __FACE_RECOGNIZER_H__
#define __FACE_RECOGNIZER_H__

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/public/session.h"

#include "face_aligner.h"
#include "face_handler.h"
#include "user_mgt.h"

class Face_Recognizer
{
public:
    virtual int regist(cv::Mat &img, face_box_t &face, bool detect_in_client, const std::string &user_name, const std::string &user_id) = 0;
    virtual int withdraw(const std::string &user_name, const std::string &user_id) = 0;
    virtual int recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold) = 0;
    virtual void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path) = 0;
};


class FaceNet_Recognizer : public Face_Recognizer
{
public:
    FaceNet_Recognizer(const std::string &model_dir, const std::string &data_dir);
    ~FaceNet_Recognizer();
    int regist(cv::Mat &img, face_box_t &face, bool detect_in_client, const std::string &user_name, const std::string &user_id);
    int withdraw(const std::string &user_name, const std::string &user_id);
    int recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold);
    int recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const std::string &user_name, const std::string &user_id, const float threshhold);
    void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path);

private:
    int init_session(const std::string &graph_path);
    int extract_feat_vec(cv::Mat &img, face_box_t &face, bool detect_in_client);
    float cal_similarity(std::vector<float> &user_feat, std::vector<float> &face_feat);
    void get_img_tensor(cv::Mat& Image, tensorflow::Tensor &input_tensor);
    void print_feat_vec(std::vector<float> &feat_vec);

private:
    std::unique_ptr<tensorflow::Session> m_session;
    std::vector<user_t> *m_users;
    std::string m_data_dir;
    std::unique_ptr<Face_Aligner> m_aligner;
};


class InsightFace_Recognizer : public Face_Recognizer
{
public:
    InsightFace_Recognizer(const std::string &model_dir, const std::string &data_dir);
    ~InsightFace_Recognizer();
    int regist(cv::Mat &img, face_box_t &face, bool detect_in_client, const std::string &user_name, const std::string &user_id);
    int withdraw(const std::string &user_name, const std::string &user_id);
    int recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold);
    int recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const std::string &user_name, const std::string &user_id, const float threshhold);
    void draw(cv::Mat &img, std::vector<face_box_t> &faces, const cv::Scalar &color, bool show, std::string &img_save_path);
    int extract_feat_vec(cv::Mat &img, face_box_t &face, bool detect_in_client);

private:
    int init_session(const std::string &graph_path);
    float cal_similarity(std::vector<float> &user_feat, std::vector<float> &face_feat);
    void get_img_tensor(cv::Mat& Image, tensorflow::Tensor &input_tensor, bool mix_feature);
    void print_feat_vec(std::vector<float> &feat_vec);

private:
    std::unique_ptr<tensorflow::Session> m_session;
    std::vector<user_t> *m_users;
    std::string m_data_dir;
    std::unique_ptr<Face_Aligner> m_aligner;
};

#endif
