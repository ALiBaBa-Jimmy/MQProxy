#include "fid_def.h"
#include "SPROamCli.h"
#include "agentTrace.h"
#include "SPRMdbOamStruct.h"
#include "SPROamTask.h"
#include "agentDbTask.h"
#include "SPRHdbOam.h"
//定义表记录的Index

#define QRY_LINK_INFO 5

// 命令行注册函数
XS32  RegAgentOamCliCmd(void)
{
	XS32 promptID;

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "agent", "agent", " ");	

	//数据库配置
	if (0 > XOS_RegistCommand(promptID, CLIOamDbCfg, "dbcfg","configure the database", " "))
	{
        agentTrace(XNULL,MD(FID_AGENTOAM, PL_ERR), "Failed to register command"  "XOS>ehssoam>dbcfg!");
        return XERROR;
    }

	return XSUCC;
}


/**********************************************************************
*
*  NAME:          CLIOamDbCfg
*  FUNTION:       db配置
*  INPUT:         
*  OUTPUT:        成功失败
*  OTHERS:        其他需说明的问题
**********************************************************************/
XVOID CLIOamDbCfg(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
	AGT_OAM_CFG_REQ_T szReqMsg = {0};
	AGT_OAM_CFG_RSP_T szRspMsg = {0};
	SEhssOamDbCfg szEhssOamDbCfg = {0};

	XU8 revStr = 0;
	XS32 ret = XSUCC;
	XU32 iRecIndex = 0;
	//获取cli命令的参数
	if(siArgc <2)
	{
		XOS_CliExtPrintf(pCliEnv,"Usage: \r\ndbcfg 5:qry [dbip dbusername dbpwd dbname dbstatus]");
		return ;
	}
	else
	{
		revStr = atoi(ppArgv[1]);
		if ((revStr == OAM_CFG_ADD) || (revStr == OAM_CFG_MOD))
		{
			if (siArgc < 6)
			{
				XOS_CliExtPrintf(pCliEnv ,"param error");
				return ;
			}
		}
		if (revStr == OAM_CFG_GET)
		{
			//if (siArgc < 3)
			{
				XOS_CliExtPrintf(pCliEnv ,"param error");
				return ;
			}
		}
		if (revStr == OAM_CFG_FMT)
		{
			if ((siArgc-2) %4 != 0)
			{
				XOS_CliExtPrintf(pCliEnv ,"param error");
				return ;
			}
		}
	}

	
	switch(revStr)
	{

		case QRY_LINK_INFO:
			{
				SDbConfigInfo szDbCfgInfo = {0};
				XBOOL bDbStatus = GetDbLinkStatus();
				ret = GetDbCfgInfo(&szDbCfgInfo);
				if (XSUCC == ret)
				{
				    XOS_CliExtPrintf(pCliEnv,"%-9s %9s %10s %7s\r\n","USER", "PASSWORD", "CONNSTRING","Status");
                    XOS_CliExtPrintf(pCliEnv,"======================================\r\n");
                    XOS_CliExtPrintf(pCliEnv,"%-9s %9s %10s %7s\r\n", szDbCfgInfo.user,
                                                                     szDbCfgInfo.pwd,
                                                                     szDbCfgInfo.ip,
                                                                    bDbStatus ? "1(OK)" : "0(Error)");

				}
				else
				{
					XOS_CliExtPrintf(pCliEnv,"linkinfo not exsit\n");
				}
				return;
				
			}
			break;
		case  OAM_CFG_FMT://格式化操作
			{
				szReqMsg.uiRecNum = 1;       //表记录数
				szReqMsg.uiMsgLen = sizeof(SEhssOamDbCfg);       //消息长度
			}
			break;
		default:
			{
				XOS_CliExtPrintf(pCliEnv,"opertype not define error");
				return ;
			}
			break;
	}
    #if 0
	if ((revStr != OAM_CFG_DEL) &&(revStr != OAM_CFG_GET))
	{
		XOS_MemCpy(szEhssOamDbCfg.cIpAddr,ppArgv[2],XOS_StrLen(ppArgv[2]));
		XOS_MemCpy(szEhssOamDbCfg.cUserName,ppArgv[3],XOS_StrLen(ppArgv[3]));
		XOS_MemCpy(szEhssOamDbCfg.cPwd,ppArgv[4],XOS_StrLen(ppArgv[4]));
		XOS_MemCpy(szEhssOamDbCfg.cDbName,ppArgv[5],XOS_StrLen(ppArgv[5]));
	}	
	//1  begin TEST 配置接入的受理台ip
	szReqMsg.uiIndex = 1;      //索引
    szReqMsg.usNeId = 1;        //网元ID
    szReqMsg.usModuleId = 1;     //模块ID
	szReqMsg.uiOperType =  revStr ;//操作类型 见 OAM_CFG_E
    szReqMsg.uiTableId = EHSS_DB_CFG;      //表ID

	szReqMsg.pData = (XS8*)XOS_Malloc(szReqMsg.uiMsgLen);
	if (szReqMsg.pData == NULL)
	{
		return ;
	}
	
	if (revStr != OAM_CFG_GET)
	{
		XOS_MemCpy(szReqMsg.pData,&szEhssOamDbCfg,szReqMsg.uiMsgLen);	
	}
	else
	{
		XOS_MemCpy(szReqMsg.pData,&iRecIndex,szReqMsg.uiMsgLen);	
	}
	

	ret = EhssMdbOamCfg(&szReqMsg,&szRspMsg);
	XOS_CliExtPrintf(pCliEnv,"ret:%d\r\n",ret);

	if (szRspMsg.pRetData != NULL)
	{
	XOS_CliExtPrintf(pCliEnv,"recIndex:%d\r\ndblinkstatus:%d\r\n",\
			((SEhssOamDbQryResult *)szRspMsg.pRetData)->recIndex,\
			((SEhssOamDbQryResult *)szRspMsg.pRetData)->cDbLinkStatus);
	}

	if (szReqMsg.pData != NULL)
	{
		free(szReqMsg.pData);
	}
	if (szRspMsg.pRetData != NULL)
	{
		free(szRspMsg.pRetData);
	}
    #endif
	return ;	
	
}

