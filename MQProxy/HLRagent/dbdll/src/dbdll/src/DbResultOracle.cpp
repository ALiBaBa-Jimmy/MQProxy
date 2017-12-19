
#include "DbResultOracle.h"
#include "ace/Log_Msg.h"
#include <occi.h>
using namespace oracle::occi;
CDbResultOracle::CDbResultOracle(XVOID)
{
	m_pResultOracle = NULL;
}

CDbResultOracle::~CDbResultOracle(XVOID)
{
	//不用delete m_pResultOracle，由系统自己释放
}
/**********************************************************************
*
*  NAME:          GetDbInt
*  FUNTION:       返回sql语句中XS32型的value值
*  INPUT:          XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Bytes CDbResultOracle::GetDbBytes(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getBytes(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbBytes error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbInt
*  FUNTION:       返回sql语句中XS32型的value值
*  INPUT:          XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XS32 CDbResultOracle::GetDbInt(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getInt(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbInt error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbUInt
*  FUNTION:       返回sql语句中unsigned XS32型的value值
*  INPUT:          XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XU32 CDbResultOracle::GetDbUInt(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getUInt(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbUInt error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbString
*  FUNTION:       返回sql语句中string型的value值
*  INPUT:		  XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
string CDbResultOracle::GetDbString(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getString(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbString error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
	
}

/**********************************************************************
*
*  NAME:          GetDbChar
*  FUNTION:       返回sql语句中string型的value值
*  INPUT:		  XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbResultOracle::GetDbChar(XU32 nCol,XS8* pCharValue)
{
	try
	{
		strcpy(pCharValue,m_pResultOracle->getString(nCol).c_str());
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbChar error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
	
}

/**********************************************************************
*
*  NAME:          GetDbDouble
*  FUNTION:       返回sql语句中double型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XD64 CDbResultOracle::GetDbDouble(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getDouble(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbDouble error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbDate
*  FUNTION:       返回sql语句中Date型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Date CDbResultOracle::GetDbDate(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getDate(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbDate error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbTimeStamp
*  FUNTION:       返回sql语句中Timestamp型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Timestamp CDbResultOracle::GetDbTimeStamp(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getTimestamp(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbTimeStamp error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbFloat
*  FUNTION:       返回sql语句中XF32型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XF32 CDbResultOracle::GetDbFloat(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getFloat(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbFloat error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbBlob
*  FUNTION:       返回sql语句中Blob型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Blob CDbResultOracle::GetDbBlob(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getBlob(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbBlob error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbClob
*  FUNTION:       返回sql语句中Clob型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Clob CDbResultOracle::GetDbClob(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getClob(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbClob error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}
/**********************************************************************
*
*  NAME:          GetDbNumber
*  FUNTION:       返回sql语句中Number型的value值
*  INPUT:        XS32 nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
Number CDbResultOracle::GetDbNumber(XU32 nCol)
{
	try
	{
		return m_pResultOracle->getNumber(nCol);
	}
	catch(SQLException &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---GetDbNumber error,error msg is %s,error code is %d\n",e.what(),e.getErrorCode()));
	}
}

/**********************************************************************
*
*  NAME:          SetResultSet
*  FUNTION:       给该类的成员变量指针赋值
*  INPUT:         XVOID *pResultSet：传入ResultSet类型的指针 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XVOID CDbResultOracle::SetResultSet(XVOID *pResultSet)
{
	m_pResultOracle = (ResultSet*)pResultSet;
}
/**********************************************************************
*
*  NAME:          IsHaveNextRow
*  FUNTION:       判断该结果集是否还有后续输出
*  INPUT:         
*  OUTPUT:        若有后续记录输出，则返回XTRUE;无，返回XFALSE
*  OTHERS:        
**********************************************************************/
XBOOL CDbResultOracle::IsHaveNextRow(XVOID)
{
	//ACE_DEBUG((LM_DEBUG,"Entering IsHaveNextRow,status() is %d\n",m_pResultOracle->status()));
	XU8 cResultStatus = 0;
	try
	{
		
		/*if ( m_pResultOracle->status() == HAVE_DATA || m_pResultOracle->status() == HAVE_STREAM_DATA)
		{
			m_pResultOracle->next();
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving IsHaveNextRow,XTRUE\n"));
			return XTRUE;
		}
		else
		{
			ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving IsHaveNextRow,XFALSE\n"));
			return XFALSE;
		}*/
		cResultStatus = m_pResultOracle->next();
		if (cResultStatus == NO_DATA_FETCH)
		{
			//ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving IsHaveNextRow,XFALSE\n"));
			return XFALSE;
		}
		else
		{
			//ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---Leaving IsHaveNextRow,XTRUE\n"));
			return XTRUE;
		}
	}
	catch(SQLException &sqlE)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---IsHaveNextRow error,error msg is %s,error code is %d\n",sqlE.what(),sqlE.getErrorCode()));
		return XFALSE;
	}
	catch(exception &e)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---IsHaveNextRow error,error msg is %s!\n",e.what()));	
		return XFALSE;
	}
	catch (...)
	{
		ACE_DEBUG((LM_DEBUG,"[%t](%M:%N:%l:%D)---IsHaveNextRow error\n"));
		return XFALSE;
	}
	
}
