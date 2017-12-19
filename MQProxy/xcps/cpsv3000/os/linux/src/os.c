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
**  date:   2006.5.15
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wentao         2006.5.15              create  
*************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>

#include <netdb.h>
#include <sys/wait.h>
#include <xostrace.h>
#include <xoscfg.h>
#include "xosencap.h"
/*#include "cmshell.h"*/

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define statfilesize 256

const XCHAR g_signal_name[32][10] = {
    "",
    "SIGHUP",    //      1  
    "SIGINT",    //     2   /
    "SIGQUIT",   //  3   /
    "SIGILL",     // 4   /
    "SIGTRAP",   //  5   /
    "SIGABRT",   //  6   = SIGIOT
    "SIGBUS",    //  7   /
    "SIGFPE",    //  8   /
    "SIGKILL",   //  9   /
    "SIGUSR1",   //  10  /
    "SIGSEGV",   //  11  /
    "SIGUSR2",   //  12  /
    "SIGPIPE",   //  13  /
    "SIGALRM",   //  14  /
    "SIGTERM",   //  15  /
    "SIGSTKFLT", //  16  /
    "SIGCHLD",   //  17  /
    "SIGCONT",   //  18  /
    "SIGSTOP",   //  19  /
    "SIGTSTP",   //  20  /
    "SIGTTIN",   //  21  /
    "SIGTTOU",   //  22  /
    "SIGURG",    //  23  /
    "SIGXCPU",   //  24  /
    "SIGXFSZ",   //  25  /
    "SIGVTALRM", //  26  /
    "SIGPROF",   //  27  /
    "SIGWINCH",  //  28  /
    "SIGIO",     //  29  /
    "SIGPWR",    //  30  /
    "SIGUNUSED"};  //  31   
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
    struct in_addr temp;
    if(0 == inet_aton((const char *)pString , &temp))
    {
        return XERROR;
    }
    *inetAddress = XOS_NtoHl(temp.s_addr);
    return XSUCC;
}


/************************************************************************
������:    XOS_StrNtoIp
���ܣ��������ַת������������Ƶ����֣�����������ַ�ַ���������'\0'������
��ֻ��ǰlen���ַ����д���;���磺"168.0.2.258"
���룺
pString  -   Ҫת���������ַ��
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
    struct in_addr temp;
    
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
        if(0 == inet_aton((const char *)pString , &temp))
        {
            free(str);
            return XERROR;
        }
        else
        {
            if(INADDR_NONE == (*inetAddress = XOS_NtoHl( temp.s_addr)))
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
    }
#endif
    
    if(0 == inet_aton((const char *)str , &temp))
    {
        free(str);
        return XERROR;
    }
    else
    {
        if(INADDR_NONE == (*inetAddress = XOS_NtoHl( temp.s_addr)))
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
    char buf[XOS_INET_ADDR_LEN]= {0};
    struct in_addr addr;

    addr.s_addr = XOS_HtoNl(inetAddress);

    XOS_Inet_ntoa(addr, buf); /*����һ����0���ַ���*/

    XOS_MemCpy(pString, buf, XOS_StrLen(buf)+1);

    return XSUCC;
    
}


