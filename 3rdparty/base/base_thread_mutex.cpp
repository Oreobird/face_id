
#include "base_thread_mutex.h"


NS_BASE_BEGIN


Thread_Mutex::Thread_Mutex(bool recursive): _recursive(recursive), _init(false), _destroy(false)
{
	init();
}



Thread_Mutex::~Thread_Mutex()
{
	destroy();
}



int Thread_Mutex::init()
{
    int nRet = 0;

	if(_init)
	{
		printf("init pthread mutex success already. \n");	
		return 0;
	}
	
    if (_recursive)
    {
        nRet = pthread_mutexattr_init(&_attr);
        if (nRet != 0) 
		{
			printf("pthread_mutexattr_init failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));	
			return nRet;
        }

        nRet = pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
        if (nRet != 0) 
		{
			printf("pthread_mutexattr_settype failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
			pthread_mutexattr_destroy(&_attr);
			
			return nRet;
        }
    	nRet = pthread_mutex_init(&_mutex, &_attr);
		
    }
	else
	{
		//尝试初始化一个已经初始化过的mutex 导致不明确的行为
		nRet = pthread_mutex_init(&_mutex, NULL);
	}

	if(nRet != 0)
	{
		printf("pthread_mutex_init failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	
	}
	else
	{
		_init = true;
	}

	pthread_mutexattr_destroy(&_attr);

	return nRet;
	
}



/*
如果是非递归锁，同一个线程2次lock， 线程就会死锁
*/
int Thread_Mutex::lock()
{
    int nRet = pthread_mutex_lock(&_mutex);
    if (nRet != 0)
    {
    	printf("pthread_mutex_lock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	
    }
	else
	{
		//printf("pthread_mutex_lock success.\n");
	}

	return nRet;
	
}



int Thread_Mutex::unlock()
{
    int nRet = pthread_mutex_unlock(&_mutex);
    if (nRet != 0)
    {
    	printf("pthread_mutex_unlock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	

    }
	else
	{
		//printf("pthread_mutex_unlock success.\n");
	}

	return nRet;
}



int Thread_Mutex::try_lock()
{
    int nRet = pthread_mutex_trylock(&_mutex);
    if (nRet == 0) 
	{
		//printf("pthread_mutex_trylock success.\n");	
    }
	else if (EBUSY == nRet) 
	{
		printf("the pthread mutex is locked ago. can't lock again.\n");
		return 0;
	}
	else
	{
		printf("pthread_mutex_trylock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));
	}

	return nRet;
	
}




int Thread_Mutex::timed_lock(int millisecond)
{
	int nRet = 0;


	struct timespec ts;
	nRet = clock_gettime(CLOCK_REALTIME, &ts);
	if(nRet != 0)
	{
		printf("clock_gettime failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
		
		return nRet;
	}

	ts.tv_sec += millisecond / 1000;
	ts.tv_nsec += (millisecond % 1000) * 1000000;

	//时间是绝对时间戳
	nRet = pthread_mutex_timedlock(&_mutex, &ts);
	if(nRet == 0)
	{
		//printf("pthread_mutex_timedlock success.\n");
	}
	else if(nRet == ETIMEDOUT)
	{
		printf("pthread_mutex_timedlock timeout!\n");
	}
	else
	{
		printf("pthread_mutex_timedlock failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
	}

	
	return nRet;
	
}





int Thread_Mutex::destroy()
{
	int nRet = 0;
	
	if(!_destroy)
	{
		//destroy 一个已经unlock 的mutex是安全的， 尝试destroy 一个locked  的mutex 返回EBUSY
    	nRet = pthread_mutex_destroy(&_mutex);
		if(nRet == 0)
		{
			//printf("pthread_mutex_destroy success.\n");
			_destroy = true;
		}
		else if(nRet == EBUSY)
		{
			printf("the pthread mutex is locked now, can't destroy.\n");
		}
		else
		{
			printf("pthread_mutex_destroy failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
		}
	
	}

	return nRet;
	
}



pthread_mutex_t* Thread_Mutex::mutex()
{
	return &_mutex;
}



Thread_Mutex_Guard::Thread_Mutex_Guard(Thread_Mutex &mutex): _mutex(mutex)
{
	_mutex.lock();
}


Thread_Mutex_Guard::~Thread_Mutex_Guard()
{
	_mutex.unlock();
}


NS_BASE_END
	

