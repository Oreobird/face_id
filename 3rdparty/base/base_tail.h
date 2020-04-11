
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
 

#ifndef _BASE_TAIL_H
#define _BASE_TAIL_H

#include "base_common.h"
#include "base_thread.h"

NS_BASE_BEGIN

class Tail_Handler
{
public:
	Tail_Handler();

	virtual ~Tail_Handler();

	/*
	成功读取文件后会回调该函数
	如果返回值为非0， 表示不需要继续读取
	*/
	virtual int process(const std::string &line);

	//文件读到末端后会调用
	virtual void complete();

};


/*
x-tail 支持
*/
class X_Tail : public Thread
{
public:
	/*
	ending 表示文件读到末端的字符串， 需要业务自定义
	truncate_mode=true: 表示文件使用truncate 方式截断，文件句柄都没有改变，需要从文件末尾开始读取文件。
	truncate_mode=false: 表示文件使用close -> mv -> create new file  方式，文件句柄发生改变需要从新文件头开始读文件。
	*/
	X_Tail(const std::string &file, const std::string &ending, Tail_Handler *handler, bool truncate_mode=true);

	~X_Tail();
	
private:
	virtual int do_init(void *args);
		
	virtual int svc();

private:
	std::string _file;
	std::string _ending; //文件结束标示字符串
	Tail_Handler *_handler;
	unsigned long long _pre_size;
	std::fstream _fin;
	bool _truncate_mode;
	bool _truncated;
	
};


NS_BASE_END

#endif


