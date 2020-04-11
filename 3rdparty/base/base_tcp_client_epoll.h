
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

 

#ifndef _BASE_TCP_CLIENT_EPOLL_H
#define _BASE_TCP_CLIENT_EPOLL_H

#include "base_common.h"
#include "base_thread.h"
#include "base_event_handler.h"
#include "base_thread_mutex.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "base_thread.h"
#include <sys/epoll.h>

NS_BASE_BEGIN


//一个TCP_Client 一个线程
class TCP_Client_Epoll : public Thread
{
public:
	/*
	handler是由外部应用new 产生，由TCP_Client 自动释放
	[注意]: 
	(1)如果使用长连接需要new TCP_Cient，外部应用不能
	delete它；
	(2)如果使用短链接需要外部应用自己保证释放它；
	
	*/
	TCP_Client_Epoll(Event_Handler *handler=NULL, bool asyn=false);

	virtual ~TCP_Client_Epoll();

	int open(const std::string &ip, unsigned short port, unsigned int timeout=1000000);

	int send_msg(const char *buf, unsigned int &len, int flags=0, unsigned int timeout=1000000);

	int rcv_msg(char *buf, unsigned int &len, unsigned int timeout=1000000);

	void close();

	bool is_close();

	int fd();

private:
	virtual int prepare();

	virtual int svc();

	virtual int do_init(void *args);
	
private:
	int epoller_create(int epoll_size);

	int epoller_ctl(int fd, int op, unsigned int events);

private:
	Event_Handler *_handler;
	std::string _ip;
	unsigned _port;
	int _fd;
	bool _asyn;
	bool _open;

	int _epfd;
	struct epoll_event *_ep_events;
	int _epoll_size;

	Thread_Mutex _mutex;
};

NS_BASE_END

#endif


