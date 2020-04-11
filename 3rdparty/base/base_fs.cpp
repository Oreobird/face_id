
#include "base_fs.h"
#include "base_string.h"

#include <fcntl.h>
#include <dirent.h>


NS_BASE_BEGIN



//#include <dirent.h>
int getFilesByDirectory_api(const std::string directory, 
	std::list<std::string> &lstFiles)
{
	int count = 0;

	if(directory.empty())
	{
		printf("the directory is empty.\n");
		return count;
	}
	
	//opendir 成功返回指向实际DIR的指针，否则返回NULL
	DIR *pDIR = NULL;
	pDIR = opendir(directory.c_str());
	if(pDIR == NULL)
	{
		printf("opendir failed. dir:%s, errno:%d, errmsg:%s\n", directory.c_str(), 
			errno, strerror(errno));
		return -1;
	}

	std::string strFile = "";
	dirent *pDirent = NULL;
	while(NULL != (pDirent = readdir(pDIR)))
	{
		strFile = pDirent->d_name;
		if("." == strFile || ".." == strFile)
		{
			continue;
		}

		lstFiles.push_back(strFile);
		count++;
	}
	
	closedir(pDIR);
	
	return count;

};



int getFileSize_stat(const std::string file, unsigned long long &size)
{
	int nRet = 0;
	
	size = 0;
	if(file == "")
	{
		printf("the file is empty.\n");
		return -1;
	}

	/*
	int stat(const char *pathname, struct stat *buf);
	作用: 返回当前文件的属性信息
	成功返回0， 失败返回-1
	如果文件时软连接， 返回的是该软连接指向的真实文件的属性信息
	*/
	struct stat filestat;
	nRet = stat(file.c_str(), &filestat);
	if(nRet != 0)
	{
		printf("stat failed. file:%s, errno:%d, errmsg:%s\n", 
			file.c_str(),errno, strerror(errno));
		
		nRet = -1;
	}

	size = filestat.st_size;

	return nRet;

}




std::string getCWD()
{
	std::string cwd = "";
	
    char tmp[2014] = {0};
    getcwd(tmp, 2013);
	cwd = tmp;

	if(cwd == "")
	{
		printf("getcwd failed. errno:%d, errmsg:%s\n", errno, strerror(errno));
	}

	return cwd;
	
};




int changeDirectory(const std::string directory)
{
	int nRet = 0;

	if(directory.empty())
	{
		printf("the directory is empty.\n");
		return -1;
	}
	
    nRet = chdir(directory.c_str());
	if(nRet != 0)
	{
		printf("chdir failed. directory:%s, errno:%d, errmsg:%s\n", 
			directory.c_str(), errno, strerror(errno));
	}


	return nRet;
	
};






int moveFile(const std::string srcFile, const std::string dstFile)
{
	int nRet = 0;

	if(srcFile.empty() | dstFile.empty())
	{
		printf("the srcFile|dstFile is empty.\n");
		return -1;
	}
	
    nRet = rename(srcFile.c_str(), dstFile.c_str());
	if(nRet != 0)
	{
		printf("rename failed. (%s > %s), errno:%d, errmsg:%s\n", 
			srcFile.c_str(), dstFile.c_str(), errno, strerror(errno));
	}

	return nRet;
	
};


bool isFileExist(const std::string file, const std::string directory)
{
	bool bExist = false;

	if(file.empty() | directory.empty())
	{
		printf("the file|directory is empty.\n");
		return bExist;
	}

	//opendir 成功返回指向实际DIR的指针，否则返回NULL
	DIR *pDIR = NULL;
	pDIR = opendir(directory.c_str());
	if(pDIR == NULL)
	{
		printf("opendir failed. dir:%s, errno:%d, errmsg:%s\n", directory.c_str(), 
			errno, strerror(errno));
		return -1;
	}
	
	std::string strFile = "";
	std::string strPath = "";
	
	dirent *pDirent = NULL;
	while(NULL != (pDirent = readdir(pDIR)))
	{
		strFile = pDirent->d_name;
		if("." == strFile || ".." == strFile)
		{
			continue;
		}

		strPath = directory + std::string("/") + strFile;
		printf("%s\n", strPath.c_str());

		if(isDirectory(strPath))
		{
			bExist = isFileExist(file, strPath);
			if(bExist)
			{
				break;
			}
		}
		else
		{
			if(file == strFile)
			{
				bExist = true;
				break;
			}
		}
	}
	
	closedir(pDIR);
	
	return bExist;

};



int getFileStat(std::string file, struct stat &filestat)
{
	int nRet = stat(file.c_str(), &filestat);
	if(nRet != 0)
	{
		printf("stat failed. file:%s, errno:%d, errmsg:%s\n", 
			file.c_str(), errno, strerror(errno));
	}

	return nRet;
}




bool isDirectory(const std::string directory)
{
	bool bRet = false;

	if(directory.empty())
	{
		printf("the directory is empty.\n");
		return bRet;
	}

	struct stat filestat;
	int nRet = getFileStat(directory, filestat);
	if(nRet != 0)
	{
		printf("getFileStat(%s) failed, ret:%d\n", directory.c_str(), nRet);
	}
	else
	{
		if(S_ISDIR(filestat.st_mode))
		{
			bRet = true;
		}
	}

	return bRet;	
	
}




bool isFile(const std::string file)
{
	bool bRet = false;

	if(file.empty())
	{
		printf("the file is empty.\n");
		return bRet;
	}
	
	struct stat filestat;
	int nRet = getFileStat(file, filestat);
	if(nRet != 0)
	{
		printf("getFileStat(%s) failed, ret:%d\n", file.c_str(), nRet);
	}
	else
	{
		if(S_ISREG(filestat.st_mode))
		{
			bRet = true;
		}
	}

	return bRet;	


}



//mode=0x777
int chmod(const std::string file, const int mode)
{
	int nRet = 0;

	if(file.empty())
	{
		printf("the file is empty.\n");
		return -1;
	}
	
    nRet = chmod(file.c_str(), mode);
	if(nRet != 0)
	{
		printf("chmod failed. file:%s, errno:%d, errmsg:%s\n", 
			file.c_str(), errno, strerror(errno));
	}

	return nRet;
	
};		



int basename(std::string path, std::string &dname, std::string &bname)
{
	int nRet = 0;

	trim(path);

	if(path.empty())
	{
		printf("the path is empty.\n");
		return -1;
	}

	//判断path 是否是路径
	if(path[path.size()-1] == '/' || path[path.size()-1] == '\\')
	{
		printf("path(%s) is directory.\n", path.c_str());
		return -2;
	}
	
	std::vector<std::string> vecStr;
	split(path, "/\\", vecStr);

	if(vecStr.size() == 1)
	{
		bname = vecStr[0];

		//判断第一个字符是路径分隔符
		if(path[0] == '/' || path[0] == '\\')
		{
			dname =  "/";
		}
		else
		{
			dname =  "./";
		}
	}
	else
	{
		//设置basename
		bname = vecStr[vecStr.size()-1];
		
		//设置dirname
		for(unsigned int i=0; i<(vecStr.size()-2); ++i)
		{
			dname = (vecStr[i] + std::string("/"));
		}
		dname += vecStr[vecStr.size()-2];
		
		//判断第一个字符是路径分隔符
		if(path[0] == '/' || path[0] == '\\')
		{
			dname =  std::string("/") + dname;
		}

	}
	
	return nRet;
	
}



NS_BASE_END


