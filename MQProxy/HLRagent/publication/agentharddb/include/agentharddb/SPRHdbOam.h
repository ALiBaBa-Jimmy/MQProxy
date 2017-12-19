/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHdbOam.h
* Author:       luhaiyan
* Date:        10-04-2014
* OverView:     ���ݿ�ģ���������Ϣ
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
#ifndef __SPR_HDB_OAM_H__
#define __SPR_HDB_OAM_H__

#include "agentHardDb.h"
#include "agentinclude.h"
#include "Getmysqldbcfg.h"

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����,
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
AGENTHARDDB_API  XS32 RegistHDbCallbackTraceFun(callbackTraceFun  traceFun);

/**********************************************************************
*
*  NAME:          InitDb
*  FUNTION:       ��ʼ��DB
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
AGENTHARDDB_API  XS32 InitDb(SDbConfigInfo *pDbCfgInfo);
//AGENTHARDDB_API XS32 InitAliasDb(SDbConfigInfo *pDbCfgInfo);

/**********************************************************************
*
*  NAME:          SetDbIp
*  FUNTION:       ����DBIP
*  INPUT:         XU8* pDbIP
*  OUTPUT:        �ɹ�ʧ��
*  OTHERS:        ������˵��������
**********************************************************************/


AGENTHARDDB_API  XBOOL GetDbLinkStatus();

AGENTHARDDB_API  XS32 DestroyDb();
/**********************************************************************
*
*  NAME:          RegistHDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����,
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
AGENTHARDDB_API  XS32 RegistHDBMySqlDbCallbackTraceFun(callbackTraceFun  traceFun);


#ifdef __cplusplus
}
#endif 

#endif


