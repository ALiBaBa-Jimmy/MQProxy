#pragma once
#include "DbFactory.h"
#include "DbServiceOracle.h"

class CDbFactoryOracle :
	public CDbFactory
{
public:
	CDbFactoryOracle(XVOID);
	~CDbFactoryOracle(XVOID);

	//��CDbService*�Ļ�÷�ʽ ��Ϊÿ�λ��һ���µ�//modify by lx 2012.09.18
	CDbService* CreateDbService(XS32 ServiceType=0);

	/*CDbService* GetDbService(XS32 ServiceType=0);
	XVOID DestroyDbService(CDbService* pDbService);
protected:
	CDbService* m_pDbService;*/
};
