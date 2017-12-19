/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: xosnetinf.c
**
** description:  网口管理的接口封装 
**
** author: spj
**
** date:   2014.12.22
**
***************************************************************
**                         history                     
** 
***************************************************************
**  author          date              modification            
**  spj             20014.12.22       create  
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/*------------------------------------------------------------------------
                包含头文件
-------------------------------------------------------------------------*/ 
#include "xosenc.h"
#include "xosnetinf.h"
#include "xmlparser.h"
#include "xosencap.h"
#include "xosinet.h"
#include "xosfilesys.h"
#include "xostrace.h"
#include "xoscfg.h"
#include "xosipmi.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
                
#define MAX_GET_RANDOM_CNT 5  /* 如果获取的随机数与之前的相同，最大重新获取随机数次数 */
#define RANDOM_STR_LEN     10  /* 生成的随机字符串长度 */ 

#define  NetDevMapFile   "/usr/local/setnetip/networkmap.xml" 

/*-------------------------------------------------------------------------
                内部数据结构定义
-------------------------------------------------------------------------*/

static t_BoardNeMaptInfo g_tNetMapInfo;       /* 网口设备名称映射表 */

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/ 
XS32 XOS_GetDevIPList ( const XS8 *pdev, t_LOCALIPINFO * pLocalIPList );
static XS32 XOS_ParserNetDevMapXml(t_BoardNeMaptInfo *ptNetInfo);
static XS32 XOS_GetRandomNum(XS8 *randombuf, XU32 buflen);
static XS32 XOS_XU32ToMask(XU32 netmask);
static XBOOL XOS_DevNameIsValid(const XS8 *pLogicName,const XS8 *pinterface);
static XS32 XOS_GetNetPortEnum(XS8 *strLogicName, XS32 *piNetPortEnum);
static XS32 ENC_AddIp(XS8 *xinterface, XS8* ip,XS8* netmask);
static XS32 ENC_DelIp(XS8 *xinterface, XS8 *dev, XS8* ip,XU32 netmaskbit);


/*-------------------------------------------------------------------------
                函数定义
-------------------------------------------------------------------------*/ 

/************************************************************************
函数名: XOS_NetInfInit
功能：初始化设置网口模块
输入：
输出：
返回：XSUCC - 成功   XERROR 失败
说明: 
************************************************************************/
XS32 XOS_NetInfInit(XVOID)
{
    if (XSUCC != XOS_ParserNetDevMapXml(&g_tNetMapInfo))
    {
        printf("XOS_NetInfInit->parser net map failed!\n");
        return XERROR;
    }
    
    return XSUCC;
}
/************************************************************************
函数名: XOS_GetNetMap
功能：获取网口名称映射文件
输入：
输出：
返回：成功返回 解析好的映射结构体指针   失败返回 Null
说明: 
************************************************************************/
XS8* XOS_GetNetMap(XVOID)
{
    /* 全局变量未初始化 */
    if (!g_tNetMapInfo.init)
    {
        return (XS8*)NULL;
    }

    return (XS8*)&g_tNetMapInfo;
}

