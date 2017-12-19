#ifndef __DIAM_DICT_DATA_H__
#define __DIAM_DICT_DATA_H__

#include <api/diam_datatype.h>
#include <api/diam_define.h>

#define DEFAULT_BASE_DICT_PATH "./config/dictionary.xml"
#define DEFAULT_PROD_DICT_PATH "./config/product.xml"

class DiamDictDataLoad
{
public:
    DiamRetCode loadDictData();
private:
    /**************************************************************************
    �� �� ��: loadBaseData
    ��������: ���ػ�������,����·�����ݵ�
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamRetCode loadBaseDict();
    /**************************************************************************
    �� �� ��: loadDictData
    ��������: �����ֵ�����
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamRetCode loadProtocolDict();
    /**************************************************************************
    �� �� ��: loadBaseData
    ��������: ���ػ�������
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void loadBaseData(const char *cfgfile);


private:
    std::string m_basePath;
};

/*
class CDiamConfigValidation
{
public:
	static bool IsApplicationIdSupported(DiamUINT32 appId);
};

class CDiamCfgLib
{
public:
	//�������캯��
	static CDiamCfgLib* Instance();

public:
	DiamDataRoot* getRootNode();

private:
	static CDiamCfgLib* instance;
	CDiamCfgLib();
	virtual ~CDiamCfgLib();

private:
	DiamDataRoot root;
};

#define DIAM_CFG_ROOT()            (CDiamCfgLib::Instance()->getRootNode())
#define DIAM_CFG_RUNTIME()         (&(CDiamCfgLib::Instance()->getRootNode()->runtime))
#define DIAM_CFG_GENERAL()         (&(CDiamCfgLib::Instance()->getRootNode()->general))
#define DIAM_CFG_PARSER()          (&(CDiamCfgLib::Instance()->getRootNode()->parser))
#define DIAM_CFG_TRANSPORT()       (&(CDiamCfgLib::Instance()->getRootNode()->transport))
#define DIAM_CFG_SESSION()         (&(CDiamCfgLib::Instance()->getRootNode()->session))
#define DIAM_CFG_AUTH_SESSION()    (&(CDiamCfgLib::Instance()->getRootNode()->session.authSessions))
#define DIAM_CFG_ACCT_SESSION()    (&(CDiamCfgLib::Instance()->getRootNode()->session.acctSessions))
#define DIAM_CFG_TRANS_SESSION()   (&(CDiamCfgLib::Instance()->getRootNode()->session.transSessions))
#define DIAM_CFG_LOG()             (&(CDiamCfgLib::Instance()->getRootNode()->log))
*/

#endif // __DIAM_DICT_DATA_H__


