/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     DbCommon.cpp
* Author:       luhaiyan
* Date:         10-15-2014
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
#include "SPRTraceStruct.h"
#include "DbCommon.h"

callbackTraceFun g_callbackTraceFunOfMySqlDb;
/**********************************************************************
*
*  NAME:          RegistMySqlDbCallbackTraceFun
*  FUNTION:       ע��trace��������ҪΪ�˽����̬��log������˵�����
*  INPUT:         ��
*  OUTPUT:        ���ص�host��Ϣ
*  OTHERS:        ������˵��������
**********************************************************************/
XS32 RegistMySqlDbCallbackTraceFun(callbackTraceFun  traceFun)
{
	g_callbackTraceFunOfMySqlDb = traceFun;
	return 0;
}

