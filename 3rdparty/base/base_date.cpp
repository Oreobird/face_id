
#include "base_date.h"

NS_BASE_BEGIN

XDate::XDate(unsigned long long timestamp): _timestamp(timestamp)
{
	if(timestamp == 0)
	{
		/*
		time_t time(time_t *t); 
		作用:返回从1970-01-01 00:00:00 +0000 (UTC) 到目前的秒数
		如果t不为NULL ， 返回的值也会存储在t 执行空间中
		*/
		unsigned long long timestamp = 0;
		timestamp = time(NULL);
		if((time_t)timestamp == (time_t)-1)
		{
			printf("time failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
		}
		else
		{
			_timestamp = timestamp;
			update();
		}	
	}

}




XDate::XDate(const char *date, const char *format): _timestamp(0)
{

	/*
	#include <time.h>
	void tzset (void);

	3个全局变量:
	extern char *tzname[2];
	extern long timezone;
	extern int daylight

	tzset()函数使用环境变量TZ的当前设置把值赋给三个全局变量:daylight,timezone和tzname
	*/
	tzset();

	/*
	char *strptime(const char *s, const char *format, struct tm *tm);
	作用: 根据format 将时间字符串转换成tm数据
	如果成功返回s数据最后匹配处理的字符下一个字符(可能是字符串结束符\0)
	如果发生错误返回NULL

	注:  glibc 尽可能将strptime 支持的格式化字符串同strftime 一样多，
	但是还是有些格式化字符比如(%Z %u $F 等) 在strptime中不支持
	
	例子:
	strptime("2001-11-12 18:31:01 CST", "%Y-%m-%d %H:%M:%S %Z", &tm);
	*/
	struct tm ret_tm;
	if(strptime(date, format, &ret_tm) == NULL)
	{
		printf("strptime failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
	}
	else
	{
		/*
		time_t mktime(strcut tm * timeptr);
		作用:  将tm数据转换成time_t(时间戳)
		如果成功返回对应时间戳
		如果失败返回-1
		*/
		unsigned long long timestamp = 0;
		timestamp = mktime(&ret_tm);
		if((time_t)timestamp == (time_t)-1)
		{
			printf("mktime failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
		}
		else
		{
			_timestamp = timestamp;
			update();
		}
	}

}




void XDate::update()
{
	if(_timestamp)
	{
		/*
		struct tm 
		{
		   int tm_sec;      //seconds  0~59
		   int tm_min;      // minutes  0~59
		   int tm_hour;     //hours      0~23
		   int tm_mday;    //day of the month    1~31
		   int tm_mon;     //month     0~11
		   int tm_year;     //year       从1900到现在的年数
		   int tm_wday;    //day of the week   0~6
		   int tm_yday;     //day in the year    0~365
		   int tm_isdst;     //daylight saving time   正数表示夏令时生效; 
		                            0表示夏令时无效;
		                            负数表示系统不支持
		};

		struct tm *localtime_r(const time_t *timep, struct tm *result);
		作用: 把time_t (时间戳) 转换成tm格式(本地时间)
		如果成功返回指针同result 一样
		否则返回NULL

		与localtime_r相近的是gmtime_r， 返回的是UTC时间(等同GMT时间--格林威治时间)
		struct tm *gmtime_r(const time_t *timep, struct tm *result); 
		
		*/
		struct tm *ptm = NULL;
		ptm = localtime_r((time_t*)&_timestamp, &_tm);
		if(ptm == NULL)
		{
			printf("localtime_r failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
		}
			

	}
	
}



unsigned int XDate::year()
{
	return _tm.tm_year+1900;
}




unsigned int XDate::month()
{
	return _tm.tm_mon+1;
}



unsigned int XDate::day()
{
	return _tm.tm_mday;
}


unsigned int XDate::hour()
{
	return _tm.tm_hour;
}


unsigned int XDate::min()
{
	return _tm.tm_min;
}


unsigned int XDate::sec()
{
	return _tm.tm_sec;
}


unsigned int XDate::week()
{
	return _tm.tm_wday+1;
}


unsigned int XDate::yday()
{
	return _tm.tm_yday;
}


struct tm XDate::datails()
{
	return _tm;
}


std::string XDate::to_string(const char *format)
{
	/*
	struct size_t strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);
	作用: 根据format 将tm数据转换成相应的时间字符串格式
	如果成功返回处理结果字符串strDest中字符的个数，
	如果发生错误返回零
	*/
	char tmp[100] = {0};
	if(format == NULL)
	{
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S %Z", &_tm);
	}
	else
	{
		strftime(tmp, sizeof(tmp), format, &_tm);
	}
	
	return std::string(tmp);
	
}




unsigned long long XDate::timestamp()
{
	return _timestamp;
}


unsigned long long XDate::spend()
{
	return _timestamp;
}



//重载操作符
XDate XDate::operator+(const XDate &obj)
{
	XDate tmp;
	tmp._timestamp = this->_timestamp + obj._timestamp;
	tmp.update();
	
	return tmp;
}


XDate XDate::operator-(const XDate &obj)
{
	XDate tmp;
	tmp._timestamp = this->_timestamp - obj._timestamp;
	tmp.update();

	return tmp;
}



XDate XDate::operator+(unsigned long long sec)
{
	XDate tmp;
	tmp._timestamp = this->_timestamp + sec;
	tmp.update();
	
	return tmp;

}



XDate XDate::operator-(unsigned long long sec)
{
	XDate tmp;
	tmp._timestamp = this->_timestamp - sec;
	tmp.update();

	return tmp;
}



bool XDate::operator==(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp == obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;
}



bool XDate::operator!=(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp != obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;
}



bool XDate::operator<(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp < obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;
}


bool XDate::operator>(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp > obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;

}


bool XDate::operator<=(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp <= obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;

}


bool XDate::operator>=(const XDate &obj)
{
	bool bRet = false;
	
	if(_timestamp >= obj._timestamp)
	{
		bRet = true; 
	}
	
	return bRet;
}


NS_BASE_END

