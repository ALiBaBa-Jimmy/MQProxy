#pragma once
#include <map>
//#include "ace/Thread.h"
//#include "ace/Synch.h"
#include "DbService.h"
#include "mysql.h"


#pragma pack(1)


#define MYSQL_CONNECT_POOL_MAX_COUNT  25		/* 连接池的最大连接数 */
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
	int  in_use;			 /* 1:此连接正在使用, 0:连接空闲 */
	XU32 fid;
	MYSQL_Connection mySql;
	t_XOSMUTEXID dbConnLock;
}_MYSQL_Connection;

typedef struct   
{
	unsigned int in_use_Count;		/* 连接池中正在使用的连接数 */
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
    XS32 DbInit(XU8* env, XU32 fid=DEFAULT_FID); 
	/*数据库环境参数销毁*/
	XS32 DbDestroy(XU32 fid=DEFAULT_FID);
	/*得到真实的EnvMode*/
	XS32 GetDbEnvMode(int dbMode);
	//parmeter : 连接字符串的格式需要定义；XS8 *connstring,ip,XS8*parmeter=数据库名
	XS32 DbInitConnection(XS8 *user, XS8 *pwd, XS8 *connstring,XS8*parmeter=NULL,bool isDefault=true, XU32 fid=DEFAULT_FID);
    
	XS32 DbDestroyConnection(XU32 fid=DEFAULT_FID);
	
	/*在连接池中，分配可用的数据库连接*/
	XS32 DbCreateConn(XU32 fid=DEFAULT_FID);

	XS32 DbCreateConn(XS8 *user,XS8 *pwd, XU32 fid=DEFAULT_FID);
    
	XS32 DBCheckConnection(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	//check所有的连接,如果没有连接成功采用ping在重新连接
	XS32 DBCheckConnection();

	/*在连接池中，释放数据库连接*/
	XS32 DbDestroyConn(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	
	/*事务提交*/
	XS32 DbCommit(XU32 dbConnId, XU32 fid=DEFAULT_FID);
	/*事务回滚*/
	XS32 DbRollback(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	CDbStatement* DbCreateStatement(XU32 dbConnId,XS8* sql, XU32 fid=DEFAULT_FID);
    
    XS32 DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment, XU32 fid=DEFAULT_FID);

	MYSQL_Connection* GetConnValue(XU32 dbConnId, XU32 fid=DEFAULT_FID);

	/*得到需要返回的以下信息 type期望按什么返回，retStr 要返回的串，strLen，传递的retStr最大长度*/
	XS32 GetString(XU32 type, XS8* retStr, XU32 strLen);


protected:

	t_XOSMUTEXID m_ConnPoolLock;//读写锁，保护m_mapConnPoolUser
	//ACE_RW_Thread_Mutex m_ConnLock[MYSQL_CONNECT_POOL_MAX_COUNT];//读写锁，保护m_mapConnPoolUser

	//最大连接数
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

	//得到msql的错误
	string TraceMysqlLastError(XU32 dbConnId);

};


