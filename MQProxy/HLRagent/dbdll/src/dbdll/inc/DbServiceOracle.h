#pragma once
#include <map>
#include "ace/Thread.h"
#include "ace/Synch.h"
#include "DbService.h"
#include <occi.h>
using namespace oracle::occi;

class CDbServiceOracle :
	public CDbService
{
public:
	CDbServiceOracle(XVOID);
	virtual ~CDbServiceOracle(XVOID);

	/*���ݿ⻷��������ʼ��*/
    XS32 DbInit(XU8* env); 
	/*���ݿ⻷����������*/
	XS32 DbDestroy(XVOID);

	/*�õ���ʵ��EnvMode*/
	XS32 GetDbEnvMode(XS32 dbMode);
	//parmeter : �����ַ����ĸ�ʽ��Ҫ���壻
	XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,XBOOL isDefault=XTRUE);
    
	XS32 DbDestroyConnection();
	
	/*�����ӳ��У�������õ����ݿ�����*/
	XU32 DbCreateConn(XVOID);

	XU32 DbCreateConn(XS8 *user,XS8 *pwd);
    
	XS32 DBCheckConnection(XU32 dbConnId);

	/*�����ӳ��У��ͷ����ݿ�����*/
	XS32 DbDestroyConn(XU32 dbConnId);

	
	/*�����ύ*/
	XS32 DbCommit(XU32 dbConnId);
	/*����ع�*/
	XS32 DbRollback(XU32 dbConnId);

	CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql);
    
    XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment);


protected:

	ACE_RW_Thread_Mutex m_ConnidConnLock;//��д��������m_mapConnidConn

	ACE_RW_Thread_Mutex m_ConnPoolLock;//��д��������m_mapConnPoolUser

	ACE_RW_Thread_Mutex m_ConnIdLock;//��д��������m_connid
	
	Environment *m_penv;
	ConnectionPool *m_pConnPool;

	XU32 m_connid;
	// taskid_conn�����Ĺ�ϣ��key connid�� value Connection*��
	std::map<XU32, Connection*> m_mapConnidConn;

	XS32 SetConnValue(Connection *conn,XU32 dbConnId);
	Connection* GetConnValue(XU32 dbConnId);

	// user_connPool�����Ĺ�ϣ��ConnectionPool* �� value XS8*��
	std::map<ConnectionPool*, SUserInfo> m_mapConnPoolUser;

	XS32 SetConnPoolMap(ConnectionPool *connPool,SUserInfo UserInfo);
	SUserInfo GetUserInfo(ConnectionPool *connPool);

};


