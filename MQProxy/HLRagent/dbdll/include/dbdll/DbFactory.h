#pragma once
#include "DbService.h"
#include "dbproj.h"

typedef enum DB_TYPE
{
	DB_TYPE_ORACLE,
	DB_TYPE_MYSQL
}DB_TYPE;
class DBPROJ_API CDbFactory
{
public:
	CDbFactory(XVOID);
	~CDbFactory(XVOID);
	static CDbFactory* getInstance(XS32 type=DB_TYPE_ORACLE);
	static XVOID DestroyDbFactory(CDbFactory* pDbFactory);
	
	//XS32 ServiceType=0
	/*virtual CDbService* GetDbService(XS32 ServiceType=0)=0;
	virtual XVOID DestroyDbService(CDbService* pDbService)=0;*/

	//将CDbService*的获得方式 改为每次获得一个新的//modify by lx 2012.09.18
	virtual CDbService* CreateDbService(XS32 ServiceType=0)=0;
	
	
};
