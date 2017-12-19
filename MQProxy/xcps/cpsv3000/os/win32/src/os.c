/***************************************************************
**
** Xinwei Telecom Technology co., ltd. ShenZhen R&D center
** 
** Core Network Department  platform team  
**
** filename: os.c
**
** description:  ����ƽ̨�ڲ��Ľӿڷ�װ 
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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/ 

#include "xosencap.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define INVALID_SEM_ID   (-1)

PROCNTQSI NtQuerySystemInformation;

/*-------------------------------------------------------------------------
                �ڲ����ݽṹ����
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/

/************************************************************************
������:    XOS_StrtoIp
���ܣ��������ַת������������Ƶ�����
���룺
pString  -   Ҫת���������ַ
inetAddress - �������淵�ص���������Ƶ�����
�����inetAddress - ��������Ƶ�����
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_StrNtoIp
���ܣ��������ַת������������Ƶ����֣�����������ַ�ַ���������'\0'������
��ֻ��ǰlen���ַ����д���;���磺"168.0.2.258"
���룺
pString  -   Ҫת���������ַ
inetAddress - �������淵�ص���������Ƶ�����
len    -    �����ַ�ַ���pString�ĳ���
�����inetAddress - ��������Ƶ�����
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����������ϵͳ�´��ڲ��죺����"202.0.2."��Windows��Linux�±���Ϊ�ǲ�����
��ʽ��IP��ַ������Solaris��vxWork��ȴ�ܳɹ�ת�������ԣ�Ϊ��
������ʹ���ϵ����⣬�������"168.0.2.258"�ĸ�ʽ����IP��
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
������:    XOS_IptoStr
���ܣ�����������Ƶ�����ת���������ַ
���룺
inetAddress  -   Ҫת�����������������
pString - �������淵�صĵ��ʮ����IP
�����pString - ���ʮ����IP
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����pStringΪ����16���ַ����飬��XCHAR pString[16]
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
������:    XOS_IpNtoStr
���ܣ�����������Ƶ�����ת���������ַ��д��pString�ֻд��ǰlen���ַ�����'\0'ֹ��
���룺
inetAddress  -   Ҫת����������������֣�ע�ⲻ�ܳ�����Χ��
pString - �������淵�صĵ��ʮ����IP
len    -    �����ַ�ַ���pString�ĳ���
�����pString - ���ʮ����IP
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����һ��pStringΪ����16���ַ�����ռ䣬��XCHAR pString[16];
ע��VXWORKS�µ��õ�ϵͳ����Ϊ inet_ntoa_b ����Ϊinet_ntoa ��
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
������:    XOS_StrNtoXU32
���ܣ����ַ���ת���ɳ���������ֻת��ǰlen���ַ���;
���룺
string    -    Ҫת�����ַ���,�磺"  1234567"
len    -    �ַ���string�ĳ���
�����value    -    ת���õ��ĳ�������
���أ� 
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����ת��ʱ������ǰ��Ŀո��ַ���ֱ����������ʱ�ſ�ʼת����
���ٴ����������ֻ��ַ���������'\0'ʱ���ء�
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
������:    XOS_XU32NtoStr
���ܣ����޷��ų�������ת�����ַ�����size������д��������'\0'ֹ������ֻд��size����
���룺
value    -    Ҫת�����޷�������
str    -    ��������ת������ַ���
size    -    �ַ����ĳ���
�����str    -    ת���õ����ַ���
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵��: windows ����ultoa��LINUX����snprintf
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
������:    XOS_GetSysTime
���ܣ���ȡϵͳʱ��(��ȷ��ǧ��֮һ��)
���룺timex -   ��������ʱ��Ľṹ��
�����timex -   ��ǰϵͳʱ�䣨��,���뼰 Ŀǰʱ����UTC����ʱ�䣬��λΪ���ӣ�
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ص��ǹ�Ԫ1970.1.1����������������
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
������:    XOS_GetSysInfo
���ܣ���ȡϵͳ��Ϣ
���룺N/A
�����systeminfo    -    ָ��ϵͳ��Ϣ�ṹ���ָ��
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
    /***** ��ȡCPU��Ϣ(CPU������ռ����) *****/
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
    
    systeminfo->dwNumberOfProcessors = SysBaseInfo.bKeNumberProcessors; /*CPU����*/
    
    /*printf("\nCPU Usage (press any key to exit):  ");*/
    
    /*while(!kbhit())*/
    for(i= 0;i <= 1;i++)    /*ȡ����CPU��Ϣ,���Ϊ1s */
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
    
    systeminfo->dwProcessorLoad = (UINT)dbIdleTime; /*CPUռ����*/    
    
    /***** ��ȡϵͳʱ��*****/
    ltime = time(XNULLP);
    SystemTime = localtime(&ltime);
    systeminfo->localtime.dt_year = SystemTime->tm_year;
    systeminfo->localtime.dt_mon = SystemTime->tm_mon;
    systeminfo->localtime.dt_mday = SystemTime->tm_mday;
    systeminfo->localtime.dt_hour = SystemTime->tm_hour;
    systeminfo->localtime.dt_min = SystemTime->tm_min;
    systeminfo->localtime.dt_sec = SystemTime->tm_sec;
    systeminfo->localtime.dt_wday = SystemTime->tm_wday;  /*���ڼ�*/
    systeminfo->localtime.dt_yday = SystemTime->tm_yday;
    systeminfo->localtime.dt_isdst = SystemTime->tm_isdst;
    
    /***** ��ȡ�ڴ���Ϣ*****/
    GlobalMemoryStatus (&MemStat);
    systeminfo->dwMemoryLoad = MemStat.dwMemoryLoad;
    systeminfo->dwTotalPhys = (XU32)MemStat.dwTotalPhys/DIV;
    systeminfo->dwAvailPhys = (XU32)MemStat.dwAvailPhys/DIV;
    systeminfo->dwTotalPageFile = (XU32)MemStat.dwTotalPageFile/DIV;
    systeminfo->dwAvailPageFile = (XU32)MemStat.dwAvailPageFile/DIV;
    
    /***** ��ȡ������Ϣ*****/
    if( INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)))
        return XERROR;
    if(TRUE != Process32First(hSnapshot, &syspe))
        return XERROR;
    /*printf("");*/
    i = 0;  /* �����洢�������*/
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
������:    XOS_SemCreate
����:�����ź���
����:
Input:
maxNum    -    �����ź������ֵ
initNum    -    �����ź����ĳ�ֵ
name      -      �ź�������
output:    semaphore   - �ź���id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_SemCreate
����:�����ź���(����)
����:
Input:
initNum    -    �����ź����ĳ�ֵ
name      -      �ź�������
output:    semaphore   - �ź���id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_SemGet
����:����ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_SemGetExt
����:����ź���
����:
Input:    semaphore    -    �ź�����id
          timeout   ��ʱʱ�䣬��λ �� 
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����  //WaitForSingleObject(s)�����ź�����ֵ����-1 ������
��ֻ�����ź�����ֵ����0 ʱ���ܽ��У�����ȴ���
���ź���֧�ֵ����ֵ��2147483647
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
        /*��ʱ���쳣*/
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
������:    XOS_SemPut
����:�ͷ��ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_SemDelete
����:ɾ���ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexCreate
���ܣ��������ĳ�ʼ��
���룺
mutex    -    ������ID
type    -    ����������
(OS_LOCK_MUTEX��OS_LOCK_CRITSEC)
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexLock
���ܣ���û�����
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexUnlock
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexDelete
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexCreate
���ܣ��������ĳ�ʼ��
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_MutexLock
���ܣ���û�����
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵������������ͬһ���̵Ĳ�ͬ�̼߳�ʹ��;
����ͬһ�߳��ڿ��Զ��lock���������XOS_MutexUnlock�ɶԳ��֣�
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
������:    XOS_MutexUnlock
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����ͬ��
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
������:    XOS_MutexDelete
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_TaskCreate
����  : ��������
����  :
Input:
pTaskName        -    ������
iTaskPri        -    �������ȼ�
iTaskStackSize    -    ��ջ��С
fTaskFun        -    ��������
pPar            -    ����
Output:    idTask            -    �����ʶ
Return:
XSUCC        -    �ɹ�
XERROR        -    ʧ��
˵��  ��
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
������: XOS_TaskCreate_Ex
����  : ��������
Input:
        pTaskName        -    ������
        iTaskPri        -    �������ȼ�
        iTaskStackSize    -    ��ջ��С
        fTaskFun        -    ��������
        pPar            -    ����      
        pOwnPar :       -    ���������ڲ�ʹ�õģ�Ŀǰ��������cpu����Ϣ
