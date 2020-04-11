
#include "base_condition.h"


NS_BASE_BEGIN


Condition::Condition(Thread_Mutex &mutex): _mutex(mutex), _init(false), _destroy(false)
{
	init();
}



Condition::~Condition()
{
	destroy();
}



int Condition::wait()
{
	int nRet = 0;

	nRet = pthread_cond_wait(&_cond, _mutex.mutex());
	if(nRet != 0)
	{
		printf("pthread_cond_wait failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}

	return nRet;

}


int Condition::timed_wait(unsigned int millisecond)
{
	int nRet = 0;

	if(millisecond == 0)
	{
		nRet = wait();
		return nRet;
	}
	else
	{
		/*
		 struct timespec 
		 {
			  time_t   tv_sec;		  //seconds
			  long      tv_nsec; 	         //nanoseconds
		 };
		*/
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

		//如果计算出来的纳秒数都超过1秒就进行进位处理
		if(ts.tv_nsec >= 1000000000)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}
		
		nRet = pthread_cond_timedwait(&_cond, _mutex.mutex(), &ts);
		if(nRet == 0)
		{
			//printf("pthread_cond_timedwait success.\n");
		}
		else if(nRet == ETIMEDOUT)
		{
			//printf("the condition is timeout!\n");
		}
		else
		{
			printf("pthread_cond_timedwait failed. ret:%d, errno:%d, errmsg:%s\n", 
					nRet, errno, strerror(errno));
		}
		
	}

	return nRet;

}



int Condition::signal()
{
	int nRet = 0;

	nRet = pthread_cond_signal(&_cond);
	if(nRet != 0)
	{
		printf("pthread_cond_signal failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}
	
	return nRet;

}


int Condition::broadcast()
{
	int nRet = 0;

	nRet = pthread_cond_broadcast(&_cond);
	if(nRet != 0)
	{
		printf("pthread_cond_broadcast failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}
	
	return nRet;
}


int Condition::init()
{
	int nRet = 0;

	if(_init)
	{
		printf("init condition success already.\n");
		return 0;
	}

	nRet = pthread_cond_init(&_cond, NULL);
	if(nRet != 0)
	{
		printf("pthread_cond_init failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
	}
	else
	{
		_init = true;
	}
	
	return nRet;
	
}



int Condition::destroy()
{
	int nRet = 0;

	if(!_destroy)
	{
		nRet = pthread_cond_destroy(&_cond);
		if(nRet == 0)
		{
			printf("pthread_cond_destroy success.\n");
			_destroy = true;
		}
		else if(nRet == EBUSY)
		{
			printf("the condition is waited now, can't destroy.\n");
		}
		else
		{
			printf("pthread_cond_destroy failed. ret:%d, errno:%d, errmsg:%s\n", 
				nRet, errno, strerror(errno));
		}
	
	}
	
	return nRet;

}


NS_BASE_END

