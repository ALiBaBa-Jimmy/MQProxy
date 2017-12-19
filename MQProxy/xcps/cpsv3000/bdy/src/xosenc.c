/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: osenc.c
**
** description:  ��IP����Ľӿڷ�װ 
**
** author: wentao
**
** date:   2006.12.4
**
***************************************************************
**                         history                     
** 
***************************************************************
**  author          date              modification            
**  wentao         2006.12.4              create  
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/*------------------------------------------------------------------------
                ����ͷ�ļ�
-------------------------------------------------------------------------*/ 
#include "xosenc.h"
#include "xosencap.h"
#include "xosinet.h"
#include "xosfilesys.h"
#include "xostrace.h"
#include "xoscfg.h"
/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/

#define WIN_IPCFGBUF_LEN 512
#define XOS_NOT_SUPPORT  0xffffffff
 
/*-------------------------------------------------------------------------
                �ڲ����ݽṹ����
-------------------------------------------------------------------------*/
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
XSTATIC XOS_SOCKET_ID  SockipGet;
#endif

/*��������eth0������IP���߼�IP������͹㲥��ַ*/
XU32 g_ulXosPhysicIP = 0;
XU32 g_ulXosLogicIP = 0;
XU32 g_ulXosTspNetMask = 0;
XU32 g_ulXosTspBroadCast = 0;

/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/ 

/*-------------------------------------------------------------------------
                ��������
-------------------------------------------------------------------------*/ 
/************************************************************************
������:    XOS_GetLocalIPList
���ܣ���ȡ�����豸������ӿں�IP��ַ�б�    
���룺 pLocalIPList        -        ��������IP��Ϣ�б�Ľṹ��
�����
pLocalIPList.nIPNum    -    ��ȡ��IP��ַ�ĸ���, 
pLocalIPList. LocalIP[i]. Interface        -    IP�����Ľӿڣ�
pLocalIPList. LocalIP[i]. LocalIPaddr    -    32 λ��IP��ַ

�磺 pLocalIPList.localIP[0].LocalIPaddr ���б��е�һ������IP��ַ.
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����Ŀǰ��windows���޷�ȡ�ö�Ӧ��IP��ַ
************************************************************************/
XS32 XOS_GetLocalIPList ( t_LOCALIPINFO * pLocalIPList )
{
#ifdef XOS_WIN32
    SOCKET_ADDRESS_LIST *pSockList = NULL;
    XS8 buff[WIN_IPCFGBUF_LEN] = {0};
    DWORD  output=0;
    XU32 i = 0;
    XS32 ret = 0;
    XS32 errNo = 0;
    
    if(pLocalIPList == XNULL)
    {
        return XERROR;
    }
    
    if(XERROR == XINET_Init())  /*��ʼ��sock lib��*/
    {
        return XERROR;
    }
    
    SockipGet = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    if(XOS_INET_INV_SOCKFD == SockipGet)
    {
        return XERROR;
    }
    ret = WSAIoctl (SockipGet,  SIO_ADDRESS_LIST_QUERY, XNULL, 
        0, &buff, WIN_IPCFGBUF_LEN, &output, XNULL, XNULL);
    
    if(ret != 0)
    {
        errNo = WSAGetLastError();
        closesocket(SockipGet);
        return XERROR;
    }
    /*��buff��ȡip ֵ*/
    pSockList = (SOCKET_ADDRESS_LIST *)buff;
    
    if( 0 == (pLocalIPList->nIPNum = (XU32)pSockList->iAddressCount))  /*ȡ��IP����*/
    {
        closesocket(SockipGet);
        return XERROR;
    }
    
    for(i=0; i < XOS_MIN((pLocalIPList->nIPNum),XOS_MAXIPNUM); i++)
    {
        /*Ŀǰwindows���޷�ȡ�ýӿ�(����)��*/
        XOS_StrNcpy(pLocalIPList->localIP[i].xinterface,"",XOS_IFNAMESIZE);
        
        pLocalIPList->localIP[i].LocalIPaddr= ((SOCKADDR_IN *) (pSockList->Address[i].lpSockaddr))->sin_addr.s_addr;
        pLocalIPList->localIP[i].LocalIPaddr = XOS_NtoHl( pLocalIPList->localIP[i].LocalIPaddr);
    }

    closesocket(SockipGet);
    
#endif
    
#ifdef XOS_LINUX
    struct ifreq buf[XOS_MAXIPNUM]; 
    struct ifconf ifc; 
    XS32 ret = 0;
    XU32 netCards =0, i =0, j =0;

    memset(buf,0,sizeof(buf));
    ifc.ifc_len  =  sizeof(buf);      
    ifc.ifc_buf  =  (caddr_t)buf;   
    
    if(pLocalIPList == XNULL)
    {
        return XERROR;
    }
    
    SockipGet = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    if(XOS_INET_INV_SOCKFD == SockipGet)
    {
        return XERROR;
    }

    ret = ioctl(SockipGet,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        close(SockipGet);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*��ȡ�����ĸ���*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* ����ȷ�������Ƿ������� */      
        ret = ioctl(SockipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*ֻ��up ���������õ�ip ����Ч */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*��ȡ�����������õ�ip*/
            ret = ioctl(SockipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*���λ��ص�ַ*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*��ȡ��Ӧ�Ľӿ�(��)�����磺eth1:1*/  
            XOS_StrNcpy(pLocalIPList->localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE);        
            
            pLocalIPList->localIP[j].LocalIPaddr= (((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr);
            pLocalIPList->localIP[j].LocalIPaddr= XOS_NtoHl(pLocalIPList->localIP[j].LocalIPaddr);
            
            j++;                                                                           
        }
    }
    if( 0 == (pLocalIPList->nIPNum = j))
    {
        close(SockipGet);
        return XERROR;
    }

    close(SockipGet);
#endif
    
    return XSUCC;
}


/************************************************************************
������:    XOS_LocalIPAdd
���ܣ� �ڱ��ؽӿ��м���һ��IP��ַ    
���룺
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��
'x'��ʾ�ڼ���������"eth0:1"��ʾ��һ�������ĵ�һ��������
ipaddress    -    Ҫ��ӵ��˽ӿڵ�IP��ַ�����ʮ���� ���ַ�������ʽ���磺"192.168.0.1"��
subnetmask    -    ָ��IP���������룬�磺"255.255.255.0"
broadcast    -    ָ��IP�Ĺ㲥��ַ���磺"192.168.0.255"��windows����Ĭ�ϵ�,������XNULL��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����ͬһ�ӿڣ��������¶�������ͬ��IP������XERROR��
��������Ľӿ�����������ʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ��XOS_GetLocalIPList() 
����ȡ��ǰ�Ľӿ�������ӦIP�����ο�����Ҫע�����뼰�㲥��ַ�ĸ�ʽ����Ч�ԡ�
************************************************************************/
XS32 XOS_LocalIPAdd ( XS8 *xinterface , XS8 *ipaddress , XS8 *subnetmask , XS8 *broadcast )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 lIP = 0;
    XU32 temp = 0;
    
    /*��ڲ����ж�*/
    if( XNULL == xinterface || XNULL == ipaddress || XNULL == subnetmask )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 
        0 == XOS_StrNcmp("", ipaddress, 1) || 0 == XOS_StrNcmp("", subnetmask, 1))
    {
        return XERROR;
    }
    
    
    if(XERROR == XOS_StrNtoIp( ipaddress, &lIP, XOS_IPSTRLEN))  /*IP ת��*/
    {
        return XERROR;
    }
    
    /*�ж�IP�Ƿ����,�����ھͲ������*/
    if( XERROR == XOS_GetLocalIPList(&llist))
    {
        return XERROR;
    }
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( lIP == llist.localIP[i].LocalIPaddr )
        {
            return XERROR;
        }
    }    
    
#ifdef XOS_LINUX    
    /*LINUX�������ڡ���ͬ�Ľӿڱ�������Ҳ������� */
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( 0 == XOS_StrNcmp(xinterface,llist.localIP[i].xinterface,XOS_IFNAMESIZE))
        {
            return XERROR;
        }
    }
    
