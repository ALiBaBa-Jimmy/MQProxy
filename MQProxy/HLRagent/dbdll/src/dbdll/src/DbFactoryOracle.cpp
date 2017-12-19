
#include "DbFactoryOracle.h"


CDbFactoryOracle::CDbFactoryOracle(XVOID)
{
	
}

CDbFactoryOracle::~CDbFactoryOracle(XVOID)
{
	
}

//CDbService* CDbFactoryOracle::GetDbService(XS32 ServiceType)
//{
//	if (m_pDbService == NULL)
//	{
//		m_pDbService = new CDbServiceOracle();
//		return m_pDbService;
//	}
//	else
//	{
//		return m_pDbService;
//	}
//}
//
//XVOID CDbFactoryOracle::DestroyDbService(CDbService* pDbService)
//{
//	CDbServiceOracle *pDbServiceOracle  = (CDbServiceOracle*)pDbService;
//	delete pDbServiceOracle;
//	pDbServiceOracle= NULL;
//	
//}
//将CDbService*的获得方式 改为每次获得一个新的//modify by lx 2012.09.18
CDbService* CDbFactoryOracle::CreateDbService(XS32 ServiceType)
{
	CDbService *pDbService = new CDbServiceOracle();
	return pDbService;
}