/************************************************************************************
文件名  :cpp_bfsm.h

文件描述:

作者    :zzt

创建日期:2005/10/28

修改记录:
         2006.03.09  zzt     新版本平台发布消息头文件。废弃老平台
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

typedef XU32                   Tstate;       //状态
typedef TResourcePool<IFsm>    IFsmPool;
typedef tcn::list<CMsg*,CMsgListAllc>  CMsgLink;

enum EPrcResult
{
    BAD_MSG,                        //消息错误
    PRC_ERR,                        //处理消息的过程中发生错误
    UNKNOWN_MSG,                    //
    PROCESSED,                      //消息被处理
    DISCARD,                        //该消息被丢弃
    KILL_FSM,                       //消息处理的结果是杀死该状态机
    FAC_NEW,                        //生成了一个新的处理本消息的对象
    FAC_GET,                        //如果处理该消息只是找到了一个现存的对象，但并未分配新的对象
    FAC_GET_LIST,                   //工厂可以找到对象列表,同时也需要传递消息到对象列表
    SAVE_MSG,                       //消息的被SAVE起来
    HOLD_MSG,                       //消息被HOLD起来
    TRANSFER_MSG,                   //消息被转发
    USER_PROC
};


//FactMgr计数项目
enum EFMgrCounterIndex
{
    FMGR_REV_INDEX,                 //接收到的消息的计数
    FMGR_SELF_PRC_INDEX,            //工厂管理者自己处理的消息的计数
    FMGR_INNER_SEND_INDEX,          //模块内部发送消息的计数
    FMGR_OUTER_SEND_INDEX,          //向其他板或模块发送的消息的计数
    FMGR_SEND_OUT_FAIL_INDEX,       //向其他板或模块发送消息失败的计数
    FMGR_TRANSFER_INDEX,            //模块转发消息的计数
    FMGR_TRANSFER_FAIL_INDEX,       //向其他板或模块转发消息失败的计数

    FMGR_MALLOC_HEAP_BUF_INDEX,     //分配的系统消息缓存的计数
    FMGR_MALLOC_HEAP_FAIL_INDEX,    //分配的系统消息缓存失败的计数
    FMGR_FREE_HEAP_BUF_INDEX,       //释放堆缓存的计数

    FMGR_UNKNOWN_INDEX,             //处理结果为UNKNOWN的消息的计数
    FMGR_FSM_PROC_INDEX,            //状态机处理的消息的计数
    FMGR_SAVE_INDEX,                //处理结果为SAVE的消息的计数
    FMGR_HOLD_INDEX,                //处理结果为HOLD的消息的计数
    FMGR_DISCARD_INDEX,             //处理结果为DISCARD的消息的计数
    FMGR_BAD_INDEX,                 //处理结果为BAD的消息的计数
    FMGR_ERR_INDEX,                 //处理出错的消息计数
    FMGR_USER_PROC_INDEX,           //应用自己释放,不需要框架处理的消息

    FMGR_INDEX_NUM
};


/*********************************************************************
名称 : CFsmAddr
职责 : 表示状态机的地址
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFsmAddr:public t_XOSUSERID
{
    //提供三个构造函数
    CFsmAddr()
    {
        XOS_MemSet(this, 0, sizeof(*this));
    }

    CFsmAddr(MDID uiMid, FSMID fsmId,BID uiBid)
    {
        PID     = uiBid;
        FID     = uiMid;
        FsmId   = fsmId; //状态机ID
    }

    CFsmAddr(MDID uiMid, FSMID fsmId)
    {
        PID     = XOS_GetLocalPID(); //使用本版ID
        FID     = uiMid;
        FsmId   = fsmId; //状态机ID
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
名称 : CMsg
职责 : 框架基类的头。其结构包含平台的消息头和框架自己的消息头
        1.包含消息的源、目地址信息。<板ID，模块ID，状态机ID>
        2.包含消息的ID,和消息的全长(长度包含CMSG+t_XOSCOMMHEAD)
        3.为了提高消息的发送效率。头中加入CPU字节序字段。
          特别注意，用户处理消息时,得到了CMsg*指针后，CMsg中的字段已经被转换成了
          本机字节序,不用用户再转换。用户在发送消息时，只需按本机字节序
          填写头部字段即可

       关于消息内存的说法:
          消息缓存类型。一类是平台的堆，另一类是m_DycBuff中的内存。当在一个模块内部发送消息
          时分配MODULE_BUFFER类型的堆，在模块之间或板间发送消息时使用SYS_BUFFER内存
          是不允许用堆栈缓存的。在CMsg的Init中将内存类型默认为栈类型,是为了防止用户发送
          消息时使用栈空间的内存
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
//发送标志，
//紧急发送，发送到对方的消息队列的头，
//一般发送，发送到对方的消息队列的尾
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
    //仅仅是为了让应用能从CMsg派生，不能初始化数据成员
    CMsg()
    {
        InitSelf();
    }
    //打印消息的内容，
    XVOID Print()
    {
    }

public:
    //查询接受者四元组
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
    //设置接受者四元组
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
    //查询发送者四元组
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
    //设置发送者四元组
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
    //其他一些字段的接口
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
    XVOID SetMsgPrio(ESendPrio ePrio) //设置消息优先级
    {
        m_cpsHead.prio = (e_MSGPRIO)ePrio;
    }

public:
    XU32 GetFullMsgLength()const
    {
        return  m_cpsHead.length + CPS_MSGHEAD_LENGTH;
    }
    XVOID* GetPayloadBeginAddr()const //获得载荷的起始地址
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
        //不要动平台的消息头部字段
        XOS_MemSet(((XU8*)this)+CPS_MSGHEAD_LENGTH, 0, sizeof(CMsg)-CPS_MSGHEAD_LENGTH);

        //不要动平台的消息头部字段
        SetSndBId(XOS_GetLocalPID());
    }

public:
    //平台消息头
    //注意:后续不能再添加自定义成员, 此类只是为了提供XOS消息头封装CPP函数
    t_XOSCOMMHEAD m_cpsHead;
};


/*********************************************************************
名称 : class  CTimerHand
职责 : 定时器句柄，这里默认的都是循环定时器
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CTimerHand
{
    friend class CTimerMgr;

public:
    //通过默认构造函数只能构建无效的定时器句柄
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
    //获得该定时器的状态
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
名称 : CTimerOutMsg
职责 : 定时器超时消息
       框架中的对象在定时器超时将收到该消息
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CTimerOutMsg: public CMsg
{
public:
    //是否为定时器refH的超时消息?
    bool IsBelongTo(const CTimerHand& refH)const
    {
        return (m_timerHand == refH.GetTimerHand());
    }

    XVOID*         m_timerHand;      //定时器句柄
    XPOINT         m_ulUserPara;     //用户参数值
    XPOINT         m_ulUserExtPara;  //为了满足一个用户参数不够的情况增加的 2008.11.13
};

/*******************************************************************
名称 : CTimerMgr
职责 : 封装平台的定时器实现机制
       1.截取系统发来的消息ID为TIMER_MSG_ID的消息,向
         TimerMgr所关联的IFactoryMgr的消息泵中添加
         CTimerOutMsg类型的消息
       2.提供基本的定时器管理接口，create,start,delete
       3.循环定时器,一次性定时器
协作 :
       1.IFactory
       2.平台全局API函数
历史 :
       修改者   日期          描述
**********************************************************************/
class CTimerMgr
{
private:
    friend class CTimerHand;