/************************************************************************
������:    XOS_IpNtoStr
���ܣ�����������Ƶ�����ת���������ַ��д��pString�ֻд��ǰlen���ַ�����'\0'ֹ��
���룺
inetAddress  -   Ҫת����������������֣�ע�ⲻ�ܳ�����Χ����
pString - �������淵�صĵ��ʮ����IP
len    -    �����ַ�ַ���pString�ĳ���
�����pString - ���ʮ����IP
����ֵ��XSUCC    -    �ɹ�
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
������:    XOS_StrNtoXU32
���ܣ����ַ���ת���ɳ���������ֻת��ǰlen���ַ���;
���룺
string    -    Ҫת�����ַ���
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
    
    if(string == XNULL || value == XNULL || len == 0 || (len+1) > MAXNUMSTRLEN )
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
˵��: windows ����ultoa��LINUX����snprintf(��size���ַ�����Ϊ'\0')
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
������:    XOS_GetSysTime
���ܣ���ȡϵͳʱ��(��ȷ��ǧ��֮һ��)
���룺timex -   ��������ʱ��Ľṹ��
�����timex -   ��ǰϵͳʱ�䣨��. ΢�룩
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ص��ǹ�Ԫ1970.1.1�����������΢����
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex )
{
    struct timeb temp;
    ftime(&temp);
    timex->time = temp.time;
    timex->millitm = temp.millitm;
    timex->timezone = temp.timezone;
    
    return XSUCC;
}

/************************************************************************
������:    XOS_GetSysRunTime
���ܣ���ȡϵͳ����ʱ��(��ȷ����)
����\
�����pSecs -   ��ǰϵͳ����ʱ�䣨�룩
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ص���ϵͳ�������������
************************************************************************/
XS32 XOS_GetSysRunTime(XU64 *pSecs)
{
    FILE *fp = NULL;
    double upsecs;
    char buf[1024] = {0};
    char *b = NULL;

    *pSecs = 0;
    
    fp = fopen ("/proc/uptime", "r");
    if (fp != NULL)
    {
        b = fgets (buf, sizeof(buf), fp);
        fclose (fp);
        if (b == buf)
        {
            sscanf (buf, "%lf", &upsecs);
            *pSecs = (XU64)upsecs;
            
            return XSUCC;
        }
    }

    return XERROR;
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
    int cpunum = 0;
    
    time_t t;    
    t_XOSTD rettime;
    
    char text[statfilesize];
    
    FILE *fp;
    int total,running,sleeping,stopped,zombie,
        totalm,usedm,freem,buffersm,totals,useds,frees,cacheds;
    float us,sy,ni,id,wa,hi,si;
    
    if( XNULL == systeminfo )
    {
        return XERROR;
    }
    
    /***** ��ȡϵͳʱ��*****/
    
    if((time_t)(-1) == time(&t))
    {
        return XERROR;
    }
    /*memcpy(&(systeminfo->localtime), localtime(&t), sizeof(SYS_TD_t));
    systeminfo->localtime.dt_year += 1900;
    systeminfo->localtime.dt_mon += 1;*/
    memcpy(&rettime, localtime(&t), sizeof(t_XOSTD));
    systeminfo->localtime.dt_year = rettime.dt_year + 1900;
    systeminfo->localtime.dt_mon = rettime.dt_mon +1;
    systeminfo->localtime.dt_mday = rettime.dt_mday;
    systeminfo->localtime.dt_hour = rettime.dt_hour;
    systeminfo->localtime.dt_min = rettime.dt_min;
    systeminfo->localtime.dt_sec = rettime.dt_sec;
    systeminfo->localtime.dt_wday = rettime.dt_wday;/*���ڼ�*/
    systeminfo->localtime.dt_yday = rettime.dt_yday;
    systeminfo->localtime.dt_isdst = rettime.dt_isdst;
    
    /***** ��ȡCPU����*****/
    if (XNULLP != (fp = fopen("/proc/stat", "r")))
    {
        while (fgets(text, 255, fp))
        {
            if(strstr(text, "cpu"))
            {
                cpunum++;
            }
        }
        fclose(fp);
    }
    else
        return XERROR;
    systeminfo->dwNumberOfProcessors = cpunum - 1;
    
    /***** ��ȡTOP������Ϣ*****/
    /*for(i = 0;i <= 3 ;i++)*/
    {
        if(XNULLP == (fp = popen("top", "r")))
            return XERROR;
        
            /************************* top ��ʽ************************
            top - 11:00:23 up  1:29,  5 users,  load average: 0.04, 0.07, 0.03
            Tasks:  85 total,   1 running,  84 sleeping,   0 stopped,   0 zombie
            Cpu(s):  0.4% us,  0.4% sy,  0.0% ni, 98.9% id,  0.2% wa,  0.0% hi,  0.1% si
            Mem:   4150264k total,   243320k used,  3906944k free,    21780k buffers
            Swap:  2031608k total,        0k used,  2031608k free,   137340k cached
        **************************************************************/
        
        while(fgetc(fp)!='T');
        fscanf(fp,"%*s %d %*s %d %*s %d %*s %d %*s %d %*s %*s %f %*s %*s %f %*s %*s %f %*s %*s %f %*s %*s %f %*s %*s %f %*s %*s %f %*s %*s %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s",
            &total,&running,&sleeping,&stopped,&zombie,
            &us,&sy,&ni,&id,&wa,&hi,&si,
            &totalm,&usedm,&freem,&buffersm,
            &totals,&useds,&frees,&cacheds);
        
            /*printf(" %d %d %d %d %d \n %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f \n %d %d %d %d \n %d %d %d %d \n\n",
            total,running,sleeping,stopped,zombie,
            us,sy,ni,id,wa,hi,si,
            totalm,usedm,freem,buffersm,
        totals,useds,frees,cacheds);*/
        
        if(XERROR == pclose(fp))
        {
            return XERROR;
        }
        /*sleep(1);*/
    }
    /*systeminfo->dwNumberOfProcessors = 0;*/
    systeminfo->dwProcessorLoad = (XU32)us;
    systeminfo->dwMemoryLoad = usedm*100/totalm;
    systeminfo->dwTotalPhys = totalm;
    systeminfo->dwAvailPhys = freem;
    systeminfo->dwTotalPageFile = totals;
    systeminfo->dwAvailPageFile = frees;
    systeminfo->dwtotalthreads    = total;
    systeminfo->dwthreadRun  = running;
    systeminfo->dwthreadssleep = sleeping;
    
    return XSUCC;
    
}
#endif


#if 0
/************************************************************************
������  : timeSetEvent_Timeout
����    : ��ʱ����ʱ����Ϣ����
����    : 
���    : none
����    : 
˵��    :
************************************************************************/
void Linux_sendmsg(void)
{
    XU32 retval;
    //XU32 i;
    struct timespec tv;
    //t_XOSCOMMHEAD temp; 
    
    while(1)
    {
        tv.tv_sec = 0;
        tv.tv_nsec = 19100000;
        retval = pselect(1, XNULLP, XNULLP, XNULLP, &tv, XNULLP);
        if(!retval)
        {/*to do
         for(i=0; i<MaxTIDNum; i++)
         {
         if(TIDList[i].taskname != XNULLP)
         {
         temp.datasrc.FID  = i;
         temp.datasrc.PID  = 0;
         temp.datadest.FID = FID_TIME;
         temp.datadest.PID = 0;
         temp.msgID        = 23;
         temp.subID        = 43;
         temp.prio         = 0;
         temp.length       = 0;
         temp.message      = XNULLP;
         
           XOS_MsgSend(&temp);
           }
        }    */
        }
    }
    //return  XSUCC;
}


XS32 Timer_clckheart(XVOID)
{
    pthread_t idTask;
    
    pthread_attr_t attr;
    if(XNULL != pthread_attr_init(&attr))
    {
        return XERROR;
    }
    if(XNULL != pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM))
    {
        return XERROR;
    }        
    if(XNULL != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {
        return XERROR;
    }   
    
    if(XNULL != pthread_create(&idTask, &attr, Linux_sendmsg, XNULL))
    {
        return XERROR;
    }
    
    return XSUCC;
}
#endif


