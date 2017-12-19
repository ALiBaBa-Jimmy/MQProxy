/*----------------------------------------------------------------------
    created:    2004/09/29
    created:    29:9:2004   9:27
    filename:
    file path:
    file base:
    file ext:   c
    author:

    purpose:
----------------------------------------------------------------------*/

#ifdef  __cplusplus
    extern  "C"{
#endif

#include "xosshell.h"
//#include "saap_def.h"
#include "tcpmain.h"
#include "tcpoamproc.h"
#include "tcproutertable.h"
#include "syshash.h"
#include "../../saap/inc/saappublichead.h"

XS32 gTcpeReouteEnable  =0;
XS32 gtestInOneHost = 0;
/*20090722 add below*/
TCPE_USER_FID_T g_tcpe_UserFID[MAX_TCPE_USER_NUM];
/*20090722 add above*/
t_XOSFIDLIST g_TcpeFidInfo = { {"FID_TCPE",  XNULL, FID_TCPE}, {TCP_InitProc , XNULL, XNULL},  {TCP_MsgProc,  TCP_TimeoutProc},  eXOSMode, XNULL};

/*TCP收发消息统计*/
// 外部引用信令点表，此表在专用的信令点表文件中定义
extern TCP_DSTID_INDEX_IP_CCB   *gTCPDstIDToIPTbl;
extern HASH_TABLE                gTCPDstIDToIPHashTbl;

// 外部引用链路表
extern TCP_LINT_CCB_INDEX       *g_pstTCP ;

//组包时的中间的临时缓冲
XU8  *g_pDatainTmpBuff = XNULL ;

/* 临时表结构 */
XU8 g_tcpemsgbuff[SAAP_MAX_MSG_LEN];
int gMaxTcpeLinkTimes = 3; // 最大拨手次数
/**********************************
函数名称    : Tcp_Init
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    :
返回值      : XS32
************************************/
XS32 Tcp_Init( )
{
    XS32 i = 0;
    // 初始化跟踪模式
    //g_ucTCPMonitorMondel = 0 ;

    // 默认发送链路握手消息
    g_Tcp_LinkHand = 1;

    //部分trace消息默认关闭
    g_Tcp_Trace =0;

    // 分配内存
    g_pDatainTmpBuff = (XU8 *)XOS_MemMalloc(FID_TCPE, 2 * MAX_IP_DATA_LEN);
    if(XNULL == g_pDatainTmpBuff)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe Init failed.");
        return XERROR;
    }
    //20100817 cxf add
    for(i =0; i < MAX_TCPE_USER_NUM; i++)
    {
        g_tcpe_UserFID[i].userType = 0;
        g_tcpe_UserFID[i].userFid = 0;
    }
    g_tcpe_UserFID[0].userType = SAAP_USER_TYPE;
    g_tcpe_UserFID[0].userFid = FID_SAAP;
    return XSUCC;

}

/**********************************
函数名称    : TCP_InitProc
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    : 功能块初始化函数
参数        : XVOID *Para1
参数        : XVOID *Para2
返回值      : XS8
************************************/
XS8 TCP_InitProc(XVOID *Para1, XVOID *Para2 )
{
    int i  = 0;
    
    if(XSUCC != tcpe_link_inittable())
    {
         return XERROR;
    }
    if(XSUCC !=TCP_init_DstID_table())
    {
        return XERROR;
    }

    //命令行注册
    TCP_command_init();

    if(XSUCC !=XOS_TimerReg(FID_TCPE,500,TCP_REG_TIMER_NUM*2 + 2,0))
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe init,reg timer fail.");
    }

    for(i = 0; i< MAX_TCPEN_LINK_NUM; i++)
    {
        SAAP_StartTimer(&g_pstTCP->lintCcb[i].htTm,FID_TCPE, TCP_TWO_SECOND, TCP_HAND_TIMER,i,TIMER_TYPE_LOOP);
    }
        
    //初始化全局变量
    if(XSUCC != Tcp_Init())
    {
       return XERROR;
    }

    //注册OAM
    TcpTblRegToOAM(FID_TCPE);
    return XSUCC;
}

