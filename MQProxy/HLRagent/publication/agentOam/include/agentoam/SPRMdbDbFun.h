/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdbPviPuiRelation.h
* Author:       luhaiyan
* Date��        09-29-2014
* OverView:     PUI��PVI�Ĺ�ϵ
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
#ifndef __SPR_MDB_DB_FUNCTION_H__
#define __SPR_MDB_DB_FUNCTION_H__
#include "agentinclude.h"
#include "SPRMdb.h"



// ����MEMDB֪ͨҪ���и����ڴ����ݿ⵽�������ݿ�ĺ���
typedef XS32 (*callbackNotifyUpdateMdbToHdbFun) (XU32 srcFid, XU32 msgId);
extern callbackNotifyUpdateMdbToHdbFun g_callbackNotifyUpdateMdbToHdbFun;


/**********************************************************************
*
*  NAME:          RegistMDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����,
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
AGNETMDB_API  XS32 RegistMDbCallbackTraceFun(callbackTraceFun  traceFun);







#endif

