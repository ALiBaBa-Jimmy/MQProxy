#pragma once
#include "DbService.h"
#include "MysqlDbDll.h"

class MYSQLDBDLL_API CDbFactory
{
public:
	virtual CDbService* GetDbService(void)=0;
	virtual void DestroyDbService(CDbService* pDbService)=0;
};
