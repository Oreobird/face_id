
#include "mysql_mgt.h"
#include "base_convert.h"

mysql_mgt g_mysql_mgt;

//mysql conn
mysql_mgt::mysql_mgt()
{

}

mysql_mgt::~mysql_mgt()
{

}



//初始化
int mysql_mgt::init(const std::string &ip, unsigned int port,
		                  const std::string &user, const std::string &pwd,
			              const std::string &db, const unsigned int cnt, const std::string &chars)
{
	int nRet = 0;

	//创建mysql连接池
	for(unsigned int i=0; i<cnt; i++)
	{
		mysql_conn_Ptr conn = new mysql_conn(i);
		nRet = conn->connect_conn(ip, port, user, pwd, db, chars);
		if(nRet != 0)
		{
			printf("connect_conn failed, error code:%d\n", nRet);
			return nRet;
		}
		_conn_queue.push_back(conn);
	}

	return nRet;

}



bool mysql_mgt::get_conn(mysql_conn_Ptr &conn)
{
	Thread_Mutex_Guard guard(_mutex);

	bool bRet = false;

	std::vector<mysql_conn_Ptr>::iterator itr = _conn_queue.begin();
	for(; itr != _conn_queue.end(); itr++)
	{
		if(!(*itr)->_used && (*itr)->_conn)
		{
			conn = *itr;
			(*itr)->_used = true;
			bRet = true;
			break;
		}
	}

	if(!bRet)
	{
		printf("no free conn is gotten, pls try next time.\n");
	}

	return bRet;

}




void mysql_mgt::check()
{
	Thread_Mutex_Guard guard(_mutex);

	std::vector<mysql_conn_Ptr>::iterator itr = _conn_queue.begin();
	for(; itr != _conn_queue.end(); itr++)
	{
		(*itr)->ping();
	}

}



void mysql_mgt::release(mysql_conn_Ptr &conn)
{
	Thread_Mutex_Guard guard(_mutex);
	conn->_used = false;
}



//----------------------------------------

MySQL_Guard::MySQL_Guard(mysql_conn_Ptr &conn): _conn(conn)
{

}


MySQL_Guard::~MySQL_Guard()
{
	PSGT_Mysql_Mgt->release(_conn);
}


mysql_conn_Ptr& MySQL_Guard::operator-> ()
{
	return _conn;
}

