/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: os.c
**
** description:  给予平台内部的接口封装 
**
** author: wentao
**
** data:   2006.4.14
**
***************************************************************
**                         history                     
** 
***************************************************************
**  author          date              modification            
**  wentao         2006.4.14              create  
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/ 

#include "xosencap.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define INVALID_SEM_ID   (-1)

PROCNTQSI NtQuerySystemInformation;

/*-------------------------------------------------------------------------
                内部数据结构定义
-------------------------------------------------------------------------*/

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
    if(INADDR_NONE == (*inetAddress = XOS_NtoHl( inet_addr((const char *)pString ))))
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
说明：各操作系统下存在差异：比如"202.0.2."在Windows、Linux下被认为是不符合
格式的IP地址，但在Solaris和vxWork下却能成功转换；所以，为了
不出现使用上的问题，请务必以"168.0.2.258"的格式输入IP。
************************************************************************/
XS32 XOS_StrNtoIp ( XCHAR *pString , XU32 *inetAddress , XU32 len)
{
    XU32 i = 0;    
    char *str = XNULL;
    
    XOS_UNUSED(i);
    
    if(pString == XNULL || inetAddress == XNULL || len == 0 || (len+1) > 32 )
    {
        return XERROR;
    }
    
    str = (char *)malloc(len+1);
    if(str == XNULL)
    {
        return XERROR;
    }
    
    strncpy(str,pString,(size_t)len);
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
        if(INADDR_NONE == (*inetAddress = XOS_NtoHl( inet_addr((XCONST XU8 *)pString ))))
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
    
    if(INADDR_NONE == (*inetAddress = XOS_NtoHl( inet_addr((const char *)str ))))
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
    char buf[XOS_INET_ADDR_LEN] = {0};
    struct in_addr addr;

    addr.S_un.S_addr = XOS_HtoNl(inetAddress);

    XOS_Inet_ntoa(addr, buf);

    XOS_MemCpy(pString, buf, XOS_StrLen(buf)+1);  

   return XSUCC;
}


/************************************************************************
函数名:    XOS_IpNtoStr
功能：将网络二进制的数字转换成网络地址并写入pString里（只写入前len个字符或遇'\0'止）
输入：
inetAddress  -   要转换的网络二进制数字（注意不能超出范围）
pString - 用来保存返回的点分十进制IP
len    -    网络地址字符串pString的长度
输出：pString - 点分十进制IP
返回值：
XSUCC    -    成功
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
    
    addr.S_un.S_addr = XOS_HtoNl(inetAddress);

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
string    -    要转换的字符串,如："  1234567"
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
说明: windows 下是ultoa，LINUX下用snprintf
************************************************************************/
#if 0
XU32 XOS_XU32NtoStr(XU32 value, XCHAR *str , XU32 size )
{
    XCHAR temp[33] = "";
    
    if(str == XNULL || size == 0)
    {
        return XERROR;
    }
    
    ultoa(value, temp, 10);
    
    if(size > XOS_StrLen(temp)+1)
    {
        XOS_MemCpy(str, temp, XOS_StrLen(temp)+1);
    }
    else
    {
        XOS_MemCpy(str, temp, size);
        *(str+size) = '\0';
    }
    
    return XSUCC;
}
#endif


XS32 XOS_XU32NtoStr(XU32 value, XCHAR *str , XU32 size )
{
    XCHAR st[32] = {0};
    
    if(str == XNULL || size == 0)
    {
        return XERROR;
    }
    
    sprintf(st,"%u",value);
    strncpy(str,st,size);
    str[size] = '\0';

    return XSUCC;
}


