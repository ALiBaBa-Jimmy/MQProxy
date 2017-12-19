#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#include "xostype.h"
#include "xosshell.h"

#pragma pack(1)

typedef struct _STaskContext
{
	XU16 taskIndex;   //task的索引
	XU16 taskcount;  //总的task个数
	XU32 startFid;   //此任务开始的fid 主要是为了不在任务内在关系FID的定义
	XBOOL  g_bBeAttached;
	XVOID* taskFidList;
	XU32  taskTid;
	XU32  taskFid;
	XU32  dbTaskId;
	XU32  messageCount;//当前处理的个数
}STaskContext;

typedef struct _STaskContextAry
{
	XU32 taskcount;//任务总共的线程数	
	XU32 endFid;
	XU32 startFid;
	XU32 dispatchFid; //当前负载的Fid值，使用与简单的负载，收到一个消息在这个任务所有的线程中依次发送
	//每个线程的上下文信息
	STaskContext *pTaskContext;
}STaskContextAry;

#pragma pack()

#ifdef __cplusplus
}
#endif 

