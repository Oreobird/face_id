
#include "base_logger_period.h"
#include "base_os.h"
#include "base_time.h"
#include "base_fs.h"

#include <stdarg.h>

NS_BASE_BEGIN

Logger_Period_Thread::Logger_Period_Thread(X_Queue<std::string> *queue,
		std::string log_path, std::string service_name, 
		EnLogPeriodMode period_mode, unsigned int period_value, unsigned long long max_size, unsigned int buf_sec, 
		std::string ip, unsigned short port): m_fp(NULL), m_open(false),
			m_log_path(log_path), m_log_file(""), m_ip(ip), m_port(port), m_service_name(service_name),
			m_pid(0), m_max_size(max_size), m_buf_sec(buf_sec), m_seq(0), 
			m_pre_buf_timestamp(0), m_period_mode(period_mode), m_period_value(period_value), m_pre_period_index(0), 
			m_queue(queue), m_log_msg("")
{
	m_pid = get_pid();

	//设置初始period index
	get_tm();
	m_pre_period_index = get_period_index();
}



Logger_Period_Thread::~Logger_Period_Thread()
{
	close();
}



int Logger_Period_Thread::svc()
{
	check_log();
	
	std::string log_msg = "";
	int nRet = m_queue->pop(log_msg, 1000);
	if(nRet == 0)
	{
		//追加日志
		m_log_msg += log_msg;
	}

	//获取当前时间戳
	unsigned long long timestamp = getTimestamp();

	//如果log缓存尺寸大于LOG_BUF_MAX_SIZE 就开始写log	
	if(m_log_msg.size() >= 2*1024*1024)
	{
		if(m_fp)
		{
			fprintf(m_fp, "%s", m_log_msg.c_str());
		}
		else
		{
			//重新放到队列后面防止日志丢失
			m_queue->push(m_log_msg);
		}
		
		m_log_msg = "";
		m_pre_buf_timestamp = timestamp;
		return 0;
	}

	//如果当前时间超过了日志缓存的最大时间， 并且有日志信息开始写log
	if(m_log_msg.size() > 0 && ((timestamp-m_pre_buf_timestamp) > m_buf_sec))
	{
		if(m_fp)
		{
			fprintf(m_fp, "%s", m_log_msg.c_str());
		}
		else
		{
			//重新放到队列后面防止日志丢失
			m_queue->push(m_log_msg);
		}
		m_log_msg = "";
		m_pre_buf_timestamp = timestamp;
	}

	return 0;
	
}



int Logger_Period_Thread::do_init(void *args)
{
	//初始化先创建日志文件
	return open();
}



int Logger_Period_Thread::open()
{
	int nRet = 0;

	if(m_open)
	{
		printf("log file(%s) is open already.\n", m_log_file.c_str());
		return -3;
	}
	

	//--------------------------------

	//判断和确定下一个周期文件(一个周期中最多创建100个子文件)
	//找到第一个不存在的日志文件开始处理，对于前面没有文件后面有
	//文件的情况还是有些问题。
	bool check = false;
	for(; m_seq<100; ++m_seq)
	{
		//设置log name
		char szLog[200] = {0};
		snprintf(szLog, 200, "%s/%s.%s.%02u.log", 
			m_log_path.c_str(), m_service_name.c_str(), get_period_date().c_str(), m_seq);
		m_log_file = szLog;

		m_fp = fopen(m_log_file.c_str(), "a+");
		if(m_fp == NULL)
		{
			printf("fopen(%s) failed. errno:%d, errmsg:%s\n", m_log_file.c_str(), errno, strerror(errno));
			return -1;
		}
		else
		{
			//打开成功可能是新创建的一个文件也可能是原来的文件
			printf("fopen(%s) success.\n", m_log_file.c_str());

			unsigned long long size = 0;
			nRet = getFileSize_stat(m_log_file, size);
			if(nRet == 0)
			{
				//判断log文件尺寸是否超过最大值
				if(size >= (m_max_size*1024*1024))
				{
					printf("the size(%lluB) of log file(%s) is bigger than max size(%dM).\n", 
						size, m_log_file.c_str(), m_max_size);
					close();
					continue;
				}
			}

			check = true;
			break;
		
		}
		
	}

	if(!check)
	{
		printf("check all file failed.\n");
		m_seq = 0; //从新开始
		return -1;		
	}

	//--------------------------------
	

	//不需要libc 缓冲， 直接交给内核缓冲
	setbuf(m_fp, NULL);
		 
	m_open = true;

	/*
	//打印文件头信息
	fprintf(m_fp, "Log file created at:%s\n", FormatDateTimeStr(0, "%Y%m%d%H%M%S").c_str());
	fprintf(m_fp, "Service:%s, Machine:%s:%d, Process:%d\n", 
		m_service_name.c_str(), m_ip.c_str(), m_port, m_pid);
	fprintf(m_fp, "IWEF|yyyymmdd hhmmss.uuuuuu|threadid|file:line| msg\n\n");
	*/
	
	return nRet;

}


