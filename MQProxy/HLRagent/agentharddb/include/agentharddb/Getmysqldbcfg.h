#pragma once
#include "xosshell.h"
#include "agentHardDb.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

#define  MAX_LENGTH_OF_DB_USER_NAME 32
#define  MAX_LENGTH_OF_DB_USER_PWD  32
#define  MAX_LENGTH_OF_DB_NAME		32
#define  MAX_LENGTH_OF_DB_IP		16

#if 0
typedef struct
{
	XU8 user[MAX_LENGTH_OF_DB_USER_NAME];
	XU8 pwd[MAX_LENGTH_OF_DB_USER_PWD]; 
	XU8 connstring[MAX_LENGTH_OF_DB_NAME];
    XU8 user_ali[MAX_LENGTH_OF_DB_USER_NAME];
	XU8 pwd_ali[MAX_LENGTH_OF_DB_USER_PWD]; 
	XU8 connstring_ali[MAX_LENGTH_OF_DB_NAME];

}SDbConfigInfo;
#endif
typedef struct
{
	XU8 user[MAX_LENGTH_OF_DB_USER_NAME];
	XU8 pwd[MAX_LENGTH_OF_DB_USER_PWD]; 
	XU8 dbName[MAX_LENGTH_OF_DB_NAME];
	XU8 ip[MAX_LENGTH_OF_DB_IP];
}SDbConfigInfo;
#pragma pack()

/************************************************************************
������:  MYSQL_GetDbInitsCfg
���ܣ�   ��XML�ļ��õ����ݿ��ʼ��ģ���������Ϣ
���룺   pFile�� xml�ļ���
�����   pOut������ַ���
���أ�   �ɹ� XSUCC
         ʧ�� XERROR
˵����
************************************************************************/
AGENTHARDDB_API XS32 MYSQL_GetDbInitsCfg(XU32 fid, SDbConfigInfo* pOut, XS8* pFile);

/************************************************************************
������:  MYSQL_RecordDbCfgToXML
���ܣ�   �����ݿ��������Ϣ���ڴ���д��xml�ļ���
���룺   pDbCfgInfo:���ݿ�������Ϣ
		 pFile��xml�ļ�
���أ�   �ɹ� XSUCC
         ʧ�� XERROR
˵����
************************************************************************/
AGENTHARDDB_API XS32 MYSQL_RecordDbCfgToXML(XU32 fid, SDbConfigInfo* pDbCfgInfo, XS8* pFile);


#ifdef __cplusplus
}
#endif
