/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMAndHDbComFun.cpp
* Author:       luhaiyan
* Date��        09-29-2014
* OverView:     
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

#include "SPRMAndHDbComFun.h"
callbackTraceFun g_callbackTraceFunOfSPRDb;

/**********************************************************************
*
*  NAME:          RegistDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����
*  INPUT:         ��
*  OUTPUT:        
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 RegistDbCallbackTraceFun(callbackTraceFun  traceFun)
{ 
    g_callbackTraceFunOfSPRDb = traceFun;
	return XSUCC;
}
