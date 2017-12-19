/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssUserMdbInfo.h
* Author:       luhaiyan
* Date��        10-12-2014
* OverView:     HSS�û��ڴ���Ϣ
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*******************************************************************************/
#pragma once
#include <map>
#include <vector>
#include <set>
#include <string.h>
#include "eHssMAndHDbPviBusiness.h"
#include "smudbdb_common.h"
using namespace std;
class CHssImsiUserMemInfo
{
public:
	CHssImsiUserMemInfo(void);
	~CHssImsiUserMemInfo(void);	

	//���¼�Ȩ��Ϣ��̬���ݿ�
	XS32 StoreUptAuthDynUserKey(string key, XU32 srcFid);
	//���¶�̬����
	XS32 StoreUptImisDynUserKey(string key, XU32 srcFid);
	XS32 StoreUptTasReposUserKey(string keyUser,string keyTasRepos,XU32 srcFid);
	XS32 StoreDelTasReposUserKey(string keyUser,string keyTasRepos,SDbTasReposDataAddReq tasReposData,XU32 srcFid);



	//���¶�̬���ݿ⵽�������ݿ�
	XS32 UptAllImisDynInfoToDb(XU32 fid, SDbContext *pSDbContext);
	XS32 UptSingleImisDynInfoToDb(string key, SDbContext *pSDbContext);

	XS32 UptAllAuthDynInfoToDb(XU32 fid, SDbContext *pSDbContext);
	XS32 UptSingleAuthDynInfoToDb(string key, SDbContext *pSDbContext);

	XS32 UptAllTasReposInfoToDb(XU32 fid, SDbContext *pSDbContext);
	XS32 UptSingleUserTasReposInfoToDb(string key,set<string> tasReposSet,SDbContext *pSDbContext);

	XS32 DelAllTasReposInfoFromDb(XU32 fid, SDbContext *pSDbContext);
	XS32 DelSingleUserTasReposInfoFromDb(string key,map<string,SDbTasReposDataAddReq> tasReposSet,SDbContext *pSDbContext);


private:
	
	std::vector<string> m_uptImisDynInfo;
	t_XOSMUTEXID m_uptImisDynInfoLock;
	XU32 m_imisDynCount;

	std::vector<string> m_uptAuthDynInfo;
	t_XOSMUTEXID m_uptAuthDynInfoLock;
	XU32 m_authDynCount;


	std::map<string,set<string> > m_uptTasReposInfo;
	t_XOSMUTEXID m_uptTasReposInfoLock;
	XU32 m_tasReposCount;

	//ֻ��ṹ��
	std::map<string,map<string,SDbTasReposDataAddReq> > m_delTasReposInfo;
	t_XOSMUTEXID m_delTasReposInfoLock;
	XU32 m_delTasReposCount;
	
};
