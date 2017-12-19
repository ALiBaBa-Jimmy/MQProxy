#pragma once
#include <map>
#include "DbFactory.h"
#include "DbService.h"

class MYSQLDBDLL_API CDbFactoryMysql :
	public CDbFactory
{
public:
	CDbFactoryMysql(void);
	~CDbFactoryMysql(void);
	static CDbFactoryMysql *GetInstance(void);
	CDbService* GetDbService(void);
	void DestroyDbService(CDbService* pDbService);
    CDbService * m_pDbService;

protected:
	static CDbFactoryMysql *m_Instance;
};