/************************************************************************
函数名: XOS_ParserNetDevMapXml
功能：解析设备名映射文件，并输出到结构体中
输入：
输出：ptNetInfo ，保存网口设备名信息的结构
返回：XSUCC - 成功   XERROR 或 1-10 - 失败
说明: 
************************************************************************/
static XS32 XOS_ParserNetDevMapXml(t_BoardNeMaptInfo *ptNetInfo)
{
	return 0;
#ifdef XOS_LINUX

    xmlDocPtr  doc      = NULL;
    xmlNodePtr cur      = NULL;
    xmlNodePtr level1cur  = NULL;
    xmlNodePtr level2cur = NULL;
    xmlNodePtr level3cur = NULL;
    xmlChar*   pTempStr = XNULL;
    t_BoardNeMaptInfo tNetInfo;
    XS8 boardname[MAX_BOARD_NAME_LEN] = {0};
    XS8 rtmname[MAX_BOARD_NAME_LEN] = "none";
    XS32 iInd = 0;  //找出各网口在map数组中的下标 enum值
    XS32 ret= 0;
    
    /*规则参数*/
    if(!ptNetInfo)
    {
        return XERROR;
    }

    XOS_MemSet(&tNetInfo, 0x00, sizeof(t_BoardNeMaptInfo));

     /* 获取单板型号 */
    if (XSUCC != XOS_GetBoardName(boardname,sizeof(boardname)))
    {
        printf("XOS_ParserNetDevMapXml: get board name fail!\n");
        return XERROR;
    }
    XOS_StrNcpy(tNetInfo.BoardName,boardname,XOS_MIN(XOS_StrLen(boardname),sizeof(tNetInfo.BoardName)));

    
    /* 获取rtm型号，如果没有后卡，rtm默认值为none */
    if (XSUCC != XOS_GetRtmName(rtmname,sizeof(rtmname)))
    {
        XOS_StrNcpy(tNetInfo.RtmName,"none",sizeof(tNetInfo.RtmName));
    }
    else
    {
        XOS_StrNcpy(tNetInfo.RtmName,rtmname,XOS_MIN(XOS_StrLen(rtmname),sizeof(tNetInfo.RtmName)));
    }
    
    /* 加记录信息 */
    printf("board name:%s\n",tNetInfo.BoardName);
    printf("rtm name:%s\n",tNetInfo.RtmName);

    /* 判断文件是否存在 */
    ret = access(NetDevMapFile, F_OK);
    if (ret)
    {
        printf("XOS_ParserNetDevMapXml: map file %s not exist! %m\n",NetDevMapFile);
        return XERROR; 
    }

    doc = xmlParseFile(NetDevMapFile);
    if (doc == XNULL)
    {
        printf("XOS_ParserNetDevMapXml: xml parase fail!\n");
        return XERROR;
    }

    /*找根节点*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR;
    }
    
    /*根节点*/
    if ( XOS_StrCmp( cur->name, "netdevice_map") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR;
    }
    level1cur = cur->xmlChildrenNode;
    while ( level1cur && xmlIsBlankNode ( level1cur ) )
    {
        level1cur = level1cur->next;
    }
    if ( level1cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR;
    }
    
    /*board主节点*/
    while ( level1cur != XNULL )
    {
        if ( !XOS_StrCmp(level1cur->name, "board" ) )
        {
            pTempStr = xmlGetProp(level1cur, (xmlChar*)"name");
            if ( pTempStr && !XOS_StrCmp(pTempStr, tNetInfo.BoardName))
            {
                /*rtm结点 */
                level2cur = level1cur->xmlChildrenNode;
                while ( level2cur && xmlIsBlankNode ( level2cur ) )
                {
                    level2cur = level2cur->next;
                }

                while ( level2cur != NULL)
                {
                    if ( !XOS_StrCmp(level2cur->name, "rtm" ) )
                    {
                        pTempStr = xmlGetProp(level2cur, (xmlChar*)"name");
                        if ( pTempStr && !XOS_StrCmp(pTempStr, tNetInfo.RtmName))
                        {
                            /* device结点 */
                            level3cur = level2cur->xmlChildrenNode;
                            while ( level3cur && xmlIsBlankNode ( level3cur ) )
                            {
                                level3cur = level3cur->next;
                            }
                            
                            while ( level3cur != NULL)
                            {
                                if ( !XOS_StrCmp(level3cur->name, "device" ) )
                                {
                                    /* 读取device tag中 name的值 */
                                    pTempStr = xmlGetProp(level3cur, (xmlChar*)"name");
                                    if ( pTempStr &&  XOS_StrLen(pTempStr) > 0 )
                                    {
                                        /* 获取端口对应的枚举值 */
                                        if (XSUCC != XOS_GetNetPortEnum(pTempStr, &iInd))
                                        {
                                            printf("XOS_ParserNetDevMapXml: Get dev logicname:%s, can't find matched enum!\n",pTempStr);
                                            level3cur = level3cur->next;
                                            continue;
                                        }

                                        XOS_StrNcpy(tNetInfo.tNetDevMap[iInd].LogicName, pTempStr, MAX_DEV_NAME_LEN);
                                        xmlFree( pTempStr );
                                        pTempStr   = XNULL;
                                    }
                                    else
                                    {
                                        return XERROR;
                                    }
                                    
                                    /* 读取device tag中 interface的值 */
                                    pTempStr = xmlGetProp(level3cur, (xmlChar*)"interface");
                                    if ( pTempStr &&  XOS_StrLen(pTempStr) > 0 )
                                    {
                                        XOS_StrNcpy(tNetInfo.tNetDevMap[iInd].DevName, pTempStr, MAX_DEV_NAME_LEN);
                                        xmlFree( pTempStr );
                                        pTempStr   = XNULL;
                                    }
                                    else
                                    {
                                        return XERROR;
                                    }
                                    tNetInfo.tNetDevMap[iInd].valid = XTRUE;
                                }
                                
                                level3cur = level3cur->next;
                                while ( level3cur && xmlIsBlankNode ( level3cur ) )
                                {
                                    level3cur = level3cur->next;
                                }
                            }

                            break; //已经找到对应rtm，完成退出
                        }
                        
                    }
                    
                    level2cur = level2cur->next;
                    while ( level2cur && xmlIsBlankNode ( level2cur ) )
                    {
                        level2cur = level2cur->next;
                    }
                }

                break; //已经找到对应board，完成退出
            }
        }
        level1cur = level1cur->next;
        while ( level1cur && xmlIsBlankNode ( level1cur ) )
        {
            level1cur = level1cur->next;
        }
    }

    if (NULL == level1cur || NULL == level2cur)
    {
        printf("XOS_ParserNetDevMapXml: Don't find correct node! level1:%p,level2:%p\n",level1cur,level2cur);
        return XERROR;
    }    

    /* 赋值给输出结果，并将初始化标志置为1 */
    tNetInfo.init = XTRUE;
    XOS_MemCpy(ptNetInfo,&tNetInfo,sizeof(t_BoardNeMaptInfo));
   
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC; 
#else
    return 0;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_LogicIfConvToDevIf
功能：逻辑设备名转化为实际设备名称
输入：ptLogicName : 逻辑设备名称
      DevNamelen  : 输出的实际设备名称buf的长度
输出：pDevName :    输出的实际设备名称
返回：XSUCC - 成功   XERROR - 失败
说明: 目前只支持linux
************************************************************************/
XS32 XOS_LogicIfConvToDevIf(const XS8 *ptLogicName,XS8 *pDevName,XS32 DevNamelen)
{
#ifdef XOS_LINUX
    t_BoardNeMaptInfo *ptNetInfo;
    XS32 i;

    if (!ptLogicName || !pDevName)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf param err!\n");
        return XERROR;   
    }

    /* 全局变量已经初始化 */
    if (NULL == (ptNetInfo = (t_BoardNeMaptInfo*)XOS_GetNetMap()))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "net map not init!\n");
        return XERROR;
    }

    for (i = 0; i < MAX_NETPORT_NUM; i++)
    {
        if (!ptNetInfo->tNetDevMap[i].valid)
            continue;
            
        if (!XOS_StrNcmp(ptLogicName, ptNetInfo->tNetDevMap[i].LogicName, sizeof(ptNetInfo->tNetDevMap[i].LogicName)))
        {
            XOS_StrNcpy(pDevName,ptNetInfo->tNetDevMap[i].DevName,DevNamelen);
            return XSUCC;
        }
    }
   
    XOS_Trace(MD(FID_ROOT, PL_LOG), "Don't find ptLogicName:%s!\n",ptLogicName);
    return XERROR;
