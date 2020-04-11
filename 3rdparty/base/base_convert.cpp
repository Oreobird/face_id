
#include "base_convert.h"
#include "base_os.h"


NS_BASE_BEGIN


int hexstring_to_bin(const char* str, char* hex)
{   
	int nRet = 0;

	if(!str || !hex)
	{
		return -1;
	}

	int len = strlen(str);
	if(len % 2)
	{
		return -2;
	}

	int size = len / 2;
	for (int i=0; i < size; i++)    
    {
         char one_byte[3] = {0};
         memcpy(one_byte, &str[i*2], 2);
         one_byte[2] = '\0';
		 /*
		 #include <stdlib.h>
		 long int strtol(const char *nptr, char **endptr, int base);
		 long long int strtoll(const char *nptr, char **endptr, int base);

		将参数nptr字符串根据参数base来转换成长整型数。
		参数base范围从2至36
		strtol()会扫描参数nptr字符串，跳过前面的空格字符，
		直到遇上数字或正负符号才开始做转换，再遇到非数字或字符串
		结束时('\0')结束转换，并将结果返回。
		若参数endptr不为NULL，则会将遇到的第一个不合条件字符地址赋值给endptr
		 */
         hex[i] = strtol(one_byte, NULL, 16);
    }
	
	return nRet;
	
} 




std::string char_to_hexstring(char c)
{
	std::string value = ""; 			   
	char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	value += hex[(c>>4)&0x0F];
	value += hex[c&0x0F];
	return value;
}



//将2进制装换16进制字符串
std::string bin_to_hexstring(const char *src, unsigned int len)
{
	std::string str = "";
	
	for(unsigned int i=0; i<len; ++i) 
	{
		str += char_to_hexstring(*(src++));
	}

	return str;
}


unsigned long long swap64(unsigned long long ll)
{
	unsigned long long llTmp = \
	  (((ll) >> 56)| \
		(((ll) & 0x00ff000000000000LL) >> 40)| \
		(((ll) & 0x0000ff0000000000LL) >> 24)| \
		(((ll) & 0x000000ff00000000LL) >> 8) | \
		(((ll) & 0x00000000ff000000LL) << 8) | \
		(((ll) & 0x0000000000ff0000LL) << 24)| \
		(((ll) & 0x000000000000ff00LL) << 40)| \
		(((ll) << 56)));

	return llTmp;
}


unsigned long long htonll(unsigned long long ll)
{
	return  isBigEndian() ? ll : swap64(ll);
}


unsigned long long ntohll(unsigned long long ll)
{
	return  isBigEndian() ? ll : swap64(ll);
}


NS_BASE_END
	

