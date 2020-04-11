
#include "base_time.h"
#include <sys/time.h>
#include <stdarg.h>


NS_BASE_BEGIN

std::string FormatDateTimeStr(unsigned long long timestamp, const char *format)
{
	std::string strDateTime = "";

	tzset();
	
	if(timestamp == 0)
	{
		timestamp = time(NULL);
		if((time_t)timestamp == (time_t)-1)
		{
			printf("time failed. errno:%d, errmsg:%s\n", errno, strerror(errno));	
			return strDateTime;
		}
	}

	struct tm *ptm = NULL;
	struct tm ret_tm = {0};
	ptm = localtime_r((time_t*)&timestamp, &ret_tm);
	if(ptm == NULL)
	{
		printf("localtime_r failed. errno:%d, errmsg:%s\n", errno, strerror(errno));	
		return strDateTime;
	}

	/*
	////考虑性能问题， 不使用strftime， 直接基于tm 结构拼装
	char tmp[16];
	snprintf(tmp, 100, "%04d%02d%02d %02d%02d%02d", ret_tm.tm_year+1900, ret_tm.tm_mon+1, ret_tm.tm_mday,
		ret_tm.tm_hour, ret_tm.tm_min, ret_tm.tm_sec);
	date[15] = '\0';
	*/
	
	size_t nRet = 0;
	char tmp[100] = {0};
	if(format == NULL)
	{
		nRet = strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S %Z", &ret_tm);
	}
	else
	{
		nRet = strftime(tmp, sizeof(tmp), format, &ret_tm);
	}

	if(nRet <= 0)
	{
		printf("strftime failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
	}
	
	strDateTime = tmp;
		

	return strDateTime;
	
}





unsigned long long getTimestamp(const std::string strDate, const char *format)
{
	unsigned long long timestamp = 0;

	tzset();

	if(strDate == "")
	{
		timestamp = time(NULL);
		if((time_t)timestamp == (time_t)-1)
		{
			printf("time failed. errno:%d, errmsg:%s\n", errno, strerror(errno));	
		}
	}
	else
	{
		if(format == NULL)
		{
			printf("format == NULL\n");	
		}
		else
		{	
			struct tm ret_tm = {0};
			if(strptime(strDate.c_str(), format, &ret_tm) == NULL)
			{
				printf("strptime failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
			}
			else
			{
				ret_tm.tm_isdst = -1;
				timestamp = mktime(&ret_tm);
				if((time_t)timestamp == (time_t)-1)
				{
					printf("mktime failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
				}
			}
			
		}
	}

	return timestamp;
	
}



/*
1s = 1000ms = 1000 000 us
millisecond     ms  毫秒
microsecond   us  微秒
*/
void sleep(unsigned long long sec, bool us_option)
{
	if(us_option)
	{
		usleep(sec);
	}
	else
	{
		sleep(sec);
	}
}



int sleep_select(unsigned long long usec)
{
	int nRet = 0;

	unsigned long long _sec = usec / 1000000;
	unsigned long long _usec = usec % 1000000;
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

	}
	else
	{
		printf("select failed, errno:%d, errmsg:%s\n", errno, strerror(errno));
	}

	return nRet;

}





int getTimeOfDay(unsigned long long &sec, unsigned long long &usec)
{
	int nRet = 0;
	
	struct timeval ts;
	nRet = gettimeofday(&ts, NULL);
	if(nRet != 0)
	{
		printf("gettimeofday failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
		
		nRet = -1;
	}

	sec  = ts.tv_sec;
	usec = ts.tv_usec;

	return nRet;

}




int getClockTime(unsigned long long &sec, unsigned long long &nsec, int clk_id)
{
	int nRet = 0;

	struct timespec ts;
	nRet = clock_gettime((clockid_t)clk_id, &ts);
	if(nRet != 0)
	{
		printf("clock_gettime failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
		
		nRet = -1;
	}

	sec  = ts.tv_sec;
	nsec = ts.tv_nsec;

	return nRet;

}



unsigned long long get_microsecond()
{
	int nRet = 0;
	
	struct timeval ts;
	nRet = gettimeofday(&ts, NULL);
	if(nRet != 0)
	{
		printf("gettimeofday failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
		
		nRet = -1;
	}
	
	return ((unsigned long long)ts.tv_sec * 1000000) + ts.tv_usec;
}



unsigned long long get_millisecond()
{
	int nRet = 0;
	
	struct timeval ts;
	nRet = gettimeofday(&ts, NULL);
	if(nRet != 0)
	{
		printf("gettimeofday failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
		
		nRet = -1;
	}
	
	return ((unsigned long long)ts.tv_sec * 1000) + (ts.tv_usec / 1000);
}


NS_BASE_END

