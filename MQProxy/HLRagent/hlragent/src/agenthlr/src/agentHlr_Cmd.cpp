/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年10月17日
**************************************************************************/

#include "agentHlrCom.h"
#include "agentHlr_Cmd.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"
#include "agentHlrUtBind.h"

extern map<string, tContext*> g_UserInfo;
extern XOS_HHASH     gHashTableDn;
extern tClearContextTime g_ClearUserInfoTime;
extern map<string, SLeaseHoldCtlTable> g_UtBindInfo;;
XVOID CLIOamDbtest(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 tmp[4] = {0};
    XU8 uidstr[9] = {0};
    XU8 uid[4] = {0};
    XU8 networkid[12] = {0};

    XU8 tel[16] = {0};
//008613120199852FFFFFFFFFFFFFFFFF
    tel[0] = 0X00;
    tel[1] = 0X86;
    tel[2] = 0X13;
    tel[3] = 0X12;
    tel[4] = 0X01;
    tel[5] = 0X99;
    tel[6] = 0X85;
    tel[7] = 0X2F;
    tel[8] = 0XFF;
    tel[9] = 0XFF;
    tel[10] = 0XFF;
    tel[11] = 0XFF;
    tel[12] = 0XFF;
    tel[13] = 0XFF;
    tel[14] = 0XFF;
    tel[15] = 0XFF;
    //agentDB_QryUIDbyTelNo(FID_HLR, uid, tel);

    XOS_CliExtPrintf(pCliEnv,"uid=%02X%02X%02X%02X", uid[0], uid[1],uid[2],uid[3]);

    MysqlDB_QryuidbyTel(FID_HLR, uid, tel);
    XOS_CliExtPrintf(pCliEnv,"uid=%02X%02X%02X%02X", uid[0], uid[1],uid[2],uid[3]);
    
    //XOS_CliExtPrintf(pCliEnv,"networkid=%s", networkid);
    return;
}
/*****************************************************************************
 Prototype    : CLIOam_QryUserInfobyuid
 Description  : telnet命令：根据uid查询用户缓存
 Input        : CLI_ENV *pCliEnv  
                XS32 siArgc       
                XCHAR **ppArgv    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XVOID CLIOam_QryUserInfobyuid(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 tmp[4] = {0};
    XU8 uidstr[9] = {0};
    XU8 uid[4] = {0};
    XS32 ret = 0;
    XU64 curr_time = 0;
    tContext Result = {0};
    XU8 reg_timeStr[LENGTH_OF_DATE+1] = {0};
    XU8 curr_timeStr[3*LENGTH_OF_DATE+1] = {0};
    XU8 upt_timeStr[2*LENGTH_OF_DATE+1] = {0};
    XU8 cCamtalkStatus = 0;
    XU8 telStr[2*LENGTH_OF_TEL + 1] = {0};

    //agentHLR_TimeMm2Str(497808682409, (XS8*)upt_timeStr);
    //XOS_CliExtPrintf(pCliEnv," updatetime = %s(%lld)\r\n", upt_timeStr, Result.updatetime);
    
    XOS_CliExtPrintf(pCliEnv,"  Totle Num = %u\r\n", UserInfo_size());
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"Ex: Qrybyuid 009A870D ");
        return;
    }
    
    agentHLR_HexToStr((XU8*)ppArgv[1], uid, LENGTH_OF_UID);

    ret = UserInfo_Qrybyuid(uid, &Result);
    if(XSUCC != ret)
    {
        XOS_CliExtPrintf(pCliEnv,"qry uid=%s failed", ppArgv[1]);
        return;
    }
    
    /*打印部分的用户缓存信息*/

    /*缓存最近的更新时间*/
    agentHLR_TimeMm2Str(Result.updatetime, (XS8*)upt_timeStr);
    XOS_CliExtPrintf(pCliEnv," updatetime = %s(%lld)\r\n", upt_timeStr, Result.updatetime);

    XOS_CliExtPrintf(pCliEnv,"        uid = %02X%02X%02X%02X\r\n", uid[0], uid[1],uid[2],uid[3]);

    XOS_MemCpy(reg_timeStr, Result.tHashDyn.prevRegDate, LENGTH_OF_DATE);
    XOS_CliExtPrintf(pCliEnv,"privRegDate = %s\r\n", reg_timeStr);
    XOS_MemCpy(reg_timeStr, Result.tHashDyn.currRegDate, LENGTH_OF_DATE);
    XOS_CliExtPrintf(pCliEnv," curRegDate = %s\r\n", reg_timeStr);
    
    
    XOS_CliExtPrintf(pCliEnv,"    homeNet = %s\r\n", Result.tHashStat.LocalNetwork);
    XOS_CliExtPrintf(pCliEnv," update Net = %s\r\n", Result.tHashDyn.cNetworkID);
    XOS_CliExtPrintf(pCliEnv,"      sagid = %02X%02X%02X%02X\r\n", 
        Result.tHashDyn.vssID[0],
        Result.tHashDyn.vssID[1],
        Result.tHashDyn.vssID[2],
        Result.tHashDyn.vssID[3]);

    XOS_CliExtPrintf(pCliEnv,"===== **Camtalk Info** ======\r\n");
    XOS_CliExtPrintf(pCliEnv,"   privRegDate = %s\r\n", Result.tHashDyn.hashCamtalkDyn.privRegDate);
    XOS_CliExtPrintf(pCliEnv,"    curRegDate = %s\r\n", Result.tHashDyn.hashCamtalkDyn.curRegDate);
    
    agentHLR_StrToHex(Result.tHashDyn.hashCamtalkDyn.camtalkTelno, telStr, LENGTH_OF_TEL);
    XOS_CliExtPrintf(pCliEnv,"  camtalkTelno = %s\r\n", telStr);
    
    cCamtalkStatus = Result.tHashDyn.hashCamtalkDyn.cCamtalkStatus;
    XOS_CliExtPrintf(pCliEnv,"cCamtalkStatus = %s(%u)\r\n", cCamtalkStatus==XTRUE?"Power On":"Power Off", cCamtalkStatus);
    
    XOS_CliExtPrintf(pCliEnv,"         cagid = %02X%02X%02X%02X\r\n", 
        Result.tHashDyn.hashCamtalkDyn.cssId[0],
        Result.tHashDyn.hashCamtalkDyn.cssId[1],
        Result.tHashDyn.hashCamtalkDyn.cssId[2],
        Result.tHashDyn.hashCamtalkDyn.cssId[3]);
    XOS_CliExtPrintf(pCliEnv,"     public ip = %u\r\n", Result.tHashDyn.hashCamtalkDyn.iCssPublicIp);
    XOS_CliExtPrintf(pCliEnv,"    private ip = %u\r\n", Result.tHashDyn.hashCamtalkDyn.iCssPrivateIp);
    
    
    curr_time = GetPresentTime();

    agentHLR_GetCurSysTime((XS8*)curr_timeStr);
    if(curr_time >= g_UpdateRequestTimeOut+Result.updatetime)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n Warn: unavailable Curr_time: %s\r\n", curr_timeStr);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n Info: available Curr_time: %s\r\n", curr_timeStr);
    }
    
    return;
}
/*****************************************************************************
 Prototype    : CLIOam_DelUserInfobyuid
 Description  : telnet命令： 根据uid删除agent缓存
 Input        : CLI_ENV *pCliEnv  
                XS32 siArgc       
                XCHAR **ppArgv    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XVOID CLIOam_DelUserInfobyuid(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 tmp[4] = {0};
    XU8 uidstr[9] = {0};
    XU8 uid[4] = {0};
    XS32 ret = 0;
    tContext Result = {0};
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"Ex: DelUserInfobyuid 009A870D ");
        return;
    }
    agentHLR_HexToStr((XU8*)ppArgv[1], uid, LENGTH_OF_UID);

    ret = UserInfo_Qrybyuid(uid, &Result);
    if(XSUCC != ret)
    {
        XOS_CliExtPrintf(pCliEnv,"Qry uid=%s failed", ppArgv[1]);
        return;
    }
    
    ret = UserInfo_Deletebyuid(uid);
    if(XSUCC != ret)
    {
        XOS_CliExtPrintf(pCliEnv,"Delete UserInfo uid=%s failed", ppArgv[1]);
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"Delete UserInfo uid=%s succefully", ppArgv[1]);
    
    return;
}
/*****************************************************************************
 Prototype    : CLIOam_DelAllUserInfobyuid
 Description  : telnet命令： 根据uid删除agent缓存
 Input        : CLI_ENV *pCliEnv  
                XS32 siArgc       
                XCHAR **ppArgv    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XVOID CLIOam_DelAllUserInfobyuid(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 tmp[4] = {0};
    XU8 uidstr[9] = {0};
    XU8 uid[4] = {0};
    XS32 ret = 0;


    XU64 cur_time = GetPresentTime();
    map<string,tContext*>::iterator it;
    
 
    for(it = g_UserInfo.begin(); it != g_UserInfo.end(); it++)
    {
      
        if(XSUCC != UserInfo_Deletebyuid(it->second->uid))
        {
            XOS_CliExtPrintf(pCliEnv,"Delete  UserInfo = %02X%02X%02X%02X failed",
                it->second->uid[0],it->second->uid[1],it->second->uid[2],it->second->uid[3]);
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"Delete All UserInfo succefully");           
        }
       
  
    }
    
    return;
}
/*****************************************************************************
 Prototype    : CLIOam_DelUserInfobyuid
 Description  : telnet命令： 根据uid删除agent缓存
 Input        : CLI_ENV *pCliEnv  
                XS32 siArgc       
                XCHAR **ppArgv    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XVOID CLIOam_cfgTimeofClear(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 tmp[4] = {0};
    XU8 uidstr[9] = {0};
    XU8 uid[4] = {0};
    XS32 ret = 0;


    XU64 cur_time = GetPresentTime();
    map<string,tContext*>::iterator it;
    
 
    for(it = g_UserInfo.begin(); it != g_UserInfo.end(); it++)
    {
      
        if(XSUCC != UserInfo_Deletebyuid(it->second->uid))
        {
            XOS_CliExtPrintf(pCliEnv,"Delete  UserInfo = %02X%02X%02X%02X failed",
                it->second->uid[0],it->second->uid[1],it->second->uid[2],it->second->uid[3]);
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"Delete All UserInfo succefully");           
        }
       
  
    }
    
    return;
}

/*****************************************************************************
 Prototype    : CLIOam_QryuidbyTel
 Description  : 根据电话号码查询uid命令
 Input        : CLI_ENV *pCliEnv  
                XS32 siArgc       
                XCHAR **ppArgv    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XVOID CLIOam_QryuidbyTel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    
    XU8 telstr[2*LENGTH_OF_TEL + 1] = {0};
    XU8 tel[LENGTH_OF_TEL] = {0};
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"Ex: QryuidbyTel 008662803316 ");
        return;
    }
    
    
    XOS_MemSet(tel, 0xFF, LENGTH_OF_TEL);
    XOS_MemSet(telstr, 'F', 2*LENGTH_OF_TEL);
    XOS_MemCpy(telstr, ppArgv[1], XOS_StrLen(ppArgv[1]));
    agentHLR_HexToStr(telstr, tel, LENGTH_OF_TEL);
    
    
    
    XU8 *uid = (XU8*)XOS_HashElemFind(gHashTableDn, tel);
    if(uid == XNULL)
    {
        XOS_CliExtPrintf(pCliEnv,"Qry uid by tel= %s failed", telstr);
        return;
    }

    XOS_CliExtPrintf(pCliEnv," uid = %u ", XOS_NtoHl(*(XU32*)uid));
    XOS_CliExtPrintf(pCliEnv," tel = %s ", telstr);
    
    
    
    return;
}


XVOID CLIOam_CfgUserInfoMemTime(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{

    XU16 hour = 0;
    XU16 minitus = 0;
	XOS_CliExtPrintf(pCliEnv,"Old Info: Hour %u:%u \r\n", g_ClearUserInfoTime.hour,g_ClearUserInfoTime.minitus);
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"Ex: cfgTimeOut 0 0  ");
        return;
    }

    hour = atoi(ppArgv[1]);
    minitus = atoi(ppArgv[2]);
    
    
    g_ClearUserInfoTime.hour = hour;
    g_ClearUserInfoTime.minitus = minitus;

    XOS_CliExtPrintf(pCliEnv,"New Info: %u:%u\r\n", g_ClearUserInfoTime.hour,g_ClearUserInfoTime.minitus);
    return;
}



XVOID CLIOam_SynAllUtBindInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    agentHLR_Syn2HlrUtInfo();
    
    return;
}



XVOID CLIOam_QryAllUtBindInfo(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU8 pid[LENGTH_OF_PID]={0};
    XU8 pidport[LENGTH_OF_PID+1]={0};
    XU8 port = 0;

    XU8 pidportStr[11] = {0};

    SLeaseHoldCtlTable Result = {0};
    
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"Ex: QryUtInfo pid port");
        return;
    }
    
    if(g_HLRagent_Type != e_TYPE_UDC)
    {
        XOS_Trace(MD(FID_HLR, PL_LOG), "g_HLRagent_Type=%u", g_HLRagent_Type);
        return ;
    }
    agentHLR_HexToStr((XU8*)ppArgv[1], pid, LENGTH_OF_PID);
    port=atoi(ppArgv[2]);

    XOS_MemCpy(pidport, pid, LENGTH_OF_PID);
    pidport[LENGTH_OF_PID] = port;
    agentHLR_StrToHex(pidport, pidportStr, LENGTH_OF_PID+1);
    
    string tmp((XS8*)pidportStr);

    
    agentHLR_GetUtInfo();

    map<string, SLeaseHoldCtlTable>::iterator it = g_UtBindInfo.find(tmp);
    XOS_CliExtPrintf(pCliEnv,"Totle Num = %u\r\n", g_UtBindInfo.size());
    if(g_UtBindInfo.end() != it)
    {
        XOS_MemCpy(&Result, &(it->second), sizeof(SLeaseHoldCtlTable));
        XOS_CliExtPrintf(pCliEnv,"           pid = %02X%02X%02X%02X\r\n", Result.pid[0], Result.pid[1], Result.pid[2], Result.pid[3]);
        XOS_CliExtPrintf(pCliEnv,"          port = %u\r\n", Result.port);
        XOS_CliExtPrintf(pCliEnv,"     creditUid = %02X%02X%02X%02X\r\n", 
            Result.creditUid[0], Result.creditUid[1], Result.creditUid[2], Result.creditUid[3]);
        XOS_CliExtPrintf(pCliEnv,"       bindUid = %02X%02X%02X%02X\r\n",
            Result.bindUid[0], Result.bindUid[1], Result.bindUid[2], Result.bindUid[3]);
        XOS_CliExtPrintf(pCliEnv,"terminalStatus = %u\r\n", Result.terminalStatus);
        XOS_CliExtPrintf(pCliEnv,"  creditStatus = %u\r\n", Result.creditStatus);
        XOS_CliExtPrintf(pCliEnv,"         NetId = %s\r\n", Result.NetId);
        
		
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"Qry failed");
    }

    

    g_UtBindInfo.clear();
    return;
}

/*****************************************************************************
 Prototype    : agentHLR_CmdInit
 Description  : telnet命令行初始化
 Input        : XVOID  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_CmdInit(XVOID)
{
    XS32 promptID;

    
    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "agent", "agent", " ");	

	//数据库配置
	if (0 > XOS_RegistCommand(promptID, CLIOamDbtest, "dbtest","dbtest", " "))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register command"  "XOS>ehssoam>dbcfg!");
        return XERROR;
    }

    /*agent 缓存配置*/
 	if (0 > XOS_RegistCommand(promptID, CLIOam_QryuidbyTel, "QryuidbyTel","QryuidbyTel", "Qry Userid by Tel"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register QryuidTel"  " ");
        return XERROR;
    }
   
	if (0 > XOS_RegistCommand(promptID, CLIOam_QryUserInfobyuid, "QryUserInfo","QryUserInfo", "Qry User info in Memory by uid"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register QryUserInfo"  " ");
        return XERROR;
    }
	if (0 > XOS_RegistCommand(promptID, CLIOam_DelUserInfobyuid, "DelUserInfo","DelUserInfo", "Del User info in Memory by uid"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register DelUserInfo"  " ");
        return XERROR;
    }
	if (0 > XOS_RegistCommand(promptID, CLIOam_DelAllUserInfobyuid, "DelAllUserInfo","DelAllUserInfo", "Del All User info in Memory by uid"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register DelUserInfo"  " ");
        return XERROR;
    }

	if (0 > XOS_RegistCommand(promptID, CLIOam_CfgUserInfoMemTime, "cfgTimeOut","cfgTimeOut", "Set User Info in Memory timer Len"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register cfgTimeOut"  "XOS!");
        return XERROR;
    }

	if (0 > XOS_RegistCommand(promptID, CLIOam_CfgUserInfoMemTime, "cfgTimeOfClear","cfgTimeOfClear", "Del All User info in Memory by timer"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register DelUserInfo"  " ");
        return XERROR;
    }

	if (0 > XOS_RegistCommand(promptID, CLIOam_SynAllUtBindInfo, "SynUtInfo","SynUtInfo", "Syn All Ut bind info 2 hlr"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register DelUserInfo"  " ");
        return XERROR;
    }
	if (0 > XOS_RegistCommand(promptID, CLIOam_QryAllUtBindInfo, "QryUtInfo","QryUtInfo", "Qry All Ut bind info 2 hlr"))
	{
        XOS_Trace(MD(FID_HLR, PL_ERR), "Failed to register DelUserInfo"  " ");
        return XERROR;
    }

    return XSUCC;
}


