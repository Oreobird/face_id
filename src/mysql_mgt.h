
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


#ifndef _MYSQL_MGT_H
#define _MYSQL_MGT_H

#include "base_thread_mutex.h"
#include "base_singleton_t.h"
#include "mysql_conn.h"


USING_NS_BASE;


//mysql ������
class mysql_mgt
{
public:
    mysql_mgt();
    virtual ~mysql_mgt();

	//��ʼ��
	int init(const std::string &ip, unsigned int port,
		const std::string &user, const std::string &pwd,
		const std::string &db, const unsigned int cnt, const std::string &chars = "");

	bool get_conn(mysql_conn_Ptr &conn);

	//���mysql״̬���Ұ�������Ҫ�󴴽����ݱ�
	void check();

	void release(mysql_conn_Ptr &conn);

private:
	//���ݿ����ӳ�
	std::vector<mysql_conn_Ptr> _conn_queue;
	Thread_Mutex _mutex;

};

#define PSGT_Mysql_Mgt Singleton_T<mysql_mgt>::getInstance()




//----------------------------
//�����࣬ ��֧�ֿ�������
class MySQL_Guard : public noncopyable
{
public:

	//����MySQL_Guard ����ֻ��ʹ���������ַ�ʽ
	MySQL_Guard(mysql_conn_Ptr &conn);

	~MySQL_Guard();

	mysql_conn_Ptr& operator-> ();

private:
	mysql_conn_Ptr _conn;

};



#endif

