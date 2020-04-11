
#include "base_tail.h"
#include "base_fs.h"
#include "base_time.h"

NS_BASE_BEGIN

Tail_Handler::Tail_Handler()
{

}

Tail_Handler::~Tail_Handler()
{

}

int Tail_Handler::process(const std::string &line)
{
	printf("line(%u):%s\n", line.size(), line.c_str());
	return 0;
}


void Tail_Handler::complete()
{
	printf("file is over!\n");
}



//---------------------------

X_Tail::X_Tail(const std::string &file, const std::string &ending, Tail_Handler *handler, bool truncate_mode):
	_file(file), _ending(ending), _handler(handler), _pre_size(0), 
	_truncate_mode(truncate_mode), _truncated(false)
{

}


X_Tail::~X_Tail()
{
	printf("destroy x-tail!\n");
	DELETE_POINTER(_handler);
}


int X_Tail::do_init(void *args)
{
	if(!isFile(_file))
	{
		return -1;
	}

	return 0;
}


int X_Tail::svc()
{
	int nRet = 0;
	
	unsigned long long size = 0;
	nRet = getFileSize_stat(_file, size);
	if(nRet != 0)
	{
		printf("getFileSize_stat(%s) failed, ret:%d\n", _file.c_str(), nRet);
		return 0;
	}

	//printf("size:%llu, _pre_size:%llu\n", size, _pre_size);
	
	if(size == _pre_size)
	{
		//当前文件没有追加信息，已经读到文件末尾需要重新打开文件继续读取
		//printf("file(%s) isn't appended!\n", _file.c_str());
		sleep(10000, true); //停顿10毫秒
		return 0;
	}
	else if(size < _pre_size)
	{
		if(_truncate_mode)
		{
			//表示文件被截断了
			_truncated = true;
			_pre_size = size;
			printf("file(%s) is truncated.\n", _file.c_str());
		}
		else
		{
			//表示文件重新建立
			_pre_size = 0;
			printf("file(%s) is new one.\n", _file.c_str());
		}
	}
	else
	{
		//no todo
	}
	
	_fin.open(_file.c_str(), std::ios::in);
	if(_fin.is_open())
	{
		if(_truncated)
		{
			/*
			在截断和重新打开文件间隙，文件可能还在持续
			写，这个时候重新打开文件并且从末尾读取文件
			会存在少读几条数据的情况。
			*/
			_fin.seekg(0, std::ios::end);
			_truncated = false;
		}
		else
		{
			//以最后一次读取文件位置为基准继续读文件
			_fin.seekg(_pre_size, std::ios::cur);
		}
		
		//读取并且处理文件
		std::string line = "";
		while(std::getline(_fin, line))
		{
			if(line == "")
			{
				continue;
			}
				
			if(line == _ending)
			{
				//文件结束
				_handler->complete();
				_fin.close();
				return -1;
			}

			//处理数据
			nRet = _handler->process(line);
			if(nRet != 0)
			{
				//如果业务自己返回非0， 表示不再对文件进行tail 操作
				_fin.close();
				return nRet;
			}
			
			/*
			文件每行以0D0A 结束(0D: 回车\r;  0A: 换行\n)
			std::getline 获取的行信息将\n 去掉，所以通过
			std::getline 计算文件尺寸的时候需要加1
			*/
			_pre_size += (line.size() + 1);
			
		}

		//读到文件末尾
		_fin.close();
		
	}
	else
	{
		//文件打开失败后还要持续尝试打开和读取文件
		printf("open file(%s) failed.\n", _file.c_str());
	}

	return 0;

}


NS_BASE_END

