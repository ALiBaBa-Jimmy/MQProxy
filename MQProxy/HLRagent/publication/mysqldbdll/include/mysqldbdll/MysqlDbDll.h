/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SmcDiamUa.h
* Author:       luhaiyan
* Date��        08-27-2014
* OverView:     SmcDiamUa�ӿ�ͷ�ļ�
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
	#ifdef MYSQLDBDLL_EXPORTS
	#define MYSQLDBDLL_API __declspec(dllexport)
	#else
	#define MYSQLDBDLL_API __declspec(dllimport)
	#endif
#else
	#define MYSQLDBDLL_API
#endif

