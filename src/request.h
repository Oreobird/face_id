
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



#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "base_common.h"
#include "base_smart_ptr_t.h"


USING_NS_BASE;



class Request : public RefCounter
{
public:
	Request();

	~Request();

	void log();

	std::string to_string();

public:
	std::string _msg_tag;  //内部设置(唯一id)
	unsigned int _req_id;  //前端设置
	std::string _req;      //追加了_msg_id的请求串
	std::string _msg_type;  //消息类型

	int _fd;               //前端fd
	std::string _ip;       //前端ip
	unsigned short _port;  //前端port

	unsigned long long _app_stmp;   	//客户端填写的时间戳
	unsigned long long _rcv_stmp;       //消息创建时间戳
	unsigned long long _process_stmp;   //外部应用填写的时间戳

	std::string _session_id;

	std::string _uuid;    //安全通道ID
	std::string _key;     //安全通道秘钥
	std::string _encry;   //是否要加密解密

};
typedef Smart_Ptr_T<Request>  Request_Ptr;


#endif


