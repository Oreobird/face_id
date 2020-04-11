
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
 

#ifndef _BASE_STRING_H
#define _BASE_STRING_H

#include "base_common.h"

NS_BASE_BEGIN

void trim(std::string &str, char ch=' ');

void trim_left(std::string &str);

void trim_right(std::string &str);

void upperCase(std::string &str);

void lowerCase(std::string &str);

//使用vector 可以方便直接访问具体索引下的元素
void split(const std::string str, const std::string delim, std::vector<std::string> &vecStr, int num=-1);

void split(const std::string str, const std::string delim, std::list<std::string> &lstStr, int num=-1);

void remove(std::string &strSrc, const std::string strSub);

int search_str(const std::string strSrc, const std::string strSearch);

bool is_digit(std::string &str);

bool is_xdigit(std::string &str);

std::string format(const char *format, ...);

std::string replace_str(const std::string strSrc, 
	const std::string strSearch, const std::string strReplace, unsigned int pos=0);

//返回随机字符串，随机字符串最小长度3
std::string random_str(unsigned int max=10);

void print_hex(const char *buf, int size, const std::string &text="");

NS_BASE_END
	

#endif


