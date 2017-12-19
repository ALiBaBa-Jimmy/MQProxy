
#include "DbStatementMysql.h"
#include "ace/Log_Msg.h"
#include "DbErrorCode.h"
CDbStatementMysql::CDbStatementMysql(void)
{
	m_pStmtMysql = NULL;
	//����CDbResultMysql,�ṩ�������ʹ��
	m_mysqlResult = new CDbResultMysql();
	memset(m_bind, 0, sizeof(m_bind)); 
	m_bBindCount = 0;
}

CDbStatementMysql::~CDbStatementMysql(void)
{
	//�ͷŴ�����m_mysqlResult
	if(NULL != m_mysqlResult)
	{
		delete m_mysqlResult;
		m_mysqlResult = NULL;
	}
}
void CDbStatementMysql::SetDbSql(XS8* sql)
{
	//m_pStmtMysql->setSQL(sql);
}
/**********************************************************************
*
*  NAME:          GetDbUpdateCount
*  FUNTION:       ���ظ��µ�����
*  INPUT:         
*  OUTPUT:        ����XU32��ֵ 
*  OTHERS:        
**********************************************************************/
XU32 CDbStatementMysql::GetDbUpdateCount()
{
	return 	(XU32)mysql_stmt_affected_rows(m_pStmtMysql);

}

