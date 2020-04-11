
#include "base_udp_client.h"
#include "base_net.h"
#include "base_os.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


NS_BASE_BEGIN

UDP_Client::UDP_Client(): _fd(-1), _connect(false), _close(true), _ip(""), _port(0)
{

}


UDP_Client::~UDP_Client()
{
	close();
}


int UDP_Client::connect(const std::string &ip, unsigned short port, bool doconnect)
{
	int nRet = 0;
	
	_fd = socket(AF_INET, SOCK_DGRAM, 0); 
	if(_fd < 0)
	{
		printf("socket(fd:%d) failed, errno:%d, errmsg:%s\n", _fd, errno, strerror(errno));
		return _fd;
	}
	else
	{
		printf("socket(%d) success.\n", _fd);
	}

	set_non_bolck(_fd);
	
	if(doconnect)
	{
		struct sockaddr_in addr;
		memset(&addr, 0x0, sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		inet_aton(ip.c_str(), &(addr.sin_addr));
		addr.sin_port = htons(port);
		nRet = ::connect(_fd, (struct sockaddr *)&addr, sizeof(addr));
		if(nRet != 0)
		{
			printf("connect(%s:%u) failed, errno:%d, errmsg:%s\n", 
					ip.c_str(), port, errno, strerror(errno));
			return nRet;
		}
		printf("connect(%s:%u) success.\n", ip.c_str(), port);
		_connect = true;

	}

	_ip = ip;
	_port = port;
	_close = false;

	return nRet;

}



int UDP_Client::sendto(const char *buf, const unsigned int len, int flags)
{
	int nRet = 0;

	if(_close)
	{
		printf("the udp client is closed.\n");
		return -1;
	}

	if(_connect)
	{
		nRet = ::sendto(_fd, buf, len, 0, NULL, 0);
		if(nRet < 0)
		{
			printf("sendto failed, fd:%d, ret:%d, errno:%d, errmsg:%s\n", _fd, nRet, errno, strerror(errno));
		}
	}
	else
	{
		struct sockaddr_in addr;
		memset(&addr, 0x0, sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		inet_aton(_ip.c_str(), &(addr.sin_addr));
		addr.sin_port = htons(_port);
		nRet = ::sendto(_fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
		if(nRet < 0)
		{
			printf("sendto failed, fd:%d, ret:%d, errno:%d, errmsg:%s\n", _fd, nRet, errno, strerror(errno));
		}
	}

	return nRet;

}




int UDP_Client::recvfrom(char *buf, const unsigned int len)
{
	int nRet = 0;
	
	if(_close)
	{
		printf("the udp client is closed.\n");
		return -1;
	}

	int addrlen = 0;
	struct sockaddr_in addr;
	nRet = ::recvfrom(_fd, buf, len, 0, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
	if(nRet < 0)
	{
		printf("recvfrom failed, fd:%d, ret:%d, errno:%d, errmsg:%s\n", _fd, nRet, errno, strerror(errno));
	}

	return nRet;
	
}



void UDP_Client::close()
{
	if(!_close)
	{
		::close(_fd);
		_close = true;
	}
}


bool UDP_Client::is_close()
{
	return _close;
}


int UDP_Client::fd()
{
	return _fd;
}


NS_BASE_END


