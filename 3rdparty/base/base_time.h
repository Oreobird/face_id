
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
 

#ifndef _BASE_TIME_H
#define _BASE_TIME_H

#include "base_common.h"

NS_BASE_BEGIN


/*
把时间戳转换成本地时间字符串
format格式可以参考:%Y-%m-%d %H:%M:%S %Z
*/
std::string FormatDateTimeStr(unsigned long long timestamp = 0, const char *format = NULL);

unsigned long long getTimestamp(const std::string strDate = "", const char *format = NULL);

void sleep(unsigned long long sec, bool us_option=false);

//select的精确度为10毫秒。在10毫秒以上比较精确，误差30微秒
int sleep_select(unsigned long long usec);

int getTimeOfDay(unsigned long long &sec, unsigned long long &usec);

int getClockTime(unsigned long long &sec, unsigned long long &nsec, int clk_id = CLOCK_REALTIME);

unsigned long long get_microsecond();

unsigned long long get_millisecond();
	

NS_BASE_END


#endif