#else
    return XERROR;
#endif /* XOS_LINUX */
}
/************************************************************************
函数名: XOS_GetNewVirDevName
功能：获取一个随机数，并组成一个新虚拟设备名
输入：pllist       : 设备列表
      pDevName     : 物理设备名称
      buflen       : pNewName buf的长度
输出：pNewName     : 得到的新虚拟设备名
返回：成功返回0，失败返回-1
说明: 目前只支持linux
************************************************************************/
static XS32 XOS_GetNewVirDevName(t_LOCALIPINFO *pllist, XS8 *pDevName, XS8 *pNewName, XS32 buflen)
{
#ifdef XOS_LINUX
    XS8  strRandomBuf[RANDOM_STR_LEN+1] = {0};
    XS8  strNewVirDev[XOS_IFNAMESIZE+1] = {0};
    XS32 i;
    XS32 j;
    
    if (XOS_GetRandomNum(strRandomBuf, sizeof(strRandomBuf)))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetRandomNum return fail!\n");
        return XERROR;
    }        

    /*判断此虚拟接口是否已存在*/
    for (j = 0; j < MAX_GET_RANDOM_CNT; j++)
    {
        XOS_Sprintf(strNewVirDev,sizeof(strNewVirDev), "%s:%s", pDevName, strRandomBuf);

        for( i=0; i < pllist->nIPNum; i++ )
        {
            if (0 == XOS_StrCmp(pllist->localIP[i].xinterface, strNewVirDev))
            {
                if (XOS_GetRandomNum(strRandomBuf, sizeof(strRandomBuf)))
                {
                    XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetRandomNum return fail!\n");
                    return XERROR;
                }

                break;
            }
        }

        if (i == pllist->nIPNum)  /* 没有找到相同的虚拟设备名称 */
        {
            break;
        }
    }

    if (j == MAX_GET_RANDOM_CNT)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "get random num fail for %d times!\n",j);
        return XERROR;    
    }

    if (XOS_StrLen(strNewVirDev) >= buflen)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "get dev name len too long!\n",j);
        return XERROR;
    }

    XOS_StrCpy(pNewName,strNewVirDev);

    return XSUCC;
#else
    return 0;
#endif 
}
/************************************************************************
函数名: XOS_AddVirIp_Ex
功能：根据逻辑设备名称和传入的ip和netmask，添加虚拟设备,并返回添加的虚拟设备名
输入：pLogicIfName : 逻辑设备名称    
      pGetIfName   : 获取新增的虚拟设备名 --------- pGetIfName[]至少17个字节 ( XOS_IFNAMESIZE )
      ip           : ip    ----- 本地序
      netmask      : 掩码  ----- 本地序
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
如果ip已在当前设备上存在,且传入的netmask与存在的相同，直接返回0;
如果ip已在当前设备上存在,且传入的netmask与存在的不同，更新设备的netmask为新值;
如果ip已在当前设备上不存在,新增虚拟设备，且设置ip，netmask;
************************************************************************/
XS32 XOS_AddVirIp_Ex(const XS8* pLogicIfName,XS8* pGetIfName, XU32 ip, XU32 netmask)
{
#ifdef XOS_LINUX
    XS8  strIP[XOS_IPSTRLEN] = {0};
    XS8  strNetmask[XOS_IPSTRLEN] = {0};
    XS8  strDev[XOS_IFNAMESIZE+1] = {0};
    XS8  strNewVirDevName[XOS_IFNAMESIZE+1] = {0};
    t_LOCALIPINFO llist;
    XU32 i = 0;

    /*入口参数判断*/
    if( XNULL == pLogicIfName || NULL == pGetIfName || 0 == ip || 0 == netmask)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP 转换*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask 转换*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* 遍历设备上所有ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /*判断IP是否存在*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            XOS_StrCpy(pGetIfName, llist.localIP[i].xinterface);
            if (netmask == llist.localIP[i].LocalNetMask)
            {
                XOS_Trace(MD(FID_ROOT, PL_INFO), "ip[0x%x]:[%s] mask:0x%x on logic dev:%s real dev:%s is already exist!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                return XSUCC;
            }
            else  /* ip相同，netmask不同，更新设备netmask */
            {   
                XOS_Trace(MD(FID_ROOT, PL_LOG), "update exist ip[0x%x]:[%s] NEW mask:0x%x on exist logic dev:%s real dev:%s!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                XOS_StrNcpy(strNewVirDevName,llist.localIP[i].xinterface,sizeof(strNewVirDevName));
                break;
            }
        }
    }

    if (i == llist.nIPNum)  /* 不存在传入的ip，需新增虚拟设备 */
    {
        if (XSUCC != XOS_GetNewVirDevName(&llist, strDev,strNewVirDevName, sizeof(strNewVirDevName)))
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetNewVirDevName fail!\n");
            return XERROR;
        }
        XOS_StrCpy(pGetIfName, strNewVirDevName);
    }


    if (XSUCC != ENC_AddIp(strNewVirDevName,strIP,strNetmask))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ENC_AddIp fail!\n");
        return XERROR;
    }

    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* 检查IP是否被成功添加*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "add ip[0x%x]:[%s] on logic dev:%s real dev:%s succ!\n", ip, strIP,pLogicIfName,llist.localIP[i].xinterface);
            return XSUCC;
        }
    }

    XOS_Trace(MD(FID_ROOT, PL_LOG), "add fail! add ip[0x%x]:[%s] on logic dev:%s real dev:%s fail!\n", ip, strIP,pLogicIfName,strDev);
    return XERROR;
