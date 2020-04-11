
/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: cuipingxu918@qq.com
 */

 

#ifndef _BASE_SIMPLE_UDP_REACTOR_H
#define _BASE_SIMPLE_UDP_REACTOR_H

#include "base_common.h"
#include "base_event_handler.h"
#include "base_thread.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "base_thread.h"
#include <sys/epoll.h>


NS_BASE_BEGIN


struct StSimpleUDPReactorAgrs
{
	std::string ip;
	unsigned short port;
	unsigned int epoll_size;

	StSimpleUDPReactorAgrs()
	{
		ip = "";
		port = 3300;
		epoll_size = 10;
	}
};


/*
UDP Reactor 重要特性:
服务器每次最多只能获取8092字节数据。
由唯一的一个event handler 接收处理。
*/
class Simple_UDP_Reactor : public Thread
{
public:	
	//handler必须由外部应用通过new 创建， 不需要关心它的释放
	Simple_UDP_Reactor(Event_Handler *handler);

	virtual ~Simple_UDP_Reactor();

	void stop();
	
private:
	virtual int prepare();
	
	virtual int svc();

	virtual int do_init(void *args);
	
private:
	int epoller_create(int epoll_size);

	int epoller_ctl(int fd, int op, unsigned int events);

	int do_bind(const std::string &ip, unsigned short port);
		
	void release();
	
private:
	Event_Handler *_handler;

	int _fd;
	int _epfd;
	struct epoll_event *_ep_events;
	int _epoll_size;
	
};


NS_BASE_END

#endif


