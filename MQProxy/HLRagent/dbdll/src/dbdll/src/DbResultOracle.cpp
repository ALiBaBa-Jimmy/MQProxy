
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
	//����delete m_pResultOracle����ϵͳ�Լ��ͷ�
}
/**********************************************************************
*
*  NAME:          GetDbInt
*  FUNTION:       ����sql�����XS32�͵�valueֵ
*  INPUT:          XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����XS32�͵�valueֵ
*  INPUT:          XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����unsigned XS32�͵�valueֵ
*  INPUT:          XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����string�͵�valueֵ
*  INPUT:		  XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����string�͵�valueֵ
*  INPUT:		  XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����double�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����Date�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����Timestamp�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����XF32�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����Blob�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����Clob�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����Number�͵�valueֵ
*  INPUT:        XS32 nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ������ĳ�Ա����ָ�븳ֵ
*  INPUT:         XVOID *pResultSet������ResultSet���͵�ָ�� 
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
*  FUNTION:       �жϸý�����Ƿ��к������
*  INPUT:         
*  OUTPUT:        ���к�����¼������򷵻�XTRUE;�ޣ�����XFALSE
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
