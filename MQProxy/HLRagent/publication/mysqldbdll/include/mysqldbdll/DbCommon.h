/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     DbCommon.h
* Author:       luhaiyan
* Date��        08-30-2014
* OverView:     eHssS6aBiz�ӿ�ͷ�ļ�
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*******************************************************************************/
#ifndef __DB_COMMON_API_H__
#define __DB_COMMON_API_H__


#include "SPRTraceStruct.h"
#include "xosshell.h"
#include "MysqlDbDll.h"
#ifdef __cplusplus
extern "C" {
#endif


extern MYSQLDBDLL_API  callbackTraceFun g_callbackTraceFunOfMySqlDb;
MYSQLDBDLL_API XS32 RegistMySqlDbCallbackTraceFun(callbackTraceFun  traceFun);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  __DB_COMMON_API_H__