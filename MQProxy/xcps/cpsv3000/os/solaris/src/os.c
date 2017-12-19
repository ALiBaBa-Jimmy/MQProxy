/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: os.c
**
**  description:  
**
**  author: wentao
**
**  data:   2006.8.30
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wentao         2006.8.30             create  
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include <netdb.h>

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define statfilesize 256


/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/
/************************************************************************
函数名:    XOS_StrtoIp
功能：将网络地址转换成网络二进制的数字
输入：
pString  -   要转换的网络地址
inetAddress - 用来保存返回的网络二进制的数字
输出：inetAddress - 网络二进制的数字
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_StrtoIp( XCHAR *pString , XU32 *inetAddress )
{
    if(BLANK_ULONG == (*inetAddress = XOS_NtoHl( inet_addr((XCONST XU8 *)pString ))))
    {
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
函数名:    XOS_StrNtoIp
功能：将网络地址转换成网络二进制的数字，输入的网络地址字符串若不按'\0'结束，
则只按前len个字符进行处理;例如："168.0.2.258"
输入：
pString  -   要转换的网络地址
inetAddress - 用来保存返回的网络二进制的数字
len    -    网络地址字符串pString的长度
输出：inetAddress - 网络二进制的数字
返回值：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_StrNtoIp ( XCHAR *pString , XU32 *inetAddress , XU32 len)
{
    XU32 i = 0;
    char *str = XNULL;
    
    XOS_UNUSED(i);
    
    if(pString == XNULL || inetAddress == XNULL || len == 0 || (len+1) > 32)
    {
        return XERROR;
    }
    
    str = (char *)malloc(len+1);
    if(str == XNULL)
    {
        return XERROR;
    }
    
    strncpy(str,pString,len);
    str[len] = '\0';
    
#if 0
    if( *(pString+len-1) != '\0')
    {
        for(i = 0; i <= len ; i++)
        {
            if(i == len)
                str[i] = '\0';
            else
                str[i] = *(pString+i);            
        }
    }
    else
    {
        if(BLANK_ULONG == (*inetAddress = XOS_NtoHl( inet_addr((XCONST XU8 *)pString ))))
        {
            free(str);
            return XERROR;
        }
        else
        {
            free(str);
            return XSUCC;
        }
    }
#endif
    
    if(BLANK_ULONG == (*inetAddress = XOS_NtoHl( inet_addr((XCONST XU8 *)str ))))
    {
        free(str);
        return XERROR;
    }
    
    free(str);
    return XSUCC;
    
}

/************************************************************************
函数名:    XOS_IptoStr
功能：将网络二进制的数字转换成网络地址
输入：
inetAddress  -   要转换的网络二进制数字
pString - 用来保存返回的点分十进制IP
输出：pString - 点分十进制IP
返回：
XSUCC    -    成功
XERROR    -    失败
说明：pString为长度16的字符数组，即XCHAR pString[16]
************************************************************************/
XS32 XOS_IptoStr( XU32 inetAddress , XCHAR *pString )
{
    char buf[XOS_INET_ADDR_LEN]= {0};
    struct in_addr addr;

    addr.s_addr = XOS_HtoNl(inetAddress);

    XOS_Inet_ntoa(addr, buf); /*返回一个带0的字符串*/

    XOS_MemCpy(pString, buf, XOS_StrLen(buf)+1);

    return XSUCC;
}

