
#include "DbFactoryMysql.h"

CDbFactoryMysql::CDbFactoryMysql(void)
{
}

CDbFactoryMysql::~CDbFactoryMysql(void)
{
	map<int,CDbService*>::iterator it;
	//¼Ó¶ÁËø
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_mapDbServiceLock );  
   
	for(it=m_mapDbService.begin();it!=m_mapDbService.end();it++)
	{
		CDbServiceMysql *pDbServiceMysql = (CDbServiceMysql *)it->second;		
		delete pDbServiceMysql;
		pDbServiceMysql = NULL;
	}
}

CDbService* CDbFactoryMysql::GetDbService(int ServiceType)
{
	CDbService *pDbService = NULL;
	map<int,CDbService*>::iterator it;
	//¼Ó¶ÁËø
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard( m_mapDbServiceLock );  
   
	it= m_mapDbService.find(ServiceType);
	
	if(it== m_mapDbService.end())
	{
		CDbServiceMysql *pDbServiceMysql = new CDbServiceMysql();
		pDbService = (CDbService *)pDbServiceMysql;
		m_mapDbService.insert(make_pair(ServiceType,pDbServiceMysql));
	}
    else 
    { 
		pDbService = (CDbService *)it->second;
	} 
	return pDbService;
}

void CDbFactoryMysql::DestroyDbService(CDbService* pDbService)
{
	CDbServiceMysql *pDbServiceMysql  = (CDbServiceMysql*)pDbService;
	if(NULL != pDbServiceMysql)
	{
		pDbServiceMysql->DbDestroy();
		delete pDbServiceMysql;
		pDbServiceMysql= NULL;
	}
	
	
}