#endif
    
    /*׼���������ִ�*/
#ifdef XOS_LINUX
    XOS_StrCat(cmdl,"/sbin/ifconfig ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,ipaddress);
    if(XNULL != broadcast && 0 != XOS_StrNcmp("", broadcast, 1))    /*broadΪ�յĻ�����ΪĬ��*/
    {
        XOS_StrCat(cmdl," broadcast ");
        XOS_StrCat(cmdl,broadcast);
    }
    else
    {
        XOS_UNUSED(broadcast);
    }
    XOS_StrCat(cmdl," netmask ");
    XOS_StrCat(cmdl,subnetmask);
#endif
#ifdef XOS_WIN32
    XOS_StrCat(cmdl,"netsh interface ip add address ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,ipaddress);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,subnetmask);
    XOS_UNUSED(broadcast);
#endif
    
    /*����ֻ��system��ִ����������˴���δ������ִ�к�ķ���ֵ�����ж�*/
    if( -1 == system((XCHAR *)cmdl) )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* ���IP�Ƿ񱻳ɹ�����*/
    {
        return XERROR;
    }
#ifdef XOS_WIN32    
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( lIP == llist.localIP[i].LocalIPaddr )
        {
            temp = 1;
        }
    }
#endif    
#ifdef XOS_LINUX    
    
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX�»�Ҫ�ж��Ƿ��Ѵ��ڶ�Ӧ�Ľӿ������磺eth1:1 */
    {
        if( lIP == llist.localIP[i].LocalIPaddr && 
            0 == XOS_StrNcmp(xinterface,llist.localIP[i].xinterface,XOS_IFNAMESIZE))
        {
            temp = 1;
        }
    }    
    
#endif
    
    if( temp == 0 )
    {
        return XERROR;
    }
    
#endif
    
    return XSUCC;
}


