
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
 

#ifndef _BASE_NET_H
#define _BASE_NET_H

#include "base_common.h"



NS_BASE_BEGIN


	//设置socket 为非阻塞
	int set_non_bolck(int fd);

	//设置socket 为 阻塞
	int set_bolck(int fd);

	int set_nodelay(int fd);
		
	//TCP状态位于 TIME_WAIT ，可以重用端口
	int set_reuseaddr(int fd);

	int set_keepalive(int fd);

	int set_closeonexec(int fd);

	int set_rcvbuf(int fd, int size);

	int set_sndbuf(int fd, int size);

	int set_rcv_timeout(int fd, int timeout);

	int set_snd_timeout(int fd, int timeout);	

	int shutdown(int fd, int flag);

	int close_socket(int fd);

	int get_local_socket(int fd, std::string &ip, unsigned short &port);
	
	int get_remote_socket(int fd, std::string &ip, unsigned short &port);

NS_BASE_END


#endif