/**********************************
函数名称    : TCP_MsgProc
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    :
   tcpe 任务的消息处理函数只接收两类数据
   1.从NTL来的通讯消息
   2.从saap来的业务数据请求消息
   3.其它类型消息不处理.
   备注:数据结构t_XOSCOMMHEAD 与 COMMON_HEADER_SAAP 等价

参数        : t_XOSCOMMHEAD *pxosMsg
参数        : XVOID *Para
返回值      : XS8
************************************/
XS8 TCP_MsgProc(XVOID *msg, XVOID *Para)
{
     t_XOSCOMMHEAD *pxosMsg=XNULL;
     COMMON_HEADER_SAAP *pstMsg = XNULL;
     if(XNULL == msg)
     {
         return XERROR;
     }
     pxosMsg=(t_XOSCOMMHEAD*)msg;
     pstMsg = (COMMON_HEADER_SAAP *)g_tcpemsgbuff;

     //如果是NLT上来的数据指示,则要对消息进行copy,属于NTL消息的预处理过程
     if(FID_NTL == pxosMsg->datasrc.FID && eDataInd == pxosMsg->msgID)
     {
         if(XSUCC != SYS_XOSMsg2CpsMsg(pxosMsg, pstMsg) )
         {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe msg proc,trans from xos ntl data in msg to cps msg failed.");
            return XERROR;
         }
     }
     else
     {
         //如果XOS平台修改了消息头,COMMON_HEADER结构应做相应修改
         pstMsg = (COMMON_HEADER_SAAP *)pxosMsg;
     }

     //ntl消息的处理
     if(FID_NTL == pstMsg->stSender.ucFId)
     {
         Tcp_ntlmsg_proc(pstMsg);
         return 0;
     }

     //对于业务消息应该只有DATA REQ及CFG类消息
     switch(pstMsg->usMsgId)
     {
        case SAAP_DATA_REQ:
            TCP_Svc2Tcp_DataReqProc( pstMsg , TCP_INVIKE_LOCAL);
            break;

//20080928下面逻辑修改,屏蔽下面处理逻辑,没有此功能,属于老平台
//        case MSG_IP_CONFIG: //用子消息来区分是哪种配置消息
//            pstMsg->usMsgId = pxosMsg->subID;
//            tcp_OAM_proc( pstMsg );
//            break;
        default:
           if(g_Tcp_Trace)
           {
             XOS_Trace(MD(FID_TCPE,PL_MIN),"tcpe msg proc,unsupport msg type %d coming.",pstMsg->usMsgId);
           }
           break;
     }

     return 0;
}
void Tcp_PacketStat(XU16 link_Index,XU32 stattype)
{
#ifdef SAG_PERFORM_STAT_INCLUDE
    if( link_Index < TCP_LINT_MAX )
    {
       switch(stattype)
       {
         case eSendMsg:
              gTcpeStaData.msgStat[link_Index].ulSendMsgCnt++;
              break;
         case eRecvErrMsg:
              gTcpeStaData.msgStat[link_Index].ulRecvErrMsgCnt++;
              break;
         case eSendErrMsg:
              gTcpeStaData.msgStat[link_Index].ulSendErrMsgCnt++;
              break;
         case eRecvMsg:
              gTcpeStaData.msgStat[link_Index].ulRecvMsgCnt++;
              break;
         case eLinkTry:
              gTcpeStaData.msgStat[link_Index].ulLinkTryCnt++;
              break;
         case eLinkHand:
              gTcpeStaData.msgStat[link_Index].ulLinkHand++;
              break;
         case eLinkHandAck:
              gTcpeStaData.msgStat[link_Index].ulLinkHandAck++;
              break;
         case eLinkLose:
              gTcpeStaData.msgStat[link_Index].ulLinkLoseCnt++;
              break;
         case eLinkShutStop:
              gTcpeStaData.msgStat[link_Index].ulLinkShutStop++;
              break;
         default:
              gTcpeStaData.msgStat[link_Index].ulOther++;
              break;
       }
    }else
    {
       switch(stattype)
       {
         case eSendMsg:
              gTcpeStaData.msgIn.ulSendMsgCnt++;
              break;
         case eRecvErrMsg:
              gTcpeStaData.msgIn.ulRecvErrMsgCnt++;
              break;
         case eSendErrMsg:
              gTcpeStaData.msgIn.ulSendErrMsgCnt++;
              break;
         case eRecvMsg:
              gTcpeStaData.msgIn.ulRecvMsgCnt++;
              break;
         case eLinkTry:
              gTcpeStaData.msgIn.ulLinkTryCnt++;
              break;
         case eLinkHand:
              gTcpeStaData.msgIn.ulLinkHand++;
              break;
         case eLinkHandAck:
              gTcpeStaData.msgIn.ulLinkHandAck++;
              break;
         case eLinkLose:
              gTcpeStaData.msgIn.ulLinkLoseCnt++;
              break;
         case eLinkShutStop:
              gTcpeStaData.msgIn.ulLinkShutStop++;
              break;
         default:
              gTcpeStaData.msgIn.ulOther++;
              break;
       }
    }
#endif
}
/*20090722 add below*/
/********************************** 
函数名称    : tcpe_setfid_byusertype
作者        : Jeff.Zeng
设计日期    : 2009年7月22日
功能描述    : 
输入参数    : XS32 userType 用户请求消息类型
参数        : XS32 userFid  用户请求消息类型路由FID
返回值        : XS32 
            : XSUCC:成功/XERROR:失败
************************************/
XS32 tcpe_setfid_byusertype(XS32 userType,XS32 userFid)
{
    XS32 i  =0;
    if(0 == userFid)
    {
        return XERROR;
    }
    for(i=0; i< MAX_TCPE_USER_NUM; i++)
    {
        if(0 == g_tcpe_UserFID[i].userFid) //cb not use ,20100817 cxf modify
        {
            g_tcpe_UserFID[i].userType = userType;
            g_tcpe_UserFID[i].userFid = userFid;
            return XSUCC;
        }
    }     
    return XERROR;
}


