/************************************************************************************
�ļ���  :cpp_module_loader.h

�ļ�����:��װģ���������Ҫ�ĺ�

����    :zzt

��������:2006/05/09

�޸ļ�¼:
          �����XOSƽ̨����,ƽ̨�ṩģ�������
          20060926--�޸Ĵ�������Ӧƽ̨�İ��ű�����ģ��
          1.ģ����������ϵ����ȫ�ɽű��м��ص�˳��������
************************************************************************************/
#ifndef __CPP_MODULE_LOADER_H_
#define __CPP_MODULE_LOADER_H_

#include "cpp_tcn_map.h"


#define TSK_STACK_SIZE   65535

struct CLink_XOSFIDLIST
{
    XVOID*           m_pFct;      //����
    t_XOSFIDLIST     m_fData;     //ģ������ṩ��һЩ�ӿں���
};

enum ETskPrio
{
    LOW_TSK   = TSK_PRIO_LOWER,
    NRML_TSK  = TSK_PRIO_NORMAL
};

typedef tcn::map<MDID,CLink_XOSFIDLIST*> CModuleMap;


//��ȡģ��map
CModuleMap& GetModuleMap();


//��ģ��ID����ģ����Ϣ�б�ģ��ID�ǰ���ȫ��Ψһ��
XVOID* FindFct(MDID mid);


//��ģ��ID����ģ������
XS8* GetModuleName(MDID mid, XS8* pOutBuf);


/*********************************************************************
���� : REG_MODULE_TASK_BEGIN
ְ�� : ע��һ���������ʼ��.��ʼ��Ĳ�������������taskId��
Э�� : ע�� ��Ϣ���ȼ� �Ƽ�TSK_PRIO_NORMAL
��ʷ :
       �޸���   ����          ����
**********************************************************************/
#define REG_TASK_BEGIN(taskName)                                \
extern t_XOSLOGINLIST g_task_##taskName;                        \
t_XOSLOGINLIST g_task_##taskName =                              \
{                                                               \
    XNULL,                                                      \
    #taskName,                                                  \
    0,  //��������ID��дһ������ֵ

#define REG_TASK_PRI_STACKSIZE(pri,stackSize)                   \
    pri,                                                        \
    stackSize,

#define REG_TASK_END()                                          \
};


/*********************************************************************
���� : REG_FID_BEGIN
ְ�� :
       ע��һ��ģ��.
       moduleId ģ��ID������һ������������֣�����һ������
       timerCallBack  --  ��ʱ����ʱ�ص�����
       memFreeType    --  ��Ϣ�ͷ�ģʽ����Ӧ���ͷŻ�����ƽ̨�ͷ�
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� : REG_MODULE_BEGIN
ְ�� : ע��һ��ͨ��ʹ�ÿ�����ɵ�ģ�顣��ʾ��ģ�����
       ��Ҫ��IFactory����
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� : REG_MODULE_BIND_TASK
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
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

#define REG_MODULE_END() //Ϊ�˶Գƣ�ʹ��һ���յĺ�

/*********************************************************************
���� : REG_MODULE_BEGIN
ְ�� : ע��һ��ͨ��ʹ�ÿ�����ɵ�ģ�顣��ʾ��ģ�����
       ��Ҫ��IFactoryMgr����
Э�� :

��ʷ :
       �޸���   ����          ����
**********************************************************************/
//ע��һ��ģ��ĵ�һ����,
#define REG_MODULE_BEGIN(moduleId)  REG_FID_BEGIN(moduleId,&CTimerMgr::TimerCallback,eUserMode)
//��һ��ģ�鵽һ������������



#endif


