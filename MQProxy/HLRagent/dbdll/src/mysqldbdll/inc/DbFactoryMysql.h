#pragma once
#include "DbFactory.h"
#include "DbServiceMysql.h"

class CDbFactoryMysql :
	public CDbFactory
{
public:
	CDbFactoryMysql(void);
	~CDbFactoryMysql(void);

	CDbService* GetDbService(int ServiceType=0);
	void DestroyDbService(CDbService* pDbService);
protected:
	
	ACE_RW_Thread_Mutex m_mapDbServiceLock;//¶ÁÐ´Ëø£¬±£»¤m_mapDbService
	// dbservice
	map<int,CDbService*> m_mapDbService;
};
