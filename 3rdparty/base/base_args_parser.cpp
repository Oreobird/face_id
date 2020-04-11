
#include "base_args_parser.h"
#include "base_string.h"


NS_BASE_BEGIN

Args_Parser::Args_Parser(): _process_name("")
{

}


Args_Parser::~Args_Parser()
{

}



//解析命令行参数格式 "-option1 value -option2 value"
void Args_Parser::parse_args(int argc, char **args)
{
	_mapArgs.clear();

	//解析执行文件名称
	std::string process_name = args[0];
	std::string::size_type pos = process_name.find_last_of("/");
	if(pos == std::string::npos)
	{
		pos = process_name.find_last_of("\\");
		if(pos == std::string::npos)
		{
			_process_name = process_name;
		}
		else
		{
			_process_name = process_name.substr(pos+1, process_name.size()-pos-1);
		}		
	}
	else
	{
		_process_name = process_name.substr(pos+1, process_name.size()-pos-1);
	}
	
	std::string tmp = "";
	for(int i=1; i<argc; i++)
	{
		tmp = args[i];
		if(tmp[0] == '-')
		{
			std::string key = tmp.substr(1);
			std::string value = "";

			if(i != argc-1)
			{
				tmp = args[i+1];
				if(tmp[0] != '-')
				{
					value = tmp;
					++i;
				}
			}
			
			_mapArgs[key] = value;
			
		}
		else
		{
			printf("invalid arg:%s\n", tmp.c_str());
		}
		
	}
	
}




//解析参数串 参数串格式 "-option1 value -option2 value"
void Args_Parser::parse_args(std::string &args)
{
	_mapArgs.clear();
	
	std::vector<std::string> vecArgs;
	split(args, " \t", vecArgs);

	std::string tmp = "";
	for(unsigned int i=0; i<vecArgs.size(); i++)
	{
		tmp = vecArgs[i];
		if(tmp[0] == '-')
		{
			std::string key = tmp.substr(1);
			std::string value = "";
			
			tmp = vecArgs[i+1];
			if(tmp[0] != '-')
			{
				value = tmp;
				++i;
			}

			_mapArgs[key] = value;
			
		}
		else
		{
			printf("invalid arg:%s\n", tmp.c_str());
		}
		
	}
	
}



void Args_Parser::show()
{
	std::map<std::string, std::string>::iterator itr = _mapArgs.begin();
	for(; itr != _mapArgs.end(); ++itr)
	{
		printf("key:%s, value:%s\n", itr->first.c_str(), itr->second.c_str());
	}
}



std::string Args_Parser::get_process_name()
{
	return _process_name;
}





NS_BASE_END


