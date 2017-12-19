#pragma once
#include "DbStatement.h"
#include "DbResultMysql.h"
#include "mysql.h"


class CDbStatementMysql :
	public CDbStatement
{

public:
	CDbStatementMysql(void);
	~CDbStatementMysql(void);
	void SetDbSql(XS8* sql);
	void SetDbTiny(XU32 nCol, XS8 value);
	void SetDbShort(XU32 nCol, XS16 value);
	void SetDbInt(XU32 nCol, XS32 value);
	void SetDbUInt(XU32 nCol, XU32 value);
	void SetDbLong(XU32 nCol, long value);
	void SetDbULong(XU32 nCol, unsigned long value);
	void SetDbString(XU32 nCol, string value);
	void SetDbDouble(XU32 nCol, double value);

	void SetDbFloat(XU32 nCol, float value);

	void SetDbChar(XU32 nCol, XS8* value);

	void SetDbBlob(XU32 nCol, XU32 len, XS8* value);
	
	XU32 GetDbUpdateCount();
    
	void SetStatement(void *pStmt);
	XS32 ExecuteQuery(CDbResult **pResult);
	XS32 ExecuteUpdate(void);
	void* GetStatement(void);

	XS32 GetErrorCode(XS32 errorCode);

	//设置预处理参数的个数,myssql使用
	void SetBindParmCount(XU32 nCount);

	XVOID CloseResultSet(CDbResult* re){return;};

	MYSQL * m_pMysql;

	string m_strSql;

protected:
	MYSQL_STMT* m_pStmtMysql;
	CDbResultMysql * m_mysqlResult;
	MYSQL_BIND m_bind[MAX_BIND_PARAM_COUNT]; 
	unsigned long m_bindLength[MAX_BIND_PARAM_COUNT]; 
	char* m_bindValue[MAX_BIND_PARAM_COUNT]; 
	XU16 m_bBindCount;
	

};