/************************************************************************
函数名:    XOS_GetSysTime
功能：获取系统时间(精确到千分之一秒)
输入：timex -   用来保存时间的结构体
输出：timex -   当前系统时间（秒,毫秒及 目前时区和UTC相差的时间，单位为分钟）
返回：
XSUCC    -    成功
XERROR    -    失败
说明：返回的是公元1970.1.1至今的秒数与毫秒数
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex )
{
    struct timeb temp;
    ftime(&temp);
    timex->time = (XU32)temp.time;
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
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime = 0.0;
    double                         dbSystemTime;
    LONG                           cpustatus;
    LARGE_INTEGER                  liOldIdleTime = {0};
    LARGE_INTEGER                  liOldSystemTime = {0};
    
    struct tm *SystemTime;
    time_t ltime;
    
    MEMORYSTATUS MemStat;
    
    HANDLE hSnapshot = NULL;
    PROCESSENTRY32 syspe;
    unsigned int i = 0;
    
    if( XNULL == systeminfo )
    {
        return XERROR;
    }
    /***** 获取CPU信息(CPU个数和占用率) *****/
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
        GetModuleHandle("ntdll"),
        "NtQuerySystemInformation"
        );
    
    if (!NtQuerySystemInformation)
        return  XERROR;
    
    /*get number of processors in the system*/
    cpustatus = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (cpustatus != NO_ERROR)
        return XERROR;
    
    systeminfo->dwNumberOfProcessors = SysBaseInfo.bKeNumberProcessors; /*CPU个数*/
    
    /*printf("\nCPU Usage (press any key to exit):  ");*/
    
    /*while(!kbhit())*/
    for(i= 0;i <= 1;i++)    /*取两次CPU信息,间隔为1s */
    {
        /*get new system time*/
        cpustatus = NtQuerySystemInformation( SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (cpustatus!=NO_ERROR)
            return XERROR;
        
        /*get new CPU's idle time*/
        cpustatus = NtQuerySystemInformation( SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (cpustatus != NO_ERROR)
            return XERROR;
        
        /*if it's a first call - skip it*/
        if (liOldIdleTime.QuadPart != 0) /* * */
        {
            /*CurrentValue = NewValue - OldValue*/
            dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
            dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
            
            /*CurrentCpuIdle = IdleTime / SystemTime*/
            dbIdleTime = dbIdleTime / dbSystemTime;
            
            /* CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors*/
            dbIdleTime = ( 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors ) + 0.5;
            
            /*printf("\r%3d%%",(UINT)dbIdleTime);*/
            
        }
        
        
        /* store new CPU's idle and system time*/
        liOldIdleTime = SysPerfInfo.liIdleTime;
        liOldSystemTime = SysTimeInfo.liKeSystemTime;
        
        /* wait one second*/
        Sleep(1000);
    }
    
    systeminfo->dwProcessorLoad = (UINT)dbIdleTime; /*CPU占用率*/    
    
    /***** 获取系统时间*****/
    ltime = time(XNULLP);
    SystemTime = localtime(&ltime);
    systeminfo->localtime.dt_year = SystemTime->tm_year;
    systeminfo->localtime.dt_mon = SystemTime->tm_mon;
    systeminfo->localtime.dt_mday = SystemTime->tm_mday;
    systeminfo->localtime.dt_hour = SystemTime->tm_hour;
    systeminfo->localtime.dt_min = SystemTime->tm_min;
    systeminfo->localtime.dt_sec = SystemTime->tm_sec;
    systeminfo->localtime.dt_wday = SystemTime->tm_wday;  /*星期几*/
    systeminfo->localtime.dt_yday = SystemTime->tm_yday;
    systeminfo->localtime.dt_isdst = SystemTime->tm_isdst;
    
    /***** 获取内存信息*****/
    GlobalMemoryStatus (&MemStat);
    systeminfo->dwMemoryLoad = MemStat.dwMemoryLoad;
    systeminfo->dwTotalPhys = (XU32)MemStat.dwTotalPhys/DIV;
    systeminfo->dwAvailPhys = (XU32)MemStat.dwAvailPhys/DIV;
    systeminfo->dwTotalPageFile = (XU32)MemStat.dwTotalPageFile/DIV;
    systeminfo->dwAvailPageFile = (XU32)MemStat.dwAvailPageFile/DIV;
    
    /***** 获取进程信息*****/
    if( INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)))
        return XERROR;
    if(TRUE != Process32First(hSnapshot, &syspe))
        return XERROR;
    /*printf("");*/
    i = 0;  /* 用来存储任务个数*/
    do
    {
        i++;
        /*printf("%-16s %4d  %ld %d %d %d %2d %4d %2d %d\n",pe.szExeFile,pe.th32ProcessID,*/
        /*    pe.dwSize,pe.cntUsage,pe.th32DefaultHeapID,pe.th32ModuleID,pe.cntThreads,pe.th32ParentProcessID,pe.pcPriClassBase,pe.dwFlags);*/
    } while(Process32Next(hSnapshot, &syspe));
    
    systeminfo->dwtotalthreads = i;
    /*printf("the number of processes : %d\n",i);*/
    if(XNULL == CloseHandle(hSnapshot))
        return XERROR;
    return XSUCC;
}
#endif


