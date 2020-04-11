
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
 

#ifndef _BASE_THREAD_H
#define _BASE_THREAD_H

#include "base_common.h"
#include <pthread.h>
#include <signal.h>

#include "base_thread_mutex.h"
#include "base_condition.h"
#include "base_smart_ptr_t.h"

NS_BASE_BEGIN



/*
【注意】所有集成Thread 的子类， 需要使用new 创建子类对象， 应用
不能显示delete 它， 由Thread 自己负责释放
*/
class Thread : public noncopyable, public RefCounter
{
public:
	enum StStatus
	{
		ST_INIT,
		ST_RUN,
		ST_SUSPEND,
		ST_EXIT,
		ST_CANCEL
	};

	virtual int prepare(){return 0;};

	//业务自己的初始化
	virtual int do_init(void *args);
		
	/*
	子类只需要实现这个就可以， 只要svc 返回0，该接口会被一直执行
	如果svc返回非0， 线程执行就结束
	*/
	virtual int svc() = 0;
	//virtual int svc(){return 0;};

	virtual void cleanup(){pthread_attr_destroy(&_attr);};
	
public:
	Thread();
	
	virtual ~Thread();

	/*
	args 外部应用传入参数
	
	*/
	int init(void *args=NULL, unsigned int thr_cnt=1, bool detach=true, unsigned int stack_size=0);

	/*
	ret的设置方法参考如下:
	int nRet = 0;
	run(&nRet);
	*/
	int run(void **ret=NULL);

    int exit(void *ret=NULL);
	
	static pthread_t thread_id();
	

	//线程信号相关
	int signal(pthread_t thread=0, int sig=SIGUSR1);

	static bool is_exist(pthread_t thread);

	static int signal_mask(int how, unsigned int num, ...);


	//线程取消相关(推荐使用下面suspend 接口)	
	int cancel(pthread_t thread=0);  

	static void testcancel();   //使用api 显示设置一个线程取消点
	
	int set_cancel_mode(int cancel_state=PTHREAD_CANCEL_ENABLE, int cancel_type=PTHREAD_CANCEL_ENABLE);
	

	//线程暂停恢复
	void suspend();

	int resume();

	bool is_suspend();

	int do_suspend();


	//线程关闭
	void stop();

	bool is_stop();

	//线程数
	int thr_count();

	void thr_status(int status);

	int thr_status();
	
private:
	int join(pthread_t thread, void **ret);

	int detach(pthread_t thread);

private:
    pthread_attr_t _attr;
	unsigned int _thr_cnt;

	int _status;
	int _suspend;
	int _stop;
	
	bool _detach;
	unsigned int _stack_size;

	std::vector<pthread_t> _threads;
};


NS_BASE_END

#endif

