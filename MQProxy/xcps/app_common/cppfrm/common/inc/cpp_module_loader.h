/************************************************************************************
文件名  :cpp_module_loader.h

文件描述:封装模块加载所需要的宏

作者    :zzt

创建日期:2006/05/09

修改记录:
          框架与XOS平台联调,平台提供模块管理功能
          20060926--修改代码以适应平台的按脚本加载模块
          1.模块间的依赖关系已完全由脚本中加载的顺序来决定
************************************************************************************/
#ifndef __CPP_MODULE_LOADER_H_
#define __CPP_MODULE_LOADER_H_

#include "cpp_tcn_map.h"


#define TSK_STACK_SIZE   65535

struct CLink_XOSFIDLIST
{
    XVOID*           m_pFct;      //工厂
    t_XOSFIDLIST     m_fData;     //模块对外提供的一些接口函数
};

enum ETskPrio
{
    LOW_TSK   = TSK_PRIO_LOWER,
    NRML_TSK  = TSK_PRIO_NORMAL
};

typedef tcn::map<MDID,CLink_XOSFIDLIST*> CModuleMap;


//获取模块map
CModuleMap& GetModuleMap();


//按模块ID查找模块信息列表。模块ID是板内全局唯一。
XVOID* FindFct(MDID mid);


//按模块ID查找模块名称
XS8* GetModuleName(MDID mid, XS8* pOutBuf);


/*********************************************************************
名称 : REG_MODULE_TASK_BEGIN
职责 : 注册一个任务的起始宏.起始宏的参数是任务名。taskId是
协作 : 注意 消息优先级 推荐TSK_PRIO_NORMAL
历史 :
       修改者   日期          描述
**********************************************************************/
#define REG_TASK_BEGIN(taskName)                                \
extern t_XOSLOGINLIST g_task_##taskName;                        \
t_XOSLOGINLIST g_task_##taskName =                              \
{                                                               \
    XNULL,                                                      \
    #taskName,                                                  \
    0,  //这里任务ID填写一个任意值

#define REG_TASK_PRI_STACKSIZE(pri,stackSize)                   \
    pri,                                                        \
    stackSize,

#define REG_TASK_END()                                          \
};


/*********************************************************************
名称 : REG_FID_BEGIN
职责 :
       注册一个模块.
       moduleId 模块ID－－是一个常量或宏名字，代表一个整数
       timerCallBack  --  定时器操时回调函数
       memFreeType    --  消息释放模式，由应用释放还是由平台释放
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
#define REG_FID_BEGIN(moduleId,timerCallBack,memFreeType)       \
extern XS8 close_module_##moduleId(XVOID *, XVOID *);           \
extern XS8 msg_entry_##moduleId(XVOID* ,XVOID *);               \
extern XS8 notice_##moduleId(XVOID*, XVOID*);                   \
extern XS8 init_module_##moduleId(XVOID *, XVOID *);            \
CLink_XOSFIDLIST __module_##moduleId =                          \
{                                                               \
    XNULL,                                                      \
    {                                                           \
        {                                                       \
            #moduleId,                                          \
            XNULL,                                              \
            moduleId                                            \
        },                                                      \
        {                                                       \
            &init_module_##moduleId,                            \
            &notice_##moduleId,                                 \
            &close_module_##moduleId                            \
        },                                                      \
        {                                                       \
            &msg_entry_##moduleId,                              \
            timerCallBack                                       \
        },                                                      \
        memFreeType,                                            \
        XNULL                                                   \
   }                                                            \
};

/*********************************************************************
名称 : REG_MODULE_BEGIN
职责 : 注册一个通过使用框架生成的模块。表示该模块的类
       需要从IFactory派生
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
#define REG_MODULE_BIND_FCT(moduleId,clsName)                                         \
XS8 close_module_##moduleId(XVOID *, XVOID *)                                         \
{                                                                                     \
    CPP_ASSERT_RV((XNULL != __module_##moduleId.m_pFct), XFALSE);                     \
    delete  static_cast<clsName*>(__module_##moduleId.m_pFct);                        \
    __module_##moduleId.m_pFct = XNULL;                                               \
    return XSUCC;                                                                     \
}                                                                                     \
XS8 msg_entry_##moduleId(XVOID* pCpsMsg,XVOID*)                                       \
{                                                                                     \
    if (XNULL != __module_##moduleId.m_pFct)                                          \
    {                                                                                 \
        return clsName::sMsgEntry(pCpsMsg,__module_##moduleId.m_pFct);                \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        XOS_MsgMemFree(moduleId, (t_XOSCOMMHEAD *)pCpsMsg);                           \
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_ERR),"modId=%d,m_pFct is null",moduleId);    \
        return XSUCC;                                                                 \
    }                                                                                 \
}                                                                                     \
XS8 notice_##moduleId(XVOID *, XVOID *)                                               \
{                                                                                     \
    if (XNULL != __module_##moduleId.m_pFct)                                          \
    {                                                                                 \
        static_cast<clsName*>(__module_##moduleId.m_pFct)->OnNotice();                \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_INFO),"modId=%d,m_pFct is null", moduleId);  \
    }                                                                                 \
    return XSUCC;                                                                     \
}                                                                                     \
XS8 init_module_##moduleId(XVOID *, XVOID *)                                          \
{                                                                                     \
    CPP_ASSERT_RV((XNULL == __module_##moduleId.m_pFct), XFALSE);                     \
    CPP_ASSERT_RV((GetModuleMap().end() == GetModuleMap().find(moduleId)), XFALSE);   \
    clsName::GetDataFromOAM();                                                        \
    __module_##moduleId.m_pFct = new clsName(moduleId);                               \
    GetModuleMap()[moduleId] = &__module_##moduleId;                                  \
    static_cast<clsName*>(__module_##moduleId.m_pFct)->OnCreate();                    \
    return XSUCC;                                                                     \
}

/*********************************************************************
名称 : REG_MODULE_BIND_TASK
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
#define REG_MODULE_BIND_TASK_WITH_EXTPARA(moduleId,taskName,logLevel)               \
extern t_XOSLOGINLIST g_task_##taskName;                                            \
extern "C"  XS32 moduleId##_Entry(HANDLE hDir,XS32 argc, XS8** argv)                \
{                                                                                   \
    (XVOID)hDir;                                                                    \
    if(XNULL == g_task_##taskName.stack)                                            \
    {                                                                               \
        g_task_##taskName.TID = __module_##moduleId.m_fData.head.FID;               \
    }                                                                               \
    g_task_##taskName.stack = &__module_##moduleId.m_fData;                         \
    XS32 ret = XOS_MMStartFid(&g_task_##taskName,0,0);                              \
    (XVOID)SetTraceLevelByFid(moduleId, logLevel);                                  \
    return ret;                                                                     \
}

#define REG_MODULE_BIND_TASK(moduleId,taskName)                                     \
    REG_MODULE_BIND_TASK_WITH_EXTPARA(moduleId,taskName,PL_ERR)

#define REG_MODULE_END() //为了对称，使用一个空的宏

/*********************************************************************
名称 : REG_MODULE_BEGIN
职责 : 注册一个通过使用框架生成的模块。表示该模块的类
       需要从IFactoryMgr派生
协作 :

历史 :
       修改者   日期          描述
**********************************************************************/
//注册一个模块的第一步宏,
#define REG_MODULE_BEGIN(moduleId)  REG_FID_BEGIN(moduleId,&CTimerMgr::TimerCallback,eUserMode)
//绑定一个模块到一个工厂管理者



#endif


