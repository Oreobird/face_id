
#include "base_socket_oper.h"
#include "base_tcp_client_epoll.h"
#include "base_net.h"
#include "base_os.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


NS_BASE_BEGIN


TCP_Client_Epoll::TCP_Client_Epoll(Event_Handler *handler, bool asyn): _handler(handler), 
	_ip(""), _port(0), _fd(-1), _asyn(asyn), _open(false), _epfd(-1), _ep_events(NULL), _epoll_size(0)
{

}


TCP_Client_Epoll::~TCP_Client_Epoll()
{
	printf("destroy tcp client!\n");
	close();

	if(_asyn)
	{
		DELETE_POINTER(_handler);
		DELETE_POINTER_ARR(_ep_events);	
	}
}



/*
同步TCP Client的一个问题是如果是对端关闭连接， 本端是不知道连接关闭的
除非本端发送消息才能检测到。
*/
int TCP_Client_Epoll::open(const std::string &ip, unsigned short port, unsigned int timeout)
{
	int nRet = 0;
	
	if(_open)
	{
		printf("the tcp client is open ago!\n");
		return -1;
	}

	_ip = ip;
	_port = port;
	
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(_fd < 0)
	{
		printf("socket(fd:%d) failed, errno:%d, errmsg:%s\n", _fd, errno, strerror(errno));
		_fd = -1;
		return _fd;
	}
	else
	{
		//printf("socket(%d) success.\n", _fd);
	}

	nRet = Socket_Oper::connect_s(_fd, ip, port, timeout);
	if(nRet != 0)
	{
		//printf("connect failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
		::close(_fd);
		_fd = -1;
		return nRet;
	}
	else
	{
		printf("connect (%s:%u) success.\n", ip.c_str(), port);
	}

	if(_asyn)
	{
		nRet = init();
		if(nRet != 0)
		{
			printf("init tcp client failed, ret:%d\n", nRet);
			::close(_fd);
			_fd = -1;
	
			return nRet;
		}

		nRet = run();
		if(nRet != 0)
		{
			printf("run tcp client failed, ret:%d\n", nRet);

			TCP_Client_Epoll::epoller_ctl(_fd, EPOLL_CTL_DEL, 0);
			::close(_epfd);
			_epfd = -1;	
				
			::close(_fd);
			_fd = -1;
			
			return nRet;
		}
		
	}

	if(nRet == 0)
	{
		_open = true;
	}
	
	return nRet;
	
}



int TCP_Client_Epoll::send_msg(const char *buf, unsigned int &len, int flags, unsigned int timeout)
{
	int nRet = 0;
	
	if(!_open)
	{
		printf("the tcp client is open ago!\n");
		return -1;
	}

	Thread_Mutex_Guard guard(_mutex);

	nRet = Socket_Oper::send_n(_fd, buf, len, flags, timeout);
	
	return nRet;
}



int TCP_Client_Epoll::rcv_msg(char *buf, unsigned int &len, unsigned int timeout)
{
	int nRet = 0;

	if(!_open)
	{
		printf("the tcp client is open ago!\n");
		return -1;
	}
		
	if(!_asyn)
	{
		Thread_Mutex_Guard guard(_mutex);
		nRet = Socket_Oper::recv(_fd, buf, len, timeout);
	}
	else
	{
		printf("the socket is asyn mode. it is useless.\n");
		nRet = -2;
	}

	return nRet;
	
}



void TCP_Client_Epoll::close()
{
	if(!_open)
	{
		return;
	}
	
	printf("close tcp client(%s:%u)!\n", _ip.c_str(), _port);

	if(_asyn)
	{
		TCP_Client_Epoll::epoller_ctl(_fd, EPOLL_CTL_DEL, 0);
		
		::close(_epfd);
		_epfd = -1;	
	}

	::close(_fd);
	_open = false;
	_fd = -1;

	if(_asyn)
	{		
		//线程停止放在最后执行防止crash
		Thread::stop();
	}
	
}




bool TCP_Client_Epoll::is_close()
{
	return !_open;
}


