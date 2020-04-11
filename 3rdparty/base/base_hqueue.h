
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

 

#ifndef _BASE_HQUEUE_H
#define _BASE_HQUEUE_H

#include "base_common.h"
#include "base_thread_mutex.h"

NS_BASE_BEGIN


template <typename T>
class H_Queue
{
public:
	H_Queue();

	~H_Queue();

	unsigned int size();

	bool empty();

	bool get(const int key, T &value);

	void insert(const int key, T &value);

	void del(const int key);

	void clear();

private:
	std::map<int, T > _items;
	Thread_Mutex _mutex;
	
};


NS_BASE_END

#include "base_hqueue.inl"

#endif


