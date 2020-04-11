
#include "base_udp_reactor.h"
#include "base_net.h"
#include "base_os.h"
#include "base_string.h"

NS_BASE_BEGIN


UDP_Reactor::UDP_Reactor(Event_Handler *handler): _handler(handler), _fd(-1),
	_epfd(-1), _ep_events(NULL), _epoll_size(0)
{

}


UDP_Reactor::~UDP_Reactor()
{
	release();
}


int UDP_Reactor::prepare()
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


int UDP_Reactor::do_init(void *args)
{
	int nRet = 0;
	
	if(!args)
	{
		printf("agrs is NULL\n");
		return -1;
	}
	StUDPReactorAgrs *_args = (StUDPReactorAgrs *)args;

	//获取系统进程最大句柄数
	struct rlimit rlim;
	nRet = get_rlimit(RLIMIT_NOFILE, &rlim);
	if(nRet != 0)
	{
		printf("get_rlimit failed, ret:%d\n", nRet);
		return nRet;
	}
	printf("get_rlimit RLIMIT_NOFILE, rlim_cur:%d\n", (int)rlim.rlim_cur);

	//计算出最大的nfds
	int epoll_size = rlim.rlim_cur > (_args->epoll_size) ? (_args->epoll_size) : rlim.rlim_cur;
	
	nRet = UDP_Reactor::epoller_create(epoll_size);
	if(nRet != 0)
	{
		printf("epoller_create failed, ret:%d\n", nRet);
		return nRet;
	}

	nRet = UDP_Reactor::do_bind(_args->ip, _args->port);
	if(nRet != 0)
	{
		printf("do_bind failed, ret:%d\n", nRet);
		return nRet;
	}
	
	return nRet;
	
}




int UDP_Reactor::do_bind(const std::string &ip, unsigned short port)
{
	int nRet = 0;
	
	struct sockaddr_in addr;
	bzero(&addr, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	if(ip != "")
	{
		inet_aton(ip.c_str(), &(addr.sin_addr));
	}
	else
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	addr.sin_port = htons(port);

	_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(_fd < 0)
	{
		printf("socket failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
		return _fd;
	}

	set_reuseaddr(_fd);
	set_non_bolck(_fd);
	set_rcvbuf(_fd, 16777216);  //udp 读缓冲区32MB

	/*
	绑定后UDP 服务就启动了
	#netstat -nap|grep server
	Active Internet connections (servers and established)
	udp        0      0 0.0.0.0:9900                0.0.0.0:*          1920/./server.bin 
	*/
	nRet = bind(_fd, (sockaddr*)&addr, sizeof(addr));
	if(nRet != 0)
	{
		printf("bind(%s:%u) failed, errno:%d, errmsg:%s\n", ip.c_str(), port, errno, strerror(errno));
		close(_fd);
		return nRet;
	}
	
	nRet = UDP_Reactor::epoller_ctl(_fd, EPOLL_CTL_ADD, EPOLLIN);
	if(nRet != 0)
	{
		close(_fd);
		return nRet;
	}
	
	printf("---- udp bind (%s:%u) ----\n", ip.c_str(), port);
	
	return nRet;
	
}



void UDP_Reactor::release()
{
	printf("release udp reactor!\n");
	
	close(_epfd);
	close(_fd);
	_epfd = -1;
	DELETE_POINTER(_handler);
	DELETE_POINTER_ARR(_ep_events);

	std::map<std::string, Event_Handler*>::iterator itr = _handlers.begin();
	for(; itr != _handlers.end(); ++itr)
	{				
		DELETE_POINTER(itr->second);
		_handlers.erase(itr);
	}
}



int UDP_Reactor::svc()
{
	int fd_cnt = epoll_wait(_epfd, _ep_events, _epoll_size, -1);
	if(fd_cnt >0)
	{
		for(int i=0; i<fd_cnt; i++)
		{
			if((_ep_events[i].events & EPOLLIN) && (_fd == _ep_events[i].data.fd))
			{
				struct sockaddr_in addr;
				int addr_len = sizeof(addr);

				char buf[8096] = {0};
			    int rcv_num = recvfrom(_fd, buf, 8095, 0, (struct sockaddr *)&addr, (socklen_t*)&addr_len);
			    if(rcv_num < 0)
			    {
			    	//接收失败
			    	printf("recvfrom failed, fd:%d, errno:%d, errmsg:%s\n", 
						_fd, errno, strerror(errno));
			    }
				else if(rcv_num >= 0)
				{
					char ip[40] = {0};
				  	if (NULL == inet_ntop(AF_INET, (char*)&addr.sin_addr.s_addr, ip, sizeof(ip)-1))
					{
						printf("inet_ntop failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
						break;
					}

					unsigned short port = ntohs(addr.sin_port);
					std::string client_id = format("%s_%u", ip, port);
					
					if(rcv_num == 0)
					{
						//接收关闭
						//客户端需要发送长度为0 的udp报文表示关闭了连接
						printf("recvfrom close, fd:%d\n", _fd);
						
						std::map<std::string, Event_Handler*>::iterator itr = _handlers.find(client_id);
						if(itr != _handlers.end())
						{
							itr->second->handle_close(ip, port);
							DELETE_POINTER(itr->second);
							_handlers.erase(itr);
						}
						else
						{
							printf("client id(%s) isn't found.\n", client_id.c_str());
						}

					}
					else
					{
						//接收数据
						printf("recvfrom success, fd:%d\n", _fd);
						
						std::map<std::string, Event_Handler*>::iterator itr = _handlers.find(client_id);
						if(itr == _handlers.end())
						{
							printf("prepare to handle new input, fd:%d\n", _fd);
							Event_Handler *handler = _handler->renew();
							_handlers.insert(std::make_pair(client_id, handler));
							handler->handle_input(ip, port, buf, rcv_num);
						}
						else
						{
							printf("prepare to handle input, fd:%d\n", _fd);
							itr->second->handle_input(ip, port, buf, rcv_num);
						}					
					}
				
				}

				break;
				
			}	
			
		}
		
	}
	else if(fd_cnt == 0)
	{
		printf("epoll_wait timeout, errno:%d, errmsg:%s\n", errno, strerror(errno));
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
			::abort();
			//return -1;
		} 
	}

	return 0;
	
}



int UDP_Reactor::epoller_create(int epoll_size)
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




int UDP_Reactor::epoller_ctl(int fd, int op, unsigned int events)
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