/************************************************************************
������:    XOS_LocalIPDelete
���ܣ�ɾ��ָ���ӿ��µ�һ��IP��ַ    
���룺
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��
'x'��ʾ�ڼ���������eth0:1��ʾ��һ�������ĵ�һ��������
ipaddress    -    Ҫɾ����IP��ַ�����ʮ���� ���ַ�������ʽ���磺"192.168.0.1"��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ɾ��ͬһIP��ַ������XERROR��
��������Ľӿ���ʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ����XOS_GetLocalIPList() 
����ȡ��ǰ�Ľӿ�������ӦIP�����ο���
ע��ֻ��ɾ����̬IP��ÿ���ӿڵ����һ����̬IP����ɾ����LINUX�������ӿڣ��Ǳ���,��:"eth0"����IP����ɾ����
************************************************************************/
XS32 XOS_LocalIPDelete (XS8 *xinterface , XS8 *ipaddress )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 temp = 0;
    XU32 lIP = 0;
    
    /*��ڲ����ж�*/
    if( XNULL == xinterface || XNULL == ipaddress )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 0 == XOS_StrNcmp("", ipaddress, 1))
    {
        return XERROR;
    }
    
    if(XERROR == XOS_StrNtoIp( ipaddress, &lIP , XOS_IPSTRLEN ))  /*IP ת��*/
    {
        return XERROR;
    }
    
    /*�ж�IP�Ƿ���ڣ��������ھͲ���ɾ��*/
    if(XERROR == XOS_GetLocalIPList(&llist))
    {
        return XERROR;
    }
    if( 1 >= llist.nIPNum ) /*�������һ��IP������ɾ�� */
    {
        return XERROR;
    }
#ifdef XOS_WIN32    
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( lIP == llist.localIP[i].LocalIPaddr )
        {
            temp = 1 ; /*Ҫɾ����IP����,temp��1*/
        }
    }
#endif    
#ifdef XOS_LINUX
    
    if(XNULL == XOS_StrNChr(xinterface, ':', XOS_IFNAMESIZE)) /*LINUX�����ӿ�������ɾ��*/
    {
        return XERROR;
    }
    
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX�»�Ҫ�жϽ�ɾ���Ľӿ�(����)�Ƿ���� */
    {
        if( lIP == llist.localIP[i].LocalIPaddr && 
            0 == XOS_StrNcmp(xinterface, llist.localIP[i].xinterface, XOS_IFNAMESIZE))
        {
            temp = 1 ;   /*�ҵ���ͬ�Ľӿڣ�temp��1*/
        }
    }
    
#endif
    if( 0 == temp ) /*������Ҫɾ���Ľӿڻ�IP*/
    {
        return XERROR;
    }
    
    /*׼���������ִ�*/    
#ifdef XOS_LINUX
    XOS_StrCat(cmdl,"/sbin/ifconfig ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," del ");
    XOS_StrCat(cmdl,ipaddress);
#endif
#ifdef XOS_WIN32
    XOS_StrCat(cmdl,"netsh interface ip delete address ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,ipaddress);
#endif
    
    /*����ֻ��system��ִ����������˴���δ������ִ�к�ķ���ֵ�����ж�*/
    if( -1 == system((XCHAR *)cmdl) )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* ���IP�Ƿ񱻳ɹ�ɾ��*/
    {
        return XERROR;
    }
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( lIP == llist.localIP[i].LocalIPaddr )
        {
            return XERROR;
        }
    }
#ifdef XOS_LINUX    
    /*LINUX�����жϽӿ��Ƿ񱻳ɹ�ɾ�� */
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( 0 == XOS_StrNcmp(xinterface,llist.localIP[i].xinterface,XOS_IFNAMESIZE))
        {
            return XERROR;
        }
    }
    
#endif
    
#endif
    return XSUCC;
}


