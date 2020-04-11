
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

 

#ifndef _BASE_SINGLETON_H
#define _BASE_SINGLETON_H

#include "base_common.h"
#include "base_thread_mutex.h"

NS_BASE_BEGIN


template <typename T>
class Singleton_T
{
public:
	static T* getInstance()
	{
		if(NULL == m_pInstance)
		{
			m_singleton_lock.lock();
		
			if(NULL == m_pInstance)
			{
				m_pInstance = new T;
			}
			
			m_singleton_lock.unlock();
		}
		
		return m_pInstance;
	}

protected:
	Singleton_T() 
	{
	}

public:
	static T *m_pInstance;
	static Thread_Mutex m_singleton_lock;	
	
};
	

/*
在动态库中使用主程序单件对象
单件静态成员对象重新被分配和赋值为 NULL
*/
template <typename T>
T* Singleton_T<T>::m_pInstance = NULL;

template <typename T>
Thread_Mutex Singleton_T<T>::m_singleton_lock;	

NS_BASE_END


#endif