int TCP_Client_Epoll::fd()
{
	return _fd;
}


int TCP_Client_Epoll::prepare()
{
	int nRet = 0;

	//阻塞进程信号SIGINT
	nRet = Thread::signal_mask(SIG_BLOCK, 1, SIGINT);
	if (nRet != 0)
	{
		printf("signal_mask failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}
	
	return nRet;
}



int TCP_Client_Epoll::svc()
{
	int nRet = 0;
	
	int fd_cnt = epoll_wait(_epfd, _ep_events, _epoll_size, 1000);
	if(fd_cnt >0)
	{
		for(int i=0; i<fd_cnt; i++)
		{
			if(_ep_events[i].events & EPOLLIN )
			{
				//处理输入事件
				nRet = _handler->handle_input(_ep_events[i].data.fd);
				if(nRet != 0)
				{
					std::string local_ip = "";
					unsigned short local_port = 0;
					get_local_socket(_fd, local_ip, local_port);
					
					std::string remote_ip = "";
					unsigned short remote_port = 0; 
					get_remote_socket(_fd, remote_ip, remote_port);
					
					printf("handle_input failed! prepare to close(fd:%d), %s:%u --> %s:%u\n", 
						_fd, local_ip.c_str(), local_port, remote_ip.c_str(), remote_port);
					
					TCP_Client_Epoll::epoller_ctl(_fd, EPOLL_CTL_DEL, 0);
					_handler->handle_close(_fd);
					close();

				}
				
			}
			else
			{
				printf("other events is arrived.\n");
			}
			
		}
		
	}
	else if(fd_cnt == 0)
	{
		//printf("epoll_wait timeout, errno:%d, errmsg:%s\n", errno, strerror(errno));
	}
	else
	{
		if(errno == EINTR)
		{
			printf("epoll_wait is interrupt, errno:%d, errmsg:%s\n", errno, strerror(errno));
		}
		else
		{
			printf("epoll_wait failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
			return -1;
		} 
	}

	return 0;


}
	


int TCP_Client_Epoll::do_init(void *args)
{
	int nRet = 0;

	if(_asyn)
	{
		//获取系统进程最大句柄数
		struct rlimit rlim;
		nRet = get_rlimit(RLIMIT_NOFILE, &rlim);
		if(nRet != 0)
		{
			printf("get_rlimit failed, ret:%d\n", nRet);
			return nRet;
		}
		//printf("get_rlimit RLIMIT_NOFILE, rlim_cur:%d\n", (int)rlim.rlim_cur);

		//计算出最大的nfds
		int epoll_size = rlim.rlim_cur;
		nRet = TCP_Client_Epoll::epoller_create(epoll_size);
		if(nRet != 0)
		{
			printf("epoller_create failed, ret:%d\n", nRet);
			return nRet;
		}

		nRet = TCP_Client_Epoll::epoller_ctl(_fd, EPOLL_CTL_ADD, EPOLLIN);
		if(nRet != 0)
		{
			printf("epoller_create failed, ret:%d\n", nRet);
			return nRet;
		}		
		
	}
	else
	{
		printf("it isn't asyn tcp client.\n");
		return -1;
	}
	
	return nRet;
	
}
	


int TCP_Client_Epoll::epoller_create(int epoll_size)
{
	int nRet = 0;
	
	_epfd = epoll_create(epoll_size);
	if(_epfd == -1)
	{
		printf("epoll_create failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
		return -1;
	}
	
	_epoll_size = epoll_size;
	_ep_events = new struct epoll_event[_epoll_size];
	
	return nRet;
}




int TCP_Client_Epoll::epoller_ctl(int fd, int op, unsigned int events)
{
	int nRet = 0;

	struct epoll_event ep_ev;
	ep_ev.data.fd = fd;
	ep_ev.events = events;
	nRet = epoll_ctl(_epfd, op, fd, &ep_ev); 
	if(nRet != 0)
	{
		printf("epoll_ctl failed, fd:%d, errno:%d, errmsg:%s\n", fd, errno, strerror(errno));
	}

	return nRet;
	
}



NS_BASE_END


