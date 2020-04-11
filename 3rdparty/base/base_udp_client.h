
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
 * Author: xucuiping(89617663@qq.com)
 */

 

#ifndef _BASE_UDP_CLIENT_H
#define _BASE_UDP_CLIENT_H

#include "base_common.h"

NS_BASE_BEGIN


class UDP_Client
{
public:
	UDP_Client();

	virtual ~UDP_Client();

	int connect(const std::string &ip, unsigned short port, bool doconnect=false);

	int sendto(const char *buf, const unsigned int len, int flags=0);

	int recvfrom(char *buf, const unsigned int len);

	void close();

	bool is_close();

	int fd();

private:
	int _fd;
	bool _connect;
	bool _close;
	std::string _ip;
	unsigned short _port;
};

NS_BASE_END

#endif