#else
    return 0;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_AddVirIp
功能：根据逻辑设备名称和传入的ip和netmask，添加虚拟设备
输入：pLogicIfName : 逻辑设备名称
      ip           : ip    ----- 本地序
      netmask      : 掩码  ----- 本地序
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
如果ip已在当前设备上存在,且传入的netmask与存在的相同，直接返回0;
如果ip已在当前设备上存在,且传入的netmask与存在的不同，更新设备的netmask为新值;
如果ip已在当前设备上不存在,新增虚拟设备，且设置ip，netmask;
************************************************************************/
XS32 XOS_AddVirIp(const XS8* pLogicIfName, XU32 ip, XU32 netmask)
{
#ifdef XOS_LINUX
    XS8  strIP[XOS_IPSTRLEN] = {0};
    XS8  strNetmask[XOS_IPSTRLEN] = {0};
    XS8  strDev[XOS_IFNAMESIZE+1] = {0};
    XS8  strNewVirDevName[XOS_IFNAMESIZE+1] = {0};
    t_LOCALIPINFO llist;
    XU32 i = 0;

    /*入口参数判断*/
    if( XNULL == pLogicIfName || 0 == ip || 0 == netmask)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP 转换*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask 转换*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* 遍历设备上所有ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /*判断IP是否存在*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            if (netmask == llist.localIP[i].LocalNetMask)
            {
                XOS_Trace(MD(FID_ROOT, PL_INFO), "ip[0x%x]:[%s] mask:0x%x on logic dev:%s real dev:%s is already exist!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                return XSUCC;
            }
            else  /* ip相同，netmask不同，更新设备netmask */
            {   
                XOS_Trace(MD(FID_ROOT, PL_LOG), "update exist ip[0x%x]:[%s] NEW mask:0x%x on exist logic dev:%s real dev:%s!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                XOS_StrNcpy(strNewVirDevName,llist.localIP[i].xinterface,sizeof(strNewVirDevName));
                break;
            }
        }
    }

    if (i == llist.nIPNum)  /* 不存在传入的ip，需新增虚拟设备 */
    {
        if (XSUCC != XOS_GetNewVirDevName(&llist, strDev,strNewVirDevName, sizeof(strNewVirDevName)))
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetNewVirDevName fail!\n");
            return XERROR;
        }
    }


    if (XSUCC != ENC_AddIp(strNewVirDevName,strIP,strNetmask))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ENC_AddIp fail!\n");
        return XERROR;
    }

    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* 检查IP是否被成功添加*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "add ip[0x%x]:[%s] on logic dev:%s real dev:%s succ!\n", ip, strIP,pLogicIfName,llist.localIP[i].xinterface);
            return XSUCC;
        }
    }

    XOS_Trace(MD(FID_ROOT, PL_LOG), "add fail! add ip[0x%x]:[%s] on logic dev:%s real dev:%s fail!\n", ip, strIP,pLogicIfName,strDev);
    return XERROR;
#else
    return 0;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_ModifyVirIp