/********************************** 
函数名称    : tcpe_getfid_byusertype
作者        : Jeff.Zeng
设计日期    : 2009年7月22日
功能描述    : 
参数        : XS32 UserType 直连模型的消息类型得到FID
返回值        : XS32 XSUCC:成功/XERROR:失败
************************************/
XS32 tcpe_getfid_byusertype(XS32 UserType)
{
    XS32 i  =0;

    for(i =0; i < MAX_TCPE_USER_NUM; i++)
    {
        if(UserType == g_tcpe_UserFID[i].userType)
        {
            return g_tcpe_UserFID[i].userFid;
        }
    }
    return 0;
}
/*20090722 add above*/
/**********************************
函数名称    : Tcp_ntlmsg_proc
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    : 功能块的通讯消息处理入口函数
参数        : COMMON_HEADER_SAAP *pstMsg
返回值      : XS32
************************************/
XS32 Tcp_ntlmsg_proc(COMMON_HEADER_SAAP *pstMsg)
{
    XS32 ulLinkIndex = 0;
    t_LINKINITACK  * pLinkInitAck  = XNULL;
    t_STARTACK     * pLinkStartAck = XNULL;
    t_SENDERROR    * pSendErrMsg   = XNULL;
    t_LINKCLOSEIND * pLinkStopInd  = XNULL;
    t_CONNIND      * pLinkConnInd  = XNULL;
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_MIN),"enter tcpe ntlmsg_proc.");
    }
    switch(pstMsg->usMsgId)
    {
        case eInitAck:
            pLinkInitAck = (t_LINKINITACK *)(pstMsg+1);
            return tcpe_link_initack(pLinkInitAck);
            //break;

        case eStartAck:
            pLinkStartAck = (t_STARTACK *)(pstMsg+1);
            return tcpe_link_startack(pLinkStartAck);
            //break;

        case eErrorSend:
            pSendErrMsg = (t_SENDERROR *)(pstMsg+1);
            return tcpe_link_errorsend(pSendErrMsg);
            //break;

        case eStopInd:
            pLinkStopInd = (t_LINKCLOSEIND *)(pstMsg +1);
            ulLinkIndex = (XS32)pLinkStopInd->appHandle;
            //统计链路被关闭次数
            Tcp_PacketStat((XU16)ulLinkIndex,eLinkShutStop);
            if(ulLinkIndex >= MAX_TCPEN_LINK_NUM)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"link index err when rev eStopInd.");
                return XERROR;
            }
            XOS_Trace(MD(FID_TCPE,PL_WARN),"link(%d) rev eStopInd from ntl.",ulLinkIndex);

            //告警
            if(TCP_LINK_OK == g_pstTCP->lintCcb[ulLinkIndex].ucHandState)
            {
                //如果当前状态是已经建立,收到链路停止指示,向网管告警
                tcpe_link_alarm(TCPE_TCP_ALARM_STATE_DISC,ulLinkIndex);
            }
            //设置链路状态
            tcpe_link_setstatus( ulLinkIndex,TCPE_TCP_LINK_STATE_STOP);
            g_pstTCP->lintCcb[ulLinkIndex].ucHandState = TCP_LINK_FAIL;

            //清除tcpe缓存
            tcpe_link_clearbuff(ulLinkIndex);

            //停止定时器
            //XOS_TimerStop(FID_TCPE,g_pstTCP->lintCcb[ulLinkIndex].htTm); //20110909 cxf del
            break;

        case eConnInd:
            pLinkConnInd = (t_CONNIND *)(pstMsg +1);
            ulLinkIndex = (XS32)pLinkConnInd->appHandle;
            if(ulLinkIndex >= MAX_TCPEN_LINK_NUM)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"link index err when rev eConInd.");
                return XERROR;
            }
            XOS_Trace(MD(FID_TCPE,PL_INFO),"link(%d) rev eConnInd from Ntl.",ulLinkIndex);
            tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_BUILD);

            //只有客户端才启动握手定时器
            //20081205 modify below
            //g_pstTCP->lintCcb[ulLinkIndex].ucHandState   =  TCP_LINK_OK;
            //20081205 modify above

            /*20090911 modify below,服务器,客户端都启动握手程序*/
            //if(g_pstTCP->lintCcb[ulLinkIndex].ucModel == eTCPClient)
            //{
                //SAAP_StartTimer(&g_pstTCP->lintCcb[ulLinkIndex].htTm,FID_TCPE, TCP_TWO_SECOND, TCP_HAND_TIMER,ulLinkIndex,TIMER_TYPE_LOOP); // ??? cxf 20110908 del
            //}
            /*20090911 modify above*/
            XOS_Trace(MD(FID_TCPE,PL_INFO),"rev eConnInd of link(%d).",ulLinkIndex);
            break;

        case eDataInd:
            pstMsg->usMsgId = SAAP_DATA_IND;
            Tcp_ntlmsg_packetdecodeproc(pstMsg);
            break;//return XSUCC;

        default:
            return XSUCC;

    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out tcpe ntlmsg_proc.");
    }
    return XSUCC;
}

/**********************************
函数名称    :
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    : 此函式处理平台上行的数据指示
参数        :  COMMON_HEADER_SAAP *pstMsg
参数        : XU8 *MsgBuffer
返回值      : void
************************************/
void TCP_Tcp2Svc_DataIndProc( COMMON_HEADER_SAAP *pstMsg, XU8 *MsgBuffer )
{
    // 定义下行的消息BUF
    XU8    ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];
    TCP_AND_NEX_LAYER_INTERFACE     *pMsg ;     // 收到的包结构
    TCP_AND_PRO_LAYER_MSG_BUF       *pstTmp;    // 出去的包结构
    XU8   handMsg[sizeof(TCP_hand_msg)];
    XU8 *pBuf ;
    XU16  usccno, lintNo ;
    if(g_Tcp_Trace)
    {
        XOS_Trace(MD(FID_TCPE,PL_MIN),"enter TCP_Tcp2Svc_DataIndProc.");
    }
    pBuf   = (XU8*)MsgBuffer;
    pMsg   = (TCP_AND_NEX_LAYER_INTERFACE*)MsgBuffer ; // 转换成TCP/IP封装结构
    pstTmp = (TCP_AND_PRO_LAYER_MSG_BUF*)ucOutMsg ;

    // 路由节点次数比对
    pstTmp->tcpHead.RouterNum = XOS_NtoHs(pMsg->RouterNum ) ;

//#ifdef SAG_PERFORM_STAT_INCLUDE
    //通过源信令点找到映射表的下标,然后得到链路号
    usccno = TCP_SearchDstIDToIpTable( XOS_NtoHs(pMsg->GuestDeviceID) );
    if(usccno < TCP_MAX_IPROUTER_NUM)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }
