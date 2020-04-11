#include "protocol.h"
#include "base_time.h"
#include "base_string.h"
#include "base_convert.h"
#include "base_logger.h"
#include "base_base64.h"
#include "common.h"

#include "rapidjson/document.h"        // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"    // for stringify JSON
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

extern base::Logger g_logger;

#define GET_JSON_STRING_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsString()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't string, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't string type"); \
            return -1; \
        } \
        result = node.GetString(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_ARRAY_NODE(father, name, node, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        node = (father)[name.c_str()]; \
        if(!node.IsArray()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't array type, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't array type"); \
            return -1; \
        } \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_UNSIGNED_INTEGER_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsUint()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't unsigned integer, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't unsigned integer type"); \
            return -1; \
        } \
        result = node.GetUint(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_BOOL_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsBool()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't bool, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't bool type"); \
            return -1; \
        } \
        result = node.GetBool(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}


#define GET_JSON_DOUBLE_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsDouble()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't double, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't double type"); \
            return -1; \
        } \
        result = node.GetDouble(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}


int XProtocol::req_parse(const std::string &req,
								unsigned int &req_type,
								std::string &img_data,
								std::string &user_id,
								std::string &user_name,
								bool &detect_in_client,
								int &mode,
								float &threshhold,
						 		std::string &err_info)
{
    std::string nodeName;
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

	//type
	nodeName = "type";
	GET_JSON_UNSIGNED_INTEGER_NODE(root, nodeName, req_type, err_info, true)

    //img_data
    nodeName = "img_data";
    GET_JSON_STRING_NODE(root, nodeName, img_data, err_info, true);

	nodeName = "id";
    GET_JSON_STRING_NODE(root, nodeName, user_id, err_info, req_type == 1 ? true : false);

    //user_name
    nodeName = "name";
    GET_JSON_STRING_NODE(root, nodeName, user_name, err_info, req_type == 1 ? true : false);

	nodeName = "detect_in_client";
	GET_JSON_BOOL_NODE(root, nodeName, detect_in_client, err_info, false);

	nodeName = "threshhold";
	GET_JSON_DOUBLE_NODE(root, nodeName, threshhold, err_info, false);

	nodeName = "mode";
	GET_JSON_UNSIGNED_INTEGER_NODE(root, nodeName, mode, err_info, false)

	return 0;
}


std::string XProtocol::gen_no_face_result(const int err_code, const std::string &msg)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"msg\":\"" << msg
			<< "\"}";

	return json_str.str();
}

std::string XProtocol::gen_regist_result(const int err_code, const std::string &msg)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"msg\":\"" << msg
			<< "\"}";

	return json_str.str();
}

std::string XProtocol::gen_recognize_result(const std::string &img_str, int width, int height, const face_box_t &face, const int err_code)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"user_name\":\"" << face.name
			<< "\",\"box\":[" << (static_cast<float>(face.box.x1) / width)
			<< "," << (static_cast<float>(face.box.y1) / height)
			<< "," << (static_cast<float>(face.box.x2) / width)
			<< "," << (static_cast<float>(face.box.y2) / height)
			<< "],\"img_data\":\"" << img_str
			<< "\"}";

	return json_str.str();
}

std::string XProtocol::gen_failed_result(const int err_code, const std::string &err_msg)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"err_msg\":\"" << err_msg
			<< "\"}";

	return json_str.str();
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, std::string &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_STRING_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%s\n", paramName.c_str(), result.c_str());

    return 0;
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, bool &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_BOOL_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%u\n", paramName.c_str(), result);

    return 0;
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, unsigned int &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_UNSIGNED_INTEGER_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%u\n", paramName.c_str(), result);

    return 0;
}
