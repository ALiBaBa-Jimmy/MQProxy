#include "DbFactory.h"
#include "DbFactoryOracle.h"
#include <iostream>
using namespace std;

static CDbFactory* pDbFactory = NULL;
static CDbFactory* pMysqlDbFactory = NULL;
CDbFactory::CDbFactory(XVOID)
{
}

CDbFactory::~CDbFactory(XVOID)
{
	
}

CDbFactory* CDbFactory::getInstance(XS32 type)
{
	if(type == DB_TYPE_ORACLE)
	{
		if(pDbFactory == NULL)
		{
			pDbFactory = new CDbFactoryOracle();
			if(pDbFactory == NULL)
			{
				cout<<"get a Instance of CDbFactory is error"<<endl;
				return NULL;
			}
		}

		return pDbFactory;
	}	
	else 
	{
		return NULL;
	}
	

}
XVOID CDbFactory::DestroyDbFactory(CDbFactory* pDbFactory)
{
	if (pDbFactory != NULL)
	{
		delete pDbFactory;
		pDbFactory = NULL;

	}
}
