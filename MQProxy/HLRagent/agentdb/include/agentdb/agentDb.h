/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     agentDb.h
* Author:       luhaiyan
* Date��        08-27-2014
* OverView:     SPRDb�ӿ�ͷ�ļ�
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
	#ifdef SPRDB_EXPORTS
	#define AGENTDB_API __declspec(dllexport)
	#else
	#define AGENTDB_API __declspec(dllimport)
	#endif
#else
	#define AGENTDB_API
#endif

