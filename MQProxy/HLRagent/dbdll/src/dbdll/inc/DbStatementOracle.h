#pragma once
#include "DbStatement.h"
#include "DbResultOracle.h"


using namespace std;

class CDbStatementOracle :
	public CDbStatement
{

public:
	CDbStatementOracle(XVOID);
	~CDbStatementOracle(XVOID);
	XVOID SetDbSql(XS8* sql);
	XVOID SetDbInt(XU32 nCol, XS32 value);
	XVOID SetDbUInt(XU32 nCol, XU32 value);

	XVOID SetDbString(XU32 nCol, string value);
	XVOID SetDbDouble(XU32 nCol, XD64 value);

	XVOID SetDbDate(XU32 nCol, Date value);
	XVOID SetDbTimestamp(XU32 nCol, Timestamp value);
	XVOID SetDbFloat(XU32 nCol, XF32 value);
	XVOID SetDbBlob(XU32 nCol, Blob value);
	XVOID SetDbClob(XU32 nCol, Clob value);
	XVOID SetDbNumber(XU32 nCol, Number value);
	XVOID SetDbBytes(XU32 nCol, Bytes value);
	XVOID SetDbChar(XU32 nCol, XS8* value);
	
	XU32 GetDbUpdateCount();
    
	XVOID SetStatement(XVOID *pStmt);
	XS32 ExecuteQuery(CDbResult **pResult);
	XS32 ExecuteUpdate(XVOID);
	XVOID* GetStatement(XVOID);

	XS32 GetErrorCode(XS32 errorCode);

	//设置预处理参数的个数,myssql使用
	XVOID SetBindParmCount(XU32 nCount){};

	virtual XVOID CloseResultSet(CDbResult* re);

	XVOID SetConnId(XU32 iConnId);

protected:
	Statement* m_pStmtOracle;
	CDbResultOracle * m_oracleResult;
	XU32 m_connId;
};
