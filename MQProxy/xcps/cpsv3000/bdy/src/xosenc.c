/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: osenc.c
**
** description:  多IP管理的接口封装 
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
                包含头文件
-------------------------------------------------------------------------*/ 
#include "xosenc.h"
#include "xosencap.h"
#include "xosinet.h"
#include "xosfilesys.h"
#include "xostrace.h"
#include "xoscfg.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/

#define WIN_IPCFGBUF_LEN 512
#define XOS_NOT_SUPPORT  0xffffffff
 
/*-------------------------------------------------------------------------
                内部数据结构定义
-------------------------------------------------------------------------*/
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
XSTATIC XOS_SOCKET_ID  SockipGet;
#endif

/*保存网卡eth0的物理IP、逻辑IP、掩码和广播地址*/
XU32 g_ulXosPhysicIP = 0;
XU32 g_ulXosLogicIP = 0;
XU32 g_ulXosTspNetMask = 0;
XU32 g_ulXosTspBroadCast = 0;

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/ 

/*-------------------------------------------------------------------------
                函数定义
-------------------------------------------------------------------------*/ 
/************************************************************************
函数名:    XOS_GetLocalIPList
功能：获取本地设备的网络接口和IP地址列表    
输入： pLocalIPList        -        用来保存IP信息列表的结构体
输出：
pLocalIPList.nIPNum    -    获取到IP地址的个数, 
pLocalIPList. LocalIP[i]. Interface        -    IP所属的接口，
pLocalIPList. LocalIP[i]. LocalIPaddr    -    32 位的IP地址

如： pLocalIPList.localIP[0].LocalIPaddr 是列表中第一个本机IP地址.
返回：
XSUCC    -    成功
XERROR    -    失败
说明：目前在windows下无法取得对应的IP地址
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
    
    if(XERROR == XINET_Init())  /*初始化sock lib库*/
    {
        return XERROR;
    }
    
    SockipGet = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
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
    /*从buff获取ip 值*/
    pSockList = (SOCKET_ADDRESS_LIST *)buff;
    
    if( 0 == (pLocalIPList->nIPNum = (XU32)pSockList->iAddressCount))  /*取得IP个数*/
    {
        closesocket(SockipGet);
        return XERROR;
    }
    
    for(i=0; i < XOS_MIN((pLocalIPList->nIPNum),XOS_MAXIPNUM); i++)
    {
        /*目前windows下无法取得接口(连接)名*/
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
    
    SockipGet = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
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
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*获取网卡的个数*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* 首先确定网卡是否工作正常 */      
        ret = ioctl(SockipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*只有up 的网卡配置的ip 才有效 */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*获取该网卡上配置的ip*/
            ret = ioctl(SockipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*屏蔽环回地址*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*获取对应的接口(别)名，如：eth1:1*/  
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
函数名:    XOS_LocalIPAdd
功能： 在本地接口中加入一个IP地址    
输入：
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，
'x'表示第几个别名，"eth0:1"表示第一块网卡的第一个别名）
ipaddress    -    要添加到此接口的IP地址，点分十进制 （字符串的形式，如："192.168.0.1"）
subnetmask    -    指定IP的子网掩码，如："255.255.255.0"
broadcast    -    指定IP的广播地址，如："192.168.0.255"（windows下是默认的,可以填XNULL）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：同一接口（别名）下多次添加相同的IP将返回XERROR；
对于输入的接口名（别名）使用者要保证其可用性，LINUX下可以通过XOS_GetLocalIPList() 
来获取当前的接口名及对应IP来做参考，还要注意掩码及广播地址的格式及有效性。
************************************************************************/
XS32 XOS_LocalIPAdd ( XS8 *xinterface , XS8 *ipaddress , XS8 *subnetmask , XS8 *broadcast )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 lIP = 0;
    XU32 temp = 0;
    
    /*入口参数判断*/
    if( XNULL == xinterface || XNULL == ipaddress || XNULL == subnetmask )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 
        0 == XOS_StrNcmp("", ipaddress, 1) || 0 == XOS_StrNcmp("", subnetmask, 1))
    {
        return XERROR;
    }
    
    
    if(XERROR == XOS_StrNtoIp( ipaddress, &lIP, XOS_IPSTRLEN))  /*IP 转换*/
    {
        return XERROR;
    }
    
    /*判断IP是否存在,若存在就不再添加*/
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
    /*LINUX下若存在“相同的接口别名”，也不再添加 */
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( 0 == XOS_StrNcmp(xinterface,llist.localIP[i].xinterface,XOS_IFNAMESIZE))
        {
            return XERROR;
        }
    }
    
#endif
    
    /*准备好命令字串*/
#ifdef XOS_LINUX
    XOS_StrCat(cmdl,"/sbin/ifconfig ");
    XOS_StrCat(cmdl, xinterface);
    XOS_StrCat(cmdl," ");
    XOS_StrCat(cmdl,ipaddress);
    if(XNULL != broadcast && 0 != XOS_StrNcmp("", broadcast, 1))    /*broad为空的话就设为默认*/
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
    
    /*这里只对system的执行情况进行了处理，未对命令执行后的返回值进行判断*/
    if( -1 == system((XCHAR *)cmdl) )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* 检查IP是否被成功加入*/
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
    
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX下还要判断是否已存在对应的接口名，如：eth1:1 */
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
函数名:    XOS_LocalIPDelete
功能：删除指定接口下的一个IP地址    
输入：
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，
'x'表示第几个别名，eth0:1表示第一块网卡的第一个别名）
ipaddress    -    要删除的IP地址，点分十进制 （字符串的形式，如："192.168.0.1"）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：多次删除同一IP地址将返回XERROR；
对于输入的接口名使用者要保证其可用性，LINUX下可以通过的XOS_GetLocalIPList() 
来获取当前的接口名及对应IP来做参考；
注：只能删除静态IP，每个接口的最后一个静态IP不能删除，LINUX下是主接口（非别名,如:"eth0"）的IP不能删除；
************************************************************************/
XS32 XOS_LocalIPDelete (XS8 *xinterface , XS8 *ipaddress )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 temp = 0;
    XU32 lIP = 0;
    
    /*入口参数判断*/
    if( XNULL == xinterface || XNULL == ipaddress )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 0 == XOS_StrNcmp("", ipaddress, 1))
    {
        return XERROR;
    }
    
    if(XERROR == XOS_StrNtoIp( ipaddress, &lIP , XOS_IPSTRLEN ))  /*IP 转换*/
    {
        return XERROR;
    }
    
    /*判断IP是否存在，若不存在就不再删除*/
    if(XERROR == XOS_GetLocalIPList(&llist))
    {
        return XERROR;
    }
    if( 1 >= llist.nIPNum ) /*本地最后一个IP不允许删除 */
    {
        return XERROR;
    }
