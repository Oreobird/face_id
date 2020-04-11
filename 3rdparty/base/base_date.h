
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
 

#ifndef _BASE_DATE_H
#define _BASE_DATE_H

#include "base_common.h"
#include <sys/time.h>


NS_BASE_BEGIN


class XDate
{
public:
	XDate(unsigned long long timestamp=0);
	XDate(const char *date, const char *format="%Y-%m-%d %H:%M:%S");
	
	unsigned int year();
	unsigned int month();
	unsigned int day();
	unsigned int hour();
	unsigned int min();
	unsigned int sec();
	unsigned int week();
	unsigned int yday();

	struct tm datails();
	std::string to_string(const char *format = NULL);

	unsigned long long timestamp();
	unsigned long long spend();
	
	//重载操作符
	XDate operator+(const XDate &obj);
	XDate operator-(const XDate &obj);
	XDate operator+(unsigned long long sec);
	XDate operator-(unsigned long long sec);	
	bool operator==(const XDate &obj);
	bool operator!=(const XDate &obj);
	bool operator<(const XDate &obj);
	bool operator>(const XDate &obj);
	bool operator<=(const XDate &obj);
	bool operator>=(const XDate &obj);

private:
	void update();
	
private:
	unsigned long long _timestamp;
	struct tm _tm;
};


NS_BASE_END

#endif


