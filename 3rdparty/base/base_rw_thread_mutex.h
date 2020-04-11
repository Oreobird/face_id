
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
 

#ifndef _BASE_RW_THREAD_MUTEX_H
#define _BASE_RW_THREAD_MUTEX_H

#include "base_common.h"
#include <pthread.h>


NS_BASE_BEGIN


/*
NPTL 线程锁系统结构体参考/usr/include/bits/pthreadtypes.h

(1) pthread_rwlock_t
typedef union
{
	  struct
	  {
		    int __lock;
		    unsigned int __nr_readers;
		    unsigned int __readers_wakeup;
		    unsigned int __writer_wakeup;
		    unsigned int __nr_readers_queued;
		    unsigned int __nr_writers_queued;
		    //FLAGS must stay at this position in the structure to maintain binary compatibility. 
		    unsigned int __flags;
		    int __writer;
	  } __data;
	  
  	char __size[__SIZEOF_PTHREAD_RWLOCK_T];
  	long int __align;
  	
} pthread_rwlock_t;


(2) pthread_rwlockattr_t
typedef union
{
	  char __size[__SIZEOF_PTHREAD_RWLOCKATTR_T];
	  long int __align;
} pthread_rwlockattr_t;

允许同时有多个读者来访问共享资源，最大可能的读者数为实际的逻辑CPU数。
写者是排他性的，一个读写锁同时只能有一个写者或多个读者(与CPU数相关)，
但不能同时既有读者又有写者。
读写锁不能递归调用
*/


class RW_Thread_Mutex : public noncopyable
{
public:

    RW_Thread_Mutex();
	
    ~RW_Thread_Mutex();
	
	int unlock();

	int lock_read();
 
	int lock_write();
	
	pthread_rwlock_t* mutex();
	
private:
 	int init();

	int destroy();
	
private:
    pthread_rwlock_t _mutex;
	
	bool _init;
	bool _destroy;
};


//---------------------------

class Thread_Read_Mutex_Guard
{
public:
	Thread_Read_Mutex_Guard(RW_Thread_Mutex &mutex);

	~Thread_Read_Mutex_Guard();

private:
	RW_Thread_Mutex &_mutex;
};


//---------------------------


class Thread_Write_Mutex_Guard
{
public:
	Thread_Write_Mutex_Guard(RW_Thread_Mutex &mutex);

	~Thread_Write_Mutex_Guard();

private:
	RW_Thread_Mutex &_mutex;
};


NS_BASE_END


#endif



