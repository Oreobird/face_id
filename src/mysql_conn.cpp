
#include "mysql_conn.h"
#include "base_convert.h"
#include "base_logger.h"
#include "common.h"

extern Logger g_logger;

//----------------------- row -----------------------

MySQL_Row::MySQL_Row()
{

}

MySQL_Row::~MySQL_Row()
{

}


std::string MySQL_Row::operator [](int field_id)
{
	std::string value = "";

	if(field_id < 0)
	{
		printf("the field id(%d) is negative.\n", field_id);

	}
	else if(m_Record.empty())
	{
		printf("row is empty.\n");
	}
	else if((field_id+1) > (int)m_Record.size())
	{
		printf("the field id(%d) is invalid.\n", field_id);
	}
	else
	{
		value = m_Record.at(field_id);
	}

	return value;

}


void MySQL_Row::operator()(const std::string &value)
{
	m_Record.push_back(value);
}


template <typename T>
T MySQL_Row::get_value(int field_id)
{
	T value;

	if(field_id < 0)
	{
		printf("the field id(%d) is negative.\n", field_id);

	}
	else if(m_Record.empty())
	{
		printf("row is empty.\n");
	}
	else if((field_id+1) > (int)m_Record.size())
	{
		printf("the field id(%d) is invalid.\n", field_id);
	}
	else
	{
		value = strTo<T>(m_Record.at(field_id));
	}

	return value;
}


//----------------------- row set -----------------------

MySQL_Row_Set::MySQL_Row_Set(): m_row(0), m_field(0)
{
	m_Record_Set.clear();
	m_Field_Name.clear();
}

MySQL_Row_Set::~MySQL_Row_Set()
{

}

int MySQL_Row_Set::field_count()
{
	return m_field;
}


void MySQL_Row_Set::field_count(int count)
{
	m_field = count;
}


int MySQL_Row_Set::row_count()
{
	return m_row;
}



void MySQL_Row_Set::row_count(int count)
{
	m_row = count;
}


bool MySQL_Row_Set::empty()
{
	return m_Record_Set.empty();
}



int MySQL_Row_Set::field_id(std::string field_name)
{
	int field_id = -1;

	std::map<std::string, int>::iterator itr = m_Field_Name.begin();
	if(itr != m_Field_Name.end())
	{
		field_id = itr->second;
	}
	else
	{
		printf("the row name(%s) isn't found.\n", field_name.c_str());
	}

	return field_id;
}



MySQL_Row MySQL_Row_Set::operator[](int row_id)
{
	MySQL_Row row;

	if(row_id < 0)
	{
		printf("row id(%d) is negative.\n", row_id);
	}
	else if(m_Record_Set.empty())
	{
		printf("row set is empty.\n");
	}
	else if ((row_id+1) > (int)m_Record_Set.size())
	{
		printf("the row id(%d) is invalid.\n", row_id);
	}
	else
	{
		row = m_Record_Set.at(row_id);
	}


	return row;

}



void MySQL_Row_Set::operator()(MySQL_Row &row)
{
	m_Record_Set.push_back(row);
}



int MySQL_Row_Set::operator()(std::string field_name)
{
	return field_id(field_name);
}



void MySQL_Row_Set::set_field_name(std::string field_name, int index)
{
	m_Field_Name[field_name] = index;
}



void MySQL_Row_Set::show_all()
{
	std::map<std::string, int>::iterator itr = m_Field_Name.begin();
	for(; itr != m_Field_Name.end(); itr++)
	{
		printf("%s\t|", itr->first.c_str());
	}
	printf("\n");


	std::vector<MySQL_Row>::iterator itr_row = m_Record_Set.begin();
	for(; itr_row != m_Record_Set.end(); itr_row++)
	{
		for(int i=0; i<m_field; i++)
		{
			//printf("%s\t|", (*itr_row)[i].c_str());
			printf("%s\t|", (*itr_row).get_value<std::string>(i).c_str());
		}
		printf("\n");
	}


}




