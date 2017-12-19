
#include "DbResultMysql.h"
#include "DbCommon.h"
#include "fid_def.h"
CDbResultMysql::CDbResultMysql(void)
{
	m_pResultMysql = NULL;
	m_mysqlRow = NULL;
	//m_pFieldLen = new unsigned long(100);
	m_lNumField = 0;
	m_pField = NULL;
	m_mysqllengths = NULL;
}

CDbResultMysql::~CDbResultMysql(void)
{
	//不用delete m_pResultMysql，由系统自己释放
	/*if(m_pFieldLen != NULL)
	{
		delete []m_pFieldLen;
	}*/
	 
}
/**********************************************************************
*
*  NAME:          GetDbTiny
*  FUNTION:       返回sql语句中tinyint型的value值
*  INPUT:          int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XS8 CDbResultMysql::GetDbTiny(XU32 nCol)
{
	XS8 value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbTiny,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}	
	if(NULL != m_mysqlRow)
	{
		value = (XS8)atoi(m_mysqlRow[nCol]);
	}

	return value;
}
/**********************************************************************
*
*  NAME:          GetDbShort
*  FUNTION:       返回sql语句中short型的value值
*  INPUT:          int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XS16 CDbResultMysql::GetDbShort(XU32 nCol)
{
	XS16 value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbShort,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}

	if(NULL != m_mysqlRow)
	{
		value = (XS16)atoi(m_mysqlRow[nCol]);
	}

	return value;
}
/**********************************************************************
*
*  NAME:          GetDbInt
*  FUNTION:       返回sql语句中int型的value值
*  INPUT:          int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XS32 CDbResultMysql::GetDbInt(XU32 nCol)
{
	XS32 value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbInt,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	if(NULL != m_mysqlRow)
	{
		value = (XS32)atoi(m_mysqlRow[nCol]);
	}

	
	return value;
}
/**********************************************************************
*
*  NAME:          GetDbUInt
*  FUNTION:       返回sql语句中unsigned int型的value值
*  INPUT:          int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
XU32 CDbResultMysql::GetDbUInt(XU32 nCol)
{
	XS32 value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbUInt,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	
	if(NULL != m_mysqlRow)
	{
		value = (XU32)atoi(m_mysqlRow[nCol]);
	}
	return value;
}
/**********************************************************************
*
*  NAME:          GetDbString
*  FUNTION:       返回sql语句中string型的value值
*  INPUT:		  int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
string CDbResultMysql::GetDbString(XU32 nCol)
{	
	string s;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbString,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return string("");
	}
	
	if(NULL != m_mysqlRow)
	{
		s = string (m_mysqlRow[nCol]);	
	}
	return s;
	
}

/**********************************************************************
*
*  NAME:          GetDbChar
*  FUNTION:       返回sql语句中string型的value值
*  INPUT:		  int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbResultMysql::GetDbChar(XU32 nCol,XS8* pCharValue)
{
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbChar,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return;
		
	}	
	if(NULL != m_mysqlRow)
	{
		memcpy(pCharValue, m_mysqlRow[nCol], strlen(m_mysqlRow[nCol]));
	}
	
}

/**********************************************************************
*
*  NAME:          GetDbDouble
*  FUNTION:       返回sql语句中double型的value值
*  INPUT:        int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
double CDbResultMysql::GetDbDouble(XU32 nCol)
{
	double value = 0;
	
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbDouble,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	
	if(NULL != m_mysqlRow)
	{
		value = *(double*)m_mysqlRow[nCol];
	}
	return value;
	
}


/**********************************************************************
*
*  NAME:          GetDbFloat
*  FUNTION:       返回sql语句中float型的value值
*  INPUT:        int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
float CDbResultMysql::GetDbFloat(XU32 nCol)
{
	float value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbFloat,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	
	if(NULL != m_mysqlRow)
	{
		value= *(float*)m_mysqlRow[nCol];
	}
	return value;
}

/**********************************************************************
*
*  NAME:          GetDbLong
*  FUNTION:       返回sql语句中long型的value值
*  INPUT:        int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
long CDbResultMysql::GetDbLong(XU32 nCol)
{
	long value =0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbLong,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	
	if(NULL != m_mysqlRow)
	{
		value = *(long*)m_mysqlRow[nCol];
	}
	return value;
}
/**********************************************************************
*
*  NAME:          GetDbULong
*  FUNTION:       返回sql语句中long型的value值
*  INPUT:        int nCol:动态sql语句中的第几个参数 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
unsigned long CDbResultMysql::GetDbULong(XU32 nCol)
{
	unsigned long value = 0;
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbULong,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return value;
	}
	if(NULL != m_mysqlRow)
	{
		value = *(unsigned long*)m_mysqlRow[nCol];
	}
	
	return value;
}
/**********************************************************************
*
*  NAME:          GetDbBlob
*  FUNTION:       返回sql语句中blob型的value值
*  INPUT:        int nCol:动态sql语句中的第几个参数
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbResultMysql::GetDbBlob(XU32 nCol,XS8* pCharValue)
{
	nCol -= 1;
	if(nCol >= m_lNumField)
	{
		g_callbackTraceFunOfMySqlDb(NULL, MD(FID_AGENTDB,PL_ERR),"GetDbBlob,nCol =%d.,m_lNumField=%d\n", nCol,m_lNumField);
		return;
	}
	
	if((NULL != m_mysqlRow) && (NULL != m_mysqllengths))
	{
		memcpy(pCharValue, m_mysqlRow[nCol], m_mysqllengths[nCol]);
	}
}
/**********************************************************************
*
*  NAME:          SetResultSet
*  FUNTION:       给该类的成员变量指针赋值
*  INPUT:         void *pResultSet：传入ResultSet类型的指针 
*  OUTPUT:        
*  OTHERS:        
**********************************************************************/
void CDbResultMysql::SetResultSet(void *pResultSet)
{
	m_pResultMysql = (MYSQL_RES*)pResultSet;
	if(NULL != m_pResultMysql)
	{
		m_lNumField = mysql_num_fields(m_pResultMysql);
		m_pField = mysql_fetch_fields(m_pResultMysql);
	}
	
	//m_pFieldLen = mysql_fetch_lengths(m_pResultMysql);
}
/**********************************************************************
*
*  NAME:          IsHaveNextRow
*  FUNTION:       判断该结果集是否还有后续输出
*  INPUT:         
*  OUTPUT:        若有后续记录输出，则返回XTRUE;无，返回XFALSE
*  OTHERS:        
**********************************************************************/
XBOOL CDbResultMysql::IsHaveNextRow(void)
{
	m_mysqlRow = mysql_fetch_row(m_pResultMysql); 	
	if(m_mysqlRow == NULL)
	{
		mysql_free_result(m_pResultMysql);
		return XFALSE;
	}

	m_mysqllengths = mysql_fetch_lengths(m_pResultMysql);
	if(m_mysqllengths == NULL)
	{
		mysql_free_result(m_pResultMysql);
		return XFALSE;
	}
	
	return XTRUE;
}
