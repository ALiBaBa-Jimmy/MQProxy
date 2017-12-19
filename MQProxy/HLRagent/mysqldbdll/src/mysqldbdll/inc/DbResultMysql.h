#pragma once

#include "DbResult.h"
#include "mysql.h"


class CDbResultMysql :
	public CDbResult
{
public:
	CDbResultMysql(void);
	~CDbResultMysql(void);

	XS8  GetDbTiny(XU32 nCol);
	XS16 GetDbShort(XU32 nCol);
	XS32 GetDbInt(XU32 nCol);
	XU32 GetDbUInt(XU32 nCol);
	long GetDbLong(XU32 nCol);
	unsigned long GetDbULong(XU32 nCol);
    string GetDbString(XU32 nCol);
	void GetDbChar(XU32 nCol,XS8* pCharValue);
	double GetDbDouble(XU32 nCol);
	float GetDbFloat(XU32 nCol);
	void GetDbBlob(XU32 nCol,XS8* pCharValue);
	void SetResultSet(void *pResultSet);
	XBOOL IsHaveNextRow(void);
protected:	
	MYSQL_RES* m_pResultMysql;
	MYSQL_ROW  m_mysqlRow;
	unsigned long m_lNumField;
	//unsigned long* m_pFieldLen;
	MYSQL_FIELD *m_pField;
	unsigned long *m_mysqllengths;

};