#if 0
/************************************************************************
������:    XOS_SemCreate
����:�����ź���
����:
Input:
maxNum    -    �����ź������ֵ(linux��û�д˲���)
initNum    -    �����ź����ĳ�ֵ
name      -      �ź�������(linux��û�д˲���)
output:    semaphore   - �ź���id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����  //sem_init(s, 0, c),������ֻ��ʼ��semaphore��ֵ��
���Ҷ���ͬһ��semaphoreֻ�ܳ�ʼ��һ��,������δ֪;
���ź���֧�ֵ����ֵ��2147483647
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum, XU32 maxNum,  const XCHAR *name)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    //XS32 ret;
    //XOS_UNUSED(initNum);
    XOS_UNUSED(maxNum);
    XOS_UNUSED(name);
#if 0   /*system V*/
    *semaphore = semget(IPC_PRIVATE, 1, IPC_CREAT|0660);
    /*  "1"Ϊ�źż��ϵ���Ŀ����������ֻ�õ���1 ��*/
    if(XERROR == *semaphore)
    {
        return XERROR;
    }
    ret = semctl(*semaphore, 0, SETVAL, initNum);    
    /*�����ھʹ򿪣�initNum��ʾ�����ź����ĳ�ʼֵ*/
    if(XERROR == ret)
    {
        return XERROR;
    }
#endif
    if(XSUCC == sem_init((sem_t *)semaphore,0,(XU32)initNum))
    {
        return XSUCC;
    }
    else
        return XERROR;
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
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    //XS32 ret;
    //XOS_UNUSED(initNum);
#if 0   /*system V*/
    *semaphore = semget(IPC_PRIVATE, 1, IPC_CREAT|0660);
    /*  "1"Ϊ�źż��ϵ���Ŀ����������ֻ�õ���1 ��*/
    if(XERROR == *semaphore)
    {
        return XERROR;
    }
    ret = semctl(*semaphore, 0, SETVAL, initNum);    
    /*�����ھʹ򿪣�initNum��ʾ�����ź����ĳ�ʼֵ*/
    if(XERROR == ret)
    {
        return XERROR;
    }
#endif
    if(XSUCC == sem_init((sem_t *)semaphore,0,(XU32)initNum))
    {
        return XSUCC;
    }
    else
        return XERROR;
}


/************************************************************************
������:    XOS_SemGet
����:����ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����  //sem_wait(s)�����ź�����ֵ����-1 ������
��ֻ�����ź�����ֵ����0 ʱ���ܽ��У�����ȴ���
���ź���֧�ֵ����ֵ��2147483647
************************************************************************/
XS32 XOS_SemGet(t_XOSSEMID *semaphore)
{
    XS32 ret;
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
#if 0   /*system V*/
    struct sembuf sb;
    sb.sem_num = 0;    /*��������źű��룬"0" ��ʾ��1 ��*/
    sb.sem_op  = -1;    /*�ź�����1*/
    sb.sem_flg = SEM_UNDO;
    
    if(XERROR == semop(*semaphore, &sb, 1) )    /* "1" ��ʾsb�ṹ����Ŀ*/
    {
        return XERROR;
    }
#endif
    ret = sem_wait((sem_t *)semaphore);
    while( XSUCC != ret && errno == EINTR )
    {
        XOS_Sleep(1);
        ret = sem_wait((sem_t *)semaphore);
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
˵����  //sem_timedwait(s)�����ź�����ֵ����-1 ������
��ֻ�����ź�����ֵ����0 ʱ���ܽ��У�����ȴ���
���ź���֧�ֵ����ֵ��2147483647
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
������:    XOS_SemPut
����:�ͷ��ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����  //sem_post(s)�����ź�������+1 ����;
************************************************************************/
XS32 XOS_SemPut(t_XOSSEMID *semaphore)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
#if 0   /*system V*/
    struct sembuf sb;
    
    sb.sem_num = 0;
    sb.sem_op  = 1;    /*�ź�����1*/
    sb.sem_flg = SEM_UNDO;
    
    if(XERROR == semop(*semaphore, &sb, 1) )
    {
        return XERROR;
    }
#endif
    if(XSUCC != sem_post((sem_t *)semaphore))
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
˵����  //sem_destroy(s)
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore)
{
    
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
#if 0   /*system V*/
    if(XERROR == semctl(*semaphore, 0, IPC_RMID, 0))    
        /*ǰ���"0" �����źŶ��еĵ�1 ���ź�*/
    {
        return XERROR;
    }
#endif
    if(XSUCC != sem_destroy((sem_t *)semaphore))
    {
        return XERROR;
    }
    
    
    return XSUCC;
}


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
    if(XNULLP == mutex)
    {
        return XERROR;
    }
    /**mutex = XNULL; */
    *mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    
    return XSUCC;
}


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
        
        /*�ڴ����߳�֮ǰ�����߳����Զ���ĵ��Ȳ��Ժ����ȼ���*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
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
        
        /*��̬���޸��������е��̵߳������ȼ��͵��Ȳ���*/    
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*Ҫsuper-userȨ��*/
        
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
    pthread_attr_t attr;
    struct sched_param priority;
    t_CpuBindCfg *pCpuBind = pOwnPar;
    XS32 ret = 0;
    
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
        
        /*�ڴ����߳�֮ǰ�����߳����Զ���ĵ��Ȳ��Ժ����ȼ���*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
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

    
        if (NULL != pCpuBind)
        {
            ret = XOS_SetThreadAffinity_t(*idTask,pCpuBind->Cpus,pCpuBind->cpunum);
            if(XSUCC != ret)
            {
                printf("XOS_SetThreadAffinity_t ret fail! ret:%d\n",ret);
            }
        }
        
        /*��̬���޸��������е��̵߳������ȼ��͵��Ȳ���*/    
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*Ҫsuper-userȨ��*/
        
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
    
    ret = pthread_cancel(idtask);
    if(XNULL != ret)
    {
        return XERROR;
    }
    
    return XSUCC;
}


