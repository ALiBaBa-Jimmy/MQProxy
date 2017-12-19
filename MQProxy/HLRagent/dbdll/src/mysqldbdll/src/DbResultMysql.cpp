
#include "DbResultMysql.h"
#include "ace/Log_Msg.h"

CDbResultMysql::CDbResultMysql(void)
{
	m_pResultMysql = NULL;
	m_mysqlRow = NULL;
	m_pFieldLen = new unsigned long(100);
}

CDbResultMysql::~CDbResultMysql(void)
{
	//不用delete m_pResultMysql，由系统自己释放
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value = (XS32)atoi(m_mysqlRow[nCol]);
	
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value = (XU32)atoi(m_mysqlRow[nCol]);
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return string("");
	}
	s = string (m_mysqlRow[nCol]);	
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
	memcpy(pCharValue, m_mysqlRow[nCol], strlen(m_mysqlRow[nCol]));
	
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value = *(double*)m_mysqlRow[nCol];
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value= *(float*)m_mysqlRow[nCol];
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value = *(long*)m_mysqlRow[nCol];
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
		ACE_ERROR((LM_ERROR,"GetDbString,nCol =%d.", nCol));
		return value;
	}
	value = *(unsigned long*)m_mysqlRow[nCol];
	return value;
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
	//m_lNumField = mysql_field_count(m_pResultMysql);
	m_pField = mysql_fetch_fields(m_pResultMysql);
	m_pFieldLen = mysql_fetch_lengths(m_pResultMysql);
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
		return false;
	}
	
	return true;
}
