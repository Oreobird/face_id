
#include "base_thread.h"
#include "base_signal.h"
#include <signal.h>
#include "base_os.h"

NS_BASE_BEGIN


void* thread_proc(void* arg)
{
    Thread* thread = (Thread*)arg;
	if(thread)
	{
		int nRet = 0;

		//线程准备工作
		nRet = thread->prepare();
		if(nRet != 0)
		{
			printf("prepare thread falied, ret:%d\n", nRet);
			printf("--- thread(thr:%u) is over ---\n", (unsigned int)Thread::thread_id());
			if(thread->dec_ref() == 0)
			{
				DELETE_POINTER(thread);
			}
			return NULL;
		}

		//线程主循环
		while(true)
		{
			//执行线程处理函数
	    	nRet = thread->svc();
			if(nRet != 0)
			{
				printf("thread svr failed, ret:%d\n", nRet);
				break;	
			}

			//判断线程是否暂停
			if(thread->is_suspend())
			{
				nRet = thread->do_suspend();
				if(nRet != 0)
				{
					printf("suspend failed, ret:%d\n", nRet);
					break;
				}
				else
				{
					printf("the thread(thr:%u) is resume.\n", (unsigned int)Thread::thread_id());
				}
			}

			//判断线程是否停止
			if(thread->is_stop())
			{
				printf("thread(thr:%u) is stop.\n", (unsigned int)Thread::thread_id());
				break;			
			}
			
		}

		//线程回收工作
		thread->cleanup();
		printf("--- thread(thr:%u) is over ---\n", (unsigned int)Thread::thread_id());
		
		if(thread->dec_ref() == 0)
		{
			DELETE_POINTER(thread);
		}
		else
		{
	//线程组中一个线程退出就设置该线程组准备退出
			printf("---- ST_EXIT ----\n");
			thread->thr_status(Thread::ST_EXIT);
		}
		
	}
	
    return NULL;
	
}



Thread::Thread(): _thr_cnt(1), _status(ST_EXIT), _suspend(false), _stop(false), \
	_detach(true), _stack_size(0)
{
	_threads.clear();
	//inc_ref();
}


Thread::~Thread()
{
	printf("destroy thread!\n");
}



int Thread::do_init(void *args)
{
	return 0;
}