功能：根据逻辑设备名称和传入的ip和netmask，修改虚拟设备ip为新ip
输入：pLogicIfName : 逻辑设备名称
      oldip        : 要修改的源ip  --- 本地序
      ip           : 设置的新的ip  --- 本地序
      netmask      : 掩码
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
如果oldip和ip都不存在,则相当于add接口
如果oldip存在，ip也存在，且ip与oldip不是同一个虚拟设备，则删除旧设备，更新ip的设备为ip，netmask
如果oldip或ip其中一个存在，则找出此设备，并设置此设备为ip，netmask
************************************************************************/
XS32 XOS_ModifyVirIp(const XS8* pLogicIfName, XU32 oldip, XU32 ip, XU32 netmask)
{
#ifdef XOS_LINUX
    XS8  strNewIP[XOS_IPSTRLEN] = {0};
    XS8  strOldIP[XOS_IPSTRLEN] = {0};
    XS8  strNetmask[XOS_IPSTRLEN] = {0};
    XS8  strDev[XOS_IFNAMESIZE+1] = {0};
    XS8  strNewVirDevName[XOS_IFNAMESIZE+1] = {0};
    XS8  *old_intf = NULL;
    XS8  *new_intf = NULL;
    XS8  *set_intf = NULL;
    t_LOCALIPINFO llist;
    XBOOL flag_old = XFALSE;
    XBOOL flag_new = XFALSE;
    XS32 maskbit = 0;
    XU32 i = 0;

    /*入口参数判断*/
    if( XNULL == pLogicIfName || 0 == ip || 0 == netmask )
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_IptoStr(oldip,strOldIP))  /*IP 转换*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strNewIP))  /*IP 转换*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask 转换*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* 遍历设备上所有ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            flag_new = XTRUE;
            new_intf = llist.localIP[i].xinterface;
        }
        
        if( oldip == llist.localIP[i].LocalIPaddr )
        {
            flag_old = XTRUE;                
            old_intf = llist.localIP[i].xinterface;
            maskbit = XOS_XU32ToMask(llist.localIP[i].LocalNetMask);
        }
    }

    /*-----------------old ip存在 且 new ip存在，删除old ip -------------------------------*/
    if (flag_new && flag_old)
    {
        if (XOS_StrNcmp(old_intf,new_intf,XOS_IFNAMESIZE))  /* ip与oldip都存在，且在不同的设备上，则删除oldip的设备 */
        {
            if (XOS_DevNameIsValid(pLogicIfName,old_intf))
            {
                if (ENC_DelIp(old_intf, strDev, strOldIP, maskbit))
                {
                    XOS_Trace(MD(FID_ROOT, PL_LOG), "del fail! del ip:[0x%x]:[%s] on logic dev:%s real dev:%s fail!\n", oldip,strOldIP, old_intf,strDev);
                }
            }
        }
        set_intf = new_intf;
    }
    /*-----------------old ip不存在 且 new ip不存在，则增加新虚拟设备 ---------------------*/
    else if ( !flag_new && !flag_old)
    {
        if (XSUCC != XOS_GetNewVirDevName(&llist, strDev,strNewVirDevName, sizeof(strNewVirDevName)))
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetNewVirDevName fail!\n");
            return XERROR;
        }
        set_intf = strNewVirDevName;
    }
    /*-----------------old ip 或 new ip 只有一个存在，则在已存在的虚拟设备上修改 ----------*/
    else
    {
        if (flag_new)
        {
            set_intf = new_intf;
        }
        else
        {
            set_intf = old_intf;
        }
    }

    /* 无论new intf是否存在，都将新ip重新设一遍 */
    if (ENC_AddIp(set_intf, strNewIP,strNetmask))
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ifconfig fail! modify ip[0x%x]:[%s] to ip[0x%x]:[%s] netmask[0x%x]:[%s] on logic dev:%s real dev:%s fail!\n", ip,strNewIP,oldip,strOldIP,netmask,strNetmask,pLogicIfName, set_intf);
        return XERROR;
    }
    
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* 检查IP是否被成功添加 */
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "modify ip[0x%x]:[%s] to ip[0x%x]:[%s] on logic dev:%s real dev:%s Succ!\n", ip,strNewIP,oldip,strOldIP,pLogicIfName,llist.localIP[i].xinterface);
            return XSUCC;
        }
    }

    XOS_Trace(MD(FID_ROOT, PL_LOG), "Fail! modify ip[0x%x]:[%s] to ip[0x%x]:[%s] on logic dev:%s real dev:%s fail!\n", ip,strNewIP,oldip,strOldIP,pLogicIfName,strDev);
    return XERROR;
#else
    return 0;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_DeleteVirIp
功能：根据逻辑设备名称和传入ip，删除虚拟设备
输入：pLogicIfName : 逻辑设备名称
      ip           : ip  ----- 本地序
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
此接口不允许删除BASE1,BASE2,FABRIC1,FABRIC2的物理ip
************************************************************************/
XS32 XOS_DeleteVirIp(const XS8* pLogicIfName, XU32 ip)
{
#ifdef XOS_LINUX
    XS8  strIP[XOS_IPSTRLEN] = {0};
    XS8  strDev[XOS_IFNAMESIZE] = {0};
    XS8  xinterface[XOS_IFNAMESIZE] = {0};
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XBOOL flag = XFALSE;
    XS32 maskbit = 0;

    /*入口参数判断*/
    if( XNULL == pLogicIfName || 0 == ip)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }


    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP 转换*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* 遍历设备上所有ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* 判断IP是否存在,可能存在多个 */
    for( i=0; i < llist.nIPNum ; i++ )
    {    
        if( ip == llist.localIP[i].LocalIPaddr 
            && XOS_DevNameIsValid(pLogicIfName,llist.localIP[i].xinterface))
        {
           	flag = XTRUE;
            maskbit = XOS_XU32ToMask(llist.localIP[i].LocalNetMask);
            
            if (XSUCC != ENC_DelIp(llist.localIP[i].xinterface, strDev, strIP, maskbit))
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR), "ENC_DelIp fail!\n");
                return XERROR;
            }
        }
    } 
    
    if( !flag ) /*不存在要删除的接口或IP*/
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "Don't find ip[0x%x]:[%s] on logic dev:%s, real dev:%s!\n", ip,strIP, pLogicIfName,strDev);
        return XERROR;
    }
    
    /* 检查IP是否被成功删除*/
    if( XSUCC != XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* 检查IP是否被成功删除*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "del fail! ip[0x%x]:[%s] on logic dev:%s real dev:%s still exist!\n", ip,strIP, pLogicIfName,xinterface);
            return XERROR;
        }
    }

    XOS_Trace(MD(FID_ROOT, PL_LOG), "del ip[0x%x]:[%s] on logic dev:%s real dev:%s succ!\n", ip, strIP,pLogicIfName,xinterface);
    
    return XSUCC;
