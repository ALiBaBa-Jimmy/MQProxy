/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: xosnetinf.c
**
** description:  ���ڹ���Ľӿڷ�װ 
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
                ����ͷ�ļ�
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
                �궨��
-------------------------------------------------------------------------*/
                
#define MAX_GET_RANDOM_CNT 5  /* �����ȡ���������֮ǰ����ͬ��������»�ȡ��������� */
#define RANDOM_STR_LEN     10  /* ���ɵ�����ַ������� */ 

#define  NetDevMapFile   "/usr/local/setnetip/networkmap.xml" 

/*-------------------------------------------------------------------------
                �ڲ����ݽṹ����
-------------------------------------------------------------------------*/

static t_BoardNeMaptInfo g_tNetMapInfo;       /* �����豸����ӳ��� */

/*-------------------------------------------------------------------------
                API ����
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
                ��������
-------------------------------------------------------------------------*/ 

/************************************************************************
������: XOS_NetInfInit
���ܣ���ʼ����������ģ��
���룺
�����
���أ�XSUCC - �ɹ�   XERROR ʧ��
˵��: 
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
������: XOS_GetNetMap
���ܣ���ȡ��������ӳ���ļ�
���룺
�����
���أ��ɹ����� �����õ�ӳ��ṹ��ָ��   ʧ�ܷ��� Null
˵��: 
************************************************************************/
XS8* XOS_GetNetMap(XVOID)
{
    /* ȫ�ֱ���δ��ʼ�� */
    if (!g_tNetMapInfo.init)
    {
        return (XS8*)NULL;
    }

    return (XS8*)&g_tNetMapInfo;
}

