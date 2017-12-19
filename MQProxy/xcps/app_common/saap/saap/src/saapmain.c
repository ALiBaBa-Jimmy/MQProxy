/*----------------------------------------------------------------------
saapRouterTable.h - 全局宏与类型定义

版权所有 2004 -2006 信威公司深研所BSC项目组.

author: 张海

修改历史记录
--------------------
添加版权说明.
----------------------------------------------------------------------*/

#ifdef  __cplusplus
    extern  "C"{
#endif

#include "../inc/saappublichead.h"
#include "../inc/saapmain.h"
#include "../inc/saapoamproc.h"
#include "../inc/saaproutertable.h"
#include "../../tcpe/inc/tcproutertable.h"
#include "../../hashlst/inc/syshash.h"
#include "../../tcpe/inc/tcpoamproc.h"

XU32 ulipreturnfailNum =0;
XU16 g_RouteNum=0 ;
XS32 gUidRouteParseM = 0; // 是否匹配UID多位
extern XS32  XOS_FIDTCPE(HANDLE hDir,XS32 argc, XCHAR** argv);

// 外部数据单元引用
extern SAAP_TELL_INDEX_DSTDEVICEID_CCB  *gSaapTelToDstIDCcTbl;
extern HASH_TABLE                        gSAAPTelToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// tell to dstid HASH表

extern SAAP_HLR_INDEX_DSTDEVICEID_CCB   *gSaapHlrToDstIDCcTbl;
extern HASH_TABLE                        gSAAPHlrToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// hlr to dstid HASH表;
extern SAAP_PID_INDEX_DSTDEVICEID_CCB   *gSaapPidToDstIDCcTbl;
extern HASH_TABLE                        gSAAPPidToDstIDHashTbl;// eid to dstid HASH表

extern SAAP_TELL_INDEX_DSTDEVICEID_CCB  stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
extern SAAP_HLR_INDEX_DSTDEVICEID_CCB   stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
extern SAAP_GLOBAL_STRUCT                g_ulSAAPGlobal ;         // 本机的信令点码

extern TCP_LINT_CCB_INDEX             *g_pstTCP;
extern SAAP_SPA_CLI_DATA_T gstSaapSpaCliData;
extern XU16 tcpe_getDefaultLink();

// 外部变量
XU8  g_ucSaapMonitorMondel;        // 1 跟踪, 0 不跟踪
XU8  g_saapStatFlag = 1;
t_SAAPSTAT g_SaapStaData;

PTIMER pSaapTimer[SAAP_TIMER_NUM];
XU32 gTestEid  = 0;
XU32 gTestUid = 0;
XU8  gTestTellNO[100] ={0};
XU32 gTestProTag = 0x80;
SAAP_REGSRVFID_FUNC pSaapGetSrvFidByProFunc = XNULL;
SAAP_UID_SEG_CFG gUidSegCfg;

t_XOSFIDLIST g_SaapFidInfo = 
{ 
 {"FID_SAAP",XNULL,FID_SAAP},
 {SAAP_InitProc,XNULL,XNULL}, 
 {SAAP_MsgProc,SAAP_TimeoutProc}, 
 eXOSMode, XNULL
};

/*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
XS32  saap_senderr_data_proc( COMMON_HEADER_SAAP *psCpsMsg )
{
    //处理SAAP的发送失败消息,向用户返回发送失败
    t_XOSCOMMHEAD* pxosMsg = XNULL;
    
    if(XNULL == psCpsMsg)
    {
       return XERROR;
    }

    if(0 == psCpsMsg->subID) // subId = 0 表示路由转发时消息发送失败的错误通知,不用通知业务层
    {
        return XSUCC;
    }
    
    pxosMsg= (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(psCpsMsg->stSender.ucFId, psCpsMsg->usMsgLen);
    if(XNULL == pxosMsg)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"saap_senderr_data_proc malloc mem fail!");
        return XERROR;
    }
    if(psCpsMsg->subID <=FID_XOSMAX)
    {
        XOS_Trace(MD(FID_SAAP,PL_INFO),"saap_senderr_data_proc submsgid %d error!",psCpsMsg->subID);
        XOS_MsgMemFree(pxosMsg->datasrc.FID,  pxosMsg);
        return XSUCC;
    }
    if(0==psCpsMsg->stReceiver.usFsmId)
    {
        XOS_Trace(MD(FID_SAAP,PL_INFO),"saap_senderr_data_proc fsmid %d error!",psCpsMsg->stReceiver.usFsmId);
        XOS_MsgMemFree(pxosMsg->datasrc.FID,  pxosMsg);
        return XSUCC;
    }

    if(psCpsMsg->subID ==FID_TCPE|| psCpsMsg->subID ==FID_SAAP)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"saap_senderr_data_proc submsgid %d error!",psCpsMsg->subID);
        XOS_MsgMemFree(pxosMsg->datasrc.FID,pxosMsg);
        return XSUCC;
    }

    //消息头转换
    pxosMsg->datasrc.PID    = XOS_GetLocalPID();
    pxosMsg->datasrc.FID    = FID_SAAP;
    pxosMsg->datasrc.FsmId  = 0;
    pxosMsg->datadest.PID   = XOS_GetLocalPID();
    pxosMsg->datadest.FID   = psCpsMsg->subID;/*特殊处理*/
    pxosMsg->datadest.FsmId = psCpsMsg->stReceiver.usFsmId;
    pxosMsg->msgID          = psCpsMsg->usMsgId;
    pxosMsg->prio           = eNormalMsgPrio;
    pxosMsg->length         = psCpsMsg->usMsgLen;


    //消息内容拷贝
    XOS_MemCpy(pxosMsg->message,psCpsMsg + 1,psCpsMsg->usMsgLen);

    //发送消息
    if(XSUCC != XOS_MsgSend(pxosMsg))
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR), "SYS MSGSEND XOS_MsgSend() fail!");
        XOS_MsgMemFree(pxosMsg->datasrc.FID,  pxosMsg);
        return XERROR;
    }
    XOS_Trace(MD(FID_SAAP,PL_INFO),"saap senderr rsp to fid %d fsmid %d",pxosMsg->datadest.FID,pxosMsg->datadest.FsmId);
    return XSUCC;
}
/*2009/04/28 add above*/

/*----------------------------------------------------------------------
    SAAP_InitProc                - EXAM功能块初始化函数
    参数说明：
----------------------------------------------------------------------*/
XS8 SAAP_InitProc(XVOID *Para1, XVOID *Para2 )
{
    XS32 ulErrNo = 0;
    SAAP_MallocGlobalMemory();
    XOS_MemSet(&gstSaapSpaCliData,0x0,sizeof(SAAP_SPA_CLI_DATA_T));
    if(XSUCC !=XOS_TimerReg(FID_SAAP,500,3,0) )
    {
      XOS_Trace(MD(FID_TCPE,PL_EXP),"SAAP InitProc,reg timer failed.");
    }
    SAAP_TimeStart();//9999 空函数
    ulErrNo = saap_local_Init();
    saap_regCmd();
    SaapTblRegToOAM(FID_SAAP);
    return ulErrNo;
}

XS8 SAAP_MsgProc(XVOID *msg, XVOID *Para)
{
    t_XOSCOMMHEAD *pxosMsg = XNULL;
    COMMON_HEADER_SAAP *pstMsg = XNULL;
    XS32 ulErrNo = 0;
    XU16 usMsgId;
    if(XNULL == msg)
    {
        return XERROR;
    }
    pxosMsg=(t_XOSCOMMHEAD*)msg;
    usMsgId= pxosMsg->msgID;
    /*9999
     pstMsg = (COMMON_HEADER_SAAP *)saapmsgbuffer;

    if(XSUCC != SYS_XOSMsg2CpsMsg(pxosMsg, pstMsg) )
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR)," Trans Xos Msg to CpsMsg failed.");
        return XERROR;
    }*/
    if(FID_NTL == pxosMsg->datasrc.FID && eErrorSend == pxosMsg->msgID)
    {
        ulipreturnfailNum++;
        return XSUCC;
    }
    pstMsg = (COMMON_HEADER_SAAP *)pxosMsg;
    switch(usMsgId)//9999 case 不当
    {
        case SAAP_DATA_REQ:
            if (g_ucSaapMonitorMondel)
            {
                XOS_Trace(MD(FID_SAAP,PL_DBG),"saap received packet msg len %d",pstMsg->usMsgLen );
            }
            ulErrNo = svc_to_saap_data_proc(pstMsg , SAAP_INVIKE_LOCAL);
            break;
        case SAAP_DATA_IND: /* 处理TCP/IP封装层上行的消息 */
            if (g_ucSaapMonitorMondel)
            {
               XOS_Trace(MD(FID_SAAP,PL_DBG),"saap received tcp packet msg len %d",pstMsg->usMsgLen );
            }
            ulErrNo =   ip_to_saap_data_proc( pstMsg ) ;
            break;
        case 0xeeee:
        case 0xeeef:
            g_SaapStaData.ulLinkFailNum++;
            XOS_Trace(MD(FID_SAAP,PL_ERR),"SAAP rev send fail from tcpe");
            /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
            saap_senderr_data_proc(pstMsg);
            /*2009/04/28 add above*/

            return  0 ;
        default:
            ulErrNo = XERROR;
            XOS_Trace(MD(FID_SAAP,PL_ERR),"saap received unknow usMsgId %d",usMsgId );
            break;

    }
    return(ulErrNo);
}

/*----------------------------------------------------------------------
                    -IP 上行 SAAP 消息处理
----------------------------------------------------------------------*/
XS32  ip_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg )
{
    XU16 shLen, flag;
    XU32 tempNo, iPos;
    XU32 ulsaapStatIndex  = 0;
    XU8  ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];

    SAAP_AND_TCP_MSG_BUF            *pTcppMsg ;    // 收到的包结构
    APP_OR_SAAP_COMMON_HEADER       *pstTmp;       // 出去的包结构
    XU16 usSubId = 0;
    
    // 包指针转换
    pTcppMsg = (SAAP_AND_TCP_MSG_BUF*) pMsg;
    pstTmp   = (APP_OR_SAAP_COMMON_HEADER *)ucOutMsg;

    // 地址转换
    iPos =  saap_globalID_to_localID(&(pstTmp->appSaapHead),&(pTcppMsg->ucBuffer[0]) );
    if ( 0 == iPos )
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"trans globalid to local id fail.");
        return XERROR;
    }

    // 业务用户数据的封装,注SAAP层数据不送至上层
    shLen =  pstTmp->appSaapHead.UserMsgLen  ;
    // 比对一下长度信息，以保护内存
    if ( shLen > SAAP_MAX_MSG_LEN )
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"saap msg len(%d) > max len(%d)",shLen,SAAP_MAX_MSG_LEN);
        return XERROR;
    }
    XOS_MemCpy(&pstTmp->ucBuffer, &pTcppMsg->ucBuffer[iPos], shLen );

    // 计算包长
    shLen =  shLen + sizeof(APP_AND_SAAP_PACKAGE_HEAD) ;

    // 本机信令，上传至业务模块
    pstTmp->com.stReceiver.ucModId = pTcppMsg->com.stReceiver.ucModId;             //接收者模块号
    /* 概据用户类型得到接收者的功能块号 */
    if(XNULL != pSaapGetSrvFidByProFunc)
    {
        pstTmp->com.stReceiver.ucFId = pSaapGetSrvFidByProFunc(pstTmp->appSaapHead.ProtocolTag); //SAAP_getFidByProTag(pstTmp->appSaapHead.ProtocolTag);
    }
#if 0
    switch(  pstTmp->appSaapHead.ProtocolTag )
    {
        case SAAP_SMAP_MSE:
            {
                 // 测试用
                 pstTmp->com.stReceiver.ucFId     = FID_MM;      //接收者功能块号
            }
            break;
        case SAAP_SHAP_MSE:
            {
                  pstTmp->com.stReceiver.ucFId     = FID_CC;      //接收者功能块号
            }
            break;
        default:
            return XERROR;   // 目前SAAP仅支持SMAP，SHAP消息
    }