void Logger_Period_Thread::close()
{
	if(m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;

		m_open = false;
	}
}


/*
存在问题: 没有周期值比较判断。
比如: 每隔5分钟
*/
unsigned int Logger_Period_Thread::get_period_index()
{	
	unsigned int period = 0;
	switch(m_period_mode)
	{	
		case LP_MIN:
		{
			period = m_tm.tm_min;
			break;
		}
		case LP_HOUR:
		{
			period = m_tm.tm_hour;
			break;
		}
		case LP_DAY:
		{
			period = m_tm.tm_mday;
			break;
		}
		case LP_MONTH:
		{
			period = m_tm.tm_mon;
			break;
		}			
		default:
		{
			period = m_tm.tm_hour;
			break;
		}
		
	}

	//当前的周期值除以预定周期值，周期索引从0开始
	return period/m_period_value;

}




std::string Logger_Period_Thread::get_period_date()
{

	char period_date[100] = {0};
	unsigned int period_postfix = m_pre_period_index * m_period_value;

	switch(m_period_mode)
	{	
		case LP_MIN:
		{
			snprintf(period_date, 100, "%d%02d%02d%02d%02d", 
				m_tm.tm_year+1900, m_tm.tm_mon+1, m_tm.tm_mday, m_tm.tm_hour, period_postfix);
			break;
		}
		case LP_HOUR:
		{
			snprintf(period_date, 100, "%d%02d%02d%02d", 
				m_tm.tm_year+1900, m_tm.tm_mon+1, m_tm.tm_mday, period_postfix);
			break;
		}
		case LP_DAY:
		{
			snprintf(period_date, 100, "%d%02d%02d", 
				m_tm.tm_year+1900, m_tm.tm_mon+1, period_postfix);
			break;
		}
		case LP_MONTH:
		{
			snprintf(period_date, 100, "%d%02d", 
				m_tm.tm_year+1900, period_postfix);
			break;
		}			
		default:
		{
			//no-todo
		}
		
	}

	return period_date;

}




void Logger_Period_Thread::check_log()
{
	int nRet = 0;

	//获取当前时间tm 信息
	get_tm();
		
	unsigned long long size = 0;
	nRet = getFileSize_stat(m_log_file, size);
	if(nRet != 0)
	{
		/*
		出现这种现象一般都是由于log 被外部删除了
		需要重新创建新的log文件
		*/
		printf("getFileSize_stat failed, ret:%d\n", nRet);
		
		//重新创建log 文件
		m_open = false;
		nRet = open();
		if(nRet != 0)
		{
			printf("open failed, ret:%d\n", nRet);
			return;
		}
		size = 0;
	}

	bool bMove = false;
	
	//判断log文件尺寸是否超过最大值
	if(size >= (m_max_size*1024*1024))
	{
		printf("the size(%lluB) of log file is bigger than max size(%dM).\n", size, m_max_size);
		bMove = true;
	}

	//判断是否时间已经变化
	unsigned int period_index = get_period_index();
	if(m_pre_period_index != period_index)
	{
		printf("period mode:%u, pre period index:%u, cur period index:%u\n", m_period_mode, m_pre_period_index, period_index);
		m_pre_period_index = period_index;
		bMove = true;
		m_seq = 0;
	}
	
	if(bMove)
	{
		//关闭原来的文件句柄
		close();

		//重新创建文件
		nRet = open();
		if(nRet != 0)
		{
			printf("open failed, ret:%d\n", nRet);
		}

		printf("--- complete to backup old log and renew log ---\n");
		
	}
	
}




void Logger_Period_Thread::get_tm()
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
			
	/*
	struct tm *localtime_r(const time_t *timep, struct tm *result);
	作用: 把time_t (时间戳) 转换成tm格式(本地时间)
	如果成功返回指针同result 一样
	否则返回NULL
	
	与localtime_r相近的是gmtime_r， 返回的是UTC时间(等同GMT时间--格林威治时间)
	struct tm *gmtime_r(const time_t *timep, struct tm *result); 
	*/
	struct tm *ptm = NULL;
	ptm = localtime_r((time_t*)&timestamp, &m_tm);
	if(ptm == NULL)
	{
		printf("localtime_r failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
	}

}




