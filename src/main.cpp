#include "base_common.h"
#include "base_args_parser.h"
#include "base_reactor.h"
#include "base_signal.h"
#include "base_logger.h"
#include "base_time.h"
#include "base_os.h"
#include "base_utility.h"

#include "req_mgt.h"
#include "face_handler_mgt.h"
#include "conf_mgt.h"
#include "req_processor.h"
#include "req_tcp_event_handler.h"
#include "user_mgt.h"

USING_NS_BASE;
using namespace cv;

extern Logger g_logger;
extern StSysInfo g_sysInfo;
extern StModelInfo g_modelInfo;

Reactor *g_reactor_req = NULL;
extern Req_Mgt *g_req_mgt;
extern Face_Handler_Mgt *g_face_handler_mgt;
extern User_Mgt *g_user_mgt;

const std::string g_version = "0.1.0";

void usage()
{
    fprintf(stderr,
        "Usage: ./srv -s cfg_file -m model_dir -l log_dir\n\n"
        "  -s  config file\n\n"
        "  -m  model dir\n\n"
        "  -d  data dir\n\n"
        "  -l  log_dir\n\n"
        "  -h  help info\n\n"
        "  version: %s, build time: %s %s\n"
        "\n",
        g_version.c_str(), __DATE__, __TIME__
    );
};



int main(int argc, char * argv[])
{

	int nRet = 0;

    if (argc < 3)
	{
        usage();
        return 0;
    }

	set_random_seed();

	sigset_t sigset;
	add_signal_in_set(sigset, 2, SIGPIPE, SIGUSR2);
	nRet = block_process_signal(sigset);
	if(nRet != 0)
	{
		printf("block_process_signal failed, ret:%d\n", nRet);
		return 0;
	}

	printf("--- prepare to start parse arg ---\n");
	Args_Parser args_parser;
	args_parser.parse_args(argc, argv);
	args_parser.show();

	std::string cfg = "";
	if(!args_parser.get_opt("s", &cfg))
	{
		printf("get_opt cfg failed!\n");
		return 0;
	}

	printf("=== complete to start parse arg ===\n");

	printf("--- prepare to init conf mgt ---\n");
	nRet = PSGT_Conf_Mgt->init(cfg);
	if(nRet != 0)
	{
		printf("init conf mgt failed, ret:%d\n", nRet);
		return 0;
	}

	g_sysInfo = PSGT_Conf_Mgt->get_sysinfo();
	printf("===== complete to init conf mgt =====\n");

	nRet = pid_file(g_sysInfo._log_id + std::string(".pid"));
	if(nRet != 0)
	{
		printf("pid file failed, ret:%d\n", nRet);
		return nRet;
	}

	if(g_sysInfo._TZ != "")
	{
		char tz[100] = {0};
		snprintf(tz, 100, "TZ=%s", (g_sysInfo._TZ).c_str());
		nRet = putenv(tz);
		if(nRet != 0)
		{
			printf("putenv(%s) failed, ret:%d\n", tz, nRet);
			return nRet;
		}
		else
		{
			printf("putenv(%s) success\n", tz);
		}
	}
	tzset();
	printf("date:%s\n", FormatDateTimeStr().c_str());

	std::string log_dir = "./../log/";
	if(!args_parser.get_opt("l", &log_dir))
	{
		printf("get_opt log_dir failed, use default log_dir\n");
	}

	nRet = g_logger.init(log_dir, g_sysInfo._log_id, MAX_LOG_SIZE, 3600);
	if(nRet != 0)
	{
		printf("init debug logger failed, ret:%d\n", nRet);
		return nRet;
	}

	XCP_LOGGER_INFO(&g_logger, "=============================================\n");
	XCP_LOGGER_INFO(&g_logger, "        date:%s\n", FormatDateTimeStr().c_str());
	XCP_LOGGER_INFO(&g_logger, "        process:%d, thread:%llu \n", get_pid(), get_thread_id());
	XCP_LOGGER_INFO(&g_logger, "        prepare to start Access Svr! ...\n");
	XCP_LOGGER_INFO(&g_logger, "=============================================\n");

	printf("--- prepare to init user mgt ---\n");
	g_user_mgt = new User_Mgt;

	nRet = g_user_mgt->init();
	if (nRet != 0)
	{
		printf("init user mgt failed, ret:%d\n", nRet);
		return 0;
	}

	printf("===== complete to init user mgt =====\n");

	printf("--- prepare to init face handler mgt ---\n");
	g_modelInfo = PSGT_Conf_Mgt->get_modelinfo();
	std::string model_dir = "./../model/";
	if(!args_parser.get_opt("m", &model_dir))
	{
		printf("get_opt model_dir failed, use default model_dir\n");
	}

	std::string data_dir = "./../data/";
	if (!args_parser.get_opt("d", &data_dir))
	{
		printf("get_opt data_dir failed, use default data_dir\n");
	}
	g_face_handler_mgt = new Face_Handler_Mgt;

	nRet = g_face_handler_mgt->init(model_dir, data_dir);
	if (nRet != 0)
	{
		printf("init face handler mgt failed, ret:%d\n", nRet);
		return 0;
	}

	printf("===== complete to init face handler mgt =====\n");

	//(7) 启动Req 工作线程池
	g_req_mgt = PSGT_Req_Mgt;//new Req_Mgt;
	
	XCP_LOGGER_INFO(&g_logger, "--- prepare to start req processor ---\n");
	Req_Processor req_processor;

	nRet = req_processor.init(NULL, g_sysInfo._thr_num);
	if(nRet != 0)
	{
		printf("init req processor failed, ret:%d\n", nRet);
		return nRet;
	}

	nRet = req_processor.run();
	if(nRet != 0)
	{
		printf("run req processor failed, ret:%d\n", nRet);
		return nRet;
	}
	XCP_LOGGER_INFO(&g_logger, "=== complete to start req processor ===\n");


	//(9) 启动req tcp reactor
	XCP_LOGGER_INFO(&g_logger, "--- prepare to start req reactor(tcp) ---\n");
	Req_TCP_Event_Handler *req_handler = new Req_TCP_Event_Handler;
	StReactorAgrs args_req;
	args_req.port = g_sysInfo._port;
	g_reactor_req = new Reactor(req_handler);
	nRet = g_reactor_req->init(&args_req);
	if(nRet != 0)
	{
		printf("init req reactor(tcp) failed, ret:%d\n", nRet);
		return nRet;
	}

	nRet = g_reactor_req->run();
	if(nRet != 0)
	{
		printf("req reactor(tcp) run failed, ret:%d\n", nRet);
		return nRet;
	}
	XCP_LOGGER_INFO(&g_logger, "=== complete to start req reactor(tcp) ===\n");


	//(11) 完成face id svr  的启动
	XCP_LOGGER_INFO(&g_logger, "===== complete to start face id Svr! =====\n");

	while(true)
	{
		::sleep(5);
	}

	return 0;

}


