/************************************************************************************
文件名称:cpp_bfsm.cpp

文件描述:

作者    :zzt

创建日期:2005/10/20

修改记录:

************************************************************************************/
#include "ha_interface.h"
#include "cpp_bfsm.h"


CMutex CMsgListAllc::m_sMutex;
CChunk CMsgListAllc::m_sChunk(sizeof(XVOID*)*3,  699050);  //系统中处于消息泵、SAVE或HOLD队列中的阿消息最多5万条

/*lint -e1551*/
/*********************************************************************
函数名称 : CreateTimer
功能描述 : 消息接收函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTimerMgr::DeleteTimer(CTimerHand& refTH)
{
    CTimerPara * tmp = ExtractTimerPara(refTH);

    //设置句柄为无效
    refTH.SetInvalide();
    if(tmp == XNULL)
    {
        return ;
    }

    //调用平台的接口删除定时器tmp->m_cpsTimerHander
    (XVOID)TimerDeleteCps(tmp->m_moduleId,tmp->m_cpsTimerHander);
    tmp->m_cpsTimerHander = XNULL;

    //释放内存
    m_CallbackPara.Deallocate(tmp);
}


/*********************************************************************
函数名称 : StopTimer
功能描述 : 停止定时器。对已经被STOP和DELETE的定时器调用该操作是合法的

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTimerMgr::StopTimer(const CTimerHand& refTH)const
{
    CTimerPara * tmp = ExtractTimerPara(refTH);

    if(XNULL == tmp)
    {
        return;
    }

    //调用平台的接口停止定时器
    EerrNo ret = TimerStopCps(tmp->m_moduleId,tmp->m_cpsTimerHander);
    CPP_ASSERT_RN(ERR_OK == ret);
}


/*********************************************************************
函数名称 : CreateTimer
功能描述 : 创建定时器。有资源占用的意思。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID  CTimerMgr::CreateTimer(CTimerHand& refTId , FSMID fsmId,TimerType timerType, XPOINT ulUserPara,XPOINT ulUserExtPara)
{
    CTimerPara * ptp = reinterpret_cast<CTimerPara *>(m_CallbackPara.Allocate());

    //框架定时器句柄使用超时回调函数的参数的起始地址
    refTId.SetTimerHand(ptp);
    if(ptp == XNULL)
    {
        return ;
    }

    //清零平台的定时器句柄
    ClearTimerHandCps(ptp->m_cpsTimerHander);

    //调用平台的创建接口初始化ptp->m_cpsTimerHander
    EerrNo rst = TimerCreateCps(m_IFct->GetId(),ptp->m_cpsTimerHander,timerType,ptp);
    if(rst != ERR_OK)
    {
        refTId.SetInvalide();
        m_CallbackPara.Deallocate(ptp);
        return ;
    }
    //初始化一些参数
    ptp->m_uiUserPara     = ulUserPara;         //用户数据，超时回调函数发送的超时消息体中回带上该数据
    ptp->m_uiUserExtPara  = ulUserExtPara;      //为了满足一个用户参数不够的情况增加的 2008.11.13
    ptp->m_moduleId       = m_IFct->GetId();
    ptp->m_tagertFsmId    = fsmId;
}


/*********************************************************************
函数名称 : TimerCallback
功能描述 : 定时器回调函数,该函数在定时器超时后会被调用

参数输入 :
参数输出 :
返回值   : 该回调函数的返回值的含义序和平台确认

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS8 CTimerMgr::TimerCallback(t_BACKPARA* cpsHt)
{
    //平台在调用超时回调之前已经将一次性的定时器句柄的状态改为非运行太，这里不用校验句柄的状态
    //先转换成平台的回调
    CPP_ASSERT_RV(XNULL != cpsHt, XERROR);

    CTimerPara * pPara = reinterpret_cast<CTimerPara*>(cpsHt->para1);
    CPP_ASSERT_RV(XNULL != pPara, XERROR);

    //因为是静态成员函数,所以不能调用工厂管理者的分配消息的接口
    CTimerOutMsg * pTimerMsg = (CTimerOutMsg *)MallocMsgBufOuterToFrame(sizeof(CTimerOutMsg),pPara->m_moduleId);
    if(pTimerMsg == XNULL)
    {
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_ERR),"Malloc CTimerOutMsg buf fail, ModID=%d, FsmId=%d, UserPara=%ul",
                  pPara->m_moduleId, pPara->m_tagertFsmId, pPara->m_uiUserPara);
        return (XS8)XERROR;
    }

    //设置发送者。注意，发送者的状态机ID的值比较特殊，是SS定义的状态机的句柄的值。
    //所以FSMID类型的长度必须与指针的类型的长度一致
    pTimerMsg->SetSenderAddr(CFsmAddr(pPara->m_moduleId,pPara->m_tagertFsmId));
    pTimerMsg->SetReciverAddr(CFsmAddr(pPara->m_moduleId,pPara->m_tagertFsmId));
    pTimerMsg->SetMsgId(SYS_TIMEOUT_MSG);

    //设置用户参数
    pTimerMsg->m_timerHand      = (XVOID*)pPara;
    pTimerMsg->m_ulUserPara     = pPara->m_uiUserPara;
    pTimerMsg->m_ulUserExtPara  = pPara->m_uiUserExtPara;

    //填写消息优先级,定时器超时事件是一个高优先级的事件
    pTimerMsg->SetMsgPrio(HIGHEST_SEND);
    if(SendMsgCps((t_XOSCOMMHEAD*)pTimerMsg) != ERR_OK)
    {
        FreeMsgBufCps(pTimerMsg,pPara->m_moduleId);//发送消息失败，释放内存
    }
    return XSUCC;
}


/*********************************************************************
函数名称 : SendMsg(t_XOSCOMMHEAD*  pMsg,PFAddPump pfn)
功能描述 : 发送消息的函数。
           若要投递到平台的消息队列中，则内存必须是操作系统托管的

参数输入 :
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CMsgBox::SendMsg(CMsg*& pMsg,ESendPrio iFlag)
{
    //不允许发送定时器超时消息
    CPP_ASSERT_RV(pMsg != XNULL, ERR_NO_DEFINE);

    pMsg->SetMsgPrio(iFlag);

    //若是本模块给本模块发送消息，不校验消息的内存类
    if(IsInnerModMsg(*pMsg))
    {
        IncCount(FMGR_INNER_SEND_INDEX);//内部
        if(iFlag == NRML_SEND )
        {
            m_pFM->AddToTail(pMsg);
        }
        else
        {
            m_pFM->AddToHead(pMsg);
        }
        return ERR_OK;
    }

    IncCount(FMGR_OUTER_SEND_INDEX);//向其他板或模块发送的消息的计数

    //调用平台的消息发送接口发送消息
    //消息发送失败,不释放消息,由用户释放
    EerrNo rst = SendMsgCps((t_XOSCOMMHEAD*)pMsg);
    if(rst != ERR_OK)
    {
        IncCount(FMGR_SEND_OUT_FAIL_INDEX);
    }
    return rst;
}


/*********************************************************************
函数名称 : SendMsg(t_XOSCOMMHEAD*  pMsg,PFAddPump pfn)
功能描述 : 发送消息的函数。
           若要投递到平台的消息队列中，则内存必须是操作系统托管的

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CMsgBox::TransferMsg(CMsg* pMsg)//紧急发送，就是投递到消息队列的头
{
    IncCount(FMGR_TRANSFER_INDEX);

    return SendMsg(pMsg,URG_SEND);
}


/*********************************************************************
函数名称 : CMsgBox::MallocMsgBuf
功能描述 : 发送消息时要注意区分消息中缓存的类型能。消息的缓存只有如下两种类型
           1.平台堆中的内存。
           2.MSGBOX中管理的内存。该内存只能在一个模块内存发送消息的情况下使用。
             消息发送的接口。MSGBOX中的消息被SAVE,HOLD,转发的情况

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CMsg* CMsgBox::MallocMsgBuf(XU32 uiFullLength)
{
    CPP_ASSERT_RV(uiFullLength >= sizeof(CMsg), XNULL);

    IncCount(FMGR_MALLOC_HEAP_BUF_INDEX);

    CMsg * pMsg = XNULL;
    pMsg =(CMsg*)MallocMsgBufCps(GetCPsLenFromFullLength(uiFullLength),m_pFM->GetId());//这里调用平台的消息分配函数
    if(pMsg != XNULL)
    {
        pMsg->InitSelf();
    }
    else
    {
        //分配失败后计数
        IncCount(FMGR_MALLOC_HEAP_FAIL_INDEX);
    }

    return pMsg;
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CMsgBox::FreeMsgBuf(CMsg *pMsg)
{
    CPP_ASSERT_RN(pMsg != XNULL);
    IncCount(FMGR_FREE_HEAP_BUF_INDEX);

    //若是平台托管的堆内存,调用平台的接口释放
    FreeMsgBufCps(pMsg,m_pFM->GetId());
}


/*********************************************************************
函数名称 : IFactory
功能描述 : 构造函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
IFactory::IFactory(MDID id, XU16 usMaxTimerNum, XU32 maxNum, bool needRandom)
    :m_IFsmPool(maxNum,needRandom), m_oprCounter(FMGR_INDEX_NUM)
{
    m_Id        = id;
    m_uiMaxFsm  = 0;
    for(XU32 i=0; i < FSM_NUM_IN_FACMGR; i++)
    {
        m_apFsm_list[i] = XNULL;
    }

    //添加定时器消息缓存的接口
    if(usMaxTimerNum != 0)
    {
        m_pTimer = new CTimerMgr(this,usMaxTimerNum);
    }
    else
    {
        m_pTimer = XNULL;
    }

    m_pMsgBox  = new CMsgBox(this);

    //定时器个数注册
    EerrNo ret = TimerNumRegCps(GetId(),usMaxTimerNum);
    if(ERR_OK != ret)
    {
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"TimerNumRegCps fail, modId=%d\r\n", m_Id);
    }
}


/*********************************************************************
函数名称 : ~IFactory
功能描述 : 虚析构函数, 无操作

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
IFactory::~IFactory()
{
    if (XNULL != m_pMsgBox)
    {
        delete    m_pMsgBox;
        m_pMsgBox = XNULL;
    }

    if (XNULL != m_pTimer)
    {
        delete    m_pTimer;
        m_pTimer  = XNULL;
    }
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFactory::OnCreate()
{
    return ERR_OK;
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFactory::OnNotice()
{
    //注册HA死锁回调(临时添加)
    (XVOID)XOS_HA_DeadLockReg(m_Id, m_Id, XNULL, XNULL);

    return ERR_OK;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool  IFactory::CanPrcMsg(CMsg *)
{
    return true;
}

/*********************************************************************
函数名称 : sMsgEntry
功能描述 : 任务入口函数.每当收到系统任务从与IFactoryMgr
           关联的消息队列中取得消息时,旧调用该接口.

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS8 IFactory::sMsgEntry(XVOID* pCpsMsg,XVOID *pObj)
{
    CPP_ASSERT_RV((XNULL != pCpsMsg) && (XNULL != pObj), XERROR);

    IFactory& IFct = *(static_cast<IFactory*>(pObj));

    IFct.SpecialProc(pCpsMsg);

    //强制转换成CMsg类型
    CMsg * pMsg = reinterpret_cast<CMsg *>(pCpsMsg);
    if (XNULL != pMsg)
    {
        IFct.AddToTail(pMsg);
    }

    EPrcResult Prc = BAD_MSG;//默认值是坏的消息
    while(IFct.m_CMsgPump.size() != 0)
    {
        pMsg = IFct.m_CMsgPump.front();
        IFct.m_CMsgPump.pop_front();
        if(XNULL == pMsg)
        {
            continue;
        }

        //记录工厂管理者收到的消息条数,
        IFct.IncCount(FMGR_REV_INDEX);

        //增加模块内部消息发送跟踪
        if (pMsg->GetSndMId() == pMsg->GetRcvMId())
        {
            (XVOID)XOS_TaTrace(pMsg->GetRcvMId(),e_TA_RECV,(t_XOSCOMMHEAD *)pMsg);
        }

        //HA模块死锁监控消息(临时添加)
        if(FID_HA == pMsg->GetSndBId())
        {
            if (HA_WATCH_MSG_REQ == pMsg->GetMsgId())
            {
                (XVOID)HA_XOSHelloProcess(IFct.m_Id, pMsg->m_cpsHead.message, pMsg->m_cpsHead.length);
                IFct.FreeMsgBuf(pMsg);
                continue;
            }
        }

        if(!(IFct.CanPrcMsg(pMsg)))
        {
            IFct.FreeMsgBuf(pMsg);
            continue;
        }

        Prc = IFct.ProcessMsg(pMsg);

        if(SAVE_MSG > Prc) //不要释放save,transfer,hold消息队列中的消息,
        {
            IFct.FreeMsgBuf(pMsg);   //定时器消息也不能释放;
        }
        else if(TRANSFER_MSG == Prc)
        {
            IFct.TransferMsg(pMsg);  //转发的,转发失败咋办?
        }
    }

    return XSUCC;
}

/*********************************************************************
函数名称 : ProcessMsg
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EPrcResult IFactory::ProcessMsg(CMsg* pMsg)
{
    CPP_ASSERT_RV(XNULL != pMsg, BAD_MSG);

    IFsm         *pTmpFsm = XNULL;
    EPrcResult    Prc = BAD_MSG;
    Tstate        OldState;
    EerrNo        result;
    XU32          SaveNum;

    //清除状态机数目
    m_uiMaxFsm = 0;

    //目的状态机ID
    FSMID TmpId = pMsg->GetRcvFsmId();

    if(INVALID_OBJ_ID == TmpId)
    {
        Prc = FsmIdIsZeroPrc(pMsg, pTmpFsm);
        switch(Prc)
        {
        case FAC_GET:
            CPP_ASSERT_RV(XNULL != pTmpFsm, BAD_MSG);
            CPP_ASSERT_RV(0 == m_uiMaxFsm, BAD_MSG);
            pMsg->SetRcvFsmId(pTmpFsm->GetId());
            m_apFsm_list[m_uiMaxFsm++] = pTmpFsm;
            break;
        case FAC_GET_LIST:
            CPP_ASSERT_RV(XNULL == pTmpFsm, BAD_MSG); //若返回值为FAC_GET_LIST, pTmpFsm必须是 XNULL
            CPP_ASSERT_RV((0 != m_uiMaxFsm) && (m_uiMaxFsm <= FSM_NUM_IN_FACMGR), BAD_MSG); //并且 m_uiMaxFsm 必须不为0,就是找到了多个状态机
            break;
        case FAC_NEW:
            CPP_ASSERT_RV(XNULL != pTmpFsm, PRC_ERR);
            CPP_ASSERT_RV(0 == m_uiMaxFsm, PRC_ERR);
            TmpId =  pTmpFsm->GetId();
            pMsg->SetRcvFsmId(TmpId);
            m_apFsm_list[m_uiMaxFsm++] = pTmpFsm;
            if(ERR_OK != pTmpFsm->Create())
            {
                m_oprCounter.Inc(FMGR_ERR_INDEX);
                result = m_IFsmPool.DeActive(TmpId);
                CPP_ASSERT_RV(ERR_OK == result, PRC_ERR);
                return PRC_ERR;
            }
            break;
        case UNKNOWN_MSG:
            m_oprCounter.Inc(FMGR_UNKNOWN_INDEX);
            return Prc;
        case DISCARD:
            m_oprCounter.Inc(FMGR_DISCARD_INDEX);
            return Prc;
        case PROCESSED:
            m_oprCounter.Inc(FMGR_SELF_PRC_INDEX);
            return Prc;
        case TRANSFER_MSG:
            return Prc;
        case BAD_MSG:
            m_oprCounter.Inc(FMGR_BAD_INDEX);
            return Prc;
        case PRC_ERR:
            m_oprCounter.Inc(FMGR_ERR_INDEX);
            return Prc;
        case USER_PROC:
            m_oprCounter.Inc(FMGR_USER_PROC_INDEX);
            return Prc;
        case KILL_FSM:
        case SAVE_MSG:
        case HOLD_MSG:
        default:
            PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"FsmIdIsZeroPrc, prc err\r\n");
            return PRC_ERR;
        }
    }
    else if(OBJ_ACTIVE == m_IFsmPool.GetStateById(TmpId))
    {
        //若目的状态机已经被激活,可以处理消息，则将之加到处理该调消息的状态机列表中
        m_apFsm_list[m_uiMaxFsm++] = m_IFsmPool.GetById(TmpId);
    }
    else
    {
        m_oprCounter.Inc(FMGR_BAD_INDEX);
        return RouteMsgErr(pMsg);
    }

    for(XU32 i = 0; i < m_uiMaxFsm; ++i)
    {
        pTmpFsm = m_apFsm_list[i];
        pTmpFsm->PrePrcMsg(pMsg);
        OldState = pTmpFsm->GetState();

        Prc = pTmpFsm->ProcessMsg(pMsg);
        //存储的消息不需要后处理
        if(SAVE_MSG != Prc)
        {
            pTmpFsm->PostPrcMsg(pMsg);
        }

        switch(Prc)
        {
        case UNKNOWN_MSG:
            m_oprCounter.Inc(FMGR_UNKNOWN_INDEX);
            break;
        case PROCESSED:
            m_oprCounter.Inc(FMGR_FSM_PROC_INDEX);
            break;
        case DISCARD:
            m_oprCounter.Inc(FMGR_DISCARD_INDEX);
            break;
        case KILL_FSM:
            m_oprCounter.Inc(FMGR_FSM_PROC_INDEX);
            KillFsm(pTmpFsm);

            continue;
        case SAVE_MSG:
            if(1 == m_uiMaxFsm)
            {
                m_oprCounter.Inc(FMGR_SAVE_INDEX);
                pTmpFsm->PutSaveMsg(pMsg);
                continue;
            }
            else
            {
                m_oprCounter.Inc(FMGR_ERR_INDEX);
                PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Save Msg, 1 != m_uiMaxFsm\r\n");
                return PRC_ERR;
            }
        case HOLD_MSG:
            if(1 == m_uiMaxFsm)
            {
                m_oprCounter.Inc(FMGR_HOLD_INDEX);
                pTmpFsm->PutHoldMsg(pMsg);
            }
            else
            {
                m_oprCounter.Inc(FMGR_ERR_INDEX);
                PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Hold Msg, 1 != m_uiMaxFsm\r\n");
                return PRC_ERR; //不允许HOLD 处理结果为FAC_GET_LIST 的消息
            }
            break;
        case TRANSFER_MSG:
            if(1 == m_uiMaxFsm)
            {
            }
            else
            {
                PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Transfer Msg, 1 != m_uiMaxFsm\r\n");
                return PRC_ERR;  //不允许Transfer 处理结果为FAC_GET_LIST 的消息
            }
            break;
        case BAD_MSG:
            break;
        case PRC_ERR:
            break;
        case USER_PROC:
            break;
        case FAC_NEW:
        case FAC_GET:
        case FAC_GET_LIST:
        default:
            PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"ProcessMsg, prc err\r\n");
            return PRC_ERR;
        }
        //注意SALVE队列中消息的释放由,
        SaveNum = pTmpFsm->GetSaveNum();

        //最多扫描SAVENUM次
        while((pTmpFsm->GetState() != OldState)&&(SaveNum > 0))
        {
            OldState = pTmpFsm->GetState();
            SaveMsgPrc(pTmpFsm);
            --SaveNum;
        }
    }

    return Prc;
}

/*********************************************************************
函数名称 :  SaveMsgPrc

实现描述 :  状态机收到了一条在当前状态下不能处理的消息,则
            状态机就将该条消息存储在自己的SAVE消息队列中,在以后每当
            状态机状态改变时,就遍历处理SAVE消息队列中存储的消息,看在
            当前状态下是否能处理.状态机在处理SAVE消息队列中的消息时
            也可能改变状态状态机的状态,所以处理SAVE消息队列中的消息
            的停止条件是队列中消息数为0或者状态机在连续两次遍历SAVE
            消息队列时状态未改变

            该函数负责释放SAVE的队列中的消息。HOLD类型得消息由用户来
            释放。一般，消息的释放由框架完成，HOLD队列中的消息才由用
            户来释放。
            用户只需要将需要暂存的消息放入SAVE消息队列中。至于SAVE消息
            处理的时机以及什么时候释放由框架完成。  HOLD消息处理的时机
            以及什么时候释放由用户自己负责

参数输入 :  IFsm *   该参数指向需要处理SAVE消息列表的状态机
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::SaveMsgPrc(IFsm *pFsm)
{
    CPP_ASSERT_RN(pFsm != XNULL);

    if(ERR_OK != pFsm->GetErr())
    {
        return;
    }

    //处理存储的消息
    XU32 SaveNum = pFsm->GetSaveNum();
    for(XU32 i=0; i < SaveNum; ++i)
    {
        CMsg *pTmpMsg = pFsm->GetSaveMsg();
        CPP_ASSERT_RN(XNULL != pTmpMsg);

        EPrcResult Prc = pFsm->ProcessMsg(pTmpMsg);
        if(SAVE_MSG != Prc)
        {
            pFsm->PostPrcMsg(pTmpMsg);
        }

        switch(Prc)
        {
            //该消息依然不能被处理
        case SAVE_MSG:
            pFsm->PutSaveMsg(pTmpMsg);        //重新存储
            break;
            //将该消息放大HOLD列表中
        case HOLD_MSG:
            pFsm->PutHoldMsg(pTmpMsg);
            break;
            //杀死状态机
        case KILL_FSM:
            FreeMsgBuf(pTmpMsg);
            KillFsm(pFsm);
            i = SaveNum; //,该状态机被KILL，则必须退出该循环
            break;
        case TRANSFER_MSG:
            TransferMsg(pTmpMsg);
            break;
        case USER_PROC:
            break;
        case BAD_MSG:
        case PRC_ERR:
        case UNKNOWN_MSG:
        case PROCESSED:
        case DISCARD:
        case FAC_NEW:
        case FAC_GET:
        case FAC_GET_LIST:
        default:
            FreeMsgBuf(pTmpMsg);
            break;
        }
    }
    pFsm->SetErr(ERR_OK);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EPrcResult IFactory::RouteMsgErr(CMsg *)
{
    return BAD_MSG;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::TransferMsg(CMsg  *pMsg)
{
    CPP_ASSERT_RN(m_pMsgBox != XNULL);
    if( m_pMsgBox->TransferMsg(pMsg)!= ERR_OK)
    {
        IncCount(FMGR_TRANSFER_FAIL_INDEX);
        FreeMsgBuf(pMsg);
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PreFreeMsgBuf(CMsg* pBuf)
{
    // do nothing
    // some modules, like protocol UA, must override this virtual function to
    // free memories hold by itself.
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::KillFsm(IFsm *pFsm)
{
    CPP_ASSERT_RN(pFsm != XNULL);

    //去激活状态机, 释放消息
    (XVOID)pFsm->Destroy();

    EerrNo ret = m_IFsmPool.DeActive(pFsm->GetId());
    CPP_ASSERT_RN(ERR_OK == ret);
}

/*********************************************************************
函数名称 : UnfrozenFsm
功能描述 : 执行该函数的前置条件是--SAVE队列不能为空

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::UnfrozenFsm(XU32 fsmId)
{
    Tstate   OldState;
    XU32     SaveNum;

    if(OBJ_ACTIVE == m_IFsmPool.GetStateById(fsmId))
    {
        IFsm *pTmpFsm =m_IFsmPool.GetById(fsmId);
        SaveNum = pTmpFsm->GetSaveNum();
        do
        {
            OldState = pTmpFsm->GetState();
            SaveMsgPrc(pTmpFsm);
            --SaveNum;
        }
        while((pTmpFsm->GetState() != OldState) && (SaveNum > 0));
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintCounter(CLI_ENV* pCliEnv)const
{
    //打印模块缓存信息
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_REV_INDEX               :%u\r\n", m_oprCounter.Get(FMGR_REV_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_SELF_PRC_INDEX          :%u\r\n", m_oprCounter.Get(FMGR_SELF_PRC_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_INNER_SEND_INDEX        :%u\r\n", m_oprCounter.Get(FMGR_INNER_SEND_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_OUTER_SEND_INDEX        :%u\r\n", m_oprCounter.Get(FMGR_OUTER_SEND_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_SEND_OUT_FAIL_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_SEND_OUT_FAIL_INDEX));

    //模块内部转发的消息的计数
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_TRANSFER_INDEX          :%d\r\n", m_oprCounter.Get(FMGR_TRANSFER_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_TRANSFER_FAIL_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_TRANSFER_FAIL_INDEX));

    //系统堆BUF的分配计数信息
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_MALLOC_HEAP_BUF_INDEX   :%u\r\n", m_oprCounter.Get(FMGR_MALLOC_HEAP_BUF_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_MALLOC_HEAP_FAIL_INDEX  :%u\r\n", m_oprCounter.Get(FMGR_MALLOC_HEAP_FAIL_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_FREE_HEAP_BUF_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_FREE_HEAP_BUF_INDEX));

    //消息处理计数
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_UNKNOWN_INDEX           :%u\r\n", m_oprCounter.Get(FMGR_UNKNOWN_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_FSM_PROC_INDEX          :%u\r\n", m_oprCounter.Get(FMGR_FSM_PROC_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_SAVE_INDEX              :%u\r\n", m_oprCounter.Get(FMGR_SAVE_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_HOLD_INDEX              :%u\r\n", m_oprCounter.Get(FMGR_HOLD_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_DISCARD_INDEX           :%u\r\n", m_oprCounter.Get(FMGR_DISCARD_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_BAD_INDEX               :%u\r\n", m_oprCounter.Get(FMGR_BAD_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_ERR_INDEX               :%u\r\n", m_oprCounter.Get(FMGR_ERR_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_USER_PROC_INDEX         :%u\r\n", m_oprCounter.Get(FMGR_USER_PROC_INDEX));

    return;
}

/*********************************************************************
函数名称 : PrintFsm
功能描述 : 打印状态机信息

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintFsm(FSMID fsmId, CLI_ENV* pCliEnv)
{
    if (BLANK_ULONG == fsmId)
    {
        m_IFsmPool.Print(pCliEnv);
    }
    else
    {
        m_IFsmPool.PrintItem(fsmId, pCliEnv);
    }
}

/*********************************************************************
函数名称 : PrintFsmRang
功能描述 : 打印某个范围状态机信息

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintFsmRang(FSMID fsmStart, XU32 uiCnt, CLI_ENV* pCliEnv)
{
    m_IFsmPool.PrintItems(fsmStart, uiCnt, pCliEnv);
}

/*********************************************************************
函数名称 : PrintFsmLastFree
功能描述 : 打印空闲状态状态机

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintFsmLastFree(XU32 uiStartId, XU32 uiCnt, CLI_ENV* pCliEnv)
{
    if (BLANK_ULONG == uiStartId)
    {
        m_IFsmPool.PrintLastFree(uiCnt, pCliEnv);
    }
    else
    {
        m_IFsmPool.PrintLastFree(uiStartId, uiCnt, pCliEnv);
    }
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
IFsm::~IFsm()
{
    m_pIFct = XNULL;
}


/*********************************************************************
函数名称 :
功能描述 : 在设置状态机状态之前被调用的函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PreSetState(Tstate)
{
}


/*********************************************************************
函数名称 : PreSendMsgProc
功能描述 : 消息发送之前被调用的函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PreSendMsgProc(CMsg* ,const CFsmAddr& ,ESendPrio )
{
}


/*********************************************************************
函数名称 :  PrePrCMsg
功能描述 :  在调用ProcessMsg(CMsg* & pMsg)之前调用的函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PrePrcMsg(CMsg *& )
{
}


/*********************************************************************
函数名称 :  PostPrCMsg
功能描述 :   在调用ProcessMsg(CMsg* & pMsg)之后调用的函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PostPrcMsg(CMsg *&)
{
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFsm::Create()
{
    return ERR_OK;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFsm::Destroy()
{
    FreeAllSaveMsg();
    FreeAllHoldMsg();
    return ERR_OK;
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::Print(CLI_ENV* pCliEnv)
{
    //XOS_CliExtWriteStrLine(pCliEnv, "the fsm of this module did not reload method Print");
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FsmId=%x, State=%d, errCode=%d, saveNum=%d, holdNum=%d\r\n",
                            GetId(),
                            GetState(),
                            GetErr(),
                            GetSaveNum(),
                            GetHoldNum());

    return;
}