#endif

    pstTmp->com.stReceiver.usFsmId = pTcppMsg->com.stReceiver.usFsmId ; //接收者内部连接号
    pstTmp->com.stSender.ucModId   = pTcppMsg->com.stSender.ucModId ;   //发送者模块号
    pstTmp->com.stSender.ucFId     = FID_SAAP;                          //发送者功能块号
    pstTmp->com.stSender.usFsmId   = pTcppMsg->com.stSender.usFsmId ;   //发送者内部连接号
    pstTmp->com.usMsgId            = SAAP_DATA_IND; //pTcppMsg->com.usMsgId ; //cxf 20101125 modify           //消息号
    pstTmp->com.usMsgLen           = shLen ;                            //后续长度之和

    // 本机号段与设备号比对
    flag = 0 ;   // 1 为本地， 0 为路由

    switch(  pstTmp->appSaapHead.uGst.GuestSerialNumTag )
    {
        case SAAP_GST_DN_TO_SHLR:
        {

            tempNo =  saap_BCD_to_GT( SAAP_GST_DN_TO_SHLR ,pstTmp->appSaapHead.uGst.uGstNID.GuestNetIDTell) ;

            flag = saap_check_no_proc(pstTmp->appSaapHead.ProtocolTag, SAAP_GST_DN_TO_SHLR , 0 ,  tempNo ) ;

        }
        break;

        case SAAP_GST_EID_TO_SHLR:
        {
            // 使用 dn 参数传递下去
            flag = saap_check_no_proc(pstTmp->appSaapHead.ProtocolTag, SAAP_GST_EID_TO_SHLR , 0 ,  pstTmp->appSaapHead.uGst.uGstNID.GuestNetID ) ;

        }
        break;

        case SAAP_GST_UID_TO_SHLR:
        {
            tempNo = pstTmp->appSaapHead.uGst.uGstNID.GuestNetID ;
            // uid 不能为 0 
            if (  tempNo ==  0 )
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR), " UID Error! -- ip_to_saap_data_proc()");
                return XERROR;
            }
            else if(tempNo ==  BLANK_ULONG  ) //如果UID为0xffffffff,则让其落地,因sxc->hlr某个消息中只知道PID,则不知道UID
            {
                flag = 1;
            }
            else
            {
                // 提出 hlr 号进行比对
                tempNo = tempNo >> 16 ;
                flag = saap_check_no_proc(pstTmp->appSaapHead.ProtocolTag, SAAP_GST_UID_TO_SHLR , (XU16)tempNo ,  0 ) ;
            }
#if 1            
            /*20100507 cxf add
            if localNode is Hlr and (cfg uid Route and not found(flag=0 ) )
            then send Route not found to App Layer
            20101125 modify ,hlr未配该UID的路由且TCPE层无默认链路*/
            if(0 == flag && SAAP_NODE_TYPE_HLR == g_ulSAAPGlobal.eidNoCfg.nodeType)
            {
                // 找不到该UID的前缀路由且TCPE未配默认链路
                if ( SAAP_ERROR_NUM ==  saap_DstID_from_Gt(pstTmp->appSaapHead.uGst,pstTmp->appSaapHead.ProtocolTag) && 0xFFFF==tcpe_getDefaultLink())
                {
                    pstTmp->com.usMsgId = SAAP_NO_ROUTE;
                    XOS_Trace(MD(FID_SAAP,PL_ERR),"LocalNode is Hlr,Not config UID[%04x]'s route,send route not found to appLayer)",tempNo);                    
                    usSubId = SAAP_ERR_NOT_FIND_UID_ROUTE;
                    flag = 1;
                }                
            }
#endif            
        }
        break;

        default:
            XOS_Trace(MD(FID_SAAP,PL_ERR), "GuestSerialNumTag  Error! -- ip_to_saap_data_proc()");
            return XERROR;   // 目前SAAP仅支持SMAP，SHAP消息
    }

    if ( !flag )
    {
        // 路由消息, 将消息ID转换为数据请求
        pstTmp->com.usMsgId = SAAP_DATA_REQ;

        g_RouteNum =  pTcppMsg->tcpHead.RouterNum ;
        pstTmp->com.subID = 0; // 0 mean not need send 2 user layer
#if 0        
        // sub 路由转发时用于保存用户层FID
        if(XNULL != pSaapGetSrvFidByProFunc )
        {
            pstTmp->com.subID = pSaapGetSrvFidByProFunc(pstTmp->appSaapHead.ProtocolTag);
        }
        else
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR),"pSaapGetSrvFidByProFunc==NULL when tranSend msg.");
        }
#endif

        XOS_Trace(MD(FID_SAAP, PL_INFO), "route msg ok! -- ip_to_saap_data_proc()");
        svc_to_saap_data_proc( (COMMON_HEADER_SAAP*)pstTmp , SAAP_INVIKE_ROUTE );
    }
    else
    {
        // 本机业务消息
        if(XNULL == pSaapGetSrvFidByProFunc )
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR),"not reg func of GetSrvFidByProFunc ,not send msg to srv.");
            return XSUCC;
        }
        pstTmp->com.usMsgId            = SAAP_DATA_IND; // 20101126 cxf add
        /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
        pstTmp->com.stReceiver.usFsmId =0;//接收者内部连接号
        pstTmp->com.stSender.usFsmId =0;
        pstTmp->com.subID = usSubId;        
        /*2009/04/28 add above for saap_senderr_rsp process*/
        SYS_MSGSEND((COMMON_HEADER_SAAP *) pstTmp );

        if(  1 == g_saapStatFlag  )
        {
           if( pstTmp->appSaapHead.ProtocolTag >= SAAP_SMAP_MSE
            && pstTmp->appSaapHead.ProtocolTag < SAAP_PRO_TAG_MAX
            )
           {
               ulsaapStatIndex = pstTmp->appSaapHead.ProtocolTag >> 4;
               ulsaapStatIndex = ulsaapStatIndex -8;
               g_SaapStaData.ulRecvMsgCnt[ulsaapStatIndex]++;
           }
        }

    }

    return XSUCC;

}



/*----------------------------------------------------------------------
    svc_to_saap_data_proc                -业务 下行 SAAP 消息处理

----------------------------------------------------------------------*/
XS32  svc_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg , XU8 flag )
{

    XU8  ucOutMsg[SAAP_MAX_MSG_LEN + sizeof(COMMON_HEADER_SAAP)];
    XU16 shLen   ;
    XU16 DstID ;
    XU32 ulSaapStatIndex = 0;
    XU32  iPos ;
    int i  = 0;
    APP_OR_SAAP_COMMON_HEADER   *pAppSaapHead ;      // 收到的ID包结构
    SAAP_AND_TCP_MSG_BUF        *pstTmp;             // 向TCPENCAP发出去的包结构

    XOS_Trace(MD(FID_SAAP, PL_DBG), "Entry! -- svc_to_saap_data_proc()");
    // 将收到的包进行强制类型转换
    pAppSaapHead = ( APP_OR_SAAP_COMMON_HEADER* )pMsg ;

    pstTmp = (SAAP_AND_TCP_MSG_BUF*)ucOutMsg ;

    pstTmp->tcpHead.RouterNum = SAAP_ROUTER_MAX_NUM ;

    //20100622 cxf add: if UidPreRoute,find UidSegRouteTbl first
    if(SAAP_GST_UID_TO_SHLR == pAppSaapHead->appSaapHead.uGst.GuestSerialNumTag)
    {
        DstID = saap_getDstID_from_UidSeg(pAppSaapHead->appSaapHead.uGst.uGstNID.GuestNetID);
        if(SAAP_ERROR_NUM ==  DstID)
        {
            DstID = saap_DstID_from_Gt(pAppSaapHead->appSaapHead.uGst,pAppSaapHead->appSaapHead.ProtocolTag);
        }
    }
    else
    {
        // 查找目的设备号表得到目的信令点编码
        DstID = saap_DstID_from_Gt(pAppSaapHead->appSaapHead.uGst,pAppSaapHead->appSaapHead.ProtocolTag);
    }
    if ( SAAP_ERROR_NUM ==  DstID )
    {
        XOS_Trace(MD(FID_SAAP,PL_WARN),"not find DstId for Gt(ProTag=%d,Type=%d,val=%x)",pAppSaapHead->appSaapHead.ProtocolTag,pAppSaapHead->appSaapHead.uGst.GuestSerialNumTag,pAppSaapHead->appSaapHead.uGst.uGstNID.GuestNetID);
        DstID = 0 ;  // 如果没有找到目的信令点，则使用 0 ，0 为保留信令点，由TCPENCAP层送往上一级节点上
    }
    XOS_Trace(MD(FID_SAAP,PL_DBG),"SAAP:Use dstIp %d send to tcpe.",DstID);

    // 这里打上目的点的信令点编码，本地的在TCP/IP层加上
    pstTmp->tcpHead.DstDeviceID    =  DstID;  //目的信令点

    pstTmp->tcpHead.InviteDeviceID = g_pstTCP->GlobalDstID ; // 源信令点,在HLR纯数据组网中,SAAP消息直接发给NAT模块,源信令在此写上

    pstTmp->tcpHead.UserType       = SAAP_USER_TYPE ;       // 用户类型

    if ( SAAP_INVIKE_LOCAL == flag)
    {
        pstTmp->tcpHead.RouterNum      = SAAP_ROUTER_MAX_NUM ;  // 加上路由次数
#ifdef HLR_DATA_NET
        pAppSaapHead->appSaapHead.uInt.InviteSerialNumTag = SAAP_INT_EID_TO_SHLR;
        pAppSaapHead->appSaapHead.uInt.uIntNID.InviteNetID  = g_ulSAAPGlobal.eidNoCfg.eidNo ;
#endif
    }
    else
    {
        // 保持原有路由数据不变
        pstTmp->tcpHead.RouterNum      =  g_RouteNum ;         // 路由次数
        g_RouteNum = SAAP_ROUTER_MAX_NUM + 1 ;
    }

    // 注：其包的封装过程为从内向外
    // 这一步完成用户数据的封装

    shLen =  pAppSaapHead->appSaapHead.UserMsgLen ;

    // 此函数完成SAAP包头的封装, 此函数完全只是数据转换，所以不用比对返回值
    iPos = saap_localID_to_globalID( &(pstTmp->ucBuffer[0]) , &(pAppSaapHead->appSaapHead) );
    if ( 0 == iPos )
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR), "GT Error! -- svc_to_saap_data_proc()");
        return XERROR;
    }

    // 内存检查，保护内存
    if ( shLen > SAAP_MAX_MSG_LEN)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR), "shLen Error! -- svc_to_saap_data_proc()");
        return XERROR;
    }
    // copy 用户数据
    XOS_MemCpy( &pstTmp->ucBuffer[iPos], pAppSaapHead->ucBuffer, shLen );

    // 处理用户类型
    pstTmp->tcpHead.UserType = SAAP_USER_TYPE ;

    shLen = shLen + (XU16)iPos  ;
    // 这里的长度应当分别计算

    pstTmp->tcpHead.MsgLen = shLen;   // 这个长度是用户长度与SAAP包长之和

    // 完成向TCP/IP的发送, 注SAAP下行的用户只有 TCP/IP 封装，原则上基本包头的联接号与消息号不变
    shLen =  shLen + sizeof(SAAP_AND_TCP_PACKAGE_HEAD);

    if(  1 == g_saapStatFlag  )
    {
        if( SAAP_INVIKE_ROUTE  == flag)
        {
            g_SaapStaData.ulRouteMsgCnt++;
        }
        else
        {
          if(pAppSaapHead->appSaapHead.ProtocolTag > SAAP_PRO_TAG_MIN
           && pAppSaapHead->appSaapHead.ProtocolTag < SAAP_PRO_TAG_MAX)
          {
              ulSaapStatIndex = pAppSaapHead->appSaapHead.ProtocolTag >> 4;
              ulSaapStatIndex = ulSaapStatIndex -8;
              g_SaapStaData.ulSendMsgCnt[ulSaapStatIndex]++;
          }
        }
    }

    // 测试用对FID作了一下调整
    pstTmp->com.stReceiver.ucModId = pAppSaapHead->com.stReceiver.ucModId ;
#ifdef HLR_DATA_NET
    if(g_ulSAAPGlobal.eidNoCfg.eidNo ==   pAppSaapHead->appSaapHead.uGst.uGstNID.GuestNetID >> 16  )
    {
        pstTmp->com.stReceiver.ucFId   = FID_NAT ;
    }
    else
    {
        pstTmp->com.stReceiver.ucFId   = FID_TCPE ;
    }