/************************************************************************
������:    XOS_LocalIPModify
���ܣ���ָ���ӿ��½�ipaddress1 ��Ϊipaddresss2 ��ָ�����������뼰�㲥��ַ    
���룺
interface    -    ָ������ӿڵ����ƣ�һ���Ӧһ���������磺windows��Ϊ"��������"��
linux��Ϊһ������"eth0:x"������'eth0'Ϊ������ʶ��
'x'��ʾ�ڼ���������eth0:1��ʾ��һ�������ĵ�һ��������
ipaddress1    -    Ҫ�޸ĵ�IP��ַ�����ʮ���� ���ַ�������ʽ���磺"192.168.0.1"��
ipaddress2    -    �޸ĺ��IP��ַ���磺"192.168.0.2"
subnetmask2    -    ָ��IP���������룬�磺"255.255.255.0"
broadcast2    -    ָ��IP�Ĺ㲥��ַ���磺"192.168.0.255"��windows����Ĭ�ϵ�,������XNULL��
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵������������Ľӿ�����IPʹ����Ҫ��֤������ԣ�LINUX�¿���ͨ��XOS_GetLocalIPList()
����ȡ��ǰ���õĽӿ�������Ӧ��IP��
************************************************************************/
XS32 XOS_LocalIPModify (XS8 *xinterface , XS8 *ipaddress1 , 
                        XS8 *ipaddress2 , XS8 *subnetmask2 , XS8 *broadcast2 )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 temp = 0;
    XU32 tempt = 0; /*�������linux���Ƿ��޸ĳɹ�*/
    XU32 lIP1 = 0;
    XU32 lIP2 = 0;
    
    /*��ڲ����ж�*/
    if( XNULL == xinterface || XNULL == ipaddress1 || XNULL == ipaddress2 || XNULL == subnetmask2 )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 0 == XOS_StrNcmp("", ipaddress1, 1) || 
        0 == XOS_StrNcmp("", ipaddress2, 1) || 0 == XOS_StrNcmp("", subnetmask2, 1))
    {
        return XERROR;
    }
    
    if(XERROR == XOS_StrNtoIp( ipaddress1, &lIP1, XOS_IPSTRLEN ))  /*IP ת��*/
    {
        return XERROR;
    }
    if(XERROR == XOS_StrNtoIp( ipaddress2, &lIP2, XOS_IPSTRLEN ))  /*IP ת��*/
    {
        return XERROR;
    }
    
    /*�ж�IP�Ƿ����*/
    if(XERROR == XOS_GetLocalIPList(&llist))
    {
        return XERROR;
    }
#ifdef XOS_WIN32    
    for( i=0; i < llist.nIPNum ; i++ ) /*Ҫ�޸ĵĽӿڻ�IP���������޸�ʧ��*/
    {
        if(lIP1 == llist.localIP[i].LocalIPaddr )
        {
            temp = 1 ;
        }
    }
#endif
#ifdef XOS_LINUX
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX�»�Ҫ�жϽӿ��µ�IP�Ƿ����*/
    {
        if( lIP1 == llist.localIP[i].LocalIPaddr && 
            0 == XOS_StrNcmp(xinterface, llist.localIP[i].xinterface, XOS_IFNAMESIZE))
        {
            temp = 1 ;
        }
    }
#endif
    
    if( 0 == temp )
    {
        return XERROR;
    }
    
    for( i=0; i < llist.nIPNum ; i++ )  /*���޸ĳɵ�IP���Ѵ������޸�ʧ��****/
    {
        if( lIP2 == llist.localIP[i].LocalIPaddr )
        {
            return XERROR;
        }
    }
    
#ifdef XOS_LINUX
    XOS_StrCat(cmdl,"/sbin/ifconfig ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,ipaddress2);
    if(XNULL != broadcast2 && 0 != XOS_StrNcmp("", broadcast2, 1))
    {
        XOS_StrCat(cmdl," broadcast ");
        XOS_StrCat(cmdl,broadcast2);
    }
    else
    {
        XOS_UNUSED(broadcast2);
    }
    XOS_StrCat(cmdl," netmask ");
    XOS_StrCat(cmdl,subnetmask2);
    
    if( -1 == system((char*)cmdl) )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* ���IP�Ƿ񱻳ɹ��޸�*/
    {
        return XERROR;
    }
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if(lIP1 == llist.localIP[i].LocalIPaddr )
        {
            return XERROR;
        }
        if(lIP2 == llist.localIP[i].LocalIPaddr && 
            0 == XOS_StrNcmp(xinterface, llist.localIP[i].xinterface, XOS_IFNAMESIZE) )
        {
            tempt = 1;
        }
    }
    if( tempt == 0 )
    {
        return XERROR;
    }
    
#endif
    
#ifdef XOS_WIN32
    if( XSUCC != XOS_LocalIPAdd(xinterface, ipaddress2, subnetmask2, broadcast2)) /*�ȼ�*/
    {
        return XERROR;
    }
    
    if( XSUCC != XOS_LocalIPDelete(xinterface,ipaddress1))/*��ɾ*/
    {
        /*ɾʧ���˾ͰѸղżӵ�Ҳɾ��*/
        if( XSUCC != XOS_LocalIPDelete(xinterface,ipaddress2))
        {
//            printf("\nWARNING! the IP 'ipaddress2' exist!\n\n");
        }
        return XERROR;
    }
#endif
    
#endif
    
    return XSUCC;
}


