#include "DbConfig.h"

#include "DbServiceOracle.h"
#include "DbStatementOracle.h"
#include <map>
#include <iostream>
#include <fstream>
#include "ace/Log_Msg.h"
#include "ace/Guard_T.h"

using namespace std;

//static XS32 InitLog()
//{
//	//打印系统日志
//	ofstream *outputFile = new ofstream("dbproj.log", ios::out);
//	if (outputFile && outputFile->rdstate() == ios::goodbit)
//	{
//		ACE_LOG_MSG->msg_ostream(outputFile, 1);
//	}
//    //ACE_LOG_MSG->open(NULL,ACE_Log_Msg::STDERR | ACE_Log_Msg::OSTREAM,0);//打印日志到屏幕和文件中
//	ACE_LOG_MSG->open(NULL,ACE_Log_Msg::OSTREAM,0);//只打印日志到文件中
//	return 0;
//}
//XS32 initlog = InitLog();




CDbServiceOracle::CDbServiceOracle(XVOID)
{
	m_penv = NULL;
	m_pConnPool = NULL;
	//初始化连接号为0
	m_connid = 0;
}

CDbServiceOracle::~CDbServiceOracle(XVOID)
{
	//不用delete m_penv,m_pConnPool,系统occi自动终止
}

/**********************************************************************
*
*  NAME:          SetConnPoolMap
*  FUNTION:       将SUserInfo和ConnectionPool *connPool的对应插入到map中
*  INPUT:         ConnectionPool *connPool:传入建立的连接池的指针
				  SUserInfo UserInfo：存储建立该连接池的用户名和密码
*  OUTPUT:        XSUCC
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::SetConnPoolMap(ConnectionPool *connPool,SUserInfo UserInfo)
{
	
	map<ConnectionPool*,SUserInfo>::iterator it;

	//加写锁
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
   
	it= m_mapConnPoolUser.find(connPool);
	
	if(it== m_mapConnPoolUser.end())
	{
		m_mapConnPoolUser.insert(make_pair(connPool,UserInfo));
	}
    else 
    { 
		it->second = UserInfo;
	}    
	return XSUCC;

}
/**********************************************************************
*
*  NAME:          GetUserInfo
*  FUNTION:       根据连接池的指针到map中查找其对应的SUserInfo信息
*  INPUT:         ConnectionPool *connPool:传入建立的连接池的指针
*  OUTPUT:        返回SUserInfo：用户名和密码信息的结构体
*  OTHERS:        
**********************************************************************/
SUserInfo CDbServiceOracle::GetUserInfo(ConnectionPool *connPool)
{
	SUserInfo userInfo = {0};
	map<ConnectionPool*,SUserInfo>::iterator it;

	//加读锁
	ACE_Read_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );  
   
	it= m_mapConnPoolUser.find(connPool);
	
	if(it== m_mapConnPoolUser.end())
	{
		return userInfo;
	}
    else 
    { 
		return it->second;
	}  
		
}
/**********************************************************************
*
*  NAME:          SetConnValue
*  FUNTION:       将dbConnId和Connection *conn的对应 插入到map中
*  INPUT:         XS64 dbConnId:连接号
				  Connection* conn：建立的数据库连接的指针
*  OUTPUT:        XSUCC
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::SetConnValue(Connection *conn,XU32 dbConnId)
{
	
	map<XU32, Connection*>::iterator it;

	//加写锁
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnidConnLock );

	it= m_mapConnidConn.find(dbConnId);
	
	if(it== m_mapConnidConn.end())
	{
		m_mapConnidConn.insert(make_pair(dbConnId,conn));
	}
    else 
    { 
		it->second = conn;
	}    
	return XSUCC;

}
/**********************************************************************
*
*  NAME:          GetConnValue
*  FUNTION:       根据dbConnId到map中查找到对应的 Connection* conn变量指针
*  INPUT:         XS64 dbConnId:连接号
*  OUTPUT:        Connection* conn
*  OTHERS:        
**********************************************************************/
Connection* CDbServiceOracle::GetConnValue(XU32 dbConnId)
{
	map<XU32, Connection*>::iterator it;
   
	//加读锁
	ACE_Read_Guard<ACE_RW_Thread_Mutex> guard( m_ConnidConnLock );  

	it= m_mapConnidConn.find(dbConnId);
	
	if(it== m_mapConnidConn.end())
	{
		return NULL;
	}
    else 
    { 
		return it->second;
	}  
		
}