#if 0
/************************************************************************
������:    CM_SockOpen 
����:��һ��socket
����:
Input:
type    -    ��ʽ(1:TCP 2:UDP 3:RAW)
iProtocol    -    Э���(���type=3,iProtocolΪ����,��������)
sock    -    socket���
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockSetBlk
����:����һ��socketΪ�������������ʽ
����:
Input:
sock    -    socket���
mode    -    0:����,1:������
Output:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockIPtoul
����  : ��ipv4��ַת����Ϊulong��
����  :
Input:     ipaddr    -    ipv4��ַ(��:"166.241.5.11")
Return:
network order address
XERROR    -    ʧ��
˵��  ��
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
������:    CM_SockUltoIPv4
����:��ulong��ַת����Ϊipv4��
����:
Input:     addr    -    network order address
Return:
ipv4��ַ(��:"166.241.5.11")
XNULL    -    ʧ��
˵����
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
������:    CM_SockBind
����  : �󶨱��ض˿�
����  :
Input:
sock    -    socket���
ipaddr    -    ipv4��ַ(��:"166.241.5.11")
port    -    �󶨵Ķ˿�
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockListen(XS32 sock);
����:����һ��socket
����:
Input:     sock    -    socket���              
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockAccept
����:����һ��socket
����:
Input :     sock    -    socket���              
Output:
rsock    -    ���ӵ�socket���
pAddr    -    ���ظ��ⲿ�û��Ĳ���
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockConnect
����:�󶨱��ض˿�
����:
Input:
sock    -    socket���
ipaddr    -    ipv4��ַ(��:"166.241.5.11")
port    -    �󶨵Ķ˿�
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockRecv
����:����TCP����
����:
Input:
sock    -    socket���
buf    -    ������
size    -    ��������С
Return:
XS32    -    �������ݳ���
XERROR    -    ʧ��
˵����
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
������:    CM_SockSend
����:����TCP����
����:
Input:
sock    -    socket���
buf    -    ������
size    -    ��������С
Return:
XS32    -    �������ݳ���
XERROR    -    ʧ��
˵����
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
������:    CM_SockUdpRecv
����:����UDP���ݰ�
����:
Input:
sock    -    socket���
buf    -    ������
size    -    ���ݴ�С
ipaddr    -    ipv4��ַ(��:"166.241.5.11")
port    -    �󶨵Ķ˿�
Return:
XS32    -    �������ݳ���
XERROR    -    ʧ��
˵����
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
������:    CM_SockUdpSend
����:����UDP���ݰ�
����:
Input:
sock    -    socket���
buf    -    ������
size    -    ���ݴ�С
Output:
ipaddr    -    ipv4��ַ(��:"166.241.5.11")
port    -    �󶨵Ķ˿�
Return:
XS32    -    �������ݳ���
XERROR    -    ʧ��                       
˵����
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
������:    CM_SockClose
����:�ر�һ��socket���
����:
Input:     sock    -    socket���
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_SockIscnt
����:�ж�һ��socket�Ƿ�������״̬
����:
Input :     sock    -    socket���
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������:    CM_Newkey
����:�½�һ����
����:
Input:     N/A    
Return:
�ɹ�    -    ָ��һ����ֵ��ָ��
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
Input:
key    -    ָ��һ����ֵ��ָ��
pointer    -    ָ��Ҫ�󶨵����ݽṹ��ָ��
Return:
�ɹ�    -    XSUCC
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
    while(1)
    {
        pause();
    }
    return XSUCC;
}


/************************************************************************
������:    XOS_Sleep
���ܣ�����˯��һ��ʱ��
���룺ms    -   ʱ�䳤��(million second)
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
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
������: XOS_GetLocalIP
����: ��ȡ���� IP��ַ
����: ��
���:
pLocalIPList.nIPNum; ��ȡ��IP �ĸ���, 
pLocalIPList.localIP[MAX_LOCALIP_NUM]; 32 λ IP .
eg. pLocalIPList.localIP[0] �ǵ�һ������IP��ַ.
����: ���� IP ��ַ
˵��: 
************************************************************************/
XS32 XOS_GetLocalIP( t_LOCALIPLIST* pLocalIPList)
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
    if ( pHost != XNULL && pHost->h_addrtype == AF_INET)
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
������: XOS_Reset
����: ��������ϵͳ
����: type---�������ͣ���δ����
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
***************************************************************/
XPUBLIC XS32 XOS_Reset(XU32 type)
{
    int ret = 1;
    //Ĭ�ϳ���������root��
#if 1
    sync();//flush file system buffer
    if((ret = reboot(RB_AUTOBOOT)))
    {
        printf("reboot succ.\r\n");
    }
    else
    {
        return XERROR;
        printf("reboot fail. reason=0x%x\r\n",ret);
    }
#endif
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
    //���ַ���,system()�Լ�clock_settime()
    //date  -s "2011-02-28 17:34:30"  system(settimestr)
#if 1
    struct timespec tp;
    tp.tv_sec = timex->time;//ע��:1900��1970������,linuxΪ1970.01.01
    if(0==clock_settime(CLOCK_REALTIME,&tp))
    {
        printf("set succ.\r\n");
    }
    else
    {
        return XERROR;
        printf("set failed.\r\n");
    }
#endif
    return XSUCC;
}


/**************************************************************
������: XOS_GetCpuRate
����: ��ȡcpuռ���ʣ�ȡ���2s��ռ����
����: rate---����CPUռ��������, rate% ,����rate
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
***************************************************************/
#define GET_CPU_RATE "#/bin/sh\n\
cpulog_1=$(cat /proc/stat | grep 'cpu' | awk '{print $2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$8}')\n\
sysidle_1=$(echo $cpulog_1 | awk '{print $4}')\n\
total_1=$(echo $cpulog_1 | awk '{print $1+$2+$3+$4+$5+$6+$7}')\n\
sleep 2\n\
cpulog_2=$(cat /proc/stat | grep 'cpu' | awk '{print $2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \"$8}')\n\
sysidle_2=$(echo $cpulog_2 | awk '{print $4}')\n\
total_2=$(echo $cpulog_2 | awk '{print $1+$2+$3+$4+$5+$6+$7}')\n\
sys_idle=`expr $sysidle_2 - $sysidle_1`\n\
total=`expr $total_2 - $total_1`\n\
sys_usage=`expr $sys_idle/$total*100 |bc -l`\n\
sys_rate=`expr 100-$sys_usage | bc -l`\n\
disp_sysrate=`expr \"scale=3;$sys_rate/1\" | bc`\n\
echo $disp_sysrate > get_cpu"
XS32 XOS_GetCpuRate(int* rate)
{
    XCHAR cpu_name[32] = {0};
    XU32 user_cpu = 0;
    XU32 nice_cpu = 0;
    XU32 system_cpu = 0;
    XU32 idle_cpu = 0;
    double total = 0.0;
    double rate_val = 0.0;
    FILE * file = NULL;

    if(NULL != (file = fopen("/proc/stat", "r"))) {
        fscanf(file, "%s %d %d %d %d", cpu_name, &user_cpu, &nice_cpu, &system_cpu, &idle_cpu);
        total = user_cpu + nice_cpu + system_cpu + idle_cpu;
        rate_val = ((user_cpu + nice_cpu + system_cpu) / total) * 100;
        *rate = (int)rate_val;
        
        fclose(file);
    }
    
    return XSUCC;
}

