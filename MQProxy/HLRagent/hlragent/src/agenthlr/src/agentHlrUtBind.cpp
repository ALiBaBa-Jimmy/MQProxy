/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年10月17日
**************************************************************************/
#include "xosshell.h"
//#include "agentHlrCom.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"
#include "agentHlrUtBind.h"


map<string, SLeaseHoldCtlTable> g_UtBindInfo;
t_XOSMUTEXID g_UtBindInfoMutext;


PTIMER  g_UtBindInfotimerHandle = NULL;




/*****************************************************************************
 Prototype    : agentHLR_UserInfoInit
 Description  : 租赁绑定终端信息同步相关初始化 初始化
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UtBindInfoInit(XVOID)
{
    XS32 ret = 0;

 
    t_BACKPARA tBackPara = {0};

    if (XSUCC != XOS_MutexCreate(&g_UtBindInfoMutext))
	{
		XOS_Trace(MD(FID_HLR, PL_EXP),"create  g_UtBindInfoMutext mutex failed!");
		return XERROR;	
	}
    
    tBackPara.para1 = TIMER_TYPE_AGENT_UTBIND_SYN;
    
	ret = XOS_TimerCreate(FID_HLR, &g_UtBindInfotimerHandle, TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackPara);
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_HLR,PL_ERR), "XOS_TimerCreate  Err!");
		return XERROR;
	}

	ret = XOS_TimerBegin(FID_HLR, g_UtBindInfotimerHandle, 1*60*60*1000);/*时长为一个小时*/
	if (XERROR == ret)
	{
		XOS_Trace(MD(FID_HLR, PL_ERR), "TimerBegin is Err!");
		return XERROR;
	}
    
	return XSUCC;
}
/*****************************************************************************
 Prototype    : agentHLR_TimerUtBindInfoPro
 Description  : 
 Input        : t_BACKPARA *tBackPara  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_TimerUtBindInfoPro(t_BACKPARA *tBackPara)
{
    
    XS32 retValue = 0;


 


   // XOS_Trace(MD(FID_HLR, PL_LOG),"Del UserInfo %u  by timer", UserInfo_size());
    
    
 
 

    return XSUCC;
}

/*****************************************************************************
 Prototype    : UserInfo_size
 Description  : 查询操作 返回缓存的大小
 Input        : None
 Output       : None
 Return Value : XU32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XU32 UtBindInfo_size(XVOID)
{
    
    XU32 tmp = 0;
     
    XOS_MutexLock(&g_UtBindInfoMutext);
    tmp = g_UtBindInfo.size();
    XOS_MutexUnlock(&g_UtBindInfoMutext);
 
    
    return tmp;

}
XVOID agentHLR_InsrtCallback(SLeaseHoldCtlTable *pIns)
{
    XU8 pidport[LENGTH_OF_PID+1]={0};

    XU8 Strpidport[11] = {0};

    XOS_MemCpy(pidport, pIns->pid, LENGTH_OF_PID);
    pidport[LENGTH_OF_PID] = pIns->port;
    agentHLR_StrToHex(pidport, Strpidport, LENGTH_OF_PID+1);

    string tmp((XS8*)Strpidport);
    
    XOS_Trace(MD(FID_HLR, PL_DBG),"Load Bind Info: pid=%02X%02X%02X%02X, port=%u", 
        pidport[0],pidport[1],pidport[2],pidport[3],pidport[4]);
    
    map<string, SLeaseHoldCtlTable>::iterator it = g_UtBindInfo.find(tmp);
    if(g_UtBindInfo.end() != it)
    {
        XOS_MemCpy(&(it->second), pIns, sizeof(SLeaseHoldCtlTable));
		return ;
    }

	g_UtBindInfo.insert(make_pair(tmp, *pIns));
	return ;
    
    
    
}
/*****************************************************************************
 Prototype    : agentHLR_GetUtInfo
 Description  : 将数据库中的租赁绑定信息写入内存准备分发给hlr
 Input        : None
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/10/22
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_GetUtInfo()
{


    XS32 ret = MysqlDB_QryAllUtBindInfo(FID_HLR);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR),"Get UtBindInfo Err");
        return XERROR;
    }
    XOS_Trace(MD(FID_HLR, PL_LOG),"Get UtBindInfo Num=%u ", UtBindInfo_size());
    return XSUCC;
}

/*****************************************************************************
 Prototype    : agentHLR_Syn2HlrUtInfo
 Description  : 向各个hlr分发所有的租赁绑定信息
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/10/22
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_Syn2HlrUtInfo(XVOID)
{

    t_Sj3_Oper_Rsp ossMsgReq = {0};	
    XU16 len = 0;
    XS32 ret = 0;
    XU8 buf[2000] = {0};
    XU32 count= 0;
    SLeaseHoldCtlTable UtLeaseInfo = {0};
    SLeaseHoldCtlPack tPack={0};

    XU32 totle=0;

    if(g_HLRagent_Type != e_TYPE_UDC)
    {
        XOS_Trace(MD(FID_HLR, PL_LOG), "g_HLRagent_Type=%u", g_HLRagent_Type);
        return XSUCC;
    }
    /*将数据库的租赁绑定数据写入内存中*/
    agentHLR_GetUtInfo();

    totle = g_UtBindInfo.size();
    if(0 == totle)
    {
        len += agentHLR_EncodeTLV(buf, e_Agent_PidBindInfo, &tPack, sizeof(SLeaseHoldCtlPack));
        ENCODEOSSREQ(ossMsgReq, e_auth_SynUtBindInfo, len, 0);
        ossMsgReq.OpHead.ucPackId = XTRUE;
        XOS_MemCpy(ossMsgReq.usBuf, buf, len);
        /*向各个hlr分发*/
        ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, MQTT_DELIVERY_2_HLR);
        if(XSUCC != ret)
        {
            XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO HLR UtInfo error!ret = %u", ret);
        }
        XOS_Trace(MD(FID_HLR, PL_LOG), "Clear All Ut info");
        return XSUCC;
        
    }
    map<string, SLeaseHoldCtlTable>::iterator it;
    for(it = g_UtBindInfo.begin(); it != g_UtBindInfo.end(); it++)
    {
        XOS_MemCpy(&UtLeaseInfo, &(it->second), sizeof(SLeaseHoldCtlTable));
        
        XOS_MemCpy(&tPack.UtLeaseInfo[tPack.count], &UtLeaseInfo, sizeof(SLeaseHoldCtlTable));
        tPack.count++;
        count++;
        if(tPack.count >= AGENT_SYNUTINFO_MAXNUM)
        {
            len += agentHLR_EncodeTLV(buf, e_Agent_PidBindInfo, &tPack, sizeof(SLeaseHoldCtlPack));
            ENCODEOSSREQ(ossMsgReq, e_auth_SynUtBindInfo, len, 0);
            if(count == AGENT_SYNUTINFO_MAXNUM)
            {
                ossMsgReq.OpHead.ucPackId = XTRUE;/*代表开始同步标志*/
            }
            XOS_MemCpy(ossMsgReq.usBuf, buf, len);

            /*向各个hlr分发*/
            ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, MQTT_DELIVERY_2_HLR);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO HLR UtInfo error!ret = %u", ret);
            }
            /*结构初始化*/
            XOS_MemSet(&tPack, 0, sizeof(SLeaseHoldCtlPack));
            
            continue;
        }
        else if(count >= totle)
        {
            len += agentHLR_EncodeTLV(buf, e_Agent_PidBindInfo, &tPack, sizeof(SLeaseHoldCtlPack));
            ENCODEOSSREQ(ossMsgReq, e_auth_SynUtBindInfo, len, 0);
            if(count < AGENT_SYNUTINFO_MAXNUM)
            {
                ossMsgReq.OpHead.ucPackId = XTRUE;/*代表开始同步标志*/

            }
             XOS_MemCpy(ossMsgReq.usBuf, buf, len);
            /*向各个hlr分发*/
            ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, MQTT_DELIVERY_2_HLR);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO HLR UtInfo error!ret = %u", ret);
            }
            break;

        }

    }
    /*清理掉内存*/
    g_UtBindInfo.clear();
    XOS_Trace(MD(FID_HLR, PL_LOG), "PUSH TO hlr UtBindInfo = %u, CurrNum=%u", count, g_UtBindInfo.size());
    return XSUCC;
}



