/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssMdb.h
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
	#ifdef AGENTOAM_EXPORTS
	#define  AGNETMDB_API __declspec(dllexport)
	#else
	#define  AGNETMDB_API __declspec(dllimport)
	#endif
#else
	#define AGNETMDB_API
#endif