//#endif

    // 0 被 -- 后为 0xfffff
    pstTmp->tcpHead.RouterNum -- ;
    if ( pstTmp->tcpHead.RouterNum  <= 0
        || pstTmp->tcpHead.RouterNum == BLANK_USHORT )
    {
        // 超过限定次数丢掉包
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_ERR),"router num become 0,discard the msg.");
        return ;
    }

    // 以下步骤完成包头转换
    pstTmp->tcpHead.DstDeviceID = XOS_NtoHs(pMsg->DstDeviceID) ;
    pstTmp->tcpHead.GuestDeviceID = XOS_NtoHs(pMsg->GuestDeviceID) ;
    pstTmp->tcpHead.UserType = XOS_NtoHs(pMsg->UserType);

    // 此处要减去 TCP/IP 封装包部分字段的长度
    pstTmp->tcpHead.MsgLen = XOS_NtoHs(pMsg->MsgLen) - SAAPANDTCP_HEAD_LEN;

    // 内存保护，长度检查
    if ( pstTmp->tcpHead.MsgLen > SAAP_MAX_MSG_LEN )
    {
        //20081016修改如下功能点,一起统计
        //if( lintNo < TCP_LINT_MAX - 2 )
        //{
           //Tcp_PacketStat(lintNo,eRecvErrMsg);
        //}
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe msg len(%x) is too long.",pstTmp->tcpHead.MsgLen );
        return ;
    }
    XOS_MemCpy( pstTmp->ucBuffer,  pBuf + sizeof(TCP_AND_NEX_LAYER_INTERFACE),  pstTmp->tcpHead.MsgLen  );

    pstTmp->com.usMsgLen =  pstTmp->tcpHead.MsgLen + sizeof(TCP_AND_PRO_LAYER_INTERFACE) ;
    pstTmp->com.usMsgId  =  pstMsg->usMsgId ; // SAAP_DATA_IND tcpmain.c line 414

    pstTmp->com.stReceiver =  pstMsg->stReceiver ;
    pstTmp->com.stSender   =  pstMsg->stSender ;

    //通过dpid获取基站索引
    if(usccno < TCP_LINT_MAX)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }

    // 比对是否是自已的信令点，如果不是，则为转接点，则直接出局
    // 如果信令点编码为 ffff , 则说明此模块为网管等没有信令点的模块，是本机信令，不用转接路由
    if(gTcpeReouteEnable) // 是否启用TCPE的路由功能
    {
        //备注:8888该分支不存在
        if (  g_pstTCP->GlobalDstID  != pstTmp->tcpHead.DstDeviceID
                &&  BLANK_USHORT !=  pstTmp->tcpHead.DstDeviceID
                 &&  0xfffd != pstTmp->tcpHead.DstDeviceID  )
        {
            // 如果是路由数据则直接返回, 这里是函数调用, ID 改为数据请求
            TCP_Svc2Tcp_DataReqProc( (COMMON_HEADER_SAAP*)pstTmp , TCP_INVIKE_ROUTE );
            return ;
        }
    }

    // 如果是握手消息，则进行握手
    if ( TCP_USERTYPE_HANDSHAKE == XOS_NtoHs(pMsg->UserType) )
    {
        XOS_MemSet( handMsg, BLANK_UCHAR, sizeof(TCP_hand_msg) );
        XOS_MemCpy( handMsg, pstMsg, sizeof(COMMON_HEADER_SAAP) );
        XOS_MemCpy( handMsg + sizeof(COMMON_HEADER_SAAP), MsgBuffer,  sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP) );
        TCP_hand_msg_proc( (COMMON_HEADER_SAAP*)handMsg );  //TCP握手
        return  ;
    }
    // 处理用户类型,这里将完成全局ID向内部ID的一次转换

    // 完成向业务层的发送

    // 20100817 cxf modify
    pstTmp->com.stReceiver.ucFId = tcpe_getfid_byusertype(pstTmp->tcpHead.UserType);
    if(0 == pstTmp->com.stReceiver.ucFId) // not find dstFid by userType
    {
        if(g_Tcp_Trace)
        {
            XOS_Trace(MD(FID_TCPE,PL_INFO),"unsupport msg for tcpe link data in usertype(0x%x).",pstTmp->tcpHead.UserType);
        }
        Tcp_PacketStat(lintNo,eRecvErrMsg);
        return;
    }
    pstTmp->com.stSender.ucFId  =  FID_TCPE ;
    SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTmp );
    Tcp_PacketStat(lintNo,eRecvMsg);
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out TCP_Tcp2Svc_DataIndProc FID_TCPE => %d.",pstTmp->com.stReceiver.ucFId);
    }
    return  ;

}

