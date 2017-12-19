#pragma once
#include "DbResult.h"
#include <occi.h>
using namespace oracle::occi;



class CDbResultOracle :
	public CDbResult
{
public:

	//声明结果集的状态
	enum EResultStatus                                   
	{
		NO_DATA_FETCH = 0,
		HAVE_DATA,
		HAVE_STREAM_DATA
	};

	CDbResultOracle(XVOID);
	~CDbResultOracle(XVOID);

	XS32 GetDbInt(XU32 nCol);
	XU32 GetDbUInt(XU32 nCol);

    string GetDbString(XU32 nCol);
	XVOID GetDbChar(XU32 nCol,XS8* pCharValue);

	XD64 GetDbDouble(XU32 nCol);
	Date GetDbDate(XU32 nCol);
	Timestamp GetDbTimeStamp(XU32 nCol);
	XF32 GetDbFloat(XU32 nCol);
	Blob GetDbBlob(XU32 nCol);
	Clob GetDbClob(XU32 nCol);
	Number GetDbNumber(XU32 nCol);
	Bytes GetDbBytes(XU32 nCol);

	XVOID SetResultSet(XVOID *pResultSet);
	XBOOL IsHaveNextRow(XVOID);

	ResultSet* GetResultSet(){return m_pResultOracle;};

protected:
	ResultSet* m_pResultOracle;

};
