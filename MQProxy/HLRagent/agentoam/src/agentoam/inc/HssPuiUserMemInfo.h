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
#include <string>
#include "eHssMDbPuiStruct.h"
#include "smudbdb_common.h"
using namespace std;

#define  MAX_MEM_PUI_DYN_INFO_NUMBER 255
class CHssPuiUserMemInfo
{
public:
	CHssPuiUserMemInfo(void);
	~CHssPuiUserMemInfo(void);
	
	//�洢�ѱ����¶�̬���ݵ��û����ݵ�����
	XS32 StoreUptPuiDynUserKey(string key,XU32 srcFid);
	//���¶�̬���ݿ⵽�������ݿ�
	XS32 UptAllPuiDynInfoToDb(XU32 fid, SDbContext *pSDbContext);
	XS32 UptSinglePuiDynInfoToDb(string key,SDbContext *pSDbContext);

private:

	std::vector<string> m_uptPuiDynInfo;
	t_XOSMUTEXID m_uptPuiDynInfoLock;
	XU32 m_puiDynCount;


};

