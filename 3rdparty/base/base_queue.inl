
#include "base_queue.h"
#include "base_time.h"

NS_BASE_BEGIN


template <typename T>
X_Queue<T>::X_Queue(unsigned int max_size, unsigned int rate_limiting): _cond(_mutex), 
_max_size(max_size), _pre_time(0), _rate_limiting(rate_limiting), _try_pop_cnt(0)
{
	_pre_time = time(NULL);
}



template <typename T>
X_Queue<T>::~X_Queue()
{

}


template <typename T>
unsigned int X_Queue<T>::size()
{
	Thread_Mutex_Guard guard(_mutex);
	return _lst_item.size();
}


template <typename T>
bool X_Queue<T>::empty()
{
	Thread_Mutex_Guard guard(_mutex);
	return _lst_item.empty();
}



template <typename T>
bool X_Queue<T>::full()
{
	Thread_Mutex_Guard guard(_mutex);
	if((_lst_item.size()) >= _max_size && (_max_size > 0))
	{
		return true;
	}
	
	return false;
}


template <typename T>
bool X_Queue<T>::full(unsigned int &used_size, unsigned int &max_size)
{
	Thread_Mutex_Guard guard(_mutex);
	used_size = _lst_item.size();
	max_size = _max_size;
	if(used_size >= _max_size && (_max_size > 0))
	{
		return true;
	}
	
	return false;
}


template <typename T>
int X_Queue<T>::pop(T &item)
{
	int nRet = 0;
	
	Thread_Mutex_Guard guard(_mutex);
	
	if(_lst_item.empty())
	{
		nRet = _cond.wait();
		if(nRet != 0)
		{
			printf("cond wait failed.\n");
			return nRet;
		}
	}

	if(!_lst_item.empty())
	{
		if(_rate_limiting > 0)
		{
			unsigned long long _cur_time = time(NULL);
			if((_cur_time-_pre_time) < 1)
			{
				++_try_pop_cnt;
				if(_try_pop_cnt > _rate_limiting)
				{
					sleep(10000, true); //停顿10毫秒
					return -1;
				}
				
			}
			else
			{
				_try_pop_cnt = 1;
				_pre_time = _cur_time;
			}
		}
		
		item = _lst_item.front();
		_lst_item.pop_front();
		
	}
	else
	{
		nRet = -1;
		//printf("no item is pop.\n");
	}

	return nRet;
	
}




template <typename T>
int X_Queue<T>::pop(T &item, unsigned int timeout)
{
	int nRet = 0;

	Thread_Mutex_Guard guard(_mutex);
	
	if(_lst_item.empty())
	{
		nRet = _cond.timed_wait(timeout);
		if(nRet == ETIMEDOUT)
		{
			//printf("cond timed_wait timeout!\n");
			return nRet;
		}
		
		if(nRet != 0)
		{
			//printf("cond timed_wait failed.\n");
			return nRet;
		}
	}

	if(!_lst_item.empty())
	{
		if(_rate_limiting > 0)
		{
			unsigned long long _cur_time = time(NULL);
			if((_cur_time-_pre_time) < 1)
			{
				++_try_pop_cnt;
				if(_try_pop_cnt > _rate_limiting)
				{
					sleep(10000, true); //停顿10毫秒
					return -1;
				}
			}
			else
			{
				//超过1秒钟
				_try_pop_cnt = 1;
				_pre_time = _cur_time;
			}
		}
		
		item = _lst_item.front();
		_lst_item.pop_front();
		
	}
	else
	{
		nRet = -1;
		//printf("no item is pop.\n");
	}

	return nRet;

}



template <typename T>
int X_Queue<T>::push(T &item)
{
	int nRet = 0;
	
	Thread_Mutex_Guard guard(_mutex);
	if((_lst_item.size()) >= _max_size && (_max_size > 0))
	{
		printf("reach max limit.\n");
		return -1;
	}
	
	_lst_item.push_back(item);
	//唤醒所有等待的线程进行处理
	_cond.broadcast();

	return nRet;
}


NS_BASE_END


