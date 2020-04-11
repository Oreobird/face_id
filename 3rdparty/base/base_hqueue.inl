
#include "base_hqueue.h"

NS_BASE_BEGIN


template <typename T>
H_Queue<T>::H_Queue()
{

}



template <typename T>
H_Queue<T>::~H_Queue()
{

}



template <typename T>
unsigned int H_Queue<T>::size()
{
	Thread_Mutex_Guard guard(_mutex);
	return _items.size();
}




template <typename T>
bool H_Queue<T>::empty()
{
	Thread_Mutex_Guard guard(_mutex);
	return _items.empty();
}




template <typename T>
bool H_Queue<T>::get(const int key, T &value)
{
	Thread_Mutex_Guard guard(_mutex);
	
	typename std::map<int, T >::iterator itr = _items.find(key);
	if(itr != _items.end())
	{
		//在超时时间内找到
		value = itr->second;
		return true;
	}

	return false;
}



template <typename T>
void H_Queue<T>::insert(const int key, T &value)
{
	Thread_Mutex_Guard guard(_mutex);
	_items.insert(std::make_pair(key, value));
}



template <typename T>
void H_Queue<T>::del(const int key)
{
	Thread_Mutex_Guard guard(_mutex);
	
	typename std::map<int, T >::iterator itr = _items.find(key);
	if(itr != _items.end())
	{
		_items.erase(itr);
	}
}



template <typename T>
void H_Queue<T>::clear()
{
	Thread_Mutex_Guard guard(_mutex);
	_items.clear();
}


NS_BASE_END


