/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdb.h
* Author:       luhaiyan
* Date��        10-12-2014
* OverView:     HSS�û��ڴ���Ϣ
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
#include "SPRMdbDbFun.h"
callbackTraceFun g_callbackTraceFunOfHssMDb;



callbackNotifyUpdateMdbToHdbFun g_callbackNotifyUpdateMdbToHdbFun;
/**********************************************************************
*
*  NAME:          RegistMDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 RegistMDbCallbackTraceFun(callbackTraceFun  traceFun)
{ 
    g_callbackTraceFunOfHssMDb = traceFun;
	return XSUCC;
}

