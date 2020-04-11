
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


#ifndef _CONF_MGT_H
#define _CONF_MGT_H

#include "base_common.h"
#include "base_singleton_t.h"
#include "base_thread_mutex.h"
#include "base_rw_thread_mutex.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "common.h"
#include "timing.h"
#include "codecs.h"

//typedef unsigned long long uint64;
USING_NS_BASE;
using namespace std;
using namespace cv;

//系统配置结构体
typedef struct StSysInfo
{
	std::string _id;                             //原始ID
	std::string _new_id;                         //新ID
	std::string _log_id;                         //日志ID
	unsigned int _pid;                           //进程ID
	std::string _ip;                             //IP地址
	std::string _ip_out;                         //公网IP地址
	unsigned short _port;                        //独立端口
	unsigned short _worker_port;                 //注册端口
	unsigned short _thr_num;                     //工作线程个数
	unsigned int _max_queue_size;                //请求队列最大尺寸
	unsigned int _rate_limit;                    //速率限制
	std::string _TZ;                             //时区

	StSysInfo():_id(""), _new_id(""),_pid(0), _ip(""), _port(0), _worker_port(0),
		_thr_num(0), _max_queue_size(0), _rate_limit(0), _TZ("")
	{
	}

}StSysInfo;

typedef struct StModelInfo
{
    std::string _recognizer;
    StModelInfo():_recognizer("recognizer_model_v1.pb")
    {
    }
} StModelInfo;

//mysql 访问配置信息结构体
typedef struct StMysql_Access
{
	std::string _ip;
	unsigned short _port;
	std::string _db;
	std::string _user;
	std::string _pwd;
	std::string _chars;
	unsigned int _num;

	StMysql_Access(): _ip(""), _port(0), _db(""), _user(""), _pwd(""), _chars(""), _num(1)
	{
	}

} StMysql_Access;

class Conf_Mgt
{
public:
	Conf_Mgt();

	~Conf_Mgt();

	int init(const std::string &cfg);

	int refresh();

	//获取本地配置文件
	StSysInfo get_sysinfo();
    StModelInfo get_modelinfo();

private:
	Thread_Mutex _mutex;
	std::string _cfg;
	StSysInfo _sysInfo;
    StModelInfo _modelInfo;
	std::map<std::string, StMysql_Access> _mysqls;
};

#define PSGT_Conf_Mgt Singleton_T<Conf_Mgt>::getInstance()

#endif