/************************************************************************
������:    XOS_GetPhysicIP
���ܣ���ȡ����IP��ַ
                gpp���»�ȡ��һ��IP��Ĭ��eth0��tsp���װDrv_GetMV0PhysicalIP 
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
************************************************************************/
XS32 XOS_GetPhysicIP(XU32 *ipaddress)
{
#ifdef XOS_WIN32
    t_LOCALIPINFO pLocalIPList;
    XOS_SOCKET_ID  PhysicipGet;
    SOCKET_ADDRESS_LIST *pSockList;
    XS8 buff[WIN_IPCFGBUF_LEN];
    DWORD  output=0;
    XS32 ret;
    XS32 errNo;

    if(ipaddress == XNULL)
    {
        return XERROR;
    }
    
    XOS_MemSet(&pLocalIPList,0,sizeof(t_LOCALIPINFO));
    
    if(XERROR == XINET_Init())  /*��ʼ��sock lib��*/
    {
        return XERROR;
    }
    
    XOS_MemSet(buff, 0, WIN_IPCFGBUF_LEN);
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    if(XOS_INET_INV_SOCKFD == PhysicipGet)
    {
        return XERROR;
    }

    ret = WSAIoctl (PhysicipGet,  SIO_ADDRESS_LIST_QUERY, XNULL, 
        0, &buff, WIN_IPCFGBUF_LEN, &output, XNULL, XNULL);
    
    if(ret != 0)
    {
        errNo = WSAGetLastError();
        closesocket(PhysicipGet);
        return XERROR;
    }
    /*��buff��ȡip ֵ*/
    pSockList = (SOCKET_ADDRESS_LIST *)buff;
    
    if( 0 == (pLocalIPList.nIPNum = (XU32)pSockList->iAddressCount))  /*ȡ��IP����*/
    {
        closesocket(PhysicipGet);
        return XERROR;
    }

    pLocalIPList.localIP[0].LocalIPaddr= ((SOCKADDR_IN *) (pSockList->Address[0].lpSockaddr))->sin_addr.s_addr;
    pLocalIPList.localIP[0].LocalIPaddr = XOS_NtoHl( pLocalIPList.localIP[0].LocalIPaddr);

    *ipaddress = pLocalIPList.localIP[0].LocalIPaddr;

    closesocket(PhysicipGet);
    return XSUCC;
#endif

#ifdef XOS_LINUX
    XOS_SOCKET_ID  PhysicipGet;
    struct ifreq buf[XOS_MAXIPNUM]; 
    struct ifconf ifc; 
    XS32 ret;
    XU32 netCards, i , j;
    t_LOCALIPINFO pLocalIPList;
    XU8 flag = 0;

    if(ipaddress == XNULL)
    {
        return XERROR;
    }
    ifc.ifc_len  =  sizeof(buf);      
    ifc.ifc_buf  =  (caddr_t)buf;   
    
    XOS_MemSet(&pLocalIPList,0,sizeof(t_LOCALIPINFO));
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    
    ret = ioctl(PhysicipGet,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        close(PhysicipGet);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*��ȡ�����ĸ���*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* ����ȷ�������Ƿ������� */      
        ret = ioctl(PhysicipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*ֻ��up ���������õ�ip ����Ч */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*��ȡ�����������õ�ip*/
            ret = ioctl(PhysicipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*���λ��ص�ַ*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*��ȡ��Ӧ�Ľӿ�(��)�����磺eth1:1*/  
            XOS_StrNcpy(pLocalIPList.localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE);        
            
            pLocalIPList.localIP[j].LocalIPaddr= (((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr);
            pLocalIPList.localIP[j].LocalIPaddr= XOS_NtoHl(pLocalIPList.localIP[j].LocalIPaddr);

            /*����eth0��ip,mak,broadcast*/
            if(0 == XOS_StrCmp(pLocalIPList.localIP[j].xinterface,"eth0"))
            {
                flag = 1;
                *ipaddress = pLocalIPList.localIP[j].LocalIPaddr;
                
                g_ulXosPhysicIP = pLocalIPList.localIP[j].LocalIPaddr;
                /*��ȡ���벢����*/
                ret = ioctl(PhysicipGet,SIOCGIFNETMASK,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                g_ulXosTspNetMask = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));
                /*��ȡ�㲥��ַ������*/
                ret = ioctl(PhysicipGet,SIOCGIFBRDADDR,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                g_ulXosTspBroadCast = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));

                close(PhysicipGet); 
                return XSUCC;
            }
            j++;                                                                           
        }
    }
    if( 0 == (pLocalIPList.nIPNum = j) || 0 == flag)
    {
        /*û�ҵ�eth0*/
        close(PhysicipGet);
        return XERROR;
    }

    close(PhysicipGet);
    return XSUCC;
#endif

/*tsp��Ĭ��mv0*/
#ifdef XOS_TSP
    XU8 hostip[20] = {0};
    XU32 subnetmask = 0;
    XU8 broadcast[20] = {0};

    if(ipaddress == XNULL)
    {
        return XERROR;
    }
    
    if(XSUCC != Drv_GetMV0PhysicalIP (hostip, &subnetmask, broadcast ))
    {
        return XERROR;
    }
    XOS_StrtoIp(hostip,ipaddress);
    g_ulXosPhysicIP = *ipaddress;
    g_ulXosTspNetMask = subnetmask;
    XOS_StrtoIp(broadcast,&g_ulXosTspBroadCast);
    return XSUCC;
#endif

    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_ModifyPhysicIP
���ܣ��޸�����IP��ַ
                  gpp�����޸ĵ�һ��IP��Ĭ��eth0��tsp���װDrv_MV0PhysicalIPModify  
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
************************************************************************/
XS32 XOS_ModifyPhysicIP(XU32 *ipaddress)
{
#ifdef XOS_LINUX
    XU8 poldIP[20] = {0};
    XU8 pnewIP[20] = {0};
    XU8 pmask[20] = {0};
    XU8 pbroadcast[20] = {0};
    XU32 localip = 0;

    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    if(0 == g_ulXosTspNetMask || 0 == g_ulXosTspNetMask || 0 == g_ulXosTspBroadCast)
    {
        XOS_GetPhysicIP(&localip);
    }
    
    XOS_IptoStr(*ipaddress,(XCHAR*)pnewIP);
    XOS_IptoStr(g_ulXosPhysicIP,(XCHAR*)poldIP);
    XOS_IptoStr(g_ulXosTspNetMask,(XCHAR*)pmask);
    XOS_IptoStr(g_ulXosTspBroadCast,(XCHAR*)pbroadcast);

    /*�޸�/etc/sysconfig/network-scripts/ifcfg-eth0�ļ�����*/
    if(XSUCC != XOS_ModifyIfcfgOneLine((XS8*)pnewIP))
    {
        printf("XOS_ModifyIfcfgOneLine failed.\r\n");
        return XERROR;
    }
    
    /*����ifconfig �����޸�eth0������ip*/
    if(XSUCC != XOS_LocalIPModify ("eth0" , (XS8*)poldIP, (XS8*)pnewIP, (XS8*)pmask, (XS8*)pbroadcast))
    {
        printf("XOS_LocalIPModify failed.\r\n");
        return XERROR;
    }
    
    return XSUCC;
#endif

#ifdef XOS_TSP
    XU8 pbroadcast[20] = {0};
    XU8 ip[20] = {0};
    XU32 localip = 0;

    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    XOS_GetPhysicIP(&localip);

    XOS_IptoStr(g_ulXosTspBroadCast,pbroadcast);
    XOS_IptoStr(*ipaddress,ip);
    
    if(XSUCC !=Drv_MV0PhysicalIPModify(ip, g_ulXosTspNetMask, pbroadcast ))
    {
        printf("Drv_MV0PhysicalIPModify failed.\r\n");
        return XERROR;
    }
    return XSUCC;
#endif

    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_GetLogicIP
���ܣ���ȡ�߼�IP��ַ��tsp���װgetLogicIp  
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:     linux windows���ݲ�֧��
************************************************************************/
XS32 XOS_GetLogicIP (XU32 * ipaddress)
{
#ifdef XOS_TSP
    XU32 mask = 0;

    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    if(XSUCC != getLogicIp(ipaddress,&mask))
    {
        printf("getLogicIp failed.\r\n");
        return XERROR;
    }
    
    g_ulXosLogicIP = *ipaddress;
    
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_AddLogicIP
���ܣ������߼�IP��tsp���װDrv_MV0LogicIPAdd  
���룺ipaddress ����IP������ָ��,�߼�ip������ipͬһ���Σ�
                  ȡ����ip�Ĺ㲥��ַ������
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:     linux windows���ݲ�֧��
************************************************************************/
XS32 XOS_AddLogicIP (XU32* ipaddress)
{
#ifdef XOS_TSP
    XU8 pbroadcast[20] = {0};
    XU8 ip[20] = {0};
    XU32 phyip = 0;
    
    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    XOS_GetPhysicIP(&phyip);

    XOS_IptoStr(g_ulXosTspBroadCast,pbroadcast);
    XOS_IptoStr(*ipaddress,ip);
    
    if(XSUCC != Drv_MV0LogicIPAdd (ip, g_ulXosTspNetMask, pbroadcast))
    {
        printf("Drv_MV0LogicIPAdd failed.\r\n");
        return XERROR;
    }
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_DeleteLogicIP
���ܣ�ɾ���߼�IP��tsp���װDrv_MV0LogicIPDelete  
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:     linux windows���ݲ�֧��
************************************************************************/
XS32 XOS_DeleteLogicIP(XU32 *ipaddress)
{
#ifdef XOS_TSP
    XU8 ip[20] = {0};

    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    XOS_IptoStr(*ipaddress,ip);
    
    if(XSUCC != Drv_MV0LogicIPDelete(ip))
    {
        printf("Drv_MV0LogicIPDelete failed.\r\n");
        return XERROR;
    }
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_ModifyLogicIP
���ܣ��޸��߼�IP��tsp���װDrv_MV0LogicIPModify 
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:     linux windows���ݲ�֧��
************************************************************************/
XS32 XOS_ModifyLogicIP(XU32 *ipaddress)
{
#ifdef XOS_TSP
    XU8 pbroadcast[20] = {0};
    XU8 plogicip[20] = {0};
    XU8 pnewlogicip[20] = {0};
    XU32 logicip = 0, localip = 0;
    
    if(ipaddress == XNULL)
    {
        return XERROR;
    }

    XOS_GetLogicIP(&logicip);

    XOS_GetPhysicIP(&localip);

    XOS_IptoStr(g_ulXosTspBroadCast, pbroadcast);
    XOS_IptoStr(g_ulXosLogicIP, plogicip);
    XOS_IptoStr(*ipaddress,pnewlogicip);

    if(XSUCC != Drv_MV0LogicIPModify (plogicip, pnewlogicip, g_ulXosTspNetMask, pbroadcast))
    {
        printf("Drv_MV0LogicIPModify failed.oldip=%s,newip=%s,mask=%x,brdcast=%s\r\n",
                                plogicip, pnewlogicip, g_ulXosTspNetMask, pbroadcast);
        return XERROR;
    }
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_GetMv2IP
���ܣ���ȡ�ڲ�IP��tsp���װDrv_GetMV2IP  
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:    linux windows���޴˽ӿ�
************************************************************************/
XS32 XOS_GetMv2IP(XU32 *ipaddress)
{
#ifdef XOS_TSP
    XU8 ip[20] = {0};
    XU32 subnetmask = 0;
    XU8 broadcast[20] = {0};
    
    if(ipaddress == XNULL)
    {
        return XERROR;
    }
    
    if(XSUCC != Drv_GetMV2IP(ip, &subnetmask, broadcast))
    {
        printf("Drv_GetMV2IP failed.\r\n");
        return XERROR;
    }
    
    XOS_StrtoIp(ip,ipaddress);
    
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_GetMv1IP
���ܣ���ȡ�ڲ�IP��tsp���װDrv_GetMV1IP  
���룺ipaddress ����IP������ָ��
�����ip��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:    linux windows���޴˽ӿ�
************************************************************************/
XS32 XOS_GetMv1IP(XU32 *ipaddress)
{
#ifdef XOS_TSP
    XU8 ip[20] = {0};
    XU32 subnetmask = 0;
    XU8 broadcast[20] = {0};
    
    if(ipaddress == XNULL)
    {
        return XERROR;
    }
    
    if(XSUCC != Drv_GetMV1IP(ip, &subnetmask, broadcast))
    {
        printf("Drv_GetMV1IP failed.\r\n");
        return XERROR;
    }
    
    XOS_StrtoIp(ip,ipaddress);
    
    return XSUCC;
#endif
    return XOS_NOT_SUPPORT;
}

/************************************************************************
������:    XOS_ModifyIfcfgOneLine
���ܣ��޸�ifcfg-eth0�ļ���IPADDR=��
���룺newip ��ip�ַ�ָ�룬���ʮ���Ƹ�ʽ
�����
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
˵��:    linux windows���޴˽ӿ�
************************************************************************/
#ifdef XOS_LINUX
XS32 XOS_ModifyIfcfgOneLine(XS8 *newip)
{
    return XOS_ModifyIfcfgByName("eth0",newip);
}

/************************************************************************
������:    XOS_GetPhysicIPByName
���ܣ�������������ȡ����IP��ַ�����룬�㲥��ַ
���룺��������
�����ip��ַ�����룬�㲥��ַ
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
************************************************************************/
XS32 XOS_GetPhysicIPByName(XS8 *name,XU32 *ipaddress,XU32 *mask,XU32 *broadcast)
{
    XOS_SOCKET_ID  PhysicipGet;
    struct ifreq buf[XOS_MAXIPNUM]; 
    struct ifconf ifc; 
    XS32 ret;
    XU32 netCards, i , j;
    t_LOCALIPINFO pLocalIPList;
    XU8 flag = 0;

    if(name == XNULL || ipaddress == XNULL || mask == XNULL || broadcast == XNULL )
    {
        return XERROR;
    }
    ifc.ifc_len  =  sizeof(buf);      
    ifc.ifc_buf  =  (caddr_t)buf;   
    
    XOS_MemSet(&pLocalIPList,0,sizeof(t_LOCALIPINFO));
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*��ʼ��socket*/
    if(XOS_INET_INV_SOCKFD == PhysicipGet)
    {
        return XERROR;
    }
    
    ret = ioctl(PhysicipGet,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        close(PhysicipGet);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*��ȡ�����ĸ���*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* ����ȷ�������Ƿ������� */      
        ret = ioctl(PhysicipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*ֻ��up ���������õ�ip ����Ч */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*��ȡ�����������õ�ip*/
            ret = ioctl(PhysicipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*���λ��ص�ַ*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*��ȡ��Ӧ�Ľӿ�(��)�����磺eth1:1*/  
            XOS_StrNcpy(pLocalIPList.localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE);        
            
            pLocalIPList.localIP[j].LocalIPaddr= (((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr);
            pLocalIPList.localIP[j].LocalIPaddr= XOS_NtoHl(pLocalIPList.localIP[j].LocalIPaddr);

            /*�������������ip,mak,broadcast*/
            if(0 == XOS_StrCmp(pLocalIPList.localIP[j].xinterface,name))
            {
                flag = 1;
                *ipaddress = pLocalIPList.localIP[j].LocalIPaddr;
                
                /*��ȡ����*/
                ret = ioctl(PhysicipGet,SIOCGIFNETMASK,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                *mask = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));
                /*��ȡ�㲥��ַ*/
                ret = ioctl(PhysicipGet,SIOCGIFBRDADDR,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                *broadcast = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));

                close(PhysicipGet); 
                return XSUCC;
            }
            j++;
        }
    }
    if( 0 == (pLocalIPList.nIPNum = j) || 0 == flag)
    {
        /*û�ҵ��������*/
        close(PhysicipGet);
        return XERROR;
    }

    close(PhysicipGet);
    return XSUCC;
}
/************************************************************************
������:    XOS_ModifyPhysicIPByName
���ܣ����������޸�����IP��ַ
���룺name Ҫ�޸ĵ�����������
        ipaddress ����IP������ָ��
�����
���أ�
XSUCC    -    �ɹ�
XERROR   -    ʧ��
************************************************************************/
XS32 XOS_ModifyPhysicIPByName (XS8 *name,XU32 *ipaddress)
{
    XU8 poldIP[20] = {0};
    XU8 pnewIP[20] = {0};
    XU8 pmask[20] = {0};
    XU8 pbroadcast[20] = {0};
    XU32 localip = 0;
    XU32 localMask = 0;
    XU32 localBroadcast = 0;

    if(name == XNULL || ipaddress == XNULL)
    {
        return XERROR;
    }

    if(XERROR == XOS_GetPhysicIPByName(name,&localip,&localMask,&localBroadcast))
    {
        return XERROR;
    }
    
    XOS_IptoStr(*ipaddress,(XCHAR*)pnewIP);
    XOS_IptoStr(localip,(XCHAR*)poldIP);
    XOS_IptoStr(localMask,(XCHAR*)pmask);
    XOS_IptoStr(localBroadcast,(XCHAR*)pbroadcast);

    /*�޸�������/etc/sysconfig/network-scripts/�����ļ�����*/
    if(XSUCC != XOS_ModifyIfcfgByName(name,(XS8*)pnewIP))
    {
        printf("XOS_ModifyIfcfgByName failed.\r\n");
        return XERROR;
    }
    
    if(XSUCC != XOS_LocalIPModify (name , (XS8*)poldIP, (XS8*)pnewIP, (XS8*)pmask, (XS8*)pbroadcast))
    {
        printf("XOS_LocalIPModify failed.\r\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
������:  XOS_ModifyIfcfgByName
���ܣ�   - �޸������������ļ���д����IP��ַ
���룺   - name Ҫ�޸ĵ�����������
         - newip ��IP�����ʮ���Ƹ�ʽ
�����
���أ�
XSUCC    - �ɹ�
XERROR   - ʧ��
************************************************************************/
XS32 XOS_ModifyIfcfgByName(XS8 *name,XS8 *newip)
{
    FILE *fp=NULL;
    XS8 buf[1024] = {0};
    XS8 buftmp[1024]={0};
    XS8 *tmp = NULL,*tmp2=NULL;
    struct in_addr ip;
    XS8 cfgPath[64] = "/etc/sysconfig/network-scripts/ifcfg-%s";
    XS8 cfgFile[64] = {0};
    
    /*�ж����*/
    if(NULL == name || NULL == newip)
    {
         printf("pls input the para name or newIP!\r\n");
         return XERROR;
    }

    if(0 == inet_aton((const char*)newip,&ip))
    {
        printf("new ip address is invalid \r\n");
        return XERROR;
    }

    XOS_Sprintf(cfgFile, sizeof(cfgFile) - 1, cfgPath,name);
    /*��ȡ��ǰ�ļ�����*/
    fp = XOS_Fopen(cfgFile,"r");
    if(NULL == fp)
    {
        printf("fopen err.");
        return XERROR;
    }
    XOS_Fread(buf,1,1000,fp);
    XOS_Fclose(fp);

    /*����IP��*/
    if(NULL == (tmp = XOS_StrStr(buf,"IPADDR=")))
    {
        printf("The file is not the ifcfg file\r\n");
        return XERROR;
    }
    tmp += XOS_StrLen("IPADDR=");
    tmp2 = tmp;

    /*�ҵ���β*/
    while('\n'!=*tmp)
    {
        tmp++;
    }

    /*������������*/
    XOS_MemCpy(buftmp,tmp,XOS_StrLen(tmp));

    XOS_MemCpy(tmp2,newip,XOS_StrLen(newip));
    tmp2 += strlen(newip);
  
    /*�ϲ��ַ���*/
    if(((sizeof(buf)- (tmp2 - buf)) - 1) >=  XOS_StrLen(buftmp))
    {
        XOS_MemCpy(tmp2,buftmp,XOS_StrLen(buftmp));
    }
    else
    {
        return XERROR;
    }    
    
    *(tmp2+XOS_StrLen(buftmp))='\0';

    /*д���ļ�*/
    fp = XOS_Fopen(cfgFile,"w");
    if(NULL == fp)
    {
        printf("fopen err.");
        return XERROR;
    }
    XOS_Fwrite(buf,1,XOS_StrLen(buf),fp);
    XOS_Fclose(fp);

    return XSUCC;
}

#endif

#ifdef __cplusplus
}
#endif /*__cplusplus */
