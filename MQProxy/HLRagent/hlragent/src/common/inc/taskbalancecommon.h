/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:    taskbalancecommon.h
* Author:       luhaiyan
* Date:			2015-01-17ֻ�ܸ�C++����
* OverView:     eHSS�Ĺ�����������Ҫ�����ĸ�����Ŀ�н��б���
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
#pragma  once

#include "xostype.h"
#include "xosshell.h"
#include "taskcommon.h"
#include <map>
#include <string>

using namespace std;
#pragma pack(1)

typedef struct _SBalanceTaskContextAry
{
	XU32 taskcount;//�����ܹ����߳���	
	XU32 endFid;
	XU32 startFid;
	t_XOSMUTEXID lockDispatchFid;
	//���ص�fid��Ϣ
	map<string, XU32 > mapDispatchFid;
	//ÿ���̵߳���������Ϣ
	STaskContext *pTaskContext;
}SBalanceTaskContextAry;



#pragma pack()

