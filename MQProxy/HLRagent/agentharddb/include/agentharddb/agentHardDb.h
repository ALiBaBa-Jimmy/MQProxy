/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssHardDb.h
* Author:       luhaiyan
* Date��        08-27-2014
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


#ifdef WIN32
	#ifdef SPRHARDDB_EXPORTS
	#define  AGENTHARDDB_API __declspec(dllexport)
	#else
	#define  AGENTHARDDB_API __declspec(dllimport)
	#endif
#else
	#define AGENTHARDDB_API
#endif