#if 0
/***********************************************************************/
XEXTERN XVOID CALLBACK timeSetEvent_Timeout(XU32 wTimerID, XU32 msg, XS32 dwUser,
                                            XS32 dw1, XS32 dw2);
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
    if(XNULL == semaphore)
    {
        return XERROR;
    }
    
    XOS_UNUSED(name);
    
    semaphore->semaId = CreateSemaphore((LPSECURITY_ATTRIBUTES)XNULLP, (LONG)initNum, (LONG)maxNum, XNULLP);
#if 0
    if(ERROR_ALREADY_EXISTS == GetLastError())
    {
        return XERROR;
    }
#endif
    if(XNULL == semaphore->semaId)
    {
        return XERROR;
    }
    
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
    if(XNULL == semaphore)
    {
        return XERROR;
    }
    
    semaphore->semaId = CreateSemaphore((LPSECURITY_ATTRIBUTES)XNULLP, (LONG)initNum, (LONG)2147483647, (LPCTSTR)XNULLP);
#if 0
    if(ERROR_ALREADY_EXISTS == GetLastError())
    {
        return XERROR;
    }
#endif
    if(XNULL == semaphore->semaId)
    {
        return XERROR;
    }

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
说明：
************************************************************************/
XS32 XOS_SemGet(t_XOSSEMID *semaphore)
{
    
    if(XNULL == semaphore|| semaphore->semaId == (HANDLE)INVALID_SEM_ID)
    {
        return XERROR;
    }
    
    if(WAIT_FAILED  == WaitForSingleObject(semaphore->semaId, INFINITE))
    {
        return XERROR;
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
说明：  //WaitForSingleObject(s)，对信号量的值进行-1 操作，
（只有在信号量的值大于0 时才能进行，否则等待）
此信号量支持的最大值是2147483647
************************************************************************/
XS32 XOS_SemGetExt(t_XOSSEMID *semaphore, XS32 timeout)
{
    
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
        /*超时或异常*/
        if(WAIT_OBJECT_0 != WaitForSingleObject(semaphore->semaId, timeout*1000))
        {
            return XERROR;
        }
        else
        {
            return XSUCC;
        }
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
说明：
************************************************************************/
XS32 XOS_SemPut(t_XOSSEMID *semaphore)
{
    XS32 ret;
    
    if(XNULL == semaphore || semaphore->semaId == (HANDLE)INVALID_SEM_ID)
    {
        return XERROR;
    }
    
    ret = ReleaseSemaphore(semaphore->semaId, 1, XNULL);
    if(0 == ret)
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
说明：
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore)
{
    XS32 ret;
    
    if(XNULL == semaphore)
    {
        return XERROR;
    }
    
    ret = CloseHandle(semaphore->semaId);
    semaphore->semaId = (HANDLE)INVALID_SEM_ID;
    if(0 == ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


#if 0        /*mod by 06.5.12*/
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
XS32 XOS_MutexCreate(t_XOSMUTEXID *mutex , XU32 type)
{
    if(XNULL == mutex)
    {
        return XERROR;
    }
    mutex->LockType = type;
    
    if (mutex->LockType == OS_LOCK_MUTEX)
    {
        mutex->LockID.mutex = CreateMutex(NULL, FALSE, NULL);
        if (mutex->LockID.mutex == NULL)
        {
            return(XERROR);
        }
        return(XSUCC);
    }
    
    if (mutex->LockType == OS_LOCK_CRITSEC)
    {
        InitializeCriticalSection(&mutex->LockID.critical);
        return(XSUCC);
    }
    
    /*InitializeCriticalSection(mutex);*/
    
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
说明：
************************************************************************/
XS32 XOS_MutexLock(t_XOSMUTEXID *mutex)
{
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    if (mutex->LockType == OS_LOCK_MUTEX)
    {
        if (WaitForSingleObject(mutex->LockID.mutex, INFINITE) == WAIT_OBJECT_0)
            return(XSUCC);
        else
            return(XERROR);
    }
    
    if (mutex->LockType == OS_LOCK_CRITSEC)
    {
        EnterCriticalSection(&mutex->LockID.critical);
        return(XSUCC);
    }
    
    /*EnterCriticalSection(mutex);*/
    
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
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    if (mutex->LockType == OS_LOCK_MUTEX)
    {
        if(0 == ReleaseMutex(mutex->LockID.mutex))
            return(XERROR);
        else
            return(XSUCC);
    }
    if (mutex->LockType == OS_LOCK_CRITSEC)
    {
        LeaveCriticalSection(&mutex->LockID.critical);
        return(XSUCC);
    }
    
    /*LeaveCriticalSection(mutex);*/
    
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
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    if (mutex->LockType == OS_LOCK_MUTEX)
    {
        if(0 == CloseHandle(mutex->LockID.mutex))
            return(XERROR);
        else
            return(XSUCC);
    }
    if (mutex->LockType == OS_LOCK_CRITSEC)
    {
        DeleteCriticalSection(&mutex->LockID.critical);
        return(XSUCC);
    }
    
    /*DeleteCriticalSection(mutex);*/
    
    return XSUCC;
}
#endif


#if 1        /*mod by 06.5.12*/
/************************************************************************
函数名:    XOS_MutexCreate
功能：互斥量的初始化
输入：mutex    -    互斥量ID
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexCreate(t_XOSMUTEXID *mutex)
{
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    InitializeCriticalSection(mutex);
    
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
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    EnterCriticalSection(mutex);
    
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
说明：同上
************************************************************************/
XS32 XOS_MutexUnlock(t_XOSMUTEXID *mutex)
{
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    LeaveCriticalSection(mutex);
    
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
    if(XNULL == mutex)
    {
        return XERROR;
    }
    
    DeleteCriticalSection(mutex);
    
    return XSUCC;
}
#endif


#if 0
XPUBLIC XS32 OS_Initialize(XVOID)
{      
    return XSUCC;
}
#endif


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
XS32 XOS_TaskCreate(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask)
{     
    if(XNULLP == idTask)
    {
        return XERROR;
    }
    if(XNULLP == fTaskFun)
    {
        return XERROR;
    }    
    /*    if(XNULLP == pTaskName)
    {
    return XERROR;
    }
    */    
    if(0 > iTaskStackSize || XOSTASKSIZEMAX < iTaskStackSize )
    {
        return XERROR;
    }
    if( XNULL == iTaskStackSize )
    {
        iTaskStackSize = XOSTASKSIZEDEFAULT;
    }
    
    if(TSK_PRIO_LOWEST > iTaskPri || TSK_PRIO_HIGHEST < iTaskPri)
    {
        return XERROR;
    }
    if( XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    
    XOS_UNUSED(pTaskName);  
    
    *idTask = (t_XOSTASKID) _beginthread(fTaskFun, iTaskStackSize, pPar);
    if(-1 == (XS32)( *idTask) )
    {
        return XERROR;
    }
    if(0 == SetThreadPriority( (HANDLE)(*idTask),(XS32)(iTaskPri-2)))
    {
        return XERROR;
    }
    
    
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
XS32 XOS_TaskCreate_Ex(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask, XVOID *pOwnPar)
{     
    if(XNULLP == idTask)
    {
        return XERROR;
    }
    if(XNULLP == fTaskFun)
    {
        return XERROR;
    }    
    /*    if(XNULLP == pTaskName)
    {
    return XERROR;
    }
    */    
    if(0 > iTaskStackSize || XOSTASKSIZEMAX < iTaskStackSize )
    {
        return XERROR;
    }
    if( XNULL == iTaskStackSize )
    {
        iTaskStackSize = XOSTASKSIZEDEFAULT;
    }
    
    if(TSK_PRIO_LOWEST > iTaskPri || TSK_PRIO_HIGHEST < iTaskPri)
    {
        return XERROR;
    }
    if( XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    
    XOS_UNUSED(pTaskName);  
    
    *idTask = (t_XOSTASKID) _beginthread(fTaskFun, iTaskStackSize, pPar);
    if(-1 == (XS32)( *idTask) )
    {
        return XERROR;
    }
    if(0 == SetThreadPriority( (HANDLE)(*idTask),(XS32)(iTaskPri-2)))
    {
        return XERROR;
    }
    
    
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
    
    if(0 == TerminateThread(idtask, 0))
        return XERROR;
    else
    {
        ret = CloseHandle(idtask);
        if(0 != ret)
        {
            return XERROR;
        }
    }
    return XSUCC;
}


#if 0
/************************************************************************
函数名:    CM_Newkey
功能:新建一个键
参数:
Input:     N/A    
Return:     成功    -    指向一个键值的指针
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
Return:     成功    -    XSUCC
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
    if(XERROR == (XS32)SuspendThread( GetCurrentThread()))
    {
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
函数名:    XOS_Sleep
功能：任务睡眠一段时间
输入：ms    -   时间长度(million second)
输出：N/A
返回：XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_Sleep(XU32 ms)
{
    Sleep(ms);
    return XSUCC;
    
}


#if 0
/************************************************************************
函数名:    CM_GetFilePath
功能：  得到文件路径
输入：  filename - 文件名字
buffer   - 装路径名的缓冲区
len      - 装路径名的缓冲区长度
输出：N/A
返回：成功 － 实际需要的缓冲区长度
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 CM_GetFilePath(XCHAR *filename, XCHAR *buffer, XU32 len)
{
    XU32 temp;
    
    temp = GetFullPathName(filename, len, buffer, XNULL);
    
    if(0 == temp)
    {
        return XERROR;
    }     
    
    return temp;
}
#endif


/************************************************************************
函数名: CM_GetLocalIP
功能: 获取本机 IP地址
输入: 无
输出: 无
返回: 本地 IP 地址
说明: 
************************************************************************/
XS32 XOS_GetLocalIP( t_LOCALIPLIST* pLocalIPList  )
{
    XCHAR szLocalIP[32]   = {0};
    struct hostent* pHost = XNULL;
    WORD wVersionRequested;
    WSADATA socket_data;
    struct in_addr inAddr;
    XU32 i = 0;
    XCHAR **ptr = XNULL;
    
    if ( XNULL == pLocalIPList )
    {
        return XERROR;
    }
    
    XOS_MemSet( pLocalIPList, 0x00, sizeof(t_LOCALIPLIST) );
    
    wVersionRequested = MAKEWORD(1, 1);    
    if( 0 != WSAStartup(wVersionRequested, &socket_data))
    {
        return XERROR;
    }
    
    if ( 0 != gethostname( szLocalIP, sizeof( szLocalIP ) )  )
    {
        if( 0 != WSACleanup())
        {
            return XERROR;
        }
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
            XOS_MemSet( &inAddr, 0x00, sizeof( inAddr ) );
            XOS_MemMove( &inAddr, *ptr, 4 );
            pLocalIPList->localIP[pLocalIPList->nIPNum++] = inAddr.s_addr;
        }
    }
    
    if( 0 != WSACleanup())
    {
        return XERROR;
    }
    
    return XSUCC;
}


/**************************************************************
函数名: XOS_Reset
功能: 重启操作系统
输入: type---重启类型，暂未定义
输出: 无
返回: 成功:XSUCC         失败:XERROR
说明: win2000及以上操作系统
***************************************************************/
XPUBLIC XS32 XOS_Reset(XU32 type)
{
    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 

    /* 打开一个进程的访问令牌 */
    /*得到本进程的句柄*/ 
    if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
    {
        return XERROR; 
    }

    // 修改进程的权限 
    LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1; // one privilege to set 赋给本进程特权 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
    
    //通知Windows NT修改本进程的权利 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
    if (GetLastError() != ERROR_SUCCESS) //失败 
    {
        return XERROR; 
    }
    //重启WINDOWS ,强行重启option: EWX_FORCE 
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) 
    {
        return XERROR; 
    }   
        
    return XSUCC;
}


/*************************************************************
函数名: XOS_SetSysTime
功能: 设置操作系统时间
输入: timex---时间，公元1970.1.1至今的秒数、毫秒等
输出: 无
返回: 成功:XSUCC         失败:XERROR
说明: 
**************************************************************/
XS32 XOS_SetSysTime( t_XOSTB *timex )
{
    time_t ltime;    
    struct tm *today;
    SYSTEMTIME st;

    if(NULL == timex)
    {
        return XERROR;
    }

    ltime = timex->time;

    today = localtime( &ltime );
    if(NULL == today)
    {
        return XERROR;
    }

    st.wYear = 1900 + today->tm_year; 
    st.wMonth = 1 + today->tm_mon; 
    st.wDayOfWeek = today->tm_wday; 
    st.wDay = today->tm_mday; 
    st.wHour = today->tm_hour; 
    st.wMinute = today->tm_min; 
    st.wSecond = today->tm_sec; 
    st.wMilliseconds = 0; 

    if(SetLocalTime(&st))
    {
        return XSUCC;
    }
    else
    {
        return XERROR;

    }
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
    *rate  = 10;
    return XSUCC;
}

#ifdef XOS_ARCH_64
XS32 XOS_GetMemInfo(XU64* kbAlloc, XU64* kbTotal)
{
    *kbAlloc = 20000;
    *kbTotal = 50000;
    return XERROR;
}

#else
XS32 XOS_GetMemInfo(XS32* kbAlloc, XS32* kbTotal)
{
    *kbAlloc = 20000;
    *kbTotal = 50000;
    return XERROR;
}
#endif

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
    XCHAR *szErrDes = NULL;

    if(NULL == errmsg || buflen == 0)
    {
        return NULL;
    }

    switch(errnum)
    {
        case 10022: szErrDes = "Invalid argument (not bind)";           break;
        case 10035: szErrDes = "Operation would block";                 break;
        case 10036: szErrDes = "Operation now in progress";             break;
        case 10037: szErrDes = "Operation already in progress";         break;
        case 10038: szErrDes = "Socket operation on non-socket";        break;
        case 10039: szErrDes = "Destination address required";          break;
        case 10048: szErrDes = "Address already in use";                break;
        case 10049: szErrDes = "Can't assign requested address";        break;
        case 10050: szErrDes = "Network is down";                       break;
        case 10051: szErrDes = "Network is unreachable";                break;
        case 10052: szErrDes = "Net dropped connection or reset";       break;
        case 10053: szErrDes = "Software caused connection abort";      break;
        case 10054: szErrDes = "Connection reset by peer";              break;
        case 10055: szErrDes = "No buffer space available";             break;
        case 10056: szErrDes = "Socket is already connected";           break;
        case 10057: szErrDes = "Socket is not connected";               break;
        case 10058: szErrDes = "Can't send after socket shutdown";      break;
        case 10059: szErrDes = "Too many references, can't splice";     break;
        case 10060: szErrDes = "Connection timed out";                  break;
        case 10061: szErrDes = "Connection refused";                    break;
        case 10062: szErrDes = "Too many levels of symbolic links";     break;
        case 10063: szErrDes = "File name too long";                    break;
        case 10064: szErrDes = "Host is down";                          break;
        case 10065: szErrDes = "No Route to Host";                      break;
        case 10066: szErrDes = "Directory not empty";                   break;
        case 10067: szErrDes = "Too many processes";                    break;
        case 10068: szErrDes = "Too many users";                        break;
        default   : szErrDes = NULL;                                    break;
    }

    if (NULL == szErrDes)
    {
        ret = strerror_s(errmsg,buflen,errnum);
        if (ret)
        {         
            szErrDes = XOS_StrNcpy(errmsg,"Unkonw errno!",buflen-1);
            buflen[buflen-1] = '\0';  
        }
        else
        {
            szErrDes = errmsg;
        }
    }
    else
    {
        buflen[buflen-1] = '\0';           
        XOS_StrNcpy(errmsg,szErrDes,buflen-1);
    }

    return szErrDes;
#else
    XCHAR *szErrDes = NULL;
    
    switch(errnum)
    {
    
        /*------------------------system errno---------------------------*/
        case  0: szErrDes = "No error";                              break;
        case  1: szErrDes = "Operation not permitted";               break;
        case  2: szErrDes = "No such file or directory";             break;
        case  3: szErrDes = "No such process";                       break;
        case  4: szErrDes = "Interrupted function call";             break;
        case  5: szErrDes = "Input/output error";                    break;
        case  6: szErrDes = "Invalid Handle";                        break;
        case  7: szErrDes = "Arg list too long";                     break;
        case  8: szErrDes = "Exec format error";                     break;
        case  9: szErrDes = "Bad file descriptor";                   break;
        case 10: szErrDes = "No child processes";                    break;
        case 11: szErrDes = "Resource temporarily unavailable";      break;
        case 12: szErrDes = "Not enough space";                      break;
        case 13: szErrDes = "Permission denied";                     break;
        case 14: szErrDes = "Bad address";                           break;
        case 16: szErrDes = "Resource device";                       break;
        case 17: szErrDes = "File exists";                           break;
        case 18: szErrDes = "Improper link";                         break;
        case 19: szErrDes = "No such device";                        break;
        case 20: szErrDes = "Not a directory";                       break;
        case 21: szErrDes = "Is a directory";                        break;
        case 22: szErrDes = "Invalid argument";                      break;
        case 23: szErrDes = "Too many open files in system";         break;
        case 24: szErrDes = "Too many open files";                   break;
        case 25: szErrDes = "Inappropriate I/O control operation";   break;
        case 27: szErrDes = "File too large";                        break;
        case 28: szErrDes = "No space left on device";               break;
        case 29: szErrDes = "Invalid seek";                          break;
        case 30: szErrDes = "Read-only file system";                 break;
        case 31: szErrDes = "Too many links";                        break;
        case 32: szErrDes = "Broken pipe";                           break;
        case 33: szErrDes = "Domain error";                          break;
        case 34: szErrDes = "Result too large";                      break;
        case 36: szErrDes = "Resource deadlock avoided";             break;
        case 38: szErrDes = "Filename too long";                     break;
        case 39: szErrDes = "No locks available";                    break;
        case 40: szErrDes = "Function not implemented";              break;
        case 41: szErrDes = "Directory not empty";                   break;
        case 42: szErrDes = "Illegal byte sequence";                 break;
        case 80: szErrDes = "String was truncated";                  break;
        
        /*------------------------Windows socket errno---------------------------*/
        case /*87*/ WSA_INVALID_PARAMETER : szErrDes = "Invalid Parameter"; break;
        case /*995*/WSA_OPERATION_ABORTED : szErrDes = "Overlapped operation aborted"; break;
        case /*996*/   WSA_IO_INCOMPLETE  : szErrDes = "Overlapped I/O Incomplete"; break;
        case /*997*/   WSA_IO_PENDING     : szErrDes = "Overlapped operations will complete later"; break;
        case /*10004*/ WSAEINTR           : szErrDes = "Interrupted function call";  break;  
        case /*10013*/ WSAEACCES          : szErrDes = "Permission denied";  break;  
        case /*10014*/ WSAEFAULT          : szErrDes = "Bad address";  break;  
        case /*10022*/ WSAEINVAL          : szErrDes = "Invalid argument";  break;  
        case /*10024*/ WSAEMFILE          : szErrDes = "Too many open files";  break;  
        case /*10035*/ WSAEWOULDBLOCK     : szErrDes = "Resource temporarily unavailable";  break;  
        case /*10036*/ WSAEINPROGRESS     : szErrDes = "Operation now in progress";  break;  
        case /*10037*/ WSAEALREADY        : szErrDes = "Operation already in progress";  break;  
        case /*10038*/ WSAENOTSOCK        : szErrDes = "Socket operation on non-socket";  break;  
        case /*10039*/ WSAEDESTADDRREQ    : szErrDes = "Destination address required";  break;  
        case /*10040*/ WSAEMSGSIZE        : szErrDes = "Message too long";  break;  
        case /*10041*/ WSAEPROTOTYPE      : szErrDes = "Protocol wrong type for socket";  break;  
        case /*10042*/ WSAENOPROTOOPT     : szErrDes = "Bad protocol option";  break;  
        case /*10043*/ WSAEPROTONOSUPPORT : szErrDes = "Protocol not supported";  break;  
        case /*10044*/ WSAESOCKTNOSUPPORT : szErrDes = "Socket type not supported";  break;  
        case /*10045*/ WSAEOPNOTSUPP      : szErrDes = "Operation not supported";  break;  
        case /*10046*/ WSAEPFNOSUPPORT    : szErrDes = "Protocol family not supported";  break;  
        case /*10047*/ WSAEAFNOSUPPORT    : szErrDes = "Address family not supported by protocol family";  break;  
        case /*10048*/ WSAEADDRINUSE      : szErrDes = "Address already in use";  break;  
        case /*10049*/ WSAEADDRNOTAVAIL   : szErrDes = "Cannot assign requested address";  break;  
        case /*10050*/ WSAENETDOWN        : szErrDes = "Network is down";  break;  
        case /*10051*/ WSAENETUNREACH     : szErrDes = "Network is unreachable";  break;  
        case /*10052*/ WSAENETRESET       : szErrDes = "Network dropped connection on reset";  break;  
        case /*10053*/ WSAECONNABORTED    : szErrDes = "Software caused connection abort";  break;  
        case /*10054*/ WSAECONNRESET      : szErrDes = "Connection reset by peer";  break;  
        case /*10055*/ WSAENOBUFS         : szErrDes = "No buffer space available";  break;  
        case /*10056*/ WSAEISCONN         : szErrDes = "Socket is already connected";  break;  
        case /*10057*/ WSAENOTCONN        : szErrDes = "Socket is not connected";  break;  
        case /*10058*/ WSAESHUTDOWN       : szErrDes = "Cannot send after socket shutdown";  break;  
        case /*10060*/ WSAETIMEDOUT       : szErrDes = "Connection timed out";  break;  
        case /*10061*/ WSAECONNREFUSED    : szErrDes = "Connection refused";  break;  
        case /*10064*/ WSAEHOSTDOWN       : szErrDes = "Host is down";  break;  
        case /*10065*/ WSAEHOSTUNREACH    : szErrDes = "No route to host";  break;  
        case /*10067*/ WSAEPROCLIM        : szErrDes = "Too many processes";  break;  
        case /*10068*/ WSAEUSERS          : szErrDes = "User quota exceeded";  break;  
        case /*10069*/ WSAEDQUOT          : szErrDes = "Disk quota exceeded";  break;
        case /*10070*/ WSAESTALE          : szErrDes = "Stale file handle reference"; break;
        case /*10071*/ WSAEREMOTE         : szErrDes = "Item is remote"; break;
        case /*10091*/ WSASYSNOTREADY     : szErrDes = "Network subsystem is unavailable";  break;  
        case /*10092*/ WSAVERNOTSUPPORTED : szErrDes = "Winsock.dll version out of range";  break;  
        case /*10093*/ WSANOTINITIALISED  : szErrDes = "Successful WSAStartup not yet performed";  break;  
        case /*10101*/ WSAEDISCON         : szErrDes = "Graceful shutdown in progress";  break;  
        case /*10102*/ WSAENOMORE         : szErrDes = "No more results";  break;  
        case /*10103*/ WSAECANCELLED      : szErrDes = "Call has been canceled";  break;  
        case /*10104*/ WSAEINVALIDPROCTABLE   : szErrDes = "Procedure call table is invalid";  break;  
        case /*10105*/ WSAEINVALIDPROVIDER    : szErrDes = "Service provider is invalid";  break;  
        case /*10106*/ WSAEPROVIDERFAILEDINIT : szErrDes = "Service provider failed to initialize";  break;  
        case /*10107*/ WSASYSCALLFAILURE      : szErrDes = "System call failure";  break;  
        case /*10108*/ WSASERVICE_NOT_FOUND   : szErrDes = "Service not found";  break;  
        case /*10109*/ WSATYPE_NOT_FOUND  : szErrDes = "Class type not found";  break;  
        case /*11001*/ WSAHOST_NOT_FOUND  : szErrDes = "Host not found";  break;  
        case /*11002*/ WSATRY_AGAIN       : szErrDes = "Nonauthoritative host not found";  break;  
        case /*11003*/ WSANO_RECOVERY     : szErrDes = "This is a nonrecoverable error";  break;  
        case /*11004*/ WSANO_DATA         : szErrDes = "No data record";  break;  
        
        default: szErrDes = "unknown errno!!"; break;
    }

    return szErrDes;
#endif
}


#ifdef __cplusplus
}
#endif /*__cplusplus */