//review part ok
/**********************************
函数名称    : TCP_Svc2Tcp_DataReqProc
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    :  业务下行平台的数据请求包处理
参数        :  COMMON_HEADER_SAAP *pstMsg
参数        : XU8 flag
返回值      : void
************************************/
void TCP_Svc2Tcp_DataReqProc( COMMON_HEADER_SAAP *pstMsg , XU8 flag )
{
    XU16 shLen ;
    XU16 usccno=TCP_ERROR_NUM, lintNo=TCP_ERROR_NUM;
    XU16  msgout_sumidx=BLANK_USHORT;
    // 定义下行的消息BUF
    XU8    ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];

    TCP_AND_PRO_LAYER_MSG_BUF       *pMsg ;
    TCP_AND_NEX_LAYER_MSG_BUF       *pstTmp;

    COMMON_MESSAGE_SAAP *pstReturnMsg ;  // 链路出错后向用户层返回
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN), "Enter TCP Svc2Tcp_DataReqProc");
    }
    Tcp_PacketStat(msgout_sumidx,eSendMsg);
    pMsg = ( TCP_AND_PRO_LAYER_MSG_BUF* )pstMsg ;  // 将收到的包转换成本层的应用协议

    // 处理数据类包
    pstTmp = (TCP_AND_NEX_LAYER_MSG_BUF *)ucOutMsg;

    // 包头标致
    pstTmp->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG ;
    pstTmp->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG ;

    // 这一步完成用户数据的封装
    // 用户数据的长度
    shLen = pMsg->tcpHead.MsgLen;
    usccno = TCP_SearchDstIDToIpTable(pMsg->tcpHead.DstDeviceID);//8888 字节序与收不同
    if(usccno < TCP_MAX_IPROUTER_NUM)
    {
        lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex;
    }

    // 内存保护
    if ( shLen > SAAP_MAX_MSG_LEN )
    {
        Tcp_PacketStat(lintNo,eSendErrMsg);
        XOS_Trace(MD(FID_TCPE,PL_WARN),"TCP Svc2Tcp_DataReqProc,msg len err.");
        return ;
    }
    XOS_MemCpy(pstTmp->ucBuffer,  pMsg->ucBuffer, shLen );

    if( TCP_INVIKE_LOCAL == flag)
    {
        // 如果是本地调用, 打上自已的信令点编码
        pstTmp->tcpHead.GuestDeviceID = XOS_HtoNs( g_pstTCP->GlobalDstID );
    }
    else
    {
        // 路由调用，源信令点不变
        pstTmp->tcpHead.GuestDeviceID = XOS_HtoNs( pMsg->tcpHead.GuestDeviceID );
        pstTmp->com.usMsgId = SAAP_DATA_REQ ;// 消息改为数据请求
    }

    // 处理用户类型,这里将完成内部ID向全局ID的一次转换
    pstTmp->tcpHead.UserType =  XOS_HtoNs(pMsg->tcpHead.UserType) ;

    // 处理路由次数,初始化时为最大值
    pstTmp->tcpHead.RouterNum  =  XOS_HtoNs( pMsg->tcpHead.RouterNum ) ;

    shLen = shLen + sizeof(TCP_AND_NEX_LAYER_INTERFACE);

    // 处理长度, 这里的长度要减去 head falg len
    pstTmp->tcpHead.MsgLen = XOS_HtoNs( (XU16)(shLen - 2) );

    // 处理包尾
    ucOutMsg[shLen + sizeof(COMMON_HEADER_SAAP)] = TCP_CURR_FLAG ;
    ucOutMsg[shLen + sizeof(COMMON_HEADER_SAAP) + 1] = TCP_END_FLAG ;
    shLen = shLen + 2 ;  // buf 总长

    // 根据目的信令点编码与链路关系完成向平台的发送
    // 处理信令点编码, 这里使用网络字节发送
    if ( 0 ==  pMsg->tcpHead.DstDeviceID)
    {

        // 检查默认上一级节点是否合法
        usccno = g_pstTCP->ProIndex;
        if ( BLANK_USHORT == g_pstTCP->ProIndex  )
        {

            /*2009/05/19 add below for saap_senderr_rsp process,submsg is src fid*/
            pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
            pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
            pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;
            pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR+1 ;
            pstReturnMsg->stHeader.subID   =       pstMsg->subID;
            pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
            pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;
            SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
            /*2009/05/19  add above for saap_senderr_rsp process*/

            //9999 是否统计发送失败,20081016添加发送失败统计
            Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
            XOS_Trace(MD(FID_TCPE,PL_ERR),"DstDeviceId =0,but can not find default link,can not send data.");
            return ;
        }
        // 否则 链路号就是 默认值
        lintNo = usccno ;

        // 0 代表GT要使用默认链路
        pstTmp->tcpHead.DstDeviceID = XOS_HtoNs(g_pstTCP->lintCcb[usccno].DstDeviceID) ;

    }
    else
    {
        pstTmp->tcpHead.DstDeviceID = XOS_HtoNs(pMsg->tcpHead.DstDeviceID);
        usccno = TCP_SearchDstIDToIpTable(pMsg->tcpHead.DstDeviceID);
        if ( TCP_ERROR_NUM != usccno )
        {
            // 如果 HASH 表中有 DPID ，则取出表里面的链路号索引值
            lintNo = gTCPDstIDToIPTbl[usccno].usLintIndex ;
        }
        else
        {
            // 否则 链路号就是 默认值
            usccno = g_pstTCP->ProIndex ;
            lintNo = usccno ;
            if ( BLANK_USHORT == g_pstTCP->ProIndex  )
            {
                /*2009/05/19 add below for saap_senderr_rsp process,submsg is src fid*/
                pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
                pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
                pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;
                pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR+1 ;
                pstReturnMsg->stHeader.subID   =       pstMsg->subID;
                pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
                pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;
                SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
                /*2009/05/19  add above for saap_senderr_rsp process*/

                //9999 是否统计发送失败,20081016添加发送失败统计
                Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
                XOS_Trace(MD(FID_TCPE,PL_ERR),"can not find link of DstIp(%x) and  without default link,can not send data.",pMsg->tcpHead.DstDeviceID);
                return ;
            }
        }
    }

    // 如果握手打开,又是本地包,则检查链路握手失败，向本地应用层返回错误
    if( 1 == g_Tcp_LinkHand  && TCP_INVIKE_LOCAL == flag
                             &&  TCP_LINK_OK != g_pstTCP->lintCcb[lintNo].ucHandState  )
    {
        pstReturnMsg = (COMMON_MESSAGE_SAAP *)ucOutMsg;
        pstReturnMsg->stHeader.stReceiver =   pstMsg->stSender ;
        pstReturnMsg->stHeader.stSender   =   pstMsg->stReceiver ;

        pstReturnMsg->stHeader.usMsgId   =    TCP_HAND_ERR ;
        /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
        pstReturnMsg->stHeader.subID   =       pstMsg->subID;
        XOS_Trace(MD(FID_TCPE,PL_INFO),"tcpe linkerr send rsp subID %d.fsmid %d",pstReturnMsg->stHeader.subID,pstReturnMsg->stHeader.stReceiver.usFsmId);
        /*2009/04/28 add above for saap_senderr_rsp process*/
        pstReturnMsg->stHeader.usMsgLen   =   0x01 ;
        pstReturnMsg->ucBuffer[0]         =   TCP_LINK_FAIL ;    // 链路错，错误值待定

        // 链路错误，回应失败
        SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstReturnMsg );
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe's hand shake state is fail,can not send data.");
        Tcp_PacketStat(lintNo,eSendErrMsg);
        return ;
    }
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_DBG),"use link(%d) to send data.",lintNo);
    }
    if ( lintNo > TCP_LINT_MAX-1 )
    {
        /* 此处无法按链路号统计,因为链路号已经超出了范围了 */
        XOS_Trace(MD(FID_TCPE,PL_ERR),"link index (%d) err.",lintNo);
        //9999 是否统计发送失败,20081016添加发送失败统计
        Tcp_PacketStat(msgout_sumidx,eSendErrMsg);
        return ;
    }

    // 完成包头包装

    pstTmp->com.stReceiver.ucModId = XOS_GetLocalPID();
    pstTmp->com.stReceiver.ucFId= FID_NTL;
    pstTmp->com.stReceiver.usFsmId =(XU16)g_pstTCP->lintCcb[lintNo].ulLinkHandle;

    pstTmp->com.stSender.ucModId = XOS_GetLocalPID();
    pstTmp->com.stSender.ucFId = FID_TCPE;
    pstTmp->com.stSender.usFsmId = lintNo;

    pstTmp->com.usMsgId = pstMsg->usMsgId;

    pstTmp->com.usMsgLen  =  shLen ;

    SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTmp );
    Tcp_PacketStat(lintNo,eSendMsg);
    return ;
}

////////////////////////////////////////////////////////////////////////
//  以下为辅助函数类
////////////////////////////////////////////////////////////////////////

