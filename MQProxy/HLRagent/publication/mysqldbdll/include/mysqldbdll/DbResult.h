#pragma once
#include <iostream>
#include "xosshell.h"
#include "MysqlDbDll.h"
using namespace std;


//注意：必须先引用系统的头文件，再引用OCCI的头文件
class MYSQLDBDLL_API CDbResult
{
public:
	virtual XS8  GetDbTiny(XU32 nCol)=0;
	virtual XS16 GetDbShort(XU32 nCol)=0;
	virtual XS32 GetDbInt(XU32 nCol)=0;
	virtual XU32 GetDbUInt(XU32 nCol)=0;
	virtual long GetDbLong(XU32 nCol)=0;
	virtual unsigned long GetDbULong(XU32 nCol)=0;
    virtual string GetDbString(XU32 nCol)=0;
	virtual void GetDbChar(XU32 nCol,XS8* pCharValue)=0;
	virtual float GetDbFloat(XU32 nCol)=0;

	virtual double GetDbDouble(XU32 nCol)=0;
	/*virtual Date GetDbDate(XU32 nCol)=0;
	virtual Timestamp GetDbTimeStamp(XU32 nCol)=0;
	
	virtual Blob GetDbBlob(XU32 nCol)=0;
	virtual Clob GetDbClob(XU32 nCol)=0;
	virtual Number GetDbNumber(XU32 nCol)=0;
	virtual Bytes GetDbBytes(XU32 nCol)=0;*/
	virtual void GetDbBlob(XU32 nCol,XS8* pCharValue)=0;

	virtual void SetResultSet(void *pResultSet)=0;
	virtual XBOOL IsHaveNextRow(void)=0;

};