#else
    pstTmp->com.stReceiver.ucFId   = FID_TCPE ;
#endif
    pstTmp->com.stReceiver.usFsmId = pAppSaapHead->com.stReceiver.usFsmId  ;
    pstTmp->com.stSender.ucModId   = pAppSaapHead->com.stSender.ucModId;
    pstTmp->com.stSender.ucFId     = FID_SAAP;
    pstTmp->com.stSender.usFsmId   = pAppSaapHead->com.stSender.usFsmId;

    pstTmp->com.usMsgId            = pAppSaapHead->com.usMsgId ;
    /*2009/04/28 add below for saap_senderr_rsp process,submsg is src fid*/
    pstTmp->com.subID= pAppSaapHead->com.subID;
    /*2009/04/28 add above for saap_senderr_rsp process*/

    pstTmp->com.usMsgLen           = shLen ;    //后续长度之和

    SYS_MSGSEND( (COMMON_HEADER_SAAP *)pstTmp );
    XOS_Trace(MD(FID_SAAP, PL_MIN), "out -- svc_to_saap_data_proc()");
    return XSUCC;

}

/*----------------------------------------------------------------------
    saapInit                - saap 初始化
----------------------------------------------------------------------*/
XS32 saap_local_Init()
{

//    XU16 i ;

    XOS_Trace(MD(FID_SAAP,PL_MIN),"enter saap_local_Init");
    // 初始化为不跟踪模式
    g_ucSaapMonitorMondel = 0 ;

    // 初始化为统计
    g_saapStatFlag = 1;

#if 0
    // 初始化将本地 GT 数据设为无效
    g_ulSAAPGlobal.eidNoCfg.flag  = 0 ;

    for ( i = 0 ; i < SAAP_LOCAL_PREFIX_NUM ; i++ )
    {
        g_ulSAAPGlobal.hlrNoCfg[i].flag = 0 ;
        g_ulSAAPGlobal.tellNoCfg[i].flag = 0 ;
    }
#endif
    return XSUCC;
}

/*----------------------------------------------------------------------
    SAAP_TimeInit                - SAAP 定时器初始化
----------------------------------------------------------------------*/
void SAAP_TimeInit()
{
    ;

}

/*----------------------------------------------------------------------
    SAAP_TimeStart                - 启动 SAAP 定时器
----------------------------------------------------------------------*/
void SAAP_TimeStart()
{
    ;
}

/*----------------------------------------------------------------------
    SAAP_TimeoutProc                - SYS 功能块处理定时器消息函数
----------------------------------------------------------------------*/
XS8 SAAP_TimeoutProc(t_BACKPARA  *pstPara)
{
    XU32 tmrName = 0;
    XU32 ulPara = 0;

    tmrName = pstPara->para2;
    ulPara = pstPara->para3;

    switch(tmrName)
    {
        case SAAP_TIMER_TEST:
             SaapPeriodTest(ulPara);
             break;
        default:
            XOS_Trace(MD(FID_SAAP,PL_ERR),"unknow timer(%d).",tmrName);
            break;
    }

    return(XSUCC);
}

/*----------------------------------------------------------------------
    localID_to_globalID                - 标准SAAP包头封装函式
    地址表示语
    7  6  5  4  3  2  1  0
   |----翻译类型 --------|
   |编号计划 -|- 编码设计|

   翻译类型保留,设为0xff
   编码设计
   比特：3 2 1 0
      0 0 0 0    未定义
      0 0 0 1    BCD，奇数个数字
      0 0 1 0    BCD，偶数个数字

编号计划:
0001： ISDN/电话编号计划（国标）
1000： SCDMA McWill系统设备编号计划 =0x08
1001： SCDMA McWill系统UID编号计划  =0x09
----------------------------------------------------------------------*/
XU32  saap_localID_to_globalID( XU8 *pSaapHead , APP_AND_SAAP_PACKAGE_HEAD *pAppSaapHead  )
{

    XU32 ulTemp1 , ulTemp2 ;

    ulTemp1 = ulTemp2 = 4 ;


    pSaapHead[0] = pAppSaapHead->ProtocolTag ;   // 协议表示语

    // 目的包头处理
    pSaapHead[1] = SAAP_GUEST_TAG;
    switch( pAppSaapHead->uGst.GuestSerialNumTag  ) //地址标签
    {

        case SAAP_GST_DN_TO_SHLR:
            {
                ulTemp1 = 8 ;

                pSaapHead[2] = 10 ; // len
                pSaapHead[3] = BLANK_UCHAR;  // 编译类型
                pSaapHead[4] = SAAP_G_DN_GT;  // 编号计划 + 编码设计
                XOS_MemCpy( &pSaapHead[5], pAppSaapHead->uGst.uGstNID.GuestNetIDTell, 8 );
                break;
            }
        case SAAP_GST_EID_TO_SHLR:
            {

                pSaapHead[2] = 6 ; // len
                pSaapHead[3] = BLANK_UCHAR;  // 编译类型
                pSaapHead[4] = SAAP_G_EID_GT;  // 编号计划 + 编码设计
                saap_GT_to_BCD( &pSaapHead[5], pAppSaapHead->uGst.uGstNID.GuestNetID );
                break;
            }
       case SAAP_GST_UID_TO_SHLR:
            {
                pSaapHead[2] = 6 ;   // len
                pSaapHead[3] = BLANK_UCHAR;  // 编译类型
                pSaapHead[4] = SAAP_G_UID_GT;  // 编号计划 + 编码设计
                saap_GT_to_BCD( &pSaapHead[5], pAppSaapHead->uGst.uGstNID.GuestNetID );
                break;
            }

        default:
            XOS_Trace(MD(FID_SAAP,PL_ERR),"Dst_GuestSerialNumTag(%x) err.",pAppSaapHead->uGst.GuestSerialNumTag);
            return 0;
    }

    // 源包头处理
    pSaapHead[5 + ulTemp1] = SAAP_INVITE_TAG;
    switch( pAppSaapHead->uInt.InviteSerialNumTag  )
    {

        case SAAP_INT_DN_TO_SHLR:
            {
                ulTemp2 = 8 ;

                pSaapHead[6 + ulTemp1]  = 10 ;
                pSaapHead[7 + ulTemp1]  = BLANK_UCHAR;  // 编译类型
                pSaapHead[8 + ulTemp1]  = SAAP_G_DN_GT;  // 编号计划 + 编码设计
                XOS_MemCpy( &pSaapHead[9 + ulTemp1] , pAppSaapHead->uInt.uIntNID.InviteNetIDTell , 8 );
                break;
            }

        case SAAP_INT_EID_TO_SHLR:
            {
                pSaapHead[6 + ulTemp1]  = 6 ;
                pSaapHead[7 + ulTemp1]  = BLANK_UCHAR;  // 编译类型
                pSaapHead[8 + ulTemp1]  = SAAP_G_EID_GT;  // 编号计划 + 编码设计
                saap_GT_to_BCD( &pSaapHead[9 + ulTemp1] , pAppSaapHead->uInt.uIntNID.InviteNetID  );
                break;
            }
        case SAAP_INT_UID_TO_SHLR:
            {
                pSaapHead[6 + ulTemp1]  = 6 ;
                pSaapHead[7 + ulTemp1]  = BLANK_UCHAR;  // 编译类型
                pSaapHead[8 + ulTemp1]  = SAAP_G_UID_GT;  // 编号计划 + 编码设计
                saap_GT_to_BCD( &pSaapHead[9 + ulTemp1] , pAppSaapHead->uInt.uIntNID.InviteNetID );
                break;
            }

        default:
            XOS_Trace(MD(FID_SAAP,PL_ERR),"Src_GuestSerialNumTag(%x) err.",pAppSaapHead->uInt.InviteSerialNumTag);
            break;
    }

    // 完成用户消息的总长
    *(XU16*)&pSaapHead[9 + ulTemp1 + ulTemp2] =  XOS_HtoNs(pAppSaapHead->UserMsgLen) ;

    return 11 + ulTemp1 + ulTemp2 ;
}

/*----------------------------------------------------------------------
    saap_globalID_to_localID                - 标准SAAP包头封装函式
    SAAP标准头
    协议用户表示语
    被叫用户地址标签
    被叫用户地址长度
    被叫用户GT码(BCD格式)

    主叫用户地址标签
    主叫用户地址长度
    主叫用户GT码(BCD格式)

主叫/被叫地址由地址表示语及地址两部分组成:
    地址表示语
    7  6  5  4  3  2  1  0
   |----翻译类型 --------|
   |编号计划 -|- 编码设计|

   翻译类型保留,设为0xff
   编码设计
   比特：3 2 1 0
      0 0 0 0    未定义
      0 0 0 1    BCD，奇数个数字
      0 0 1 0    BCD，偶数个数字

编号计划:
0001： ISDN/电话编号计划（国标）
1000： SCDMA McWill系统设备编号计划 =0x08
1001： SCDMA McWill系统UID编号计划  =0x09*/

XU32  saap_globalID_to_localID( APP_AND_SAAP_PACKAGE_HEAD *pAappSaapHead  ,  XU8 *pSaapHead  )
{
    XU32 id  ,  ulTemp1 , ulTemp2 ;
    // 增加一些关键字段的检查
    ulTemp1 = 4;
    ulTemp2 = 4 ;

    pAappSaapHead->ProtocolTag  = pSaapHead[0] ;

    // 目的地址转换
    switch( pSaapHead[4] )  //地址表示语 高 4 bit 表示编号计划,低4 bit表示 编码设计(0未定义,1-BCD奇数个数字,2-偶数个数字)
    {
        case SAAP_G_DN_GT:
            {

                ulTemp1 = 8 ;

                // BCD 码中存的是DN
                pAappSaapHead->uGst.GuestSerialNumTag = SAAP_GST_DN_TO_SHLR;
                XOS_MemCpy( pAappSaapHead->uGst.uGstNID.GuestNetIDTell, &(pSaapHead[5]), 8 );

            }
            break;

        case SAAP_G_EID_GT:
            {
                // BCD 码中存的是设备ID
                id = saap_BCD_to_GT(SAAP_GST_EID_TO_SHLR  , &(pSaapHead[5]));
                pAappSaapHead->uGst.GuestSerialNumTag = SAAP_GST_EID_TO_SHLR;
                pAappSaapHead->uGst.uGstNID.GuestNetID = id ;
            }
            break;
        case SAAP_G_UID_GT:
            {
                // BCD 码存的为UID
                id = saap_BCD_to_GT(SAAP_GST_UID_TO_SHLR  , &(pSaapHead[5]));
                pAappSaapHead->uGst.GuestSerialNumTag  = SAAP_GST_UID_TO_SHLR;
                pAappSaapHead->uGst.uGstNID.GuestNetID = id ;
            }
            break;

        default:
            return 0 ;
    }
    // 源地址转换
    switch( pSaapHead[ 8 + ulTemp1 ] )
    {

        case SAAP_G_DN_GT:
            {
                ulTemp2 = 8 ;

                // BCD 码中存的是DN
                pAappSaapHead->uInt.InviteSerialNumTag  = SAAP_INT_DN_TO_SHLR;
                XOS_MemCpy( pAappSaapHead->uInt.uIntNID.InviteNetIDTell , &(pSaapHead[ 9 + ulTemp1]), 8 );
            }
            break;

        case SAAP_G_EID_GT:
            {
                // BCD 码向 PID 的转换
                id = saap_BCD_to_GT(SAAP_INT_EID_TO_SHLR , &(pSaapHead[ 9 + ulTemp1]));
                pAappSaapHead->uInt.InviteSerialNumTag = SAAP_INT_EID_TO_SHLR;
                pAappSaapHead->uInt.uIntNID.InviteNetID = id ;
            }
            break;

        case SAAP_G_UID_GT:
            {
                // BCD 码向 PID 的转换
                id = saap_BCD_to_GT(SAAP_INT_UID_TO_SHLR , &(pSaapHead[ 9 + ulTemp1]));
                pAappSaapHead->uInt.InviteSerialNumTag  = SAAP_INT_UID_TO_SHLR;
                pAappSaapHead->uInt.uIntNID.InviteNetID = id ;
            }
            break;
        default:
            XOS_Trace(MD(FID_SAAP,PL_ERR),"Src_addInd(%x) err.",pSaapHead[ 8 + ulTemp1 ] );
            // 目前不处理
            return 0 ;
    }

    pAappSaapHead->UserMsgLen = XOS_NtoHs(*(XU16*)&pSaapHead[9 + ulTemp1 + ulTemp2]) ;

    return   11 + ulTemp1 + ulTemp2  ;

}

