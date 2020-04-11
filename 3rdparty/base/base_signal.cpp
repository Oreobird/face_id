
#include "base_signal.h"

#include <signal.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>

NS_BASE_BEGIN


int send_signal(pid_t pid, int signo)
{
	int nRet = 0;
	nRet = kill(pid, signo);
	if(nRet != 0)
	{
		printf("kill failed. pid:%d, signo:%d, errno:%d, errmsg:%s\n", 
			pid, signo, errno, strerror(errno));
	}

	return nRet;
	
}



int add_signal_in_set(sigset_t &sigset, unsigned int num, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, num);

	//初始化信号集
	sigemptyset(&sigset);
	
	int sig = 0;
	for(unsigned int i=0; i<num; i++)
	{
		sig = va_arg(args, int); 
		nRet = sigaddset(&sigset, sig);
		if(nRet != 0)
		{
			printf("sigaddset failed. signo:%d, errno:%d, errmsg:%s\n", 
				sig, errno, strerror(errno));
			va_end(args);
			
			return nRet;
		}
		
	}
	va_end(args);

	return nRet;
	
}



int block_process_signal(sigset_t sigset)
{
	int nRet = 0;

	nRet = sigprocmask(SIG_BLOCK, &sigset, NULL);
	if(nRet != 0)
	{
		printf("sigprocmask failed. errno:%d, errmsg:%s\n", 
			errno, strerror(errno));
	}

	return nRet;
	
};



int wait_singal(sigset_t sigset, int &signo)
{
	int nRet = 0;

	while (true) 
	{
		nRet = sigwait(&sigset, &signo);
		if (EINTR == nRet) 
		{
		    continue;
		}
		else if (nRet != 0)
		{
			printf("sigwait failed. errno:%d, errmsg:%s\n", 
				errno, strerror(errno));
		}
		else
		{
			printf("sigwait success. signo:%d\n", signo);\
		}

		break;
	
	}

	return nRet;
	
}




int signal_api(sighandler_t handler, unsigned int num, ...)
{
	int nRet = 0;
	
	va_list args;
   	va_start(args, num);
	
	int sig = 0;
	for(unsigned int i=0; i<num; i++)
	{
		sig = va_arg(args, int); 
		sighandler_t ret = signal(sig, handler);
		if(ret == SIG_ERR)
		{
			printf("signal failed. signo:%d, errno:%d, errmsg:%s\n", 
				sig, errno, strerror(errno));
			nRet = -1;
			//如果失败了也不用管， 继续操作剩下的信号sig
		}
		
	}
	va_end(args);

	return nRet;

}




NS_BASE_END


