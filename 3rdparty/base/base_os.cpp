
#include "base_os.h"
#include "base_signal.h"
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/resource.h>


NS_BASE_BEGIN



bool isBigEndian()
{	
	bool bTemp = true;
	
	int pch = 1;
	if(*((char*)&pch) == 1)
	{
		bTemp = false;
	}
	
	return bTemp;
}


pid_t get_pid()
{
	return getpid();
}



pid_t get_ppid()
{
	return getppid();
}



unsigned long get_thread_id()
{
	return pthread_self();  
}


bool is_pid_exist(pid_t pid)
{
	int nRet = 0;

	nRet = send_signal(pid, 0);
	if(nRet == 0)
	{
		return true;
	}

	return false;
}



//#include <sys/resource.h>
int get_rlimit(int resource, struct rlimit *rlim)
{
	int nRet = 0;
	
	nRet = getrlimit(resource, rlim);
	if(nRet != 0)
	{
		printf("getrlimit failed. resource:%d, errno:%d, errmsg:%s\n", 
			resource, errno, strerror(errno));
	}

	return nRet;

}



int get_cpu_number_proc(unsigned short &num)
{
	int nRet =0;

	num = 0;

	FILE* fp = fopen("/proc/cpuinfo", "r");
	if (NULL == fp) 
	{
		printf("fopen /proc/cpuinfo failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
		return -1;
	}
	
	char line[1024] = {0};
	while (fgets(line, sizeof(line)-1, fp))
	{
		char* name = line;
		char* value = strchr(line, ':');
		if (NULL == value)
		{
			continue;
		}
	
		if (0 == strncmp("processor", name, sizeof("processor")-1))
		{
			num = (unsigned short)atoi(++value);
		}
	}
	fclose(fp);
	
	++num;
	
	return nRet;
	
};


NS_BASE_END
	