/*----------------------------------------------------------------------
    saap_GT_to_BCD                - 此函数完成 GT 到BCD码的转换
----------------------------------------------------------------------*/
void  saap_GT_to_BCD( XU8 SaapHeadBCD[] ,  XU32 GtID   )
{

    XU32 tempNum ;
    XU8 temp1 , temp2  ;
    temp1 = temp2 = 0 ;

    tempNum = GtID ;
    temp1 =  (XU8)(tempNum / 0x10000000);
    tempNum = tempNum & 0x0FFFFFFF;
    temp2 =  (XU8)(tempNum / 0x1000000);
    temp2 = temp2 << 4 ;

    SaapHeadBCD[0] = temp1 + temp2 ;

    tempNum = tempNum & 0x00FFFFFF;
    temp1 =  (XU8)(tempNum / 0x100000);
    tempNum = tempNum & 0x000FFFFF;
    temp2 =  (XU8)(tempNum / 0x10000);
    temp2 = temp2 << 4 ;
    SaapHeadBCD[1] = temp1 + temp2 ;

    tempNum = tempNum & 0x0000FFFF;
    temp1 =  (XU8)(tempNum / 0x1000);
    tempNum = tempNum & 0x00000FFF;
    temp2 =  (XU8)(tempNum / 0x100);
    temp2 = temp2 << 4 ;
    SaapHeadBCD[2] = temp1 + temp2 ;

    tempNum = tempNum & 0x000000FF;
    temp1 =  (XU8)(tempNum / 0x10);
    tempNum = tempNum & 0x0000000F;
    temp2 =  (XU8)tempNum ;
    temp2 = temp2 << 4 ;
    SaapHeadBCD[3] = temp1 + temp2 ;

}

/*----------------------------------------------------------------------
    saap_BCD_to_GT                - 此函数BCD码的转换出GT码
    BCD 0x27452132 <=> GT 0x72541223
----------------------------------------------------------------------*/
XU32  saap_BCD_to_GT(  XU8 flag , XU8 SaapHeadBCD[]  )
{

    XU32 num , tempNum1 , tempNum2 , i , j ;
    XU8 temp , temp1 , temp2  ;

    temp = temp1 = temp2 = 0 ;
    num = tempNum1 = tempNum2 = 0 ;

    // 注此处电话号码使用 10 进制，UID，PID 使用 16 进制

    switch( flag )
    {
    case SAAP_GST_DN_TO_SHLR :
    case SAAP_INT_DN_TO_SHLR :
            {
                // 这里只取电话号码的区号与局号做GT，查找局向表
                // 查找位数, j 表示局号有几位，主要是处理电话号码有可能少于 8 位的情况
                j = 0 ;
                for( i = 0 ; i < 4 ; i++)
                {
                    temp = SaapHeadBCD[i];

                    temp1 = temp & 0x0F ;   //low 4 bit
                    temp2 = temp & 0xF0 ;   //high 4 bit
                    temp2 = temp2 >> 4 ;

                    if( 15 == temp1 )
                    {
                        continue ;
                    }
                    j++ ;

                    if (15 == temp2 )
                    {
                        continue ;
                    }
                    j++ ;

                }

                if( j > 8) //不支持>8位
                {
                    j = 8;
                }

                switch( j )
                {

                case 1 : // 处理 1 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        tempNum1 = temp1 ;
                        num = num + tempNum1 ;
                    }
                    break;

                case 2 : // 处理 2 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10;
                        tempNum2 = temp2 ;
                        num = num + tempNum1 + tempNum2 ;
                    }
                    break;

                case 3 : // 处理 3 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 100;
                        tempNum2 = temp2 * 10;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        tempNum1 = temp1 ;
                        num = num + tempNum1 ;
                    }
                    break;

                case 4 : // 处理 4 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 1000;
                        tempNum2 = temp2 * 100;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10;
                        tempNum2 = temp2 ;
                        num = num + tempNum1 + tempNum2 ;
                    }
                    break;
                case 5 :  // 处理 5 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10000;
                        tempNum2 = temp2 * 1000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 100;
                        tempNum2 = temp2 * 10;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[2];
                        temp1 = temp & 0x0F ;
                        tempNum1 = temp1 ;
                        num = num + tempNum1 ;
                    }
                    break;
                case 6 : // 处理 6 位
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 100000;
                        tempNum2 = temp2 * 10000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 1000;
                        tempNum2 = temp2 * 100;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[2];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10;
                        tempNum2 = temp2 ;
                        num = num + tempNum1 + tempNum2 ;
                        }
                        break;
                case 7 :
                        {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 1000000;
                        tempNum2 = temp2 * 100000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10000;
                        tempNum2 = temp2 * 1000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[2];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 100;
                        tempNum2 = temp2 * 10;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[3];
                        temp1 = temp & 0x0F ;
                        tempNum1 = temp1 ;
                        num = num + tempNum1 ;

                         }
                    break;

                case 8 :
                    {
                        temp = SaapHeadBCD[0];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10000000;
                        tempNum2 = temp2 * 1000000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[1];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 100000;
                        tempNum2 = temp2 * 10000;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[2];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 1000;
                        tempNum2 = temp2 * 100;
                        num = num + tempNum1 + tempNum2 ;

                        temp = SaapHeadBCD[3];
                        temp1 = temp & 0x0F ;
                        temp2 = temp & 0xF0 ;
                        temp2 = temp2 >> 4 ;

                        tempNum1 = temp1 * 10;
                        tempNum2 = temp2 ;
                        num = num + tempNum1 + tempNum2 ;

                    }
                    break;
                default:
                    {
                        // 电话号码错误
                        return 0 ;
                    }
                    break;
                }

            }
            break;

    case SAAP_GST_EID_TO_SHLR :
    case SAAP_GST_UID_TO_SHLR :
    case SAAP_INT_EID_TO_SHLR :
    case SAAP_INT_UID_TO_SHLR :
        {
            temp =  SaapHeadBCD[0];
            temp1 = temp & 0x0F ;
            temp2 = temp & 0xF0 ;
            tempNum1 = temp1 * 0x10000000 ;
            tempNum2 = temp2 * 0x100000;
            num = num + tempNum1 + tempNum2 ;

            temp =  SaapHeadBCD[1];
            temp1 = temp & 0x0F ;
            temp2 = temp & 0xF0 ;
            tempNum1 = temp1 * 0x100000 ;
            tempNum2 = temp2 * 0x1000;
            num = num + tempNum1 + tempNum2 ;

            temp =  SaapHeadBCD[2];
            temp1 = temp & 0x0F ;
            temp2 = temp & 0xF0 ;
            tempNum1 = temp1 * 0x1000 ;
            tempNum2 = temp2 * 0x10;
            num = num + tempNum1 + tempNum2 ;

            temp =  SaapHeadBCD[3];
            temp1 = temp & 0x0F ;
            temp2 = temp & 0xF0 ;
            tempNum1 = temp1 * 0x10 ;
            tempNum2 = temp2 >> 4 ;
            num = num + tempNum1 + tempNum2 ;
        }
        break;

    default:
        XOS_Trace(MD(FID_SAAP,PL_ERR),"unknow %d DN_PLAG_FLAG.");
        break;
    }

    return num ;
}

XU16  saap_getDstID_from_UidSeg(XU32 ulUid)
{
    int i  =0;
    XU16 usDstDpId = SAAP_ERROR_NUM;
        
    if(0 != gUidSegCfg.ulUidSegRouteFlag)
    {
        for(i=0 ;i < MAX_UID_SEG_NUM; i++)
        {
            if(0 == gUidSegCfg.stUidSeg[i].ucFlag)
            {
                continue;
            }
            if( ulUid >= gUidSegCfg.stUidSeg[i].ulUidStart
                && ulUid <= gUidSegCfg.stUidSeg[i].ulUidEnd)
            {
                usDstDpId = gUidSegCfg.stUidSeg[i].dstDpID;
                return usDstDpId;
            }
        }
    }
    return SAAP_ERROR_NUM;
}

XS32 saap_SetUidSegRouteFlag(XU32 ulFlag)
{
    gUidSegCfg.ulUidSegRouteFlag = ulFlag;
    return XSUCC;
}
/**
*@brief add UidSegRoute
*@param ulUidStart 
*@param ulUidEnd
*@return XERROR -fail,other-routeIdx
*/
XS32 saap_addUidSegRoute(XU32 ulUidStart,XU32 ulUidEnd,XU16 dstDp)
{
    XS32 i  = 0;
    for(i = 0; i < MAX_UID_SEG_NUM; i++)
    {
        if(0 == gUidSegCfg.stUidSeg[i].ucFlag)
        {
            gUidSegCfg.stUidSeg[i].ucFlag = 1;
            gUidSegCfg.stUidSeg[i].ulUidStart = ulUidStart;
            gUidSegCfg.stUidSeg[i].ulUidEnd = ulUidEnd;
            gUidSegCfg.stUidSeg[i].dstDpID = dstDp;
            return i;
        }
    }
    return XERROR;
}