    //定时器超时后平台调用超时回调函数时传递的参数VOID*
    struct CTimerPara
    {
        PTIMER  m_cpsTimerHander;  //平台定时器句柄
        MDID    m_moduleId;        //定时器超时消息目的工厂管理者ID
        FSMID   m_tagertFsmId;     //定时器超时消息目的状态机ID
        XPOINT  m_uiUserPara;      //用户参数值
        XPOINT  m_uiUserExtPara;   //为了满足一个用户参数不够的情况增加的 2008.11.13
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

    //创建定时器，FctId 定时器所属工厂,定时器所属状态机,定时器类型(循环，一次性)
    XVOID CreateTimer(CTimerHand& refTId, FSMID fsmId,TimerType timerType = TIMER_TYPE_LOOP, XPOINT ulUserPara = 0, XPOINT ulUserExtPara = 0 );
    XVOID StopTimer(const CTimerHand& refTId)const;
    XVOID DeleteTimer(CTimerHand& refTId);
    XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount);  //启动毫秒定时器,单位为MS
    XVOID StartSTimer(const CTimerHand& tid, XU32 Second)       //秒定时器
    {
        StartMSTimer(tid,Second*1000);
    }
    //注册给平台的定时器超时回调函数
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
名称 : class CMsgListAllc
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CMsgListAllc
{
public:
    //构造函数,个数和大小
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
    //以下两个成员必须在MAIN函数执行之前初始化
    static CMutex m_sMutex;
    static CChunk m_sChunk;
};

/*********************************************************************
名称 : class IFsm
职责 :

协作 :
历史 :
       修改者   日期          描述
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

    //处理消息的三个步骤. 消息处理前的预处理,消息处理,消息处理后
    virtual XVOID       PrePrcMsg(CMsg*  &pMsg);
    virtual EPrcResult ProcessMsg(CMsg* pMsg) = 0;
    virtual XVOID       PostPrcMsg(CMsg* &pBuf);
    virtual EerrNo      Create();
    virtual EerrNo      Destroy();

    //存储和HOLD的消息队列,
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
        //将消息加到尾巴上
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

    //要求从中间释放
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

    //打印信息接口
    virtual XVOID Print(CLI_ENV* pCliEnv);

public:
    //消息操作的委托接口
    XVOID GetFsmAddr(CFsmAddr& addr)const;
    CMsg *GetMsgBuf(XU32 uiFullLen);
    XVOID FreeMsgBuf(CMsg* pBuf);
    EerrNo SendMsg(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri = NRML_SEND);

public:
    //定时器操作的接口
    //创建定时器.当定时器超时后回向创建该定时器的IFsm对象发送一条消息
    inline XVOID CreateTimer(CTimerHand& refTId ,TimerType timerType = TIMER_TYPE_LOOP,XPOINT ulUserPara = 0,XPOINT ulUserExtPara = 0);
    //删除定时器.
    inline XVOID StopTimer(const CTimerHand& refTId)const;
    inline XVOID DeleteTimer(CTimerHand& refTId);
    //启动毫秒定时器
    inline XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount);
    //启动秒定时器
    inline XVOID StartSTimer(const CTimerHand& tid, XU32 Second);

protected:
    //在调用设置状态机状态之前被调用的函数
    virtual  XVOID PreSetState(Tstate newState);
    virtual  XVOID PreSendMsgProc(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri);

private:
    CMsg* GetMsgFormList(CMsgLink& refLink)
    {
        if(refLink.empty())
        {
            return XNULL;
        }
        //从头上取得消息
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

    CMsgLink        m_save;   //save消息队列,有些消息当前状态不能处理
    CMsgLink        m_hold;   //Hold消息队列,有些消息当前状态不能处理
};


/*********************************************************************
名称 : class IFactory
职责 : 管理类型相同的从IFsm派生的对象
协作:
历史 :
       修改者   日期          描述
**********************************************************************/
class IFactory
{
    friend class CTimerMgr;
    friend class CMsgBox;

public:
    //id 模块ID, usMaxTimerNum 最大定时器个数, maxNum 最大状态机个数
    IFactory(MDID id, XU16 usMaxTimerNum, XU32 maxNum, bool needRandom = true);
    virtual ~IFactory();