#define _____PRO_FROM_HLR___

XS32 agentHLR_ProcUtLeaseMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    XU16 bufLen = 0;
    XS32 ret = XSUCC;
    
    SLeaseHoldCtlTable szLeaseHoldCtlTable = {0};

    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    

    /*解码 消息*/
    ret = agentHLR_DecodeTLV(pHlrMsg->usBuf, bufLen, e_Agent_UtLease, &szLeaseHoldCtlTable);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Decode e_Agent_UtLease Err: bufLen=%u.", bufLen);
        return XERROR;
    }

    
    ret = MysqlDB_UptUtBindInfo(fid, &szLeaseHoldCtlTable);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "UPDATE e_Agent_UtLease Err");
        return XERROR;
    }


    
    return XSUCC;
}


/*****************************************************************************
 Prototype    : agentHLR_UtInfoInit
 Description  : 租赁绑定部分初始化
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UtInfoInit(XVOID)
{
    XS32 ret = 0;


    if(g_HLRagent_Type == e_TYPE_UDC)
    {

        /*下面的循环定时器是 用户缓存信息核查定时器*/
        t_BACKPARA tBackPara = {0};

        tBackPara.para1 = TIMER_TYPE_AGENT_UTBIND_SYN;
        
    	ret = XOS_TimerCreate(FID_HLR, &g_UtBindInfotimerHandle, TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackPara);
    	if (XERROR == ret)
    	{
    		XOS_Trace(MD(FID_HLR,PL_ERR), "XOS_TimerCreate  Err!");
    		return XERROR;
    	}

    	ret = XOS_TimerBegin(FID_HLR, g_UtBindInfotimerHandle, TIMER_LENGTH_AGENT_UTINFO);/*时长为一个小时*/
    	if (XERROR == ret)
    	{
    		XOS_Trace(MD(FID_HLR, PL_ERR), "TimerBegin is Err!");
    		return XERROR;
    	}
    }
    
	return XSUCC;
}
/*****************************************************************************
 Prototype    : agentHLR_UtInfoTimerSynPro
 Description  : 租赁绑定信息时钟同步处理
 Input        : t_BACKPARA *tBackPara  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/25
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UtInfoTimerSynPro(t_BACKPARA *tBackPara)
{
    
    XS32 retValue = 0;
    XU64 cur_time = GetPresentTime();
    t_XOSTD TM_Struct = {0};

    XOS_GetTmTime(&TM_Struct);

    if(!(TM_Struct.dt_hour == 0 && TM_Struct.dt_min == 0))/*不是24点 整不处理直接返回*/
    {
        
        return XSUCC;
    }

    agentHLR_Syn2HlrUtInfo();
    
    XOS_Trace(MD(FID_HLR, PL_ERR), "UT INFO end Syn");
    return XSUCC;
}