Output:    
        idTask            -    �����ʶ
Return:
        XSUCC        -    �ɹ�
        XERROR        -    ʧ��
˵��  ����XOS_TaskCreate���˸�����pOwnPar
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
������ : XOS_TaskDel
����   : ɾ������(������)
����:
Input :    idTask        -    �����ʶ
Output:    N/A
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��

  ˵����
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
������:    CM_Newkey
����:�½�һ����
����:
Input:     N/A    
Return:     �ɹ�    -    ָ��һ����ֵ��ָ��
ʧ��    -    XNULLP
˵����
************************************************************************/
/*XSTATIC XVOID *CM_Newkey(XVOID)*/
XPUBLIC XVOID *CM_Newkey(XVOID)
{
    return XNULLP;
}


/************************************************************************
������:    CM_Setkey
����:���߳����ݺ�һ��������һ��
����:
Input:     key    -    ָ��һ����ֵ��ָ��
pointer    -    ָ��Ҫ�󶨵����ݽṹ��ָ��
Return:     �ɹ�    -    XSUCC
ʧ��    -    XERROR
˵����
************************************************************************/
XPUBLIC XU32 CM_Setkey(XVOID *key, XCONST XVOID *pointer)
{
    XOS_UNUSED(key);
    XOS_UNUSED(pointer);
    
    return XSUCC;
}


