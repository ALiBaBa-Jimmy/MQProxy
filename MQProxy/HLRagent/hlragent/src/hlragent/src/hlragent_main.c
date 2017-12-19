/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssDbTask.cpp
* Author:       luhaiyan
* Date：        10-15-2014
* OverView:     任务入口
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

#include "fid_def.h"
#include "MqttTask.h"
#include "SPROamTask.h"
#include "agentuaTask.h"
#include "agentHlrTask.h"
#include "agentDbTask.h"
#include "taskcommon.h"
#include "ha_interface.h"


extern XU64 g_UpdateRequestTimeOut;
STaskContextAry g_szAgentOamTaskContextAry={0};
STaskContextAry g_szMQTTTaskContextAry={0};
STaskContextAry g_szAgentUaTaskContextAry={0};
STaskContextAry g_szAgentDBTaskContextAry={0};
STaskContextAry g_szAgentHlrTaskContextAry={0};

t_XOSFIDLIST g_agentTaskOam  =
{
		{"FID_AGENTOAM",  XNULL, FID_AGENTOAM},
		{agent_OamInitProc , agent_OamNoticeProc, XNULL},
		{agent_OamMsgProc,  agent_OamTimeoutProc},
		eXOSMode,
		XNULL
};			


/*MQTT初始化注册结构体*/
t_XOSFIDLIST g_MQTTTaskInfo = 
{
	{"FID_MQTT",  XNULL, FID_MQTT},
	{MQTT_InitProc, MQTT_NoticeProc, XNULL},
	{MQTT_XosMsgProc,  MQTT_TimeoutProc},
	eXOSMode,
	XNULL
};

/*FID_UA 初始化注册结构体*/
t_XOSFIDLIST g_AgentUaTaskInfo = 
{
	{"FID_UA",  XNULL, FID_UA},
	{agentUA_InitProc, agentUA_NoticeProc, XNULL},
	{agentUA_XosMsgProc,  agentUA_TimeoutProc},
	eXOSMode,
	XNULL
};

/*数据库初始化注册结构体*/
t_XOSFIDLIST g_agentDBTaskInfo = 
{
		{"FID_AGENTDB",  XNULL, FID_AGENTDB},
		{agentDB_InitProc, agentDB_NoticeProc, XNULL},
		{agentDB_XosMsgProc,  agentDB_TimeoutProc},
		eXOSMode,
		XNULL
};
/*FID_UA 初始化注册结构体*/
t_XOSFIDLIST g_AgentHlrTaskInfo = 
{
	{"FID_HLR",  XNULL, FID_HLR},
	{agentHLR_InitProc, agentHLR_NoticeProc, XNULL},
	{agentHLR_XosMsgProc,  agentHLR_TimeoutProc},
	eXOSMode,
	XNULL
};


//设置消息队列的个数
XVOID SetLoginListQueNum(t_XOSLOGINLIST *t,XS32 argc,XS8 **argv)
{
	if(argc > 0)
	{
		//消息队列中消息的个数
		if((XNULLP != argv[0]) && (XOS_StrLen(argv[0]) > 0))
		{
			t->quenum = (XU32)strtoul(argv[0],NULL,10);
		}
		else
		{
			t->quenum = MAX_MSGS_IN_QUE;
		}
	}	
}

XS32 GetArgvbyNo(XS32 argc,XS8 **argv,XU8 Num, XU8* pResult)
{

	if(argc> Num)
	{
		
        XOS_MemCpy(pResult, argv[Num], XOS_StrLen(argv[Num]));
        
	}
	return XSUCC;
}
XU16 GetThreadNum(XS32 argc,XS8 **argv)
{
	XU16 temp = 1;
	if(argc> 1)
	{
		temp = (XU16)strtoul(argv[1],NULL,10);
		if(temp <= 0)
		{
			temp = 1;
		}
		else if(temp > 10)
		{
			temp = 10;
		}
	}
	return temp;
}
//1 ---- 进行状态切换
//0 ---- 不进行状态切换
XS32 HaResWatchCall(XU32 Resource, void *param)
{
	if(XOS_HA_WATCH_MEM & Resource)				//"内存满"
	{
		XOS_Trace(MD(*(XU32*)param,PL_WARN),"FID %d :Memory is full",*(XU32*)param);
		return 1;		//状态切换
	}
	else if(XOS_HA_WATCH_MSG_QUE & Resource)	//"队列满"
	{
		XOS_Trace(MD(*(XU32*)param,PL_WARN),"FID %d :MsgQue is full",*(XU32*)param);
		return 0;		//不进行状态切换
	}
	return XSUCC;
}


