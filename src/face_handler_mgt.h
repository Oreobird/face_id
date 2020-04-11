#ifndef __FACE_HANDLER_MGT_H__
#define __FACE_HANDLER_MGT_H__

#include "base_common.h"
#include "base_singleton_t.h"
#include "base_thread_mutex.h"
#include "base_rw_thread_mutex.h"
#include "base_condition.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include "common.h"

#include "face_detector.h"
#include "face_recognizer.h"
#include "face_handler.h"

//typedef unsigned long long uint64;
USING_NS_BASE;
using namespace std;
using namespace cv;

typedef struct _face_handler
{
    #ifdef ARCFACE
	std::shared_ptr<Face_Handler<Mtcnn_Detector, InsightFace_Recognizer>> face_handler;
    #else
    std::shared_ptr<Face_Handler<Mtcnn_Detector, FaceNet_Recognizer>> face_handler;
    #endif
    bool busy;
} face_handler_t;


class Face_Handler_Mgt
{
public:
	Face_Handler_Mgt();

	~Face_Handler_Mgt();

	int init(const std::string &model_dir, const std::string &data_dir);

    int get_handler(std::shared_ptr<face_handler_t> &handler);

    int put_handler(std::shared_ptr<face_handler_t> &handler);


private:
	Thread_Mutex _mutex;
	std::vector<std::shared_ptr<face_handler_t>> m_handlers;
    unsigned short m_handler_size;

    unsigned short m_free_handler_num;
    Condition _cond;
};

#define PSGT_Face_Handler_Mgt Singleton_T<Face_Handler_Mgt>::getInstance()

#endif
