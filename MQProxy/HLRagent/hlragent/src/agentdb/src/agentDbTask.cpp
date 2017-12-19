/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbTask.cpp
* Author:       luhaiyan
* Date��        10-15-2014
* OverView:     ���ݿ�Ĵ���
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


#include "agentinclude.h"
#include "agentDbTask.h"
#include "SPRHdbOam.h"
#include "agentTrace.h"
#include "DbFactory.h"
#include "SPRMAndHDbComFun.h"
#include "SPRCommFun.h"
#include "SPRMdbDbFun.h"
#include "clishell.h"
#include "MqttComFun.h"
#include "agentDB_API.h"
#include "DbCommon.h"






/**************************************************************************
�� �� ��: eHssDb_InitProc
��������: ���ݿ��̳߳�ʼ������
��    ��:
�� �� ֵ: XSUCC �ɹ�  XERROR ʧ��
**************************************************************************/
XS8 agentDB_InitProc(XVOID *pPara1, XVOID *Para2)
{	
	XU32 fid = FID_AGENTDB;
	//RegUserDataCliCmd();
	RegistHDBMySqlDbCallbackTraceFun(agentTrace);
	RegistHDbCallbackTraceFun(agentTrace);
	RegistMDbCallbackTraceFun(agentTrace);
	RegistDbCallbackTraceFun(agentTrace);

	if(XSUCC !=DbInitData(fid))
	{
		XOS_Trace(MD(fid, PL_EXP),"agent Connect to Oracle Db failed.");			
		return XERROR;
	}
    else
    {
    
        /*�������ݿ��еĶ�����Ϣ*/    
        //agentDB_SubQryAll(fid);
    }
	

    return XSUCC;
}

/**************************************************************************
�� �� ��: SPRDb_XosMsgProc
��������: ���ݿ��߳���Ϣ������
��    ��:
�� �� ֵ:
**************************************************************************/
XS8 agentDB_XosMsgProc(XVOID *msg, XVOID *pPara)
{


	


    return XSUCC;
}

/**************************************************************************
�� �� ��: SPRDb_TimeoutProc
��������: ������XOSƽ̨ע�ᳬʱ�ص��ӿ�,��ʱ����Ҫ���д���
��    ��:
�� �� ֵ: XSUCC
**************************************************************************/
XS8 agentDB_TimeoutProc(t_BACKPARA  *pstPara)
{


	
    return XSUCC;
}

/**************************************************************************
�� �� ��: SPRDb_NoticeProc
��������: ������XOSƽ̨ע��֪ͨ�����ص��ӿ�
��    ��:
�� �� ֵ: XSUCC
**************************************************************************/
XS8 agentDB_NoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{
	return XSUCC;
}


/***************************************************************
������XdbTimerStartSynDyn
���ܣ���ʼ��������ͬ����̬������Ϣ��ʱ��
���룺��
�������
���أ�XSUCC/XERROR                    
***************************************************************/
XS32 XdbTimerStartSynDyn(XU32 fid)
{


	return 0;
}

/***************************************************************
������XdbTimerStartInitDb
���ܣ���ʼ���������ݿⶨʱ��
���룺��
�������
���أ�XSUCC/XERROR                    
***************************************************************/
XS32 XdbTimerStartInitDb(XU32 fid)
{


	return 0;
}



/***************************************************************
������GetDbCfgInfo
���ܣ������Ƿ�֧���������ñ�ʾ���������ļ�����oam�ڴ��ж�ȡ���ݿ�������Ϣ
���룺SDbConfigInfo *pDbCfgInfo���������ݿ�������Ϣ
�������
���أ�XSUCC/XERROR                    
***************************************************************/
XS32 GetDbCfgInfo(SDbConfigInfo *pDbCfgInfo)
{
	XS32 ret = XERROR;
	//�������ļ��ж�ȡ���ݿ�������Ϣ
	ret = MYSQL_GetDbInitsCfg(FID_AGENTDB, pDbCfgInfo, "./dbcfg.xml");
	if (ret != XSUCC)
	{
		XOS_Trace(MD(FID_AGENTDB,PL_ERR),"MYSQL_GetDbInitsCfg error.");
		return XERROR;
	}

	
	return XSUCC;
}

/***************************************************************
������DbInitData
���ܣ���ʼ���������ݿ����еĳ�ʼ�����ݲ���
���룺��
�������
���أ�XSUCC/XERROR , ֻҪû�г�ʼ���ɹ���Ҫ������ʼ�����Ӷ�ʱ��                   
***************************************************************/
XS32 DbInitData(XU32 fid)
{


	if(XSUCC != DbInitDataHandler(fid))
	{
	    XOS_Trace(MD(FID_AGENTDB,PL_ERR),"DbInitDataHandler error.");
        return XERROR;

	}


	return XSUCC;
}

/***************************************************************
������DbInitDataHandler
���ܣ���ʼ���������ݿ����еĳ�ʼ�����ݲ���,��DbInitData���ã�DbInitData�������ط�����
���룺��
�������
���أ�XSUCC/XERROR , ����ֻ�ǳ������ش������������ DbInitData������ʱ��                  
***************************************************************/
XS32 DbInitDataHandler(XU32 fid)
{
	XS32 nRet = XERROR;
	SDbConfigInfo szDbCfgInfo = {0};
	nRet = GetDbCfgInfo(&szDbCfgInfo);
	if (nRet != XSUCC)
	{
        XOS_Trace(MD(fid,PL_ERR),"Get Db Configure Error.");
		return XERROR;
	}

    if(0 == XOS_MemCmp(szDbCfgInfo.ip, "0.0.0.0", XOS_StrLen("0.0.0.0")))
    {
        XOS_Trace(MD(fid,PL_LOG)," db.ip = 0.0.0.0 invalid");
        return XSUCC;
    }
	nRet = InitDb(&szDbCfgInfo);
	if(XSUCC != nRet)
	{
        XOS_Trace(MD(fid,PL_ERR),"init Db connection Error.");
		return XERROR;
		
	}

   
	return XSUCC;
	
}



