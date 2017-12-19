#pragma once
#include <map>
//#include "ace/Thread.h"
//#include "ace/Synch.h"
#include "DbService.h"
#include "mysql.h"


#pragma pack(1)


#define MYSQL_CONNECT_POOL_MAX_COUNT  25		/* ���ӳص���������� */
#define DEFAULT_FID  1100

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
	XU32 fid;
	MYSQL_Connection mySql;
	t_XOSMUTEXID dbConnLock;
}_MYSQL_Connection;

typedef struct   
{
	unsigned int in_use_Count;		/* ���ӳ�������ʹ�õ������� */
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
    XS32 DbInit(XU8* env, XU32 fid=DEFAULT_FID); 
	/*���ݿ⻷����������*/
	XS32 DbDestroy(XU32 fid=DEFAULT_FID);
	/*�õ���ʵ��EnvMode*/
	XS32 GetDbEnvMode(int dbMode);
	//parmeter : �����ַ����ĸ�ʽ��Ҫ���壻XS8 *connstring,ip,XS8*parmeter=���ݿ���
	XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,bool isDefault=true, XU32 fid=DEFAULT_FID);
    
	XS32 DbDestroyConnection(XU32 fid=DEFAULT_FID);
	
	/*�����ӳ��У�������õ����ݿ�����*/
	XS32 DbCreateConn(XU32 fid=DEFAULT_FID);

	XS32 DbCreateConn(XS8 *user,XS8 *pwd, XU32 fid=DEFAULT_FID);
    
	XS32 DBCheckConnection(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	//check���е�����,���û�����ӳɹ�����ping����������
	XS32 DBCheckConnection();

	/*�����ӳ��У��ͷ����ݿ�����*/
	XS32 DbDestroyConn(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	
	/*�����ύ*/
	XS32 DbCommit(XU32 dbConnId, XU32 fid=DEFAULT_FID);
	/*����ع�*/
	XS32 DbRollback(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql, XU32 fid=DEFAULT_FID);
    
    XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment, XU32 fid=DEFAULT_FID);

	MYSQL_Connection* GetConnValue(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	/*�õ���Ҫ���ص�������Ϣ type������ʲô���أ�retStr Ҫ���صĴ���strLen�����ݵ�retStr��󳤶�*/
	XS32 GetString(XU32 type, XS8* retStr, XU32 strLen);


protected:

	t_XOSMUTEXID m_ConnPoolLock;//��д��������m_mapConnPoolUser
	//ACE_RW_Thread_Mutex m_ConnLock[MYSQL_CONNECT_POOL_MAX_COUNT];//��д��������m_mapConnPoolUser

	//���������
	unsigned int m_nMaxConnect;

private :
	string m_defUserName;
	string m_defUserPwd;
	string m_defDbName;
	string m_defHost;
	int m_defDbPort;
	bool m_bInit;
	
	//_MYSQL_LinkState linkState;

	_MYSQL_ConnectionPool mysql_connectionpool;

	//�õ�msql�Ĵ���
	string TraceMysqlLastError(XU32 dbConnId);

};


