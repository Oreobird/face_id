
#include "base_url_parser.h"
#include "base_string.h"
#include "base_convert.h"

NS_BASE_BEGIN

std::string URL_Parser::escape(std::string &str, bool form_data)
{
	std::string strRet = "";
	
	for(unsigned int i=0; i<str.size(); ++i) 
	{
		if(std::isalnum(str[i])) 
		{
			strRet += str[i];
		}
		else 
		{
			if(form_data && (str[i] == ' '))
			{
				strRet += "+";
			}
			else
			{

			}
			strRet += (std::string("%") + char_to_hexstring(str[i]));
		}
	}
	
	return strRet;

}



std::string URL_Parser::unescape(std::string &str, bool form_data)
{
	std::string strRet = "";
	
	std::string::const_iterator itr = str.begin();
	for(; itr != str.end(); ++itr) 
	{
		/*
		std::distance(begin_itr, end_itr); 
		返回容器指定范围中元素个数
		*/
		if ((*itr == '%') &&
			std::isxdigit(*(itr + 1)) && 
			std::isxdigit(*(itr + 2)))
		{
			++itr;

			std::string hex_str(itr, itr+2);
			//printf("hex_str:%s\n", hex_str.c_str());
				
			char hex = ' '; 
			hexstring_to_bin(hex_str.c_str(), &hex);
			strRet += hex;
			
			++itr;
			continue;
		}
		else if(form_data && (*itr == '+'))
		{
			strRet += " ";
		}
		else
		{
			strRet += *itr;
		}
		
	}
	
	return strRet;

}


NS_BASE_END