XS32 saap_delUidSegRoute(XS32 routeIdx)
{
    if(routeIdx >= MAX_UID_SEG_NUM)
    {
        return XERROR;
    }
    if(0 == gUidSegCfg.stUidSeg[routeIdx].ucFlag)
    {
        return XERROR;    
    }
    gUidSegCfg.stUidSeg[routeIdx].ucFlag = 0;
    return XSUCC;
}
// 此函数完成GT到信令点的查找，返回为信令点编码，对于SAAP结点，信令点编码不能为 0xFFFF
XU16  saap_DstID_from_Gt(  struct Gst gstVale,XU32 ProtocolTag  )
{
    XU16    usDstID;
    XU32 uidGt  ;
    XU32  usccno , dnGt , eidGt, tempNo ;
    XU32 i = 0;
    XU32 ulRouteTblId  = SAAP_ROUTE_TBL_MAX;

    usccno = SAAP_ERROR_NUM ;

    usDstID = SAAP_ERROR_NUM ;
    switch(ProtocolTag) //用户表示语
    {
        case SAAP_SMAP_MSE: //SXc<->HLR ,SMC->HLR ,HLR<->HLR SMAP消息
            ulRouteTblId = SAAP_ROUTE_TBL_HLR;
            break;
        case SAAP_SHLR2SMC_MSE:
        case SAAP_SXC2SMC_MSE:
        case SAAP_SMC2SMC_MSE:
            ulRouteTblId = SAAP_ROUTE_TBL_SMC;
            break;
        case SAAP_SHAP_MSE:
            ulRouteTblId = SAAP_ROUTE_TBL_HLR;
            break;
        case SAAP_X2RTB_MSE: /*20111215 cxf add begin: add rbt route func*/
            ulRouteTblId = SAAP_ROUTE_TBL_RBT;
            break;            
        default:
            XOS_Trace(MD(FID_SAAP,PL_EXP),"unknow proTag(0x%x)",ProtocolTag);
            ulRouteTblId = SAAP_ROUTE_TBL_HLR; //8888 其它值的处理
            break;
    }
    switch( gstVale.GuestSerialNumTag )
    {
    case SAAP_GST_DN_TO_SHLR :
        {
            // 这里只取电话号码的区号与局号做GT，查找电话号码局向表
            dnGt =  saap_BCD_to_GT(SAAP_GST_DN_TO_SHLR , gstVale.uGstNID.GuestNetIDTell);

            tempNo = dnGt ;
            for ( i = 0 ; i < 8*2 ; i++ ) //8888 why macro
            {
                // 号段比对
                usccno = SAAP_SearchTellToDstIDTable(tempNo,ulRouteTblId);
                if ( SAAP_ERROR_NUM != usccno)
                {
                    gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[ulRouteTblId][0];
                    usDstID = gSaapTelToDstIDCcTbl[usccno].DstID ;
                    break;
                }
                else
                {
                    tempNo = tempNo / 10 ;
                }
            }
            // zhanghai modify  090311
            // SAC 组网如果找不到则找 '1' 号配置表, 这个表为 SAC 默认表
            if ( SAAP_ERROR_NUM == usccno )
            {
                usccno = SAAP_SearchTellToDstIDTable(1,ulRouteTblId);
                if ( SAAP_ERROR_NUM != usccno)
                {
                    gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[ulRouteTblId][0];
                    usDstID = gSaapTelToDstIDCcTbl[usccno].DstID ;
                }
            }
            // zhanghai modify  end      

        }
        break;
    case SAAP_GST_EID_TO_SHLR :
        {
            // 直接取ID
            eidGt =   gstVale.uGstNID.GuestNetID  ;
            usccno = SAAP_SearchPidToDstIDTable(eidGt);
            if ( SAAP_ERROR_NUM != usccno)
            {
                usDstID = gSaapPidToDstIDCcTbl[usccno].DstID ;
            }
        }
        break;

    case SAAP_GST_UID_TO_SHLR :
        {
            // UID用高 16 位做 gt , 查找HLRID号表
          
            if(0 == gUidRouteParseM) //20100830 cxf modify
            {
                uidGt = (XU16) (gstVale.uGstNID.GuestNetID >> 16);
                usccno = SAAP_SearchHlrToDstIDTable(uidGt,ulRouteTblId);
                if ( SAAP_ERROR_NUM != usccno)
                {
                    gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulRouteTblId][0];
                    usDstID = gSaapHlrToDstIDCcTbl[usccno].DstID ;
                }                 
            }
            else
            {                
                /*20100830 cxf add begin: find UidPreRoute from  len_4 to len_6*/
                uidGt = ( gstVale.uGstNID.GuestNetID >> 4);
                for(i=4; i<=6; i++)
                {
                    uidGt = ( uidGt >> 4 );
                    usccno = SAAP_SearchHlrToDstIDTable(uidGt,ulRouteTblId);
                    if ( SAAP_ERROR_NUM != usccno)
                    {
                        gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulRouteTblId][0];
                        usDstID = gSaapHlrToDstIDCcTbl[usccno].DstID ;
                        break;
                    }                
                }
            }
            /*20100830 cxf add end */         
            // zhanghai modify  090311
            // SAC 组网如果找不到则找 '1' 号配置表, 这个表为 SAC 莫认表
            if ( SAAP_ERROR_NUM == usccno )
            {
                usccno = SAAP_SearchHlrToDstIDTable( 1 , ulRouteTblId);
                if ( SAAP_ERROR_NUM != usccno)
                {
                    gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulRouteTblId][0];
                    usDstID = gSaapHlrToDstIDCcTbl[usccno].DstID ;
                }
            }
            // zhanghai modify  end            

        }
        break;

    default:
        break;
    }

    return usDstID ;
}

//////////////////////////////////////////////////////////////////////////

// 测试专用
XU32 saap_testPro()
{

    int  j  ;
    // SMAP 消息测试
    XU8  ucOutMsg[255];
    APP_OR_SAAP_COMMON_HEADER  *pstSaapHand ;    // 发出握手包结构

    for ( j = 0 ; j < 255 ; j++)
    {
        ucOutMsg[j]  = 1 ;
    }

    pstSaapHand = (APP_OR_SAAP_COMMON_HEADER*) ucOutMsg ;

    pstSaapHand->appSaapHead.ProtocolTag  = SAAP_SMAP_MSE ;

    pstSaapHand->appSaapHead.uGst.GuestSerialNumTag = SAAP_GST_UID_TO_SHLR ;
    pstSaapHand->appSaapHead.uGst.uGstNID.GuestNetID = 0x12345678 ;

    pstSaapHand->appSaapHead.uInt.InviteSerialNumTag = SAAP_INT_UID_TO_SHLR ;

    pstSaapHand->appSaapHead.uInt.uIntNID.InviteNetID = 0x87654321  ;
    pstSaapHand->appSaapHead.UserMsgLen = 10 ;

    pstSaapHand->ucBuffer[0]  = 0x01 ;
    pstSaapHand->ucBuffer[1]  = 0x02 ;
    pstSaapHand->ucBuffer[2]  = 0x03 ;
    pstSaapHand->ucBuffer[3]  = 0x04 ;
    pstSaapHand->ucBuffer[4]  = 0x05 ;
    pstSaapHand->ucBuffer[5]  = 0x06 ;
    pstSaapHand->ucBuffer[6]  = 0x07 ;
    pstSaapHand->ucBuffer[7]  = 0x08 ;
    pstSaapHand->ucBuffer[8]  = 0x09 ;
    pstSaapHand->ucBuffer[9]  = 0x10 ;

    pstSaapHand->com.stReceiver.ucModId = 8;//SYS_ThisModIdGet() ;
    pstSaapHand->com.stReceiver.ucFId   = FID_SAAP;
    pstSaapHand->com.stReceiver.usFsmId = 0xFFFF ;
    pstSaapHand->com.stSender.ucModId   = 8;//SYS_ThisModIdGet();
    pstSaapHand->com.stSender.ucFId     = 1;
    pstSaapHand->com.stSender.usFsmId   = 0xFFFF;
    pstSaapHand->com.usMsgId            = SAAP_DATA_REQ ; ;
    pstSaapHand->com.usMsgLen           = sizeof(APP_OR_SAAP_COMMON_HEADER) + 10  - 1 ;     //后续长度之和

    SYS_MSGSEND((COMMON_HEADER_SAAP *) pstSaapHand );

    return XSUCC ;

}

// 此函数用于比对GT号段是否为本机号段
XU16 saap_check_no_proc(XU32 proTag, XU8 flag , XU16 hlrno, XU32 dnno)
{

    XU16 i , j  ;
    XU32  tempNo ;
    XU32 dstNodeType;
    XU32 ulLocalCfgNum = 0;
    
    //此项判断保留为以后扩展用,目前暂不提供STP类型
    /*if(SAAP_NODE_TYPE_STP == g_ulSAAPGlobal.pidNo.nodeType) //如果是令转接点,直接返回
    {
        return 0;
    }*/
    switch(proTag)
    {
        case SAAP_SMAP_MSE:
            dstNodeType = SAAP_NODE_TYPE_HLR;
            break;
        case SAAP_SHAP_MSE:
            dstNodeType = SAAP_NODE_TYPE_SXC;
            break;
        case SAAP_SHLR2SMC_MSE:
        case SAAP_SXC2SMC_MSE: //此项也可能是smc->sxc,但smc的判断兼容SXC的判断
        case SAAP_SMC2SMC_MSE:
            dstNodeType = SAAP_NODE_TYPE_SMC;
            break;
        /*20111215 cxf add begin: add rbt route func*/
        case SAAP_X2RTB_MSE:
            dstNodeType = SAAP_NODE_TYPE_RBT;
            break;
        /*20111215 cxf add end*/
        default:
            break;

    }

    switch( flag )
    {
        case SAAP_GST_UID_TO_SHLR:
            {
                if(dstNodeType != g_ulSAAPGlobal.eidNoCfg.nodeType)
                {
                    return 0;
                }
                for ( j = 0 ; j < SAAP_LOCAL_PREFIX_NUM ; j++)
                {
                    // 比对数据有效标致,如果为 0 则表示无效,直接进入下一轮比对
                    if ( g_ulSAAPGlobal.hlrNoCfg[j].flag == 0 )
                    {
                        continue ;
                    }
                    ulLocalCfgNum++;
                    if( hlrno == g_ulSAAPGlobal.hlrNoCfg[j].hlrNo )
                    {
                        return 1 ;  // 返回成功
                    }
                }



                // zhanghai modify 090311    SAC 如果一个不配，收消须
                //if(  SAAP_LOCAL_PREFIX_NUM == j )
                if(0 == ulLocalCfgNum) /*20100507 cxf modify*/
                {
                    return 1 ;  // 返回成功
                }    
                // by end

                
            }
            break;

        case SAAP_GST_DN_TO_SHLR:
            {
                if(dstNodeType != g_ulSAAPGlobal.eidNoCfg.nodeType)
                {
                    return 0;
                }
                tempNo = dnno ;

                for ( i = 0 ; i < 6 ; i++ )
                {
                    // 号段比对

                    for ( j = 0 ; j < SAAP_LOCAL_PREFIX_NUM ; j++)
                    {
                        // 比对数据有效标致,如果为 0 则表示无效,直接进入下一轮比对
                        if ( g_ulSAAPGlobal.tellNoCfg[j].flag == 0 )
                        {
                            continue ;
                        }

                        // 比对值
                        if( tempNo == g_ulSAAPGlobal.tellNoCfg[j].tellNo )
                        {
                            return 1 ; // 返回成功
                        }
                    }

                    tempNo = tempNo / 10 ;


                    // zhanghai modify 090311    SAC 如果一个不配，收消须
                    if(  SAAP_LOCAL_PREFIX_NUM == j && SAAP_ROUTE_TBL_HLR == dstNodeType)
                    {
                        return 1 ;  // 返回成功
                    }    
                    // by end

                    
                }
            }
            break;

        case SAAP_GST_EID_TO_SHLR:
            {

                // 比对数据有效标致,如果为 0 则表示无效,直接进入下一轮比对
                /*20090326 addvised by zhanghai*/
                if (XOS_NtoHl(dnno) == 0xfeffffff || XOS_NtoHl(dnno) == 0xfffffffe)
                {
                   return 1;
                }
                /*20090326 addvised by zhanghai*/
                if ( g_ulSAAPGlobal.eidNoCfg.flag == 0 )
                {
                    return 0 ;
                }

                if( dnno == g_ulSAAPGlobal.eidNoCfg.eidNo )
                {
                    return 1 ;  // 返回成功
                }

            }
            break ;
        default:
            break ;
    }

    //返回不匹配
    return 0 ;

}
XS32  XOS_FIDSAAP(HANDLE hDir,XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST NTLLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &NTLLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    NTLLoginList.stack     = &g_SaapFidInfo;
    XOS_StrNcpy(NTLLoginList.taskname , "Tsk_SAAP", MAX_TID_NAME_LEN);

#ifdef MAKE_SXC_LINUX    
    NTLLoginList.TID        = 221;
#else
    NTLLoginList.TID        = FID_SAAP;
#endif    
    NTLLoginList.prio      = TSK_PRIO_NORMAL;
    NTLLoginList.quenum = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&NTLLoginList,XNULLP, XNULLP);
    return ret;
}