/**********************************************************************
*
*  NAME:          SetDbInt
*  FUNTION:       ��int�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  int value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbInt(XU32 nCol, XS32 value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(XS32));
    memset(m_bindValue[nCol], 0, sizeof(XS32));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_LONG;  
    m_bind[nCol]. buffer			= m_bindValue[nCol];   
    m_bind[nCol]. buffer_length	= sizeof(XS32);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;     
	m_bindLength[nCol] = sizeof(XS32);
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol]; 
	
	*(XS32*)m_bindValue[nCol] = value;



	
	
}
/**********************************************************************
*
*  NAME:          SetDbUInt
*  FUNTION:       ��unsigned int�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  unsigned int value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbUInt(XU32 nCol, XU32 value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(XU32));
    memset(m_bindValue[nCol], 0, sizeof(XU32));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_LONG;  
    m_bind[nCol]. buffer			= m_bindValue[nCol];  
    m_bind[nCol]. buffer_length	= sizeof(XU32);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bindLength[nCol] = sizeof(XU32);
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];   
	
	
	*(XU32*)m_bindValue[nCol] = value;

}
/**********************************************************************
*
*  NAME:          SetDbString
*  FUNTION:       ��string�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  int value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbString(XU32 nCol, string value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(value.length() +1);
    memset(m_bindValue[nCol], 0 , (value.length() +1));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_STRING;  
	m_bind[nCol]. buffer			= m_bindValue[nCol];  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];   
	m_bindLength[nCol] = value.length()+1;	
	
	memcpy(m_bindValue[nCol], value.c_str(), value.length());

}
/**********************************************************************
*
*  NAME:          SetDbDouble
*  FUNTION:       ��double�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  double value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbDouble(XU32 nCol, double value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(double));
    memset(m_bindValue[nCol], 0, sizeof(double));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_DOUBLE;  
	m_bind[nCol]. buffer			= (char *)&value;  
	m_bind[nCol]. buffer_length	= sizeof(double);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];      
	m_bindLength[nCol] = sizeof(double);
	*(double*)m_bindValue[nCol] = value;
}

/**********************************************************************
*
*  NAME:          SetStatement
*  FUNTION:       ��Statement�͵�ָ�봫��CDbStatement����
*  INPUT:         void *pStmt:����Statement�͵�ָ��
				  
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetStatement(void *pStmt)
{
	m_pStmtMysql = (MYSQL_STMT*)pStmt;
}

/**********************************************************************
*
*  NAME:          SetDbFloat
*  FUNTION:       ��float�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbFloat(XU32 nCol, float value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(float));
    memset(m_bindValue[nCol], 0, sizeof(float));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_FLOAT;  
	m_bind[nCol]. buffer			= (char *)&value;  
	m_bind[nCol]. buffer_length	= sizeof(float);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];  
	m_bindLength[nCol] = sizeof(float);
	*(float*)m_bindValue[nCol] = value;
	
}

/**********************************************************************
*
*  NAME:          SetDbLong
*  FUNTION:       ��long�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  long value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbLong(XU32 nCol, long value)
{
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(long));
    memset(m_bindValue[nCol], 0, sizeof(long));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_LONG;  
	m_bind[nCol]. buffer			= (char *)&value;  
	m_bind[nCol]. buffer_length	= sizeof(long);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];   
	m_bindLength[nCol] = sizeof(long);
	*(long*)m_bindValue[nCol] = value;
	

}
/**********************************************************************
*
*  NAME:          SetDbULong
*  FUNTION:       ��unsigned long�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  unsigned long value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbULong(XU32 nCol, unsigned long value)
{	
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	m_bindValue[nCol] = (char*)malloc(sizeof(unsigned long));
    memset(m_bindValue[nCol], 0, sizeof(unsigned long));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_LONG;  
	m_bind[nCol]. buffer			= (char *)&value;  
	m_bind[nCol]. buffer_length	= sizeof(unsigned long);  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];   
	m_bindLength[nCol] = sizeof(unsigned long);
	*(unsigned long*)m_bindValue[nCol] = value;
	
}
/**********************************************************************
*
*  NAME:          SetDbChar
*  FUNTION:       ��Number�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  int nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetDbChar(XU32 nCol, XS8* value)
{	
	m_bBindCount = (m_bBindCount > nCol)?m_bBindCount:nCol;
	nCol -= 1;
	XU32 nStrLen = strlen(value);
	m_bindValue[nCol] = (char*)malloc(nStrLen+1);
     memset(m_bindValue[nCol], 0, sizeof(nStrLen+1));
	m_bind[nCol]. buffer_type	= MYSQL_TYPE_STRING;  
	m_bind[nCol]. buffer			= m_bindValue[nCol];  
    m_bind[nCol]. is_null		=  (my_bool*) 0;   
    m_bind[nCol]. length			= (unsigned long*)&m_bindLength[nCol];   
	m_bindLength[nCol] = nStrLen;
	
	memset(m_bindValue[nCol], 0 , (nStrLen +1));
	memcpy(m_bindValue[nCol], value, nStrLen);

	
	
}
/**********************************************************************
*
*  NAME:          ExecuteQuery
*  FUNTION:       ��Ӧ�ò����ִ�в�ѯ������õ�ResultSet�����ע��CDbResult��
*  INPUT:         
				  
*  OUTPUT:        XSUCC;XERROR;oracle������
*  OTHERS:        
**********************************************************************/
 XS64 CDbStatementMysql::ExecuteQuery(CDbResult **pResult)
{
	if (m_pStmtMysql == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"ExecuteQuery fail."));
		return ERROR_CODE_DB_NULL_PTR;
	}
	if(m_bBindCount > 0)
	{
		my_bool bRet = mysql_stmt_bind_param(m_pStmtMysql, m_bind);
		//if(bRet)  
		{          
		   ACE_ERROR((LM_ERROR,"mysql_stmt_bind_param fail,%s.", mysql_stmt_error(m_pStmtMysql)));
		  // return XERROR;
		}
		ACE_ERROR((LM_ERROR,"mysql_stmt_bind_param bRet=%d.", bRet));

	
	}

	if (mysql_query(m_pStmtMysql->mysql, "SET NAMES 'gb2312'"))   /*��ѯ�����������*/
	{
		ACE_ERROR((LM_ERROR,"mysql_query fail,error %s.",mysql_stmt_error(m_pStmtMysql)));
		mysql_close(m_pStmtMysql->mysql);
		return 	XERROR;/*mysql_error(&conn); ���ش����*/		
	}

	//int nRet = mysql_stmt_execute(m_pStmtMysql);	
	int nRet = mysql_query(m_pStmtMysql->mysql, m_strSql.c_str());	
	if(nRet != 0)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"ExecuteQuery fail,result =%d, error %s.", nRet, mysql_stmt_error(m_pStmtMysql)));
		return nRet;
	}
	int nRetCol = mysql_field_count(m_pStmtMysql->mysql);
	if(nRetCol <=0 )
	{
		ACE_ERROR((LM_ERROR,"ExecuteQuery return col is  %d.\n", nRetCol));
		return ERROR_CODE_DB_NULL_PTR;
	}
	int rowCnt = mysql_stmt_affected_rows(m_pStmtMysql);
	ACE_DEBUG((LM_DEBUG,"ExecuteQuery return col is  %d, row is %d.\n",nRetCol,rowCnt));

	MYSQL_RES * pResultSet = mysql_store_result(m_pStmtMysql->mysql);	
	if (pResultSet != NULL)
	{		
		//�����������ݿ�ʵ�ֵ����ݿ���صĽ����
		m_mysqlResult->SetResultSet(pResultSet);
		//���������ݿ���oracle�����ʵ�ַ��ظ�Ӧ�ò�
		*pResult = m_mysqlResult;
		return XSUCC;
		
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"ExecuteQuery is result is null,error =%d, reason =%s.\n", mysql_errno(m_pStmtMysql->mysql), mysql_error(m_pStmtMysql->mysql)));
		return ERROR_CODE_DB_NULL_PTR;
	}


}
/**********************************************************************
*
*  NAME:          ExecuteUpdate
*  FUNTION:       ��Ӧ�ò����ִ�и��£�������ɾ����ʱ����
*  INPUT:         
*  OUTPUT:        XSUCC(0);������
*  OTHERS:        
**********************************************************************/
XS64 CDbStatementMysql::ExecuteUpdate(void)
{
	if (m_pStmtMysql == NULL)
	{
		//��MYSQL_Connection����һ��Statement��
		ACE_ERROR((LM_ERROR,"ExecuteUpdate fail."));
		return ERROR_CODE_DB_NULL_PTR;
	}
	if(m_bBindCount > 0)
	{
		mysql_stmt_bind_param(m_pStmtMysql, m_bind);
	}

	if (mysql_query(m_pStmtMysql->mysql, "SET NAMES 'gb2312'"))   /*��ѯ�����������*/
	{
		ACE_ERROR((LM_ERROR,"mysql_query fail,error %s.",mysql_stmt_error(m_pStmtMysql)));
		mysql_close(m_pStmtMysql->mysql);
		return 	XERROR;/*mysql_error(&conn); ���ش����*/		
	}
	int nRet = mysql_stmt_execute(m_pStmtMysql);
	if(nRet != 0 )
	{
		ACE_ERROR((LM_ERROR,"ExecuteQuery  reason =%s.\n", mysql_error(m_pStmtMysql->mysql)));
		return ERROR_CODE_DB_NULL_PTR;
	}
	int rowCnt = mysql_stmt_affected_rows(m_pStmtMysql);
	ACE_DEBUG((LM_DEBUG,"ExecuteQuery  row is %d.\n",rowCnt));

	
	
	return nRet;
	
}
/**********************************************************************
*
*  NAME:          GetStatement
*  FUNTION:       ����ע���Statement��ָ��
*  INPUT:         
*  OUTPUT:        Statement* 
*  OTHERS:        
**********************************************************************/
void* CDbStatementMysql::GetStatement(void)
{
	return (void*)m_pStmtMysql;
}
/**********************************************************************
*
*  NAME:          GetErrorCode
*  FUNTION:       ������Բ�ͬ���ݿ�ƽ̨����Ĵ�����
*  INPUT:         XS32 errorCode
*  OUTPUT:        ������ 
*  OTHERS:        
**********************************************************************/
XS32 CDbStatementMysql::GetErrorCode(XS32 errorCode)
{   
	XS32 tmpErroCode = 0;
	////�ɸ�����Ҫ������չ���Է��صĴ�������д���
	//switch(errorCode)
	//{
	//	case 1:
	//		//��ʱֻ���ǣ���������ʱ����ֵ��ͻ���ص�codeΪ1
	//		tmpErroCode = errorCode;
	//	default:
	//		//�������������룬��ʱ�����ǣ�ֱ�ӷ���
	//		tmpErroCode = errorCode;
	//		break;

	//}
	return tmpErroCode;

}

/**********************************************************************
*
*  NAME:          SetBindParmCount
*  FUNTION:      ����Ԥ��������ĸ���,myssqlʹ��
*  INPUT:         XU32 nCount
*  OUTPUT:        ������ 
*  OTHERS:        
**********************************************************************/
void CDbStatementMysql::SetBindParmCount(XU32 nCount)
{
	m_bBindCount = nCount;
}