//命名需要修改
XU32 StartMutlTask(t_XOSLOGINLIST *xosLoginList,XU32 StartFid,STaskContextAry* pTaskContextAry,
		  XU8*taskNamePre, t_XOSFIDLIST* pOrgiXosFidList)
{
	XU32 i = 0;
	XU32 taskIndex = 0;
	XS32 ret = XSUCC;
	XS32 ret2 = XSUCC;
    XU32 fid = 0;
	t_XOSFIDLIST * XTask = XNULL;
	STaskContext * currentTaskContext = XNULL;
	XS8 taskName[MAX_FID_NAME_LEN +1] = {0};
	XS8 fidName[MAX_FID_NAME_LEN +1] = {0};
	pTaskContextAry->pTaskContext = (STaskContext *)XOS_Malloc(sizeof(STaskContext)
		* pTaskContextAry->taskcount);
	XOS_MemSet(pTaskContextAry->pTaskContext,0,(sizeof(STaskContext) * pTaskContextAry->taskcount));


	for(i = 0 ; i < pTaskContextAry->taskcount ; i++)
	{
		taskIndex = i;		
        fid = StartFid + taskIndex;
		XTask = (t_XOSFIDLIST *)XOS_Malloc(sizeof(t_XOSFIDLIST));	
        if(XNULL == XTask)
        {
            XOS_Trace(MD(fid,PL_ERR),"XOS_Malloc is NULL");
            continue;
        }
		if(pTaskContextAry->taskcount>1)
		{
			sprintf(fidName,"FID_%s_%02d",taskNamePre, (taskIndex+1));
			sprintf(taskName,"Tsk_%s_%02d",taskNamePre, (taskIndex+1));
		}
		else
		{
			sprintf(fidName,"FID_%s",taskNamePre);
			sprintf(taskName,"Tsk_%s",taskNamePre);
		}
       
		XOS_MemCpy(XTask, pOrgiXosFidList,sizeof(t_XOSFIDLIST));		
		XOS_MemCpy(XTask->head.FIDName,fidName, sizeof(taskName));
		XTask->head.FID = StartFid + taskIndex;
		xosLoginList->stack				=	XTask;
		
		XOS_StrNcpy(xosLoginList->taskname,taskName,MAX_TID_NAME_LEN);
		xosLoginList->TID				=	StartFid+taskIndex;
		xosLoginList->prio				=	TSK_PRIO_NORMAL;
		xosLoginList->stacksize			=	XNULL;
		currentTaskContext = &pTaskContextAry->pTaskContext[taskIndex];
		currentTaskContext->taskTid		=	xosLoginList->TID;
		currentTaskContext->taskIndex	=	taskIndex;
		currentTaskContext->taskFidList	=	XTask;
		currentTaskContext->taskFid		=	XTask->head.FID;
		currentTaskContext->taskcount   =   pTaskContextAry->taskcount;
		currentTaskContext->startFid	=   StartFid;
		ret = XOS_MMStartFid(xosLoginList,XNULLP,(XVOID*)currentTaskContext);
        #if 0
		ret2 = XOS_HA_ResWatchReg(fid,XOS_HA_WATCH_ALL,HaResWatchCall,&fid);
		if(XSUCC != ret2)
		{
			XOS_Trace(MD(fid, PL_ERR), "XOS_HA_ResWatchReg Error!!! FID is %d",fid);
		}
        #endif
	}
	return ret;
	
}

