#include "DbConfig.h"

#include "DbServiceMysql.h"
#include "DbStatementMysql.h"
#include <map>
//#include "ace/Log_Msg.h"
//#include "ace/Guard_T.h"
#include "DbCommon.h"
#include "fid_def.h"
//#include <fstream>

#include "DbErrorCode.h"


CDbServiceMysql::CDbServiceMysql(void)
{
	m_defUserName = string("");
	m_defUserPwd= string("");
	m_defDbName= string("");
	m_defHost= string("");
	m_defDbPort = 3306;
	m_nMaxConnect = MYSQL_CONNECT_POOL_MAX_COUNT;
	memset(&mysql_connectionpool, 0, sizeof(mysql_connectionpool));
	for (XU32 iIndex = 0; iIndex < m_nMaxConnect; ++iIndex)
	{		
		if(XSUCC != XOS_MutexCreate(&mysql_connectionpool.st[iIndex].dbConnLock))
		{
			g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"XOS_MutexCreate dbConnLock fail!,index = %d\n",iIndex);
		}
	}
	if(XSUCC != XOS_MutexCreate(&m_ConnPoolLock))
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"XOS_MutexCreate m_ConnPoolLock fail!\n");
	}
	XOS_MutexLock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
	m_bInit = false;   	
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
}

