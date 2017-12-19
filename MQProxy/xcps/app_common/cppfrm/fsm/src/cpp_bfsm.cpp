/************************************************************************************
�ļ�����:cpp_bfsm.cpp

�ļ�����:

����    :zzt

��������:2005/10/20

�޸ļ�¼:

************************************************************************************/
#include "ha_interface.h"
#include "cpp_bfsm.h"


CMutex CMsgListAllc::m_sMutex;
CChunk CMsgListAllc::m_sChunk(sizeof(XVOID*)*3,  699050);  //ϵͳ�д�����Ϣ�á�SAVE��HOLD�����еİ���Ϣ���5����

/*lint -e1551*/
/*********************************************************************
�������� : CreateTimer
�������� : ��Ϣ���պ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTimerMgr::DeleteTimer(CTimerHand& refTH)
{
    CTimerPara * tmp = ExtractTimerPara(refTH);

    //���þ��Ϊ��Ч
    refTH.SetInvalide();
    if(tmp == XNULL)
    {
        return ;
    }

    //����ƽ̨�Ľӿ�ɾ����ʱ��tmp->m_cpsTimerHander
    (XVOID)TimerDeleteCps(tmp->m_moduleId,tmp->m_cpsTimerHander);
    tmp->m_cpsTimerHander = XNULL;

    //�ͷ��ڴ�
    m_CallbackPara.Deallocate(tmp);
}


/*********************************************************************
�������� : StopTimer
�������� : ֹͣ��ʱ�������Ѿ���STOP��DELETE�Ķ�ʱ�����øò����ǺϷ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTimerMgr::StopTimer(const CTimerHand& refTH)const
{
    CTimerPara * tmp = ExtractTimerPara(refTH);

    if(XNULL == tmp)
    {
        return;
    }

    //����ƽ̨�Ľӿ�ֹͣ��ʱ��
    EerrNo ret = TimerStopCps(tmp->m_moduleId,tmp->m_cpsTimerHander);
    CPP_ASSERT_RN(ERR_OK == ret);
}


/*********************************************************************
�������� : CreateTimer
�������� : ������ʱ��������Դռ�õ���˼��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID  CTimerMgr::CreateTimer(CTimerHand& refTId , FSMID fsmId,TimerType timerType, XPOINT ulUserPara,XPOINT ulUserExtPara)
{
    CTimerPara * ptp = reinterpret_cast<CTimerPara *>(m_CallbackPara.Allocate());

    //��ܶ�ʱ�����ʹ�ó�ʱ�ص������Ĳ�������ʼ��ַ
    refTId.SetTimerHand(ptp);
    if(ptp == XNULL)
    {
        return ;
    }

    //����ƽ̨�Ķ�ʱ�����
    ClearTimerHandCps(ptp->m_cpsTimerHander);

    //����ƽ̨�Ĵ����ӿڳ�ʼ��ptp->m_cpsTimerHander
    EerrNo rst = TimerCreateCps(m_IFct->GetId(),ptp->m_cpsTimerHander,timerType,ptp);
    if(rst != ERR_OK)
    {
        refTId.SetInvalide();
        m_CallbackPara.Deallocate(ptp);
        return ;
    }
    //��ʼ��һЩ����
    ptp->m_uiUserPara     = ulUserPara;         //�û����ݣ���ʱ�ص��������͵ĳ�ʱ��Ϣ���лش��ϸ�����
    ptp->m_uiUserExtPara  = ulUserExtPara;      //Ϊ������һ���û�����������������ӵ� 2008.11.13
    ptp->m_moduleId       = m_IFct->GetId();
    ptp->m_tagertFsmId    = fsmId;
}


/*********************************************************************
�������� : TimerCallback
�������� : ��ʱ���ص�����,�ú����ڶ�ʱ����ʱ��ᱻ����

�������� :
������� :
����ֵ   : �ûص������ķ���ֵ�ĺ������ƽ̨ȷ��

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS8 CTimerMgr::TimerCallback(t_BACKPARA* cpsHt)
{
    //ƽ̨�ڵ��ó�ʱ�ص�֮ǰ�Ѿ���һ���ԵĶ�ʱ�������״̬��Ϊ������̫�����ﲻ��У������״̬
    //��ת����ƽ̨�Ļص�
    CPP_ASSERT_RV(XNULL != cpsHt, XERROR);

    CTimerPara * pPara = reinterpret_cast<CTimerPara*>(cpsHt->para1);
    CPP_ASSERT_RV(XNULL != pPara, XERROR);

    //��Ϊ�Ǿ�̬��Ա����,���Բ��ܵ��ù��������ߵķ�����Ϣ�Ľӿ�
    CTimerOutMsg * pTimerMsg = (CTimerOutMsg *)MallocMsgBufOuterToFrame(sizeof(CTimerOutMsg),pPara->m_moduleId);
    if(pTimerMsg == XNULL)
    {
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_ERR),"Malloc CTimerOutMsg buf fail, ModID=%d, FsmId=%d, UserPara=%ul",
                  pPara->m_moduleId, pPara->m_tagertFsmId, pPara->m_uiUserPara);
        return (XS8)XERROR;
    }

    //���÷����ߡ�ע�⣬�����ߵ�״̬��ID��ֵ�Ƚ����⣬��SS�����״̬���ľ����ֵ��
    //����FSMID���͵ĳ��ȱ�����ָ������͵ĳ���һ��
    pTimerMsg->SetSenderAddr(CFsmAddr(pPara->m_moduleId,pPara->m_tagertFsmId));
    pTimerMsg->SetReciverAddr(CFsmAddr(pPara->m_moduleId,pPara->m_tagertFsmId));
    pTimerMsg->SetMsgId(SYS_TIMEOUT_MSG);

    //�����û�����
    pTimerMsg->m_timerHand      = (XVOID*)pPara;
    pTimerMsg->m_ulUserPara     = pPara->m_uiUserPara;
    pTimerMsg->m_ulUserExtPara  = pPara->m_uiUserExtPara;

    //��д��Ϣ���ȼ�,��ʱ����ʱ�¼���һ�������ȼ����¼�
    pTimerMsg->SetMsgPrio(HIGHEST_SEND);
    if(SendMsgCps((t_XOSCOMMHEAD*)pTimerMsg) != ERR_OK)
    {
        FreeMsgBufCps(pTimerMsg,pPara->m_moduleId);//������Ϣʧ�ܣ��ͷ��ڴ�
    }
    return XSUCC;
}


/*********************************************************************
�������� : SendMsg(t_XOSCOMMHEAD*  pMsg,PFAddPump pfn)
�������� : ������Ϣ�ĺ�����
           ��ҪͶ�ݵ�ƽ̨����Ϣ�����У����ڴ�����ǲ���ϵͳ�йܵ�

�������� :
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CMsgBox::SendMsg(CMsg*& pMsg,ESendPrio iFlag)
{
    //�������Ͷ�ʱ����ʱ��Ϣ
    CPP_ASSERT_RV(pMsg != XNULL, ERR_NO_DEFINE);

    pMsg->SetMsgPrio(iFlag);

    //���Ǳ�ģ�����ģ�鷢����Ϣ����У����Ϣ���ڴ���
    if(IsInnerModMsg(*pMsg))
    {
        IncCount(FMGR_INNER_SEND_INDEX);//�ڲ�
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

    IncCount(FMGR_OUTER_SEND_INDEX);//���������ģ�鷢�͵���Ϣ�ļ���

    //����ƽ̨����Ϣ���ͽӿڷ�����Ϣ
    //��Ϣ����ʧ��,���ͷ���Ϣ,���û��ͷ�
    EerrNo rst = SendMsgCps((t_XOSCOMMHEAD*)pMsg);
    if(rst != ERR_OK)
    {
        IncCount(FMGR_SEND_OUT_FAIL_INDEX);
    }
    return rst;
}


/*********************************************************************
�������� : SendMsg(t_XOSCOMMHEAD*  pMsg,PFAddPump pfn)
�������� : ������Ϣ�ĺ�����
           ��ҪͶ�ݵ�ƽ̨����Ϣ�����У����ڴ�����ǲ���ϵͳ�йܵ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CMsgBox::TransferMsg(CMsg* pMsg)//�������ͣ�����Ͷ�ݵ���Ϣ���е�ͷ
{
    IncCount(FMGR_TRANSFER_INDEX);

    return SendMsg(pMsg,URG_SEND);
}


/*********************************************************************
�������� : CMsgBox::MallocMsgBuf
�������� : ������ϢʱҪע��������Ϣ�л���������ܡ���Ϣ�Ļ���ֻ��������������
           1.ƽ̨���е��ڴ档
           2.MSGBOX�й�����ڴ档���ڴ�ֻ����һ��ģ���ڴ淢����Ϣ�������ʹ�á�
             ��Ϣ���͵Ľӿڡ�MSGBOX�е���Ϣ��SAVE,HOLD,ת�������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CMsg* CMsgBox::MallocMsgBuf(XU32 uiFullLength)
{
    CPP_ASSERT_RV(uiFullLength >= sizeof(CMsg), XNULL);

    IncCount(FMGR_MALLOC_HEAP_BUF_INDEX);

    CMsg * pMsg = XNULL;
    pMsg =(CMsg*)MallocMsgBufCps(GetCPsLenFromFullLength(uiFullLength),m_pFM->GetId());//�������ƽ̨����Ϣ���亯��
    if(pMsg != XNULL)
    {
        pMsg->InitSelf();
    }
    else
    {
        //����ʧ�ܺ����
        IncCount(FMGR_MALLOC_HEAP_FAIL_INDEX);
    }

    return pMsg;
}


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CMsgBox::FreeMsgBuf(CMsg *pMsg)
{
    CPP_ASSERT_RN(pMsg != XNULL);
    IncCount(FMGR_FREE_HEAP_BUF_INDEX);

    //����ƽ̨�йܵĶ��ڴ�,����ƽ̨�Ľӿ��ͷ�
    FreeMsgBufCps(pMsg,m_pFM->GetId());
}


/*********************************************************************
�������� : IFactory
�������� : ���캯��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

    //��Ӷ�ʱ����Ϣ����Ľӿ�
    if(usMaxTimerNum != 0)
    {
        m_pTimer = new CTimerMgr(this,usMaxTimerNum);
    }
    else
    {
        m_pTimer = XNULL;
    }

    m_pMsgBox  = new CMsgBox(this);

    //��ʱ������ע��
    EerrNo ret = TimerNumRegCps(GetId(),usMaxTimerNum);
    if(ERR_OK != ret)
    {
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"TimerNumRegCps fail, modId=%d\r\n", m_Id);
    }
}


/*********************************************************************
�������� : ~IFactory
�������� : ����������, �޲���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFactory::OnCreate()
{
    return ERR_OK;
}


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFactory::OnNotice()
{
    //ע��HA�����ص�(��ʱ���)
    (XVOID)XOS_HA_DeadLockReg(m_Id, m_Id, XNULL, XNULL);

    return ERR_OK;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool  IFactory::CanPrcMsg(CMsg *)
{
    return true;
}

/*********************************************************************
�������� : sMsgEntry
�������� : ������ں���.ÿ���յ�ϵͳ�������IFactoryMgr
           ��������Ϣ������ȡ����Ϣʱ,�ɵ��øýӿ�.

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS8 IFactory::sMsgEntry(XVOID* pCpsMsg,XVOID *pObj)
{
    CPP_ASSERT_RV((XNULL != pCpsMsg) && (XNULL != pObj), XERROR);

    IFactory& IFct = *(static_cast<IFactory*>(pObj));

    IFct.SpecialProc(pCpsMsg);

    //ǿ��ת����CMsg����
    CMsg * pMsg = reinterpret_cast<CMsg *>(pCpsMsg);
    if (XNULL != pMsg)
    {
        IFct.AddToTail(pMsg);
    }

    EPrcResult Prc = BAD_MSG;//Ĭ��ֵ�ǻ�����Ϣ
    while(IFct.m_CMsgPump.size() != 0)
    {
        pMsg = IFct.m_CMsgPump.front();
        IFct.m_CMsgPump.pop_front();
        if(XNULL == pMsg)
        {
            continue;
        }

        //��¼�����������յ�����Ϣ����,
        IFct.IncCount(FMGR_REV_INDEX);

        //����ģ���ڲ���Ϣ���͸���
        if (pMsg->GetSndMId() == pMsg->GetRcvMId())
        {
            (XVOID)XOS_TaTrace(pMsg->GetRcvMId(),e_TA_RECV,(t_XOSCOMMHEAD *)pMsg);
        }

        //HAģ�����������Ϣ(��ʱ���)
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

        if(SAVE_MSG > Prc) //��Ҫ�ͷ�save,transfer,hold��Ϣ�����е���Ϣ,
        {
            IFct.FreeMsgBuf(pMsg);   //��ʱ����ϢҲ�����ͷ�;
        }
        else if(TRANSFER_MSG == Prc)
        {
            IFct.TransferMsg(pMsg);  //ת����,ת��ʧ��զ��?
        }
    }

    return XSUCC;
}

/*********************************************************************
�������� : ProcessMsg
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EPrcResult IFactory::ProcessMsg(CMsg* pMsg)
{
    CPP_ASSERT_RV(XNULL != pMsg, BAD_MSG);

    IFsm         *pTmpFsm = XNULL;
    EPrcResult    Prc = BAD_MSG;
    Tstate        OldState;
    EerrNo        result;
    XU32          SaveNum;

    //���״̬����Ŀ
    m_uiMaxFsm = 0;

    //Ŀ��״̬��ID
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
            CPP_ASSERT_RV(XNULL == pTmpFsm, BAD_MSG); //������ֵΪFAC_GET_LIST, pTmpFsm������ XNULL
            CPP_ASSERT_RV((0 != m_uiMaxFsm) && (m_uiMaxFsm <= FSM_NUM_IN_FACMGR), BAD_MSG); //���� m_uiMaxFsm ���벻Ϊ0,�����ҵ��˶��״̬��
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
        //��Ŀ��״̬���Ѿ�������,���Դ�����Ϣ����֮�ӵ�����õ���Ϣ��״̬���б���
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
        //�洢����Ϣ����Ҫ����
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
                return PRC_ERR; //������HOLD ������ΪFAC_GET_LIST ����Ϣ
            }
            break;
        case TRANSFER_MSG:
            if(1 == m_uiMaxFsm)
            {
            }
            else
            {
                PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Transfer Msg, 1 != m_uiMaxFsm\r\n");
                return PRC_ERR;  //������Transfer ������ΪFAC_GET_LIST ����Ϣ
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
        //ע��SALVE��������Ϣ���ͷ���,
        SaveNum = pTmpFsm->GetSaveNum();

        //���ɨ��SAVENUM��
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
�������� :  SaveMsgPrc

ʵ������ :  ״̬���յ���һ���ڵ�ǰ״̬�²��ܴ������Ϣ,��
            ״̬���ͽ�������Ϣ�洢���Լ���SAVE��Ϣ������,���Ժ�ÿ��
            ״̬��״̬�ı�ʱ,�ͱ�������SAVE��Ϣ�����д洢����Ϣ,����
            ��ǰ״̬���Ƿ��ܴ���.״̬���ڴ���SAVE��Ϣ�����е���Ϣʱ
            Ҳ���ܸı�״̬״̬����״̬,���Դ���SAVE��Ϣ�����е���Ϣ
            ��ֹͣ�����Ƕ�������Ϣ��Ϊ0����״̬�����������α���SAVE
            ��Ϣ����ʱ״̬δ�ı�

            �ú��������ͷ�SAVE�Ķ����е���Ϣ��HOLD���͵���Ϣ���û���
            �ͷš�һ�㣬��Ϣ���ͷ��ɿ����ɣ�HOLD�����е���Ϣ������
            �����ͷš�
            �û�ֻ��Ҫ����Ҫ�ݴ����Ϣ����SAVE��Ϣ�����С�����SAVE��Ϣ
            �����ʱ���Լ�ʲôʱ���ͷ��ɿ����ɡ�  HOLD��Ϣ�����ʱ��
            �Լ�ʲôʱ���ͷ����û��Լ�����

�������� :  IFsm *   �ò���ָ����Ҫ����SAVE��Ϣ�б��״̬��
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::SaveMsgPrc(IFsm *pFsm)
{
    CPP_ASSERT_RN(pFsm != XNULL);

    if(ERR_OK != pFsm->GetErr())
    {
        return;
    }

    //����洢����Ϣ
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
            //����Ϣ��Ȼ���ܱ�����
        case SAVE_MSG:
            pFsm->PutSaveMsg(pTmpMsg);        //���´洢
            break;
            //������Ϣ�Ŵ�HOLD�б���
        case HOLD_MSG:
            pFsm->PutHoldMsg(pTmpMsg);
            break;
            //ɱ��״̬��
        case KILL_FSM:
            FreeMsgBuf(pTmpMsg);
            KillFsm(pFsm);
            i = SaveNum; //,��״̬����KILL��������˳���ѭ��
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EPrcResult IFactory::RouteMsgErr(CMsg *)
{
    return BAD_MSG;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PreFreeMsgBuf(CMsg* pBuf)
{
    // do nothing
    // some modules, like protocol UA, must override this virtual function to
    // free memories hold by itself.
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::KillFsm(IFsm *pFsm)
{
    CPP_ASSERT_RN(pFsm != XNULL);

    //ȥ����״̬��, �ͷ���Ϣ
    (XVOID)pFsm->Destroy();

    EerrNo ret = m_IFsmPool.DeActive(pFsm->GetId());
    CPP_ASSERT_RN(ERR_OK == ret);
}

/*********************************************************************
�������� : UnfrozenFsm
�������� : ִ�иú�����ǰ��������--SAVE���в���Ϊ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintCounter(CLI_ENV* pCliEnv)const
{
    //��ӡģ�黺����Ϣ
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_REV_INDEX               :%u\r\n", m_oprCounter.Get(FMGR_REV_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_SELF_PRC_INDEX          :%u\r\n", m_oprCounter.Get(FMGR_SELF_PRC_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_INNER_SEND_INDEX        :%u\r\n", m_oprCounter.Get(FMGR_INNER_SEND_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_OUTER_SEND_INDEX        :%u\r\n", m_oprCounter.Get(FMGR_OUTER_SEND_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_SEND_OUT_FAIL_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_SEND_OUT_FAIL_INDEX));

    //ģ���ڲ�ת������Ϣ�ļ���
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_TRANSFER_INDEX          :%d\r\n", m_oprCounter.Get(FMGR_TRANSFER_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_TRANSFER_FAIL_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_TRANSFER_FAIL_INDEX));

    //ϵͳ��BUF�ķ��������Ϣ
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_MALLOC_HEAP_BUF_INDEX   :%u\r\n", m_oprCounter.Get(FMGR_MALLOC_HEAP_BUF_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_MALLOC_HEAP_FAIL_INDEX  :%u\r\n", m_oprCounter.Get(FMGR_MALLOC_HEAP_FAIL_INDEX));
    (XVOID)XOS_CliExtPrintf(pCliEnv, "FMGR_FREE_HEAP_BUF_INDEX     :%u\r\n", m_oprCounter.Get(FMGR_FREE_HEAP_BUF_INDEX));

    //��Ϣ�������
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
�������� : PrintFsm
�������� : ��ӡ״̬����Ϣ

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : PrintFsmRang
�������� : ��ӡĳ����Χ״̬����Ϣ

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFactory::PrintFsmRang(FSMID fsmStart, XU32 uiCnt, CLI_ENV* pCliEnv)
{
    m_IFsmPool.PrintItems(fsmStart, uiCnt, pCliEnv);
}

/*********************************************************************
�������� : PrintFsmLastFree
�������� : ��ӡ����״̬״̬��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
IFsm::~IFsm()
{
    m_pIFct = XNULL;
}


/*********************************************************************
�������� :
�������� : ������״̬��״̬֮ǰ�����õĺ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PreSetState(Tstate)
{
}


/*********************************************************************
�������� : PreSendMsgProc
�������� : ��Ϣ����֮ǰ�����õĺ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PreSendMsgProc(CMsg* ,const CFsmAddr& ,ESendPrio )
{
}


/*********************************************************************
�������� :  PrePrCMsg
�������� :  �ڵ���ProcessMsg(CMsg* & pMsg)֮ǰ���õĺ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PrePrcMsg(CMsg *& )
{
}


/*********************************************************************
�������� :  PostPrCMsg
�������� :   �ڵ���ProcessMsg(CMsg* & pMsg)֮����õĺ���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID IFsm::PostPrcMsg(CMsg *&)
{
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFsm::Create()
{
    return ERR_OK;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo IFsm::Destroy()
{
    FreeAllSaveMsg();
    FreeAllHoldMsg();
    return ERR_OK;
}


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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



