#include <sys/ioctl.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h> //#include <linux/tcp.h>
#include <sys/ioctl.h>

#include "base_socket_oper.h"
#include "base_net.h"
#include "base_logger.h"

#include "msg_oper.h"


extern Logger g_logger;

Thread_Mutex Msg_Oper::_mutex_msg;

int Msg_Oper::send_msg(int fd, const std::string &session_id, const std::string &uuid, const std::string &encry, const std::string &key,
	const std::string &method, const unsigned int req_id, const std::string &msg_tag,
	const int code, const std::string &msg, const std::string &body, bool is_object)
{
	int nRet = 0;

	std::string buf = "";
	unsigned int len = buf.size();
	{
		Thread_Mutex_Guard guard(_mutex_msg);
// 		int opt = 1;
// 		nRet = setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt));
// 		if(nRet < 0)
// 		{
// 			XCP_LOGGER_ERROR(&g_logger, "setsockopt TCP_QUICKACK failed, fd:%d. errno:%d, errmsg:%s\n", fd, errno, strerror(errno));
// 			return -1;
// 		}
		//nRet = Socket_Oper::send_n(fd, buf.c_str(), len, 0, 300000);
		nRet = Socket_Oper::send_s(fd, buf.c_str(), len, 0);
	}

	if(nRet == 0)
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg success. fd:%d, buf(%u):%s\n", fd, buf.size(), buf.c_str());
	}
	else if(nRet == 2)
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg timeout, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, buf.size(), buf.c_str());
		XCP_LOGGER_INFO(&g_logger, "send(%d) failed, errno:%d, errmsg:%s\n", fd, errno, strerror(errno));
	}
	else
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg to failed, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, buf.size(), buf.c_str());
	}

	return nRet;

}


int Msg_Oper::send_evg_msg(int fd, const std::string &buf)
{
	int nRet = 0;
	unsigned int buf_len = buf.length() + 1;

	//dbg("json_str len:%d\n", buf_len);

	unsigned int msg_len = sizeof(evg_msg_t) + buf_len;

	evg_msg_t *msg = (evg_msg_t *) new char[msg_len];
	if (msg == NULL)
	{
		XCP_LOGGER_INFO(&g_logger, "Malloc fail!\n");
		return -1;
	}

	memset(msg, 0, msg_len);
	msg->buf_len = htonl(buf_len);
	memcpy(msg->buf, buf.c_str(), buf_len);

	{
		Thread_Mutex_Guard guard(_mutex_msg);
		// 		int opt = 1;
		// 		nRet = setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt));
		// 		if(nRet < 0)
		// 		{
		// 			XCP_LOGGER_ERROR(&g_logger, "setsockopt TCP_QUICKACK failed, fd:%d. errno:%d, errmsg:%s\n", fd, errno, strerror(errno));
		// 			return -1;
		// 		}
		//nRet = Socket_Oper::send_n(fd, buf.c_str(), len, 0, 300000);
		nRet = Socket_Oper::send_n(fd, (char *)msg, msg_len, 0, 1000000);
	}

	if(nRet == 0)
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg success. fd:%d, buf(%u):%s\n", fd, msg_len, buf.c_str());
	}
	else if(nRet == 2)
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg timeout, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, msg_len, buf.c_str());
		XCP_LOGGER_INFO(&g_logger, "send(%d) failed, errno:%d, errmsg:%s\n", fd, errno, strerror(errno));
	}
	else
	{
		XCP_LOGGER_INFO(&g_logger, "send rsp msg to failed, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, msg_len, buf.c_str());
	}

	if (msg)
	{
		delete [] msg;
		msg = NULL;
	}

	return nRet;

}

int Msg_Oper::send_msg(int fd, const std::string &buf)
{
	int nRet = 0;

	std::string new_req = buf + std::string("\n");
	unsigned int len = new_req.size();
	{
		Thread_Mutex_Guard guard(_mutex_msg);
		nRet = Socket_Oper::send_n(fd, new_req.c_str(), len, 0, 1000000);
	}

	if(nRet == 0)
	{
		XCP_LOGGER_INFO(&g_logger, "send msg success. fd:%d, buf(%u):%s\n", fd, buf.size(), buf.c_str());
	}
	else if(nRet == 2)
	{
		XCP_LOGGER_ERROR(&g_logger, "send msg timeout, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, buf.size(), buf.c_str());
	}
	else
	{
		XCP_LOGGER_ERROR(&g_logger, "send msg to failed, ret:%d, fd:%d, buf(%u):%s\n",
			nRet, fd, buf.size(), buf.c_str());
	}

	return nRet;

}
