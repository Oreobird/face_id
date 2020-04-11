
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
 * Author: 89617663@qq.com
 */

 

#ifndef _BASE_REACTOR_H
#define _BASE_REACTOR_H

#include "base_common.h"
#include "base_event_handler.h"
#include "base_thread.h"
#include "base_thread_mutex.h"
#include "base_hqueue.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "base_thread.h"
#include <sys/epoll.h>


NS_BASE_BEGIN


struct StReactorAgrs
{
	std::string ip;
	unsigned short port;
	unsigned int backlog;
	unsigned int epoll_size;

	StReactorAgrs()
	{
		ip = "";
		port = 3300;
		backlog = 20480;
		epoll_size = 40960;
	}
	
};



class Reactor : public Thread
{
public:	
	//handler必须由外部应用通过new 创建， 不需要关心它的释放
	Reactor(Event_Handler *handler);

	virtual ~Reactor();

	int listen_fd();

	//关闭监听，停止网络io 服务
	void stop_listen();
	
private:
	virtual int prepare();
	
	virtual int svc();

	virtual int do_init(void *args);
	
private:
	int epoller_create(int epoll_size);

	int epoller_ctl(int fd, int op, unsigned int events);

	int do_listen(const std::string &ip, unsigned short port, int backlog);

	void release();

public:
	//该接口不允许在event_handler的回调函数中被调用，只能在其他的线程中被调用
	void del_fd(int fd);
	
private:
	Event_Handler *_handler;
	H_Queue<Event_Handler_Ptr> _handlers;

	int _listen_fd;
	int _epfd;
	struct epoll_event *_ep_events;
	int _epoll_size;
	
};


NS_BASE_END

#endif


