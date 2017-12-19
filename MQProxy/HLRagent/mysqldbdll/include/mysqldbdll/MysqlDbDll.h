/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SmcDiamUa.h
* Author:       luhaiyan
* Date：        08-27-2014
* OverView:     SmcDiamUa接口头文件
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
	#ifdef MYSQLDBDLL_EXPORTS
	#define MYSQLDBDLL_API __declspec(dllexport)
	#else
	#define MYSQLDBDLL_API __declspec(dllimport)
	#endif
#else
	#define MYSQLDBDLL_API
#endif