#else
    return XSUCC;
#endif /* XOS_LINUX */
}
/************************************************************************
函数名: XOS_CheckVirIp
功能：检查ip在传入的逻辑设备上是否存在
输入：pLogicIfName : 逻辑设备名称
      ip           : ip ----- 本地序
输出：
返回：1: ip存在  (IP_EXIST)
      0: ip不存在 (IP_NOT_EXIST)
     -1: 返回错误
说明: 目前只支持linux
判断ip是否存在于当前逻辑设备
************************************************************************/
XS32 XOS_CheckVirIp(const XS8* pLogicIfName, XU32 ip)
{
#ifdef XOS_LINUX
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XS8  strDev[XOS_IFNAMESIZE] = {0};

    /*入口参数判断*/
    if( XNULL == pLogicIfName || 0 == ip)
    {
        return XERROR;
    }
    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        
        return XERROR;
    }

    /* 遍历设备上所有ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        return XERROR;
    }
    
    /*判断IP是否存在*/
    for( i=0; i < llist.nIPNum; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            return IP_EXIST;
        }
    }
    
    return IP_NOT_EXIST;
#else
    return 0;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_GetAllIPList
功能：list所有网卡的设备列表
输入：
输出：pLocalIPList : 保存设备列表
返回：成功返回0，失败返回-1
说明: 目前只支持linux
************************************************************************/
XS32 XOS_GetAllIPList ( t_LOCALIPINFO * pLocalIPList )
{
#ifdef XOS_LINUX
    struct ifreq *pifr; 
    struct ifreq buf[XOS_MAXIPNUM]; 
    struct ifconf ifc; 
    XOS_SOCKET_ID  Sock;
    XS32 ret = 0;
    XU32 netCards =0, i =0, j =0;
    XU32 ipaddr,netmask;

    if(XNULL == pLocalIPList)
    {
        return XERROR;
    }

    memset(pLocalIPList, 0, sizeof(t_LOCALIPINFO));
    
    Sock = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
    if(XOS_INET_INV_SOCKFD == Sock)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "socket fail! %s\n",strerror(errno));
        return XERROR;
    }
    
    memset(buf,0,sizeof(buf));
    ifc.ifc_len  =  sizeof(buf);      
    ifc.ifc_buf  =  (caddr_t)buf;   

    ret = ioctl(Sock,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get all dev conf fail! %s\n",strerror(errno));
        close(Sock);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*获取网卡的个数*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        pifr = &buf[i];

        /* 判断设备是否正常 */
        if (ioctl(Sock, SIOCGIFFLAGS, pifr) < 0)
        {
            close(Sock);
            XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get dev:%s flag fail! %s\n",pifr->ifr_name,strerror(errno));
            return XERROR;
        }  

        /*获取该网卡上配置的ip, 若没有设置ip，则会失败*/
        if (ioctl(Sock,  SIOCGIFADDR,  pifr) != 0)
        {
            continue;
        }

        ipaddr = (((struct sockaddr_in*)(&pifr->ifr_addr))->sin_addr.s_addr);
        
        ret = ioctl(Sock,  SIOCGIFNETMASK,  pifr); 
        if(ret != 0  )
        {
            continue;
        }

        netmask = (((struct sockaddr_in*)(&pifr->ifr_netmask))->sin_addr.s_addr);

        XOS_StrNcpy(pLocalIPList->localIP[j].xinterface,pifr->ifr_name,XOS_IFNAMESIZE);        
        pLocalIPList->localIP[j].LocalIPaddr  = XOS_NtoHl(ipaddr);
        pLocalIPList->localIP[j].LocalNetMask = XOS_NtoHl(netmask);
        
        j++;                                                                           
    }
    
    pLocalIPList->nIPNum = j;
    close(Sock);

    return XSUCC;
#else
    return XERROR;
#endif /* XOS_LINUX */
}

