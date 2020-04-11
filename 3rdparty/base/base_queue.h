
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

 

#ifndef _BASE_QUEUE_H
#define _BASE_QUEUE_H

#include "base_common.h"
#include "base_thread_mutex.h"
#include "base_condition.h"

NS_BASE_BEGIN

template <typename T>
class X_Queue
{
public:
	X_Queue(const unsigned int max_size=0, const unsigned int rate_limiting=0);

	~X_Queue();

	unsigned int size();

	bool empty();

	bool full();

	bool full(unsigned int &used_size, unsigned int &max_size);

	int pop(T &item);

	int pop(T &item, const unsigned int timeout);

	int push(T &item);

private:
	std::deque<T> _lst_item;
	Thread_Mutex _mutex;
	Condition _cond;
	unsigned int _max_size;  //队列最大限制
	unsigned long long _pre_time;  //之前的时间戳
	unsigned int _rate_limiting;  //限速值，单位: 元素个数/每秒
	unsigned int _try_pop_cnt;  //尝试pop元素总数
};


NS_BASE_END

#include "base_queue.inl"

#endif


