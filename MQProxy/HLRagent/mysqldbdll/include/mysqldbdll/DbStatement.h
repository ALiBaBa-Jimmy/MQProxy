#pragma once

#include <iostream>
#include "DbResult.h"
#include "MysqlDbDll.h"
using namespace std;

#define MAX_BIND_PARAM_COUNT 100
class MYSQLDBDLL_API CDbStatement
{
public:
	virtual void SetDbSql(XS8* sql)=0;
	virtual void SetDbTiny(XU32 nCol, XS8 value)=0;
	virtual void SetDbShort(XU32 nCol, XS16 value)=0;
	virtual void SetDbInt(XU32 nCol, XS32 value)=0;
	virtual void SetDbUInt(XU32 nCol, XU32 value)=0;
	virtual void SetDbLong(XU32 nCol, long value)=0;
	virtual void SetDbULong(XU32 nCol, unsigned long value)=0;
	virtual void SetDbString(XU32 nCol, string value)=0;
	virtual void SetDbDouble(XU32 nCol, double value)=0;
	virtual void SetDbFloat(XU32 nCol, float value)=0;

	/*virtual void SetDbDate(XU32 nCol, Date value)=0;
	virtual void SetDbTimestamp(XU32 nCol, Timestamp value)=0;
	*/
	//virtual void SetDbBlob(XU32 nCol, Blob value)=0;
	//virtual void SetDbClob(XU32 nCol, Clob value)=0;
	//virtual void SetDbNumber(XU32 nCol, Number value)=0;
	//virtual void SetDbBytes(XU32 nCol, Bytes value)=0;
	/*为避免出现传入string的长度超过16个字节，程序崩溃的问题，添加该类型代替SetDbString*/
	virtual void SetDbChar(XU32 nCol, XS8* value)=0;
	virtual void SetDbBlob(XU32 nCol, XU32 len, XS8* value)=0;
	virtual XU32 GetDbUpdateCount()=0;

	virtual XS32 ExecuteQuery(CDbResult **pResult)=0;
	virtual void SetStatement(void *pStmt) = 0;
	virtual void* GetStatement(void) = 0;
	virtual XS32 ExecuteUpdate(void)=0;

	virtual XS32 GetErrorCode(XS32 errorCode)=0;

	//设置预处理参数的个数,myssql使用
	virtual void SetBindParmCount(XU32 nCount)=0;
	virtual XVOID CloseResultSet(CDbResult* re)=0;
};






