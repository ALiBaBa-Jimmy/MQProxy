/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssOamTask.cpp
* Author:       luhaiyan
* Date��        30-08-2014
* OverView:     eHSSOAMģ���������
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
#include "SPROamTask.h"
#include "agentinclude.h"

#include "oam_main.h"
#include "SPROamCli.h"
#include "agentDbTask.h"
#include "SPRCommFun.h"
#include "agentuaOam.h"







/**************************************************************************
�� �� ��: SPROamInitProc
��������: ��ʼ���ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ: XSUCC �ɹ�  XERROR ʧ��
**************************************************************************/
XS8 agent_OamInitProc(XVOID *pPara1, XVOID *Para2)
{
	XS32 ret = 0;
	//ע��oamcli������
	RegAgentOamCliCmd();



    XOS_RegAppVer("HLRagent", "R001B16D31SP05");
    //R001B16D31SP04  ��ѯ�����з���

    return XSUCC;
}

/**************************************************************************
�� �� ��: SPROamMsgProc
��������: ��Ϣ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
XS8 agent_OamMsgProc(XVOID *msg, XVOID *pPara)
{

    XS8 ret = XERROR;
	t_XOSCOMMHEAD* pxosMsg = (t_XOSCOMMHEAD*)msg;
	AGT_OAM_CFG_REQ_T* pRecvMsg = XNULL;

	if(XNULL == msg)
    {
		return XERROR;
    }

	pRecvMsg = (AGT_OAM_CFG_REQ_T *)pxosMsg->message;

	switch(pRecvMsg->uiTableId)
	{


    	case MML_UA_LINK_TABLE_ID:
    		ret = agentUA_LinkHandler(pRecvMsg);
    		break;


    	default:
    		XOS_PRINT(MD(FID_UA,PL_ERR),"oam_XosMsgProc recv msg from unknown MML table id %u",pRecvMsg->uiTableId);
    		ret = XERROR;
    		break;
	}
    
    return XSUCC;
}

/**************************************************************************
�� �� ��: SPROamTimeoutProc
��������: ��ʱ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ: XSUCC
**************************************************************************/
XS8 agent_OamTimeoutProc(t_BACKPARA  *pstPara)
{
    return XSUCC;
}

/**************************************************************************
�� �� ��: SPROamNoticeProc
��������: ֪ͨ����������ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
XS8 agent_OamNoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{	

	



	return XSUCC;
}







