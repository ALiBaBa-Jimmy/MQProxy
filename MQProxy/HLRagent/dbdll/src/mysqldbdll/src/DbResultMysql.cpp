
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
	//����delete m_pResultMysql����ϵͳ�Լ��ͷ�
}

/**********************************************************************
*
*  NAME:          GetDbInt
*  FUNTION:       ����sql�����int�͵�valueֵ
*  INPUT:          int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����unsigned int�͵�valueֵ
*  INPUT:          int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����string�͵�valueֵ
*  INPUT:		  int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����string�͵�valueֵ
*  INPUT:		  int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����double�͵�valueֵ
*  INPUT:        int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����float�͵�valueֵ
*  INPUT:        int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����long�͵�valueֵ
*  INPUT:        int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����long�͵�valueֵ
*  INPUT:        int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ������ĳ�Ա����ָ�븳ֵ
*  INPUT:         void *pResultSet������ResultSet���͵�ָ�� 
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
*  FUNTION:       �жϸý�����Ƿ��к������
*  INPUT:         
*  OUTPUT:        ���к�����¼������򷵻�XTRUE;�ޣ�����XFALSE
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