//mysql conn
mysql_conn::mysql_conn(int seq): _mysql(NULL), _ip(""), _port(3306), _user(""), _pwd(""),
_db(""), _chars(""), _conn(false), _used(false), _seq(seq)
{

}

mysql_conn::~mysql_conn()
{


}

int mysql_conn::connect_conn(const std::string &ip, unsigned int port,
		                  const std::string &user, const std::string &pwd,
			              const std::string &db, const std::string &chars)
{
	int nRet = 0;

	if(_conn)
	{
		printf("the mysql is already connected.\n");
		return 0;
	}

	_ip = ip;
	_port = port;
	_user = user;
	_pwd = pwd;
	_db = db;
	_chars = chars;

	_mysql = mysql_init(NULL);
	if(_mysql == NULL)
	{
		printf("mysql_init failed, error code:%s\n", mysql_error(_mysql));
		return -1;
	}

	//设置链接超时时间
	unsigned char conn_timeout = 5;
	mysql_options(_mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&conn_timeout);

	//设置读写超时时间
    unsigned int read_timeout = 5;
	unsigned int write_timeout = 5;
	mysql_options(_mysql, MYSQL_OPT_READ_TIMEOUT, (const char *)&read_timeout);
	mysql_options(_mysql, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&write_timeout);

	my_bool auto_reconnect = 1;
	mysql_options(_mysql, MYSQL_OPT_RECONNECT, &auto_reconnect);

	printf("prepare to connect db server ...\n");
	printf("ip:%s\nport:%d\nuser:%s\npwd:%s\ndb:%s\nchar_set:%s\n",
		    _ip.c_str(), _port, _user.c_str(), _pwd.c_str(), _db.c_str(), _chars.c_str());

	if (!mysql_real_connect(_mysql, _ip.c_str(), _user.c_str(), _pwd.c_str(),
		 _db.c_str(), _port, NULL, CLIENT_MULTI_STATEMENTS))
	{
		printf("mysql_real_connect failed, ip:%s, port:%d, user:%s, pwd:%s, db:%s, chars:%s, "
			"error code:%s, err msg:%s\n",
			 _ip.c_str(), _port, _user.c_str(), _pwd.c_str(),
			 _db.c_str(), _chars.c_str(), mysql_error(_mysql), mysql_error(_mysql));

		return -2;
	}
	else
	{
		printf("mysql_real_connect success.\n");
	}


	//设置连接语言集
	if(chars != "")
	{
		if(mysql_set_character_set(_mysql, chars.c_str()))
		{
			printf("mysql_set_character_set failed, err code:%d, err msg:%s\n",
				mysql_errno(_mysql), mysql_error(_mysql));
			return -3;
		}
	}

	_conn = true;

	printf("connect mysql success!!!\n");

	return nRet;

}


