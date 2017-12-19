/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHardDb.h
* Author:       luhaiyan
* Date：        08-27-2014
* OverView:     
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


#ifdef WIN32
	#ifdef SPRHARDDB_EXPORTS
	#define  AGENTHARDDB_API __declspec(dllexport)
	#else
	#define  AGENTHARDDB_API __declspec(dllimport)
	#endif
#else
	#define AGENTHARDDB_API
#endif

