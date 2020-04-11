
#include "base_rw_thread_mutex.h"


NS_BASE_BEGIN


RW_Thread_Mutex::RW_Thread_Mutex():_init(false), _destroy(false)
{
	init();
}



RW_Thread_Mutex::~RW_Thread_Mutex()
{
	destroy();
}



int RW_Thread_Mutex::init()
{
    int nRet = 0;

	if(_init)
	{
		printf("init pthread rw mutex success already. \n");	
		return 0;
	}
	
	/*
	有两种方法创建互斥锁，静态方式和动态方式。
	POSIX定义了一个宏PTHREAD_MUTEX_INITIALIZER来静态初始化互斥锁，具体参考
	/usr/include/pthread.h
	# define PTHREAD_MUTEX_INITIALIZER \  { { 0, 0, 0, 0, 0, { 0 } } }
	
	例如: pthread_rwlock_t _mutex = PTHREAD_RWLOCK_INITIALIZER;
	Thread_Mutex 使用动态方式， 具体见下

	*/
	nRet = pthread_rwlock_init(&_mutex, NULL);
	if(nRet != 0)
	{
		printf("pthread_rwlock_init failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	
	}
	else
	{
		_init = true;
	}

	return nRet;
	
}



int RW_Thread_Mutex::unlock()
{
    int nRet = pthread_rwlock_unlock(&_mutex);
    if (nRet != 0)
    {
    	printf("pthread_rwlock_unlock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	

    }
	else
	{
		//printf("pthread_rwlock_unlock success.\n");
	}

	return nRet;
}



int RW_Thread_Mutex::lock_read()
{
    int nRet = pthread_rwlock_rdlock(&_mutex);
    if (nRet != 0)
    {
    	printf("pthread_rwlock_rdlock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	
    }
	else
	{
		//printf("pthread_rwlock_rdlock success.\n");
	}

	return nRet;

}

int RW_Thread_Mutex::lock_write()
{
    int nRet = pthread_rwlock_wrlock(&_mutex);
    if (nRet != 0)
    {
    	printf("pthread_rwlock_wrlock failed. ret:%d, errno:%d, errmsg:%s\n", 
			nRet, errno, strerror(errno));	
    }
	else
	{
		//printf("pthread_rwlock_wrlock success.\n");
	}

	return nRet;

}


int RW_Thread_Mutex::destroy()
{
	int nRet = 0;
	
	if(!_destroy)
	{
		//destroy 一个已经unlock 的mutex是安全的， 尝试destroy 一个locked  的mutex 返回EBUSY
    	nRet = pthread_rwlock_destroy(&_mutex);
		if(nRet == 0)
		{
			printf("pthread_rwlock_destroy success.\n");
			_destroy = true;
		}
		else if(nRet == EBUSY)
		{
			printf("the pthread rw mutex is locked now, can't destroy.\n");
		}
		else
		{
			printf("pthread_rwlock_destroy failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
		}
	
	}

	return nRet;
	
}



pthread_rwlock_t* RW_Thread_Mutex::mutex()
{
	return &_mutex;
}


//---------------------------------


Thread_Read_Mutex_Guard::Thread_Read_Mutex_Guard(RW_Thread_Mutex &mutex): _mutex(mutex)
{
	_mutex.lock_read();
}


Thread_Read_Mutex_Guard::~Thread_Read_Mutex_Guard()
{
	_mutex.unlock();
}


//---------------------------------


Thread_Write_Mutex_Guard::Thread_Write_Mutex_Guard(RW_Thread_Mutex &mutex): _mutex(mutex)
{
	_mutex.lock_write();
}


Thread_Write_Mutex_Guard::~Thread_Write_Mutex_Guard()
{
	_mutex.unlock();
}


NS_BASE_END