/************************************************************************
函数名:    XOS_IpNtoStr
功能：将网络二进制的数字转换成网络地址并写入pString里（只写入前len个字符或遇'\0'止）
输入：
inetAddress  -   要转换的网络二进制数字（注意不能超出范围，）
pString - 用来保存返回的点分十进制IP
len    -    网络地址字符串pString的长度

输出：pString - 点分十进制IP
返回值：XSUCC    -    成功
XERROR    -    失败
说明：一般pString为长度16的字符数组空间，即XCHAR pString[16];
注：VXWORKS下调用的系统函数为 inet_ntoa_b ，因为inet_ntoa ：
Each time this routine is called, 18 bytes are allocated from memory.
************************************************************************/
XS32 XOS_IpNtoStr ( XU32 inetAddress , XCHAR *pString , XU32 len)
{
    char buf[XOS_INET_ADDR_LEN] = {0};

    struct in_addr addr;
    
    if(pString == XNULL || len == 0)
    {
        return XERROR;
    }
    
    addr.s_addr = XOS_HtoNl(inetAddress);

    XOS_Inet_ntoa(addr, buf);


    if(len > XOS_StrLen(buf)+1)
    {
        XOS_MemCpy(pString, buf, XOS_StrLen(buf)+1);
    }
    else
    {
        XOS_MemCpy(pString, buf, len);
        *(pString+len) = '\0';
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_StrNtoXU32
功能：将字符串转换成长整型数（只转换前len个字符）;
输入：
string    -    要转换的字符串
len    -    字符串string的长度
输出：value    -    转换得到的长整型数
返回：
XSUCC    -    成功
XERROR    -    失败
说明：转换时会跳过前面的空格字符，直到遇到数字时才开始转换，
到再次遇到非数字或字符串结束符'\0'时返回。
************************************************************************/
XS32 XOS_StrNtoXU32( XCHAR *string , XU32 *value ,XU32 len )
{
    XU32 i = 0;
    char *str = XNULL;
    
    XOS_UNUSED(i);
    
    if(string == XNULL || value == XNULL || len == 0 || (len+1) > MAXNUMSTRLEN)
    {
        return XERROR;
    }
    
    str = (char *)malloc(len+1);
    if(str == XNULL)
    {
        return XERROR;
    }
    
    strncpy(str,string,len);
    str[len] = '\0';
    
#if 0    
    if( *(string+len-1) != '\0')
    {
        for(i = 0; i <= len ; i++)
        {
            if(i == len)
                str[len] = '\0';
            else
                str[i] = *(string+i);            
        }
    }
    else
    {
        if(0L == (*value = atol((const char *)string)))
        {
            free(str);
            return XERROR;
        }
        else
        {
            free(str);
            return XSUCC;    
        }
    }
#endif
    
    if(0L == (*value = atol((const char *)str)))
    {
        free(str);
        return XERROR;
    }
    
    free(str);
    return XSUCC;
}


/************************************************************************
函数名:    XOS_XU32NtoStr
功能：将无符号长整型数转换成字符串（size够长就写到最后加入'\0'止，否则只写入size个）
输入：
value    -    要转换的无符号整型
str    -    用来保存转换后的字符串
size    -    字符串的长度
输出：str    -    转换得到的字符串
返回值：
XSUCC    -    成功
XERROR    -    失败
说明: windows 下是ultoa，LINUX下用snprintf(第size个字符被置为'\0')
************************************************************************/
XS32 XOS_XU32NtoStr(XU32 value, XCHAR *str , XU32 size )
{
    if(str == XNULL || size == 0)
    {
        return XERROR;
    }
    
    if(0 > snprintf(str,size,"%u",value))
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_GetSysTime
功能：获取系统时间(精确到千分之一秒)
输入：timex -   用来保存时间的结构体
输出：timex -   当前系统时间（秒. 微秒）
返回：
XSUCC    -    成功
XERROR    -    失败
说明：返回的是公元1970.1.1至今的秒数与微秒数
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex )
{
    struct timeb temp;
    if(-1 == ftime(&temp))
    {
        return XERROR;
    }
    timex->time = temp.time;
    timex->millitm = temp.millitm;
    timex->timezone = temp.timezone;
    return XSUCC;
}



#if 0
/************************************************************************
函数名:    XOS_GetSysInfo
功能：获取系统信息
输入：N/A
输出：systeminfo    -    指向系统信息结构体的指针
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_GetSysInfo(t_XOSSYSINFO *systeminfo)
{

}
#endif


#if 0
/************************************************************************
函数名:    XOS_SemCreate
功能:计数信号量
参数:
Input:
maxNum    -    计数信号量最大值
initNum    -    计数信号量的初值
name      -      信号量名字
output:    semaphore   - 信号量id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum, XU32 maxNum,  const XCHAR *name)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    /*XS32 ret;*/
    XOS_UNUSED(maxNum);/**/
    XOS_UNUSED(name);
    if(-1 == sem_init((sem_t *)semaphore,0,(XU32)initNum))
    {
        return XERROR;
    }
    else
        return XSUCC;
}
#endif


/************************************************************************
函数名:    XOS_SemCreate
功能:计数信号量(无名)
参数:
Input:
initNum    -    计数信号量的初值
name      -      信号量名字
output:    semaphore   - 信号量id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum )
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    /*XS32 ret;*/
    /*XOS_UNUSED(initNum);*/
    if(-1 == sem_init((sem_t *)semaphore,0,(XU32)initNum))
    {
        return XERROR;
    }
    else
        return XSUCC;
}


