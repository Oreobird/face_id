#include "base_string.h"
#include "base_args_parser.h"
#include "base_logger.h"

#include "request.h"

extern Logger g_logger;

Request::Request(): _msg_tag(""), _req_id(0), _req(""), _msg_type(""), _fd(-1), _ip(""), _port(0),
	_app_stmp(0), _rcv_stmp(0), _process_stmp(0), _session_id(""), _uuid(""), _key(""), _encry("")
{

}


Request::~Request()
{

}


void Request::log()
{
	XCP_LOGGER_INFO(&g_logger, "req: req_id=%u, msg_type=%s, msg_tag=%s, fd=%d, client=(%s:%u), app_stmp:%llu, rcv_stmp:%llu, process_stmp:%llu, session id:%s, uuid:%s, key:%s, encry:%s, req(%u):%s\n",
		_req_id, _msg_type.c_str(), _msg_tag.c_str(), _fd, _ip.c_str(), _port, _app_stmp, _rcv_stmp, _process_stmp, _session_id.c_str(), _uuid.c_str(), _key.c_str(), _encry.c_str(), _req.size(), _req.c_str());
}


std::string Request::to_string()
{
	return format("req: req_id=%u, msg_type=%s, msg_tag=%s, fd=%d, client=(%s:%u), app_stmp:%llu, rcv_stmp:%llu, process_stmp:%llu, session id:%s, uuid:%s, key:%s, encry:%s, req(%u):%s",
		_req_id, _msg_type.c_str(), _msg_tag.c_str(), _fd, _ip.c_str(), _port, _app_stmp, _rcv_stmp, _process_stmp, _session_id.c_str(), _uuid.c_str(), _key.c_str(), _encry.c_str(), _req.size(), _req.c_str());
}


