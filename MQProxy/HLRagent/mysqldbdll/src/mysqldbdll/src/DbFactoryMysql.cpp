#include "DbFactoryMysql.h"
#include "DbServiceMysql.h"

CDbFactoryMysql *CDbFactoryMysql::m_Instance = NULL;


CDbFactoryMysql::CDbFactoryMysql(void)
{  
	CDbServiceMysql *pDbServiceMysql = new CDbServiceMysql();
	m_pDbService = (CDbService *)pDbServiceMysql;
}

CDbFactoryMysql::~CDbFactoryMysql(void)
{
	if(NULL !=m_pDbService)
	{
		delete m_pDbService;
		m_pDbService = NULL;
	}
}

CDbFactoryMysql *CDbFactoryMysql::GetInstance(void)
{
	if(NULL == m_Instance)
	{
		static CDbFactoryMysql instance;
		m_Instance = &instance;
	}

	return m_Instance;
}

CDbService* CDbFactoryMysql::GetDbService(void)
{
    if(NULL == m_pDbService)
    {     
        CDbServiceMysql *pDbServiceMysql = new CDbServiceMysql();
        m_pDbService = (CDbService *)pDbServiceMysql;
    }
    return m_pDbService;
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
