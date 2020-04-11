
#include "base_uid.h"
#include "base_time.h"
#include "base_utility.h"


NS_BASE_BEGIN

unsigned long long	UID::m_uid = 0;
Thread_Mutex UID::m_inc_lock;

//递增全局序列
std::string UID::uid_inc(const std::string &prefix)
{
	Thread_Mutex_Guard guard(m_inc_lock);
	
	++m_uid;
	
	if(0 == m_uid) 
	{
		m_uid = 1;
	}
	
	char tmp[100] = {0};
	snprintf(tmp, 100, "%s_%llu_%llu", prefix.c_str(), getTimestamp(), m_uid);

	return tmp;
}

NS_BASE_END


