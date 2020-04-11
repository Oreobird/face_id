
#include "base_base64.h"
#include "base_string.h"
#include "base_convert.h"

NS_BASE_BEGIN



//--------------------------- base64 ------------------------------	
const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
Base64::Base64()
{

}



Base64::~Base64()
{

}



int Base64::encrypt(const char *pSrc, const unsigned int len, char* &pDst)
{
	int nRet = 0;
	
	unsigned int count = len / 3;
	unsigned int padding_len = len % 3;
	
	if(padding_len)
	{
		++count;
	}
	
	pDst = new char[count*4+1];
	memset(pDst, 0x0, count*4+1);
	//处理前面的3字节对
	for(unsigned int i=0; i<count-1; ++i)
	{		
		unsigned char a = (unsigned char)(pSrc[i*3]);
		unsigned char b = (unsigned char)(pSrc[i*3+1]);
		unsigned char c = (unsigned char)(pSrc[i*3+2]);
		
		pDst[i*4] = base64[a >> 2];
		pDst[i*4+1] = base64[((a & 0x03) << 4) | (b >> 4)];
		pDst[i*4+2] = base64[((b & 0x0F) << 2) | (c >> 6)];
		pDst[i*4+3] = base64[c & 0x3F];
	}

	//处理最后一个3字节对
	unsigned int last_id = count - 1;
	if(padding_len == 0)
	{
		unsigned char a = (unsigned char)(pSrc[last_id*3]);
		unsigned char b = (unsigned char)(pSrc[last_id*3+1]);
		unsigned char c = (unsigned char)(pSrc[last_id*3+2]);
		
		pDst[last_id*4] = base64[a >> 2];
		pDst[last_id*4+1] = base64[((a & 0x03) << 4) | (b >> 4)];
		pDst[last_id*4+2] = base64[((b & 0x0F) << 2) | (c >> 6)];
		pDst[last_id*4+3] = base64[c & 0x3F];
	}
	else if(padding_len == 1)
	{
		unsigned char a = (unsigned char)(pSrc[last_id*3]);
		
		pDst[last_id*4] = base64[a >> 2];
		pDst[last_id*4+1] = base64[(a & 0x03) << 4];
		pDst[last_id*4+2] = '=';
		pDst[last_id*4+3] = '=';
	}
	else if(padding_len == 2)
	{
		unsigned char a = (unsigned char)(pSrc[last_id*3]);
		unsigned char b = (unsigned char)(pSrc[last_id*3+1]);
		
		pDst[last_id*4] = base64[a >> 2];
		pDst[last_id*4+1] = base64[((a & 0x03) << 4) | (b >> 4)];
		pDst[last_id*4+2] = base64[(b & 0x0F) << 2];
		pDst[last_id*4+3] = '=';
	}

	//print_hex(pDst, count*4+1);
	
	return nRet;

}





int Base64::decrypt(const char *pSrc, char* &pDst)
{
	int nRet = 0;

	//判断字符个数时候是4 的倍数
	if(strlen(pSrc) % 4)
	{
		printf("the len(%d) is icorrect.\n", strlen(pSrc));
		return -1;
	}

	//判断是否包含非base64 字符
	for(unsigned int i=0; i<strlen(pSrc); i++)
	{
		if(!is_base64_alphabet(pSrc[i]))
		{
			printf("%c isn't base64 alphabet.\n", pSrc[i]);
			return -2;
		}
	}

	//判断有几个= 字符
	int equal_count = search_str(pSrc, "=");
	if(equal_count > 2)
	{
		printf("equal count(%d) > 2.\n", equal_count);
		return -3;		
	}

	//重新初始化并且转换源字符串，去掉原base64字符串后面的=
	char *base64_src = new char[strlen(pSrc)-equal_count+1];
	for(unsigned int i=0; i< (strlen(pSrc)-equal_count); i++)
	{
		base64_src[i] = get_base64_value(pSrc[i]);
	}
	base64_src[strlen(base64_src)] = '\0';

	//开辟目标字符串空间
	unsigned int count = strlen(pSrc) / 4;
	//解码串长度
	unsigned int dst_len = 0;
	if(equal_count == 0)
	{
		dst_len = count * 3;
	}
	else if(equal_count == 1)
	{
		dst_len = count * 3 - 1;
	}
	else if(equal_count == 2)
	{
		dst_len = count * 3 - 2;
	}
	else
	{
	}
	
	pDst = new char[dst_len+1];
	memset(pDst, 0x0, dst_len+1);

	//处理前面的4字节对
	for(unsigned int i=0; i<count-1; ++i)
	{
		unsigned char a = (unsigned char)(base64_src[i*4]);
		unsigned char b = (unsigned char)(base64_src[i*4+1]);
		unsigned char c = (unsigned char)(base64_src[i*4+2]);
		unsigned char d = (unsigned char)(base64_src[i*4+3]);
		
		pDst[i*3] = (a << 2) | (b >> 4);
		pDst[i*3+1] = (b << 4) | (c >> 2);
		pDst[i*3+2] = (c << 6) | d;
	}

	//处理最后一个4 字节对
	unsigned char last_id = count - 1;
	if(equal_count == 0)
	{
		unsigned char a = (unsigned char)(base64_src[last_id*4]);
		unsigned char b = (unsigned char)(base64_src[last_id*4+1]);
		unsigned char c = (unsigned char)(base64_src[last_id*4+2]);
		unsigned char d = (unsigned char)(base64_src[last_id*4+3]);

		pDst[last_id*3] = (a << 2) | (b >> 4);
		pDst[last_id*3+1] = (b << 4) | (c >> 2);
		pDst[last_id*3+2] = (c << 6) | d;
		
	}
	else if(equal_count == 1)
	{
		unsigned char a = (unsigned char)(base64_src[last_id*4]);
		unsigned char b = (unsigned char)(base64_src[last_id*4+1]);
		unsigned char c = (unsigned char)(base64_src[last_id*4+2]);

		pDst[last_id*3] = (a << 2) | (b >> 4);
		pDst[last_id*3+1] = (b << 4) | (c >> 2);
	}
	else if(equal_count == 2)
	{
		unsigned char a = (unsigned char)(base64_src[last_id*4]);
		unsigned char b = (unsigned char)(base64_src[last_id*4+1]);
		
		pDst[last_id*3] = (a << 2) | (b >> 4);
	}

	DELETE_POINTER_ARR(base64_src);

	return nRet;
	
}






//ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
bool Base64::is_base64_alphabet(char ch)
{
	bool bRet = false;
	if((ch > 47 && ch < 58) ||
	   (ch > 64 && ch < 91) ||
	   (ch > 96 && ch< 123) ||
	   (ch == '+') ||
	   (ch == '/') ||
	   (ch == '='))
	{
		bRet = true;
	}

	return bRet;
}



char Base64::get_base64_value(char ch)
{
	char value = ' ';

	if ((ch >= 'A') && (ch <= 'Z'))   // A ~ Z
	{
	   value = ch - 'A';
	}
	else if ((ch >= 'a') && (ch <= 'z'))   // a ~ z
	{
	   value = ch - 'a' + 26;
	}
	else if ((ch >= '0') && (ch <= '9'))   // 0 ~ 9
	{
	   value = ch - '0' + 52;
	}
	else
	{
		switch (ch)    // 其它字符
		{
		case '+':
		   value = 62;
		case '/':
		   value = 63;
		default:
		   value = 0;
		}
	}
	
	return value;

}


NS_BASE_END

