#include "base_os.h"
#include "base_convert.h"
#include "base_string.h"
#include "base_logger.h"

#include "face_handler_mgt.h"

using namespace std;

Face_Handler_Mgt *g_face_handler_mgt = NULL;

Face_Handler_Mgt::Face_Handler_Mgt():_cond(_mutex)
{
}


Face_Handler_Mgt::~Face_Handler_Mgt()
{
}


int Face_Handler_Mgt::init(const std::string &model_dir, const std::string &data_dir)
{
	int nRet = 0;

	get_cpu_number_proc(m_handler_size);
	m_handler_size = 1;
	m_free_handler_num = m_handler_size;

	for (int i = 0; i < m_handler_size; i++)
	{
		auto handler(std::make_shared<face_handler_t>());
		if (handler == NULL)
		{
			return -1;
		}

		#ifdef ARCFACE
		handler->face_handler = std::make_shared<Face_Handler<Mtcnn_Detector, InsightFace_Recognizer>>(model_dir, data_dir);
		#else
		handler->face_handler = std::make_shared<Face_Handler<Mtcnn_Detector, FaceNet_Recognizer>>(model_dir, data_dir);
		#endif

		if (handler->face_handler == NULL)
		{
			std::cout << "face handler init fail" << std::endl;
			return -1;
		}

		handler->busy = false;

		if (true)
		{
			base::Thread_Mutex_Guard guard(_mutex);
			m_handlers.push_back(handler);
		}
	}

	return nRet;
}

int Face_Handler_Mgt::get_handler(std::shared_ptr<face_handler_t> &handler)
{
	int nRet = 0;

	base::Thread_Mutex_Guard guard(_mutex);

	if (m_free_handler_num == 0)
	{
		//cout << "====wait for free handler========" << endl;
		nRet = _cond.wait(); // wait for free detector
		if (nRet != 0)
		{
			return nRet;
		}
	}

	std::vector<std::shared_ptr<face_handler_t>>::iterator iter;
	for (iter = m_handlers.begin(); iter != m_handlers.end(); ++iter)
	{
		if (!((*iter)->busy))
		{
			//dbg("found free detector\n");
			(*iter)->busy = true;
			handler = *iter;
			m_free_handler_num--;
			//dbg("---> free detector num:%d\n", m_free_handler_num);
			return 0;
		}
	}

	return nRet;
}

int Face_Handler_Mgt::put_handler(std::shared_ptr<face_handler_t> &handler)
{
	int nRet = 0;
	//dbg("===put_handler========\n");

	if (handler == nullptr)
	{
		return -1;
	}

	base::Thread_Mutex_Guard guard(_mutex);

	handler->busy = false;

	m_free_handler_num++;

	nRet = _cond.signal();

	return nRet;
}



