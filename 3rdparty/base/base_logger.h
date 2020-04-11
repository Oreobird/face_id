
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
 

#ifndef _BASE_LOGGER_H
#define _BASE_LOGGER_H

#include "base_common.h"
#include "base_thread_mutex.h"
#include "base_thread.h"
#include "base_queue.h"


NS_BASE_BEGIN

//Logger_Thread 写日志线程
class Logger_Thread : public Thread
{		
public:
	Logger_Thread(X_Queue<std::string> *queue,
		std::string log_path, std::string service_name, 
		unsigned int max_size, unsigned long long max_sec, unsigned int buf_sec, 
		std::string ip, unsigned short port);

	virtual ~Logger_Thread();
	
	int open();

	void close();
	
private:
	virtual int svc();

	virtual int do_init(void *args);
	
private:
	void check_log();	
	
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
	unsigned long long m_max_sec;  //超过这个秒数就换日志文件新写
	unsigned int m_max_size;
	unsigned int m_buf_sec;

	//日志控制参数	
	unsigned long long m_pre_timestamp;
	std::string  m_create_date;
	unsigned long long m_cur_timestamp;

	X_Queue<std::string> *m_queue;
	std::string m_log_msg;
};



//外部应用调用
class Logger
{
public:
	Logger();

	~Logger();

	int init(std::string log_path="./", std::string service_name="test", 
			 unsigned int max_size=MAX_LOG_SIZE, unsigned long long max_sec=3600, 
			 unsigned int buf_sec=LOG_BUF_SECS, std::string ip="127.0.0.l", unsigned short port=0);

	int log_info(const char* file, int line, const char *format, ...);

	int log_warning(const char* file, int line, const char *format, ...);

	int log_error(const char* file, int line, const char *format, ...);

	int log_fatal(const char* file, int line, const char *format, ...);
	
	void close();
	
private:
	//写日志会自动在日志串后面加上'\n' 用于换行
	//接口性能在 15W/s， 落地11M/s
	int do_log(const char *level, const char* file, int line, const char *format, va_list args);


	
private:
	X_Queue<std::string> *m_queue;
	Logger_Thread *m_thread;

	bool m_init;
	bool m_close;
	
	
};




//方便应用调用(普通日志)
#define XCP_LOGGER_INFO(logger, format, ...)\
if (logger)\
{\
	(logger)->log_info(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_LOGGER_WARNING(logger, format, ...)\
if (logger)\
{\
	(logger)->log_warning(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_LOGGER_ERROR(logger, format, ...)\
if (logger)\
{\
	(logger)->log_error(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};


#define XCP_LOGGER_FATAL(logger, format, ...)\
if (logger)\
{\
	(logger)->log_fatal(__FILE__, __LINE__, format, ##__VA_ARGS__);\
};

NS_BASE_END

#endif


