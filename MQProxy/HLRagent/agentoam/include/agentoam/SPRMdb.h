/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdb.h
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
	#ifdef AGENTOAM_EXPORTS
	#define  AGNETMDB_API __declspec(dllexport)
	#else
	#define  AGNETMDB_API __declspec(dllimport)
	#endif
#else
	#define AGNETMDB_API
#endif


