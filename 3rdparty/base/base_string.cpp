
#include "base_string.h"
#include "base_utility.h"


NS_BASE_BEGIN

void trim(std::string &str, char ch)
{
	if(str.empty())
	{
		return;
	}

	int i = str.length() -1;
	for(; (i>=0) && (std::isspace(str[i]) || (str[i]==ch)); i--);

	if(i < 0)
	{
		str = "";
		return;
	}
	
	int j = 0;
	for(; (j<i) && (std::isspace(str[j]) || (str[j]==ch)); j++);

	if(j <= i)
	{
		str = str.substr(j, i+1-j);
	}
	
};




void trim_left(std::string &str)
{
	if(str.empty())
	{
		return;
	}

	int i = 0;
	for(; std::isspace(str[i]) && (i < (int)str.length()); i++);

	if(i >= (int)str.length())
	{
		str = "";
	}
	else
	{
		str = str.substr(i, str.length()-i);
	}

}




void trim_right(std::string &str)
{
	if(str.empty())
	{
		return;
	}

	int i = str.length() -1;
	for(; (i>=0) && std::isspace(str[i]); i--);

	if(i < 0)
	{
		str = "";
	}
	else
	{
		str = str.substr(i, i+1);
	}

}




void upperCase(std::string &str)
{
	if(str.empty())
	{
		return;
	}

	for(std::string::size_type i=0; i<str.length(); i++)
	{
		str[i] += toupper(str[i]);
	}
};




void lowerCase(std::string &str)
{
	if(str.empty())
	{
		return;
	}

	for(std::string::size_type i=0; i<str.length(); i++)
	{
		str[i] += tolower(str[i]);
	}

};




void split(const std::string str, const std::string delim, std::vector<std::string> &vecStr, int num)
{
	if(str.empty() || delim.empty())
	{
		return;
	}
	
	char *pBuffer = new char[str.size()+1];
	memcpy(pBuffer, str.c_str(), str.size());
	pBuffer[str.size()] = '\0';

	int _num = 0;
	char *pToken = NULL;
	char *pSaveptr = NULL;
	pToken = strtok_r(pBuffer, delim.c_str(), &pSaveptr);
	while(pToken != NULL)
	{
		++_num;
		vecStr.push_back(pToken);
		pToken = strtok_r(NULL, delim.c_str(), &pSaveptr);
		if(_num == num)
		{
			break;
		}
	}

	DELETE_POINTER_ARR(pBuffer);
	
};



void split(const std::string str, const std::string delim, std::list<std::string> &lstStr, int num)
{	
	if(str.empty() || delim.empty())
	{
		return;
	}
	
	char *pBuffer = new char[str.size()+1];
	memcpy(pBuffer, str.c_str(), str.size());
	pBuffer[str.size()] = '\0';

	int _num = 0;
	char *pToken = NULL;
	char *pSaveptr = NULL;
	pToken = strtok_r(pBuffer, delim.c_str(), &pSaveptr);
	while(pToken != NULL)
	{
		++_num;
		
		lstStr.push_back(pToken);
		pToken = strtok_r(NULL, delim.c_str(), &pSaveptr);
		if(_num == num)
		{
			break;
		}
	}

	DELETE_POINTER_ARR(pBuffer);
	
}





void remove(std::string &strSrc, const std::string strSub)
{
	std::string::size_type pos = 0;
	if((pos = strSrc.find(strSub)) == std::string::npos || strSub.empty())
	{
		return;
	}
	
	while(pos != std::string::npos)
	{
		strSrc.erase(pos, strSub.size());
		pos = strSrc.find(strSub);
	}
	
}


int search_str(const std::string strSrc, const std::string strSearch)
{
	bool nCnt = 0;
	if(strSrc.empty() || strSearch.empty())
	{
		return nCnt;
	}

	std::string::size_type pos = strSrc.find(strSearch);
	while(pos != std::string::npos)
	{
		++nCnt;
		pos = strSrc.find(strSearch, pos+strSearch.size());
	}

	return nCnt;
}


bool is_digit(std::string &str)
{
	if(str == "")
	{
		return false;
	}
	
	for(unsigned int i=0; i< str.size(); ++i)
	{
		if(std::isdigit(str[i]))
		{
			continue;
		}
		else
		{
			return false;
		}
	}

	return true;
	
}



bool is_xdigit(std::string &str)
{
	if(str == "")
	{
		return false;
	}
	
	for(unsigned int i=0; i< str.size(); ++i)
	{
		if(std::isxdigit(str[i]))
		{
			continue;
		}
		else
		{
			return false;
		}
	}

	return true;
	
}



std::string format(const char *format, ...)
{
	std::string str = "";
	
	va_list args;
	va_start(args, format);

	char buf[4096] = {0};
	//memset(buf, 0x0, 4096);
	vsnprintf(buf, 4096, format, args);
		
	va_end(args);
	
	str = buf;

	return str;

}


std::string replace_str(const std::string strSrc, 
	const std::string strSearch, const std::string strReplace, unsigned int pos)
{
	std::string strNew = strSrc;
	if(strSrc.empty() || strSearch.empty() || strReplace.empty() || (pos >= strSrc.size()))
	{
		printf("invalid parameter! src:%s, search:%s, replace:%s, pos:%u\n", 
			strSrc.c_str(), strSearch.c_str(), strReplace.c_str(), pos);
		return strNew;
	}

	std::string::size_type srclen = strSearch.size();
	std::string::size_type dstlen = strReplace.size();
	std::string::size_type tag = strNew.find(strSearch, pos);
	while (tag != std::string::npos) 
	{
		strNew.replace(tag, srclen, strReplace);
		tag += dstlen;
		tag = strNew.find(strSearch, tag);
	}	

	return strNew;


}



//可见字符集[32~126]，32是空格去掉总共94个
std::string random_str(unsigned int max)
{	
	unsigned int count = get_random(max);
	if(count < 3)
	{
		count = 3;
	}

	unsigned int index = 0;
	std::string str = "";
	for(unsigned int i=0; i<count; i++)
	{
		index = get_random(93);
		str += char(index+33);
	}

	return str;
}


void print_hex(const char *buf, int size, const std::string &text)
{
	if((buf == NULL) || (size < 1))
	{
		printf("(buf == NULL) || (size < 1)\n");
		return;
	}
	
	std::string header = "buffer";
	if(text != "")
	{
		header = text;
	}
	
	int row = size/16;
	row += (size%16) ? 1 : 0;
		
	printf("%s-content(%d byte):\n", header.c_str(), size);
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<16; j++)
		{
			
			printf("%02x ", (unsigned char)buf[i*16+j]);
			

			//if((i == (row-1)) && (j != 0) && (j == ((size%16)-1)))
			if((i == (row-1)) && (j == ((size%16)-1)))
			{
				for(int k=0; k<(16-(size%16)); k++)
				{
					printf("%2c ", ' ');
				}
				
				break;
			}
			
		}
		printf("--- ");
		
		for(int j=0; j<16; j++)
		{
			if(((unsigned char)buf[i*16+j]>31) && ((unsigned char)buf[i*16+j]<129))
			{
				printf("%c", buf[i*16+j]);
			}
			else
			{
				printf(".");
			}
			
			//if((i == (row-1)) && (j != 0) && (j == ((size%16)-1)))
			if((i == (row-1)) && (j == ((size%16)-1)))
			{
				break;
			}
						
		}		
		printf("\n");
		
	}
	printf("\n");

}



NS_BASE_END


