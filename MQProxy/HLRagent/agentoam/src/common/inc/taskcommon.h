#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#include "xostype.h"
#include "xosshell.h"

#pragma pack(1)

typedef struct _STaskContext
{
	XU16 taskIndex;   //task������
	XU16 taskcount;  //�ܵ�task����
	XU32 startFid;   //������ʼ��fid ��Ҫ��Ϊ�˲����������ڹ�ϵFID�Ķ���
	XBOOL  g_bBeAttached;
	XVOID* taskFidList;
	XU32  taskTid;
	XU32  taskFid;
	XU32  dbTaskId;
	XU32  messageCount;//��ǰ����ĸ���
}STaskContext;

typedef struct _STaskContextAry
{
	XU32 taskcount;//�����ܹ����߳���	
	XU32 endFid;
	XU32 startFid;
	XU32 dispatchFid; //��ǰ���ص�Fidֵ��ʹ����򵥵ĸ��أ��յ�һ����Ϣ������������е��߳������η���
	//ÿ���̵߳���������Ϣ
	STaskContext *pTaskContext;
}STaskContextAry;

#pragma pack()

#ifdef __cplusplus
}
#endif 