/************************************************************************
函数名:    XOS_SemGet
功能:获得信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：  //sem_wait(s)，对信号量的值进行-1 操作，
（只有在信号量的值大于0 时才能进行，否则等待）
此信号量支持的最大值是2147483647
************************************************************************/
XS32 XOS_SemGet(t_XOSSEMID *semaphore)
{
    XS32 ret;
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    
    ret = sem_wait((sem_t *)semaphore);
    while(XSUCC != ret  && errno == EINTR )
    {
        XOS_Sleep(1);
        ret = sem_wait((sem_t *)semaphore);
    }
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_SemGetExt
功能:获得信号量
参数:
Input:    semaphore    -    信号量的id
          timeout   超时时间，单位 秒
Return:
XSUCC    -    成功
XERROR    -    失败
说明：  //sem_timedwait(s)，对信号量的值进行-1 操作，
（只有在信号量的值大于0 时才能进行，否则等待）
此信号量支持的最大值是2147483647
************************************************************************/
XS32 XOS_SemGetExt(t_XOSSEMID *semaphore, XS32 timeout)
{
    XS32 ret = 0;
    
    if( NULL == semaphore)
    {
        return XERROR;
    }

    if( -1 >= timeout )
    {
        return XOS_SemGet(semaphore);
    }
    else
    {
        struct timespec waittime;
        waittime.tv_sec = (time(NULL) + timeout);
        waittime.tv_nsec = 0;

        while ((ret = sem_timedwait(semaphore, &waittime)) == -1 && errno == EINTR)
               continue;       /* Restart if interrupted by handler */

        return ( 0 == ret ? XSUCC : XERROR);
    }
}


/************************************************************************
函数名:    XOS_SemPut
功能:释放信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：  //sem_post(s)，对信号量进行+1 操作;
************************************************************************/
XS32 XOS_SemPut(t_XOSSEMID *semaphore)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    if(XSUCC != sem_post((sem_t *)semaphore))
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_SemDelete
功能:删除信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：  //sem_destroy(s),此sem要在没被使用的情况下才能destroy,
Only  a  semaphore  that was created  using  sem_init(3RT)
may   be   destroyed   using sem_destroy();
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    if(XSUCC != sem_destroy((sem_t *)semaphore))
    {
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_MutexCreate
功能：互斥量的初始化
输入：
mutex    -    互斥量ID
type    -    互斥量类型
(OS_LOCK_MUTEX和OS_LOCK_CRITSEC)
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexCreate(t_XOSMUTEXID *mutex )
{
    if(XNULLP == mutex)
    {
        return XERROR;
    }
    
    *mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_MutexLock
功能：获得互斥量
输入：mutex    -    互斥量ID
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：必须是在同一进程的不同线程间使用;
若在同一线程内可以多次lock（但最好与XOS_MutexUnlock成对出现）
************************************************************************/
XS32 XOS_MutexLock(t_XOSMUTEXID *mutex)
{
    if(XNULLP == mutex)
    {
        return XERROR;
    }
    XS32 ret;
    
    ret = pthread_mutex_lock(mutex);
    if(XNULL != ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_MutexUnlock
功能：互斥量解锁
输入：mutex    -    互斥量ID
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexUnlock(t_XOSMUTEXID *mutex)
{
    if(XNULLP == mutex)
    {
        return XERROR;
    }
    
    XS32 ret;
    
    ret = pthread_mutex_unlock(mutex);
    if(XNULL != ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_MutexDelete
功能：互斥量销毁
输入：mutex    -    互斥量ID
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexDelete(t_XOSMUTEXID *mutex)
{
    if(XNULLP == mutex)
    {
        return XERROR;
    }
    /**mutex = XNULL; */
    mutex = (pthread_mutex_t *)XNULLP;
    
    return XSUCC;
}


/************************************************************************
函数名:    XOS_TaskCreate
功能  : 创建任务
参数  :
Input:
pTaskName        -    任务名
iTaskPri        -    任务优先级
iTaskStackSize    -    堆栈大小
fTaskFun        -    任务处理函数
pPar            -    参数
Output:    idTask            -    任务标识
Return:
XSUCC        -    成功
XERROR        -    失败
说明  ：
************************************************************************/
XS32 XOS_TaskCreate(XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask)
{    
    pthread_attr_t attr;
    struct sched_param priority;
    
    if(XNULLP == idTask)
    {
        return XERROR;
    }
    if(XNULLP == fTaskFun)
    {
        return XERROR;
    }    
    
    if(XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    else
        if(TSK_PRIO_LOWEST > iTaskPri || TSK_PRIO_HIGHEST < iTaskPri)
        {
            return XERROR;
        }
        
        XOS_UNUSED(iTaskStackSize);
        XOS_UNUSED(pTaskName);    
        
        if(XSUCC != pthread_attr_init(&attr))
        {
            return XERROR;
        }
        if(XSUCC != pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM))
        {
            return XERROR;
        }        
        if(XSUCC != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
        {
            return XERROR;
        }
        
        /*在创建线程之前设置线程属性对象的调度策略和优先级：*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) /*成功返回0*/
        {
            return XERROR;
        }        
        priority.sched_priority=iTaskPri;
        if(XSUCC != pthread_attr_setschedparam(&attr, (struct sched_param*)&priority))
        {
            return XERROR;
        }            
        
        if(XSUCC != pthread_create(idTask, &attr, fTaskFun, pPar))
        {
            return XERROR;
        }
        
        /*动态地修改正在运行的线程调度优先级和调度策略*/    
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*成功返回0*/
        
        return XSUCC;
}

/************************************************************************
函数名: XOS_TaskCreate_Ex
功能  : 创建任务
Input:
        pTaskName        -    任务名
        iTaskPri        -    任务优先级
        iTaskStackSize    -    堆栈大小
        fTaskFun        -    任务处理函数
        pPar            -    参数      
        pOwnPar :       -    创建函数内部使用的，目前用来传递cpu绑定信息
Output:    
        idTask            -    任务标识
Return:
        XSUCC        -    成功
        XERROR        -    失败
说明  ：比XOS_TaskCreate多了个参数pOwnPar
************************************************************************/
XS32 XOS_TaskCreate_Ex(XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask, XVOID *pOwnPar)
{    
    pthread_attr_t attr;
    struct sched_param priority;
    
    if(XNULLP == idTask)
    {
        return XERROR;
    }
    if(XNULLP == fTaskFun)
    {
        return XERROR;
    }    
    
    if(XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    else
        if(TSK_PRIO_LOWEST > iTaskPri || TSK_PRIO_HIGHEST < iTaskPri)
        {
            return XERROR;
        }
        
        XOS_UNUSED(iTaskStackSize);
        XOS_UNUSED(pTaskName);    
        
        if(XSUCC != pthread_attr_init(&attr))
        {
            return XERROR;
        }
        if(XSUCC != pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM))
        {
            return XERROR;
        }        
        if(XSUCC != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
        {
            return XERROR;
        }
        
        /*在创建线程之前设置线程属性对象的调度策略和优先级：*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) /*成功返回0*/
        {
            return XERROR;
        }        
        priority.sched_priority=iTaskPri;
        if(XSUCC != pthread_attr_setschedparam(&attr, (struct sched_param*)&priority))
        {
            return XERROR;
        }            
        
        if(XSUCC != pthread_create(idTask, &attr, fTaskFun, pPar))
        {
            return XERROR;
        }
        
        /*动态地修改正在运行的线程调度优先级和调度策略*/    
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*成功返回0*/
        
        return XSUCC;
}


/************************************************************************
函数名 : XOS_TaskDel
功能   : 删除任务(请慎用)
参数:
Input :    idTask        -    任务标识
Output:    N/A
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_TaskDel(t_XOSTASKID idtask)
{
    XS32 ret;
    
    if(XNULL == idtask)
    {
        return XERROR;
    }
    
    ret = pthread_cancel(idtask);
    if(XNULL != ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


#if 0
/************************************************************************
函数名:    CM_SockOpen 
功能:打开一个socket
参数:
Input:
type    -    方式(1:TCP 2:UDP 3:RAW)
iProtocol    -    协议号(如果type=3,iProtocol为非零,其它填零)
sock    -    socket句柄
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockOpen(e_XOSIPTYPE type, XS32 iProtocol, XS32 *sock)
{
    if((XOS_IP_TYPE_NOTUSE == type) || (type > XOS_IP_TYPE_MAX) || (XNULL == sock))
    {
        return XERROR;
    }
    if (XOS_IP_TYPE_RAW == type)
    {
        *sock = socket(AF_INET, SOCK_RAW, iProtocol);        
    }
    else
    {
        *sock = socket(AF_INET, (XOS_IP_TYPE_TCP == type) ? SOCK_STREAM : SOCK_DGRAM, 0);
    }
    
    if(XERROR == *sock)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockSetBlk
功能:设置一个socket为阻塞或非阻塞方式
参数:
Input:
sock    -    socket句柄
mode    -    0:阻塞,1:非阻塞
Output:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockSetBlk(XS32 sock, XS32 mode)
{
    XS32 ret;
    ret = ioctl(sock, FIONBIO, (XS32)&mode);
    if(XERROR == ret)
    {
        return XERROR;
    }
    return XSUCC;    
}


#if 0
/************************************************************************
函数名:    CM_SockIPtoul
功能  : 将ipv4地址转换成为ulong型
参数  :
Input:     ipaddr    -    ipv4地址(如:"166.241.5.11")
Return:
network order address
XERROR    -    失败
说明  ：
************************************************************************/
XS32 CM_SockIPtoul(XS8 *ipaddr)
{
    XS32 ret; 
    
    if(XNULL == ipaddr)
    {
        return XERROR;
    }
    
    ret = inet_addr(ipaddr);
    if(XERROR == ret)
    {
        return XERROR;
    }
    
    return ret;
}


/************************************************************************
函数名:    CM_SockUltoIPv4
功能:将ulong地址转换成为ipv4型
参数:
Input:     addr    -    network order address
Return:
ipv4地址(如:"166.241.5.11")
XNULL    -    失败
说明：
************************************************************************/
XS8 *CM_SockUltoIP(struct in_addr addr)
{
    XS8 *ret;
    
    ret = inet_ntoa(addr);
    if(XNULL == ret)
    {
        return XNULL;
    }
    
    return ret;
}
#endif


/************************************************************************
函数名:    CM_SockBind
功能  : 绑定本地端口
参数  :
Input:
sock    -    socket句柄
ipaddr    -    ipv4地址(如:"166.241.5.11")
port    -    绑定的端口
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockBind(XS32 sock, XS8 *ipaddr, XU16 port)
{
    struct sockaddr_in s;
    XS32 ret;
    
    if(XNULL == ipaddr)
    {
        return XERROR;
    }
    
    memset(&s, 0, sizeof(s));
    
    s.sin_family = AF_INET;
    s.sin_port = htons(port);
    s.sin_addr.s_addr = inet_addr(ipaddr);
    
    ret = bind(sock, (struct sockaddr*)&s, sizeof(s));
    if(ret == XERROR)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockListen(XS32 sock);
功能:侦听一个socket
参数:
Input:     sock    -    socket句柄              
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockListen(XS32 sock)
{
    XS32 ret;
    
    ret = listen(sock, 10);
    if(ret == XERROR)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockAccept
功能:接受一个socket
参数:
Input :     sock    -    socket句柄              
Output:
rsock    -    连接的socket句柄
pAddr    -    返回给外部用户的参数
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockAccept(XS32 sock, XS32 *rsock, struct sockaddr_in *pAddr)
{
    struct sockaddr_in *s;
    XS32 size;
    
    if(XNULL == rsock)
    {
        return XERROR;
    }
    
    memset(&s, 0, sizeof(s));
    size = sizeof(s);
    
    *rsock = accept(sock, (struct sockaddr*)s, &size);
    if(XERROR == *rsock)
    {
        return XERROR;
    }
    
    if(XNULL != pAddr)
    {
        memcpy(pAddr, s, sizeof(struct sockaddr_in));
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockConnect
功能:绑定本地端口
参数:
Input:
sock    -    socket句柄
ipaddr    -    ipv4地址(如:"166.241.5.11")
port    -    绑定的端口
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockConnect(XS32 sock, XS8 *ipaddr, XU16 port)
{
    struct sockaddr_in s;
    XS32 ret;
    
    memset(&s, 0, sizeof(s));
    
    s.sin_family = AF_INET;
    s.sin_port = htons(port);
    s.sin_addr.s_addr = inet_addr(ipaddr);
    
    ret= connect(sock, (struct sockaddr*)&s, sizeof(s));
    if(ret == XERROR)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockRecv
功能:接收TCP数据
参数:
Input:
sock    -    socket句柄
buf    -    缓冲区
size    -    缓冲区大小
Return:
XS32    -    接收数据长度
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockRecv(XS32 sock, XU8 *buf, XS32 size)
{
    XS32 i;
    
    if(XNULL == buf)
    {
        return XERROR;
    }
    
    i = recv(sock, (char*)buf, size, 0);    
    if((XNULL == i) || (XERROR == i))
    {
        return XERROR;
    }
    
    return i;
}


/************************************************************************
函数名:    CM_SockSend
功能:发送TCP数据
参数:
Input:
sock    -    socket句柄
buf    -    缓冲区
size    -    缓冲区大小
Return:
XS32    -    接收数据长度
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockSend(XS32 sock, XU8 *buf, XS32 size)
{
    XS32 i;
    
    if(XNULL == buf)
    {
        return XERROR;
    }
    
    i = send(sock, (char*)buf, size, 0);
    if((XNULL == i) || (XERROR == i))
    {
        return XERROR;
    }
    
    return i;
}


/************************************************************************
函数名:    CM_SockUdpRecv
功能:接收UDP数据包
参数:
Input:
sock    -    socket句柄
buf    -    缓冲区
size    -    数据大小
ipaddr    -    ipv4地址(如:"166.241.5.11")
port    -    绑定的端口
Return:
XS32    -    接收数据长度
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockUdpRecv(XS32 sock, XU8 *buf, XS32 size, XS8 *ipaddr, XU16 *port)
{
    struct sockaddr_in s;
    XS32 i, j;
    
    if(XNULL == buf) 
    {
        return XERROR;
    }
    
    i = sizeof(s);
    
    j = recvfrom(sock, (char*)buf, size, 0, (struct sockaddr*)&s, &i);
    if((XNULL == j) || (XERROR == j))
    {
        return XERROR;
    }
    
    if(XNULL != ipaddr)
    {
        ipaddr = inet_ntoa(s.sin_addr);
    }
    else if(XNULL != port)
    {
        *port = s.sin_port;
    }
    
    return j;
}


/************************************************************************
函数名:    CM_SockUdpSend
功能:发送UDP数据包
参数:
Input:
sock    -    socket句柄
buf    -    缓冲区
size    -    数据大小
Output:
ipaddr    -    ipv4地址(如:"166.241.5.11")
port    -    绑定的端口
Return:
XS32    -    发送数据长度
XERROR    -    失败                       
说明：
************************************************************************/
XS32 CM_SockUdpSend(XS32 sock, XU8 *buf, XS32 size, XS8 *ipaddr, XU16 *port)
{
    struct sockaddr_in s;
    XS32 ret;
    
    memset(&s, 0, sizeof(s));
    
    s.sin_family = AF_INET;
    s.sin_port = htons(*port);
    s.sin_addr.s_addr = inet_addr(ipaddr);
    
    ret = sendto(sock, (char*)buf, size, 0, (struct sockaddr*)&s, sizeof(s));
    if((XNULL == ret) || (XERROR == ret))
    {
        return XERROR;
    }
    
    return ret;
}


/************************************************************************
函数名:    CM_SockClose
功能:关闭一个socket句柄
参数:
Input:     sock    -    socket句柄              
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockClose(XS32 sock)
{
    XS32 ret;
    
    ret = close(sock);
    
    if(XERROR == ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_SockIscnt
功能:判断一个socket是否处于连接状态
参数:
Input :     sock    -    socket句柄              
Return:
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 CM_SockIscnt(XS32 sock)
{
    struct sockaddr_in s;
    XS32 i, ret;
    
    i = sizeof(s);
    
    ret = getpeername(sock, (struct sockaddr*)&s, &i);
    if(XERROR == ret )
    {
        return XERROR;
    }
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_Newkey
功能:新建一个键
参数:
Input:     N/A    
Return:
成功    -    指向一个键值的指针
失败    -    XNULLP
说明：
************************************************************************/
/*XSTATIC XVOID *CM_Newkey(XVOID)*/
XPUBLIC XVOID *CM_Newkey(XVOID)
{
    return XNULLP;
}


/************************************************************************
函数名:    CM_Setkey
功能:将线程数据和一个键绑定在一起
参数:
Input:     key    -    指向一个键值的指针
pointer    -    指向要绑定的数据结构的指针
Return:
成功    -    XSUCC
失败    -    XERROR
说明：
************************************************************************/
XPUBLIC XU32 CM_Setkey(XVOID *key, XCONST XVOID *pointer)
{
    XOS_UNUSED(key);
    XOS_UNUSED(pointer);
    
    return XSUCC;
}


/************************************************************************
函数名:    CM_Getkey
功能:取得键绑定的线程数据
参数:
Input:     key    -    指向一个键值的指针
Return:
成功    -    返回键绑定的数据(非0)    
失败    -    XNULLP
说明：
************************************************************************/
XPUBLIC XVOID *CM_Getkey(XVOID *key)
{
    XOS_UNUSED(key);
    
    return XNULLP;
}
#endif


/************************************************************************
函数名:    XOS_SusPend
功能：挂起任务
输入：N/A
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SusPend( XVOID )
{
    while(1)
    {
        pause();
    }
    return XSUCC;
}


/************************************************************************
函数名:    XOS_Sleep
功能：任务睡眠一段时间
输入：ms    -   时间长度(million second)
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_Sleep(XU32 ms)
{
    
    XS32 ret;
    struct timespec req;
    struct timespec rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    
    ret = nanosleep(&req, &rem);
    if( XERROR == ret && errno != EINTR )
    {
        return XERROR;
    }
    
    while( XERROR == ret && errno == EINTR )
    {   
        req = rem;
        ret = nanosleep(&req, &rem);
        if( XERROR == ret && errno != EINTR )
        {
            return XERROR;
        }
        
    }
    
    return XSUCC;
    
}


/************************************************************************
函数名: CM_GetLocalIP
功能: 获取本机 IP地址
输入: 无
输出:
pLocalIPList.nIPNum; 获取的IP 的个数, 
pLocalIPList.localIP[MAX_LOCALIP_NUM]; 32 位 IP .
eg. pLocalIPList.localIP[0] 是第一个本机IP地址.
返回: 本地 IP 地址
说明: 
************************************************************************/
XS32 XOS_GetLocalIP( t_LOCALIPLIST* pLocalIPList  )
{
    XCHAR szLocalIP[32]   = {0};
    struct hostent* pHost = XNULL;
    struct in_addr inAddr;
    XU32 i = 0;
    XCHAR **ptr = XNULL;
    
    if ( XNULL == pLocalIPList )
    {
        return XERROR;
    }
    
    XOS_MemSet( pLocalIPList, 0x00, sizeof(t_LOCALIPLIST) );
    
    if ( 0 != gethostname( szLocalIP, sizeof( szLocalIP ) )  )
    {
        return XERROR;
    }
    
    pLocalIPList->nIPNum = 0;
    pHost = gethostbyname( szLocalIP );
    if ( pHost != XNULL && pHost->h_addrtype == AF_INET )
    {
        for( i = 0, ptr = pHost->h_addr_list; 
             MAX_LOCALIP_NUM > i && XNULL != *ptr;
             i++, ptr++ )
        {    
            XOS_MemSet ( &inAddr, 0x00, sizeof( inAddr ) );
            XOS_MemMove( &inAddr, *ptr, 4 );
            pLocalIPList->localIP[pLocalIPList->nIPNum++] = inAddr.s_addr;
        }
    }
    
    return XSUCC;
}


/**************************************************************
函数名: XOS_Reset
功能: 重启操作系统
输入: type---重启类型，暂未定义
输出: 无
返回: 成功:XSUCC         失败:XERROR
说明: 暂不实现
***************************************************************/
XPUBLIC XS32 XOS_Reset(XU32 type)
{
    printf("not support solaris.\r\n");
    return XSUCC;
}


/**************************************************************
函数名: XOS_SetSysTime
功能: 设置操作系统时间
输入: timex---时间
输出: 无
返回: 成功:XSUCC         失败:XERROR
说明: 暂不实现
***************************************************************/
XS32 XOS_SetSysTime( t_XOSTB *timex )
{
    printf("not support solaris.\r\n");
    return XSUCC;
}


/**************************************************************
函数名: XOS_GetCpuRate
功能: 重启操作系统
输入: rate---保存CPU占用率数据, rate% ,整数rate
输出: 无
返回: 成功:XSUCC         失败:XERROR
说明: 
***************************************************************/
XS32 XOS_GetCpuRate(int *rate)
{
    printf("not support solaris.\r\n");
    return XSUCC;
}

/************************************************************************
函数名:XOS_Strerror
功能:  将传入的errno，转换为对应的错误信息字符串
输入:  errnum  错误码
输出:  
返回: success返回传入的errmsg的地址，fail返回"Unknown error"
说明: 函数返回值，就是字符串首地址，可以直接使用，只能读，不能写
************************************************************************/
XCHAR *XOS_StrError(XS32 errnum)
{
#if 0
    XS32 ret = 0;
    XCHAR *szErrDes;

    if(NULL == errmsg || buflen == 0)
    {
        return NULL;
    }
    
    /* use GNU-specific version */    
    #if _GNU_SOURCE || (_POSIX_C_SOURCE < 200112L && _XOPEN_SOURCE < 600)
        XCHAR* pTemp;
        pTemp = strerror_r(errnum, errmsg, buflen);
        errmsg[buflen-1] = '\0';
        return XOS_StrNcpy(errmsg, pTemp, buflen-1);
    #else  /* use POSIX.1-2001 version #################### here*/
        ret = strerror_r(errnum, errmsg, buflen);
        if(ret)
        {
            if (EINVAL == errno)
                szErrDes = "buflen is not enough";   /* strerror_r返回错误，因为buf长度不够 */
            else /* ERANGE or other*/
                szErrDes = "Invalid argument! Unknow errno!";
        }
        errmsg[buflen-1] = '\0';
        return ret == 0 ? errmsg : XOS_StrNcpy(errmsg,szErrDes,buflen-1);
    #endif
#else
    XCHAR *szErrDes = NULL;
    
    switch(errnum)
    {  
        case   0: szErrDes = "Success";                                            break;
        case   1: szErrDes = "Operation not permitted";                            break;
        case   2: szErrDes = "No such file or directory";                          break;
        case   3: szErrDes = "No such process";                                    break;
        case   4: szErrDes = "Interrupted system call";                            break;
        case   5: szErrDes = "Input/output error";                                 break;
        case   6: szErrDes = "No such device or address";                          break;
        case   7: szErrDes = "Argument list too long";                             break;
        case   8: szErrDes = "Exec format error";                                  break;
        case   9: szErrDes = "Bad file descriptor";                                break;
        case  10: szErrDes = "No child processes";                                 break;
        case  11: szErrDes = "Resource temporarily unavailable";                   break;
        case  12: szErrDes = "Cannot allocate memory";                             break;
        case  13: szErrDes = "Permission denied";                                  break;
        case  14: szErrDes = "Bad address";                                        break;
        case  15: szErrDes = "Block device required";                              break;
        case  16: szErrDes = "Device or resource busy";                            break;
        case  17: szErrDes = "File exists";                                        break;
        case  18: szErrDes = "Invalid cross-device link";                          break;
        case  19: szErrDes = "No such device";                                     break;
        case  20: szErrDes = "Not a directory";                                    break;
        case  21: szErrDes = "Is a directory";                                     break;
        case  22: szErrDes = "Invalid argument";                                   break;
        case  23: szErrDes = "Too many open files in system";                      break;
        case  24: szErrDes = "Too many open files";                                break;
        case  25: szErrDes = "Inappropriate ioctl for device";                     break;
        case  26: szErrDes = "Text file busy";                                     break;
        case  27: szErrDes = "File too large";                                     break;
        case  28: szErrDes = "No space left on device";                            break;
        case  29: szErrDes = "Illegal seek";                                       break;
        case  30: szErrDes = "Read-only file system";                              break;
        case  31: szErrDes = "Too many links";                                     break;
        case  32: szErrDes = "Broken pipe";                                        break;
        case  33: szErrDes = "Numerical argument out of domain";                   break;
        case  34: szErrDes = "Numerical result out of range";                      break;
        case  35: szErrDes = "Resource deadlock avoided";                          break;
        case  36: szErrDes = "File name too long";                                 break;
        case  37: szErrDes = "No locks available";                                 break;
        case  38: szErrDes = "Function not implemented";                           break;
        case  39: szErrDes = "Directory not empty";                                break;
        case  40: szErrDes = "Too many levels of symbolic links";                  break;
        case  42: szErrDes = "No message of desired type";                         break;
        case  43: szErrDes = "Identifier removed";                                 break;
        case  44: szErrDes = "Channel number out of range";                        break;
        case  45: szErrDes = "Level 2 not synchronized";                           break;
        case  46: szErrDes = "Level 3 halted";                                     break;
        case  47: szErrDes = "Level 3 reset";                                      break;
        case  48: szErrDes = "Link number out of range";                           break;
        case  49: szErrDes = "Protocol driver not attached";                       break;
        case  50: szErrDes = "No CSI structure available";                         break;
        case  51: szErrDes = "Level 2 halted";                                     break;
        case  52: szErrDes = "Invalid exchange";                                   break;
        case  53: szErrDes = "Invalid request descriptor";                         break;
        case  54: szErrDes = "Exchange full";                                      break;
        case  55: szErrDes = "No anode";                                           break;
        case  56: szErrDes = "Invalid request code";                               break;
        case  57: szErrDes = "Invalid slot";                                       break;
        case  59: szErrDes = "Bad font file format";                               break;
        case  60: szErrDes = "Device not a stream";                                break;
        case  61: szErrDes = "No data available";                                  break;
        case  62: szErrDes = "Timer expired";                                      break;
        case  63: szErrDes = "Out of streams resources";                           break;
        case  64: szErrDes = "Machine is not on the network";                      break;
        case  65: szErrDes = "Package not installed";                              break;
        case  66: szErrDes = "Object is remote";                                   break;
        case  67: szErrDes = "Link has been severed";                              break;
        case  68: szErrDes = "Advertise error";                                    break;
        case  69: szErrDes = "Srmount error";                                      break;
        case  70: szErrDes = "Communication error on send";                        break;
        case  71: szErrDes = "Protocol error";                                     break;
        case  72: szErrDes = "Multihop attempted";                                 break;
        case  73: szErrDes = "RFS specific error";                                 break;
        case  74: szErrDes = "Bad message";                                        break;
        case  75: szErrDes = "Value too large for defined data type";              break;
        case  76: szErrDes = "Name not unique on network";                         break;
        case  77: szErrDes = "File descriptor in bad state";                       break;
        case  78: szErrDes = "Remote address changed";                             break;
        case  79: szErrDes = "Can not access a needed shared library";             break;
        case  80: szErrDes = "Accessing a corrupted shared library";               break;
        case  81: szErrDes = ".lib section in a.out corrupted";                    break;
        case  82: szErrDes = "Attempting to link in too many shared libraries";    break;
        case  83: szErrDes = "Cannot exec a shared library directly";              break;
        case  84: szErrDes = "Invalid or incomplete multibyte or wide character";  break;  
        case  85: szErrDes = "Interrupted system call should be restarted";        break;
        case  86: szErrDes = "Streams pipe error";                                 break;
        case  87: szErrDes = "Too many users";                                     break;
        case  88: szErrDes = "Socket operation on non-socket";                     break;
        case  89: szErrDes = "Destination address required";                       break;
        case  90: szErrDes = "Message too long";                                   break;
        case  91: szErrDes = "Protocol wrong type for socket";                     break;
        case  92: szErrDes = "Protocol not available";                             break;
        case  93: szErrDes = "Protocol not supported";                             break;
        case  94: szErrDes = "Socket type not supported";                          break;
        case  95: szErrDes = "Operation not supported";                            break;
        case  96: szErrDes = "Protocol family not supported";                      break;
        case  97: szErrDes = "Address family not supported by protocol";           break;
        case  98: szErrDes = "Address already in use";                             break;
        case  99: szErrDes = "Cannot assign requested address";                    break;
        case 100: szErrDes = "Network is down";                                    break;
        case 101: szErrDes = "Network is unreachable";                             break;
        case 102: szErrDes = "Network dropped connection on reset";                break;
        case 103: szErrDes = "Software caused connection abort";                   break;
        case 104: szErrDes = "Connection reset by peer";                           break;
        case 105: szErrDes = "No buffer space available";                          break;
        case 106: szErrDes = "Transport endpoint is already connected";            break;
        case 107: szErrDes = "Transport endpoint is not connected";                break;
        case 108: szErrDes = "Cannot send after transport endpoint shutdown";      break;
        case 109: szErrDes = "Too many references: cannot splice";                 break;
        case 110: szErrDes = "Connection timed out";                               break;
        case 111: szErrDes = "Connection refused";                                 break;
        case 112: szErrDes = "Host is down";                                       break;
        case 113: szErrDes = "No route to host";                                   break;
        case 114: szErrDes = "Operation already in progress";                      break;
        case 115: szErrDes = "Operation now in progress";                          break;
        case 116: szErrDes = "Stale file handle";                                  break;
        case 117: szErrDes = "Structure needs cleaning";                           break;
        case 118: szErrDes = "Not a XENIX named type file";                        break;
        case 119: szErrDes = "No XENIX semaphores available";                      break;
        case 120: szErrDes = "Is a named type file";                               break;
        case 121: szErrDes = "Remote I/O error";                                   break;
        case 122: szErrDes = "Disk quota exceeded";                                break;
        case 123: szErrDes = "No medium found";                                    break;
        case 124: szErrDes = "Wrong medium type";                                  break;
        case 125: szErrDes = "Operation canceled";                                 break;
        case 126: szErrDes = "Required key not available";                         break;
        case 127: szErrDes = "Key has expired";                                    break;
        case 128: szErrDes = "Key has been revoked";                               break;
        case 129: szErrDes = "Key was rejected by service";                        break;
        case 130: szErrDes = "Owner died";                                         break;
        case 131: szErrDes = "State not recoverable";                              break;
        case 132: szErrDes = "Operation not possible due to RF-kill";              break;
        default : szErrDes = "unknown errno!!";                                     break;
    }

    return szErrDes;
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


