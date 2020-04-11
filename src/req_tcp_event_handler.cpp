#include <arpa/inet.h>
#include "base_socket_oper.h"
#include "base_time.h"
#include "base_net.h"
#include "base_string.h"
#include "base_uid.h"
#include "base_logger.h"
#include "base_logger_period.h"

#include "req_tcp_event_handler.h"
#include "req_mgt.h"
#include "msg_oper.h"


extern Logger g_logger;
extern StSysInfo g_sysInfo;
Req_Mgt *g_req_mgt;

USING_NS_BASE;
Req_TCP_Event_Handler::Req_TCP_Event_Handler(): _buf(""), _cnt(0),_buf_len(0),_pre_time(0)
{

};

Req_TCP_Event_Handler::~Req_TCP_Event_Handler()
{

};

int Req_TCP_Event_Handler::handle_accept(int fd)
{
	int nRet = 0;

	std::string local_ip = "";
	unsigned short local_port = 0;
	base::get_local_socket(fd, local_ip, local_port);

	std::string remote_ip = "";
	unsigned short remote_port = 0;
	base::get_remote_socket(fd, remote_ip, remote_port);

	XCP_LOGGER_INFO(&g_logger, "accept from app(fd:%d), %s:%u --> %s:%u\n",
		fd, remote_ip.c_str(), remote_port, local_ip.c_str(), local_port);


	return nRet;

};



/*
处理读事件
telnet 消息结尾\r\n
客户端消息结尾\n
*/
int Req_TCP_Event_Handler::handle_input(int fd)
{
	int nRet = 0;

	std::string ip = "";
	unsigned short port = 0;
	get_remote_socket(fd, ip, port);

	unsigned int buf_len = 0;
	unsigned int size = sizeof(buf_len);
	nRet = Socket_Oper::recv(fd, (char *)&buf_len, size, 300000);
	if(nRet == 0)
	{
		XCP_LOGGER_INFO(&g_logger, "rcv close form app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
		return -1;
	}
	else if(nRet == 1)
	{
		//XCP_LOGGER_INFO(&g_logger, "rcv success from app(fd:%d), req(%u):%s\n", fd, buf_len, buf);
	}
	else
	{
		XCP_LOGGER_INFO(&g_logger, "rcv failed from app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
		return 0;
	}

	_buf_len = ntohl(buf_len);
	dbg("recv: %d\n", _buf_len);

	char *buf = new char[_buf_len + 1];
	memset(buf, 0, _buf_len+1);

	#if 1
	nRet = Socket_Oper::recv_n(fd, buf, _buf_len, 1000000);
	if(nRet == 0)
	{
		XCP_LOGGER_INFO(&g_logger, "rcv close from app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
		delete[] buf;
		return -1;
	}
	else if(nRet == 1)
	{
		//XCP_LOGGER_INFO(&g_logger, "rcv success from app(fd:%d), req(%u):%s\n", fd, buf_len, buf);
	}
	else
	{
		XCP_LOGGER_INFO(&g_logger, "rcv failed from app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
		delete[] buf;
		return 0;
	}
	#else
	int data_left = _buf_len;
	char *ptr = buf;
	int nread = 0;
	while (data_left > 0)
	{
		nread = read(fd, ptr, data_left);
		if (nread < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				nread = 0;
			}
			else
			{
				XCP_LOGGER_INFO(&g_logger, "rcv failed from app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
				delete[] buf;
				return 0;
			}
		}
		else if (nread == 0)
		{
			XCP_LOGGER_INFO(&g_logger, "rcv close form app(fd:%d, peer:%s:%u), ret:%d\n", fd, ip.c_str(), port, nRet);
			delete[] buf;
			return -1;
		}

		data_left -= nread;
		ptr += nread;
	}
	#endif

	buf[_buf_len] = '\0';

	std::string msg_tag = UID::uid_inc(g_sysInfo._log_id);

	std::string session_id = "";
	Request_Ptr req = new Request;
	req->_rcv_stmp = get_microsecond();
	req->_req = buf;
	delete[] buf;
	req->_msg_tag = msg_tag;
	req->_ip = ip;
	req->_port = port;
	req->_fd = fd;

	XCP_LOGGER_INFO(&g_logger, "prepare to push into req mgt, req:%s\n", req->to_string().c_str());

	unsigned int used_size = 0;
	unsigned int max_size = 0;
	if(!(g_req_mgt->full(used_size, max_size)))
	{
		nRet = g_req_mgt->push_req(req);
		if(nRet != 0)
		{
			XCP_LOGGER_ERROR(&g_logger, "push into req mgt failed, ret:%d, req:%s\n", nRet, req->to_string().c_str());
			Msg_Oper::send_msg(fd, session_id, "", "false", "", "", 0, msg_tag, ERR_PUSH_QUEUE_FAIL, "server is busy, try it later!");
		}

	}
	else
	{
		XCP_LOGGER_ERROR(&g_logger, "req mgt is full, req:%s\n", req->to_string().c_str());
		Msg_Oper::send_msg(fd, session_id, "", "false", "", "", 0, msg_tag, ERR_QUEUE_FULL, "req mgt is full.");
	}

	return 0;

};




//处理连接关闭事件
int Req_TCP_Event_Handler::handle_close(int fd)
{
	int nRet = 0;
	std::string err_info = "";

	std::string local_ip = "";
	unsigned short local_port = 0;
	get_local_socket(fd, local_ip, local_port);

	std::string remote_ip = "";
	unsigned short remote_port = 0;
	get_remote_socket(fd, remote_ip, remote_port);

	XCP_LOGGER_INFO(&g_logger, "close from app(fd:%d), %s:%u --> %s:%u,errno:%d\n",
		fd, remote_ip.c_str(), remote_port, local_ip.c_str(), local_port,errno);

	return nRet;

};




Event_Handler* Req_TCP_Event_Handler::renew()
{
	return new Req_TCP_Event_Handler;
};


