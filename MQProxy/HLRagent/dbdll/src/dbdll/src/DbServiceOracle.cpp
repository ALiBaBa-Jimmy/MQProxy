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
//	//��ӡϵͳ��־
//	ofstream *outputFile = new ofstream("dbproj.log", ios::out);
//	if (outputFile && outputFile->rdstate() == ios::goodbit)
//	{
//		ACE_LOG_MSG->msg_ostream(outputFile, 1);
//	}
//    //ACE_LOG_MSG->open(NULL,ACE_Log_Msg::STDERR | ACE_Log_Msg::OSTREAM,0);//��ӡ��־����Ļ���ļ���
//	ACE_LOG_MSG->open(NULL,ACE_Log_Msg::OSTREAM,0);//ֻ��ӡ��־���ļ���
//	return 0;
//}
//XS32 initlog = InitLog();




CDbServiceOracle::CDbServiceOracle(XVOID)
{
	m_penv = NULL;
	m_pConnPool = NULL;
	//��ʼ�����Ӻ�Ϊ0
	m_connid = 0;
}

CDbServiceOracle::~CDbServiceOracle(XVOID)
{
	//����delete m_penv,m_pConnPool,ϵͳocci�Զ���ֹ
}

/**********************************************************************
*
*  NAME:          SetConnPoolMap
*  FUNTION:       ��SUserInfo��ConnectionPool *connPool�Ķ�Ӧ���뵽map��
*  INPUT:         ConnectionPool *connPool:���뽨�������ӳص�ָ��
				  SUserInfo UserInfo���洢���������ӳص��û���������
*  OUTPUT:        XSUCC
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::SetConnPoolMap(ConnectionPool *connPool,SUserInfo UserInfo)
{
	
	map<ConnectionPool*,SUserInfo>::iterator it;

	//��д��
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
*  FUNTION:       �������ӳص�ָ�뵽map�в������Ӧ��SUserInfo��Ϣ
*  INPUT:         ConnectionPool *connPool:���뽨�������ӳص�ָ��
*  OUTPUT:        ����SUserInfo���û�����������Ϣ�Ľṹ��
*  OTHERS:        
**********************************************************************/
SUserInfo CDbServiceOracle::GetUserInfo(ConnectionPool *connPool)
{
	SUserInfo userInfo = {0};
	map<ConnectionPool*,SUserInfo>::iterator it;

	//�Ӷ���
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
*  FUNTION:       ��dbConnId��Connection *conn�Ķ�Ӧ ���뵽map��
*  INPUT:         XS64 dbConnId:���Ӻ�
				  Connection* conn�����������ݿ����ӵ�ָ��
*  OUTPUT:        XSUCC
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::SetConnValue(Connection *conn,XU32 dbConnId)
{
	
	map<XU32, Connection*>::iterator it;

	//��д��
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
*  FUNTION:       ����dbConnId��map�в��ҵ���Ӧ�� Connection* conn����ָ��
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        Connection* conn
*  OTHERS:        
**********************************************************************/
Connection* CDbServiceOracle::GetConnValue(XU32 dbConnId)
{
	map<XU32, Connection*>::iterator it;
   
	//�Ӷ���
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

/*�õ���ʵ��EnvMode*/
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
*  FUNTION:       ����Ӧ�ò㴫��Ļ�������ģʽ����env��ֵ����ʼ��occi����
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
	
	//���������Ĳ�����4���ֽڵ�����
	SDllEnv* penvMode = (SDllEnv*)env;
	penvMode->mod = GetDbEnvMode(penvMode->mod);
	
	//��ʼ��occi��������ʱmodeΪTHREADED_MUTEXED����ʾ֧�ֶ��߳�
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
*  FUNTION:       �ر�occi����
*  INPUT:        
*  OUTPUT:        XSUCC(0);
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroy(XVOID)
{	
	if ( m_penv != NULL )
    {
        Environment::terminateEnvironment(m_penv);//�ر�occi����
		
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
*  FUNTION:       ��ʼ�����ӳأ�����ConnectionPool*��SUserInfo��ӳ���ϵ
*  INPUT:         XS8 *user:���ݿ��û����� XS8 *pwd�����ݿ�����, XS8 *connstring:���ݿ����Ӵ�
				  ���磺root/xinwei@ora10g_172.16.8.119
				  XS8* parmeter������Ĳ�����Ϣ�����磺MIN_CONNECT_NUM��MAX_CONNECT_NUM��INCREASE_CONNECT_NUM
				  XBOOL isDefault���Ƿ���Ĭ���û�
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
		//�������ӳ�
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
		//�����û���Ϣ��ConnectionPool��ӳ���ϵ
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
*  FUNTION:       �Ͽ����ӳ�
*  INPUT:         XS8 *user
*  OUTPUT:        XSUCC(0);XERROR
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroyConnection()
{
	//��д��
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );  

	if (m_pConnPool != NULL)
	{
		try
		{
			m_penv->terminateConnectionPool(m_pConnPool);//�ر����ӳ�
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---terminateConnectionPool ERROR,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
			return XERROR;
		}
		//��map��ɾ�������ӳغ��û���Ϣ��ӳ��
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
*  FUNTION:       �ô������ӳص��û��������������ӳ��п��õ����ӣ�����һ������ dbConnIdΪ���䵽�����Ӻ�
*  INPUT:         
*  OUTPUT:        dbConnId��ֵ��
				  XERROR;
*  OTHERS:        ��֧���û�����������Ϊ�յ����
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
		//�����Ӻ�Id�����̱߳���
		{
			ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_ConnIdLock);
			m_connid++;

			//modify by lx 2012.10.08//�޸����������򣬱�֤m_connid��conn�Ķ�Ӧ��ϵ
			//���ӺŴ�1��ʼ����
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
*  FUNTION:       �ò�ͬ�ڴ������ӳص��û��������������ӳ��п��õ����ӣ�
				  ����һ������ dbConnIdΪ���䵽�����Ӻ�
*  INPUT:         XS8 *user���������ӵ��û���
				  XS8 *pwd���������ӵ�����
*  OUTPUT:        dbConnId��ֵ
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
	//�����Ӻ�Id�����̱߳���
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_ConnIdLock);
		m_connid++;

		//modify by lx 2012.10.08//�޸����������򣬱�֤m_connid��conn�Ķ�Ӧ��ϵ
		//���ӺŴ�1��ʼ����
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
*  FUNTION:       ��֤�����ӺŶ�Ӧ�������Ƿ����
*  INPUT:         XS64 dbConnId:���Ӻ�
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
*  FUNTION:       �����ӳ��У��ͷ����ݿ�����
*  INPUT:         XS64 dbConnId�����Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceOracle::DbDestroyConn(XU32 dbConnId)
{
	Connection *conn = GetConnValue(dbConnId);
	XS32 nRet = XERROR;

	//��д��
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnidConnLock ); 

	if (conn != NULL)
	{
		try
		{
			m_pConnPool->terminateConnection(conn);//�ͷ����ݿ�����
			//SetConnValue(NULL,dbConnId);//��m_mapConnidConn�У���dbConnId��Ӧ��Connectionָ����ΪNULL
			m_mapConnidConn.erase(dbConnId);//����dbConnId�Ͷ�Ӧ��Connection�ļ�¼��map��ɾ��
			nRet = XSUCC;
		}
		catch(SQLException &e)
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---�ͷ�����ʱ����,error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
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
*  FUNTION:       ����dbConnId��m_mapConnidConn�в��Ҷ�Ӧ��conn����connִ��commit
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
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
*  FUNTION:       ����dbTaskID��m_mapConnidConn�в��Ҷ�Ӧ��conn����connִ��rollback
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
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
*  FUNTION:       ����һ��Statement��������ע��CDbStatement��
*  INPUT:         XS64 dbConnId�����Ӻ�,
				  XS8* sql�������sql���
				  				  
*  OUTPUT:        CDbStatement* pStatment�����CDbStatement��ָ�룬��SetStatement()ʵ�ֽ�Statement*ע�뵽CDbStatement��
*  OTHERS:        ������˵��������
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
			//��Connection����һ��Statement��
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
				//��Statementע��CDbStatement��
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
*  FUNTION:       ��ֹ��Statement
*  INPUT:         XS64 dbConnId�����Ӻ�,
				  CDbStatement* pStatment�����봦����CDbStatement��ָ�룬����GetStatement()������Ӧ��Statement��ָ��
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
	//ɾ��CDbStatement��ָ��
	if( pStatment != NULL)
	{
		CDbStatementOracle * pDbStatementOracle =(CDbStatementOracle*)pStatment;
		delete pDbStatementOracle;
		pDbStatementOracle = NULL;
	}
	return nRet;
}

