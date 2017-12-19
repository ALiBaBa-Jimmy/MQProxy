#include "DbConfig.h"

#include "DbServiceMysql.h"
#include "DbStatementMysql.h"
#include <map>
#include "ace/Log_Msg.h"

//#include <fstream>

#include "DbErrorCode.h"


CDbServiceMysql::CDbServiceMysql(void)
{
	m_defUserName = string("");
	m_defUserPwd= string("");
	m_defDbName= string("");
	m_defDbPort = 3306;
	memset(&mysql_connectionpool, 0, sizeof(mysql_connectionpool));
}

CDbServiceMysql::~CDbServiceMysql(void)
{
	
}





/*�õ���ʵ��EnvMode*/
XS32 CDbServiceMysql::GetDbEnvMode(int dbMode)
{
	int trueDbMode = 0;
	switch (dbMode)	
	{
	case DEFAULT:
		//trueDbMode = OCI_DEFAULT;
		break;
	case OBJECT:
		//trueDbMode = OCI_OBJECT;
		break;
	case  SHARED:
		//trueDbMode = OCI_SHARED;
		break;
	case NO_USERCALLBACKS:
		//trueDbMode = OCI_NO_UCB;
		break;
	case THREADED_MUTEXED :
		//trueDbMode = OCI_THREADED;
		break;
	case THREADED_UNMUTEXED:
		//trueDbMode = OCI_THREADED | OCI_NO_MUTEX;
		break;
	case EVENTS :
		//trueDbMode =  OCI_EVENTS;
		break;
	case USE_LDAP:
		//trueDbMode =  OCI_USE_LDAP;
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
XS64 CDbServiceMysql::DbInit(XU8* env)
{
	
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
XS64 CDbServiceMysql::DbDestroy(void)
{	
	
	return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbInitConnection
*  FUNTION:       ��ʼ�����ӳ�
*  INPUT:         XS8 *user:���ݿ��û����� XS8 *pwd�����ݿ�����, XS8 *connstring:	���ݿ���			  
				  XS8* parmeter�����ݿ�ip
				  bool isDefault���Ƿ���Ĭ���û�
*  OUTPUT:        XSUCC(0);XERROR
*  OTHERS:        
**********************************************************************/
XS64 CDbServiceMysql::DbInitConnection(XS8* user, XS8* pwd, XS8 *connstring,XS8* parmeter,bool isDefault)
{
	ACE_DEBUG((LM_DEBUG,"CDbServiceMysql::DbInitConnection begin:user=%s,pwd=%s,connstring=%s\n",user,pwd,connstring));
	
	int iIndex = 0;
	XS32 nRet = 0;


	XS8 *dbname = connstring;
	XS8* host = parmeter;
	int  dbport = m_defDbPort;
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	
	if (0 < mysql_connectionpool.in_use_Count) // mysql_connectionpool.in_use_Count must be zero!!!
	{
		ACE_ERROR((LM_ERROR,"mysql_connectionpool.in_use_Count >0 .\n"));		
		return XERROR;
	}

	for (iIndex = 0; iIndex < MYSQL_CONNECT_POOL_MAX_COUNT; ++iIndex)
	{	
		ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnLock[iIndex] );
	
		if (NULL == mysql_init(&mysql_connectionpool.st[iIndex].mySql))
		{			
			 ACE_ERROR((LM_ERROR,"Init mysql connect handle failed.\n"));	
			 return ERROR_CODE_DB_NULL_PTR;
		}
		mysql_connectionpool.st[iIndex].mySql.reconnect = 1; /* ����Ϊ�Զ����� */
		
		//mysql_options(&mysql_connectionpool.st[iIndex].mySql, MYSQL_OPT_RECONNECT,0);	
	
		if (!mysql_real_connect(&mysql_connectionpool.st[iIndex].mySql, host, user,pwd,dbname,dbport,NULL,CLIENT_FOUND_ROWS))
		{
			ACE_ERROR((LM_ERROR,"Error:mysql_query is error! %s",TraceMysqlLastError(iIndex).c_str()));		
	   		return ERROR_CODE_DB_CONN_FAIL;
 		}

		// �ر��Զ��ύģʽ
		mysql_autocommit(&mysql_connectionpool.st[iIndex].mySql, 0);
	}
	if(isDefault)
	{
		m_defUserName = string(user);
		m_defUserPwd= string(pwd);
		m_defDbName= string(connstring);
	}
	
	ACE_DEBUG((LM_DEBUG,"CDbServiceMysql::DbInitConnection end:user=%s,pwd=%s,ip=%s, dbname =%s.\n",
		user,pwd,connstring, parmeter));
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
XS64 CDbServiceMysql::DbDestroyConnection(XS8 *user)
{	
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	for (int iIndex = 0; iIndex < MYSQL_CONNECT_POOL_MAX_COUNT; ++iIndex)
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnLock[iIndex] );
		mysql_close( &mysql_connectionpool.st[iIndex].mySql );
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
XS64 CDbServiceMysql::DbCreateConn(void)
{
	XS64 dbConnId = 0;
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	if (MYSQL_CONNECT_POOL_MAX_COUNT <= mysql_connectionpool.in_use_Count)
	{
		ACE_DEBUG((LM_DEBUG,"CDbServiceMysql::DbCreateConn(),mysql connectpool is full!\n"));
	
		return ERROR_CODE_DB_CONN_POOL_IS_FULL;
	}
	
	for (int iIndex = 1; iIndex < MYSQL_CONNECT_POOL_MAX_COUNT; ++iIndex)  /* 0 ��ҵ����*/
	{
		ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnLock[iIndex] );
		if (0 == mysql_connectionpool.st[iIndex].in_use)
		{
			dbConnId = iIndex;
			mysql_connectionpool.st[iIndex].in_use = 1;
			mysql_connectionpool.in_use_Count++;
			linkState.Link[iIndex].QryCount++;
			linkState.Link[iIndex].use_state = 1;
			break;
		}
	}

	ACE_DEBUG((LM_DEBUG,"CDbServiceMysql::DbCreateConn(),dbConnId=%d!\n",dbConnId));
	return dbConnId;	
	
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
XS64 CDbServiceMysql::DbCreateConn(XS8 *user,XS8 *pwd)
{
	return DbCreateConn();		
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
XS64 CDbServiceMysql::DBCheckConnection(XU64 dbConnId)
{	
	if (0 > dbConnId || MYSQL_CONNECT_POOL_MAX_COUNT<= dbConnId)
	{
		ACE_DEBUG((LM_DEBUG,"MYSQL_Connection does not exsit,error, dbConnId=%d!\n", dbConnId));	
		return XERROR;
	}
	else	
	{
		ACE_DEBUG((LM_DEBUG,"MYSQL_Connection exsit,success, dbConnId=%d!\n", dbConnId));	
		return XSUCC;
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
XS64 CDbServiceMysql::DbDestroyConn(XU64 dbConnId)
{
	int iIndex = 0;
	
	if (0 == mysql_connectionpool.in_use_Count)
	{
		return XSUCC;
	}

	if (0 > dbConnId || MYSQL_CONNECT_POOL_MAX_COUNT<= dbConnId)
	{
		ACE_ERROR((LM_ERROR, "CDbServiceMysql::DbDestroyConn()free a  mysql connect(%d) error!", dbConnId));	
		return XERROR;
	}
	
	if (1 == mysql_connectionpool.st[dbConnId].in_use)
	{
		mysql_connectionpool.st[dbConnId].in_use = 0;
		mysql_connectionpool.in_use_Count--;
		linkState.Link[iIndex].use_state = 0;
		ACE_DEBUG((LM_DEBUG, "CDbServiceMysql::DbDestroyConn()free a  mysql connect(%d) success!", dbConnId));	
	}

	return XSUCC;
}

/**********************************************************************
*
*  NAME:          DbCommit
*  FUNTION:       
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS64 CDbServiceMysql::DbCommit(XU64 dbConnId)
{
	XS64 nRet = DBCheckConnection(dbConnId);
	if(XSUCC != nRet)
	{
		ACE_ERROR((LM_ERROR, "CDbServiceMysql::DbCommit() mysql connect(%d) error!", dbConnId));	
		return nRet;
	}
	return mysql_commit(&mysql_connectionpool.st[dbConnId].mySql);
	
}
/**********************************************************************
*
*  NAME:          DbRollback
*  FUNTION:       
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS64 CDbServiceMysql::DbRollback(XU64 dbConnId)
{
	XS64 nRet = DBCheckConnection(dbConnId);
	if(XSUCC != nRet)
	{
		ACE_ERROR((LM_ERROR, "CDbServiceMysql::DbCommit() mysql connect(%d) error!", dbConnId));	
		return nRet;
	}
	return mysql_rollback(&mysql_connectionpool.st[dbConnId].mySql);
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
CDbStatement* CDbServiceMysql::DbCreateStatement(XU64 dbConnId,XS8* sql)
{	
	if (sql == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"DbCreateStatement fail,sql is null.dbConnId=%d.", dbConnId));
		return NULL;
	}
	MYSQL_Connection * pConn = GetConnValue(dbConnId);
	if (pConn == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"DbCreateStatement fail, conn is null.dbConnId=%d.", dbConnId));
		return NULL;
	}
	MYSQL_STMT * pStmt = mysql_stmt_init(pConn);
	if (pStmt == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"DbCreateStatement fail, stmt is null.dbConnId=%d.", dbConnId));
		return NULL;
	}

	// ׼��sql
	int nRet = mysql_stmt_prepare(pStmt, sql, strlen(sql));
	if (nRet != 0)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"DbCreateStatement prepare fail, dbConnId=%d, sql =%s.", dbConnId, sql));
		ACE_ERROR((LM_ERROR,"DbCreateStatement prepare fail! %s",TraceMysqlLastError(dbConnId).c_str()));		
		return NULL;
	}


	CDbStatementMysql * pStatementMysql = new CDbStatementMysql();
	if (pStatementMysql != NULL)
	{
		//��Statementע��CDbStatement��
		pStatementMysql->SetStatement(pStmt); 
		pStatementMysql->m_strSql= string(sql);
	
		return pStatementMysql;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"DbCreateStatement new  CDbStatementMysql is NULL ,dbConnId: %d\n",dbConnId));	
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
XS64 CDbServiceMysql::DbDestroyStatement(XU64 dbConnId,CDbStatement* pStatment)
{
	if (pStatment == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"DbDestroyStatement fail,pStatment is null.dbConnId=%d.", dbConnId));
		return NULL;
	}
	
	XS64 nRet = mysql_stmt_close((MYSQL_STMT *) pStatment->GetStatement());
	
	//ɾ��CDbStatement��ָ��
	CDbStatementMysql * pDbStatementMysql =(CDbStatementMysql*)pStatment;
	delete pDbStatementMysql;
	pDbStatementMysql = NULL;
	return nRet;
}

/**********************************************************************
*
*  NAME:          GetConnValue
*  FUNTION:       ����dbConnId��map�в��ҵ���Ӧ�� Connection* conn����ָ��
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        MYSQL_Connection** conn
*  OTHERS:        
**********************************************************************/
MYSQL_Connection* CDbServiceMysql::GetConnValue(XU64 dbConnId)
{
	if(XSUCC != DBCheckConnection(dbConnId))
	{
		return NULL;
	}
	else
	{
		return &mysql_connectionpool.st[dbConnId].mySql;
	}
		
}

/**********************************************************************
*
*  NAME:          TraceMysqlLastError
*  FUNTION:       �õ�msql�Ĵ���
*  INPUT:         XS64 dbConnId:���Ӻ�
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
string CDbServiceMysql::TraceMysqlLastError(XU64 dbConnId)
{
	MYSQL_Connection * pDbConn = GetConnValue(dbConnId);
	if(NULL == pDbConn )
	{
		string str = "error connId " + dbConnId;
		
		return str;
	}
	return  string(mysql_error(pDbConn));
}