/**************************************************************
������: XOS_GetMemInfo
����: ��ȡ�ڴ�ռ����
����: rate---�����ڴ�ռ��������, rate% ,����rate
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
***************************************************************/
XU64 XOS_GetValueFromFile(const char* fileName)
{
    XU64 size = 0;
    FILE* fp = fopen(fileName, "r");
    if (fp == XNULLP)
    {
        return XERROR;
    }
    char text[statfilesize] = "";
    fgets(text, statfilesize - 1, fp);
    fclose(fp);

    XOS_StrToLongNum(text, &size);
    return size;
}

#ifdef XOS_ARCH_64

XS32 XOS_GetMemInfo(XU64* kbAlloc, XU64* kbTotal)
{
    system("cat /proc/meminfo | awk '/MemTotal/{print $2}' > tmp_GetMemInfo");
    XU64 total = XOS_GetValueFromFile("tmp_GetMemInfo");
    system("cat /proc/meminfo | awk '/MemFree/{print $2}' > tmp_GetMemInfo");
    XU64 freeRate = XOS_GetValueFromFile("tmp_GetMemInfo");
    system("rm -rf tmp_GetMemInfo");
    if (total == XERROR || freeRate == XERROR)
    {
        return XERROR;
    }
    *kbTotal = total;
    *kbAlloc = (total - freeRate);
    return XSUCC;
}
#else

XS32 XOS_GetMemInfo(XS32* kbAlloc, XS32* kbTotal)
{
    system("cat /proc/meminfo | awk '/MemTotal/{print $2}' > tmp_GetMemInfo");
    XS32 total = (XS32)XOS_GetValueFromFile("tmp_GetMemInfo");
    system("cat /proc/meminfo | awk '/MemFree/{print $2}' > tmp_GetMemInfo");
    XS32 freeRate = (XS32)XOS_GetValueFromFile("tmp_GetMemInfo");
    system("rm -rf tmp_GetMemInfo");
    if (total == XERROR || freeRate == XERROR)
    {
        return XERROR;
    }
    *kbTotal = total;
    *kbAlloc = (total - freeRate);
    return XSUCC;
}

#endif
/**************************************************************
������: XOS_signal
����: ��װϵͳ�źŴ�����
����: 
���: ��
����: 
˵��: 
***************************************************************/
typedef void (*XOS_Sighandler_t)(int);   /*�źŴ�����ָ��*/

XOS_Sighandler_t XOS_signal(int signo, XOS_Sighandler_t func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
    if (signo != SIGALRM)
        act.sa_flags |= SA_RESTART;
#endif
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);

}

/**************************************************************
������: XOS_RegisterSignal
����: ע�����ϵͳ���źţ��Բ���ϵͳ���͵��źź��Բ���¼��־
����: 
���: ��
����: 
˵��: 
***************************************************************/
XVOID XOS_RegisterAllSignal()
{
    
    /*��alarm���õļ�ʱ����ʱʱ����setitimer���õ�ʵʱ���ʱ�䵽��ʱ�������ź� ���źű���ʱ��ģ��ʹ��*/
    //signal(SIGALRM, signal_handle_proc); /*���źű���ʱ��ģ��ʹ��*/
    
    //XOS_signal(SIGINT, SIG_IGN);    /*CTRL+C��Ĭ����ֹ*/

    XOS_signal(SIGIO, SIG_IGN);     /*�첽I/O�¼�,����IO����ģ�͵����ݽ��ջ��ͣ�Ĭ����ֹ,���ʹ���첽IOģ��ʱ�����ܽ��в�׽*/
    
    XOS_signal(SIGPIPE, SIG_IGN);   /*д���ܵ�ʱ����������ֹ;��д���׽���ʱ���Զ��ѹرգ�Ĭ����ֹ*/
    
    /*decrements both when the process executes and when the system is executing on behalf of the process.  
     Coupled with  ITIMER_VIRTUAL,  this timer is usually used to profile the time spent by the application 
     in user and kernel space.  SIGPROF is delivered upon expiration.
    */
    //XOS_signal(SIGPROF, SIG_IGN);   /*��Ĭ����ֹ*/

    //signal(SIGUNUSED, signal_handle_proc);
    
     /*��setitimer���õ�������ʱ�䵽��ʱ�������źţ�Ĭ����ֹ*/
    //XOS_signal(SIGVTALRM, SIG_IGN);
    
    //signal(SIGABRT, signal_handle_proc);
    

    //signal(SIGBUS, signal_handle_proc);

    //signal(SIGCLD, signal_handle_proc);
    //signal(SIGCHLD, signal_handle_proc);
    
    //signal(SIGCONT, signal_handle_proc);  

    //signal(SIGFPE, signal_handle_proc);
    
    //signal(SIGHUP, signal_handle_proc);
    
    
    //signal(SIGILL, signal_handle_proc);
    //signal(SIGTRAP, signal_handle_proc);
    
    //signal(SIGIOT, signal_handle_proc);

    //signal(SIGKILL, signal_handle_proc);
    
    //signal(SIGPWR, signal_handle_proc);
    //signal(SIGQUIT, signal_handle_proc);

    //signal(SIGSEGV, signal_handle_proc);
    //signal(SIGSTOP, signal_handle_proc);
    //signal(SIGSYS, signal_handle_proc);
    //signal(SIGTERM, signal_handle_proc);
    //signal(SIGTRAP, signal_handle_proc);
    //signal(SIGTSTP, signal_handle_proc);
    //signal(SIGTTIN, signal_handle_proc);
    //signal(SIGTTOU, signal_handle_proc);
    //signal(SIGURG, signal_handle_proc);
    //signal(SIGUSR1, signal_handle_proc);
    //signal(SIGUSR2, signal_handle_proc);
    
    //signal(SIGWINCH, signal_handle_proc);
    //signal(SIGXCPU, signal_handle_proc);
    //signal(SIGXFSZ, signal_handle_proc);
    
    //signal(SIGSTKFLT, signal_handle_proc);
}