int RegSaapCli(XU32 retId)
{

    XOS_RegistCommand(retId,
        saap_showsaapinfo,
        "saapinfo",
        "display saap configuration",
        "saapinfo");
        //"[item 1-local 2- eidR 3-UidR,4-TellR,255-all],[epType 1-hlr,2-smc,255-all]");

    XOS_RegistCommand(retId,
        saap_setlocalep,
        "seteid",
        "config saap local eid,NodeType",
        "[eid][nodeType 0-hlr,1-smc,2-sxc]");

    XOS_RegistCommand(retId,
        saap_setlocaluidprefix,
        "setuidpre",
        "config saap local uid prefix",
        "[index,uidpre]");

    XOS_RegistCommand(retId,
        saap_setlocaltellnoprefix,
        "settellpre",
        "config saap local tell prefix",
        "[index,tellpre]");

    XOS_RegistCommand(retId,
        saap_deleid,
        "deleid",
        "del saap local eid",
        "eid");

    XOS_RegistCommand(retId,
        saap_deluidprifix,
        "deluidpre",
        "del saap local uid prefix",
        "index");

    XOS_RegistCommand(retId,
        saap_deltellnoprefix,
        "deltellpre",
        "del saap local tell prefix",
        "index");

    XOS_RegistCommand(retId,
        saap_addeidroute,
        "addeidr",
        "add eid route",
        "eid ,dstDp(0x)");

    XOS_RegistCommand(retId,
        saap_adduidroute,
        "adduidr",
        "add uid prefix route",
        "uidPreFix ,epType(0-hlr,1-smc),dstDp(0x)");

    XOS_RegistCommand(retId,
        saap_addtellnoroute,
        "addtellr",
        "add tellno prefix route",
        "TellNoPreFix ,epType(0-hlr,1-smc),dstDp(0x)");

    XOS_RegistCommand(retId,
        saap_delEidRoute,
        "deleidr",
        "del eid route",
        "Eid");

    XOS_RegistCommand(retId,
        saap_delUidRoute,
        "deluidr",
        "del uid prefix route",
        "uidPreFix,epType(0-hlr,1-smc)");

    XOS_RegistCommand(retId,
        saap_delTellNoRoute,
        "deltellr",
        "del Tell No Route",
        "tellPre,epType(0-hlr,1-smc)");

    XOS_RegistCommand(retId,
        SaapGetDstDpByGt,
        "getdp",
        "getDstId by gt",
        "Rmode(1-tell,8-EID,9-UID),epType(0-HLR,1-SMC),GT");

    XOS_RegistCommand(retId,
        saap_statclear,
        "statclear",
        "clear saap stat",
        "no para");

    XOS_RegistCommand(retId,
        saap_showstat,
        "showstat",
        "show saap stat",
        "no para");

    XOS_RegistCommand(retId,
        saap_setstatswitch,
        "setstatswitch",
        "set saap stat switch flag",
        "0-off;1-on;2-clear stat");


    XOS_RegistCommand(retId,
        SaapTest,
        "saaptest",
        "saap fun test",
        "protag(0x80 ->HLR; 0x90,0xB0,0xC0 ->SMC; 0xA0 SXC<->SXC),RouteType(1-dn,8-EID,9-UID),GT");

    XOS_RegistCommand(retId,
        saap_SetTestData,
        "settestdata",
        "set saap test data",
        "protag(0x80 ->HLR; 0x90,0xB0,0xC0 ->SMC; 0xA0 SXC<->SXC),Eid,Uid,TellNo\r\n settestdata 0x80 15 0x13730102 56789");

    XOS_RegistCommand(retId,
        saap_StartTestTimer,
        "starttest",
        "start test timer",
        "Peiod(second),numPerPeriod\r\n(starttest 1 100)");

    XOS_RegistCommand(retId,
        saap_StopTestTimer,
        "stoptest",
        "stop test timer",
        "no para");

    XOS_RegistCommand(retId,
        saap_CliSetFilter,
        "setfilter",
        "set saap filter data",
        "linkHandle,ip,port,codePlan,gt");

    XOS_RegistCommand(retId,
        saap_CliClsFilter,
        "clsfilter",
        "clear saap filter data",
        "ip,port");
    XOS_RegistCommand(retId,
        saap_CliShowFilter,
        "showfilter",
        "show saap filter data",
        "no para");
    
    XOS_RegistCommand(retId,
        saap_showUidSeg,
        "showuidseg",
        "show uid segment config",
        "no para");
    return 0;
}

XS32 saap_regCmd()
{
    /*---------------命令行注册部分---------------------*/
    XS32 ret = XOS_RegistCmdPrompt(SYSTEM_MODE,"saap","SAAP","no paramater");
    XU32 iret = (XU32) ret;
    RegSaapCli(iret);
    return XSUCC;
}

XVOID saap_showsaapinfo(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
 //   int i  = 0;
//    int recordnum = 0;
    int showItem  =0xFF; //查看项
    XU32 ulFlag = 0xFF;
    if(XNULL == ppArgv)
    {
        return;
    }
    if(siArgc > 1)
    {
        showItem = XOS_ATOI(ppArgv[1]);
    }
    if(siArgc > 2)
    {
        ulFlag = XOS_ATOI(ppArgv[2]);
    }

    if( 1 == showItem || 0xff == showItem)
    {
        saap_showlocalinfo(pCliEnv);
    }

    if( 2 == showItem || 0xff == showItem)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n");
        saap_showeidroute(pCliEnv, ulFlag);
    }

    if( 3 == showItem || 0xff == showItem)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n");
        saap_showhlrroute(pCliEnv, ulFlag);
    }

    if(4 == showItem || 0xff == showItem)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n");
        saap_showsmcroute(pCliEnv, ulFlag);
    }

    if(5 == showItem || 0xff == showItem)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n");
        saap_showrbtroute(pCliEnv, ulFlag);        
    }
    return;
}

XVOID saap_showlocalinfo(CLI_ENV* pCliEnv)
{
    XS32 i = 0;
    XOS_CliExtPrintf(pCliEnv,"------------------saap local info begin-----------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"validFlag =%d\r\n",g_ulSAAPGlobal.eidNoCfg.flag);
    XOS_CliExtPrintf(pCliEnv,"localEID  =%d\r\n",g_ulSAAPGlobal.eidNoCfg.eidNo);
    XOS_CliExtPrintf(pCliEnv,"nodeType  =%d(0:hlr 1:smc 2:sxc 3:rbtc)\r\n",g_ulSAAPGlobal.eidNoCfg.nodeType);

    XOS_CliExtPrintf(pCliEnv,"------------------local uid prefix----------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"index\t uidPrefix\r\n");
    for(i = 0 ;i < SAAP_LOCAL_PREFIX_NUM;i++)
    {
        if(g_ulSAAPGlobal.hlrNoCfg[i].flag)
        {
            XOS_CliExtPrintf(pCliEnv,"[%d]\t 0x%x\r\n",i,g_ulSAAPGlobal.hlrNoCfg[i].hlrNo);
        }
    }

    XOS_CliExtPrintf(pCliEnv,"------------------local tell prefix---------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,"index\t tellNoPreFix\r\n");
    for(i = 0 ;i < SAAP_LOCAL_PREFIX_NUM;i++)
    {
        if(g_ulSAAPGlobal.tellNoCfg[i].flag)
        {
            XOS_CliExtPrintf(pCliEnv,"[%d]\t %d\r\n",i,g_ulSAAPGlobal.tellNoCfg[i].tellNo);
        }
    }
    XOS_CliExtPrintf(pCliEnv,"------------------saap local info end-------------------\r\n");
    return;
}
XVOID saap_showeidroute(CLI_ENV* pCliEnv,XU32 ulFlag)
{
    XU32 i  =0;
    XU32 recordnum  = 0;
    XOS_CliExtPrintf(pCliEnv,"------------------EID ->Route Table---------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"EID\t\t DstDpCode(0x)\r\n");
    for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
    {
        if(BLANK_USHORT == gSaapPidToDstIDCcTbl[i].DstID)
        {
            continue;
        }
        recordnum++;
        XOS_CliExtPrintf(pCliEnv,"%d\t\t %04x\r\n",gSaapPidToDstIDCcTbl[i].pidNo,gSaapPidToDstIDCcTbl[i].DstID);
    }
    return;
}

XVOID saap_showhlrroute(CLI_ENV* pCliEnv,XU32 ulFlag)
{
    int i  = 0;
    int recordnum = 0;

    if(1== ulFlag || 0xff == ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"------------------UID->HLR Route Table------------------\r\n");
        recordnum = 0;
        XOS_CliExtPrintf(pCliEnv,"UIDP(0x)\t\t DstDpCode(0x)\r\n");
        gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_HLR][0];
        for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
        {
            if(BLANK_USHORT == gSaapHlrToDstIDCcTbl[i].DstID)
            {
                continue;
            }
            recordnum++;
            XOS_CliExtPrintf(pCliEnv,"%x\t\t %04x\r\n",gSaapHlrToDstIDCcTbl[i].hlrNo,gSaapHlrToDstIDCcTbl[i].DstID);
        }

    }

    if(2 == ulFlag || 0xff == ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"------------------TELL->HLR Route Table-----------------\r\n");
        recordnum = 0;
        XOS_CliExtPrintf(pCliEnv,"TellNo\t\t DstDpCode(0x)\r\n");
        gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_HLR][0];
        for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
        {
            if(BLANK_USHORT == gSaapTelToDstIDCcTbl[i].DstID)
            {
                continue;
            }
            recordnum++;
            XOS_CliExtPrintf(pCliEnv,"%d\t\t %04x\r\n",gSaapTelToDstIDCcTbl[i].tellNo,gSaapTelToDstIDCcTbl[i].DstID);
        }
    }

    return;
}

XVOID saap_showsmcroute(CLI_ENV* pCliEnv,XU32 ulFlag)
{
    int i  = 0;
    int recordnum = 0;

    if(1 == ulFlag || 0xff ==ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"------------------UID->SMC Route Table------------------\r\n");
        recordnum = 0;
        XOS_CliExtPrintf(pCliEnv,"UIDP(0x)\t\t DstDpCode(0x)\r\n");
        gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_SMC][0];//88888
        for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
        {
            if(BLANK_USHORT == gSaapHlrToDstIDCcTbl[i].DstID)
            {
                continue;
            }
            recordnum++;
            XOS_CliExtPrintf(pCliEnv,"%x\t\t %04x\r\n",gSaapHlrToDstIDCcTbl[i].hlrNo,gSaapHlrToDstIDCcTbl[i].DstID);
        }
    }

    if(2 == ulFlag || 0xff == ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"------------------TELL->SMC Route Table-----------------\r\n");
        recordnum = 0;
        XOS_CliExtPrintf(pCliEnv,"TellNo\t\t DstDpCode(0x)\r\n");
        gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_SMC][0];
        for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
        {
            if(BLANK_USHORT == gSaapTelToDstIDCcTbl[i].DstID)
            {
                continue;
            }
            recordnum++;
            XOS_CliExtPrintf(pCliEnv,"%d\t\t %04x\r\n",gSaapTelToDstIDCcTbl[i].tellNo,gSaapTelToDstIDCcTbl[i].DstID);
        }
        XOS_CliExtPrintf(pCliEnv,"--------------------------------------------------------\r\n");
    }

    return;
}

XVOID saap_showrbtroute(CLI_ENV* pCliEnv,XU32 ulFlag)
{
    int i  = 0;
    int recordnum = 0;

    if(2 == ulFlag || 0xff == ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"------------------TELL->RBTC Route Table-----------------\r\n");
        recordnum = 0;
        XOS_CliExtPrintf(pCliEnv,"TellNo\t\t DstDpCode(0x)\r\n");
        gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_RBT][0];
        for( i =0;i<SAAP_MAX_SAAPCCB_NUM;i++)
        {
            if(BLANK_USHORT == gSaapTelToDstIDCcTbl[i].DstID)
            {
                continue;
            }
            recordnum++;
            XOS_CliExtPrintf(pCliEnv,"%d\t\t %04x\r\n",gSaapTelToDstIDCcTbl[i].tellNo,gSaapTelToDstIDCcTbl[i].DstID);
        }
        XOS_CliExtPrintf(pCliEnv,"--------------------------------------------------------\r\n");
    }

    return;
}


/*
config local saap info
*/
XVOID saap_setlocalep(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 Eid = 0;
    XU32 nodeType  = 0;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    Eid = XOS_ATOI(ppArgv[1]);
    nodeType = XOS_ATOI(ppArgv[2]);
    if(SAAP_NODE_TYPE_MAX <= nodeType)
    {
         XOS_CliExtPrintf(pCliEnv,"node type err.",nodeType);
         return;
    }
    if(XSUCC != saap_setLocalEID( Eid,nodeType) )
    {
        XOS_CliExtPrintf(pCliEnv,"set Eid fail.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set Eid ok.");
    }
    return;
}

XVOID saap_setlocaluidprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 index = 0;
//    XS32 ulFlag = 0;
    XU32 hlrNo  = 0;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    index = XOS_ATOI(ppArgv[1]);
    if(SAAP_LOCAL_PREFIX_NUM <= index)
    {
        XOS_CliExtPrintf(pCliEnv,"index should < %d",SAAP_LOCAL_PREFIX_NUM);
        return;
    }
    XOS_StrToNum(ppArgv[2],&hlrNo);
    if(XERROR != saap_SetSaapSstNoID( SAAP_OAM_CF_HLRNO_FLAG , index, 1 , hlrNo , 0 ) )
    {
        XOS_CliExtPrintf(pCliEnv,"set uid prefix ok.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set uid prefix fail.");
    }

    return;
}