/*得到真实的EnvMode*/
XS32 CDbServiceOracle::GetDbEnvMode(XS32 dbMode)
{
	XS32 trueDbMode = 0;
	switch (dbMode)	
	{
	case DEFAULT:
		trueDbMode = OCI_DEFAULT;
		break;
	case OBJECT:
		trueDbMode = OCI_OBJECT;
		break;
	case  SHARED:
		trueDbMode = OCI_SHARED;
		break;
	case NO_USERCALLBACKS:
		trueDbMode = OCI_NO_UCB;
		break;
	case THREADED_MUTEXED :
		trueDbMode = OCI_THREADED;
		break;
	case THREADED_UNMUTEXED:
		trueDbMode = OCI_THREADED | OCI_NO_MUTEX;
		break;
	case EVENTS :
		trueDbMode =  OCI_EVENTS;
		break;
	case USE_LDAP:
		trueDbMode =  OCI_USE_LDAP;
		break;
	default :
		break;
  };

	return trueDbMode;
}
/**********************************************************************
*
*  NAME:          DbInit
*  FUNTION:       根据应用层传入的环境变量模式参数env的值，初始化occi环境
*  INPUT:        
*  OUTPUT:        XSUCC(0);XERROR
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbInit(XU8* env)
{
	if (env == NULL)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---env is NULL!error!\n"));
		return XERROR;
	}
	
	//环境变量的参数是4个字节的类型
	SDllEnv* penvMode = (SDllEnv*)env;
	penvMode->mod = GetDbEnvMode(penvMode->mod);
	
	//初始化occi环境，此时mode为THREADED_MUTEXED，表示支持多线程
	//m_penv = Environment::createEnvironment(THREADED_MUTEXED);
	m_penv = Environment::createEnvironment((Environment::Mode)penvMode->mod);
	if(m_penv == NULL)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---m_penv = NULL,init occi environment is error!\n"));
		return XERROR;
	}

	return XSUCC;					
}

/**********************************************************************
*
*  NAME:          DbDestroy
*  FUNTION:       关闭occi环境
*  INPUT:        
*  OUTPUT:        XSUCC(0);
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroy(XVOID)
{	
	if ( m_penv != NULL )
    {
        Environment::terminateEnvironment(m_penv);//关闭occi环境
		
    }
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Destroy occi environment is error\n"));
		return XERROR;
	}
	return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbInitConnection
*  FUNTION:       初始化连接池，建立ConnectionPool*和SUserInfo的映射关系
*  INPUT:         XS8 *user:数据库用户名， XS8 *pwd：数据库密码, XS8 *connstring:数据库连接串
				  例如：root/xinwei@ora10g_172.16.8.119
				  XS8* parmeter：传入的参数信息，例如：MIN_CONNECT_NUM，MAX_CONNECT_NUM，INCREASE_CONNECT_NUM
				  XBOOL isDefault：是否是默认用户
*  OUTPUT:        XSUCC(0);XERROR
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbInitConnection(XS8* user, XS8* pwd, XS8 *connstring,XS8* parmeter,XBOOL isDefault)
{
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle::DbInitConnection begin\n"));
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle user=%s,pwd=%s,connstring=%s\n",user,pwd,connstring));
	SUserInfo UserInfo;
	if (isDefault == XTRUE)
	{
		if(m_penv == NULL)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle::m_penv = NULL,error!\n"));
			return XERROR;
		}
		//创建连接池
		try
		{
			m_pConnPool = m_penv->createConnectionPool(user,
									pwd,
									connstring,
									MIN_CONNECT_NUM,
									MAX_CONNECT_NUM, 
									INCREASE_CONNECT_NUM);
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---createConnectionPool ERROR,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
			return XERROR;
		}

		if (m_pConnPool == NULL)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---init connectionpool error!\n"));
			return XERROR;
		}
		//设置用户信息和ConnectionPool的映射关系
		UserInfo.user = user;
		UserInfo.pwd = pwd;
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle::SetConnPoolMap start\n"));
		SetConnPoolMap(m_pConnPool,UserInfo);
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle::SetConnPoolMap end\n"));

	}
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CDbServiceOracle::DbInitConnection end\n"));
	return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbDestroyConnection
*  FUNTION:       断开连接池
*  INPUT:         XS8 *user
*  OUTPUT:        XSUCC(0);XERROR
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroyConnection()
{
	//加写锁
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );  

	if (m_pConnPool != NULL)
	{
		try
		{
			m_penv->terminateConnectionPool(m_pConnPool);//关闭连接池
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---terminateConnectionPool ERROR,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
			return XERROR;
		}
		//在map中删除该连接池和用户信息的映射
		try{
			m_mapConnPoolUser.erase(m_pConnPool);
		
		//	delete m_pConnPool;
		//	m_pConnPool = NULL;
		}catch(...)
		{
		}
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---user destroy connectionpool error!\n"));
		return XERROR;
	}
    return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbCreateConn
*  FUNTION:       用创建连接池的用户名和密码获得连接池中可用的连接，分配一个连接 dbConnId为分配到的连接号
*  INPUT:         
*  OUTPUT:        dbConnId的值；
				  XERROR;
*  OTHERS:        不支持用户名或者密码为空的情况
**********************************************************************/
XU32 CDbServiceOracle::DbCreateConn(XVOID)
{
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Entering DbCreateConn.\n"));
	SUserInfo UserInfo = GetUserInfo(m_pConnPool);
	Connection *conn = NULL;

	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---DbCreateConn UserInfo.user is %s,UserInfo.pwd is %s.\n",UserInfo.user,UserInfo.pwd));
	if (UserInfo.user!= NULL && UserInfo.pwd != NULL)
	{
		try
		{
			conn = m_pConnPool->createConnection(UserInfo.user,UserInfo.pwd);
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---DbCreateConn ERROR,error msg is %s,error code is %d,UserInfo.user is %s,UserInfo.pwd is %s\n",e.what(),e.getErrorCode(),UserInfo.user,UserInfo.pwd));
			return XERROR;
		}
		if (conn == NULL )
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Connection is NULL,createConnection is error\n"));	
			return XERROR;
		}
		XU32 dbConnId = 0;
		//对连接号Id做多线程保护
		{
			ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_ConnIdLock);
			m_connid++;

			//modify by lx 2012.10.08//修改锁的作用域，保证m_connid和conn的对应关系
			//连接号从1开始递增
			dbConnId = m_connid;		
			SetConnValue(conn,dbConnId);
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetConnValue success,dbConnId is %d\n",dbConnId));
		}
		
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving DbCreateConn(XVOID) success,dbConnId is %d\n",dbConnId));
		return dbConnId;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---UserInfo doesn't exist,createConnection use default userInfo is error\n"));	
		return XERROR;
	}
	
}
/**********************************************************************
*
*  NAME:          DbCreateConn
*  FUNTION:       用不同于创建连接池的用户名和密码获得连接池中可用的连接，
				  分配一个连接 dbConnId为分配到的连接号
*  INPUT:         XS8 *user：创建连接的用户名
				  XS8 *pwd：创建连接的密码
*  OUTPUT:        dbConnId的值
				  XERROR;
*  OTHERS:        
**********************************************************************/
XU32 CDbServiceOracle::DbCreateConn(XS8 *user,XS8 *pwd)
{
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Entering DbCreateConn.user is %s,pwd is %s\n",user,pwd));
	Connection *conn = NULL;
	try
	{
		conn = m_pConnPool->createConnection(user,pwd);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---DbCreateConn ERROR,error msg is %s,error code is %d,usr is %s,pwd is %s\n",e.what(),e.getErrorCode(),user,pwd));
		return XERROR;
	}
	if (conn == NULL )
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Connection is NULL,createConnection is error\n"));		
		return XERROR;
	}

	XU32 dbConnId = 0;
	//对连接号Id做多线程保护
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_ConnIdLock);
		m_connid++;

		//modify by lx 2012.10.08//修改锁的作用域，保证m_connid和conn的对应关系
		//连接号从1开始递增
		dbConnId = m_connid;		
		SetConnValue(conn,dbConnId); 

		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetConnValue success,dbConnId is %d\n",dbConnId));
	}
	
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving DbCreateConn(XS8 *user,XS8 *pwd) success,dbConnId is %d\n",dbConnId));
	return dbConnId;		
}
/**********************************************************************
*
*  NAME:          DBCheckConnection
*  FUNTION:       验证该连接号对应的连接是否存在
*  INPUT:         XS64 dbConnId:连接号
*  OUTPUT:        XSUCC;
				  XERROR;
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DBCheckConnection(XU32 dbConnId)
{
	Connection* pConn = GetConnValue(dbConnId);
	if (pConn != NULL)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Connection exsit,success!\n"));	
		return XSUCC;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Connection does not exsit,error!\n"));	
		return XERROR;
	}
	
}
/**********************************************************************
*
*  NAME:          DbDestroyConn
*  FUNTION:       在连接池中，释放数据库连接
*  INPUT:         XS64 dbConnId：连接号
*  OUTPUT:        XSUCC(0);错误码
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroyConn(XU32 dbConnId)
{
	Connection *conn = GetConnValue(dbConnId);
	XS32 nRet = XERROR;

	//加写锁
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnidConnLock ); 

	if (conn != NULL)
	{
		try
		{
			m_pConnPool->terminateConnection(conn);//释放数据库连接
			//SetConnValue(NULL,dbConnId);//将m_mapConnidConn中，该dbConnId对应的Connection指针置为NULL
			m_mapConnidConn.erase(dbConnId);//将该dbConnId和对应的Connection的记录从map中删掉
			nRet = XSUCC;
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---释放连接时出错,error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
			nRet = XERROR;
		} 
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---DbReleaseConn error,Connection doesn't exist\n"));
		nRet = XERROR;
	}

	return nRet;
}

/**********************************************************************
*
*  NAME:          DbCommit
*  FUNTION:       根据dbConnId到m_mapConnidConn中查找对应的conn，由conn执行commit
*  INPUT:         XS64 dbConnId:连接号
*  OUTPUT:        XSUCC(0);错误码
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbCommit(XU32 dbConnId)
{
	if(0 == dbConnId)
	{
		return XERROR;
	}
	Connection *conn = GetConnValue(dbConnId);
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Execute DbCommit,connId id %d!\n",dbConnId));	
	if (conn != NULL)
	{
		try
		{
			conn->commit();		
		}
		catch(SQLException &e)
		{
			ACE_ERROR((LM_ERROR,"[%t](%M:%N:%l:%D)---Commit is error,error message is %s,errorcode is %d,connId is %d\n",e.what(),e.getErrorCode(),dbConnId));
			return XERROR;
		}
	}
	else
	{
		ACE_ERROR((LM_ERROR,"[%t](%M:%N:%l:%D)---DbCommit error,Connection doesn't exist,connId is %d\n",dbConnId));
		return XERROR;
	}
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving DbCommit success,connId id %d!\n",dbConnId));	
	return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbRollback
*  FUNTION:       根据dbTaskID到m_mapConnidConn中查找对应的conn，由conn执行rollback
*  INPUT:         XS64 dbConnId:连接号
*  OUTPUT:        XSUCC(0);错误码
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbRollback(XU32 dbConnId)
{
	Connection *conn = GetConnValue(dbConnId);
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Execute DbRollback,connId id %d!\n",dbConnId));	
	if (conn != NULL)
	{
		try
		{
			conn->rollback();
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Rollback is error,error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
			return XERROR;
		}
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---DbRollback error,Connection doesn't exist\n"));
		return XERROR;
	}
	return XSUCC;
}

/**********************************************************************
*
*  NAME:          DbCreateStatement
*  FUNTION:       创建一个Statement，并将其注入CDbStatement中
*  INPUT:         XS64 dbConnId：连接号,
				  XS8* sql：传入的sql语句
				  				  
*  OUTPUT:        CDbStatement* pStatment：输出CDbStatement的指针，由SetStatement()实现将Statement*注入到CDbStatement中
*  OTHERS:        其他需说明的问题
**********************************************************************/
CDbStatement* CDbServiceOracle::DbCreateStatement(XU32 dbConnId,XS8* sql)
{
	Connection *conn = GetConnValue(dbConnId);
	Statement *pStmt = NULL;
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---connId is %d,Entering DbCreateStatementsql is : (%s)\n",dbConnId,sql));
	
	if (conn != NULL)
	{
		if (sql == NULL)
		{
			//由Connection创建一个Statement类
			pStmt = conn->createStatement();
		}
		else
		{
			
			try
			{
				pStmt = conn->createStatement(sql);
				if(pStmt != NULL)
				{
					pStmt->setAutoCommit(FALSE);
				}

			}
			catch(SQLException &e)
			{
				ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---createStatement error,error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
				ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---sql is : (%s)\n",sql));
				return NULL;	
			}
		}
	
		if (pStmt != NULL)
		{
            CDbStatementOracle * pStatementOracle = new CDbStatementOracle();
			if (pStatementOracle != NULL)
			{
				//将Statement注入CDbStatement中
				pStatementOracle->SetStatement(pStmt); 
				pStatementOracle->SetConnId(dbConnId);
				return pStatementOracle;
			}
			else
			{
				ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---pStatementOracle is NULL ,error,dbConnId: %d\n",dbConnId));	
				return NULL;	
			}
			//pStmt->setPrefetchRowCount(FETCH_BATCH_COUNT);
		}
		else
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---CreateStatement is error,dbConnId: %d\n",dbConnId));	
			return NULL;
		}
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---get a connection for executeQuery,error,dbConnId: %d\n!",dbConnId));	
		return NULL;
	}
		
}
/**********************************************************************
*
*  NAME:          DbDestroyStatement
*  FUNTION:       终止该Statement
*  INPUT:         XS64 dbConnId：连接号,
				  CDbStatement* pStatment：传入处理类CDbStatement的指针，由其GetStatement()获得其对应的Statement的指针
*  OUTPUT:        XSUCC(0);XERROR(-1)
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment)
{
	Connection *conn = GetConnValue(dbConnId);
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Execute DbDestroyStatement,connId id %d!\n",dbConnId));	
	XS32 nRet = XERROR;
	if (conn != NULL)
	{
		try
		{
			conn->terminateStatement((Statement*)pStatment->GetStatement());
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---terminateStatement error,error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
			return XERROR;	
		}
		nRet = XSUCC;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---get a connection for executeQuery,error,dbConnId: %d\n",dbConnId));	
	}
	//删除CDbStatement的指针
	if( pStatment != NULL)
	{
		CDbStatementOracle * pDbStatementOracle =(CDbStatementOracle*)pStatment;
		delete pDbStatementOracle;
		pDbStatementOracle = NULL;
	}
	return nRet;
}

