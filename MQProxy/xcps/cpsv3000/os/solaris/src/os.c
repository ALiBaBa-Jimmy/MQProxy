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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include <netdb.h>

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define statfilesize 256


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
    if(BLANK_ULONG == (*inetAddress = XOS_NtoHl( inet_addr((XCONST XU8 *)pString ))))
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
˵����
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

}
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
    
    ret = sem_wait((sem_t *)semaphore);
    while(XSUCC != ret  && errno == EINTR )
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
˵����  //sem_destroy(s),��semҪ��û��ʹ�õ�����²���destroy,
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
    mutex = (pthread_mutex_t *)XNULLP;
    
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
        
        /*�ڴ����߳�֮ǰ�����߳����Զ���ĵ��Ȳ��Ժ����ȼ���*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) /*�ɹ�����0*/
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
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*�ɹ�����0*/
        
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
        
        /*�ڴ����߳�֮ǰ�����߳����Զ���ĵ��Ȳ��Ժ����ȼ���*/
        if(XSUCC != pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) /*�ɹ�����0*/
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
        /*pthread_setschedparam (*idTask, SCHED_FIFO, &priority);*/ /*�ɹ�����0*/
        
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
Input:     key    -    ָ��һ����ֵ��ָ��
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
������: CM_GetLocalIP
����: ��ȡ���� IP��ַ
����: ��
���:
pLocalIPList.nIPNum; ��ȡ��IP �ĸ���, 
pLocalIPList.localIP[MAX_LOCALIP_NUM]; 32 λ IP .
eg. pLocalIPList.localIP[0] �ǵ�һ������IP��ַ.
����: ���� IP ��ַ
˵��: 
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
������: XOS_Reset
����: ��������ϵͳ
����: type---�������ͣ���δ����
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: �ݲ�ʵ��
***************************************************************/
XPUBLIC XS32 XOS_Reset(XU32 type)
{
    printf("not support solaris.\r\n");
    return XSUCC;
}


/**************************************************************
������: XOS_SetSysTime
����: ���ò���ϵͳʱ��
����: timex---ʱ��
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: �ݲ�ʵ��
***************************************************************/
XS32 XOS_SetSysTime( t_XOSTB *timex )
{
    printf("not support solaris.\r\n");
    return XSUCC;
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
    printf("not support solaris.\r\n");
    return XSUCC;
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


