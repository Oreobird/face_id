
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

 

#ifndef _BASE_MQUEUE_H
#define _BASE_MQUEUE_H

#include "base_common.h"
#include "base_thread_mutex.h"
#include "base_condition.h"

NS_BASE_BEGIN

template <typename T>
class M_Queue
{
public:
	M_Queue();

	~M_Queue();

	unsigned int size();

	bool empty();

	int pop(const std::string &key, T &item);

	int pop(const std::string &key, T &item, const long long timeout);

	int pop_front(T &item);

	int push(const std::string &key, T &item);

private:
	std::map<std::string, T > _items;
	Thread_Mutex _mutex;
	Condition _cond;
};


NS_BASE_END

#include "base_mqueue.inl"

#endif