/************************************************************************
������:  XOS_System
����:    ͨ��pipeִ��������,���سɹ����
����:    pCommand  -������
���:
����:    �ɹ������ļ�������,���򷵻�NULL
˵��:    ��API��������ļ��̳У�ϵͳ����system�����ļ��̳е�����
************************************************************************/
XS32 XOS_System(const XCHAR * pCommand) 
{ 
    pid_t pid; 
    XS32 status; 
    XS32 i = 0;

    if(pCommand == NULL) 
    { 
        return (1); 
    } 

    if((pid = fork())<0) 
    { 
        status = -1; 
    } 
    else if(pid == 0) 
    { 
        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */  
        for (i = 3; i < sysconf(_SC_OPEN_MAX); i++) 
        {
            close(i);  
        }
        
        execl("/bin/sh", "sh", "-c", pCommand, (char *)0); 
        _exit(127); 
    } 
    else 
    { 
        /*��������SIGCHLD��SIG_IGN����ʱ����ȡ����status����ֵ��XOS_System�޷���������*/
        while(waitpid(pid, &status, 0) < 0) 
        { 
            if(errno != EINTR) 
            { 
                status = -1; 
                break; 
            } 
        } 
    } 

    return status; 
} 


/************************************************************************
������:XOS_Popen
����:  ͨ��pipeִ��������,���سɹ���񣬲���ȡ������ݡ�
����:  pCommand-������
       pMode   -'r' or 'w'
���:
����: �ɹ������ļ�������,���򷵻�NULL
˵��: ��API��������ļ��̳У�ϵͳ����popen�����ļ��̳е�����
************************************************************************/
FILE* XOS_Popen(const XCHAR *pCommand, const XCHAR *pMode, pid_t *pid)  
{  
    XS32 i;
    XS32 parent_end = 0;
    XS32 child_end = 0;
    XS32 child_std_end = 0;
    XS32 pipe_fds[2] = {0};
    pid_t child_pid = 0;
    
    if(NULL == pCommand || NULL == pMode || pid == NULL)
    {
        return (NULL);
    }

    *pid = 0;

    if (pipe(pipe_fds) < 0)
    {
        return(NULL);   
    }

    /* only allow "r" or "w" */  
    if (pMode[0] == 'r' && pMode[1] == '\0')
    {
        parent_end = pipe_fds[0];
        child_end = pipe_fds[1];
        child_std_end = STDOUT_FILENO;
    }
    else if (pMode[0] == 'w' && pMode[1] == '\0')
    {        
        parent_end = pipe_fds[1];
        child_end = pipe_fds[0];
        child_std_end = STDIN_FILENO;
    }
    else
    {  
        return(NULL);  
    }   
    
    child_pid = fork();
    if (child_pid < 0)
    {
        close(child_end);
        close (parent_end);
        return NULL;
    }
    else if (child_pid == 0)  /* child */  
    {
        close (parent_end);
        if (child_end != child_std_end)
        {
            dup2 (child_end, child_std_end);
            close (child_end);
        }

        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */  
        for (i = 3; i < sysconf(_SC_OPEN_MAX); i++) 
        {
            close(i);
        }
  
        execl("/bin/sh", "sh", "-c", pCommand, (char *) 0);  
        
        _exit(127);  
    } 
    close(child_end);
    
    *pid = child_pid;

    return fdopen(parent_end, pMode);  
}  

/************************************************************************
������:XOS_ExeCmdByPopen_Ex
����:  ͨ��popenִ��������,���سɹ���񣬲���ȡ������ݡ�
����:  pCmd-������
       pBuf-���淵�ؽ��������
       len-����pBuf�Ŀ��ó���
       ms - popen�󣬻�ȡ������֮ǰ����Ҫdelay��ʱ�䣬��λ����,������2000ms
���:
����: �ɹ���������ִ�еĽ��,���򷵻�XERROR
˵��: ��API���ᵼ���ļ��̳�
************************************************************************/
XS32 XOS_ExeCmdByPopen_Ex(XCHAR *pCmd, XCHAR *pBuf, XU32 len, XU32 ms)
{
    XS32 count = 2000; /*����ȡ2000��*/
    FILE *fp = NULL;
    XCHAR *pTmp = NULL;
    XU32 nLen = 0;
    XS32 status = 0;
    pid_t pid;


    if(NULL == pCmd || NULL == pBuf || 0 == len)
    {
        return XERROR;
    }
   
    fp = XOS_Popen(pCmd, "r",&pid);//ͨ��һ���ű���ת���ڽű��йر����е��ļ���������exec fd<&-        
    if(NULL == fp)
    {
        perror("XOS_ExeCmdByPopen:popen() failed");
        return XERROR;
    }

    if (ms > 2000)
    {
        ms = 2000;
    }
    XOS_Sleep(ms);

    pBuf[0] = '\0';
    pTmp = pBuf;
    while(count > 0 && NULL != fgets(pTmp, len, fp))
    {
        nLen = strlen(pTmp);
        if(0 == nLen)/*����������*/
        {
            break;
        }
        pTmp += nLen;
        len -= nLen;
        count--;
    }

    /* ���� close��waitpid��� �൱��pclose. 
    -- Ϊ�˽��XOS_Poen��pclose����������ʬ���̵����� */
    close(fileno(fp));

    while(waitpid(pid, &status, 0) < 0) 
    { 
        if(errno != EINTR) 
        { 
            status = -1; 
            break; 
        } 
    } 

    return status;    
}

