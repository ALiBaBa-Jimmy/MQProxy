#pragma once

#include <iostream>
#include "DbResult.h"
using namespace std;

#define MAX_BIND_PARAM_COUNT 100
class CDbStatement
{
public:
	virtual XVOID SetDbSql(XS8* sql)=0;
	virtual XVOID SetDbInt(XU32 nCol, XS32 value)=0;
	virtual XVOID SetDbUInt(XU32 nCol, XU32 value)=0;

	virtual XVOID SetDbString(XU32 nCol, string value)=0;
	virtual XVOID SetDbDouble(XU32 nCol, XD64 value)=0;
	virtual XVOID SetDbFloat(XU32 nCol, XF32 value)=0;

	/*virtual XVOID SetDbDate(XU32 nCol, Date value)=0;
	virtual XVOID SetDbTimestamp(XU32 nCol, Timestamp value)=0;
	*/
	//virtual XVOID SetDbBlob(XU32 nCol, Blob value)=0;
	//virtual XVOID SetDbClob(XU32 nCol, Clob value)=0;
	//virtual XVOID SetDbNumber(XU32 nCol, Number value)=0;
	//virtual XVOID SetDbBytes(XU32 nCol, Bytes value)=0;
	/*为避免出现传入string的长度超过16个字节，程序崩溃的问题，添加该类型代替SetDbString*/
	virtual XVOID SetDbChar(XU32 nCol, XS8* value)=0;
	virtual XU32 GetDbUpdateCount()=0;

	virtual XS32 ExecuteQuery(CDbResult **pResult)=0;
	virtual XVOID SetStatement(XVOID *pStmt) = 0;
	virtual XVOID* GetStatement(XVOID) = 0;
	virtual XS32 ExecuteUpdate(XVOID)=0;

	virtual XS32 GetErrorCode(XS32 errorCode)=0;

	//设置预处理参数的个数,myssql使用
	virtual XVOID SetBindParmCount(XU32 nCount)=0;

	virtual XVOID CloseResultSet(CDbResult* re)=0;
};