int mysql_conn::query_sql(const std::string sql, MySQL_Row_Set &row_set, std::string &errInfo)
{

	int nRet = 0;

	if(!_conn)
	{
		XCP_LOGGER_ERROR(&g_logger, "the mysql is not connected.\n");
		errInfo = "the mysql is not connected";
		return ERR_GET_MYSQL_CONN_FAILED;
	}

	if(sql.empty())
	{
		XCP_LOGGER_ERROR(&g_logger, "the sql is empty.\n");
		errInfo = "the sql is empty";
		return ERR_MYSQL_OPER_EXCEPTION;
	}

	XCP_LOGGER_INFO(&g_logger, "excute sql command:%s.\n", sql.c_str());
	nRet = mysql_real_query(_mysql, sql.c_str(), sql.size());
	if (0 != nRet)
	{
		errInfo = mysql_error(_mysql);
		XCP_LOGGER_ERROR(&g_logger, "mysql_real_query failed, sql:%s, ret:%d, err code:%d, err msg:%s\n",
			sql.c_str(), nRet, mysql_errno(_mysql), errInfo.c_str());
		return ERR_MYSQL_OPER_EXCEPTION;
	}

	//printf("mysql_real_query success, sql:%s\n", sql.c_str());

	MYSQL_RES* pRes = NULL;
	pRes = mysql_store_result(_mysql);
	if(!pRes)
	{
		XCP_LOGGER_ERROR(&g_logger, "mysql_store_result failed, sql:%s, err code:%d, err msg:%s\n",
			sql.c_str(), mysql_errno(_mysql), mysql_error(_mysql));

		return ERR_MYSQL_OPER_EXCEPTION;
	}

	int num_field = mysql_num_fields(pRes);
	int num_row = mysql_num_rows(pRes);

	row_set.field_count(num_field);
	row_set.row_count(num_row);

	if (0 == num_row)
	{
		//printf("no row is selected.\n");
		mysql_free_result(pRes);
		//return -5;
		XCP_LOGGER_INFO(&g_logger, "query success but get zero record.\n");
		return 0;
	}

	//获取查询结果字段名列表
	MYSQL_FIELD *fields = NULL;
	fields = mysql_fetch_fields(pRes);
	for(int i = 0; i<num_field; i++)
	{
		row_set.set_field_name(fields[i].org_name, i);
	}


	//MYSQL_ROW是在MYSQL_RES的基础上操作所以没有内存释放的问题
	MYSQL_ROW ppRow = NULL;
	while ((ppRow = mysql_fetch_row(pRes)) != NULL)
	{
		MySQL_Row row;
		for (int i=0; i<num_field; i++)
		{
			/*
			如果数据表字段值为NULL，通过MYSQL_ROW 获取到对应字段值也为
			NULL，这个地方开发需要判断， 防止crash
			*/
			if(ppRow[i] == NULL)
			{
				//row(".null");
				row("");
			}
			else
			{
				std::string temp = ppRow[i];
				row(temp);
			}
		}

		row_set(row);

	}

	mysql_free_result(pRes);

	XCP_LOGGER_INFO(&g_logger, "query sucess. get %d record\n", num_row);
	return 0;

}




int mysql_conn::execute_sql(const std::string sql, unsigned long long &last_insert_id, unsigned long long &affected_rows, std::string &errInfo)
{
	int nRet = 0;

	if(!_conn)
	{
		XCP_LOGGER_ERROR(&g_logger, "the mysql is not connected.\n");
		errInfo = "the mysql is not connected";
		return ERR_GET_MYSQL_CONN_FAILED;
	}

	if(sql.empty())
	{
		XCP_LOGGER_INFO(&g_logger, "the sql is empty.\n");
		errInfo = "the sql is empty";
		return ERR_MYSQL_OPER_EXCEPTION;
	}

	XCP_LOGGER_INFO(&g_logger, "excute sql command:%s.\n", sql.c_str());

	nRet = mysql_real_query(_mysql, sql.c_str(), sql.size());
	if (0 != nRet)
	{
		errInfo = mysql_error(_mysql);
		XCP_LOGGER_ERROR(&g_logger, "execute_sql failed, ret:%d, err code:%d, err msg:%s\n",
			nRet, mysql_errno(_mysql), errInfo.c_str());
		if(mysql_errno(_mysql) == ER_DUP_ENTRY)
		{
			return ERR_DUPLICATE_KEY;
		}

		if(mysql_errno(_mysql) == ER_TABLE_EXISTS_ERROR)
		{
			return ERR_TABLE_EXISTS;
		}

		return ERR_MYSQL_OPER_EXCEPTION;
	}

	//printf("exec sql success, sql:%s\n", sql.c_str());


	//返回由以前的INSERT或UPDATE语句为AUTO_INCREMENT列生成的值
	last_insert_id = (unsigned long long)mysql_insert_id(_mysql);

	/*
	返回上次UPDATE更改的行数，上次DELETE 删除的行数，
	或上次INSERT语句插入的行数
	*/
	affected_rows = (unsigned long long)mysql_affected_rows(_mysql);

	//把执行多条sql语句返回的多个结果集清理掉，为后面执行sql准备
	MYSQL_RES* pRes = NULL;
	do
	{
		pRes = mysql_store_result(_mysql);
		if(pRes)
		{
			mysql_free_result(pRes);
		}
	}
	while (!mysql_next_result(_mysql));

	XCP_LOGGER_INFO(&g_logger, "excute sql command success, last insert id:%d, affected rows:%d.\n",
		last_insert_id, affected_rows);

	return 0;

}