/************************************************************************
������: XOS_ParserNetDevMapXml
���ܣ������豸��ӳ���ļ�����������ṹ����
���룺
�����ptNetInfo �����������豸����Ϣ�Ľṹ
���أ�XSUCC - �ɹ�   XERROR �� 1-10 - ʧ��
˵��: 
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
    XS32 iInd = 0;  //�ҳ���������map�����е��±� enumֵ
    XS32 ret= 0;
    
    /*�������*/
    if(!ptNetInfo)
    {
        return XERROR;
    }

    XOS_MemSet(&tNetInfo, 0x00, sizeof(t_BoardNeMaptInfo));

     /* ��ȡ�����ͺ� */
    if (XSUCC != XOS_GetBoardName(boardname,sizeof(boardname)))
    {
        printf("XOS_ParserNetDevMapXml: get board name fail!\n");
        return XERROR;
    }
    XOS_StrNcpy(tNetInfo.BoardName,boardname,XOS_MIN(XOS_StrLen(boardname),sizeof(tNetInfo.BoardName)));

    
    /* ��ȡrtm�ͺţ����û�к󿨣�rtmĬ��ֵΪnone */
    if (XSUCC != XOS_GetRtmName(rtmname,sizeof(rtmname)))
    {
        XOS_StrNcpy(tNetInfo.RtmName,"none",sizeof(tNetInfo.RtmName));
    }
    else
    {
        XOS_StrNcpy(tNetInfo.RtmName,rtmname,XOS_MIN(XOS_StrLen(rtmname),sizeof(tNetInfo.RtmName)));
    }
    
    /* �Ӽ�¼��Ϣ */
    printf("board name:%s\n",tNetInfo.BoardName);
    printf("rtm name:%s\n",tNetInfo.RtmName);

    /* �ж��ļ��Ƿ���� */
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

    /*�Ҹ��ڵ�*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return XERROR;
    }
    
    /*���ڵ�*/
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
    
    /*board���ڵ�*/
    while ( level1cur != XNULL )
    {
        if ( !XOS_StrCmp(level1cur->name, "board" ) )
        {
            pTempStr = xmlGetProp(level1cur, (xmlChar*)"name");
            if ( pTempStr && !XOS_StrCmp(pTempStr, tNetInfo.BoardName))
            {
                /*rtm��� */
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
                            /* device��� */
                            level3cur = level2cur->xmlChildrenNode;
                            while ( level3cur && xmlIsBlankNode ( level3cur ) )
                            {
                                level3cur = level3cur->next;
                            }
                            
                            while ( level3cur != NULL)
                            {
                                if ( !XOS_StrCmp(level3cur->name, "device" ) )
                                {
                                    /* ��ȡdevice tag�� name��ֵ */
                                    pTempStr = xmlGetProp(level3cur, (xmlChar*)"name");
                                    if ( pTempStr &&  XOS_StrLen(pTempStr) > 0 )
                                    {
                                        /* ��ȡ�˿ڶ�Ӧ��ö��ֵ */
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
                                    
                                    /* ��ȡdevice tag�� interface��ֵ */
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

                            break; //�Ѿ��ҵ���Ӧrtm������˳�
                        }
                        
                    }
                    
                    level2cur = level2cur->next;
                    while ( level2cur && xmlIsBlankNode ( level2cur ) )
                    {
                        level2cur = level2cur->next;
                    }
                }

                break; //�Ѿ��ҵ���Ӧboard������˳�
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

    /* ��ֵ����������������ʼ����־��Ϊ1 */
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
������: XOS_LogicIfConvToDevIf
���ܣ��߼��豸��ת��Ϊʵ���豸����
���룺ptLogicName : �߼��豸����
      DevNamelen  : �����ʵ���豸����buf�ĳ���
�����pDevName :    �����ʵ���豸����
���أ�XSUCC - �ɹ�   XERROR - ʧ��
˵��: Ŀǰֻ֧��linux
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

    /* ȫ�ֱ����Ѿ���ʼ�� */
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
������: XOS_GetNewVirDevName
���ܣ���ȡһ��������������һ���������豸��
���룺pllist       : �豸�б�
      pDevName     : �����豸����
      buflen       : pNewName buf�ĳ���
�����pNewName     : �õ����������豸��
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
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

    /*�жϴ�����ӿ��Ƿ��Ѵ���*/
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

        if (i == pllist->nIPNum)  /* û���ҵ���ͬ�������豸���� */
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
������: XOS_AddVirIp_Ex
���ܣ������߼��豸���ƺʹ����ip��netmask����������豸,��������ӵ������豸��
���룺pLogicIfName : �߼��豸����    
      pGetIfName   : ��ȡ�����������豸�� --------- pGetIfName[]����17���ֽ� ( XOS_IFNAMESIZE )
      ip           : ip    ----- ������
      netmask      : ����  ----- ������
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
���ip���ڵ�ǰ�豸�ϴ���,�Ҵ����netmask����ڵ���ͬ��ֱ�ӷ���0;
���ip���ڵ�ǰ�豸�ϴ���,�Ҵ����netmask����ڵĲ�ͬ�������豸��netmaskΪ��ֵ;
���ip���ڵ�ǰ�豸�ϲ�����,���������豸��������ip��netmask;
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

    /*��ڲ����ж�*/
    if( XNULL == pLogicIfName || NULL == pGetIfName || 0 == ip || 0 == netmask)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP ת��*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask ת��*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* �����豸������ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /*�ж�IP�Ƿ����*/
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
            else  /* ip��ͬ��netmask��ͬ�������豸netmask */
            {   
                XOS_Trace(MD(FID_ROOT, PL_LOG), "update exist ip[0x%x]:[%s] NEW mask:0x%x on exist logic dev:%s real dev:%s!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                XOS_StrNcpy(strNewVirDevName,llist.localIP[i].xinterface,sizeof(strNewVirDevName));
                break;
            }
        }
    }

    if (i == llist.nIPNum)  /* �����ڴ����ip�������������豸 */
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

    /* ���IP�Ƿ񱻳ɹ����*/
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
������: XOS_AddVirIp
���ܣ������߼��豸���ƺʹ����ip��netmask����������豸
���룺pLogicIfName : �߼��豸����
      ip           : ip    ----- ������
      netmask      : ����  ----- ������
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
���ip���ڵ�ǰ�豸�ϴ���,�Ҵ����netmask����ڵ���ͬ��ֱ�ӷ���0;
���ip���ڵ�ǰ�豸�ϴ���,�Ҵ����netmask����ڵĲ�ͬ�������豸��netmaskΪ��ֵ;
���ip���ڵ�ǰ�豸�ϲ�����,���������豸��������ip��netmask;
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

    /*��ڲ����ж�*/
    if( XNULL == pLogicIfName || 0 == ip || 0 == netmask)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP ת��*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask ת��*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* �����豸������ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /*�ж�IP�Ƿ����*/
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( ip == llist.localIP[i].LocalIPaddr )
        {
            if (netmask == llist.localIP[i].LocalNetMask)
            {
                XOS_Trace(MD(FID_ROOT, PL_INFO), "ip[0x%x]:[%s] mask:0x%x on logic dev:%s real dev:%s is already exist!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                return XSUCC;
            }
            else  /* ip��ͬ��netmask��ͬ�������豸netmask */
            {   
                XOS_Trace(MD(FID_ROOT, PL_LOG), "update exist ip[0x%x]:[%s] NEW mask:0x%x on exist logic dev:%s real dev:%s!\n",ip,strIP,netmask, pLogicIfName,llist.localIP[i].xinterface);
                XOS_StrNcpy(strNewVirDevName,llist.localIP[i].xinterface,sizeof(strNewVirDevName));
                break;
            }
        }
    }

    if (i == llist.nIPNum)  /* �����ڴ����ip�������������豸 */
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

    /* ���IP�Ƿ񱻳ɹ����*/
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
������: XOS_ModifyVirIp
���ܣ������߼��豸���ƺʹ����ip��netmask���޸������豸ipΪ��ip
���룺pLogicIfName : �߼��豸����
      oldip        : Ҫ�޸ĵ�Դip  --- ������
      ip           : ���õ��µ�ip  --- ������
      netmask      : ����
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
���oldip��ip��������,���൱��add�ӿ�
���oldip���ڣ�ipҲ���ڣ���ip��oldip����ͬһ�������豸����ɾ�����豸������ip���豸Ϊip��netmask
���oldip��ip����һ�����ڣ����ҳ����豸�������ô��豸Ϊip��netmask
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

    /*��ڲ����ж�*/
    if( XNULL == pLogicIfName || 0 == ip || 0 == netmask )
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_IptoStr(oldip,strOldIP))  /*IP ת��*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(ip,strNewIP))  /*IP ת��*/
    {
        return XERROR;
    }

    if(XERROR == XOS_IptoStr(netmask,strNetmask))  /*netmask ת��*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* �����豸������ip */
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

    /*-----------------old ip���� �� new ip���ڣ�ɾ��old ip -------------------------------*/
    if (flag_new && flag_old)
    {
        if (XOS_StrNcmp(old_intf,new_intf,XOS_IFNAMESIZE))  /* ip��oldip�����ڣ����ڲ�ͬ���豸�ϣ���ɾ��oldip���豸 */
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
    /*-----------------old ip������ �� new ip�����ڣ��������������豸 ---------------------*/
    else if ( !flag_new && !flag_old)
    {
        if (XSUCC != XOS_GetNewVirDevName(&llist, strDev,strNewVirDevName, sizeof(strNewVirDevName)))
        {
            XOS_Trace(MD(FID_ROOT, PL_LOG), "XOS_GetNewVirDevName fail!\n");
            return XERROR;
        }
        set_intf = strNewVirDevName;
    }
    /*-----------------old ip �� new ip ֻ��һ�����ڣ������Ѵ��ڵ������豸���޸� ----------*/
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

    /* ����new intf�Ƿ���ڣ�������ip������һ�� */
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

    /* ���IP�Ƿ񱻳ɹ���� */
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
������: XOS_DeleteVirIp
���ܣ������߼��豸���ƺʹ���ip��ɾ�������豸
���룺pLogicIfName : �߼��豸����
      ip           : ip  ----- ������
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
�˽ӿڲ�����ɾ��BASE1,BASE2,FABRIC1,FABRIC2������ip
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

    /*��ڲ����ж�*/
    if( XNULL == pLogicIfName || 0 == ip)
    {
        return XERROR;
    }

    if( '\0' == pLogicIfName[0] )
    {
        return XERROR;
    }


    if(XERROR == XOS_IptoStr(ip,strIP))  /*IP ת��*/
    {
        return XERROR;
    }

    if (XERROR == XOS_LogicIfConvToDevIf(pLogicIfName, strDev, sizeof(strDev)))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_LogicIfConvToDevIf return fail!\n");
        return XERROR;
    }

    /* �����豸������ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* �ж�IP�Ƿ����,���ܴ��ڶ�� */
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
    
    if( !flag ) /*������Ҫɾ���Ľӿڻ�IP*/
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "Don't find ip[0x%x]:[%s] on logic dev:%s, real dev:%s!\n", ip,strIP, pLogicIfName,strDev);
        return XERROR;
    }
    
    /* ���IP�Ƿ񱻳ɹ�ɾ��*/
    if( XSUCC != XOS_GetDevIPList(strDev, &llist))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_GetDevIPList return fail!\n");
        return XERROR;
    }

    /* ���IP�Ƿ񱻳ɹ�ɾ��*/
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
������: XOS_CheckVirIp
���ܣ����ip�ڴ�����߼��豸���Ƿ����
���룺pLogicIfName : �߼��豸����
      ip           : ip ----- ������
