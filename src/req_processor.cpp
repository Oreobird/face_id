#include "req_processor.h"
#include "protocol.h"
#include "req_mgt.h"
#include "base_net.h"
#include "base_string.h"
#include "base_logger.h"
#include "base_uid.h"
#include "base_utility.h"

#include "msg_oper.h"
#include "conf_mgt.h"
#include "face_handler_mgt.h"
#include "face_handler.h"
#include <arpa/inet.h>

extern int g_mode;
extern Logger g_logger;
extern StSysInfo g_sysInfo;
extern Req_Mgt *g_req_mgt;
extern Face_Handler_Mgt *g_face_handler_mgt;

using namespace cv;
using namespace std;

#define MAX_CLIENT 100

Req_Processor::Req_Processor()
{

}


Req_Processor::~Req_Processor()
{

}

int Req_Processor::do_init(void *args)
{
	return 0;
}

int Req_Processor::svc()
{
	int nRet = 0;
	std::string err_info = "";
	std::string json_str;

	Request_Ptr req;
	nRet = g_req_mgt->get_req(req);
	if(nRet != 0)
	{
		return 0;
	}

	unsigned int req_type = 0;
	std::string user_id = "";
	std::string user_name = "";
	std::string img_data = "";
	bool detect_in_client = false;
	float threshhold = 1.0;

	nRet = XProtocol::req_parse(req->_req, req_type, img_data, user_id, user_name,
								detect_in_client, g_mode, threshhold, err_info);
	if (nRet != 0)
	{
		XCP_LOGGER_INFO(&g_logger, "it is invalid req, ret:%d, err_info:%s, req:%s\n", nRet, err_info.c_str(), req->to_string().c_str());
		json_str = XProtocol::gen_failed_result(ERR_SYSTEM, err_info);
	}
	else
	{
		if (!img_data.empty())
		{
			Mat img = MatCodecs::base64_to_mat(img_data);

			dbg("req len:%ld, img width:%d ,height:%d\n", strlen(req->_req.c_str()), img.cols, img.rows);

			XCP_LOGGER_INFO(&g_logger, "img width:%d ,height:%d\n", img.cols, img.rows);

			if (img.empty())
			{
				XCP_LOGGER_ERROR(&g_logger, "img is empty.\n");
				json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "img decode failed.");
			}
			else
			{
				std::shared_ptr<face_handler_t> handler;

				nRet = g_face_handler_mgt->get_handler(handler);
				if (nRet != 0)
				{
					XCP_LOGGER_INFO(&g_logger, "get detector failed\n");
					return 0;
				}

				std::vector<face_box_t> faces;

				if (handler != nullptr && handler->face_handler != nullptr)
				{
		        	nRet = handler->face_handler->face_detect(img, faces, detect_in_client);

			    	if (nRet < 0)
			    	{
			    		XCP_LOGGER_INFO(&g_logger, "face detect failed\n");
						json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "face detect error.");
			    		return -1;
			    	}

					if (faces.size() > 0)
					{
						if (req_type == 1)
						{
							dbg("face regist\n");
							int ret = handler->face_handler->face_regist(img, faces[0], user_name, user_id);
							if (ret < 0)
							{
								XCP_LOGGER_INFO(&g_logger, "face regist failed\n");
								json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "face regist error.");
							}
							else
							{
								json_str = XProtocol::gen_regist_result(ERR_SUCCESS, "Face regist OK.");
							}
						}
						else if (req_type == 2)
						{
							dbg("face withdraw\n");
							int ret = handler->face_handler->face_withdraw(user_name, user_id);
							if (ret < 0)
							{
								XCP_LOGGER_INFO(&g_logger, "face regist failed\n");
								json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "face delete error.");
							}
							else
							{
								json_str = XProtocol::gen_regist_result(ERR_SUCCESS, "Face delete OK.");
							}
						}
						else
						{
							dbg("face recognize\n");
							int ret = 0;

							if (g_mode == 1) // test mode
							{
								ret = handler->face_handler->face_recognize(img, faces, detect_in_client, user_name, user_id, threshhold);
							}
							else
							{
								ret = handler->face_handler->face_recognize(img, faces, detect_in_client, threshhold);
							}

							if (ret < 0)
							{
								XCP_LOGGER_INFO(&g_logger, "face recognize failed\n");
								json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "face recognize error.");
							}
							else
							{
								cv::Rect face_rect(faces[0].box.x1, faces[0].box.y1,
													faces[0].box.x2 - faces[0].box.x1,
													faces[0].box.y2 - faces[0].box.y1);
		    					cv::Mat face_mat = img(face_rect).clone();

								std::string img_data_str = MatCodecs::mat_to_base64(face_mat);
								face_mat.release();
								json_str = XProtocol::gen_recognize_result(img_data_str, img.cols, img.rows, faces[0], ERR_SUCCESS);
							}
						}
					}
					else
					{
						XCP_LOGGER_INFO(&g_logger, "No face detect.\n");
						json_str = XProtocol::gen_failed_result(ERR_NO_FACE_DETECT, "No face detect.");
					}
				}

				g_face_handler_mgt->put_handler(handler);
			}
		}
	}

	#if 1
	Msg_Oper::send_evg_msg(req->_fd, json_str);
	#else
	Msg_Oper::send_msg(req->_fd, json_str);
	#endif
	return 0;

}
