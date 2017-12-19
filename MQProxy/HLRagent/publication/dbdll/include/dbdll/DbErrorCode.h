#pragma once
#include "errorcode.h"

typedef enum
{
	ERROR_CODE_DB_SUCCESS=0,
	ERROR_CODE_DB_BEGIN = 1400,
	ERROR_CODE_DB_NULL_PTR,						// 空指针错误
	ERROR_CODE_DB_CONN_FAIL,					// 数据库连接错误
	ERROR_CODE_DB_CONN_POOL_IS_FULL,	// 连接池满
	/*ERROR_CODE_DB_DECODE_ERROR,				//3 解码错误
	ERROR_CODE_DB_TRANSFER_PROCESS_NULLPTR,	//4 传输层没有初始化处理类
	ERROR_CODE_DB_TIME_OUT,	//超时	*/
	ERROR_CODE_DB_END,
	ERROR_CODE_DB_NO_DATA_EFFECT,        //查询结果为空
	ERROR_CODE_DB_EXECUTE_QUERY,			 //数据库执行出错
    ERROR_CODE_DB_STMENT_ERROR
}DB_ERROR_CODE_DEF;
