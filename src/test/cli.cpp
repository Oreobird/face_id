#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include "../common.h"
#include "../codecs.h"
#include "../msg_oper.h"
#include "../protocol.h"
#include "base_logger.h"
using namespace std;
using namespace cv;

#define IMG_TEST 1

#define BUFF_SIZE 1024
#define TIME_OUT 5

extern base::Logger g_logger;

std::string json_path;
std::string img_path;
std::string result_img_path;

struct s_client {
    int fd;
    int (*create_socket) (void);
    int (*connect_srv) (const char *ip, int port);
    int (*send_to_srv) (evg_msg_t *msg, int msg_len);
    int (*recv_from_srv) (evg_msg_t *msg);
    void (*close_connect) (void);
} g_client;

void usage(char *bin_name)
{
    printf("usage: %s -i image_file -j json_file -o result.jpg -t type\n", bin_name);
   	printf("	-i: image file path to detect.\n");
   	printf("	-j: output json file path.\n");
   	printf("	-o: output detect result image file path.\n");
   	printf("	-t: landmark detect type, int [0, 2]:0-recognize, 1-regist, 2-withdraw\n");
    printf("eg: %s -i ./001.jpg -t 1 -u zgs -d zgs\n", bin_name);
}

int set_socket_nonblock(int fd)
{
    int flags = -1;
    int ret = -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        dbg("fcntl get socket error:%s\n", strerror(errno));
        return -1;
    }

    flags |= O_NONBLOCK;

    ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        dbg("fcntl set socket error:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int set_socket_block(int fd)
{
    int flags = -1;
    int ret = -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        dbg("fcntl get socket error:%s\n", strerror(errno));
        return -1;
    }

    flags &= (~O_NONBLOCK);

    ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        dbg("fcntl set socket error:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int check_connect_timeout(int fd)
{
    struct timeval timeout;
    fd_set rdset;
    int optval;
    int ret = -1;
    socklen_t len;

    if (errno == EINPROGRESS)
    {
        len = sizeof(int);
        timeout.tv_sec = TIME_OUT;
        timeout.tv_usec = 0;
        FD_ZERO(&rdset);
        FD_SET(fd, &rdset);

        ret = select(fd + 1, NULL, &rdset, NULL, &timeout);
        if (ret < 0)
        {
            dbg("Connect error, err msg:%s\n", strerror(errno));
            return -1;
        }
        else if (ret == 0)
        {
            dbg("Connect timeout, err msg:%s\n", strerror(errno));
            return -1;
        }
        else
        {
            ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &len);
            if (ret < 0)
            {
                dbg("getsockopt(SO_ERROR):%s\n", strerror(errno));
                return -1;
            }

            if (optval)
            {
               dbg("Client connect error:%s\n", strerror(optval));
               return -1;
           }
        }
    }
    else
    {
        dbg("Connect to server failed\n");
        return -1;
    }

    return 0;
}

int connect_server(const char *ip, int port)
{
    struct sockaddr_in srv;
    int ret = -1;

    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(ip);
    srv.sin_port = htons(port);

    ret = set_socket_nonblock(g_client.fd);
    if (ret < 0)
    {
        close(g_client.fd);
        return -1;
    }

    ret = connect(g_client.fd, (struct sockaddr*)&srv, sizeof(srv));
    if (ret != 0)
    {
        ret = check_connect_timeout(g_client.fd);
        if (ret < 0)
        {
            close(g_client.fd);
            return -1;
        }
    }

    ret = set_socket_block(g_client.fd);
    if (ret < 0)
    {
        close(g_client.fd);
        return -1;
    }

    return 0;
}

int send_to_server(evg_msg_t *msg, int msg_len)
{
    if (g_client.fd > 0 && msg && msg_len > 0)
    {
        int data_left = msg_len;
        int nwrite = 0;
		char *ptr = (char *)msg;

        while (data_left > 0)
        {
         	nwrite = write(g_client.fd, ptr, data_left);
         	if (nwrite <= 0)
         	{
         		if (nwrite < 0 && errno == EINTR)
         			nwrite = 0;
         		else
         			break;
         	}

            data_left -= nwrite;
            ptr += nwrite;
        }

        dbg("need write:%d, writen data len:%d\n", msg_len, msg_len - data_left);
    }

    return 0;
}

int recv_from_server(evg_msg_t *msg)
{
	if (g_client.fd > 0)
	{
	    unsigned int nread = 0;
		unsigned int buf_len = 0;
        nread = read(g_client.fd, (char *)&buf_len, sizeof(buf_len));
        buf_len = ntohl(buf_len);
		dbg("read buf_len:%d\n", buf_len);

		if (buf_len > 0)
		{
			int msg_len = (sizeof(evg_msg_t) + buf_len) * sizeof(char);
		    msg = (evg_msg_t *)malloc(msg_len);
		    if (msg == NULL)
		    {
		    	dbg("malloc failed\n");
		    	return -1;
		    }

			memset(msg, 0, msg_len);
			msg->buf_len = buf_len;

			dbg("reading for msg buf\n");

	        int data_left = msg->buf_len;
			char *ptr = msg->buf;
			nread = 0;

	        while (data_left > 0)
	        {
	        	nread = read(g_client.fd, ptr, data_left);
	        	if (nread < 0)
	        	{
	        		if (errno == EINTR || errno == EAGAIN)
	        		{
	        			nread = 0;
	        		}
	        		else
	        		{
	        			dbg("errno:%s\n", strerror(errno));
	        			break;
	        		}
	        	}
	        	else if (nread == 0)
	        	{
	        		break;
	        	}

	            data_left -= nread;
	            ptr += nread;
	        }

			dbg("need read:%d, read data len:%d\n", msg->buf_len, msg->buf_len - data_left);

	        if (nread < 0 && errno != EAGAIN)
	        {
	            dbg("read data failed\n");
	            close(g_client.fd);
	            return -1;
	        }
	        else if (nread == 0)
	        {
	            dbg("disconnted\n");
	            close(g_client.fd);
	            return -1;
	        }

			unsigned int err_code;
			std::string errInfo;

			//dbg("msg:%s\n", msg->buf);

	    	int nRet = XProtocol::get_specific_params(msg->buf, "err_code", err_code, errInfo);
			dbg("err_code: %d\n", err_code);

			std::string user_name;
			nRet = XProtocol::get_specific_params(msg->buf, "user_name", user_name, errInfo);

			dbg("user name: %s\n", user_name.c_str());

			std::string img_result_str;

			nRet = XProtocol::get_specific_params(msg->buf, "img_data", img_result_str, errInfo);

			cv::Mat img_result = MatCodecs::base64_to_mat(img_result_str);

			#if 0
			if (!img_result.empty())
			{
				cv::imshow("img_result", img_result);
			}
			#endif
		    if (msg)
		    	free(msg);
	    }
	}

	return 0;
}

void close_connect(void)
{
    if (g_client.fd > 0)
    {
        close(g_client.fd);
    }
}

int create_socket(void)
{
   int fd = -1;

   fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
   if (fd < 0)
   {
       dbg("creage_socket failed\n");
       return -1;
   }

   return fd;
}

static int init_client(struct s_client *client)
{
    if (client == NULL)
    {
        return -1;
    }

    client->fd = -1;
    client->create_socket = create_socket;
    client->connect_srv = connect_server;
    client->send_to_srv = send_to_server;
    client->recv_from_srv = recv_from_server;
    client->close_connect = close_connect;
    return 0;
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char default_img[256] = {0};
	char default_json[256] = {0};
	char default_result[256] = {0};
	char ch;
	int req_type = 0;
	std::string user_name;
	std::string user_id;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);

	while ((ch = getopt(argc, argv, "hj:i:o:t:u:d:")) != EOF)
	{
		switch (ch)
		{
			case 'j':
				json_path = optarg;
				break;
			case 'i':
				img_path = optarg;
				break;
			case 'o':
				result_img_path = optarg;
				break;
			case 't':
				req_type = atoi(optarg);
				break;
			case 'u':
				user_name = optarg;
				break;
			case 'd':
				user_id = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_path.empty())
	{
		snprintf(default_img, sizeof(default_img), "%s/sample.jpg", prog_path);
		img_path = default_img;
	}
	if (json_path.empty())
	{
		snprintf(default_json, sizeof(default_json), "%s/quality.json", prog_path);
		json_path = default_json;
	}
	if (result_img_path.empty())
	{
		snprintf(default_result, sizeof(default_result), "%s/result.jpg", prog_path);
		result_img_path = default_result;
	}
	if (user_name.empty())
	{
		user_name = "Oreo";
	}
	if (user_id.empty())
	{
		user_id = "44088119000101xxxx";
	}


    int ret = init_client(&g_client);
    if (ret < 0)
    {
        dbg("init_client failed\n");
        return -1;
    }

    g_client.fd = g_client.create_socket();
    ret = g_client.connect_srv("127.0.0.1", 8080);
    if (ret < 0)
    {
        dbg("connect server failed\n");
        return -1;
    }

	#if IMG_TEST
	Mat img = imread(img_path);
	if (img.empty())
	{
		dbg("imread failed\n");
		return -1;
	}

	std::string data_str = MatCodecs::mat_to_base64(img);
    dbg("data_str len:%d\n", (int)(data_str.length()));
	#endif

	std::ostringstream req;
	bool detect_in_client = false;
	unsigned int mode = 0; // 1: test mode

	req	<< "{\"type\":" << req_type
		<< ",\"id\":\"" << user_id
		<< "\", \"name\":\"" << user_name
		<< "\", \"detect_in_client\":" << (detect_in_client ? "true" : "false")
		<< ", \"mode\":" << mode
    #if IMG_TEST
     	<< ",\"img_data\":\"" << data_str
    #else
	 	<< ",\"video_path\":\"" << img_path
	#endif
		<< "\"}";

	//std::cout << req.str() << std::endl;
    evg_msg_t *msg;
	int buf_len = req.str().length() + 1;
	int msg_len = (sizeof(evg_msg_t) + buf_len) * sizeof(char);

    msg = (evg_msg_t *)malloc(msg_len);
    if (msg == NULL)
    {
    	dbg("malloc failed\n");
    	return -1;
    }

	memset(msg, 0, msg_len);

	msg->buf_len = htonl(buf_len);
	memcpy(msg->buf, (char*)req.str().c_str(), buf_len);

	g_client.send_to_srv(msg, msg_len);
	if (msg)
		free(msg);

	evg_msg_t *recv_msg;
	g_client.recv_from_srv(recv_msg);

	g_client.close_connect();
	return 0;
}
