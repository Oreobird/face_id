#include "req_mgt.h"
#include "base_logger.h"

extern Logger g_logger;
extern StSysInfo g_sysInfo;

Req_Mgt::Req_Mgt()
{
	_queue = new X_Queue<Request_Ptr>(g_sysInfo._max_queue_size);
}


Req_Mgt::~Req_Mgt()
{


}

int Req_Mgt::push_req(Request_Ptr req)
{
	return _queue->push(req);
}


int Req_Mgt::get_req(Request_Ptr &req)
{
	return _queue->pop(req);
}


bool Req_Mgt::full()
{
	return _queue->full();
}


bool Req_Mgt::full(unsigned int &used_size, unsigned int &max_size)
{
	return _queue->full(used_size, max_size);
}
