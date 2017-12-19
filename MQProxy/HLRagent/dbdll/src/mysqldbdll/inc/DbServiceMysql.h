#pragma once
#include <map>
#include "ace/Thread.h"
#include "ace/Synch.h"
#include "DbService.h"
#include "mysql.h"


#pragma pack(1)


#define MYSQL_CONNECT_POOL_MAX_COUNT  10		/* ���ӳص���������� */

typedef MYSQL 		MYSQL_Connection;
//typedef MYSQL_ROW   MysqlRow, *pMysqlRow;

typedef struct
{
	XU8 	connect_state;   
	struct 
	{
		XU32	QryCount;
		XU8     use_state;
	}Link[MYSQL_CONNECT_POOL_MAX_COUNT];
}_MYSQL_LinkState;

typedef struct
{	
	int  in_use;			 /* 1:����������ʹ��, 0:���ӿ��� */
	MYSQL_Connection mySql;
}_MYSQL_Connection;

typedef struct   
{
	int in_use_Count;		/* ���ӳ�������ʹ�õ������� */
	_MYSQL_Connection st[MYSQL_CONNECT_POOL_MAX_COUNT];
}_MYSQL_ConnectionPool;


#pragma pack()



class CDbServiceMysql :
	public CDbService
{
public:
	CDbServiceMysql(void);
	virtual ~CDbServiceMysql(void);

	/*���ݿ⻷��������ʼ��*/
    XS64 DbInit(XU8* env); 
	/*���ݿ⻷����������*/
	XS64 DbDestroy(void);
	/*�õ���ʵ��EnvMode*/
	XS32 GetDbEnvMode(int dbMode);
	//parmeter : �����ַ����ĸ�ʽ��Ҫ���壻XS8 *connstring,ip,XS8*parmeter=���ݿ���
	XS64 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,bool isDefault=true);
    
	XS64 DbDestroyConnection();
	
	/*�����ӳ��У�������õ����ݿ�����*/
	XS64 DbCreateConn(void);

	XS64 DbCreateConn(XS8 *user,XS8 *pwd);
    
	XS64 DBCheckConnection(XU64 dbConnId);

	/*�����ӳ��У��ͷ����ݿ�����*/
	XS64 DbDestroyConn(XU64 dbConnId);

	
	/*�����ύ*/
	XS64 DbCommit(XU64 dbConnId);
	/*����ع�*/
	XS64 DbRollback(XU64 dbConnId);

	CDbStatement* DbCreateStatement(XU64 dbConnId,XS8* sql);
    
    XS64 DbDestroyStatement(XU64 dbConnId,CDbStatement* pStatment);

	MYSQL_Connection* GetConnValue(XU64 dbConnId);


protected:

	ACE_RW_Thread_Mutex m_ConnPoolLock;//��д��������m_mapConnPoolUser
	ACE_RW_Thread_Mutex m_ConnLock[MYSQL_CONNECT_POOL_MAX_COUNT];//��д��������m_mapConnPoolUser



private :
	string m_defUserName;
	string m_defUserPwd;
	string m_defDbName;
	int m_defDbPort;

	_MYSQL_LinkState linkState;

	_MYSQL_ConnectionPool mysql_connectionpool;

	//�õ�msql�Ĵ���
	string TraceMysqlLastError(XU64 dbConnId);

};