�����
���أ�1: ip����  (IP_EXIST)
      0: ip������ (IP_NOT_EXIST)
     -1: ���ش���
˵��: Ŀǰֻ֧��linux
�ж�ip�Ƿ�����ڵ�ǰ�߼��豸
************************************************************************/
XS32 XOS_CheckVirIp(const XS8* pLogicIfName, XU32 ip)
{
#ifdef XOS_LINUX
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XS8  strDev[XOS_IFNAMESIZE] = {0};

    /*��ڲ����ж�*/
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

    /* �����豸������ip */
    if( XERROR == XOS_GetDevIPList(strDev, &llist))
    {
        return XERROR;
    }
    
    /*�ж�IP�Ƿ����*/
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
������: XOS_GetAllIPList
���ܣ�list�����������豸�б�
���룺
�����pLocalIPList : �����豸�б�
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
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
    
    Sock = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
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
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*��ȡ�����ĸ���*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        pifr = &buf[i];

        /* �ж��豸�Ƿ����� */
        if (ioctl(Sock, SIOCGIFFLAGS, pifr) < 0)
        {
            close(Sock);
            XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get dev:%s flag fail! %s\n",pifr->ifr_name,strerror(errno));
            return XERROR;
        }  

        /*��ȡ�����������õ�ip, ��û������ip�����ʧ��*/
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
������: XOS_GetDevIPList
���ܣ�listָ���������豸�б�
���룺pdev : ָ���豸
�����pLocalIPList : �����豸�б�
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
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
    
    Sock = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    if(XOS_INET_INV_SOCKFD == Sock)
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "socket fail! %s\n",strerror(errno));
        return XERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    XOS_StrNcpy(ifr.ifr_name, pdev, IFNAMSIZ);
    
    /* �ж��豸�Ƿ����� */
    if (ioctl(Sock, SIOCGIFFLAGS, &ifr) < 0)
    {
        close(Sock);
        XOS_Trace(MD(FID_ROOT, PL_LOG), "ioctl get dev flag fail! %s\n",strerror(errno));
        return XERROR;
    }
    
    /* ����豸û��up������up */        
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
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*��ȡ�����ĸ���*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /*�ҳ��봫���豸����Ӧ�����нӿ�(��)��*/
        if (XOS_StrNcmp(buf[i].ifr_name, pdev, XOS_StrLen(pdev)))
        {
            continue;
        }
        
        /*��ȡ�����������õ�ip, ��û������ip�����ʧ��*/
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
������: XOS_GetRandomNum
���ܣ���ȡ�����
���룺
�����
���أ��ɹ�����0��ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
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

    buftemp[XOS_StrLen(buftemp)-1] = '\0';  //��������������һ���ֽڵ� \n �ַ�

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

    /* ������ִ�к�ķ���ֵ�����ж� */
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
        
    /* ������ִ�к�ķ���ֵ�����ж� */
    if(0 != XOS_ExeCmdRetVal((XCHAR *)cmd) )
    {
        XOS_Trace(MD(FID_ROOT, PL_LOG), "exe cmd:%s ret fail!\n",cmd);
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������: XOS_XU32ToMask
���ܣ���xu32�����������Ϊ��1�ĸ���
���룺netmask : ����  ------------ ������ : 0xFFFF0000 = "255.255.0.0"
�����
���أ�������������λΪ1�ĸ���
˵��: Ŀǰֻ֧��linux
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
������: XOS_DevNameIsValid
���ܣ��жϴ�����豸���Ƿ�Ϸ�
���룺
�����
���أ��Ϸ�����true���Ƿ�����false
˵��: Ŀǰֻ֧��linux
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
������: XOS_GetNetPortEnum
���ܣ������豸���ַ����߼�����ת��Ϊenumֵ
���룺strLogicName : �豸���ַ����߼�����
�����piNetPortEnum : Ҫ���ص��豸��ö��ֵ
���أ���ȷ����0�� ʧ�ܷ���-1
˵��: Ŀǰֻ֧��linux
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
������: XOS_GetDevNameFromEnum
���ܣ������豸��enumֵת��Ϊ�ַ��������豸����
���룺devenum : �豸��ö��ֵ
�����
���أ���ȷ�����豸�ַ��������豸���ƣ� ʧ�ܷ���null
˵��: Ŀǰֻ֧��linux
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
������: XOS_GetLogicNameFromEnum
���ܣ������豸��enumֵת��Ϊ�ַ����߼�����
���룺devenum : �豸��ö��ֵ
�����
���أ���ȷ�����豸�ַ����߼����ƣ� ʧ�ܷ���null
˵��: Ŀǰֻ֧��linux
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