    virtual EerrNo OnCreate();
    virtual EerrNo OnNotice();

    //初始化函数,在构着函数调用后调用该函数
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
    //以下是消息相关方法
    //向消息泵中添加一条消息,必须添加到尾端
    XVOID AddToTail(CMsg* p)
    {
        m_CMsgPump.push_back(p);
    }
    XVOID AddToHead(CMsg* p)
    {
        m_CMsgPump.push_front(p);
    }

    //任务入口函数
    static XS8 sMsgEntry(XVOID* pCpsMsg,XVOID *pObj);
    EPrcResult ProcessMsg(CMsg* pMsg);
    XVOID SaveMsgPrc(IFsm *pFsm);
    virtual XVOID SpecialProc(XVOID*& pCpsMsg) {};

    virtual bool CanPrcMsg(CMsg *pMsg);
    virtual EPrcResult FsmIdIsZeroPrc(CMsg *pMsg, IFsm *& rpFsm) = 0; //处理FsmId=0消息
    virtual EPrcResult RouteMsgErr(CMsg *pMsg); //Fsm找不到错误消息处理

    //消息管理的委托的接口
    CMsg *GetMsgBuf(XU32 uiFullLen);
    virtual XVOID PreFreeMsgBuf(CMsg* pBuf);
    XVOID FreeMsgBuf(CMsg* pBuf);
    virtual EerrNo SendMsg(CMsg* pBuf,const CFsmAddr& refTarget,ESendPrio pri = NRML_SEND);
    XVOID TransferMsg(CMsg* pBuf); //紧急转发

public:
    //以下是定时器管理相关方法
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
    XVOID StartMSTimer(const CTimerHand& tid, XU32 uimsCount) //启动毫秒定时器
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->StartMSTimer(tid, uimsCount);
    }
    XVOID StartSTimer(const CTimerHand& tid, XU32 Second) //启动秒定时器
    {
        CPP_ASSERT_RN(XNULL != m_pTimer);
        m_pTimer->StartSTimer(tid, Second);
    }

public:
    //以下是资源池相关方法
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
    //以下是调试信息方法
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
    //禁止拷贝和赋值
    FORBID_COPY_ASSIGN(IFactory);

protected:
    MDID            m_Id;          //FactMgr模块ID

