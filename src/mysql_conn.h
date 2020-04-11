
/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: 89617663@qq.com
 */


#ifndef _MYSQL_CONN_H
#define _MYSQL_CONN_H

#include "base_smart_ptr_t.h"
#include "mysql.h"
#include "mysqld_error.h"


USING_NS_BASE;


//----------------------- row -----------------------
class MySQL_Row
{
public:
	MySQL_Row();

	~MySQL_Row();

	//???????б?????(?????)
	std::string operator [](int field_id);

	//??巽????????б?????
	template <typename T>
	T get_value(int field_id);

	//??m_Record ???????????
	void operator()(const std::string &value);

private:
	std::vector<std::string> m_Record;
};



//----------------------- row set -----------------------

class MySQL_Row_Set
{
public:
	MySQL_Row_Set();

	~MySQL_Row_Set();

	int field_count();

	int row_count();

	bool empty();

	MySQL_Row operator[](int row_id);

	void operator()(MySQL_Row &row);

	//?????????????????????
	int operator()(std::string field_name);

	void show_all();


	//????3?????MySQL_Conn ???? ????ò?????
	void field_count(int count);

	void row_count(int count);

	void set_field_name(std::string field_name, int index);

private:
	int field_id(std::string field_name);


private:
	int m_row;
	int m_field;
	std::vector<MySQL_Row> m_Record_Set;      //?洢???????????
	std::map<std::string, int> m_Field_Name;  //?洢???????????????

};



//mysql ??????
class mysql_conn : public RefCounter
{
public:
	mysql_conn(int seq);

	~mysql_conn();

	int connect_conn(const std::string &ip, unsigned int port,
		                  const std::string &user, const std::string &pwd,
			              const std::string &db, const std::string &chars = "");


	int query_sql(const std::string sql, MySQL_Row_Set &row_set, std::string &errInfo);

	int execute_sql(const std::string sql, unsigned long long &last_insert_id, unsigned long long &affected_rows, std::string &errInfo);

	//???buffer ????????
	unsigned long escape_string(char *to, const char *from, unsigned long length);

	void release_conn();

	//???ping ??mysql ????????м??
	bool ping();

	//????
	int autocommit(bool open);
	int commit();
	int rollback();

public:
	MYSQL *_mysql;

	std::string  _ip;
	unsigned int _port;
	std::string  _user;
	std::string  _pwd;
	std::string  _db;
	std::string  _chars;

	bool _conn;   //??????????
	bool _used;   //??????????
	int _seq;

};
typedef Smart_Ptr_T<mysql_conn>  mysql_conn_Ptr;

#endif