/************************************************************************
函数名: XOS_GetDevIPList
功能：list指定网卡的设备列表
输入：pdev : 指定设备
输出：pLocalIPList : 保存设备列表
返回：成功返回0，失败返回-1
说明: 目前只支持linux
************************************************************************/
XS32 XOS_GetDevIPList ( const XS8 *pdev, t_LOCALIPINFO * pLocalIPList )
{
#ifdef XOS_LINUX
    struct ifreq ifr; 
    struct ifreq buf[XOS_MAXIPNUM]; 
    struct ifconf ifc; 
    XOS_SOCKET_ID  Sock;
    XS32 ret = 0;
    XU32 netCards =0, i =0, j =0;
    XU32 ipaddr,netmask;

    if(XNULL == pLocalIPList || NULL == pdev)
    {
        return XERROR;
    }

    memset(pLocalIPList, 0, sizeof(t_LOCALIPINFO));
    
    Sock = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
    if(XOS_INET_INV_SOCKFD == Sock)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "socket fail! %s\n",strerror(errno));
        return XERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    XOS_StrNcpy(ifr.ifr_name, pdev, IFNAMSIZ);
    
    /* 判断设备是否正常 */
    if (ioctl(Sock, SIOCGIFFLAGS, &ifr) < 0)
    {
        close(Sock);
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get dev flag fail! %s\n",strerror(errno));
        return XERROR;
    }
    
    /* 如果设备没有up，将其up */        
    if( !(ifr.ifr_flags & IFF_UP) )
    {
        ifr.ifr_flags |= IFF_UP;
        if (ioctl(Sock,  SIOCSIFFLAGS, &ifr) < 0)
        {
            close(Sock);
            XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl set dev flag fail! %s\n",strerror(errno));
            return XERROR;
        }
    }

    memset(buf,0,sizeof(buf));
    ifc.ifc_len  =  sizeof(buf);      
    ifc.ifc_buf  =  (caddr_t)buf;   

    ret = ioctl(Sock,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get all dev conf fail! %s\n",strerror(errno));
        close(Sock);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*获取网卡的个数*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /*找出与传入设备名对应的所有接口(别)名*/
        if (XOS_StrNcmp(buf[i].ifr_name, pdev, XOS_StrLen(pdev)))
        {
            continue;
        }
        
        /*获取该网卡上配置的ip, 若没有设置ip，则会失败*/
        if (ioctl(Sock,  SIOCGIFADDR,  (XS8 *)&buf[i]) != 0)
        {
            continue;
        }

        ipaddr = (((struct sockaddr_in*)(&buf[i].ifr_addr))->sin_addr.s_addr);
        
        ret = ioctl(Sock,  SIOCGIFNETMASK,  (XS8 *)&buf[i] ); 
        if(ret != 0  )
        {
            continue;
        }

        netmask = (((struct sockaddr_in*)(&buf[i].ifr_netmask))->sin_addr.s_addr);

        XOS_StrNcpy(pLocalIPList->localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE-1);        
        pLocalIPList->localIP[j].LocalIPaddr  = XOS_NtoHl(ipaddr);
        pLocalIPList->localIP[j].LocalNetMask = XOS_NtoHl(netmask);
        
        j++;                                                                           
    }
    
    pLocalIPList->nIPNum = j;
    close(Sock);

    return XSUCC;
#else
    return XERROR;
#endif /* XOS_LINUX */
}
/************************************************************************
函数名: XOS_GetRandomNum
功能：获取随机数
输入：
输出：
返回：成功返回0，失败返回-1
说明: 目前只支持linux
************************************************************************/
static XS32 XOS_GetRandomNum(XS8 *randombuf, XU32 buflen)
{
#ifdef XOS_LINUX
    XS8 buftemp[XOS_MAXCMDLEN] = {0};
    XS8 cmdl[XOS_MAXCMDLEN] = {0};

    if (!randombuf)
    {
        return XERROR;
    }

    XOS_Sprintf(cmdl,sizeof(cmdl),"cat /proc/sys/kernel/random/uuid| cksum | cut -f1 -d\" \"");
    if( XERROR == XOS_ExeCmdByPopen_Ex((XCHAR *)cmdl, buftemp, sizeof(buftemp),10))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "exe cmd:%s fail!\n",cmdl);
        return XERROR;
    }

    buftemp[XOS_StrLen(buftemp)-1] = '\0';  //清楚命令结果中最后一个字节的 \n 字符

    XOS_StrNcpy(randombuf, buftemp, XOS_MIN(buflen, XOS_StrLen(buftemp)));

    return XSUCC;
#else
        return 0;
