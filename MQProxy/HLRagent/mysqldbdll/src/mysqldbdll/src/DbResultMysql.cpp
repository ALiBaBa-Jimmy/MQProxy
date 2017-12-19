
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
	//����delete m_pResultMysql����ϵͳ�Լ��ͷ�
	/*if(m_pFieldLen != NULL)
	{
		delete []m_pFieldLen;
	}*/
	 
}
/**********************************************************************
*
*  NAME:          GetDbTiny
*  FUNTION:       ����sql�����tinyint�͵�valueֵ
*  INPUT:          int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����short�͵�valueֵ
*  INPUT:          int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����string�͵�valueֵ
*  INPUT:		  int nCol:��̬sql����еĵڼ������� 
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
*  FUNTION:       ����sql�����blob�͵�valueֵ
*  INPUT:        int nCol:��̬sql����еĵڼ�������
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
*  FUNTION:       ������ĳ�Ա����ָ�븳ֵ
*  INPUT:         void *pResultSet������ResultSet���͵�ָ�� 
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