// 解包组包处理函数，此函数从IP流中完成业务包的分立
// 此函数注译的 error 处，是用于单元测试检测错误的点
/**********************************
函数名称    :
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    :
参数        : COMMON_HEADER_SAAP *pstMsg
返回值      : void
************************************/
void Tcp_ntlmsg_packetdecodeproc( COMMON_HEADER_SAAP *pstMsg )
{
    COMMON_MESSAGE_SAAP * pMsg ;
    XU16 usNo  , usLen ,  bufLen  ;
    XU16  i , usbufLen;
    XU16  msgin_sumidx=TCP_ERROR_NUM;
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"enter tcpe ntlmsg_packetdecodeproc.");
    }

    Tcp_PacketStat(msgin_sumidx,eRecvMsg);
    //备注:tcpe 在处理通讯消息时,将链路号usNo作为子消息的参数值
    usNo =  pstMsg->stReceiver.usFsmId ;
    if ( usNo > TCP_LINT_MAX  )
    {
        //补充统计888
        Tcp_PacketStat(msgin_sumidx,eRecvErrMsg);
        //XOS_Trace(MD(FID_TCPE,PL_ERR),"TCPE ntlDataInd_DecodeProc,link index(%d) > max(%d) ",usNo,TCP_LINT_MAX);
        return ;
    }

    pMsg = (COMMON_MESSAGE_SAAP * )pstMsg;

    // 得到包长，并检查长度
    usLen = pMsg->stHeader.usMsgLen ;
    if( usLen > MAX_IP_DATA_LEN || usLen <= 0 )
    {
        Tcp_PacketStat(usNo,eRecvErrMsg);
        return ;
    }

    // 清空全局缓冲
    XOS_MemSet( g_pDatainTmpBuff ,0x0, MAX_IP_DATA_LEN * 2 );

    // 得到缓冲中包的长度,并拼接
    bufLen = g_pstTCP->lintCcb[usNo].bufLen;
    if( bufLen > 0 )
    {
        if ( bufLen + usLen >=  MAX_IP_DATA_LEN * 2 )  // 内存保护
        {
           Tcp_PacketStat(usNo,eRecvErrMsg);
           return ;
        }
        // 清空中间BUF
        XOS_MemCpy( g_pDatainTmpBuff,&g_pstTCP->lintCcb[usNo].ucBuffer[0], bufLen );
        XOS_MemCpy(&g_pDatainTmpBuff[bufLen], pMsg->ucBuffer, usLen );

        // 得到新的长度信息
        usLen = usLen +  bufLen;
    }
    else
    {
        // 直接 copy BUF
        XOS_MemCpy( g_pDatainTmpBuff, pMsg->ucBuffer, usLen );
    }

    // 清空资源
    //只要完成组包，则清 0
    g_pstTCP->lintCcb[usNo].bufLen = 0 ;
    XOS_MemSet( &g_pstTCP->lintCcb[usNo].ucBuffer[0],0x0,MAX_IP_DATA_LEN );

    // 包头定位实步，重定位处理, 此处失步重定位前的包将被丢弃
    for( i = 0 ;  i  < usLen ; i++  )
    {
        if ( 0 == i )
        {
            if ( usLen - i <= SAAPANDTCP_HEAD_LEN)//9999 算法提前,不用每次比较
            {
                // 比对一下长度，如果长度小于包头，则直接等待下一包的到来
                XOS_MemCpy( &g_pstTCP->lintCcb[usNo].ucBuffer[0], &g_pDatainTmpBuff[i], usLen - i );
                g_pstTCP->lintCcb[usNo].bufLen = usLen - i ;
                Tcp_PacketStat(usNo,eOtherStat);
                return ;
            }
        }

        if(  TCP_CURR_FLAG == g_pDatainTmpBuff[ i ]  &&
             TCP_HEAD_FLAG == g_pDatainTmpBuff[ i + 1 ] )
        {
            usbufLen = XOS_NtoHs(*(XU16*)&g_pDatainTmpBuff[ i + 2 ] );

            // 内存保护,超长包
            if( usbufLen > MAX_IP_DATA_LEN - 100 )
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe's bufLen(%d) too long,now support max %d len,ingore it.",usbufLen,MAX_IP_DATA_LEN - 100 );
                Tcp_PacketStat(usNo,eRecvErrMsg);
                return ;
            }

            //全局缓冲区中至少有一个完整的数据包
            if( i + 4 + usbufLen <= usLen   )   //usbufLen应该包括这2字节
            {
                if(  TCP_CURR_FLAG == g_pDatainTmpBuff[ i + 2 + usbufLen ]  &&
                     TCP_END_FLAG  == g_pDatainTmpBuff[ i + 2 + usbufLen + 1 ] )
                {
                   // 业务数据包找到，转至业务处理
                    TCP_Tcp2Svc_DataIndProc(pstMsg, (XU8*)&g_pDatainTmpBuff[i] );
                    //下面加3跟TCP ip_to_tcp_DataInd的处理有关,其数据结构中包括3个辅助字节
                    i = i  +  usbufLen + 3 ;   // 循环对 i 加了一次，这里只加 3
                }
                else
                {
                    //8888 这个处理逻辑有误,三次收不到结束标识,做为出错报统计吗?
                    Tcp_PacketStat(usNo,eRecvErrMsg);
                }

            }
            else
            {
                XOS_MemCpy( &g_pstTCP->lintCcb[usNo].ucBuffer[0], &g_pDatainTmpBuff[i], usLen - i );
                g_pstTCP->lintCcb[usNo].bufLen = usLen - i ;
                Tcp_PacketStat(usNo,eOtherStat);
                //数据不完整,继续接收,
                return ;
            }

        }
        else
        {
            Tcp_PacketStat(usNo,eRecvErrMsg);
        }
    }
    if(g_Tcp_Trace)
    {
      XOS_Trace(MD(FID_TCPE,PL_MIN),"out tcpe ntlmsg_packetdecodeproc.");
    }
}

/**********************************
函数名称    :
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    : 定时器消息处理函数
参数        : t_BACKPARA  *pstPara
返回值      : XS8 TCP_TimeoutProc
************************************/
XS8 TCP_TimeoutProc(t_BACKPARA  *pstPara)
{
    XS32 tmrName = 0;
    XS32 ulPara = 0;
    //XS32 ulLinkIndex  = 0;

    tmrName = pstPara->para2;
    ulPara = pstPara->para3;

    switch(tmrName)
    {
        case TCP_HAND_TIMER: // 握手定时器
            {
                Tcp_lint_scan( (XU16)ulPara );
            }
            break;
        default:
            if(g_Tcp_Trace)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"TCP_TimeoutProc,unknown timer msg.");
            }
            break;
    }
    return XSUCC;
}

