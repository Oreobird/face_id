

#include "base_convert.h"

NS_BASE_BEGIN


template <typename T>
bool Args_Parser::get_opt(const std::string &key, T *value, T* default_value)
{
	bool bRet = false;

	std::map<std::string, std::string>::iterator itr = _mapArgs.find(key);
	if(itr != _mapArgs.end())
	{
		if(value != NULL)
		{
			*value = strTo<T>(itr->second);
		}
		bRet = true;
	}
	else
	{
		if((value != NULL) && (default_value != NULL))
		{
			*value = *default_value;
		}
	}
	
	return bRet;
	
}


NS_BASE_END


