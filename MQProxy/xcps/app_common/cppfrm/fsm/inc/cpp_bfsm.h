/************************************************************************************
�ļ���  :cpp_bfsm.h

�ļ�����:

����    :zzt

��������:2005/10/28

�޸ļ�¼:
         2006.03.09  zzt     �°汾ƽ̨������Ϣͷ�ļ���������ƽ̨
************************************************************************************/
#ifndef __CPP_BFSM_H_
#define __CPP_BFSM_H_

#include "cpp_adapter.h"
#include "cpp_common.h"
#include "cpp_tcn_list.h"
#include "cpp_frmmiddle.h"
#include "cpp_mutex.h"



class CMsg;
class CMsgBox;
class CMsgListAllc;
class IFsm;
class IFactory;


#define FSM_NUM_IN_FACMGR      256

typedef XU32                   Tstate;       //״̬
typedef TResourcePool<IFsm>    IFsmPool;
typedef tcn::list<CMsg*,CMsgListAllc>  CMsgLink;

enum EPrcResult
{
    BAD_MSG,                        //��Ϣ����
    PRC_ERR,                        //������Ϣ�Ĺ����з�������
    UNKNOWN_MSG,                    //
    PROCESSED,                      //��Ϣ������
    DISCARD,                        //����Ϣ������
    KILL_FSM,                       //��Ϣ����Ľ����ɱ����״̬��
    FAC_NEW,                        //������һ���µĴ�����Ϣ�Ķ���
    FAC_GET,                        //����������Ϣֻ���ҵ���һ���ִ�Ķ��󣬵���δ�����µĶ���
    FAC_GET_LIST,                   //���������ҵ������б�,ͬʱҲ��Ҫ������Ϣ�������б�
    SAVE_MSG,                       //��Ϣ�ı�SAVE����
    HOLD_MSG,                       //��Ϣ��HOLD����
    TRANSFER_MSG,                   //��Ϣ��ת��
    USER_PROC
};


//FactMgr������Ŀ
enum EFMgrCounterIndex
{
    FMGR_REV_INDEX,                 //���յ�����Ϣ�ļ���
    FMGR_SELF_PRC_INDEX,            //�����������Լ��������Ϣ�ļ���
    FMGR_INNER_SEND_INDEX,          //ģ���ڲ�������Ϣ�ļ���
    FMGR_OUTER_SEND_INDEX,          //���������ģ�鷢�͵���Ϣ�ļ���
    FMGR_SEND_OUT_FAIL_INDEX,       //���������ģ�鷢����Ϣʧ�ܵļ���
    FMGR_TRANSFER_INDEX,            //ģ��ת����Ϣ�ļ���
    FMGR_TRANSFER_FAIL_INDEX,       //���������ģ��ת����Ϣʧ�ܵļ���

    FMGR_MALLOC_HEAP_BUF_INDEX,     //�����ϵͳ��Ϣ����ļ���
    FMGR_MALLOC_HEAP_FAIL_INDEX,    //�����ϵͳ��Ϣ����ʧ�ܵļ���
    FMGR_FREE_HEAP_BUF_INDEX,       //�ͷŶѻ���ļ���

    FMGR_UNKNOWN_INDEX,             //������ΪUNKNOWN����Ϣ�ļ���
    FMGR_FSM_PROC_INDEX,            //״̬���������Ϣ�ļ���
    FMGR_SAVE_INDEX,                //������ΪSAVE����Ϣ�ļ���
    FMGR_HOLD_INDEX,                //������ΪHOLD����Ϣ�ļ���
    FMGR_DISCARD_INDEX,             //������ΪDISCARD����Ϣ�ļ���
    FMGR_BAD_INDEX,                 //������ΪBAD����Ϣ�ļ���
    FMGR_ERR_INDEX,                 //����������Ϣ����
    FMGR_USER_PROC_INDEX,           //Ӧ���Լ��ͷ�,����Ҫ��ܴ������Ϣ

    FMGR_INDEX_NUM
};