/**************************************************************************
函 数 名: SPRDbEntryFunc
函数功能: 数据库线程入口函数
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS32 MQTTEntryFunc(HANDLE hDir,XS32 argc, XS8** argv)
{



    t_XOSLOGINLIST xosLoginList ={0};

	XU32 ret = XSUCC;
	XU8 taskName[MAX_FID_NAME_LEN + 1] = {0};
	sprintf((XS8*)taskName,"%s","Tsk_MQTT");
	
	g_szMQTTTaskContextAry.taskcount = 1;

	SetLoginListQueNum(&xosLoginList,argc,argv);
	g_szMQTTTaskContextAry.taskcount = GetThreadNum(argc,argv);
	
	ret = StartMutlTask(&xosLoginList,FID_MQTT, &g_szMQTTTaskContextAry, taskName, &g_MQTTTaskInfo);



	return ret;
}
/**************************************************************************
函 数 名: OamEntryFunc
函数功能: OamEntryFunc入口函数
参    数:
返 回 值:
**************************************************************************/
XS32  AGENT_OamEntryFunc(HANDLE hDir,XS32 argc, XS8** argv)
{

    t_XOSLOGINLIST xosLoginList ={0};

	XU32 ret = XSUCC;
	XU8 taskName[MAX_FID_NAME_LEN + 1] = {0};
	sprintf((XS8*)taskName,"%s","Tsk_AgOam");
	
	g_szAgentOamTaskContextAry.taskcount = 1;

	SetLoginListQueNum(&xosLoginList,argc,argv);
	g_szAgentOamTaskContextAry.taskcount = GetThreadNum(argc,argv);
	
	ret = StartMutlTask(&xosLoginList,FID_AGENTOAM, &g_szAgentOamTaskContextAry, taskName, &g_agentTaskOam);



	return ret;

}
/**************************************************************************
函 数 名: OamEntryFunc
函数功能: OamEntryFunc入口函数
参    数:
返 回 值:
**************************************************************************/
XS32  AGENT_UaEntryFunc(HANDLE hDir,XS32 argc, XS8** argv)
{

    t_XOSLOGINLIST xosLoginList ={0};

	XU32 ret = XSUCC;
	XU8 taskName[MAX_FID_NAME_LEN + 1] = {0};
	sprintf((XS8*)taskName,"%s","Tsk_AgUA");
	
	g_szAgentUaTaskContextAry.taskcount = 1;

	SetLoginListQueNum(&xosLoginList,argc,argv);
	g_szAgentUaTaskContextAry.taskcount = GetThreadNum(argc,argv);
	
	ret = StartMutlTask(&xosLoginList,FID_UA, &g_szAgentUaTaskContextAry, taskName, &g_AgentUaTaskInfo);



	return ret;

}
/**************************************************************************
函 数 名: OamEntryFunc
函数功能: OamEntryFunc入口函数
参    数:
返 回 值:
**************************************************************************/
XS32  AGENT_DBEntryFunc(HANDLE hDir,XS32 argc, XS8** argv)
{

    t_XOSLOGINLIST xosLoginList ={0};

	XU32 ret = XSUCC;
	XU8 taskName[MAX_FID_NAME_LEN + 1] = {0};
	sprintf((XS8*)taskName,"%s","Tsk_AgDB");
	
	g_szAgentDBTaskContextAry.taskcount = 1;

	SetLoginListQueNum(&xosLoginList,argc,argv);
	g_szAgentDBTaskContextAry.taskcount = GetThreadNum(argc,argv);
	
	ret = StartMutlTask(&xosLoginList,FID_AGENTDB, &g_szAgentDBTaskContextAry, taskName, &g_agentDBTaskInfo);



	return ret;

}

/**************************************************************************
函 数 名: OamEntryFunc
函数功能: OamEntryFunc入口函数
参    数:
返 回 值:
**************************************************************************/
XS32  AGENT_HLREntryFunc(HANDLE hDir,XS32 argc, XS8** argv)
{
    
    t_XOSLOGINLIST xosLoginList ={0};

	XU32 ret = XSUCC;
	XU8 taskName[MAX_FID_NAME_LEN + 1] = {0};
    XU8 tmp[32] = {0};
	sprintf((XS8*)taskName,"%s","Tsk_AgHLR");
	
	g_szAgentHlrTaskContextAry.taskcount = 1;

	SetLoginListQueNum(&xosLoginList,argc,argv);
	g_szAgentHlrTaskContextAry.taskcount = GetThreadNum(argc,argv);

    GetArgvbyNo(argc,argv,2,tmp);
    g_UpdateRequestTimeOut = (atoi(tmp))*60*1000;
    
    //g_UpdateRequestTimeOut = g_UpdateRequestTimeOut>0?g_UpdateRequestTimeOut:3*60*1000;
	
	ret = StartMutlTask(&xosLoginList,FID_HLR, &g_szAgentHlrTaskContextAry, taskName, &g_AgentHlrTaskInfo);



	return ret;

}


