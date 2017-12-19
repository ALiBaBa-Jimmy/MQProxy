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
函数名:  MYSQL_GetDbInitsCfg
功能：   从XML文件得到数据库初始化模块的配置信息
输入：   pFile， xml文件名
输出：   pOut，输出字符串
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
AGENTHARDDB_API XS32 MYSQL_GetDbInitsCfg(XU32 fid, SDbConfigInfo* pOut, XS8* pFile);

/************************************************************************
函数名:  MYSQL_RecordDbCfgToXML
功能：   将数据库的配置信息从内存中写入xml文件中
输入：   pDbCfgInfo:数据库配置信息
		 pFile：xml文件
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
AGENTHARDDB_API XS32 MYSQL_RecordDbCfgToXML(XU32 fid, SDbConfigInfo* pDbCfgInfo, XS8* pFile);


#ifdef __cplusplus
}
#endif