int Thread::init(void *args, unsigned int thr_cnt, bool detach, unsigned int stack_size)
{
	int nRet = 0;

	if(_status == ST_INIT)
	{
		printf("the thread is init ago.\n");
		return 0;		
	}

	_stop = false;
	_suspend = false;
	_detach = detach;
	_stack_size = stack_size;
	_thr_cnt = thr_cnt;
	
    nRet = pthread_attr_init(&_attr);
    if (nRet != 0)
    {
        printf("pthread_attr_init failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
		return nRet;
    }

    // 设置线程栈大小
    if (stack_size > 0)
    {
		nRet = pthread_attr_setstacksize(&_attr, stack_size);
		if (nRet != 0)
	    {
	        printf("pthread_attr_setstacksize failed. ret:%d, errno:%d, errmsg:%s\n", 
					nRet, errno, strerror(errno));
			return nRet;
	    }
			
	}

	//阻塞进程信号SIGUSR1
	nRet = Thread::signal_mask(SIG_BLOCK, 1, SIGUSR1);
	if (nRet != 0)
	{
		printf("signal_mask failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
		return nRet;
	}

	//执行业务自己的初始化方法
	nRet = do_init(args);
	if (nRet != 0)
	{
		printf("do_init failed. ret:%d\n", nRet);
		return nRet;
	}
	
	_status = ST_INIT;
	
	return nRet;
	
}




int Thread::run(void **ret)
{
	int nRet = 0;

	if(_status != ST_INIT)
	{
		printf("the status of thread is invalid. status:%d\n", _status);
		return -2;		
	}

	 pthread_t thread = 0;
	for(unsigned int i=0; i<_thr_cnt; i++)
	{
		nRet = pthread_create(&thread , &_attr, thread_proc, this);
		if(nRet != 0)
		{
			printf("pthread_create failed. ret:%d, errno:%d, errmsg:%s\n", 
					nRet, errno, strerror(errno));
		}
		else
		{
			printf("--- pthread_create(thr:%u) success ---\n", (unsigned int)thread);
			inc_ref();
		}

		//统一管理所有线程id
		_threads.push_back(thread);
		
		_status = ST_RUN;
		
		if(_detach)
		{
			nRet = detach(thread);
		}

	}
	
	/*
	因为线程处理函数都是相同的，

	没有必要同步等待所有线程都退出， 这里只等待最后一个创建的线程
	*/
	if(!_detach)
	{
		//同步等待指定线程执行完成， 并且获取线程结束的返回码
		nRet = join(thread, ret);
	}
		
	return nRet;
	
}




/*
线程终止，mutex、condition、semaphores、file descriptors 不会释放
注册在atexit的进程退出函数是不会被调用，那些都是
在进程退出的时候才会执行处理
*/
int Thread::exit(void *ret)
{
	int nRet = 0;

	pthread_exit(ret);

	_status = ST_EXIT;

	return nRet;
	
}



int Thread::cancel(pthread_t thread)
{
	int nRet = 0;

	if(thread == 0)
	{
		thread = thread_id();
	}
	
	nRet = pthread_cancel(thread);
	if (nRet != 0)
	{
		printf("pthread_cancel(thr:%u)failed. ret:%d, errno:%d, errmsg:%s\n", 
				(unsigned int)thread, nRet, errno, strerror(errno));
	}

	_status = ST_CANCEL;
	
	return nRet;

}



pthread_t Thread::thread_id()
{
	return pthread_self();
}



int Thread::signal(pthread_t thread, int sig)
{
	int nRet = 0;

	nRet = pthread_kill(thread, sig);
	if (nRet != 0)
	{
		printf("pthread_kill(thr:%u) failed. ret:%d, errno:%d, errmsg:%s\n", 
				(unsigned int)thread, nRet, errno, strerror(errno));
	}
	
	return nRet;
	
}



bool Thread::is_exist(pthread_t thread)
{
	int bRet = true;

	int nRet = pthread_kill(thread, 0);
	if (nRet != 0)
	{
		printf("pthread_kill(thr:%u) failed. ret:%d, errno:%d, errmsg:%s\n", 
				(unsigned int)thread, nRet, errno, strerror(errno));
		bRet = false;
	}
	
	return bRet;

}





int Thread::signal_mask(int how, unsigned int num, ...)
{
	int nRet = 0;

	va_list args;
   	va_start(args, num);

	//初始化信号集
	sigset_t sigset;
	sigemptyset(&sigset);
	
	int sig = 0;
	for(unsigned int i=0; i<num; i++)
	{
		sig = va_arg(args, int); 
		nRet = sigaddset(&sigset, sig);
		if(nRet != 0)
		{
			printf("sigaddset failed. signo(%i):%d, errno:%d, errmsg:%s\n", 
				i, sig, errno, strerror(errno));
		}
	}
	va_end(args);
	
	nRet = pthread_sigmask(how, &sigset, NULL);
	if (nRet != 0)
	{
		printf("pthread_sigmask failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}
	
	return nRet;
	
}



void Thread::testcancel()
{
	//显式方式设置一个线程取消点
	pthread_testcancel();
}



int Thread::set_cancel_mode(int cancel_state, int cancel_type)
{
	int nRet = 0;

	int oldstate = 0;
	nRet = pthread_setcancelstate(cancel_state, &oldstate);
	if (nRet != 0)
	{
		printf("pthread_setcancelstate failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}


	nRet = pthread_setcanceltype(cancel_type, &oldstate);
	if (nRet != 0)
	{
		printf("pthread_setcanceltype failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}

	return nRet;
	
}




void Thread::suspend()
{
	_suspend = true;
}



int Thread::resume()
{
	int nRet = 0;

	//发信号给所有的线程唤醒它们
	std::vector<pthread_t>::iterator itr = _threads.begin();
	for(; itr != _threads.end(); itr++)
	{
		nRet = signal(*itr);
		if (nRet != 0)
		{
			printf("signal(thr:%u) failed. ret:%d, errno:%d, errmsg:%s\n", 
					(unsigned int)*itr, nRet, errno, strerror(errno));
		}
		else
		{
			_suspend = false;
		}

	}
	
	return nRet;
	
}



bool Thread::is_suspend()
{
	return _suspend;
}




int Thread::do_suspend()
{
	int nRet = 0;

	sigset_t sigset;
	nRet = add_signal_in_set(sigset, 1, SIGUSR1);
	if(nRet != 0)
	{
		printf("add_signal_in_set failed. ret:%d\n", nRet);
		return nRet;
	}

	int signo = 0;
	nRet = wait_singal(sigset, signo);
	if(nRet != 0)
	{
		printf("wait_singal failed. ret:%d\n", nRet);
	}

	return nRet;
	
}



void Thread::stop()
{
	_stop = true;
}



bool Thread::is_stop()
{
	return _stop;
}




int Thread::join(pthread_t thread, void **ret)
{
	int nRet = 0;

    nRet = pthread_join(thread, ret);
	if (nRet != 0)
	{
		printf("pthread_join(thr:%u) failed. ret:%d, errno:%d, errmsg:%s\n", 
				(unsigned int)thread, nRet, errno, strerror(errno));
	}

	return nRet;
	
}




int Thread::detach(pthread_t thread)
{
	int nRet = 0;
	
    nRet = pthread_detach(thread);
    if (nRet != 0)
    {
		printf("pthread_detach(thr:%u) failed. ret:%d, errno:%d, errmsg:%s\n", 
				(unsigned int)thread, nRet, errno, strerror(errno));
	}

	return nRet;
}


int Thread::thr_count()
{
	return _threads.size();
}



void Thread::thr_status(int status)
{
	_status =  status;
}

int Thread::thr_status()
{
	return _status;
}

NS_BASE_END