/*********************************************************************
���� : CFsmAddr
ְ�� : ��ʾ״̬���ĵ�ַ
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFsmAddr:public t_XOSUSERID
{
    //�ṩ�������캯��
    CFsmAddr()
    {
        XOS_MemSet(this, 0, sizeof(*this));
    }

    CFsmAddr(MDID uiMid, FSMID fsmId,BID uiBid)
    {
        PID     = uiBid;
        FID     = uiMid;
        FsmId   = fsmId; //״̬��ID
    }

    CFsmAddr(MDID uiMid, FSMID fsmId)
    {
        PID     = XOS_GetLocalPID(); //ʹ�ñ���ID
        FID     = uiMid;
        FsmId   = fsmId; //״̬��ID
    }

    bool IsInSameModule(const CFsmAddr& right)const
    {
        return (PID == right.PID)&&(FID == right.FID);
    }

    bool operator==(const CFsmAddr&other)const
    {
        return IsInSameModule(other)&&(FsmId == other.FsmId);
    }

    XU32 GetBId()const
    {
        return PID;
    }
    XVOID SetBId(XU32 uiBId)
    {
        PID = uiBId;
    }

    XU32 GetMId()const
    {
        return FID;
    }
    XVOID SetMId(XU32 uiMid)
    {
        FID = uiMid;
    }

    XU32 GetFsmId()const
    {
        return FsmId;
    }
    XVOID SetFsmId(XU32 uiFsmId)
    {
        FsmId = uiFsmId;
    }
};

/*********************************************************************
���� : CMsg
ְ�� : ��ܻ����ͷ����ṹ����ƽ̨����Ϣͷ�Ϳ���Լ�����Ϣͷ
        1.������Ϣ��Դ��Ŀ��ַ��Ϣ��<��ID��ģ��ID��״̬��ID>
        2.������Ϣ��ID,����Ϣ��ȫ��(���Ȱ���CMSG+t_XOSCOMMHEAD)
        3.Ϊ�������Ϣ�ķ���Ч�ʡ�ͷ�м���CPU�ֽ����ֶΡ�
          �ر�ע�⣬�û�������Ϣʱ,�õ���CMsg*ָ���CMsg�е��ֶ��Ѿ���ת������
          �����ֽ���,�����û���ת�����û��ڷ�����Ϣʱ��ֻ�谴�����ֽ���
          ��дͷ���ֶμ���

       ������Ϣ�ڴ��˵��:
          ��Ϣ�������͡�һ����ƽ̨�Ķѣ���һ����m_DycBuff�е��ڴ档����һ��ģ���ڲ�������Ϣ
          ʱ����MODULE_BUFFER���͵Ķѣ���ģ��֮����䷢����Ϣʱʹ��SYS_BUFFER�ڴ�
          �ǲ������ö�ջ����ġ���CMsg��Init�н��ڴ�����Ĭ��Ϊջ����,��Ϊ�˷�ֹ�û�����
          ��Ϣʱʹ��ջ�ռ���ڴ�
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
//���ͱ�־��
//�������ͣ����͵��Է�����Ϣ���е�ͷ��
//һ�㷢�ͣ����͵��Է�����Ϣ���е�β
enum ESendPrio
{
    LOW_SEND   = eChuckMsgPrio,
    NRML_SEND  = eNormalMsgPrio,
    URG_SEND,
    HIGHEST_SEND
};

class CMsg
{
    friend class CMsgBox;
    friend class IFactory;
    friend CMsg* MallocMsgBufOuterToFrame(XU32, MDID);

public:
    //������Ϊ����Ӧ���ܴ�CMsg���������ܳ�ʼ�����ݳ�Ա
    CMsg()
    {
        InitSelf();
    }
    //��ӡ��Ϣ�����ݣ�
    XVOID Print()
    {
    }

public:
    //��ѯ��������Ԫ��
    CFsmAddr GetReciverAddr()
    {
        return CFsmAddr(GetRcvMId(),GetRcvFsmId(),GetRcvBId());
    }
    BID GetRcvBId()const
    {
        return m_cpsHead.datadest.PID;
    }
    MDID GetRcvMId()const
    {
        return m_cpsHead.datadest.FID;
    }
    FSMID GetRcvFsmId()const
    {
        return m_cpsHead.datadest.FsmId;
    }

public:
    //���ý�������Ԫ��
    XVOID SetReciverAddr(const CFsmAddr& addr)
    {
        SetRcvBId(addr.GetBId());
        SetRcvMId(addr.GetMId());
        SetRcvFsmId(addr.FsmId);
    }
    XVOID SetRcvBId(BID uiBid)
    {
        m_cpsHead.datadest.PID = uiBid;
    }
    XVOID SetRcvMId(MDID mId)
    {
        m_cpsHead.datadest.FID = mId;
    }
    XVOID SetRcvFsmId(FSMID id )
    {
        m_cpsHead.datadest.FsmId = id ;
    }

public:
    //��ѯ��������Ԫ��
    CFsmAddr GetSenderAddr()
    {
        return CFsmAddr(GetSndMId(),GetSndFsmId(),GetSndBId());
    }
    BID GetSndBId()const
    {
        return m_cpsHead.datasrc.PID;
    }
    MDID GetSndMId()const
    {
        return m_cpsHead.datasrc.FID;
    }
    FSMID GetSndFsmId()const
    {
        return m_cpsHead.datasrc.FsmId;
    }
    XU32 GetMsgLen()
    {
        return m_cpsHead.length;
    }

public:
    //���÷�������Ԫ��
    XVOID SetSenderAddr(const CFsmAddr& addr)
    {
        SetSndBId(addr.GetBId());
        SetSndMId(addr.GetMId());
        SetSndFsmId(addr.FsmId);
    }
    XVOID SetSndBId(BID uiBid)
    {
        m_cpsHead.datasrc.PID = uiBid;
    }
    XVOID SetSndMId(MDID mId)
    {
        m_cpsHead.datasrc.FID = mId;
    }
    XVOID SetSndFsmId(FSMID id)
    {
        m_cpsHead.datasrc.FsmId = id;
    }
    XVOID SetMsgLen(XU32 uiMsgLen)
    {
        m_cpsHead.length = uiMsgLen;
    }

public:
    //����һЩ�ֶεĽӿ�
    TMSGID GetMsgId()const
    {
        return m_cpsHead.msgID;
    }
    XVOID SetMsgId(TMSGID msgId)
    {
        m_cpsHead.msgID = msgId;
    }
    TMSGID GetSubId()const
    {
        return m_cpsHead.subID;
    }
    XVOID SetSubId(TMSGID subID)
    {
        m_cpsHead.subID = subID;
    }
    XU32 GetTraceId()const
    {
        return m_cpsHead.traceID;
    }
    XVOID SetTraceId(XU32 trId)
    {
        m_cpsHead.traceID = trId;
    }
    XU32 GetLogId()const
    {
        return m_cpsHead.logID;
    }
    XVOID SetLogId(XU32 LogId)
    {
        m_cpsHead.logID = LogId;
    }
    XVOID SetMsgPrio(ESendPrio ePrio) //������Ϣ���ȼ�
    {
        m_cpsHead.prio = (e_MSGPRIO)ePrio;
    }

public:
    XU32 GetFullMsgLength()const
    {
        return  m_cpsHead.length + CPS_MSGHEAD_LENGTH;
    }
    XVOID* GetPayloadBeginAddr()const //����غɵ���ʼ��ַ
    {
        return (XU8*)(this)+sizeof(CMsg);
    }
    XU32 GetPayloadLength()const
    {
        CPP_ASSERT_RV((GetFullMsgLength() >= sizeof(CMsg)), 0);
        return GetFullMsgLength() - sizeof(CMsg);
    }
    static CMsg* CastCMsg(t_XOSCOMMHEAD& refHead)
    {
        return reinterpret_cast<CMsg*>(&refHead);
    }
    XVOID* GetCpsMsgPoint()const
    {
        return m_cpsHead.message;
    }
    XVOID SetCpsMsgPoint(XVOID * message)
    {
        m_cpsHead.message = message;
    }

public:
    XVOID InitSelf()
    {
        //��Ҫ��ƽ̨����Ϣͷ���ֶ�
        XOS_MemSet(((XU8*)this)+CPS_MSGHEAD_LENGTH, 0, sizeof(CMsg)-CPS_MSGHEAD_LENGTH);

        //��Ҫ��ƽ̨����Ϣͷ���ֶ�
        SetSndBId(XOS_GetLocalPID());
    }

public:
    //ƽ̨��Ϣͷ
    //ע��:��������������Զ����Ա, ����ֻ��Ϊ���ṩXOS��Ϣͷ��װCPP����
    t_XOSCOMMHEAD m_cpsHead;
};


/*********************************************************************
���� : class  CTimerHand
ְ�� : ��ʱ�����������Ĭ�ϵĶ���ѭ����ʱ��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CTimerHand
{
    friend class CTimerMgr;

public:
    //ͨ��Ĭ�Ϲ��캯��ֻ�ܹ�����Ч�Ķ�ʱ�����
    CTimerHand()
    {
        SetInvalide();
    }
    ~CTimerHand()
    {
        SetInvalide();
    }

    bool IsInvalidHand()const
    {
        return (XNULL == m_ssTimerHand);
    }
    XVOID* GetTimerHand()const
    {
        return m_ssTimerHand;
    }
    //��øö�ʱ����״̬
    e_TIMESTATE GetCurState()const;


private:
    XVOID SetTimerHand(XVOID *p)
    {
        m_ssTimerHand = p;
    }
    XVOID SetInvalide()
    {
        SetTimerHand(XNULL);
    }

private:
    XVOID* m_ssTimerHand;
};


/*********************************************************************
���� : CTimerOutMsg
ְ�� : ��ʱ����ʱ��Ϣ
       ����еĶ����ڶ�ʱ����ʱ���յ�����Ϣ
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CTimerOutMsg: public CMsg
{
public:
    //�Ƿ�Ϊ��ʱ��refH�ĳ�ʱ��Ϣ?
    bool IsBelongTo(const CTimerHand& refH)const
    {
        return (m_timerHand == refH.GetTimerHand());
    }

    XVOID*         m_timerHand;      //��ʱ�����
    XPOINT         m_ulUserPara;     //�û�����ֵ
    XPOINT         m_ulUserExtPara;  //Ϊ������һ���û�����������������ӵ� 2008.11.13
};

/*******************************************************************
���� : CTimerMgr
ְ�� : ��װƽ̨�Ķ�ʱ��ʵ�ֻ���
       1.��ȡϵͳ��������ϢIDΪTIMER_MSG_ID����Ϣ,��
         TimerMgr��������IFactoryMgr����Ϣ�������
         CTimerOutMsg���͵���Ϣ
       2.�ṩ�����Ķ�ʱ������ӿڣ�create,start,delete
       3.ѭ����ʱ��,һ���Զ�ʱ��
Э�� :
       1.IFactory
       2.ƽ̨ȫ��API����
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CTimerMgr
{
private:
    friend class CTimerHand;

    //��ʱ����ʱ��ƽ̨���ó�ʱ�ص�����ʱ���ݵĲ���VOID*
    struct CTimerPara
    {
        PTIMER  m_cpsTimerHander;  //ƽ̨��ʱ�����
        MDID    m_moduleId;        //��ʱ����ʱ��ϢĿ�Ĺ���������ID
        FSMID   m_tagertFsmId;     //��ʱ����ʱ��ϢĿ��״̬��ID
        XPOINT  m_uiUserPara;      //�û�����ֵ
        XPOINT  m_uiUserExtPara;   //Ϊ������һ���û�����������������ӵ� 2008.11.13
    };

#define PARA_LENGTH sizeof(CTimerPara)

public:
    CTimerMgr(IFactory* pFct,XU16 usMaxTimerNum)
        :m_CallbackPara(PARA_LENGTH,usMaxTimerNum)
    {
        CPP_ASSERT_RN((pFct != XNULL) && (usMaxTimerNum != 0));
        m_IFct = pFct;
    }
    ~CTimerMgr() throw()
    {
        m_IFct = XNULL;
    }

    //������ʱ����FctId ��ʱ����������,��ʱ������״̬��,��ʱ������(ѭ����һ����)
    XVOID CreateTimer(CTimerHand& refTId, FSMID fsmId,TimerType timerType = TIMER_TYPE_LOOP, XPOINT ulUserPara = 0, XPOINT ulUserExtPara = 0 );
    XVOID StopTimer(const CTimerHand& refTId)const;
    XVOID DeleteTimer(CTimerHand& refTId);
    XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount);  //�������붨ʱ��,��λΪMS
    XVOID StartSTimer(const CTimerHand& tid, XU32 Second)       //�붨ʱ��
    {
        StartMSTimer(tid,Second*1000);
    }
    //ע���ƽ̨�Ķ�ʱ����ʱ�ص�����
    static XS8 TimerCallback(t_BACKPARA*);

private:
    static CTimerPara * ExtractTimerPara(const CTimerHand& refTH)
    {
        return reinterpret_cast<CTimerPara *>(refTH.GetTimerHand());
    }

private:
    CChunk         m_CallbackPara;
    IFactory   *m_IFct;
};

/*********************************************************************
���� : class CMsgListAllc
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CMsgListAllc
{
public:
    //���캯��,�����ʹ�С
    CMsgListAllc(size_t size)
    {
        CPP_ASSERT_RN(m_sChunk.GetBlockSize() == size);
    }

    static XVOID* allocate()
    {
        CMutexGuard tmp(m_sMutex);
        return m_sChunk.Allocate();
    }
    static XVOID deallocate(XVOID  *p)
    {
        if(XNULL != p)
        {
            CMutexGuard tmp(m_sMutex);
            m_sChunk.Deallocate(p);
        }
    }
    bool operator == (const CPreAllocator& other)const
    {
        return false;
    }
    bool operator != (const CPreAllocator& other)const
    {
        return true;
    }

private:
    //����������Ա������MAIN����ִ��֮ǰ��ʼ��
    static CMutex m_sMutex;
    static CChunk m_sChunk;
};

/*********************************************************************
���� : class IFsm
ְ�� :

Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class IFsm
{
public:
    IFsm(IFactory *pIFct, Tstate state,FSMID fsmId = INVALID_OBJ_ID)
    {
        CPP_ASSERT_RN(pIFct != XNULL);
        m_pIFct    = pIFct;
        m_state    = state;
        m_eCode    = ERR_OK;
        SetId(fsmId);
    }
    virtual ~IFsm();

    FSMID GetId()const
    {
        return  m_Id;
    }
    XVOID SetId(FSMID Id)
    {
        m_Id = Id;
    }
    IFactory* GetFact()const
    {
        return m_pIFct;
    }

    EerrNo GetErr()const
    {
        return m_eCode;
    }
    XVOID SetErr(EerrNo en)
    {
        m_eCode = en;
    }

    Tstate GetState()const
    {
        return m_state;
    }

    XVOID SetState(Tstate newState)
    {
        PreSetState(newState);
        m_state = newState;
    }

    //������Ϣ����������. ��Ϣ����ǰ��Ԥ����,��Ϣ����,��Ϣ�����
    virtual XVOID       PrePrcMsg(CMsg*  &pMsg);
    virtual EPrcResult ProcessMsg(CMsg* pMsg) = 0;
    virtual XVOID       PostPrcMsg(CMsg* &pBuf);
    virtual EerrNo      Create();
    virtual EerrNo      Destroy();

    //�洢��HOLD����Ϣ����,
    XU32 GetSaveNum()const
    {
        return (XU32)m_save.size();
    }
    CMsg* GetSaveMsg()
    {
        return GetMsgFormList(m_save);
    }
    XVOID PutSaveMsg(CMsg* pMsg)
    {
        CPP_ASSERT_RN(pMsg != XNULL);
        //����Ϣ�ӵ�β����
        m_save.push_back(pMsg);
    }
    XU32 GetHoldNum()const
    {
        return (XU32)m_hold.size();
    }
    CMsg* GetHoldMsg()
    {
        return GetMsgFormList(m_hold);
    }
    XVOID PutHoldMsg(CMsg* pMsg)
    {
        CPP_ASSERT_RN(pMsg != XNULL);
        m_hold.push_back(pMsg);
    }

    XVOID PutHoldMsgToHead(CMsg* pMsg)
    {
        CPP_ASSERT_RN(pMsg != XNULL);
        m_hold.push_front(pMsg);
    }

    //Ҫ����м��ͷ�
    inline EerrNo FreeHoldMsg(CMsg* pMsg);
    XVOID FreeAllHoldMsg()
    {
        FreeLinkMsg(m_hold);
    }
    XVOID FreeAllSaveMsg()
    {
        FreeLinkMsg(m_save);
    }
    bool IsEmptySaveMsg()const
    {
        return m_save.empty();
    }
    bool IsEmptyHoldMsg()const
    {
        return m_hold.empty();
    }

    //��ӡ��Ϣ�ӿ�
    virtual XVOID Print(CLI_ENV* pCliEnv);

public:
    //��Ϣ������ί�нӿ�
    XVOID GetFsmAddr(CFsmAddr& addr)const;
    CMsg *GetMsgBuf(XU32 uiFullLen);
    XVOID FreeMsgBuf(CMsg* pBuf);
    EerrNo SendMsg(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri = NRML_SEND);

public:
    //��ʱ�������Ľӿ�
    //������ʱ��.����ʱ����ʱ����򴴽��ö�ʱ����IFsm������һ����Ϣ
    inline XVOID CreateTimer(CTimerHand& refTId ,TimerType timerType = TIMER_TYPE_LOOP,XPOINT ulUserPara = 0,XPOINT ulUserExtPara = 0);
    //ɾ����ʱ��.
    inline XVOID StopTimer(const CTimerHand& refTId)const;
    inline XVOID DeleteTimer(CTimerHand& refTId);
    //�������붨ʱ��
    inline XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount);
    //�����붨ʱ��
    inline XVOID StartSTimer(const CTimerHand& tid, XU32 Second);

protected:
    //�ڵ�������״̬��״̬֮ǰ�����õĺ���
    virtual  XVOID PreSetState(Tstate newState);
    virtual  XVOID PreSendMsgProc(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri);

private:
    CMsg* GetMsgFormList(CMsgLink& refLink)
    {
        if(refLink.empty())
        {
            return XNULL;
        }
        //��ͷ��ȡ����Ϣ
        CMsg * ptmp = refLink.front();
        refLink.pop_front();
        return ptmp;
    }
    XVOID FreeLinkMsg(CMsgLink& MsgLink);

protected:
    FSMID           m_Id;
    IFactory*    m_pIFct;

private:
    Tstate          m_state;
    EerrNo          m_eCode;

    CMsgLink        m_save;   //save��Ϣ����,��Щ��Ϣ��ǰ״̬���ܴ���
    CMsgLink        m_hold;   //Hold��Ϣ����,��Щ��Ϣ��ǰ״̬���ܴ���
};


/*********************************************************************
���� : class IFactory
ְ�� : ����������ͬ�Ĵ�IFsm�����Ķ���
Э��:
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class IFactory
{
    friend class CTimerMgr;
    friend class CMsgBox;

public:
    //id ģ��ID, usMaxTimerNum ���ʱ������, maxNum ���״̬������
    IFactory(MDID id, XU16 usMaxTimerNum, XU32 maxNum, bool needRandom = true);
    virtual ~IFactory();

    virtual EerrNo OnCreate();
    virtual EerrNo OnNotice();

    //��ʼ������,�ڹ��ź������ú���øú���
    EerrNo Init()
    {
        return OnCreate();
    }
    MDID GetId()const
    {
        return m_Id;
    }
    XVOID SetId(MDID id)
    {
        m_Id = id;
    }

public:
    //��������Ϣ��ط���
    //����Ϣ�������һ����Ϣ,������ӵ�β��
    XVOID AddToTail(CMsg* p)
    {
        m_CMsgPump.push_back(p);
    }
    XVOID AddToHead(CMsg* p)
    {
        m_CMsgPump.push_front(p);
    }

    //������ں���
    static XS8 sMsgEntry(XVOID* pCpsMsg,XVOID *pObj);
    EPrcResult ProcessMsg(CMsg* pMsg);
    XVOID SaveMsgPrc(IFsm *pFsm);
    virtual XVOID SpecialProc(XVOID*& pCpsMsg) {};

    virtual bool CanPrcMsg(CMsg *pMsg);
    virtual EPrcResult FsmIdIsZeroPrc(CMsg *pMsg, IFsm *& rpFsm) = 0; //����FsmId=0��Ϣ
    virtual EPrcResult RouteMsgErr(CMsg *pMsg); //Fsm�Ҳ���������Ϣ����

    //��Ϣ�����ί�еĽӿ�
    CMsg *GetMsgBuf(XU32 uiFullLen);
    virtual XVOID PreFreeMsgBuf(CMsg* pBuf);
    XVOID FreeMsgBuf(CMsg* pBuf);
    virtual EerrNo SendMsg(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri = NRML_SEND);
    XVOID TransferMsg(CMsg* pBuf); //����ת��

public:
    //�����Ƕ�ʱ��������ط���
    XVOID  CreateTimer(CTimerHand& refTId ,TimerType tt = TIMER_TYPE_LOOP, FSMID fsmId = INVALID_OBJ_ID, XPOINT ulUserPara=0, XPOINT ulUserExtPara = 0)
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->CreateTimer(refTId, fsmId,tt,ulUserPara,ulUserExtPara);
    }
    XVOID DeleteTimer(CTimerHand& refTId)
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->DeleteTimer(refTId);
    }
    XVOID StopTimer(const CTimerHand& refTId)const
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->StopTimer(refTId);
    }
    XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount) //�������붨ʱ��
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->StartMSTimer(tid, uimsCount);
    }
    XVOID StartSTimer(const CTimerHand& tid, XU32 Second) //�����붨ʱ��
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->StartSTimer(tid, Second);
    }

public:
    //��������Դ����ط���
    IFsmPool& GetResource()
    {
        return m_IFsmPool;
    }
    const IFsmPool& GetResource()const
    {
        return m_IFsmPool;
    }
    IFsm* ActiveFsm()
    {
        IFsm *ptmp = m_IFsmPool.Active();
        if(XNULL != ptmp)
        {
            CPP_ASSERT_RV(ptmp->IsEmptyHoldMsg()&& ptmp->IsEmptySaveMsg(), ptmp);
        }
        return ptmp;
    }
    IFsm* ActiveFsm(FSMID id)
    {
        IFsm *ptmp = m_IFsmPool.Active(id);
        if(XNULL != ptmp)
        {
            CPP_ASSERT_RV(ptmp->IsEmptyHoldMsg()&& ptmp->IsEmptySaveMsg(), ptmp);
        }
        return ptmp;
    }
    EerrNo Input(IFsm *pFsm)
    {
        return m_IFsmPool.Input(pFsm);
    }
    IFsm* GetById(FSMID fsmId)const
    {
        return m_IFsmPool.GetById(fsmId);
    }

    XVOID KillFsm(IFsm *pFsm);
    XVOID UnfrozenFsm(XU32 fsmId);

public:
    //�����ǵ�����Ϣ����
    XVOID PrintCounter(CLI_ENV* pCliEnv)const;
    XVOID PrintFsm(FSMID fsmId, CLI_ENV* pCliEnv);
    XVOID PrintFsmRang(FSMID fsmStart, XU32 uiCnt, CLI_ENV* pCliEnv);
    XVOID PrintFsmLastFree(XU32 uiStartId, XU32 uiCnt, CLI_ENV* pCliEnv);

    XVOID IncCount(XU32 uiIdx)
    {
        m_oprCounter.Inc(uiIdx);
    }

    XVOID ClrCounter()
    {
        m_oprCounter.ClearAll();
    }

private:
    //��ֹ�����͸�ֵ
    FORBID_COPY_ASSIGN(IFactory);

protected:
    MDID            m_Id;          //FactMgrģ��ID

    XU32            m_uiMaxFsm;    //ͬʱ����һ����Ϣ�����״̬�������ܳ���FSM_NUM_IN_FACTMGR(255)��(�ಥ�����)
    IFsm*           m_apFsm_list[FSM_NUM_IN_FACMGR]; //��ʱ����״̬������,����save��Ϣ��
    IFsmPool        m_IFsmPool;    //IFSM�������Ͷ���Ļ����

    CMsgLink        m_CMsgPump;    //��Ϣ��
    CMsgBox*        m_pMsgBox;     //��Ϣ���������

private:
    CTimerMgr*      m_pTimer;      //��ʱ�����������
    CSimpleCounter  m_oprCounter;  //���Լ�����Ϣ
};

/*********************************************************************
���� : class CMsgBox
ְ�� : ��װƽ̨��Ϣ�ķ���,���ͺ��ͷŻ���
       1.��ͬһģ���ڲ�������Ϣ�Ļ������÷�����е���Ϣ
       2.ת������Ϣ��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CMsgBox
{
public:
    CMsgBox(IFactory* pOwner)
    {
        CPP_ASSERT_RN(pOwner != XNULL);
        m_pFM = pOwner;
    }
    ~CMsgBox()
    {
        m_pFM  = XNULL;
    }

public:
    //uiFullLength,��Ϣ����������, ����XOS��Ϣͷ
    CMsg* MallocMsgBuf(XU32 uiFullLength);
    XVOID  FreeMsgBuf(CMsg *pMsg);
    EerrNo SendMsg(CMsg*& pMsg,ESendPrio iFlag = NRML_SEND);//Ĭ��Ͷ�ݵ���Ϣ���е�β��
    EerrNo TransferMsg(CMsg* pMsg);//�������ͣ�����Ͷ�ݵ���Ϣ���е�ͷ

private:
    //��ģ���ڲ�����Ϣ, �������ڲ�����֮�����Ϣ����
    bool IsInnerModMsg(const CMsg& refMsg)const
    {
        return (refMsg.GetRcvBId() == XOS_GetLocalPID())
               && (refMsg.GetRcvMId() == m_pFM->GetId());
    }
    XU32 GetPayloadLength(const CMsg&refMsg)const
    {
        CPP_ASSERT_RV(refMsg.GetFullMsgLength() >= sizeof(CMsg), 0);
        return refMsg.GetFullMsgLength()-sizeof(CMsg);
    }
    XVOID IncCount(XU32 uiIdx)
    {
        m_pFM->IncCount(uiIdx);
    }

private:
    //��ֹ�����͸�ֵ
    FORBID_COPY_ASSIGN(CMsgBox);

private:
    IFactory* m_pFM;
};




///////////////////////////////////////////////////
//��CTimerHand������������
///////////////////////////////////////////////////
inline e_TIMESTATE CTimerHand::GetCurState()const
{
    if(IsInvalidHand())
    {
        return TIMER_STATE_NULL;
    }
    CTimerMgr::CTimerPara* tmpTP = static_cast<CTimerMgr::CTimerPara*>(m_ssTimerHand);
    CPP_ASSERT_RV(XNULL != tmpTP, TIMER_STATE_NULL);

    return XOS_TimerGetState(tmpTP->m_moduleId,tmpTP->m_cpsTimerHander);
}


///////////////////////////////////////////////////
//��CTimerMgr������������
///////////////////////////////////////////////////
inline XVOID CTimerMgr::StartMSTimer(const CTimerHand& tid, XU32 uimsCount)
{
    CTimerPara*ptp = ExtractTimerPara(tid);
    if(ptp == XNULL)
    {
        return ;
    }
    CPP_ASSERT_RN(m_CallbackPara.InChunk(ptp));

    EerrNo ret = TimerStartCps(ptp->m_moduleId,ptp->m_cpsTimerHander,uimsCount);
    CPP_ASSERT_RN(ERR_OK == ret);
}


///////////////////////////////////////////////////
//��IFactoryMgr������������
///////////////////////////////////////////////////
inline CMsg * IFactory::GetMsgBuf(XU32 uiFullLen)
{
    CMsg* pMsg = m_pMsgBox->MallocMsgBuf(uiFullLen);
    if(pMsg != XNULL)
    {
        //���÷����ߵĿ�ID
        pMsg->SetSndMId(GetId());
    }
    return pMsg;
}

inline XVOID IFactory::FreeMsgBuf(CMsg *pMsg)
{
    this->PreFreeMsgBuf(pMsg);
    m_pMsgBox->FreeMsgBuf(pMsg);
}

inline EerrNo IFactory::SendMsg(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri)
{
    CPP_ASSERT_RV(pBuf != XNULL, ERR_NOT_INITIAL);
    pBuf->SetReciverAddr(refTarget);
    return m_pMsgBox->SendMsg(pBuf,pri);
}


///////////////////////////////////////////////////
//��IFsm������������
///////////////////////////////////////////////////
inline XVOID IFsm::CreateTimer(CTimerHand& refTId ,TimerType tt,XPOINT ulUserPara,XPOINT ulUserExtPara)
{
    m_pIFct->CreateTimer(refTId,tt, GetId(), ulUserPara, ulUserExtPara);
}

inline XVOID IFsm::DeleteTimer(CTimerHand& refTId)
{
    m_pIFct->DeleteTimer(refTId);
}

inline XVOID IFsm::StopTimer(const CTimerHand& refTId) const
{
    m_pIFct->StopTimer(refTId);
}

inline XVOID IFsm::StartMSTimer(const CTimerHand& tid, XU32 uimsCount)
{
    m_pIFct->StartMSTimer(tid, uimsCount);
}

inline XVOID IFsm::StartSTimer(const CTimerHand& tid, XU32 Second)
{
    m_pIFct->StartSTimer(tid, Second);
}

inline EerrNo IFsm::FreeHoldMsg(CMsg* pMsg)
{
    for(CMsgLink::iterator iter = m_hold.begin(); iter != m_hold.end(); ++iter)
    {
        if(*iter == pMsg)
        {
            m_hold.erase(iter);
            m_pIFct->FreeMsgBuf(pMsg);
            return ERR_OK;
        }
    }
    return ERR_OBJ_NOT_FOUND;
}

inline CMsg * IFsm::GetMsgBuf(XU32 uiFullLen)
{
    CMsg* pMsg = m_pIFct->GetMsgBuf(uiFullLen);
    if(pMsg != XNULL)
    {
        pMsg->SetSndFsmId(GetId());
    }
    return pMsg;
}

inline XVOID IFsm::FreeMsgBuf(CMsg* pBuf)
{
    m_pIFct->FreeMsgBuf(pBuf);
}

inline EerrNo IFsm::SendMsg(CMsg* pMsg,const CFsmAddr& refTarget,ESendPrio pri)
{
    PreSendMsgProc(pMsg,refTarget,pri);
    return m_pIFct->SendMsg(pMsg,refTarget,pri);
}

inline XVOID IFsm::FreeLinkMsg(CMsgLink& MsgLink)
{
    for(CMsgLink::iterator it = MsgLink.begin(); it != MsgLink.end(); ++it)
    {
        m_pIFct->FreeMsgBuf(*it);
    }
    MsgLink.clear();
}

inline XVOID IFsm::GetFsmAddr(CFsmAddr& addr)const
{
    addr.FsmId   = GetId();
    addr.FID     = m_pIFct->GetId();
    addr.PID     = XOS_GetLocalPID();
}


/*********************************************************************
�������� : MallocMsgBufOuterToFrame
�������� : ���֮���ʵ������֮�еĶ�������ϢҪ���øýӿڷ��仺��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline CMsg* MallocMsgBufOuterToFrame(XU32 uiFullMsgLen,MDID mid)
{
    CMsg* pMsg =(CMsg*)MallocMsgBufCps(GetCPsLenFromFullLength(uiFullMsgLen),mid);
    if(pMsg != NULL)
    {
        pMsg->InitSelf();
    }
    return pMsg;
}


#endif