/**********************************
函数名称    : Tcp_lint_scan
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    : 握手定时器处理，发送握手请求
参数        : XU16 Index
返回值      : void
************************************/
void Tcp_lint_scan(XU16 Index)
{
    XU8   ucOutMsg[sizeof(TCP_hand_msg) + 1];
    // 发出握手包结构
    TCP_hand_msg  *pstTcpHand ;

    if ( g_Tcp_LinkHand == 0 )
    {
        return ;
    }

    if(Index >= TCP_LINT_MAX )
    {
       XOS_Trace(MD(FID_TCPE,PL_WARN),"tcpe hand time out msg proc,input link index para %d is error",Index);
       return;
    }

    // ??? 20110908 cxf add
    if( LINT_CCB_NULL ==g_pstTCP->lintCcb[Index].ucFlag) // not config it ,return;
    {
        return;
    }
    //
    // 握手控制
    if( g_pstTCP->lintCcb[Index].hankCount >= gMaxTcpeLinkTimes/*MAX_TCPE_HAND_TIMES*/  )
    {
        //告警
        /*如果握手次数超过10次,并且处于连接的状态时,告警上报*/
        if( TCP_LINK_OK == g_pstTCP->lintCcb[Index].ucHandState)
        {
            tcpe_link_alarm(TCPE_TCP_ALARM_STATE_DISC,Index);
        }

        // 不通
        g_pstTCP->lintCcb[Index].ucHandState  = TCP_LINK_FAIL ;
        g_pstTCP->lintCcb[Index].hankCount    = 0 ;
        Tcp_PacketStat(Index,eLinkLose);

        //20081205 add below
        //如果底层处于连通状态而握手失败,则重建链路,
        //同时修改握手机制,需要等到握手响应成功后才置为正常TCP_LINK_OK,不应该是链路建立置TCP_LINK_OK
        // if( TCPE_TCP_LINK_STATE_BUILD == g_pstTCP->lintCcb[Index].ulLinkStatus  ) // ??? 20110908 cxf del
        {
           XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe link[%d] hand time out,rebuild it.",Index);

           //停止定时器
           //XOS_TimerStop(FID_TCPE,g_pstTCP->lintCcb[Index].htTm); //??? 20110909 cxf del

           //重建链路
           tcpe_link_release(Index);

           tcpe_link_init(Index);           
           return;
        }
        //20081205 add above
    }

    /*20110908 cxf add below */
    if( TCPE_TCP_LINK_STATE_BUILD != g_pstTCP->lintCcb[Index].ulLinkStatus) // not connected,no send shake hand msg
    {
        g_pstTCP->lintCcb[Index].hankCount ++;
        return;
    }
    /*20110908 cxf add above*/
    
    pstTcpHand = (TCP_hand_msg*) ucOutMsg ;
    //封装包头
    pstTcpHand->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG;
    pstTcpHand->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG;

    pstTcpHand->tcpHead.MsgLen        = XOS_HtoNs(sizeof(XU16) * 5 + sizeof(XU8) );
    pstTcpHand->tcpHead.DstDeviceID   = XOS_HtoNs(g_pstTCP->lintCcb[Index].DstDeviceID);
    pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(g_pstTCP->GlobalDstID);
    pstTcpHand->tcpHead.UserType      = XOS_HtoNs(TCP_USERTYPE_HANDSHAKE);
    pstTcpHand->tcpHead.RouterNum     = XOS_HtoNs(TCP_ROUTER_MAX_NUM);

    //消息类型,握手请求
    pstTcpHand->msgType               = TCP_hand_reg ;
    //封装包尾
    pstTcpHand->Tailbuff[0]           = TCP_CURR_FLAG;
    pstTcpHand->Tailbuff[1]           = TCP_END_FLAG;

    if( DEFAULT_TCPEN_MEDIA_LINKID == Index )
    {
        pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(0xfffd);
    }

    // 完成包头包装
    pstTcpHand->comHead.stSender.ucFId     = FID_TCPE;
    pstTcpHand->comHead.stSender.usFsmId   = Index;
    pstTcpHand->comHead.stReceiver.ucFId   = FID_NTL;
    pstTcpHand->comHead.stReceiver.usFsmId = g_pstTCP->lintCcb[Index].ulLinkHandle;
    pstTcpHand->comHead.usMsgId            = SAAP_DATA_REQ ;

    pstTcpHand->comHead.usMsgLen           = sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP); //后续长度之和
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_MIN),"tcp linkindex = %d,dstId = 0x%x,hand msg send out",(XU16)Index,pstTcpHand->tcpHead.DstDeviceID);
    }
    SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstTcpHand );

    g_pstTCP->lintCcb[Index].hankCount++ ;

}