#ifdef XOS_WIN32    
    for( i=0; i < llist.nIPNum ; i++ )
    {
        if( lIP == llist.localIP[i].LocalIPaddr )
        {
            temp = 1 ; /*要删除的IP存在,temp置1*/
        }
    }
#endif    
#ifdef XOS_LINUX
    
    if(XNULL == XOS_StrNChr(xinterface, ':', XOS_IFNAMESIZE)) /*LINUX下主接口名不能删除*/
    {
        return XERROR;
    }
    
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX下还要判断将删除的接口(别名)是否存在 */
    {
        if( lIP == llist.localIP[i].LocalIPaddr && 
            0 == XOS_StrNcmp(xinterface, llist.localIP[i].xinterface, XOS_IFNAMESIZE))
        {
            temp = 1 ;   /*找到相同的接口，temp置1*/
        }
    }
    
#endif
    if( 0 == temp ) /*不存在要删除的接口或IP*/
    {
        return XERROR;
    }
    
    /*准备好命令字串*/    
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
    
    /*这里只对system的执行情况进行了处理，未对命令执行后的返回值进行判断*/
    if( -1 == system((XCHAR *)cmdl) )
    {
        return XERROR;
    }
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* 检查IP是否被成功删除*/
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
    /*LINUX下再判断接口是否被成功删除 */
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
函数名:    XOS_LocalIPModify
功能：在指定接口下将ipaddress1 改为ipaddresss2 并指定其子网掩码及广播地址    
输入：
interface    -    指定网络接口的名称（一般对应一个网卡，如：windows下为"本地连接"；
linux下为一个别名"eth0:x"，其中'eth0'为网卡标识，
'x'表示第几个别名，eth0:1表示第一块网卡的第一个别名）
ipaddress1    -    要修改的IP地址，点分十进制 （字符串的形式，如："192.168.0.1"）
ipaddress2    -    修改后的IP地址，如："192.168.0.2"
subnetmask2    -    指定IP的子网掩码，如："255.255.255.0"
broadcast2    -    指定IP的广播地址，如："192.168.0.255"（windows下是默认的,可以填XNULL）
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：对于输入的接口名及IP使用者要保证其可用性，LINUX下可以通过XOS_GetLocalIPList()
来获取当前可用的接口名及对应的IP；
************************************************************************/
XS32 XOS_LocalIPModify (XS8 *xinterface , XS8 *ipaddress1 , 
                        XS8 *ipaddress2 , XS8 *subnetmask2 , XS8 *broadcast2 )
{
#if ( defined (XOS_LINUX) || defined (XOS_WIN32) )
    XU8 cmdl[XOS_MAXCMDLEN] = "";
    t_LOCALIPINFO llist;
    XU32 i = 0;
    XU32 temp = 0;
    XU32 tempt = 0; /*用来检测linux上是否修改成功*/
    XU32 lIP1 = 0;
    XU32 lIP2 = 0;
    
    /*入口参数判断*/
    if( XNULL == xinterface || XNULL == ipaddress1 || XNULL == ipaddress2 || XNULL == subnetmask2 )
    {
        return XERROR;
    }
    if( 0 == XOS_StrNcmp("", xinterface, 1) || 0 == XOS_StrNcmp("", ipaddress1, 1) || 
        0 == XOS_StrNcmp("", ipaddress2, 1) || 0 == XOS_StrNcmp("", subnetmask2, 1))
    {
        return XERROR;
    }
    
    if(XERROR == XOS_StrNtoIp( ipaddress1, &lIP1, XOS_IPSTRLEN ))  /*IP 转换*/
    {
        return XERROR;
    }
    if(XERROR == XOS_StrNtoIp( ipaddress2, &lIP2, XOS_IPSTRLEN ))  /*IP 转换*/
    {
        return XERROR;
    }
    
    /*判断IP是否存在*/
    if(XERROR == XOS_GetLocalIPList(&llist))
    {
        return XERROR;
    }
#ifdef XOS_WIN32    
    for( i=0; i < llist.nIPNum ; i++ ) /*要修改的接口或IP不存在则修改失败*/
    {
        if(lIP1 == llist.localIP[i].LocalIPaddr )
        {
            temp = 1 ;
        }
    }
#endif
#ifdef XOS_LINUX
    for( i=0; i < llist.nIPNum ; i++ )  /*LINUX下还要判断接口下的IP是否存在*/
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
    
    for( i=0; i < llist.nIPNum ; i++ )  /*想修改成的IP若已存在则修改失败****/
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
    
    if(XERROR == XOS_GetLocalIPList(&llist)) /* 检查IP是否被成功修改*/
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
    if( XSUCC != XOS_LocalIPAdd(xinterface, ipaddress2, subnetmask2, broadcast2)) /*先加*/
    {
        return XERROR;
    }
    
    if( XSUCC != XOS_LocalIPDelete(xinterface,ipaddress1))/*再删*/
    {
        /*删失败了就把刚才加的也删掉*/
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
函数名:    XOS_GetPhysicIP
功能：获取物理IP地址
                gpp板下获取第一个IP，默认eth0，tsp板封装Drv_GetMV0PhysicalIP 
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
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
    
    if(XERROR == XINET_Init())  /*初始化sock lib库*/
    {
        return XERROR;
    }
    
    XOS_MemSet(buff, 0, WIN_IPCFGBUF_LEN);
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
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
    /*从buff获取ip 值*/
    pSockList = (SOCKET_ADDRESS_LIST *)buff;
    
    if( 0 == (pLocalIPList.nIPNum = (XU32)pSockList->iAddressCount))  /*取得IP个数*/
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
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
    
    ret = ioctl(PhysicipGet,  SIOCGIFCONF,  (XS8 *)&ifc);
    if(ret != 0)
    {
        close(PhysicipGet);
        return XERROR;
    }
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*获取网卡的个数*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* 首先确定网卡是否工作正常 */      
        ret = ioctl(PhysicipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*只有up 的网卡配置的ip 才有效 */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*获取该网卡上配置的ip*/
            ret = ioctl(PhysicipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*屏蔽环回地址*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*获取对应的接口(别)名，如：eth1:1*/  
            XOS_StrNcpy(pLocalIPList.localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE);        
            
            pLocalIPList.localIP[j].LocalIPaddr= (((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr);
            pLocalIPList.localIP[j].LocalIPaddr= XOS_NtoHl(pLocalIPList.localIP[j].LocalIPaddr);

            /*查找eth0的ip,mak,broadcast*/
            if(0 == XOS_StrCmp(pLocalIPList.localIP[j].xinterface,"eth0"))
            {
                flag = 1;
                *ipaddress = pLocalIPList.localIP[j].LocalIPaddr;
                
                g_ulXosPhysicIP = pLocalIPList.localIP[j].LocalIPaddr;
                /*获取掩码并保存*/
                ret = ioctl(PhysicipGet,SIOCGIFNETMASK,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                g_ulXosTspNetMask = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));
                /*获取广播地址并保存*/
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
        /*没找到eth0*/
        close(PhysicipGet);
        return XERROR;
    }

    close(PhysicipGet);
    return XSUCC;
#endif

/*tsp板默认mv0*/
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
函数名:    XOS_ModifyPhysicIP
功能：修改物理IP地址
                  gpp板下修改第一个IP，默认eth0，tsp板封装Drv_MV0PhysicalIPModify  
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
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

    /*修改/etc/sysconfig/network-scripts/ifcfg-eth0文件内容*/
    if(XSUCC != XOS_ModifyIfcfgOneLine((XS8*)pnewIP))
    {
        printf("XOS_ModifyIfcfgOneLine failed.\r\n");
        return XERROR;
    }
    
    /*调用ifconfig 命令修改eth0网卡的ip*/
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
函数名:    XOS_GetLogicIP
功能：获取逻辑IP地址，tsp板封装getLogicIp  
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:     linux windows下暂不支持
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
函数名:    XOS_AddLogicIP
功能：设置逻辑IP，tsp板封装Drv_MV0LogicIPAdd  
输入：ipaddress 物理IP，整型指针,逻辑ip和物理ip同一网段，
                  取物理ip的广播地址和掩码
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:     linux windows下暂不支持
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
函数名:    XOS_DeleteLogicIP
功能：删除逻辑IP，tsp板封装Drv_MV0LogicIPDelete  
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:     linux windows下暂不支持
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
函数名:    XOS_ModifyLogicIP
功能：修改逻辑IP，tsp板封装Drv_MV0LogicIPModify 
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:     linux windows下暂不支持
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
函数名:    XOS_GetMv2IP
功能：获取内部IP，tsp板封装Drv_GetMV2IP  
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:    linux windows下无此接口
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
函数名:    XOS_GetMv1IP
功能：获取内部IP，tsp板封装Drv_GetMV1IP  
输入：ipaddress 物理IP，整型指针
输出：ip地址
返回：
XSUCC    -    成功
XERROR   -    失败
说明:    linux windows下无此接口
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
函数名:    XOS_ModifyIfcfgOneLine
功能：修改ifcfg-eth0文件的IPADDR=行
输入：newip 新ip字符指针，点分十进制格式
输出：
返回：
XSUCC    -    成功
XERROR   -    失败
说明:    linux windows下无此接口
************************************************************************/
#ifdef XOS_LINUX
XS32 XOS_ModifyIfcfgOneLine(XS8 *newip)
{
    return XOS_ModifyIfcfgByName("eth0",newip);
}

/************************************************************************
函数名:    XOS_GetPhysicIPByName
功能：根据网卡名获取物理IP地址，掩码，广播地址
输入：网卡名字
输出：ip地址，掩码，广播地址
返回：
XSUCC    -    成功
XERROR   -    失败
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
    
    PhysicipGet = socket(AF_INET, SOCK_STREAM, 0); /*初始化socket*/
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
    
    netCards  =  ifc.ifc_len/sizeof( struct ifreq );  /*获取网卡的个数*/
    for(i=0, j=0; i < XOS_MIN(netCards, XOS_MAXIPNUM); i++)
    {
        /* 首先确定网卡是否工作正常 */      
        ret = ioctl(PhysicipGet,  SIOCGIFFLAGS,  (XS8 *)&buf[i] );
        if(ret != 0)
        {
            continue;
        }
        /*只有up 的网卡配置的ip 才有效 */      
        if( buf[i].ifr_flags  &  IFF_UP )  
        {
            /*获取该网卡上配置的ip*/
            ret = ioctl(PhysicipGet,  SIOCGIFADDR,  (XS8 *)&buf[i] ); 
            if(ret != 0  )
            {
                continue;
            }
            /*屏蔽环回地址*/
            if(XOS_NtoHl(((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr) == 0x7f000001)
            {
                continue;
            }
            
            /*获取对应的接口(别)名，如：eth1:1*/  
            XOS_StrNcpy(pLocalIPList.localIP[j].xinterface,buf[i].ifr_name,XOS_IFNAMESIZE);        
            
            pLocalIPList.localIP[j].LocalIPaddr= (((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr);
            pLocalIPList.localIP[j].LocalIPaddr= XOS_NtoHl(pLocalIPList.localIP[j].LocalIPaddr);

            /*查找这个网卡的ip,mak,broadcast*/
            if(0 == XOS_StrCmp(pLocalIPList.localIP[j].xinterface,name))
            {
                flag = 1;
                *ipaddress = pLocalIPList.localIP[j].LocalIPaddr;
                
                /*获取掩码*/
                ret = ioctl(PhysicipGet,SIOCGIFNETMASK,(char *)&buf[i]);
                if(ret != 0  )
                {
                    close(PhysicipGet);
                    return XERROR;
                }
                *mask = ntohl((((struct  sockaddr_in  *)(&buf[i].ifr_addr))->sin_addr.s_addr));
                /*获取广播地址*/
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
        /*没找到这个网卡*/
        close(PhysicipGet);
        return XERROR;
    }

    close(PhysicipGet);
    return XSUCC;
}
/************************************************************************
函数名:    XOS_ModifyPhysicIPByName
功能：根据名字修改物理IP地址
输入：name 要修改的网卡的名字
        ipaddress 物理IP，整型指针
输出：
返回：
XSUCC    -    成功
XERROR   -    失败
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

    /*修改网卡在/etc/sysconfig/network-scripts/配置文件内容*/
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
函数名:  XOS_ModifyIfcfgByName
功能：   - 修改网卡的配置文件，写入新IP地址
输入：   - name 要修改的网卡的名字
         - newip 新IP，点分十进制格式
输出：
返回：
XSUCC    - 成功
XERROR   - 失败
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
    
    /*判断入参*/
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
    /*读取当前文件内容*/
    fp = XOS_Fopen(cfgFile,"r");
    if(NULL == fp)
    {
        printf("fopen err.");
        return XERROR;
    }
    XOS_Fread(buf,1,1000,fp);
    XOS_Fclose(fp);

    /*查找IP项*/
    if(NULL == (tmp = XOS_StrStr(buf,"IPADDR=")))
    {
        printf("The file is not the ifcfg file\r\n");
        return XERROR;
    }
    tmp += XOS_StrLen("IPADDR=");
    tmp2 = tmp;

    /*找到结尾*/
    while('\n'!=*tmp)
    {
        tmp++;
    }

    /*拷贝后续内容*/
    XOS_MemCpy(buftmp,tmp,XOS_StrLen(tmp));

    XOS_MemCpy(tmp2,newip,XOS_StrLen(newip));
    tmp2 += strlen(newip);
  
    /*合并字符串*/
    if(((sizeof(buf)- (tmp2 - buf)) - 1) >=  XOS_StrLen(buftmp))
    {
        XOS_MemCpy(tmp2,buftmp,XOS_StrLen(buftmp));
    }
    else
    {
        return XERROR;
    }    
    
    *(tmp2+XOS_StrLen(buftmp))='\0';

    /*写入文件*/
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
