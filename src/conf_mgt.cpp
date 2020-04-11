#include "base_os.h"
#include "base_convert.h"
#include "base_string.h"
#include "base_xml_parser.h"
#include "base_logger.h"
#include "conf_mgt.h"
#include "mysql_mgt.h"
#include "common.h"

using namespace std;

int g_mode;
Logger g_logger;
StSysInfo g_sysInfo;
StModelInfo g_modelInfo;
extern mysql_mgt g_mysql_mgt;


Conf_Mgt::Conf_Mgt(): _cfg("")
{
}


Conf_Mgt::~Conf_Mgt()
{

}


int Conf_Mgt::init(const std::string &cfg)
{
	int nRet = 0;

	_cfg = cfg;
	nRet = refresh();
	if(nRet != 0)
	{
		XCP_LOGGER_INFO(&g_logger, "refresh conf mgt falied, ret:%d\n", nRet);
		return nRet;
	}

	//启动conf mgt timer
// 	XCP_LOGGER_INFO(&g_logger, "--- prepare to start conf mgt timer ---\n");
// 	Select_Timer *timer_conf = new Select_Timer;
// 	Conf_Timer_handler *conf_thandler = new Conf_Timer_handler;
// 	nRet = timer_conf->register_timer_handler(conf_thandler, 10000000);
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_INFO(&g_logger, "register conf mgt timer handler falied, ret:%d\n", nRet);
// 		return nRet;
// 	}
//
// 	nRet = timer_conf->init();
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_ERROR(&g_logger, "int conf mgt timer failed, ret:%d\n", nRet);
// 		return nRet;
// 	}
//
// 	nRet = timer_conf->run();
// 	if(nRet != 0)
// 	{
// 		XCP_LOGGER_ERROR(&g_logger, "conf mgt timer run failed, ret:%d\n", nRet);
// 		return nRet;
// 	}
// 	XCP_LOGGER_INFO(&g_logger, "=== complete to start conf mgt timer ===\n");

	//mysql write
	nRet = g_mysql_mgt.init(_mysqls["write"]._ip, _mysqls["write"]._port, _mysqls["write"]._user, _mysqls["write"]._pwd, _mysqls["write"]._db,
		_mysqls["write"]._num, _mysqls["write"]._chars);
	if(nRet != 0)
	{
		XCP_LOGGER_ERROR(&g_logger, "init mysql write failed, ret:%d\n", nRet);
		return nRet;
	}
	else
	{
		printf("init mysql_mgt success!\n");
	}


	return nRet;

}


