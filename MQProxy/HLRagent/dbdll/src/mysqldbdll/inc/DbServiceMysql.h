#pragma once
#include <map>
#include "ace/Thread.h"
#include "ace/Synch.h"
#include "DbService.h"
#include "mysql.h"


#pragma pack(1)


#define MYSQL_CONNECT_POOL_MAX_COUNT  10		/* 连接池的最大连接数 */

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
	int  in_use;			 /* 1:此连接正在使用, 0:连接空闲 */
	MYSQL_Connection mySql;
}_MYSQL_Connection;

typedef struct   
{
	int in_use_Count;		/* 连接池中正在使用的连接数 */
	_MYSQL_Connection st[MYSQL_CONNECT_POOL_MAX_COUNT];
}_MYSQL_ConnectionPool;


#pragma pack()



class CDbServiceMysql :
	public CDbService
{
public:
	CDbServiceMysql(void);
	virtual ~CDbServiceMysql(void);

	/*数据库环境参数初始化*/
    XS64 DbInit(XU8* env); 
	/*数据库环境参数销毁*/
	XS64 DbDestroy(void);
	/*得到真实的EnvMode*/
	XS32 GetDbEnvMode(int dbMode);
	//parmeter : 连接字符串的格式需要定义；XS8 *connstring,ip,XS8*parmeter=数据库名
	XS64 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,bool isDefault=true);
    
	XS64 DbDestroyConnection();
	
	/*在连接池中，分配可用的数据库连接*/
	XS64 DbCreateConn(void);

	XS64 DbCreateConn(XS8 *user,XS8 *pwd);
    
	XS64 DBCheckConnection(XU64 dbConnId);

	/*在连接池中，释放数据库连接*/
	XS64 DbDestroyConn(XU64 dbConnId);

	
	/*事务提交*/
	XS64 DbCommit(XU64 dbConnId);
	/*事务回滚*/
	XS64 DbRollback(XU64 dbConnId);

	CDbStatement* DbCreateStatement(XU64 dbConnId,XS8* sql);
    
    XS64 DbDestroyStatement(XU64 dbConnId,CDbStatement* pStatment);

	MYSQL_Connection* GetConnValue(XU64 dbConnId);


protected:

	ACE_RW_Thread_Mutex m_ConnPoolLock;//读写锁，保护m_mapConnPoolUser
	ACE_RW_Thread_Mutex m_ConnLock[MYSQL_CONNECT_POOL_MAX_COUNT];//读写锁，保护m_mapConnPoolUser



private :
	string m_defUserName;
	string m_defUserPwd;
	string m_defDbName;
	int m_defDbPort;

	_MYSQL_LinkState linkState;

	_MYSQL_ConnectionPool mysql_connectionpool;

	//得到msql的错误
	string TraceMysqlLastError(XU64 dbConnId);

};


