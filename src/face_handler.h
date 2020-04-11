#ifndef __FACE_HANDLER_H__
#define __FACE_HANDLER_H__

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "common.h"

struct Bbox
{
    float score;
    int x1;
    int y1;
    int x2;
    int y2;
    float area;
    float ppoint[10];
    float regreCoord[4];
};

typedef struct _face_box
{
    struct Bbox box;
    std::string name;
    std::vector<float> feat_vec;
} face_box_t;

template <typename Detector, typename Recognizer>
class Face_Handler
{
public:
	Face_Handler(const std::string &model_dir, const std::string &data_dir)
        :m_detect_in_client(false),
        m_detector(std::make_unique<Detector>(model_dir, data_dir)),
        m_recognizer(std::make_unique<Recognizer>(model_dir, data_dir))
	{
	};

	~Face_Handler()
    {
    };

    int face_detect(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client=false)
    {
        int ret = 0;
        m_detect_in_client = detect_in_client;

        if (detect_in_client)
        {
            face_box_t face;
            face.box.x1 = 0;
            face.box.y1 = 0;
            face.box.x2 = img.cols - 1;
            face.box.y2 = img.rows - 1;
            faces.push_back(face);
        }
        else
        {
            if (m_detector)
            {
                ret = m_detector->detect(img, faces);
            }

            #if 0 // draw in recognizer
            if (ret == 0)
            {
                m_detector->draw(img, cv::Scalar(255, 0, 0), false, "");
            }
            #endif
        }

        return ret;
    };

    int face_regist(cv::Mat &img, face_box_t &face, const std::string &user_name, const std::string &user_id)
    {
        return m_recognizer->regist(img, face, m_detect_in_client, user_name, user_id);
    };

    int face_withdraw(const std::string &user_name, const std::string &user_id)
    {
        return m_recognizer->withdraw(user_name, user_id);
    }

    int face_recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client, const float threshhold=1.0)
    {
        int ret = 0;
        ret = m_recognizer->recognize(img, faces, m_detect_in_client, threshhold);

        if (ret == 0)
        {
            #if 0
            cv::Scalar color(0, 255, 0);
            std::string save_path = "";
            m_recognizer->draw(img, m_faces, color, false, save_path);
            #endif
        }

        #if 0
        std::string save_path = "../data/recognize_" + user_name + ".jpg";
        m_detector->draw(img, cv::Scalar(0, 0, 255), false, save_path);
        #endif

        return ret;
    };

    int face_recognize(cv::Mat &img, std::vector<face_box_t> &faces, bool detect_in_client,
                        const std::string &user_name, const std::string &user_id, const float threshhold=1.0)
    {
        int ret = 0;
        ret = m_recognizer->recognize(img, faces, m_detect_in_client, user_name, user_id, threshhold);
        return ret;
    };

private:
    bool m_detect_in_client;
    std::unique_ptr<Detector> m_detector;
    std::unique_ptr<Recognizer> m_recognizer;
};


#endif
