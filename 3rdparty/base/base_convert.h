

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
 

#ifndef _BASE_CONVERT_H
#define _BASE_CONVERT_H

#include "base_common.h"

NS_BASE_BEGIN


//hex由外部分配， size = strlen(str)/2
//将16进制字符串转换成2进制串
int hexstring_to_bin(const char* str, char* hex);

//将char字符0xAB 转换成"AB"
std::string char_to_hexstring(char c);


//将2进制装换16进制字符串
std::string bin_to_hexstring(const char *src, unsigned int len);


/*
htonl/ntohl
htons/ntons
*/
unsigned long long htonll(unsigned long long ll);

unsigned long long ntohll(unsigned long long ll);	


template<typename T>
T strTo(const std::string& str);


template<typename T>
std::string toStr(const T& t);

NS_BASE_END


#include "base_convert.inl"


#endif