#endif /* XOS_LINUX */
}
static XS32 ENC_AddIp(XS8 *xinterface, XS8* ip,XS8* netmask)
{
    XS8 cmd[XOS_MAXCMDLEN] = {0};

    if (!xinterface || !ip || !netmask)
    {
        return XERROR;
    }
    
    XOS_Sprintf(cmd, sizeof(cmd), "/sbin/ifconfig %s %s netmask %s", xinterface, ip, netmask);

    /* 对命令执行后的返回值进行判断 */
    if(0 != XOS_ExeCmdRetVal((XCHAR *)cmd) )
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "exe cmd:%s ret fail!\n",cmd);
        return XERROR;
    }

    return XSUCC;
}
static XS32 ENC_DelIp(XS8 *xinterface, XS8 *dev, XS8* ip,XU32 netmaskbit)
{
    XS8 cmd[XOS_MAXCMDLEN] = {0};
    
    if (!xinterface || !ip || !dev || netmaskbit == 0)
    {
        return XERROR;
    }

    XOS_Sprintf(cmd, sizeof(cmd), 
        "/sbin/ip addr del %s/%d brd + dev %s label %s",ip, netmaskbit, dev, xinterface);
        
    /* 对命令执行后的返回值进行判断 */
    if(0 != XOS_ExeCmdRetVal((XCHAR *)cmd) )
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "exe cmd:%s ret fail!\n",cmd);
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名: XOS_XU32ToMask
功能：将xu32的子网掩码变为是1的个数
输入：netmask : 掩码  ------------ 本地序 : 0xFFFF0000 = "255.255.0.0"
输出：
返回：返回子网掩码位为1的个数
说明: 目前只支持linux
************************************************************************/
static XS32 XOS_XU32ToMask(XU32 netmask)
{
    XU32 tempmask = netmask;
    XS32 bitnum = 8*sizeof(tempmask);

    if (0 == tempmask)
    {
        return 0;
    }
    
    while (0 == (tempmask & 0x1))
    {
        bitnum--;
        tempmask = tempmask>>1;
    }
    return bitnum;
}
/************************************************************************
函数名: XOS_DevNameIsValid
功能：判断传入的设备名是否合法
输入：
输出：
返回：合法返回true，非法返回false
说明: 目前只支持linux
************************************************************************/
static XBOOL XOS_DevNameIsValid(const XS8 *pLogicName,const XS8 *pinterface)
{
    if ((XNULL == XOS_StrNChr((XCHAR*)pinterface, ':', XOS_IFNAMESIZE))
      && ( (0 == XOS_StrCmp((char*)pLogicName, BASE1))
        || (0 == XOS_StrCmp((char*)pLogicName, BASE2))
        || (0 == XOS_StrCmp((char*)pLogicName, FABRIC1))
        || (0 == XOS_StrCmp((char*)pLogicName, FABRIC2))))
    {
        return XFALSE;
    }

    return XTRUE;
}

/************************************************************************
函数名: XOS_GetNetPortEnum
功能：根据设备的字符串逻辑名称转换为enum值
输入：strLogicName : 设备的字符串逻辑名称
输出：piNetPortEnum : 要返回的设备的枚举值
返回：正确返回0， 失败返回-1
说明: 目前只支持linux
************************************************************************/
static XS32 XOS_GetNetPortEnum(XS8 *strLogicName, XS32 *piNetPortEnum)
{
    if (0 == XOS_StrCmp(strLogicName,BASE1))
    {
        *piNetPortEnum = E_BASE1;
    }
    else if (0 == XOS_StrCmp(strLogicName,BASE2))
    {
        *piNetPortEnum = E_BASE2;
    }
    else if (0 == XOS_StrCmp(strLogicName,FABRIC1))
    {
        *piNetPortEnum = E_FABRIC1;
    }
    else if (0 == XOS_StrCmp(strLogicName,FABRIC2))
    {
        *piNetPortEnum = E_FABRIC2;
    }
    else if (0 == XOS_StrCmp(strLogicName,FRONT1))
    {
        *piNetPortEnum = E_FRONT1;
    }
    else if (0 == XOS_StrCmp(strLogicName,FRONT2))
    {
        *piNetPortEnum = E_FRONT2;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK1))
    {
        *piNetPortEnum = E_BACK1;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK2))
    {
        *piNetPortEnum = E_BACK2;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK3))
    {
        *piNetPortEnum = E_BACK3;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK4))
    {
        *piNetPortEnum = E_BACK4;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK5))
    {
        *piNetPortEnum = E_BACK5;
    }
    else if (0 == XOS_StrCmp(strLogicName,BACK6))
    {
        *piNetPortEnum = E_BACK6;
    }
    else
    {
        *piNetPortEnum = E_PORT_UNKNOWN;
        XOS_Trace(MD(FID_ROOT, PL_LOG), "Unknown dev name:%s\n",strLogicName);
        return -1;
    }

    return 0;
}

/************************************************************************
函数名: XOS_GetDevNameFromEnum
功能：根据设备的enum值转换为字符串物理设备名称
输入：devenum : 设备的枚举值
输出：
返回：正确返回设备字符串物理设备名称， 失败返回null
说明: 目前只支持linux
************************************************************************/
XS8* XOS_GetDevNameFromEnum(XS32 devenum)
{
    if (devenum < 0 || devenum >= MAX_NETPORT_NUM)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "err dev enum:%d!\n",devenum);
        return NULL;
    }

    if (g_tNetMapInfo.tNetDevMap[devenum].valid)
    {
        return g_tNetMapInfo.tNetDevMap[devenum].DevName;
    }
    else
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "invalid dev enum:%d!\n",devenum);
        return NULL;
    }
}

/************************************************************************
函数名: XOS_GetLogicNameFromEnum
功能：根据设备的enum值转换为字符串逻辑名称
输入：devenum : 设备的枚举值
输出：
返回：正确返回设备字符串逻辑名称， 失败返回null
说明: 目前只支持linux
************************************************************************/
XS8* XOS_GetLogicNameFromEnum(XS32 devenum)
{
    if (devenum < 0 || devenum >= MAX_NETPORT_NUM)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "err dev enum:%d!\n",devenum);
        return NULL;
    }

    if (g_tNetMapInfo.tNetDevMap[devenum].valid)
    {
        return g_tNetMapInfo.tNetDevMap[devenum].LogicName;
    }
    else
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "invalid dev enum:%d!\n",devenum);
        return NULL;
    }
}

#ifdef __cplusplus
}
#endif /*__cplusplus */
