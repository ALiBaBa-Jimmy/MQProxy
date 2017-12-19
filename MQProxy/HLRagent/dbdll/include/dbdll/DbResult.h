#pragma once
#include <iostream>
#include "publictype.h"
#include "xostype.h"
using namespace std;


//注意：必须先引用系统的头文件，再引用OCCI的头文件
class CDbResult
{
public:
	virtual XS32 GetDbInt(XU32 nCol)=0;
	virtual XU32 GetDbUInt(XU32 nCol)=0;

    virtual string GetDbString(XU32 nCol)=0;
	virtual XVOID GetDbChar(XU32 nCol,XS8* pCharValue)=0;
virtual XF32 GetDbFloat(XU32 nCol)=0;

	virtual XD64 GetDbDouble(XU32 nCol)=0;
	/*virtual Date GetDbDate(XU32 nCol)=0;
	virtual Timestamp GetDbTimeStamp(XU32 nCol)=0;
	
	virtual Blob GetDbBlob(XU32 nCol)=0;
	virtual Clob GetDbClob(XU32 nCol)=0;
	virtual Number GetDbNumber(XU32 nCol)=0;
	virtual Bytes GetDbBytes(XU32 nCol)=0;*/

	virtual XVOID SetResultSet(XVOID *pResultSet)=0;
	virtual XBOOL IsHaveNextRow(XVOID)=0;

};
