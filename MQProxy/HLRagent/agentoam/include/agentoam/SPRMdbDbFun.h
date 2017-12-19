/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdbPviPuiRelation.h
* Author:       luhaiyan
* Date：        09-29-2014
* OverView:     PUI和PVI的关系
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
#ifndef __SPR_MDB_DB_FUNCTION_H__
#define __SPR_MDB_DB_FUNCTION_H__
#include "agentinclude.h"
#include "SPRMdb.h"



// 用于MEMDB通知要进行更新内存数据库到物理数据库的函数
typedef XS32 (*callbackNotifyUpdateMdbToHdbFun) (XU32 srcFid, XU32 msgId);
extern callbackNotifyUpdateMdbToHdbFun g_callbackNotifyUpdateMdbToHdbFun;


/**********************************************************************
*
*  NAME:          RegistMDbCallbackTraceFun
*  FUNTION:       注册trace函数，主要为了解决动态库log输出不了的问题,
*  INPUT:         无
*  OUTPUT:        
*  OTHERS:        其他需说明的问题
**********************************************************************/
AGNETMDB_API  XS32 RegistMDbCallbackTraceFun(callbackTraceFun  traceFun);







#endif

