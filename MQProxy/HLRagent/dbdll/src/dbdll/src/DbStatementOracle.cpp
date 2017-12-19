
#include "DbStatementOracle.h"
#include "ace/Log_Msg.h"
#include <occi.h>
using namespace oracle::occi;
CDbStatementOracle::CDbStatementOracle(XVOID)
{
	m_connId = 0;
	m_pStmtOracle = NULL;
	//创建CDbResultOracle,提供给结果集使用
	m_oracleResult = new CDbResultOracle();
}

CDbStatementOracle::~CDbStatementOracle(XVOID)
{
	m_connId = 0;
	//释放创建的m_oracleResult
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
*  FUNTION:       返回更新的行数
*  INPUT:         
*  OUTPUT:        返回XU32的值 
*  OTHERS:        
**********************************************************************/
XU32 CDbStatementOracle::GetDbUpdateCount()
{
	return m_pStmtOracle->getUpdateCount();
}
/**********************************************************************
*
*  NAME:          SetDbBytes
*  FUNTION:       将Bytes型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Bytes value：需要赋给该参数的值
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
*  FUNTION:       将XS32型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  XS32 value：需要赋给该参数的值
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
*  FUNTION:       将unsigned XS32型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  unsigned XS32 value：需要赋给该参数的值
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
*  FUNTION:       将string型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  XS32 value：需要赋给该参数的值
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
*  FUNTION:       将XD64型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  XD64 value：需要赋给该参数的值
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
*  FUNTION:       将Statement型的指针传入CDbStatement类中
*  INPUT:         XVOID *pStmt:传入Statement型的指针
				  
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
*  FUNTION:       将Date型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Date value：需要赋给该参数的值
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
*  FUNTION:       将Timestamp型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Timestamp value：需要赋给该参数的值
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
*  FUNTION:       将XF32型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Timestamp value：需要赋给该参数的值
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
*  FUNTION:       将XF32型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Timestamp value：需要赋给该参数的值
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
*  FUNTION:       将Clob型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Clob value：需要赋给该参数的值
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
*  FUNTION:       将Number型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Timestamp value：需要赋给该参数的值
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
*  FUNTION:       将Number型的value值传入动态sql语句中
*  INPUT:         
				  XS32 nCol:动态sql语句中的第几个参数 
				  Timestamp value：需要赋给该参数的值
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
*  FUNTION:       由应用层调用执行查询，将获得的ResultSet结果集注入CDbResult中
*  INPUT:         
				  
*  OUTPUT:        XSUCC;XERROR;oracle错误码
*  OTHERS:        
**********************************************************************/
 XS32 CDbStatementOracle::ExecuteQuery(CDbResult **pResult)
{
	*pResult = NULL;
	string strSql = m_pStmtOracle->getSQL();
	ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---connId id %d,ExecuteQuery sql is %s\n",m_connId,strSql.c_str()));

	ResultSet *pResultSet;
	//执行oracle的查询
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
		//设置虚拟数据库实现的数据库相关的结果集
		m_oracleResult->SetResultSet(pResultSet);
		//将虚拟数据库层的oracle具体的实现返回给应用层
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
*  FUNTION:       由应用层调用执行更新，当增、删、改时调用
*  INPUT:         
*  OUTPUT:        XSUCC(0);错误码
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
*  FUNTION:       返回注入的Statement的指针
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
*  FUNTION:       返回针对不同数据库平台定义的错误码
*  INPUT:         XS32 errorCode
*  OUTPUT:        错误码 
*  OTHERS:        
**********************************************************************/
XS32 CDbStatementOracle::GetErrorCode(XS32 errorCode)
{   
	XS32 tmpErroCode = 0;
	//可根据需要进行扩展，对返回的错误码进行处理
	switch(errorCode)
	{
		case 1:
			//暂时只考虑，插入数据时，键值冲突返回的code为1
			tmpErroCode = errorCode;
		default:
			//对于其它错误码，暂时不考虑，直接返回
			tmpErroCode = errorCode;
			break;

	}
	return tmpErroCode;

}
XVOID CDbStatementOracle::SetConnId(XU32 iConnId)
{
	m_connId = iConnId;
}