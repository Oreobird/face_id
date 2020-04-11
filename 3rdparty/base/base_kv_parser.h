
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

 

#ifndef _BASE_KV_PARSER_H
#define _BASE_KV_PARSER_H

#include "base_common.h"

NS_BASE_BEGIN


class KV_Parser
{
public:
	
	KV_Parser();

	~KV_Parser();
	
	//将转义后的字符串装换成原始字符串
	void msg(const std::string &str);

	//将原始字符串转换成转义后的字符串
	std::string new_msg(std::string tail="");

	void clear();

	//获取到的是非转义的value；设置的是非转义的value
	std::string& operator [](const std::string &key);

	void show();
	
private:
	std::map<std::string, std::string> _mapKV;
	
};





NS_BASE_END

#endif


