
#include "DbStatementOracle.h"
#include "ace/Log_Msg.h"
#include <occi.h>
using namespace oracle::occi;
CDbStatementOracle::CDbStatementOracle(XVOID)
{
	m_connId = 0;
	m_pStmtOracle = NULL;
	//����CDbResultOracle,�ṩ�������ʹ��
	m_oracleResult = new CDbResultOracle();
}

CDbStatementOracle::~CDbStatementOracle(XVOID)
{
	m_connId = 0;
	//�ͷŴ�����m_oracleResult
	if(NULL != m_oracleResult)
	{
		delete m_oracleResult;
		m_oracleResult = NULL;
	}
}
XVOID CDbStatementOracle::SetDbSql(XS8* sql)
{
	m_pStmtOracle->setSQL(sql);
}

XVOID CDbStatementOracle::CloseResultSet(CDbResult* re)
{
	if(re != NULL)
	{
		m_pStmtOracle->closeResultSet(static_cast<CDbResultOracle*>(re)->GetResultSet());
	}
}
/**********************************************************************
*
*  NAME:          GetDbUpdateCount
*  FUNTION:       ���ظ��µ�����
*  INPUT:         
*  OUTPUT:        ����XU32��ֵ 
*  OTHERS:        
**********************************************************************/
XU32 CDbStatementOracle::GetDbUpdateCount()
{
	return m_pStmtOracle->getUpdateCount();
}
/**********************************************************************
*
*  NAME:          SetDbBytes
*  FUNTION:       ��Bytes�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Bytes value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbBytes(XU32 nCol, Bytes value)
{
	try
	{
		return m_pStmtOracle->setBytes(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbBytes error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbInt
*  FUNTION:       ��XS32�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  XS32 value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbInt(XU32 nCol, XS32 value)
{
	try
	{
		m_pStmtOracle->setInt(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbInt error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbUInt
*  FUNTION:       ��unsigned XS32�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  unsigned XS32 value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbUInt(XU32 nCol, XU32 value)
{
	try
	{
		m_pStmtOracle->setUInt(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbUInt error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}

}
/**********************************************************************
*
*  NAME:          SetDbString
*  FUNTION:       ��string�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  XS32 value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbString(XU32 nCol, string value)
{
	try
	{
		m_pStmtOracle->setString(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbString error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}

}
/**********************************************************************
*
*  NAME:          SetDbString
*  FUNTION:       ��XD64�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  XD64 value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbDouble(XU32 nCol, XD64 value)
{
	try
	{
		m_pStmtOracle->setDouble(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbDouble error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}

/**********************************************************************
*
*  NAME:          SetStatement
*  FUNTION:       ��Statement�͵�ָ�봫��CDbStatement����
*  INPUT:         XVOID *pStmt:����Statement�͵�ָ��
				  
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetStatement(XVOID *pStmt)
{
	m_pStmtOracle = (Statement*)pStmt;
}
/**********************************************************************
*
*  NAME:          SetDbDate
*  FUNTION:       ��Date�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Date value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbDate(XU32 nCol, Date value)
{
	try
	{
		m_pStmtOracle->setDate(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbDate error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbTimestamp
*  FUNTION:       ��Timestamp�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbTimestamp(XU32 nCol, Timestamp value)
{
	try
	{
		m_pStmtOracle->setTimestamp(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbTimestamp error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbFloat
*  FUNTION:       ��XF32�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbFloat(XU32 nCol, XF32 value)
{
	try
	{
		m_pStmtOracle->setFloat(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbFloat error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbFloat
*  FUNTION:       ��XF32�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbBlob(XU32 nCol, Blob value)
{
	try
	{
		m_pStmtOracle->setBlob(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbBlob error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbFloat
*  FUNTION:       ��Clob�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Clob value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbClob(XU32 nCol, Clob value)
{
	try
	{
		m_pStmtOracle->setClob(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbClob error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          SetDbNumber
*  FUNTION:       ��Number�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbNumber(XU32 nCol, Number value)
{
	try
	{
		m_pStmtOracle->setNumber(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbNumber error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}

/**********************************************************************
*
*  NAME:          SetDbChar
*  FUNTION:       ��Number�͵�valueֵ���붯̬sql�����
*  INPUT:         
				  XS32 nCol:��̬sql����еĵڼ������� 
				  Timestamp value����Ҫ�����ò�����ֵ
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbStatementOracle::SetDbChar(XU32 nCol, XS8* value)
{
	try
	{
		m_pStmtOracle->setString(nCol,value);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---SetDbChar error!,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          ExecuteQuery
*  FUNTION:       ��Ӧ�ò����ִ�в�ѯ������õ�ResultSet�����ע��CDbResult��
*  INPUT:         
				  
*  OUTPUT:        XSUCC;XERROR;oracle������
*  OTHERS:        
**********************************************************************/
 XS32 CDbStatementOracle::ExecuteQuery(CDbResult **pResult)
{
	*pResult = NULL;
	string strSql = m_pStmtOracle->getSQL();
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---connId id %d,ExecuteQuery sql is %s\n",m_connId,strSql.c_str()));

	ResultSet *pResultSet;
	//ִ��oracle�Ĳ�ѯ
	try
	{
		//ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---executeQuery\n"));
		pResultSet = m_pStmtOracle->executeQuery();
		//ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---executeQuery success\n"));

	}catch(SQLException e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---error message is %s,errorcode is %d\n",e.what(),e.getErrorCode()));
		return XERROR;
	}
	
	if (pResultSet != NULL)
	{
		//�����������ݿ�ʵ�ֵ����ݿ���صĽ����
		m_oracleResult->SetResultSet(pResultSet);
		//���������ݿ���oracle�����ʵ�ַ��ظ�Ӧ�ò�
		*pResult = m_oracleResult;
		return XSUCC;
		
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---ExecuteQuery is error\n"));
		return XERROR;
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
XS32 CDbStatementOracle::ExecuteUpdate(XVOID)
{
	string strSql = m_pStmtOracle->getSQL();
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---connId id %d,ExecuteUpdate sql is %s\n",m_connId,strSql.c_str()));

	try
	{
		m_pStmtOracle->executeUpdate();
	}
	catch(SQLException &e)
	{
		XS32 nErr = GetErrorCode(e.getErrorCode());
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---ExecuteUpdate is error,error message is %s,errorcode is %d\n",e.what(),nErr));
		return nErr;
	}
	return XSUCC;
	
}
/**********************************************************************
*
*  NAME:          GetStatement
*  FUNTION:       ����ע���Statement��ָ��
*  INPUT:         
*  OUTPUT:        Statement* 
*  OTHERS:        
**********************************************************************/
XVOID* CDbStatementOracle::GetStatement(XVOID)
{
	return (XVOID*)m_pStmtOracle;
}
/**********************************************************************
*
*  NAME:          GetErrorCode
*  FUNTION:       ������Բ�ͬ���ݿ�ƽ̨����Ĵ�����
*  INPUT:         XS32 errorCode
*  OUTPUT:        ������ 
*  OTHERS:        
**********************************************************************/
XS32 CDbStatementOracle::GetErrorCode(XS32 errorCode)
{   
	XS32 tmpErroCode = 0;
	//�ɸ�����Ҫ������չ���Է��صĴ�������д���
	switch(errorCode)
	{
		case 1:
			//��ʱֻ���ǣ���������ʱ����ֵ��ͻ���ص�codeΪ1
			tmpErroCode = errorCode;
		default:
			//�������������룬��ʱ�����ǣ�ֱ�ӷ���
			tmpErroCode = errorCode;
			break;

	}
	return tmpErroCode;

}
XVOID CDbStatementOracle::SetConnId(XU32 iConnId)
{
	m_connId = iConnId;
}