/************************************************************************
������:XOS_ExeCmdByPopen
����:  ͨ��popenִ��������,���سɹ���񣬲���ȡ������ݡ�
����:  pCmd-������
       pBuf-���淵�ؽ��������
       len-����pBuf�Ŀ��ó���
���:
����: �ɹ���������ִ�н��,���򷵻�XERROR
˵��: ��API���ᵼ���ļ��̳�
************************************************************************/
XS32 XOS_ExeCmdByPopen(XCHAR *pCmd, XCHAR *pBuf, XU32 len)
{
    XS32 count = 2000; /*����ȡ2000��*/
    FILE *fp = NULL;
    XCHAR szRead[1024] = {0};
    XCHAR *pTmp = NULL;
    XU32 nLen = 0;
    XS32 status = 0;
    pid_t pid;


    if(NULL == pCmd || 0 == len)
    {
        return XERROR;
    }
   
    fp = XOS_Popen(pCmd, "r",&pid);//ͨ��һ���ű���ת���ڽű��йر����е��ļ���������exec fd<&-        
    if(NULL == fp)
    {
        perror("XOS_ExeCmdByPopen:popen() failed");
        return XERROR;
    }

    XOS_Sleep(100);
    
    if(NULL != pBuf)
    {
        pBuf[0] = '\0';
        pTmp = pBuf;
        while(count > 0 && NULL != fgets(pTmp, len, fp))
        {
            nLen = strlen(pTmp);
            if(0 == nLen)/*����������*/
            {
                break;
            }
            pTmp += nLen;
            len -= nLen;
            count--;
        }
    }
    else
    {   
        /*��ֹ�ű���ִ�е�ʱ�������Ҫ����д����������߸����̺����˳��ˣ����¹ܵ��ƻ�*/
        szRead[0] = '\0';
        while(count > 0 && NULL != fgets(szRead, sizeof(szRead)-1, fp))
        {
            count--;
        }
    }
    
    /* ���� close��waitpid��� �൱��pclose. 
    -- Ϊ�˽��XOS_Popen��pclose����������ʬ���̵����� */
    close(fileno(fp));

    while(waitpid(pid, &status, 0) < 0) 
    { 
        if(errno != EINTR) 
        { 
            status = -1; 
            break; 
        } 
    } 

    return status;    
}

/************************************************************************
������:XOS_ExeCmdBySystem
����:  ͨ��systemִ��������,ֻ���سɹ���񣬲��ܻ�ȡ������ݡ�
����:  pCmd-������
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: ��API���ᵼ���ļ��̳�,ϵͳ����system�ᵼ���ļ��̳еķ���
************************************************************************/
XS8 XOS_ExeCmdBySystem(XCHAR *pCmd)
{
    pid_t pid;
    XU32 i = 0;
    XS32 rst = 0;

    if(NULL == pCmd)
    {
        return XERROR;
    }

    if((pid = fork()) < 0)
    {
        perror("XOS_ExeCmdBySystem:fork() failed\r\n");
    }
    else if (pid == 0)
    {
        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */  
        for (i = 3; i < sysconf(_SC_OPEN_MAX); i++) 
        {
            close(i);  
        }   
        
        rst = XOS_System(pCmd); /*��ϵͳ���õ���ִ�лᵼ���ļ��̳�*/
        if(127 == rst || -1 == rst)
        {
            perror("XOS_ExeCmdBySystem:system() failed\r\n");
        }
        
        exit(0);
    }
    return XSUCC;
    
}

/************************************************************************
������:XOS_ExeCmd
����:  ִ��������,ֻ���سɹ����
����:  pCmd-������
       pBuf-���淵�ؽ�������ݣ�ΪNULLʱ���򲻹��������еķ�������
       len-����pBuf�Ŀ��ó���
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: ��API���ᵼ���ļ��̳У����pBufΪ�գ��򲻷���ִ�н��
************************************************************************/
XPUBLIC XS32 XOS_ExeCmd(XCHAR *pCmd, XCHAR *pBuf, XU32 len)
{
    if(NULL == pCmd)
    {
        return XERROR;
    }

    if(pBuf)
    {
        return XOS_ExeCmdByPopen(pCmd, pBuf, len);
    }
    else
    {
        return XOS_System(pCmd);
    }

    return XSUCC;
}
/************************************************************************
������: XOS_ExeCmdRetVal
����: ִ��shell�����ȡ��ִ������ķ���ֵ
����:
���:
����: ���ر�ִ������ķ���ֵ
˵��: -1: forkʧ��
      -2: systemִ������ʧ��
      ����: ��ִ������ķ���ֵ��Ϊ�Ǹ��� 0-127
************************************************************************/
XPUBLIC XS32 XOS_ExeCmdRetVal(XCHAR *pCmd)
{
    XS32 status = 0;

    if(NULL == pCmd)
    {
        return XERROR;
    }

    status = XOS_System(pCmd);

    if (-1 == status)
    {
        perror("XOS_System return -1!fork faild!\n");
        return -1;
    }
    else
    {
        //printf("dbg:system exit status value = [0x%x]!\n", status);

        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else
        {
            printf("run cmd %s exit err! exit status = [0x%x], errstr:%m\n",pCmd, status);
            return -2;
        }
    }
}

/************************************************************************
������: XOS_GetCpuCoreNum
���ܣ���ȡcpu����
���룺
�����
���أ����ص�ǰϵͳcpu���� 
************************************************************************/
XS32 XOS_GetCpuCoreNum(XVOID)
{
    return sysconf(_SC_NPROCESSORS_CONF);
}

