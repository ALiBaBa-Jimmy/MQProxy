/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbTask.cpp
* Author:       luhaiyan
* Date：        10-15-2014
* OverView:     数据库的处理
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


#include "agentinclude.h"
#include "agentDbTask.h"
#include "SPRHdbOam.h"
#include "agentTrace.h"
#include "DbFactory.h"
#include "SPRMAndHDbComFun.h"
#include "SPRCommFun.h"
#include "SPRMdbDbFun.h"
#include "clishell.h"
#include "MqttComFun.h"
#include "agentDB_API.h"
#include "DbCommon.h"






/**************************************************************************
函 数 名: eHssDb_InitProc
函数功能: 数据库线程初始化函数
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS8 agentDB_InitProc(XVOID *pPara1, XVOID *Para2)
{	
	XU32 fid = FID_AGENTDB;
	//RegUserDataCliCmd();
	RegistHDBMySqlDbCallbackTraceFun(agentTrace);
	RegistHDbCallbackTraceFun(agentTrace);
	RegistMDbCallbackTraceFun(agentTrace);
	RegistDbCallbackTraceFun(agentTrace);

	if(XSUCC !=DbInitData(fid))
	{
		XOS_Trace(MD(fid, PL_EXP),"agent Connect to Oracle Db failed.");			
		return XERROR;
	}
    else
    {
    
        /*加载数据库中的订阅信息*/    
        //agentDB_SubQryAll(fid);
    }
	

    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_XosMsgProc
函数功能: 数据库线程消息处理函数
参    数:
返 回 值:
**************************************************************************/
XS8 agentDB_XosMsgProc(XVOID *msg, XVOID *pPara)
{


	


    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_TimeoutProc
函数功能: 用于向XOS平台注册超时回调接口,暂时不需要进行处理
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 agentDB_TimeoutProc(t_BACKPARA  *pstPara)
{


	
    return XSUCC;
}

/**************************************************************************
函 数 名: SPRDb_NoticeProc
函数功能: 用于向XOS平台注册通知函数回调接口
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 agentDB_NoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{
	return XSUCC;
}


/***************************************************************
函数：XdbTimerStartSynDyn
功能：初始化并启动同步动态数据信息定时器
输入：无
输出：无
返回：XSUCC/XERROR                    
***************************************************************/
XS32 XdbTimerStartSynDyn(XU32 fid)
{


	return 0;
}

/***************************************************************
函数：XdbTimerStartInitDb
功能：初始化连接数据库定时器
输入：无
输出：无
返回：XSUCC/XERROR                    
***************************************************************/
XS32 XdbTimerStartInitDb(XU32 fid)
{


	return 0;
}



/***************************************************************
函数：GetDbCfgInfo
功能：根据是否支持网管配置标示，从配置文件或者oam内存中读取数据库配置信息
输入：SDbConfigInfo *pDbCfgInfo：返回数据库配置信息
输出：无
返回：XSUCC/XERROR                    
***************************************************************/
XS32 GetDbCfgInfo(SDbConfigInfo *pDbCfgInfo)
{
	XS32 ret = XERROR;
	//从配置文件中读取数据库配置信息
	ret = MYSQL_GetDbInitsCfg(FID_AGENTDB, pDbCfgInfo, "./dbcfg.xml");
	if (ret != XSUCC)
	{
		XOS_Trace(MD(FID_AGENTDB,PL_ERR),"MYSQL_GetDbInitsCfg error.");
		return XERROR;
	}

	
	return XSUCC;
}

/***************************************************************
函数：DbInitData
功能：初始化连接数据库后进行的初始化数据操作
输入：无
输出：无
返回：XSUCC/XERROR , 只要没有初始化成功都要创建初始化连接定时器                   
***************************************************************/
XS32 DbInitData(XU32 fid)
{


	if(XSUCC != DbInitDataHandler(fid))
	{
	    XOS_Trace(MD(FID_AGENTDB,PL_ERR),"DbInitDataHandler error.");
        return XERROR;

	}


	return XSUCC;
}

/***************************************************************
函数：DbInitDataHandler
功能：初始化连接数据库后进行的初始化数据操作,被DbInitData调用，DbInitData被其它地方调用
输入：无
输出：无
返回：XSUCC/XERROR , 这里只是出来返回错误，如果错误由 DbInitData创建定时器                  
***************************************************************/
XS32 DbInitDataHandler(XU32 fid)
{
	XS32 nRet = XERROR;
	SDbConfigInfo szDbCfgInfo = {0};
	nRet = GetDbCfgInfo(&szDbCfgInfo);
	if (nRet != XSUCC)
	{
        XOS_Trace(MD(fid,PL_ERR),"Get Db Configure Error.");
		return XERROR;
	}

    if(0 == XOS_MemCmp(szDbCfgInfo.ip, "0.0.0.0", XOS_StrLen("0.0.0.0")))
    {
        XOS_Trace(MD(fid,PL_LOG)," db.ip = 0.0.0.0 invalid");
        return XSUCC;
    }
	nRet = InitDb(&szDbCfgInfo);
	if(XSUCC != nRet)
	{
        XOS_Trace(MD(fid,PL_ERR),"init Db connection Error.");
		return XERROR;
		
	}

   
	return XSUCC;
	
}



