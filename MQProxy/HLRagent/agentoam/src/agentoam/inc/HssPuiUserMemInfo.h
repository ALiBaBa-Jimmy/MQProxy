/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssUserMdbInfo.h
* Author:       luhaiyan
* Date：        10-12-2014
* OverView:     HSS用户内存信息
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
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
	
	//存储已被更新动态数据的用户数据的索引
	XS32 StoreUptPuiDynUserKey(string key,XU32 srcFid);
	//更新动态数据库到物理数据库
	XS32 UptAllPuiDynInfoToDb(XU32 fid, SDbContext *pSDbContext);
	XS32 UptSinglePuiDynInfoToDb(string key,SDbContext *pSDbContext);

private:

	std::vector<string> m_uptPuiDynInfo;
	t_XOSMUTEXID m_uptPuiDynInfoLock;
	XU32 m_puiDynCount;


};

