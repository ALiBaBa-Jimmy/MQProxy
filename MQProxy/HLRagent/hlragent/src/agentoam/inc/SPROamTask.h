/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssOamTask.h
* Author:       luhaiyan
* Date:        30-08-2014
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

#pragma once

#include "xosshell.h"

//#include "diamuaOamStruct.h"
#define HA_STATUS_NOTIFY_MSG_ID 20
//#define ALARM_NOTIFY_MSG_ID 21
#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************************
�� �� ��: SPROamInitProc
��������: ��ʼ���ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 agent_OamInitProc(XVOID *pPara1, XVOID *pPara2 );

/**************************************************************************
�� �� ��: SPROamMsgProc
��������: ��Ϣ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 agent_OamMsgProc(XVOID *msg, XVOID *pPara);

/**************************************************************************
�� �� ��: SPROamTimeoutProc
��������: ��ʱ����ӿ�,��ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 agent_OamTimeoutProc(t_BACKPARA  *pstPara);

/**************************************************************************
�� �� ��: SPROamNoticeProc
��������: ֪ͨ����������ƽ̨ע��
��    ��:
�� �� ֵ:
**************************************************************************/
 XS8 agent_OamNoticeProc(XVOID* pLVoid, XVOID* pRVoid);












#ifdef __cplusplus
}
#endif /* __cplusplus */




