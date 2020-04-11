
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


#ifndef  _BASE_ARGS_PARSER_H
#define  _BASE_ARGS_PARSER_H

#include "base_common.h"


NS_BASE_BEGIN


class Args_Parser
{
public:
	Args_Parser();
	~Args_Parser();

	//解析命令行参数格式 "-option1 value -option2 value"
	void parse_args(int argc, char **args);

	//解析参数串 参数串格式 "-option1 value -option2 value"
	void parse_args(std::string &args);

	std::string get_process_name();

	template <typename T>
	bool get_opt(const std::string &key, T* value=NULL, T* default_value=NULL);

	void show();

private:
	std::map<std::string, std::string> _mapArgs;
	std::string _process_name;
	
};


NS_BASE_END

#include "base_args_parser.inl"

#endif