unsigned long mysql_conn::escape_string(char *to, const char *from, unsigned long length)
{
	/*
	unsigned long mysql_real_escape_string(MYSQL *mysql, char *to, const char *from, unsigned long length)
	The string pointed to by from must be length bytes long. You must allocate the to buffer to be at least length*2+1 bytes long.
	返回值: The length of the value placed into to, not including the terminating null character.
	*/
	return mysql_real_escape_string(_mysql, to, from, length);
}




void mysql_conn::release_conn()
{
	if(_conn)
	{
		//mysql_close 是关闭本端，m_pMySQL无效，对无效的m_pMySQL 操作都会crash
		mysql_close(_mysql);
		_conn = false;
	}
	else
	{
		XCP_LOGGER_INFO(&g_logger, "the mysql isn't connect, can't release it now.\n");
	}

}



//通过ping 对mysql 长连接进行检查
bool mysql_conn::ping()
{
	/*
	mysql_ping 处理的是mysql server 检测到某个连接由于长期没有使用，
	mysql server 会在超时后自动关闭该连接，使用mysql_ping能够
	检测mysql server 端已经关闭， 自动使用原来的连接参数重新连接。
	如果mysql_close 后再调用 mysql_ping 会crash
	*/
	int nRet = mysql_ping(_mysql);
	if(nRet != 0)
	{
		XCP_LOGGER_INFO(&g_logger, "mysql server isn't connectd, mysql_ping failed, ip:%s, port:%d, user:%s, pwd:%s, "
			"db:%s, chars:%s, err code:%d, err msg%s\n",
			 _ip.c_str(), _port, _user.c_str(), _pwd.c_str(),
			 _db.c_str(), _chars.c_str(), mysql_errno(_mysql),  mysql_error(_mysql));

		_conn = false;

		//如果mysql 连接失败了， 之前已经使用的conn 将被设置为没在使用状态
		if(_used)
		{
			_used = false;
		}
	}
	else
	{
		_conn = true;
	}

	return _conn;

}



int mysql_conn::autocommit(bool open)
{
	int nRet = 0;

	if(!_conn)
	{
		XCP_LOGGER_INFO(&g_logger, "the mysql is not connected.\n");
		return -1;
	}

	//1表示启动autocommit模式；0表示禁止autocommit模式
	nRet = mysql_autocommit(_mysql, (open? 1: 0));
	if (0 != nRet)
	{
		XCP_LOGGER_INFO(&g_logger, "mysql_autocommit failed, ret:%d, err code:%d, err msg%s\n", nRet, mysql_errno(_mysql), mysql_error(_mysql));
	}

	return nRet;

}



int mysql_conn::commit()
{
	int nRet = 0;

	if(!_conn)
	{
		XCP_LOGGER_INFO(&g_logger, "the mysql is not connected.\n");
		return -1;
	}

	nRet = mysql_commit(_mysql);
	if (0 != nRet)
	{
		XCP_LOGGER_INFO(&g_logger, "mysql_commit failed, ret:%d, err code:%d, err msg%s\n", nRet, mysql_errno(_mysql), mysql_error(_mysql));
	}

	return nRet;

}


int mysql_conn::rollback()
{
	int nRet = 0;

	if(!_conn)
	{
		XCP_LOGGER_INFO(&g_logger, "the mysql is not connected.\n");
		return -1;
	}

	nRet = mysql_rollback(_mysql);
	if (0 != nRet)
	{
		XCP_LOGGER_INFO(&g_logger, "mysql_rollback failed, ret:%d, err code:%d, err msg%s\n", nRet, mysql_errno(_mysql), mysql_error(_mysql));
	}

	return nRet;
}