/************************************************************************
������:    CM_Getkey
����:ȡ�ü��󶨵��߳�����
����:
Input:     key    -    ָ��һ����ֵ��ָ��
Return:
�ɹ�    -    ���ؼ��󶨵�����(��0)    
ʧ��    -    XNULLP
˵����
************************************************************************/
XPUBLIC XVOID *CM_Getkey(XVOID *key)
{
    XOS_UNUSED(key);
    return XNULLP;
}
#endif


/************************************************************************
������:    XOS_SusPend
���ܣ���������
���룺N/A
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    XOS_Sleep
���ܣ�����˯��һ��ʱ��
���룺ms    -   ʱ�䳤��(million second)
�����N/A
���أ�XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_Sleep(XU32 ms)
{
    Sleep(ms);
    return XSUCC;
    
}


#if 0
/************************************************************************
������:    CM_GetFilePath
���ܣ�  �õ��ļ�·��
���룺  filename - �ļ�����
buffer   - װ·�����Ļ�����
len      - װ·�����Ļ���������
�����N/A
���أ��ɹ� �� ʵ����Ҫ�Ļ���������
XOS_ERROR    -    ʧ��
˵����
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
������: CM_GetLocalIP
����: ��ȡ���� IP��ַ
����: ��
���: ��
����: ���� IP ��ַ
˵��: 
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
������: XOS_Reset
����: ��������ϵͳ
����: type---�������ͣ���δ����
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: win2000�����ϲ���ϵͳ
***************************************************************/
XPUBLIC XS32 XOS_Reset(XU32 type)
{
    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 

    /* ��һ�����̵ķ������� */
    /*�õ������̵ľ��*/ 
    if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
    {
        return XERROR; 
    }

    // �޸Ľ��̵�Ȩ�� 
    LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1; // one privilege to set ������������Ȩ 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
    
    //֪ͨWindows NT�޸ı����̵�Ȩ�� 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
    if (GetLastError() != ERROR_SUCCESS) //ʧ�� 
    {
        return XERROR; 
    }
    //����WINDOWS ,ǿ������option: EWX_FORCE 
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) 
    {
        return XERROR; 
    }   
        
    return XSUCC;
}


/*************************************************************
������: XOS_SetSysTime
����: ���ò���ϵͳʱ��
����: timex---ʱ�䣬��Ԫ1970.1.1����������������
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
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
������: XOS_GetCpuRate
����: ��������ϵͳ
����: rate---����CPUռ��������, rate% ,����rate
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
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
������:XOS_Strerror
����:  �������errno��ת��Ϊ��Ӧ�Ĵ�����Ϣ�ַ���
����:  errnum  ������
���:  
����: success���ش����errmsg�ĵ�ַ��fail����"Unknown error"
˵��: ��������ֵ�������ַ����׵�ַ������ֱ��ʹ�ã�ֻ�ܶ�������д
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

