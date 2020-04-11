
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
 * Author: xucuiping(89617663@qq.com)
 */
 

#ifndef _BASE_LOGGER_PERIOD_H
#define _BASE_LOGGER_PERIOD_H

#include "base_common.h"
#include "base_thread_mutex.h"
#include "base_thread.h"
#include "base_queue.h"


NS_BASE_BEGIN


/*
按照周期打日志，每个周期最多100个文件
每行日志没有日志头信息
*/

enum EnLogPeriodMode
{
	LP_MIN=0,
	LP_HOUR,
	LP_DAY,
	LP_MONTH
};


//Logger_Thread 写日志线程
class Logger_Period_Thread : public Thread
{		
public:
	Logger_Period_Thread(X_Queue<std::string> *queue,
		std::string log_path, std::string service_name, 
		EnLogPeriodMode period_mode, unsigned int period_value, unsigned long long max_size, unsigned int buf_sec, 
		std::string ip, unsigned short port);

	virtual ~Logger_Period_Thread();
	
	int open();

	void close();
	
private:
	virtual int svc();

	virtual int do_init(void *args);
	
private:
	unsigned int get_period_index();

	std::string get_period_date();
	
	void check_log();	

	void get_tm();
	
private:
	FILE *m_fp;
	bool m_open;
	std::string m_log_path;
	std::string m_log_file;

	//打印参数
	std::string m_ip;
	short m_port;
	std::string m_service_name;
	int m_pid;
	unsigned long long m_max_size;
	unsigned int m_buf_sec;

	//日志控制参数
	unsigned int m_seq;
	unsigned long long m_pre_buf_timestamp;
	unsigned int m_period_mode;
	unsigned int m_period_value;
	unsigned int m_pre_period_index;
	struct tm m_tm;

	X_Queue<std::string> *m_queue;
	std::string m_log_msg;

};



//外部应用调用
class Logger_Period
{
public:
	Logger_Period();

	~Logger_Period();

	int init(std::string log_path="./", std::string service_name="test", 
			EnLogPeriodMode period_mode=LP_HOUR, unsigned int period_value=1, unsigned long long max_size=MAX_LOG_SIZE, unsigned int buf_sec=3,
			std::string ip="127.0.0.l", unsigned short port=0);

	int log_info(const char* file, int line, const char *format, ...);

	int log_warning(const char* file, int line, const char *format, ...);

	int log_error(const char* file, int line, const char *format, ...);

	int log_fatal(const char* file, int line, const char *format, ...);

	void close();
	
private:
	//写日志会自动在日志串后面加上'\n' 用于换行
	int do_log(const char *level, const char* file, int line, const char *format, va_list args);

	
private:
	X_Queue<std::string> *m_queue;
	Logger_Period_Thread *m_thread;

	bool m_init;
	bool m_close;
	
	
};




//方便应用调用
#define XCP_PERIOD_LOGGER_INFO(logger, format, ...)\
if (logger)\
{\
	(logger)->log_info(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_PERIOD_LOGGER_WARNING(logger, format, ...)\
if (logger)\
{\
	(logger)->log_warning(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_PERIOD_LOGGER_ERROR(logger, format, ...)\
if (logger)\
{\
	(logger)->log_error(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_PERIOD_LOGGER_FATAL(logger, format, ...)\
if (logger)\
{\
	(logger)->log_fatal(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


NS_BASE_END

#endif