//------------------------------------------

Logger_Period::Logger_Period(): m_queue(NULL), m_thread(NULL), m_init(false), m_close(false)
{

}


Logger_Period::~Logger_Period()
{

}


int Logger_Period::init(std::string log_path, std::string service_name, 
		 EnLogPeriodMode period_mode, unsigned int period_value, unsigned long long max_size, unsigned int buf_sec,
		 std::string ip, unsigned short port)
{
	int nRet = 0;

	if(m_init)
	{
		printf("Logger is init already.\n");
		return -1;
	}

	if(log_path.empty())
	{
		printf("log_path is empty.\n");
		return -1;
	}

	if(!isDirectory(log_path))
	{
		printf("%s isn't not directory.\n", log_path.c_str());
		return -1;		
	}

	//设置服务名称
	if(service_name == "")
	{
		service_name = "test";
	}
	

	//判断日志周期信息
	switch(period_mode)
	{	
		case LP_MIN:
		{
			if((period_value > 59) || (period_value == 0))
			{
				printf("invlide period info, period mode:%d, period value:%d\n", period_mode, period_value);
				return -1;
			}
			break;

		}
		case LP_HOUR:
		{
			if((period_value > 23) || (period_value == 0))
			{
				printf("invlide period info, period mode:%d, period value:%d\n", period_mode, period_value);
				return -1;
			}
			break;
		}
		case LP_DAY:
		{
			if((period_value > 31) || (period_value == 0))
			{
				printf("invlide period info, period mode:%d, period value:%d\n", period_mode, period_value);
				return -1;
			}
			break;
		}
		case LP_MONTH:
		{
			if((period_value > 12) || (period_value == 0))
			{
				printf("invlide period info, period mode:%d, period value:%d\n", period_mode, period_value);
				return -1;
			}
			break;
		}			
		default:
		{
			printf("invlide period mode, period mode:%d, period value:%d\n", period_mode, period_value);
			return -1;
		}
		
	}
	

	//设置日志最大尺寸，单位: M
	if(max_size > MAX_LOG_SIZE)
	{
		//max_size = MAX_LOG_SIZE;
	}
	else if(max_size < MIN_LOG_SIZE)
	{
		max_size = MIN_LOG_SIZE;
	}
	else
	{
		//no to-do
	}

	//设置日志缓存超时时间
	if(buf_sec == 0)
	{
		buf_sec = 1;
	}


	//设置ip
	if(ip == "")
	{
		ip = "127.0.0.1";
	}
	
	//创建log 队列
	m_queue = new X_Queue<std::string>(1000000);

	//启动logger thread
	m_thread = new Logger_Period_Thread(m_queue, log_path, service_name, 
		period_mode, period_value, max_size, buf_sec, ip, port);
	
	nRet = m_thread->init();
	if(nRet != 0)
	{
		printf("Logger_Thread init failed, ret:%d\n", nRet);
		return nRet;
	}
	
	nRet = m_thread->run(NULL);
	if(nRet != 0)
	{
		printf("Logger_Thread run failed, ret:%d\n", nRet);
		return nRet;
	}

	m_init = true;
	
	return nRet;
	
}



int Logger_Period::log_info(const char* file, int line, const char *format, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, format);

	nRet = do_log("I", file, line, format, args);

	va_end(args);

	return nRet;

}

int Logger_Period::log_warning(const char* file, int line, const char *format, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, format);

	nRet = do_log("W", file, line, format, args);

	va_end(args);

	return nRet;

}

int Logger_Period::log_error(const char* file, int line, const char *format, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, format);

	nRet = do_log("E", file, line, format, args);

	va_end(args);

	return nRet;

}

int Logger_Period::log_fatal(const char* file, int line, const char *format, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, format);

	nRet = do_log("F", file, line, format, args);

	va_end(args);

	return nRet;

}



void Logger_Period::close()
{
	DELETE_POINTER(m_queue);

	m_thread->close();
	m_thread->stop();
}



int Logger_Period::do_log(const char *level, const char* file, int line, const char *format, va_list args)
{
	int nRet = 0;
	
	if(!m_init)
	{
		printf("logger isn't init.\n");
		return -1;
	}
	
	//打印应用日志，使用vsnprintf
	char log_msg[MAX_LOG_LEN] = {0};
	vsnprintf(log_msg, MAX_LOG_LEN, format, args);

	std::string log = "";
	log = log_msg;
	
	m_queue->push(log);
	
	return nRet;

}
	


NS_BASE_END


