#ifndef __COMMON_H__
#define __COMMON_H__


#ifdef DEBUG
#define dbg(fmt, args...)  printf(fmt, ##args)
#else
#define dbg(fmt, args...) do{}while(0)
#endif

enum EnErrCode
{
	ERR_SUCCESS = 0,
	ERR_SYSTEM = -1,

	//Common
	ERR_COMMON_BEGIN 				         = -8000,
	ERR_QUEUE_FULL 					         = -8001,    //队列满
	ERR_PUSH_QUEUE_FAIL 			         = -8002,    //插入队列失败
	ERR_INVALID_REQ 				         = -8003,    //请求串格式非法
	ERR_REACH_MAX_MSG 				         = -8004,    //请求串大于最大长度
	ERR_REACH_MIN_MSG 				         = -8005,    //请求串小于最小长度
	ERR_SEND_FAIL 					         = -8006,    //发送失败
	ERR_RCV_FAIL 					         = -8007,    //接收失败
	ERR_BASE64_ENCODE_FAILED		         = -8018,    //BASE64 编码失败
	ERR_BASE64_DECODE_FAILED		         = -8019,    //BASE64 解码失败
    ERR_GET_MYSQL_CONN_FAILED                = -8010,    //获取mysql 链接失败
    ERR_MYSQL_OPER_EXCEPTION                 = -8011,    //MYSQL操作异常
    ERR_DUPLICATE_KEY                        = -8012,    //重复键
    ERR_TABLE_EXISTS                         = -8013,    //数据表存在

	ERR_NO_FACE_DETECT                       = -9001,   //目标检测失败
};

const unsigned int EXTRA_CONNCTION_POOL_NUM = 3;      //每个存储链接在处理线程池的基础上额外多加3个链接


#endif