int Conf_Mgt::refresh()
{
	int nRet = 0;

	XML_Parser _parser;
	nRet = _parser.parse_file(_cfg);
	if(nRet != 0)
	{
		printf("init conf mgt failed, ret:%d, cfg:%s\n", nRet, _cfg.c_str());
		XCP_LOGGER_INFO(&g_logger, "init conf mgt failed, ret:%d, cfg:%s\n", nRet, _cfg.c_str());
		return nRet;
	}

	//---------------------- sysinfo ---------------------

	StSysInfo sysInfo;

	//id
	XML_Node node;
	nRet = _parser.get_node("face_id/system/id", node);
	if(nRet != 0)
	{
		printf("get id failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get id failed, ret:%d\n", nRet);
		return -1;
	}
	sysInfo._id = node.get_text();
	trim(sysInfo._id);
	if(sysInfo._id == "")
	{
		printf("id is empty\n");
		XCP_LOGGER_INFO(&g_logger, "id is empty\n");
		return -1;
	}

	//ip
	nRet = _parser.get_node("face_id/system/ip", node);
	if(nRet != 0)
	{
		printf("get ip failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get ip failed, ret:%d\n", nRet);
		return -1;
	}
	sysInfo._ip = node.get_text();
	trim(sysInfo._ip);
	if(sysInfo._ip == "")
	{
		printf("ip is empty\n");
		XCP_LOGGER_INFO(&g_logger, "ip is empty\n");
		return -1;
	}


	//ip_out
	nRet = _parser.get_node("face_id/system/ip_out", node);
	if(nRet != 0)
	{
		printf("get ip_out failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get ip_out failed, ret:%d\n", nRet);
		sysInfo._ip_out = sysInfo._ip;
	}
	else
	{
		sysInfo._ip_out = node.get_text();
		trim(sysInfo._ip_out);
		if(sysInfo._ip_out == "")
		{
			printf("ip_out is empty\n");
			XCP_LOGGER_INFO(&g_logger, "ip_out is empty\n");
			return -1;
		}
	}


	//port
	nRet = _parser.get_node("face_id/system/port", node);
	if(nRet != 0)
	{
		printf("get port failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get port failed, ret:%d\n", nRet);
		return -1;
	}
	else
	{
		sysInfo._port = (unsigned short)atoll(node.get_text().c_str());
		if(sysInfo._port == 0)
		{
			printf("port is 0\n");
			XCP_LOGGER_INFO(&g_logger, "port is 0\n");
			return -1;
		}
	}

	//Message ID命名规则： [svr id]_[ip_out]_[port]
	sysInfo._log_id = base::format("%s_%s_%u", sysInfo._id.c_str(), sysInfo._ip_out.c_str(), sysInfo._port);
	sysInfo._new_id = base::format("%s_%llu_%d", sysInfo._log_id.c_str(), getTimestamp(), get_pid());

	//thr_num
	nRet = _parser.get_node("face_id/system/thr_num", node);
	if(nRet != 0)
	{
		printf("get thr_num failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get thr_num failed, ret:%d\n", nRet);
		get_cpu_number_proc(sysInfo._thr_num);
	}
	else
	{
		sysInfo._thr_num = (unsigned short)atoll(node.get_text().c_str());
		if(sysInfo._thr_num == 0)
		{
			printf("thr_num is 0\n");
			XCP_LOGGER_INFO(&g_logger, "thr_num is 0\n");
			return -1;
		}
	}


	//max_queue_size
	nRet = _parser.get_node("face_id/system/max_queue_size", node);
	if(nRet != 0)
	{
		printf("get max_queue_size failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get max_queue_size failed, ret:%d\n", nRet);
	}
	sysInfo._max_queue_size = (unsigned int)atoll(node.get_text().c_str());
	if(sysInfo._max_queue_size < 100000)
	{
		printf("max_queue_size < 100000\n");
		XCP_LOGGER_INFO(&g_logger, "max_queue_size < 100000\n");
		sysInfo._max_queue_size = 100000;
	}

	//TZ
	nRet = _parser.get_node("face_id/system/TZ", node);
	if(nRet != 0)
	{
		printf("get TZ failed, ret:%d\n", nRet);
	}
	else
	{
		sysInfo._TZ = node.get_text();
	}

	if (true)
	{
		Thread_Mutex_Guard guard(_mutex);
		_sysInfo = sysInfo;
	}

	//------------------  model  ------------------
	StModelInfo modelInfo;

	nRet = _parser.get_node("face_id/model/recognizer", node);
	if (nRet != 0)
	{
		printf("get insightface model name failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get insightface model name failed, ret:%d\n", nRet);
	}
	else
	{
		modelInfo._recognizer = node.get_text();
	}

	if (true)
	{
		Thread_Mutex_Guard guard(_mutex);
		_modelInfo = modelInfo;
	}

	//------------------  mysql  ------------------
	std::map<std::string, StMysql_Access> mysqls;
	std::vector<XML_Node> vecNode;

	vecNode.clear();
	nRet = _parser.get_nodes("face_id/mysql/svr", vecNode);
	if(nRet != 0)
	{
		printf("get_nodes admin_svr/mysql/svr failed, ret:%d\n", nRet);
		XCP_LOGGER_INFO(&g_logger, "get mysql svr failed\n");
		return -1;
	}

	if(vecNode.empty())
	{
		printf("mysql/svr is empty\n");
		XCP_LOGGER_INFO(&g_logger, "mysql/svr is empty\n");
		return -1;
	}

	for(unsigned int i=0; i<vecNode.size(); ++i)
	{
		StMysql_Access stMysql_Access;

		std::string mode = "";
		mode = vecNode[i].get_attr_str("mode");
		trim(mode);
		if(mode != "write")
		{
			printf("mysql svr mode is invalid, mode:%s\n", mode.c_str());
			return -1;
		}

		stMysql_Access._ip = vecNode[i].get_attr_str("ip");
		trim(stMysql_Access._ip);
		if(stMysql_Access._ip == "")
		{
			printf("mysql svr ip is empty\n");
			return -1;
		}

		stMysql_Access._port = (unsigned short)atoll(vecNode[i].get_attr_str("port").c_str());
		if(stMysql_Access._port == 0)
		{
			printf("mysql svr port is 0\n");
			return -1;
		}

		stMysql_Access._db = vecNode[i].get_attr_str("db");
		if(stMysql_Access._db == "")
		{
			printf("mysql svr db is empty\n");
			return -1;
		}

		stMysql_Access._user = vecNode[i].get_attr_str("user");
		if(stMysql_Access._user == "")
		{
			printf("mysql svr user is empty\n");
			return -1;
		}

		stMysql_Access._pwd = vecNode[i].get_attr_str("pwd");
		if(stMysql_Access._pwd == "")
		{
			printf("mysql svr pwd is empty\n");
			return -1;
		}

		stMysql_Access._chars = vecNode[i].get_attr_str("chars");
		if(stMysql_Access._chars == "")
		{
			printf("mysql svr chars is empty\n");
			return -1;
		}

		/**stMysql_Access._num = (unsigned int)atoll(vecNode[i].get_attr_str("num").c_str());
		if(stMysql_Access._num == 0)
		{
			printf("mysql svr num is 0\n");
			return -1;
		}**/
		//连接池的长度= 线程数+3
		stMysql_Access._num = sysInfo._thr_num + EXTRA_CONNCTION_POOL_NUM;
		mysqls[mode] = stMysql_Access;
	}

	if (mysqls.size() == 0)
	{
		printf("the size of mysql svr is 0\n");
		return -1;
	}
	else
	{
		Thread_Mutex_Guard guard(_mutex);
		_mysqls = mysqls;
	}

	return 0;
}


//获取副本
StSysInfo Conf_Mgt::get_sysinfo()
{
	Thread_Mutex_Guard guard(_mutex);
	return _sysInfo;
}

StModelInfo Conf_Mgt::get_modelinfo()
{
	Thread_Mutex_Guard guard(_mutex);
	return _modelInfo;
}