/**********************************
函数名称    : TCP_hand_msg_proc
作者        : codeReview
设计日期    : 2008年10月8日
功能描述    :
参数        : COMMON_HEADER_SAAP *pstMsg
返回值      : void
************************************/
void TCP_hand_msg_proc( COMMON_HEADER_SAAP *pstMsg )
{
    XU16  srcLinkPos=TCP_ERROR_NUM , lintno=TCP_ERROR_NUM;
    XU16  srcDpId = 0;
    XU16  msghand_sumidx=TCP_ERROR_NUM;

    TCP_hand_msg  *pMsg ;
    TCP_hand_msg  *pstTcpHand ;
    XU8  ucOutMsg[sizeof(TCP_hand_msg) + 1];

    pMsg = (TCP_hand_msg *)pstMsg;

    switch( pMsg->msgType )
    {
        case TCP_hand_reg:
            {
                //tcp server process tcp client hand reg msg
                pstTcpHand = (TCP_hand_msg*)ucOutMsg ;

                pstTcpHand->tcpHead.ucHeadFlag7E  = TCP_CURR_FLAG;  //封装包头
                pstTcpHand->tcpHead.ucHeadFlagA5  = TCP_HEAD_FLAG;

                pstTcpHand->tcpHead.MsgLen        = XOS_HtoNs(sizeof(XU16) * 5 + sizeof(XU8));
                pstTcpHand->tcpHead.DstDeviceID   = pMsg->tcpHead.GuestDeviceID;

                pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(g_pstTCP->GlobalDstID);
                if(gtestInOneHost) // 为方便在本机上测试,
                {
                    pstTcpHand->tcpHead.DstDeviceID = pMsg->tcpHead.DstDeviceID;
                    pstTcpHand->tcpHead.GuestDeviceID = pMsg->tcpHead.DstDeviceID;
                }
                pstTcpHand->tcpHead.UserType      = XOS_HtoNs(TCP_USERTYPE_HANDSHAKE);
                pstTcpHand->tcpHead.RouterNum     = XOS_HtoNs(TCP_ROUTER_MAX_NUM);

                pstTcpHand->msgType               = TCP_hand_ack ;  //握手应答

                pstTcpHand->Tailbuff[0]           = TCP_CURR_FLAG;   // 包尾
                pstTcpHand->Tailbuff[1]           = TCP_END_FLAG;   // 包尾

                //根据源信令点编码与链路关系完成向平台的发送
                //因此,每个目的信令点只能配置一条链路888,否则链路查找会错位
                srcDpId = XOS_NtoHs(pMsg->tcpHead.GuestDeviceID);
                srcLinkPos = TCP_SearchDstIDToIpTable(srcDpId);
                if ( TCP_ERROR_NUM == srcLinkPos)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"rev hand msg,but can't find the SrcDpid(0x%x) link.",srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHand);
                    return ; // error
                }
                if(srcLinkPos < TCP_MAX_IPROUTER_NUM)
                {
                    lintno = gTCPDstIDToIPTbl[srcLinkPos].usLintIndex;
                }

                if ( lintno > TCP_LINT_MAX-1)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"link index(%d) err of dp(%x)",lintno,srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHand);
                    return;
                }

                //告警恢复
                if( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState)
                {
                    tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,lintno);
                }
                /*20090911 comment below,tcp client and tcp server hand both needed*/
                //g_pstTCP->lintCcb[lintno].hankCount =  0 ;
                //g_pstTCP->lintCcb[lintno].ucHandState = TCP_LINK_OK ;
                /*20090911 comment above,tcp client and tcp server hand both needed*/

                if( DEFAULT_TCPEN_MEDIA_LINKID == lintno )
                {
                    pstTcpHand->tcpHead.GuestDeviceID = XOS_HtoNs(0xfffd);
                }

                Tcp_PacketStat(lintno,eLinkHand);
                // 完成包头包装
                pstTcpHand->comHead.stReceiver.ucFId = FID_NTL;
                pstTcpHand->comHead.stSender.ucFId = FID_TCPE;
                pstTcpHand->comHead.stReceiver.usFsmId = g_pstTCP->lintCcb[lintno].ulLinkHandle;
                pstTcpHand->comHead.stSender.usFsmId = lintno;
                pstTcpHand->comHead.usMsgId            = SAAP_DATA_REQ ;
                //后续长度之和
                pstTcpHand->comHead.usMsgLen           = sizeof(TCP_hand_msg) - sizeof(COMMON_HEADER_SAAP);

                SYS_MSGSEND((COMMON_HEADER_SAAP *)pstTcpHand );

            }
            break;
        case TCP_hand_ack:  // 处理握手ACK
            {
                pstTcpHand = (TCP_hand_msg*)pstMsg ;

                //找索引
                srcDpId = XOS_NtoHs(pMsg->tcpHead.GuestDeviceID);
                srcLinkPos  =  TCP_SearchDstIDToIpTable( XOS_NtoHs(pstTcpHand->tcpHead.GuestDeviceID) );

                if ( TCP_ERROR_NUM == srcLinkPos)
                {
                    Tcp_PacketStat(msghand_sumidx,eLinkHandAck);
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"rev hand ack,but can not find the link of srcDpId(%x)",srcDpId);
                    return ; // error
                }
                // 得到链路号
                if(srcLinkPos < TCP_MAX_IPROUTER_NUM)
                {
                    lintno = gTCPDstIDToIPTbl[srcLinkPos].usLintIndex;
                }
                if ( lintno > TCP_LINT_MAX-1 )
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"link index(%d) err of dp(%x)",lintno,srcDpId);
                    Tcp_PacketStat(msghand_sumidx,eLinkHandAck);
                    return;
                }

                if ( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState )
                {
                    Tcp_PacketStat(lintno,eLinkTry);
                }else
                {
                    Tcp_PacketStat(lintno,eLinkHandAck);
                }
                //告警恢复
                if( TCP_LINK_FAIL == g_pstTCP->lintCcb[lintno].ucHandState)
                {
                    tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,lintno);
                }
                g_pstTCP->lintCcb[lintno].hankCount =  0 ;    // 链路清
                g_pstTCP->lintCcb[lintno].ucHandState = TCP_LINK_OK ;   // 通信ok

            }
            break;
        default:
            Tcp_PacketStat(msghand_sumidx,eOtherStat);
            break;
    }
}

// 此函数用来检查配置链路号的有效性
XS32 tcp_check_lintIndex(XU16 index)
{
    if( 0 == index )
    {
        return  0 ;
    }

    if( 0 < index &&  TCP_LINT_MAX > index )
    {
        return  0 ;
    }
    else
    {
        return  1 ;
    }

}
     
XS32  XOS_FIDTCPE(HANDLE hDir,XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST NTLLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &NTLLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    NTLLoginList.stack     = &g_TcpeFidInfo;
    XOS_StrNcpy(NTLLoginList.taskname , "Tsk_TCPE", MAX_TID_NAME_LEN);

#ifdef MAKE_SXC_LINUX    
    NTLLoginList.TID        = 221;  
#else
    NTLLoginList.TID        = FID_TCPE;  
#endif  

    NTLLoginList.prio      = TSK_PRIO_NORMAL;
    NTLLoginList.quenum = MAX_MSGS_IN_QUE;
    NTLLoginList.stacksize = 102400;
    ret = XOS_MMStartFid(&NTLLoginList,XNULLP,XNULLP);

    return ret;
}

XU16 tcpe_getDefaultLink()
{
    return g_pstTCP->ProIndex;
}

#ifdef __cplusplus
    }
#endif


