
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
 

#ifndef _BASE_COMMON_H
#define _BASE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <sstream>

//#include "google/tcmalloc.h"

#define NS_BASE_BEGIN		namespace base {
#define NS_BASE_END 			}
#define USING_NS_BASE		using namespace base;

NS_BASE_BEGIN


//---------------  公共宏---------------
#define DELETE_POINTER(x) \
if (NULL != x)\
{\
	delete x;\
	x = NULL;\
};


#define DELETE_POINTER_ARR(x) \
if (NULL != x)\
{\
	delete []x;\
	x = NULL;\
};


#define MAX_LOG_SIZE 1024	 //单位: M byte
#define MIN_LOG_SIZE 500 	 //单位: M byte
#define LOG_BUF_SECS 3		//日志缓存超时时间， 单位: 秒
#define MAX_LOG_LEN 8092	//每条日志最大尺寸， 单位: 字节

#define LF     (unsigned char) 10
#define CR     (unsigned char) 13
#define CRLF   "\x0d\x0a"


//--------------- 公共常量---------------


//--------------- 公共结构体---------------


//---------------  公共方法---------------


//---------------  公共类---------------
class noncopyable
{
protected:
	noncopyable() {};
	~noncopyable() {};

private:
	noncopyable(const noncopyable&) {};
	const noncopyable& operator=(const noncopyable&) {return *this;};
};



//--------------- 类型定义---------------
typedef unsigned long long  _u64;
typedef unsigned int  _u32;
typedef long long  _i64;
typedef int _i32;
typedef unsigned short  _us;
typedef unsigned char  _uc;


NS_BASE_END


#endif