/************************************************************************
������: XOS_SetThreadAffinity_t
���ܣ������̼߳�cpu��, ��ָ�����̵߳��߳�id
���룺thread : �߳�pid
      pCpuId : cpu�ŵ�����, cpu��0��ʼ
      usCpuNum ����Ԫ�ظ��� 
�����
���أ�XSUCC - �ɹ� XERROR  -  ʧ��
˵������ָ���̵߳�ָ��cpu
************************************************************************/
XS32 XOS_SetThreadAffinity_t(pthread_t thread,const XU16 *pCpuId, XS32 CpuNum)
{
    cpu_set_t tMask;
    XS32 i = 0;
    XS32 ret = 0;
    XS32 MaxCpuNum = XOS_GetCpuCoreNum();

    if (0 == CpuNum)
        return 0;

    if(CpuNum >= MaxCpuNum)
    {    
        printf("param err! uCpuNum too big!");
        return XERROR;
    }

    if (!pCpuId)
    {    
        printf("param err! pCpuId is null!");
        return -100;
    }

    CPU_ZERO(&tMask);
    for (i = 0; i < CpuNum; i++)
    {
        if (pCpuId[i] > MaxCpuNum-1)
        {
            printf("param err! CpuId:%d\n",pCpuId[i]);
            return -200;
        }
        CPU_SET(pCpuId[i], &tMask);
    }

    ret = pthread_setaffinity_np(thread, sizeof(tMask), &tMask);
    if(ret)
    {
        perror("sched_setaffinity fail!");  
    }

    return ret;
}


/************************************************************************
������: XOS_SetThreadAffinity
���ܣ������̵߳�cpu��
���룺pCpuId : cpu�ŵ�����, cpu��0��ʼ
      usCpuNum ����Ԫ�ظ��� 
�����
���أ�XSUCC - �ɹ� XERROR  -  ʧ��
˵�����ɸ����̵߳���, �������õ�����λ�þ������ǰ������̻߳��ǵ�ǰ�߳�:
1������������߳��е��ã���ʹ���̺߳�֮�󴴽��������̰߳���Ч
2������������߳��е��ã�ֻ�Ե�ǰ�߳���Ч
************************************************************************/
XS32 XOS_SetThreadAffinity(const XU16 *pCpuId, XS32 CpuNum)
{
    cpu_set_t tMask;
    XS32 i = 0;
    XS32 ret = 0;
    XS32 MaxCpuNum = XOS_GetCpuCoreNum();

    if (0 == CpuNum)
        return 0;

    if(CpuNum >= MaxCpuNum)
    {    
        printf("param err! uCpuNum too big!");
        return XERROR;
    }

    if (!pCpuId)
    {    
        printf("param err! pCpuId is null!");
        return -100;
    }

    CPU_ZERO(&tMask);
    for (i = 0; i < CpuNum; i++)
    {
        if (pCpuId[i] > MaxCpuNum-1)
        {
            printf("param err! CpuId:%d\n",pCpuId[i]);
            return -200;
        }
        CPU_SET(pCpuId[i], &tMask);
    }

    //ret = pthread_setaffinity_np(pthread_self(),sizeof(tMask), &tMask);
    ret = sched_setaffinity(syscall(SYS_gettid), sizeof(tMask), &tMask);
    if(ret)
    {
        perror("sched_setaffinity fail!");  
    }

    return ret;
}


/************************************************************************
 ������: XOS_GetLogicPid
 ����: ��ȡ��ǰ���̵��߼����̺�
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetLogicPid(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TCN_PROC_ID");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);    
}
/************************************************************************
 ������: XOS_GetNeId
 ����: ��ȡ��ǰ���̵���Ԫid
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetNeId(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TCN_NE_ID");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);
}
/************************************************************************
 ������: XOS_GetWorkspaceId
 ����: ��ȡ��ǰ���̵Ĺ�����id
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetWorkspaceId(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TCN_WORKSPACE_ID");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);    
}

/************************************************************************
 ������: XOS_GetNeType
 ����: ��ȡ��ǰ���̵���Ԫ����
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetNeTypeId(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TCN_NE_TYPE");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);    
}
/************************************************************************
 ������: XOS_GetNeTypeStr
 ����: ��ȡ��ǰ���̵���Ԫ����
 ����:
 ���:
 ����: �ɹ������ַ���, ���򷵻�NULL
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS8* XOS_GetNeTypeStr(XVOID)
{
    XS32 netypeid = XOS_GetNeTypeId();
    static XS8 *strNeType[] = 
    {
        "",
        "emme", /* 1 */
        "tcf",  /* 2 */
        "tmg",  /* 3 */
        "spgw", /* 4 */
        "ehss", /* 5 */
        "smc",  /* 6 */
        "tas",  /* 7 */
        "agt",  /* 8 */
        "tcn",  /* 9 */
        "oms",  /* 10 */
    };

    if (netypeid >= sizeof(strNeType)/sizeof(strNeType[0]) || netypeid <= 0)
    {
        return NULL;
    }
    
    return strNeType[netypeid];
}
/************************************************************************
 ������: XOS_GetProcTypeStr
 ����: ��ȡ��ǰ���̵Ľ�������
 ����:
 ���:
 ����: �ɹ������ַ���, ���򷵻�NULL
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS8* XOS_GetProcTypeStr(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TCN_PROC_TYPE");
    if (!str || !strlen(str))
    {
        return NULL;
    }

    return str;
}

/************************************************************************
 ������: XOS_GetTsHighSlot
 ����: ��ȡTS�ĸ߲�λ��
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetTsHighSlot(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TS_HIGH_SLOT");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);    
}

/************************************************************************
 ������: XOS_GetTsLowSlot
 ����: ��ȡTS�ĵͲ�λ��
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetTsLowSlot(XVOID)
{
    XCHAR *str = NULL;

    str = getenv("TS_LOW_SLOT");
    if (!str || !strlen(str))
    {
        return -1;
    }

    return strtol(str, NULL, 10);    
}

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
                szErrDes = "buflen is not enough";   /* strerror_r���ش�����Ϊbuf���Ȳ��� */
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



