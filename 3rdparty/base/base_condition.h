

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

 
#ifndef _BASE_CONDITION_H
#define _BASE_CONDITION_H
 
#include "base_common.h"
#include "base_thread_mutex.h"

#include <pthread.h>

NS_BASE_BEGIN


class Condition : public noncopyable
{
public:

    Condition(Thread_Mutex &mutex);
	
    ~Condition();

    int wait();

	//millisecond 是相对时间，内部会将其转化成绝对时间(单位: 毫秒)
    int timed_wait(unsigned int millisecond);

    int signal();

    int broadcast();

private:
	int init();
	int destroy();
	
private:
    pthread_cond_t _cond;
	Thread_Mutex &_mutex;

	bool _init;
	bool _destroy;
	
};


NS_BASE_END

#endif