XVOID saap_setlocaltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 index = 0;
    XU32 tellNo = 0;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    index = XOS_ATOI(ppArgv[1]);
    if(SAAP_LOCAL_PREFIX_NUM <= index)
    {
        XOS_CliExtPrintf(pCliEnv,"index should < %d",SAAP_LOCAL_PREFIX_NUM);
        return;
    }
    //tellNo= SYS_Str2Bcd(ppArgv[2]); 8888
    tellNo = XOS_ATOI(ppArgv[2]);
    if(XERROR !=saap_SetSaapSstNoID( SAAP_OAM_CF_DN_FLAG , index , 1 , 0 , tellNo ) )
    {
        XOS_CliExtPrintf(pCliEnv,"set tellno prefix ok.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set tellno prefix fail.");
    }
    return;

}

XVOID saap_deleid(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    if(XSUCC != saap_setLocalEID( BLANK_ULONG,0) )
    {
        XOS_CliExtPrintf(pCliEnv,"set Eid fail.");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set Eid ok.");
    }
    return;
}
XVOID saap_deluidprifix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 index = 0;
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    index = XOS_ATOI(ppArgv[1]);
    if(XSUCC != saap_SetSaapSstNoID( SAAP_OAM_CF_HLRNO_FLAG , index, 0 , 0 , 0 ) )
    {
        XOS_CliExtPrintf(pCliEnv,"del uid prifix fail.");
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"del uid prifix ok.");
    return;
}

XVOID saap_deltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 index = 0;
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    index = XOS_ATOI(ppArgv[1]);
    if(XSUCC != saap_SetSaapSstNoID( SAAP_OAM_CF_DN_FLAG , index, 0 , 0 , 0 ) )
    {
        XOS_CliExtPrintf(pCliEnv,"del tellno prifix fail.");
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"del tellno prifix ok.");
    return;
}

XVOID saap_addeidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 Eid = 0;
    XU32 DstID  = 0;
    XU32 ulFlag = BLANK_ULONG;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    Eid = XOS_ATOI(ppArgv[1]);
    XOS_StrToNum(ppArgv[2],&DstID);
    ulFlag = saap_setGTTable(SAAP_OP_EID_TABLE_TAG, Eid,(XU16)DstID,0);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"add eid(%d) router fail.",Eid);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"add eid(%d) router ok.",Eid);
    return;

}

XVOID saap_adduidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 hlrNo = 0;
    XU32 DstID  = 0;
    XU32 epType  = 0;
    XU32 ulFlag = BLANK_ULONG;
    if(siArgc < 4)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    XOS_StrToNum(ppArgv[1],&hlrNo);
    epType =XOS_ATOI(ppArgv[2]);
    XOS_StrToNum(ppArgv[3],&DstID);
    if(SAAP_ROUTE_TBL_MAX <= epType)
    {
        XOS_CliExtPrintf(pCliEnv,"ep type error.");
        return;
    }

    ulFlag = saap_setGTTable(SAAP_OP_HLR_TABLE_TAG, hlrNo,(XU16)DstID,epType);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"add uid(%x)-(%d) router fail.",hlrNo,epType);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"add uid(%x)-epType(%d) ->0x%04x router ok.",hlrNo,epType,DstID);
    return;

}

XVOID saap_addtellnoroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32  tellNo;
    XU32 DstID  = 0;
    XU32 epType  = 0;
    XU32 ulFlag = BLANK_ULONG;
    if(siArgc < 4)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    if(strlen(ppArgv[1]) > 8)
    {
        XOS_CliExtPrintf(pCliEnv,"tellno'len should < 8");
        return;
    }

    tellNo = XOS_ATOI(ppArgv[1]);
    epType =XOS_ATOI(ppArgv[2]);
    XOS_StrToNum(ppArgv[3],&DstID);
    if(SAAP_ROUTE_TBL_MAX <= epType)
    {
        XOS_CliExtPrintf(pCliEnv,"ep type error.");
        return;
    }

    ulFlag = saap_setGTTable(SAAP_OP_TEL_TABLE_TAG, tellNo, (XU16)DstID,epType);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"add tellno(%d)-eptype(%d) router fail.",tellNo,epType);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"add router tellno(%d)-epType(%d)->%04x  ok.",tellNo,epType,DstID);
    return;

}

XVOID saap_delEidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ulFlag = 0;
    XU32 Eid  = 0;

    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    Eid = XOS_ATOI(ppArgv[1]);
    ulFlag = saap_delGTTable(SAAP_OP_EID_TABLE_TAG, Eid,0);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"del eid(%d) route fail.",Eid);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"del eid(%d) route ok.",Eid);
    return;
}

XVOID saap_delUidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ulFlag = 0;
    XU32 hlrNo  = 0;
    XU32 epType =0;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    XOS_StrToNum(ppArgv[1],&hlrNo);
    epType = XOS_ATOI(ppArgv[2]);
    if(SAAP_ROUTE_TBL_MAX <= epType)
    {
        XOS_CliExtPrintf(pCliEnv,"ep type error.");
        return;
    }
    ulFlag = saap_delGTTable(SAAP_OP_HLR_TABLE_TAG,(XU16)hlrNo,epType);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"del uid (0x%x) route fail.",hlrNo);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"del uid (0x%x)-%d route ok.",hlrNo,epType);
    return;
}

XVOID saap_delTellNoRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ulFlag = 0;
    XU32 tellNo  = 0;
    XU32 epType = 0;
    if(siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num error");
        return;
    }
    tellNo = XOS_ATOI(ppArgv[1]);
    epType = XOS_ATOI(ppArgv[2]);
    if(SAAP_ROUTE_TBL_MAX <= epType)
    {
        XOS_CliExtPrintf(pCliEnv,"ep type error.");
        return;
    }
    ulFlag = saap_delGTTable(SAAP_OP_TEL_TABLE_TAG, tellNo,epType);
    if(XSUCC != ulFlag)
    {
        XOS_CliExtPrintf(pCliEnv,"del tellno(%d) fail.",tellNo);
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"del tellno(%d)-%d ok.",tellNo,epType);
    return;
}
/********************************** 
函数名称    : saap_statclear
作者        : 
设计日期    : 2008年9月24日 CodeReview
功能描述    : 
参数        : CLI_ENV *pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID 
************************************/
XVOID saap_statclear(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
     XOS_MemSet(&g_SaapStaData, 0, sizeof(t_SAAPSTAT));
     XOS_CliExtPrintf(pCliEnv,  "\r\n cls saap statistic ok.");
}


/********************************** 
函数名称    : saap_showstat
作者        : ..
设计日期    : 2008年9月25日
功能描述    : 查询统计结果
参数        : CLI_ENV *pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值        : XVOID 
************************************/
XVOID saap_showstat(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 ulCount  = 5;//8888 5是什么
    XU32 i  =0;

    XOS_CliExtPrintf(pCliEnv, "\r\n-----------------------------------------------------------------");
    XOS_CliExtPrintf(pCliEnv,"\r\nsend fail num=%d",g_SaapStaData.ulLinkFailNum);
    XOS_CliExtPrintf(pCliEnv,"\r\nsaap routenum=%d",g_SaapStaData.ulRouteMsgCnt);

    for (i =0;i< ulCount ;i++)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\nRevMsgCnt[%d]=%d", i,g_SaapStaData.ulRecvMsgCnt[i]);
    }
    XOS_CliExtPrintf(pCliEnv, "\r\n-----------------------------------------------------------------");
    for (i =0;i< ulCount ;i++)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\nSendMsgCnt[%d]=%d",i, g_SaapStaData.ulSendMsgCnt[i]);
    }

    XOS_CliExtPrintf(pCliEnv, "\r\n(stat Index:0:XXX->HLR,1:HLR->SMC,2:SXC<->SXC,3:SXC<->SMC,4:SMC<->SMC)");
    XOS_CliExtPrintf(pCliEnv, "\r\n-----------------------------------------------------------------");

    return;

}
// 设置统计开关属性
/********************************** 
函数名称    : saap_setstatswitch
作者        : 
设计日期    : 2008年9月24日 CodeReview
功能描述    : 
参数        : CLI_ENV *pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID 
Review Error
************************************/
XVOID saap_setstatswitch(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU8  sw;

    if( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "input para num error\r\n");
        return;
    }

    sw = (XU8)XOS_ATOL(ppArgv[1]);
    if( (0 != sw) && (1 != sw) && (2 != sw) )
    {
        XOS_CliExtPrintf(pCliEnv, "input para error\r\n");
        return;
    }

    if( 0 == sw )
    {
        g_saapStatFlag = 0;
        XOS_CliExtPrintf(pCliEnv,  "saap statistic switch off.\r\n");
    }
    else if( 1 == sw)
    {
        g_saapStatFlag = 1;
        XOS_CliExtPrintf(pCliEnv,  "saap statistic switch on.\r\n");
    }
    else if( 2 == sw)
    {
        XOS_MemSet(&g_SaapStaData, 0, sizeof(t_SAAPSTAT));

        XOS_CliExtPrintf(pCliEnv,  "clear saap statistic ok\r\n");
    }
    return;

}

XS32 saapinvoketcpe()
{
    XOS_FIDTCPE(0,1,0);
    return XSUCC;
}
/*
SAAP Test func
saaptest protag, routeType, GT
*/
XVOID SaapTest(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 ProTag = 0;
    XU32 routeType = 0;
    XU32 gtValue;
    XU8 DN[8];
    XU8 TellNo[16];

    XOS_MemSet(TellNo,0,sizeof(TellNo) );
    if(4 > siArgc)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\npara num err.");
        return;
    }

    XOS_StrToNum(ppArgv[1],&ProTag);
    if(SAAP_SMAP_MSE != ProTag
        &&  SAAP_SHLR2SMC_MSE != ProTag
        && SAAP_SHAP_MSE != ProTag
        && SAAP_SXC2SMC_MSE != ProTag
        && SAAP_SMC2SMC_MSE != ProTag
        && SAAP_X2RTB_MSE != ProTag /*20111215 cxf add begin: add rbt route func*/
        )
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n protag err.(0X80-(smap SXC/SMC/HLR->HLR),0X90(HLR->smc),0XA0 (hap,SXC<->SXC>,0xB0(sxc->smc),0Xc0(SMC<->SMC>)");
        return;
    }

    routeType = XOS_ATOI(ppArgv[2]);
    if(SAAP_GST_DN != routeType
       && SAAP_GST_EID != routeType
       && SAAP_GST_UID != routeType)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n route type err.(1- by dn,8- by Eid,9- by Uid)");
        return;
    }
    if(SAAP_GST_DN == routeType)
    {
        strcpy(TellNo,ppArgv[3]);
        SAAP_StrTelToDn(TellNo, DN);
    }
    else
    {
        XOS_StrToNum(ppArgv[3], &gtValue);
    }

    SaapSendTestMsg(ProTag, routeType, gtValue, TellNo);
    return;
}

XU32 SaapGetLocalEid()
{
    return g_ulSAAPGlobal.eidNoCfg.eidNo;
}

/**
*
*/
XU32 SaapGetLocalTell()
{
    XS32 i = 0;

    for(i = 0; i < SAAP_LOCAL_PREFIX_NUM; i++ )
    {
        if(g_ulSAAPGlobal.tellNoCfg[i].flag)
        {
            return g_ulSAAPGlobal.tellNoCfg[i].tellNo;
        }
    }
    return 0xFFFFFFFF;
}

XU32 SaapNum2Bcd(XU32 ulNum,char *pBcd,int *pLen)
{
    int Len = 0;
    char szTmp[128] = {0};   
    
    sprintf(szTmp,"%d",ulNum);

    SAAP_NumCvt2Bcd(szTmp, pBcd, pLen);
    return XSUCC;
}

XU32 SaapRBcd2Bcd(XU32 RBcd)
{
    int i  = 0;
    char *pchr = 0;
    XU32 bcdVal = 0;

    bcdVal = RBcd;
    pchr = &bcdVal;
    for(i = 0; i < sizeof(XU32); i++)
    {
        *pchr = ( ( (*pchr) & 0x0F) << 4) | ( ((*pchr) >> 4) & 0x0F);
		*pchr++;
    }
    return bcdVal;
}

