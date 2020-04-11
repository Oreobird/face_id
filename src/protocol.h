#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "common.h"
#include <iostream>
#include "face_handler.h"

using namespace std;

class XProtocol
{
public:
    static int req_parse(const std::string &req,
								unsigned int &req_type,
								std::string &img_data,
                                std::string &user_id,
                                std::string &user_name,
                                bool &detect_in_client,
                                int &mode,
                                float &threshhold,
						 		std::string &err_info);

    static std::string gen_no_face_result(const int err_code, const std::string &msg);
    static std::string gen_regist_result(const int err_code, const std::string &msg);
    static std::string gen_recognize_result(const std::string &img_str, int width, int height, const face_box_t &face, const int err_code);

    static std::string gen_failed_result(const int err_code, const std::string &err_msg);

    static int get_specific_params(const std::string &req, const std::string &paramName, std::string &result, std::string &err_info);
    static int get_specific_params(const std::string &req, const std::string &paramName, bool &result, std::string &err_info);
    static int get_specific_params(const std::string &req, const std::string &paramName, unsigned int &result, std::string &err_info);
};


#endif