    XU32            m_uiMaxFsm;    //同时处理一条消息的最大状态机数不能超过FSM_NUM_IN_FACTMGR(255)个(多播的情况)
    IFsm*           m_apFsm_list[FSM_NUM_IN_FACMGR]; //临时保存状态机对象,处理save消息用
    IFsmPool        m_IFsmPool;    //IFSM派生类型对象的缓冲池

    CMsgLink        m_CMsgPump;    //消息泵
    CMsgBox*        m_pMsgBox;     //消息处理类对象

private:
    CTimerMgr*      m_pTimer;      //定时器管理类对象
    CSimpleCounter  m_oprCounter;  //调试计数信息
};

/*********************************************************************
名称 : class CMsgBox
职责 : 封装平台消息的分配,发送和释放机制
       1.在同一模块内部发送消息的话，不用分配堆中的消息
       2.转发的消息。
协作 :
历史 :
       修改者   日期          描述
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
    //uiFullLength,消息的完整长度, 包括XOS消息头
    CMsg* MallocMsgBuf(XU32 uiFullLength);
    XVOID  FreeMsgBuf(CMsg *pMsg);
    EerrNo SendMsg(CMsg*& pMsg,ESendPrio iFlag = NRML_SEND);//默认投递到消息队列的尾部
    EerrNo TransferMsg(CMsg* pMsg);//紧急发送，就是投递到消息队列的头

private:
    //是模块内部的消息, 即任务内部对象之间的消息发送
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
    //禁止拷贝和赋值
    FORBID_COPY_ASSIGN(CMsgBox);

private:
    IFactory* m_pFM;
};




///////////////////////////////////////////////////
//类CTimerHand内联函数定义
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
//类CTimerMgr内联函数定义
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
//类IFactoryMgr内联函数定义
///////////////////////////////////////////////////
inline CMsg * IFactory::GetMsgBuf(XU32 uiFullLen)
{
    CMsg* pMsg = m_pMsgBox->MallocMsgBuf(uiFullLen);
    if(pMsg != XNULL)
    {
        //设置发送者的块ID
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
//类IFsm内联函数定义
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
函数名称 : MallocMsgBufOuterToFrame
功能描述 : 框架之外的实体向框架之中的对象发送消息要调用该接口分配缓存

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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