XU32 SaapXwBcd2Bcd(XU32 RcvBcd)
{
    int i  = 0;
    XU32 bcdBak = 0;
    char cTmp = 0;
    char szTmp[32] ={0};
    int sLen = 0;
    bcdBak = RcvBcd;

    bcdBak = SaapRBcd2Bcd(bcdBak);
    // delete 0xf
    for(i  =0; i < 8; i++)
    {        
        cTmp = bcdBak & 0x0F;
        if(0xF != cTmp)
        {
            break;
        }
        else
        {
            bcdBak = (bcdBak >> 4);
        }
    }    
    return bcdBak;
}
    
/**
*rcvBcd 0x12345 0x21435FFF
*num return 12345
*/
XU32 SaapBcd2Num(XU32 RcvBcd)
{
    int i  = 0;
    XU32 bcdBak = 0;
    char cTmp = 0;
    char szTmp[32] ={0};
    int sLen = 0;
    bcdBak = RcvBcd;

    // delete 0xf
    for(i  =1; i < 7; i++)
    {        
        cTmp = bcdBak & 0x0F;
        if(0xF != cTmp)
        {
            break;
        }
        else
        {
            bcdBak = (bcdBak >> i*4);
        }
    }

    bcdBak = SaapRBcd2Bcd(bcdBak);
    sprintf(szTmp,"%x",bcdBak);
    bcdBak = atoi(szTmp);
    return bcdBak;
}

XS32 SaapGetLocalBcdTell(char *pBcd,int *pLen)
{
    int i = 0;
    XS32 numTell = 0xFFFFFFFF;

    for(i = 0; i < SAAP_LOCAL_PREFIX_NUM; i++ )
    {
        if(g_ulSAAPGlobal.tellNoCfg[i].flag)
        {            
            numTell =  g_ulSAAPGlobal.tellNoCfg[i].tellNo;
        }
    }
    SaapNum2Bcd(numTell,pBcd,pLen);
    return XSUCC;
}

/**
*get TellNo Route data
*@para nodeType 0-Hlr,1-SMC,2-SXC,3-RBTC
*@para *pNum ,outputPara
*@para aucRouteTell,output para, array upbound is 255
*chenxiaofeng 20120116
*
*/
XS32 SaapGetLocalRouteTell(XU32 nodeType,XU32 *pNum,XU32 aulRouteTell[])
{
    int i = 0;
    int isum = 0;
    SAAP_TELL_INDEX_DSTDEVICEID_CCB *pSaapTelTbl = NULL;

    if(nodeType >= SAAP_NODE_TYPE_MAX)
    {
        return XERROR;
    }
    *pNum =0;
    
    pSaapTelTbl = &stSaapTelToDstIDCcTbl[nodeType][0];
    for(i = 0; i < SAAP_MAX_SAAPCCB_NUM; i++)
    {
        if(BLANK_USHORT != pSaapTelTbl[i].DstID )
        {            
            aulRouteTell[isum] = pSaapTelTbl[i].tellNo;
            isum++;
        }
    }
    *pNum = isum;
    return XSUCC;
}

XS32 SaapSendTestMsg(XU32 ProTag, XU32 routeType,XU32 gtValue,XU8 tellNo[8])
{
    XU8 DN[8];
    XU16 usMsgLen = 0;
    SAAP_MSG_TEST_MSG *stSaapTestMsg = XNULL;
    t_XOSCOMMHEAD* pxosMsg = XNULL;

     XOS_MemSet(&stSaapTestMsg,0,sizeof(stSaapTestMsg)) ;
    usMsgLen = sizeof(APP_AND_SAAP_PACKAGE_HEAD)  + sizeof(DN) + sizeof(XU32); // DN ,UID
    pxosMsg= (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SAAP, usMsgLen);
    if(XNULL == pxosMsg)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"Saap SendTestMsg malloc mem fail!r\n");
        return XERROR;
    }

    SAAP_StrTelToDn(tellNo, DN);

    stSaapTestMsg = (SAAP_MSG_TEST_MSG *)pxosMsg;

    stSaapTestMsg->MsgHead.datadest.PID = XOS_GetLocalPID();
    stSaapTestMsg->MsgHead.datadest.FID = FID_SAAP;
    stSaapTestMsg->MsgHead.datadest.FsmId = 0;

    stSaapTestMsg->MsgHead.datasrc.PID = XOS_GetLocalPID();
    stSaapTestMsg->MsgHead.datasrc.FID = FID_SAAP; //暂用此FID发消息
    stSaapTestMsg->MsgHead.datasrc.FsmId = 666;

    stSaapTestMsg->MsgHead.prio = eNormalMsgPrio;
    stSaapTestMsg->MsgHead.msgID = SAAP_DATA_REQ;
    stSaapTestMsg->MsgHead.subID = 999;
    stSaapTestMsg->MsgHead.length = usMsgLen;

    stSaapTestMsg->SaapHead.ProtocolTag               = (XU8)ProTag;
    stSaapTestMsg->SaapHead.uGst.GuestSerialNumTag    = (XU8)routeType;
    if(SAAP_GST_DN == routeType)
    {
        XOS_MemCpy(&stSaapTestMsg->SaapHead.uGst.uGstNID.GuestNetIDTell,DN,sizeof(DN) );
        strcpy(stSaapTestMsg->DN, tellNo);
    }
    else
    {
        stSaapTestMsg->ulGtValue = gtValue;
        stSaapTestMsg->SaapHead.uGst.uGstNID.GuestNetID = gtValue;
    }
    stSaapTestMsg->SaapHead.uInt.InviteSerialNumTag   = SAAP_INT_EID;
    stSaapTestMsg->SaapHead.uInt.uIntNID.InviteNetID  = 1;
    stSaapTestMsg->SaapHead.UserMsgLen                = usMsgLen - sizeof(APP_AND_SAAP_PACKAGE_HEAD);

        //发送消息
    if(XSUCC != XOS_MsgSend(pxosMsg))
    {
        XOS_Trace(MD(FID_SAAP,PL_EXP), "Saap sendTestMsg XOS_MsgSend() fail!");
        XOS_MsgMemFree(pxosMsg->datasrc.FID,  pxosMsg);
        return XERROR;
    }

    return XSUCC;
}

/* 提供给SAAP的根据ProTag得到接收者FID的接口函数*/
/*XS32 SAAP_getFidByProTag(XS32 proTab)
{
    return 1;
}*/

XVOID saap_SetTestData(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{

    if(5 != siArgc)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }
    XOS_StrToNum(ppArgv[1],&gTestProTag);
    XOS_StrToNum(ppArgv[2],&gTestEid);
    XOS_StrToNum(ppArgv[3],&gTestUid);
    XOS_MemCpy(&gTestTellNO, ppArgv[4],strlen(ppArgv[3])+1);
    XOS_CliExtPrintf(pCliEnv,"set test data ok.");
    return;
}
XVOID saap_CliSetFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    SAAP_MON_FILTER_T stSaapFilter;

    if(6 > siArgc)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.\r\n");
        return ;
    }

    XOS_StrToNum(ppArgv[1], &stSaapFilter.ullinkHandle);
    stSaapFilter.ulOptType = 1; // set filter
    XOS_StrtoIp(ppArgv[2], &stSaapFilter.ulSpaCliIpAddr);
    stSaapFilter.ulSpaCliPort = XOS_ATOI(ppArgv[3]);
    stSaapFilter.codePlan = XOS_ATOI(ppArgv[4]);
    XOS_StrToNum(ppArgv[5],&stSaapFilter.ulGt);
    SAAP_SetMonFilter(&stSaapFilter);
    return;
}

XVOID saap_CliShowFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 i  = 0;
    XCHAR ipAddr[20] = {0};

    XOS_CliExtPrintf(pCliEnv,"\r\nspaLinkHandle=%x,",gstSaapSpaCliData.linkHandle);
    XOS_CliExtPrintf(pCliEnv,"\r\n%-5s %-16s %-5s %-5s %-8s %-8s","Idx","ipAddr","Port","Type","GT(0x)","GT");
    XOS_MemSet(ipAddr,0,sizeof(ipAddr) );
    for(i= 0;i< SAAP_MAX_SPA_CLI_NUM;i++)
    {
         if(FLAG_NO == gstSaapSpaCliData.cliData[i].busyflag)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,"\r\n%-5d %-16s %-5d %-5d %-8x %-8d",i,
            SAAP_IptoStr(gstSaapSpaCliData.cliData[i].IpAddr,ipAddr),gstSaapSpaCliData.cliData[i].port,gstSaapSpaCliData.cliData[i].ucCodePlan,gstSaapSpaCliData.cliData[i].gtValue,gstSaapSpaCliData.cliData[i].gtValue);
    }
    XOS_CliExtPrintf(pCliEnv,"\r\ncliNum=%d",gstSaapSpaCliData.cliNum);
    XOS_CliExtPrintf(pCliEnv,"\r\n(Type: 1-DN,8-EID,9-UID,255-ALL)");
    return;
}

XVOID saap_CliClsFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    SAAP_MON_FILTER_T stSaapFilter;

    if(3 > siArgc)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return ;
    }

    stSaapFilter.ulOptType = 0; // cls filter
    XOS_StrtoIp(ppArgv[1], &stSaapFilter.ulSpaCliIpAddr);
    stSaapFilter.ulSpaCliPort = XOS_ATOI(ppArgv[2]);
    SAAP_SetMonFilter(&stSaapFilter);
    return;
}
/**/
XVOID saap_StopTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    if(XSUCC != XOS_TimerStop(FID_SAAP,pSaapTimer[SAAP_TIMER_TEST]) )
    {
        XOS_CliExtPrintf(pCliEnv,"stop timer fail.");
        return;
    }
    XOS_CliExtPrintf(pCliEnv,"stop timer succ.");
    return;
}
XVOID saap_StartTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 testPeriod = 1000;
    XU32 numPerSecond  = 1000;
    if( siArgc < 3)
    {
        XOS_CliExtPrintf(pCliEnv,"para num err.");
        return;
    }

    testPeriod = XOS_ATOI(ppArgv[1]);
    if( testPeriod < 1 )
    {
        testPeriod = 1;
    }
    numPerSecond = XOS_ATOI(ppArgv[2]);
    if( XSUCC != SAAP_StartTimer(&pSaapTimer[SAAP_TIMER_TEST], FID_SAAP, testPeriod * 1000, SAAP_TIMER_TEST, numPerSecond, TIMER_TYPE_LOOP) )
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n Start Tset Timer fail.");
    }
    XOS_CliExtPrintf(pCliEnv,"\r\n Start Tset Timer ok.");
    return;
}
XS32 SaapPeriodTest(XU32 numPerSecond)
{
    XU32 i =0;

    for(i =0;i< numPerSecond/3;i++)
    {
        SaapSendTestMsg(gTestProTag, 1,  gTestUid, gTestTellNO); // 1 mean route by tell
    }
    for(i =0;i< numPerSecond/3;i++)
    {
        SaapSendTestMsg(gTestProTag, 8,  gTestEid, gTestTellNO); // 8 mean route by eid
    }

    for(i =0;i< numPerSecond/3;i++)
    {
        SaapSendTestMsg(gTestProTag, 9,  gTestUid, gTestTellNO); // 9 mean route by uid
    }
    return XSUCC;

}

XU32 Saap_RegGetSrvFidByPro(SAAP_REGSRVFID_FUNC pFunc)
{
    if(XNULL != pFunc)
    {
        pSaapGetSrvFidByProFunc = pFunc;
    }
    return XSUCC;
}

XVOID saap_showUidSeg(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv)
{
    XS32 i = 0;
    XOS_CliExtPrintf(pCliEnv,"UidSegRouteFlag:%d",gUidSegCfg.ulUidSegRouteFlag);
    XOS_CliExtPrintf(pCliEnv,"\r\n%-3s %-8s %-8s  %-8s","Idx","UidBegin","UidEnd","dstDp");
    for(i = 0; i < MAX_UID_SEG_NUM; i++)
    {
        if(0 == gUidSegCfg.stUidSeg[i].ucFlag)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,"\r\n%-3d %08x %08x  %-8x",i,gUidSegCfg.stUidSeg[i].ulUidStart,gUidSegCfg.stUidSeg[i].ulUidEnd
            ,gUidSegCfg.stUidSeg[i].dstDpID);
    }
    return;
}

#ifdef __cplusplus
    }
#endif


