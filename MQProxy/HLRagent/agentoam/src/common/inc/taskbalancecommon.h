/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:    taskbalancecommon.h
* Author:       luhaiyan
* Date:			2015-01-17只能给C++调用
* OverView:     eHSS的公共函数，需要包含的各个项目中进行编译
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#pragma  once

#include "xostype.h"
#include "xosshell.h"
#include "taskcommon.h"
#include <map>
#include <string>

using namespace std;
#pragma pack(1)

typedef struct _SBalanceTaskContextAry
{
	XU32 taskcount;//任务总共的线程数	
	XU32 endFid;
	XU32 startFid;
	t_XOSMUTEXID lockDispatchFid;
	//负载的fid信息
	map<string, XU32 > mapDispatchFid;
	//每个线程的上下文信息
	STaskContext *pTaskContext;
}SBalanceTaskContextAry;



#pragma pack()

