
#include "base_mqueue.h"
#include "base_time.h"

NS_BASE_BEGIN


template <typename T>
M_Queue<T>::M_Queue(): _cond(_mutex)
{
}



template <typename T>
M_Queue<T>::~M_Queue()
{

}


template <typename T>
unsigned int M_Queue<T>::size()
{
	Thread_Mutex_Guard guard(_mutex);
	return _items.size();
}


template <typename T>
bool M_Queue<T>::empty()
{
	Thread_Mutex_Guard guard(_mutex);
	return _items.empty();
}




template <typename T>
int M_Queue<T>::pop(const std::string &key, T &item)
{
	int nRet = 0;
	
	Thread_Mutex_Guard guard(_mutex);
	
	if(_items.empty())
	{
		nRet = _cond.wait();
		if(nRet != 0)
		{
			printf("cond wait failed.\n");
			return -1;
		}
	}

	typename std::map<std::string, T >::iterator itr = _items.find(key);
	if(itr != _items.end())
	{
		item = itr->second;
		_items.erase(itr);
	}
	else
	{
		nRet = -2;
		//printf("no item is pop.\n");
	}

	return nRet;
	
}




template <typename T>
int M_Queue<T>::pop(const std::string &key, T &item, const long long timeout)
{
	int nRet = 0;

	long long _timeout = timeout;
	long long pre = get_millisecond();

	Thread_Mutex_Guard guard(_mutex);

	if(_items.empty())
	{
		nRet = _cond.timed_wait(_timeout);
		if(nRet != 0)
		{
			return -1;
		}
	}
	
	while(true)
	{		
		typename std::map<std::string, T >::iterator itr = _items.find(key);
		if(itr != _items.end())
		{
			//在超时时间内找到
			item = itr->second;
			_items.erase(itr);
			return 0;
		}
		else
		{
			//没找到
			long long cur = get_millisecond();
			_timeout -= (cur-pre);
			if(_timeout <= 0)
			{
				return -2;
			}
			pre = cur;
			
			nRet = _cond.timed_wait(_timeout);
			if(nRet != 0)
			{
				return -1;
			}
		}
		
	}
	
	return nRet;

}




template <typename T>
int M_Queue<T>::pop_front(T &item)
{
	int nRet = 0;
	
	Thread_Mutex_Guard guard(_mutex);
	
	if(_items.empty())
	{
		nRet = _cond.wait();
		if(nRet != 0)
		{
			printf("cond wait failed.\n");
			return -1;
		}
	}

	item = _items.begin()->second;
	_items.erase(_items.begin());
	
	return nRet;
}





template <typename T>
int M_Queue<T>::push(const std::string &key, T &item)
{
	int nRet = 0;
	
	Thread_Mutex_Guard guard(_mutex);
	
	_items.insert(std::make_pair(key, item));
	//唤醒所有等待的线程进行处理
	_cond.broadcast();

	return nRet;
}


NS_BASE_END


