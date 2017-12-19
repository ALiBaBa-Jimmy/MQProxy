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

	/*数据库环境参数初始化*/
    XS32 DbInit(XU8* env); 
	/*数据库环境参数销毁*/
	XS32 DbDestroy(XVOID);

	/*得到真实的EnvMode*/
	XS32 GetDbEnvMode(XS32 dbMode);
	//parmeter : 连接字符串的格式需要定义；
	XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,XBOOL isDefault=XTRUE);
    
	XS32 DbDestroyConnection();
	
	/*在连接池中，分配可用的数据库连接*/
	XU32 DbCreateConn(XVOID);

	XU32 DbCreateConn(XS8 *user,XS8 *pwd);
    
	XS32 DBCheckConnection(XU32 dbConnId);

	/*在连接池中，释放数据库连接*/
	XS32 DbDestroyConn(XU32 dbConnId);

	
	/*事务提交*/
	XS32 DbCommit(XU32 dbConnId);
	/*事务回滚*/
	XS32 DbRollback(XU32 dbConnId);

	CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql);
    
    XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment);


protected:

	ACE_RW_Thread_Mutex m_ConnidConnLock;//读写锁，保护m_mapConnidConn

	ACE_RW_Thread_Mutex m_ConnPoolLock;//读写锁，保护m_mapConnPoolUser

	ACE_RW_Thread_Mutex m_ConnIdLock;//读写锁，保护m_connid
	
	Environment *m_penv;
	ConnectionPool *m_pConnPool;

	XU32 m_connid;
	// taskid_conn参数的哈希表（key connid， value Connection*）
	std::map<XU32, Connection*> m_mapConnidConn;

	XS32 SetConnValue(Connection *conn,XU32 dbConnId);
	Connection* GetConnValue(XU32 dbConnId);

	// user_connPool参数的哈希表（ConnectionPool* ， value XS8*）
	std::map<ConnectionPool*, SUserInfo> m_mapConnPoolUser;

	XS32 SetConnPoolMap(ConnectionPool *connPool,SUserInfo UserInfo);
	SUserInfo GetUserInfo(ConnectionPool *connPool);

};