CDbServiceMysql::~CDbServiceMysql(void)
{
	g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbDestroyConnection \n");
	//ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	//XOS_MutexLock(&m_ConnPoolLock);
	for (unsigned int iIndex = 0; iIndex < m_nMaxConnect; ++iIndex)
	{		
		XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
		mysql_close( &mysql_connectionpool.st[iIndex].mySql );
		XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
	}
	//XOS_MutexUnlock(&m_ConnPoolLock);
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
XS32 CDbServiceMysql::DbInit(XU8* env, XU32 fid)
{
	//������mod������Ҫ������������Ӹ���
	SDllEnv* pDllEnv = (SDllEnv*)env;
	if(pDllEnv->mod > MYSQL_CONNECT_POOL_MAX_COUNT)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbInit error pDllEnv->mod=%d\n",pDllEnv->mod);
		return XERROR;
	}
	if(pDllEnv->mod <= 0 )
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbInit error pDllEnv->mod=%d\n",pDllEnv->mod);
		m_nMaxConnect = MYSQL_CONNECT_POOL_MAX_COUNT;
	}
	else
	{
		m_nMaxConnect = pDllEnv->mod;
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
XS32 CDbServiceMysql::DbDestroy(XU32 fid)
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
XS32 CDbServiceMysql::DbInitConnection(XS8* user, XS8* pwd, XS8 *connstring,XS8* parmeter,bool isDefault, XU32 fid)
{
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbInitConnection begin:user=%s,pwd=%s,connstring=%s\n",user,pwd,connstring);
	
	unsigned int iIndex = 0;

	XS8 *dbname = connstring;
	XS8* host = parmeter;
	int  dbport = m_defDbPort;
	//ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	XOS_MutexLock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");


	if (0 < mysql_connectionpool.in_use_Count) // mysql_connectionpool.in_use_Count must be zero!!!
	{	
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---mysql_connectionpool.in_use_Count >0 .\n");
		return XERROR;
	}
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");

	for (iIndex = 0; iIndex < m_nMaxConnect; ++iIndex)
	{
		XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
		MYSQL *pMysql = mysql_init(&mysql_connectionpool.st[iIndex].mySql);
		if (NULL == pMysql)
		{			
			 g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---Init mysql connect handle failed.\n");
			 XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock);
			 g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
			 return ERROR_CODE_DB_NULL_PTR;
		}
		mysql_connectionpool.st[iIndex].in_use = 0;
		mysql_connectionpool.st[iIndex].mySql.reconnect = 1; /* ����Ϊ�Զ����� */
		
		//mysql_options(&mysql_connectionpool.st[iIndex].mySql, MYSQL_OPT_RECONNECT,0);	
	
		pMysql = mysql_real_connect(&mysql_connectionpool.st[iIndex].mySql, host, user,pwd,dbname,dbport,NULL,CLIENT_FOUND_ROWS);
		if (pMysql == NULL)
		{
			XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock);
			 g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
			g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---Error:mysql_query is error! %s",TraceMysqlLastError(iIndex).c_str());
	   		return ERROR_CODE_DB_CONN_FAIL;
 		}
		//�����Զ�����
		//my_bool my_true = 1;
		//mysql_options(&mysql_connectionpool.st[iIndex].mySql,MYSQL_OPT_RECONNECT, &my_true);
		// �ر��Զ��ύģʽ
		mysql_autocommit(&mysql_connectionpool.st[iIndex].mySql, 0);
		XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
	}
	if(isDefault)
	{
		m_defUserName = string(user);
		m_defUserPwd= string(pwd);
		m_defDbName= string(connstring);
		m_defHost= string(parmeter);
	}
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbInitConnection end:user=%s,pwd=%s,ip=%s, dbname =%s.\n",
		user,pwd, parmeter, connstring);
    XOS_MutexLock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
	m_bInit = true;
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
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
XS32 CDbServiceMysql::DbDestroyConnection(XU32 fid)
{	
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbDestroyConnection \n");
	//ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	
	XOS_MutexLock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
	m_bInit = false;   	
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");

	for (unsigned int iIndex = 0; iIndex < m_nMaxConnect; ++iIndex)
	{		
		XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
		mysql_close( &mysql_connectionpool.st[iIndex].mySql );
		mysql_connectionpool.st[iIndex].in_use = 0;
		XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n",iIndex);
	}

	XOS_MutexLock(&m_ConnPoolLock);	 
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
	mysql_connectionpool.in_use_Count = 0;
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");

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
XS32 CDbServiceMysql::DbCreateConn(XU32 fid)
{

	if(!m_bInit)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbCreateConn(),mysql not init!\n");
		return ERROR_CODE_DB_CONN_FAIL;
	}
	XS32 dbConnId = ERROR_CODE_DB_CONN_FAIL;
	//ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );
	XOS_MutexLock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
	if (m_nMaxConnect <= mysql_connectionpool.in_use_Count)
	{
		XOS_MutexUnlock(&m_ConnPoolLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbCreateConn(),mysql connectpool is full!\n");
		return ERROR_CODE_DB_CONN_POOL_IS_FULL;
	}
	XOS_MutexUnlock(&m_ConnPoolLock);
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
  	unsigned int iIndex = 0;
	for (iIndex = 0; iIndex < m_nMaxConnect; ++iIndex)  /* 0 ��ҵ����*/
	{
		if (0 == mysql_connectionpool.st[iIndex].in_use)
		{
			dbConnId = iIndex;	

			XOS_MutexLock(&m_ConnPoolLock);
			g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
			mysql_connectionpool.in_use_Count++;	
			XOS_MutexUnlock(&m_ConnPoolLock);
			g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
			
			//�ҵ��ͼ��������ⱻ�����̷߳�����
			XOS_MutexLock(&mysql_connectionpool.st[dbConnId].dbConnLock);
			g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", iIndex);
			mysql_connectionpool.st[iIndex].in_use = 1;	
			mysql_connectionpool.st[iIndex].fid = fid;	 
			
		
			break;
		}
	}

	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbCreateConn(),dbConnId=%d!\n",dbConnId);

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
XS32 CDbServiceMysql::DbCreateConn(XS8 *user,XS8 *pwd, XU32 fid)
{
	return DbCreateConn();		
}
/**********************************************************************
*
*  NAME:          DBCheckConnection
*  FUNTION:       ��֤�����ӺŶ�Ӧ�������Ƿ����
*  INPUT:         XS32 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC;
				  XERROR;
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DBCheckConnection(XU32 dbConnId, XU32 fid)
{	
	if (m_nMaxConnect<= dbConnId)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---MYSQL_Connection does not exist,error, dbConnId=%d!\n", dbConnId);
		return XERROR;
	}
	else	
	{
		//ACE_DEBUG((LM_DEBUG,"(%N:%l:%D)---MYSQL_Connection exsit,success, dbConnId=%d!\n", dbConnId));	
		//mysql_ping(&mysql_connectionpool.st[dbConnId].mySql);
		return XSUCC;
	}
	
}

/**********************************************************************
*
*  NAME:          DBCheckConnection
*  FUNTION:       check���е������Ƿ�������
*  INPUT:         
*  OUTPUT:        XSUCC;
				  XERROR;
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DBCheckConnection()
{
	if( !m_bInit )
	{
        g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbCreateConn(),mysql not init!\n");
        return ERROR_CODE_DB_CONN_FAIL;
	}
	
	for( unsigned int uiIndex = 0; uiIndex < m_nMaxConnect; uiIndex++ )
	{
		XOS_MutexLock(&mysql_connectionpool.st[uiIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", uiIndex);
        mysql_ping(&mysql_connectionpool.st[uiIndex].mySql);
		XOS_MutexUnlock(&mysql_connectionpool.st[uiIndex].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", uiIndex);
	}
	return XSUCC;
}
/**********************************************************************
*
*  NAME:          DbDestroyConn
*  FUNTION:       �����ӳ��У��ͷ����ݿ�����
*  INPUT:         XS32 dbConnId�����Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DbDestroyConn(XU32 dbConnId, XU32 fid)
{
		
//	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_ConnPoolLock );

	if ( m_nMaxConnect<= dbConnId)
	{
		//XOS_MutexUnlock(mysql_connectionpool.st[dbConnId].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---CDbServiceMysql::DbDestroyConn()free a  mysql connect(%d) error!\n", dbConnId);
		return XERROR;
	}
	
	if (1 == mysql_connectionpool.st[dbConnId].in_use)
	{
		mysql_connectionpool.st[dbConnId].in_use = 0;
		XOS_MutexUnlock(&mysql_connectionpool.st[dbConnId].dbConnLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&mysql_connectionpool.st[iIndex].dbConnLock) i=%d.\n", dbConnId);

		
		XOS_MutexLock(&m_ConnPoolLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexLock(&m_ConnPoolLock).\n");
		mysql_connectionpool.in_use_Count--;	
		XOS_MutexUnlock(&m_ConnPoolLock);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_MIN),"(%N:%l:%D)---DBCFG XOS_MutexUnlock(&m_ConnPoolLock).\n");
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---CDbServiceMysql::DbDestroyConn()free a  mysql connect(%d) success!\n", dbConnId);

	}
	
	
	return XSUCC;
}

/**********************************************************************
*
*  NAME:          DbCommit
*  FUNTION:       
*  INPUT:         XS32 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DbCommit(XU32 dbConnId, XU32 fid)
{
	XS32 nRet = DBCheckConnection(dbConnId, fid);
	if(XSUCC != nRet)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---CDbServiceMysql::DbCommit() mysql connect(%d) error!\n", dbConnId);
		return nRet;
	}
	return mysql_commit(&mysql_connectionpool.st[dbConnId].mySql);
	
}
/**********************************************************************
*
*  NAME:          DbRollback
*  FUNTION:       
*  INPUT:         XS32 dbConnId:���Ӻ�
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DbRollback(XU32 dbConnId, XU32 fid)
{
	XS32 nRet = DBCheckConnection(dbConnId, fid);
	if(XSUCC != nRet)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---CDbServiceMysql::DbCommit() mysql connect(%d) error!\n", dbConnId);
		return nRet;
	}
	return mysql_rollback(&mysql_connectionpool.st[dbConnId].mySql);
}

/**********************************************************************
*
*  NAME:          DbCreateStatement
*  FUNTION:       ����һ��Statement��������ע��CDbStatement��
*  INPUT:         XS32 dbConnId�����Ӻ�,
				  XS8* sql�������sql���
				  				  
*  OUTPUT:        CDbStatement* pStatment�����CDbStatement��ָ�룬��SetStatement()ʵ�ֽ�Statement*ע�뵽CDbStatement��
*  OTHERS:        ������˵��������
**********************************************************************/
CDbStatement* CDbServiceMysql::DbCreateStatement(XU32 dbConnId,XS8* sql, XU32 fid)
{	
	if (sql == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbCreateStatement fail,sql is null.dbConnId=%d.\n", dbConnId);
		return NULL;
	}
	g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---DbCreateStatement dbConnId: %d,sql =%s.\n", dbConnId, sql);
	MYSQL_Connection * pConn = GetConnValue(dbConnId);
	if (pConn == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbCreateStatement fail, conn is null.dbConnId=%d.\n", dbConnId);
		return NULL;
	}
	MYSQL_STMT * pStmt = mysql_stmt_init(pConn);
	if (pStmt == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbCreateStatement fail, stmt is null.dbConnId=%d.\n", dbConnId);
		return NULL;
	}

	// ׼��sql
	int nRet = mysql_stmt_prepare(pStmt, sql, strlen(sql));
	if (nRet != 0)
	{
		//��MYSQL_Connection����һ��Statement��	
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbCreateStatement prepare fail, dbConnId=%d, sql =%s.\n", dbConnId, sql);
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbCreateStatement prepare fail! %s\n",TraceMysqlLastError(dbConnId).c_str());
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
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_DBG),"(%N:%l:%D)---DbCreateStatement new  CDbStatementMysql is NULL ,dbConnId: %d,sql =%s.\n", dbConnId, sql);
		return NULL;	
	}
	
		
}
/**********************************************************************
*
*  NAME:          DbDestroyStatement
*  FUNTION:       ��ֹ��Statement
*  INPUT:         XS32 dbConnId�����Ӻ�,
				  CDbStatement* pStatment�����봦����CDbStatement��ָ�룬����GetStatement()������Ӧ��Statement��ָ��
*  OUTPUT:        XSUCC(0);XERROR(-1)
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::DbDestroyStatement(XU32 dbConnId,CDbStatement* pStatment, XU32 fid)
{
	if (pStatment == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		g_callbackTraceFunOfMySqlDb(NULL, MD(fid,PL_ERR),"(%N:%l:%D)---DbDestroyStatement fail,pStatment is null.dbConnId=%d.\n", dbConnId);
		return XERROR;
	}
	
	XS32 nRet = mysql_stmt_close((MYSQL_STMT *) pStatment->GetStatement());
	
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
*  INPUT:         XS32 dbConnId:���Ӻ�
*  OUTPUT:        MYSQL_Connection** conn
*  OTHERS:        
**********************************************************************/
MYSQL_Connection* CDbServiceMysql::GetConnValue(XU32 dbConnId, XU32 fid)
{
	if(XSUCC != DBCheckConnection(dbConnId, fid))
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
*  INPUT:         XS32 dbConnId:���Ӻ�
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
string CDbServiceMysql::TraceMysqlLastError(XU32 dbConnId)
{
	MYSQL_Connection * pDbConn = GetConnValue(dbConnId);
	if(NULL == pDbConn )
	{
		string str = "error connId " + dbConnId;
		
		return str;
	}
	return  string(mysql_error(pDbConn));
}



/**********************************************************************
*
*  NAME:          GetString
*  FUNTION:       �õ���Ҫ���ص�������Ϣ 
*  INPUT:         type������ʲô���أ�retStr Ҫ���صĴ���strLen�����ݵ�retStr��󳤶�
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XS32 CDbServiceMysql::GetString(XU32 type, XS8* retStr, XU32 strLen)
{
	XS8 sTmp[100] ={0};		

	if( !m_bInit )
	{
		XOS_Sprintf(sTmp, sizeof(sTmp), "db is not init\n");
		return ERROR_CODE_DB_CONN_FAIL;
	}


	XOS_MemSet(sTmp, 0, sizeof(sTmp));
	XOS_Sprintf(sTmp, sizeof(sTmp), "index     IsUsed     fid     totalCount=%d,dbInfo=%s\r\n", m_nMaxConnect, m_defHost.c_str());
	XOS_StrCat(retStr,sTmp); 	

	for( unsigned int uiIndex = 0; uiIndex < m_nMaxConnect; uiIndex++ )
	{
		if((XOS_StrLen(retStr) + sizeof(sTmp)) > strLen)
		{
			break;
		}
		XOS_MemSet(sTmp, 0, sizeof(sTmp));
		XOS_Sprintf(sTmp, sizeof(sTmp), "%5d     %6d     %d\r\n", uiIndex, mysql_connectionpool.st[uiIndex].in_use, mysql_connectionpool.st[uiIndex].fid);
		XOS_StrCat(retStr,sTmp); 		
	}
	
	return XSUCC;
}