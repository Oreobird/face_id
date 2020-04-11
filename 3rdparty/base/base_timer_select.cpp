
#include "base_timer_select.h"


NS_BASE_BEGIN

Select_Timer_handler::Select_Timer_handler()
{

}

Select_Timer_handler::~Select_Timer_handler()
{


}

int Select_Timer_handler::handle_timeout(void *args)
{
	int nRet = 0;

	return nRet;
}



//-----------------------------------------

Select_Timer::Select_Timer(): _handler(NULL)
{


}



Select_Timer::~Select_Timer()
{
	DELETE_POINTER(_handler);
}



int Select_Timer::register_timer_handler(Select_Timer_handler *handler, unsigned long interval)
{
	int nRet = 0;

	if(handler == NULL)
	{
		printf("handler == NULL\n");
		return -1;
	}

	if(_handler != NULL)
	{
		printf("already register timer handler ago.\n");
		return -2;
	}

	if(interval == 0)
	{
		printf("interval == 0.\n");
		return -3;	
	}

	_sec = interval/1000000;
    _usec = interval%1000000;
	_handler = handler;
	
	return nRet;

}


int Select_Timer::svc()
{
	int nRet = 0;

	timeval timeout;
	timeout.tv_sec = _sec;
	timeout.tv_usec = _usec;
	nRet = select(1, NULL, NULL, NULL, &timeout);
	if(nRet > 0)
	{		
	}
	else if(nRet == 0)
	{
		//printf("select timeout, errno:%d, errmsg:%s\n", errno, strerror(errno));
		_handler->handle_timeout();
	}
	else
	{
		if(errno == EINTR)
		{
			printf("select is interrupt, errno:%d, errmsg:%s\n", errno, strerror(errno));
			return 0;
		}
		else if(errno == EINVAL)
		{
			printf("nfds is negative or the value contained within timeout is invalid.\n");
			return -1;
		}
		else
		{
			printf("select failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
			return -2;
		}

	}
			
	return 0;
	
}


NS_BASE_END


