
#include "base_utility.h"
#include "base_close_helper.h"
#include "base_os.h"

#include <sys/stat.h>
#include <math.h>
#include <signal.h>
#include <execinfo.h>
#include "base_time.h"


NS_BASE_BEGIN

void init_daemon()
{
	printf("---- init_daemon ----\n");
	printf("---- father process ----\n");

	//父进程
	int pid = 0;
	if(pid == fork())
	{
		printf("---- exit father process ----\n");
		exit(0); //结柬父进程
	}
	else if(pid<0)
	{
		printf("fork child process failed.\n");
		exit(-1);
	}

	//子进程
	printf("---- child process ----\n");
	
	//setsid 子进程成为新的会活组长和进程组长，并与原来的控制终端分离
	setsid();

	//chdir("/"); //改变孙进程工作路径 "/"
	
	umask(0);   //重新设置文件创建屏蔽字

	printf("---- complete to init_daemon ----\n");


};



void xprintf(const char* file, int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	unsigned long long sec = 0;
	unsigned long long usec = 0;
	getTimeOfDay(sec, usec);
	std::string date = FormatDateTimeStr(sec, "%Y/%m/%d %H:%M:%S");

	char buf[1024] = {0};
	snprintf(buf, 1024, "%s.%llu|%u:%lu|%s:%d|%s", 
		date.c_str(), usec, get_pid(), get_thread_id(), file, line, format);
	vprintf(buf, args);
		
	va_end(args);
}



void set_random_seed()
{
	srand((unsigned int)time(NULL));
}


//#include <math.h>
unsigned int get_random(unsigned int range)
{
	unsigned int num = 0;
	
	if(range == 0)
	{
		num = rand();
	}
	else
	{
		float result = ((float)rand()/(float)RAND_MAX)*((float)range);		
		num = (unsigned int)round(result);
	}
	
	return num;
	
}



int pid_file(const std::string &file)
{
	int nRet = 0;
	
	if(file == "")
	{
		printf("pid_file is invalid.\n");	
		return -1;
	}
	
	FILE *fp = NULL;
	fp = fopen(file.c_str(), "w");
	if(fp == NULL)
	{
		printf("fopen(%s) failed, errno:%d, errmsg:%s\n", file.c_str(), errno, strerror(errno));
 		return -1;
	}

	CloseHelper<FILE*> tmp(fp);
		
	/*
	fprintf 写缓存， 在执行fclose(fp)的时候才真正将缓冲数据写入
	硬盘，write 这种低级io操作是没有写缓存的
	*/
	fprintf(fp, "%d,%llu", get_pid(), getTimestamp());
    
    return nRet;

}



NS_BASE_END


