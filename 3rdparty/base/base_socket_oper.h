
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

 

#ifndef _BASE_SOCKET_OPER_H
#define _BASE_SOCKET_OPER_H

#include "base_common.h"

NS_BASE_BEGIN


class Socket_Oper
{
public:
	
	/*
	在超时时间中循环接收指定长度的数据(同步阻塞方式)
	buf由外部应用申请空间
	len指定外部分配空间大小，处理后返回接收到的数据长度
	timeout单位是微秒
	返回值:
	(1) 0:fd关闭
	(2) 1:正常
	(3) 2:超时
	(4)小于0: fd接收失败返回负值错误码
	*/
	static int recv_n(int fd, char *buf, unsigned int &len, const unsigned int timeout=10000);



	/*
	在超时时间中仅接收一次指定长度的数据(同步阻塞方式)
	buf由外部应用申请空间
	len指定外部分配空间大小，处理后返回接收到的数据长度
	timeout单位是微秒
	返回值:
	(1) 0:fd关闭
	(2) 1:正常
	(3) 2:超时
	(4)小于0: fd接收失败返回负值错误码
	*/
	static int recv(int fd, char *buf, unsigned int &len, const unsigned int timeout=10000);
	


	/*
	在超时时间中循环发送指定长度的数据(同步阻塞方式)
	buf由外部应用申请空间
	len指定外部分配空间大小，处理后返回接收到的数据长度
	timeout单位是微秒
	返回值:
	(1) 1:正常
	(2) 2:超时
	(3)小于0: fd接收失败返回负值错误码
	*/
	static int send_n(int fd, const char *buf, unsigned int &len, int flags=0, const unsigned int timeout=10000);


	/*
	在超时时间中仅发送一次指定长度的数据(同步阻塞方式)
	buf由外部应用申请空间
	len指定外部分配空间大小，处理后返回接收到的数据长度
	timeout单位是微秒
	返回值:
	(1) 1:正常
	(2) 2:超时
	(3)小于0: fd接收失败返回负值错误码
	*/
	static int send(int fd, const char *buf, unsigned int &len, int flags=0, const unsigned int timeout=10000);


	static int send_s(int fd, const char *buf, unsigned int &len, int flags=0);
	


	/*
	(1) 1:正常
	(2) 2:超时
	(3)小于0: fd接收失败返回负值错误码
	*/
	static int connect(int fd, std::string ip, unsigned short port, const unsigned int timeout=0);


	static int connect_s(int fd, std::string ip, unsigned short port, const unsigned int timeout=0);	

};

NS_BASE_END

#endif


