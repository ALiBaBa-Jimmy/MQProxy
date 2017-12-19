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
**   wentao         2006.8.15             create  
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�../../02XOS
-------------------------------------------------------------------------*/
#include "xosencap.h"

#ifdef XOS_VXWORKS
XEXTERN t_XOSTT gMsec;
XU8 ucSpyStartFlag = 0;
//����������С��256��ÿ������spy��Ϣռ80�ֽ�
//XS8 gspyCurrInfo[(256+6)*80];
//XU32 gspyCpuRate = 0;
#define X_FREQ_SEC 5

#include <spyLib.h>
#endif
extern XU32 spyIdleIncTicks;
extern XU32 spyIncTicks;
/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                ��������
-------------------------------------------------------------------------*/
extern void reset();
extern XS32 XOS_LocalTime(XCONST t_XOSTT *times  , t_XOSTD *timetm);
extern void Set_Rtc_Time2(int second,int minute,int hour,int mday,int month,int year,int Weekday);

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
    if(INADDR_NONE == (*inetAddress = XOS_NtoHl( inet_addr(pString ))))
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
    
    if(INADDR_NONE == (*inetAddress = XOS_NtoHl( inet_addr(str ))))
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
    struct in_addr addr;
    
    addr.s_addr = XOS_HtoNl(inetAddress);
    inet_ntoa_b(addr ,pString);
    
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
    XCHAR temp[18] = {0};
    struct in_addr addr;
    
    if(pString == XNULL || len == 0)
    {
        return XERROR;
    }
    
    addr.s_addr = XOS_HtoNl(inetAddress);
    inet_ntoa_b(addr ,temp);
    
    if(len > XOS_StrLen(temp)+1)
    {
        XOS_MemCpy(pString, temp, XOS_StrLen(temp)+1);
    }
    else
    {
        XOS_MemCpy(pString, temp, len);
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
˵�������ص��ǰ�������������
ע����������ȷ�ĳ�ʼʱ�䣬����Ի�ȡ��ǰ����ʵʱ��      
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex )
{
    struct timespec tp; 
    if(-1 == clock_gettime(CLOCK_REALTIME, &tp))
    {
        return XERROR;
    }
    timex->time = tp.tv_sec;
#ifdef XOS_VXWORKS
    timex->time = timex->time + gMsec;
#endif    
    timex->millitm = tp.tv_nsec/1000000;
    timex->timezone = 0;
    
    return XSUCC;
}


/************************************************************************
������:    XOS_GetSysInfo
���ܣ���ȡϵͳ��Ϣ
���룺N/A
�����systeminfo    -    ָ��ϵͳ��Ϣ�ṹ���ָ��
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
***********************************************************************
XS32 XOS_GetSysInfo(t_XOSSYSINFO *systeminfo)
{
  return XSUCC;
}*/


#if 0
/************************************************************************
**************************************************************************
������  : timeSetEvent_Timeout
����    : ��ʱ����ʱ����Ϣ����
����    : 
���    : none
����    : 
˵��    :
**********************************************************************/
void Vxworks_sendmsg(void)
{
    /*return  XSUCC;*/
}


XS32 Timer_clckheart(XVOID)
{
#if 0
    int idTask;
    
    XOS_TaskCreate("Timer", 70, 40960, Vxworks_sendmsg, XNULL, &idTask);
    
    return XSUCC;
#endif
    
#if 0
    t_XOSCOMMHEAD *temp = XNULLP;
    XU8 data[20];
    XU8 i;
    
    for(i = 0; i < 20; i++)
    {
        data[i] = i;
    }
    
    temp = XOS_MsgMemMalloc(FID_1_1, 100);
    
    printf("Timer send message\n");
    temp->datasrc.FID  = 10;
    temp->datasrc.PID  = 0;
    temp->datadest.FID = FID_1_1;
    temp->datadest.PID = 0;
    temp->msgID = 23;
    temp->subID = 43;
    temp->prio = 0;
    
    XOS_MemCpy(temp->message, &data, 20);
    XOS_MsgSend(temp);
    /* ��ʱ����ʱ����ӿ�
       handelΪ��ʱ��Ψһ������붨ʱ������ʱ�Ķ�Ӧ
       tΪ�ش������ṹָ��*/
    
    return XSUCC;
#endif
}
#endif


#if 0
/************************************************************************
������:    XOS_SemCreate
����:�����ź���
����:
Input:
maxNum        -    �����ź������ֵ(linux��û�д˲���)
initNum    -    �����ź����ĳ�ֵ
name       -   �ź�������(linux��û�д˲���)
output:  semaphore  -   �ź���id
Return:
XSUCC        -    �ɹ�
XERROR        -    ʧ��
˵����������ź���ִ��take����ʱ������ź�������0����õ��ź������ź�����1;
����ź���Ϊ0��������������
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum, XU32 maxNum,  const XCHAR *name)
{
    if(XNULLP == semaphore)
    {
        return XERROR;
    }
    XOS_UNUSED(maxNum);
    XOS_UNUSED(name);
    
    *semaphore = semCCreate(SEM_Q_PRIORITY, initNum);
    
    return (NULL == *semaphore? XERROR:XSUCC);
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
    
    *semaphore = semCCreate(SEM_Q_PRIORITY, initNum);
    
    return (NULL == *semaphore? XERROR:XSUCC);
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
    STATUS ret;
    if( (XNULLP == semaphore) || (XNULL== *semaphore) )
    {
        return XERROR;
    }
    
    ret = semTake(*semaphore, intContext() ? NO_WAIT:WAIT_FOREVER);
    
    return ( OK== ret ? XSUCC:XERROR);
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
˵����  //semTake(s)�����ź�����ֵ����-1 ������
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
        if(intContext())/*���жϻ����У���������*/
        {
            return XERROR;
        }
        ret = semTake(*semaphore, timeout*sysClkRateGet());

        return ( OK == ret ? XSUCC : XERROR);
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
    if( (XNULLP == semaphore) || (NULL== *semaphore) )
    {
        return XERROR;
    }

    return (OK == semGive(*semaphore)? XSUCC:XERROR);
}


/************************************************************************
������:    XOS_SemDelete
����:ɾ���ź���
����:
Input:    semaphore    -    �ź�����id
Return:    XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore)
{
    if( (XNULLP == semaphore) || (NULL== *semaphore) )
    {
        return XERROR;
    }

    return (OK == semDelete(*semaphore)? XSUCC:XERROR);
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
    
    *mutex = semMCreate(SEM_Q_PRIORITY);

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
    STATUS ret;
    XBOOL back = XFALSE;
    if( (XNULLP == mutex) || (NULL== *mutex) )
    {
        return XERROR;
    }
    
    back = intContext();
    
    if ( XFALSE != back )
    {
        printf("XERROR: XOS_MutexLock->intContext interrupt context\r\n");
    }
    
    ret = semTake(*mutex, back ? NO_WAIT:WAIT_FOREVER);

    return ( OK== ret ? XSUCC:XERROR);
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
    if( (XNULLP == mutex) || (NULL== *mutex) )
    {
        return XERROR;
    }

    return ( OK == semGive(*mutex)? XSUCC:XERROR);
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
    
    if( (XNULLP == mutex) || (NULL== *mutex) )
    {
        return XERROR;
    }
    
    return (OK == semDelete(*mutex)? XSUCC:XERROR);
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
    if(XNULLP == idTask)
    {
        return XERROR;
    }
    
    if(XNULLP == fTaskFun)
    {
        return XERROR;
    }
    if( XNULLP == pTaskName )
    {
        return XERROR;
    }
    
    if(XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    else
        if(TSK_PRIO_LOWEST < iTaskPri || TSK_PRIO_HIGHEST > iTaskPri)
        {
            return XERROR;
        }
        
        if(0 > iTaskStackSize || XOSTASKSIZEMAX < iTaskStackSize )
        {
            return XERROR;
        }
        if(iTaskStackSize == XNULL)
        {
            iTaskStackSize = XOSTASKSIZEDEFAULT;
        }
        
        *idTask = taskSpawn((char*)pTaskName, iTaskPri, VX_FP_TASK, iTaskStackSize, (FUNCPTR)fTaskFun,
            (int)pPar, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        
        return ( ERROR == *idTask? XERROR:XSUCC);
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
    if( XNULLP == pTaskName )
    {
        return XERROR;
    }
    
    if(XNULL == iTaskPri )
    {
        iTaskPri = TSK_PRIO_NORMAL;
    }
    else
        if(TSK_PRIO_LOWEST < iTaskPri || TSK_PRIO_HIGHEST > iTaskPri)
        {
            return XERROR;
        }
        
        if(0 > iTaskStackSize || XOSTASKSIZEMAX < iTaskStackSize )
        {
            return XERROR;
        }
        if(iTaskStackSize == XNULL)
        {
            iTaskStackSize = XOSTASKSIZEDEFAULT;
        }
        
        *idTask = taskSpawn((char*)pTaskName, iTaskPri, VX_FP_TASK, iTaskStackSize, (FUNCPTR)fTaskFun,
            (int)pPar, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        
        return ( ERROR == *idTask? XERROR:XSUCC);
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
    if(XNULL == idtask)
    {
        return XERROR;
    }
    return (OK == taskDelete(idtask)? XSUCC:XERROR);
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
Input:
sock    -    socket���
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
Input :     sock    -    socket���
ipaddr    -    ipv4��ַ(��:"166.241.5.11")
port    -    �󶨵Ķ˿�
Return:     XSUCC    -    �ɹ�
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
Input :
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
    printf("XOS_SusPend-------------\n");
    if( OK != taskSuspend(taskIdSelf()))
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
        /*printf("the errno is:%d\n",errno);*/
        req = rem;
        ret = nanosleep(&req, &rem);
        if( XERROR == ret && errno != EINTR )
        {
            return XERROR;
        }
    }

    return XSUCC;
}

#if 0
{
    XSTATIC XBOOL first = XTRUE;
    XSTATIC XU32 tmr_ticks = 60;
    
    if(first)
    {
        tmr_ticks = sysClkRateGet();
        first = XFALSE;
    }
    taskDelay( (XS32) (ms *tmr_ticks /1000 ));

    return XSUCC;
    
}
#endif


#if 1
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
    //XOS_UNUSED(pLocalIPList);
    XCHAR szLocalIP[32]   = {0};
    
    XOS_MemSet(pLocalIPList,0x00,sizeof(t_LOCALIPLIST));
    if ( 0 != gethostname( szLocalIP,sizeof(szLocalIP) ) )
    {
        return XERROR ;
    }
    
    if ( XERROR != (pLocalIPList->localIP[0] != hostGetByName(szLocalIP)) )
    {
        pLocalIPList->nIPNum = 1;
    }
    else
    {
        return XERROR ;
    }
    return XSUCC;
}
#endif

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
#ifdef XOS_VXWORKS
#ifdef XOS_TSP
    reset();
    return XSUCC;
#endif
    printf("reboot...\r\n");
    reboot(BOOT_NORMAL);
#endif
    return XSUCC;
}


/*************************************************************
������: XOS_SetSysTime
����: ���ò���ϵͳʱ��
����: timex---ʱ�䣬��Ԫ1970.1.1����������������
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: tsp��ʱ�����ú�����������Ч
              ����vxworks��������Ч
**************************************************************/
XS32 XOS_SetSysTime( t_XOSTB *timex )
{
    //tsp����Ҫ���õ�ʱ������������Ȼ��Ч
#ifdef XOS_VXWORKS
#ifdef XOS_TSP
    XCHAR strssTimeOut[20]={0};
    t_XOSTT now = 0;
    t_XOSTD oam_imte;
    struct tm InTm = {0,0,0,0,0,0,0,0,0};
    XS8 *TimeHead;
    XS8 strBak[20]={0};
    XS8 Year[5];
    XS8 Month[3];
    XS8 Day[3];
    XS8 s_hour[3];
    XS8 s_minute[3];
    XS8 s_second[3];
    XS8 s_wday[3];
    XS32 timezone_len=0;//(480*60);//������

    if(NULL == timex)
    {
        return XERROR;
    }

    timezone_len = timex->timezone*60;
    now = timex->time + timezone_len;

    XOS_LocalTime(&now, &oam_imte);
    sprintf(strssTimeOut, "%04d%02d%02d-%02d%02d%02d-%02d",
                       oam_imte.dt_year+1900, oam_imte.dt_mon+1,
                       oam_imte.dt_mday, oam_imte.dt_hour,
                       oam_imte.dt_min, oam_imte.dt_sec,oam_imte.dt_wday);
    
    strssTimeOut[18]=0x0;

    //������������
    XOS_StrCpy(strBak,strssTimeOut);

    //������������ڵ����
    TimeHead = strBak;
    XOS_StrNcpy(Year,TimeHead,4);
    Year[4] = 0;
    InTm.tm_year = atoi(Year);

    //������������ڵ��·�
    TimeHead = &strBak[4];
    XOS_StrNcpy(Month,TimeHead,2);
    Month[2] = 0;
    InTm.tm_mon = atoi(Month);

    //������������ڵ�����
    TimeHead = &strBak[6];
    XOS_StrNcpy(Day,TimeHead,2);
    Day[2] = 0;
    InTm.tm_mday = atoi(Day);

    //������������ڵ�ʱ
    TimeHead = &strBak[9];
    XOS_StrNcpy(s_hour,TimeHead,2);
    s_hour[3] = 0;
    InTm.tm_hour= atoi(s_hour);

    //������������ڵķ�
    TimeHead = &strBak[11];
    XOS_StrNcpy(s_minute,TimeHead,2);
    s_minute[2] = 0;
    InTm.tm_min= atoi(s_minute);

    //������������ڵ���
    TimeHead = &strBak[13];
    XOS_StrNcpy(s_second,TimeHead,2);
    s_second[2] = 0;
    InTm.tm_sec= atoi(s_second);

    //������������ڵ�wday
    TimeHead = &strBak[16];
    XOS_StrNcpy(s_wday,TimeHead,2);
    s_wday[2] = 0;
    InTm.tm_wday= atoi(s_wday);

    Set_Rtc_Time2(InTm.tm_sec,InTm.tm_min,InTm.tm_hour,InTm.tm_mday,InTm.tm_mon,InTm.tm_year,InTm.tm_wday);
    //printf("Set systime ok!\r\n");
   
    return XSUCC;
#else
    //���ú�����������
    struct timespec tp;
    
    if(NULL == timex)
    {
        return XERROR;
    }

    tp.tv_sec = timex->time;
    
    if(0 == clock_settime(CLOCK_REALTIME,&tp))
    {
        printf("set systime ok!\r\n");
        return XSUCC;
    }
    else
    {
        return XERROR;
        printf("set systime fail!\r\n");
    }
    
#endif
#endif
    
    return XSUCC;
}

/**************************************************************
������: XOS_spyInfoPrint
����: spy��Ϣ������������spy��Ϣ��ȫ�ֱ�����
����: fmt--spy��Ϣ
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
***************************************************************/
XS32 XOS_spyInfoPrint(const XS8 *fmt, ...)
{
    /*    va_list va;
    XS32 len;

    va_start(va, fmt);
    len = vsprintf(gspyCurrInfo, fmt, va);
    va_end(va);
    printf(gspyCurrInfo);*/

    return XSUCC;
}
#if 0
XS32 XOS_spyInfoPrint(const XS8 *fmt, ...)
{
    va_list va;
    XS32 len;
 //   static XS32 offset,len;  //�����ӡָ��ƫ��
/*    XS8 temp[3]={0};
    XS8 *pdest = (XS8 *)NULL;
    XS32 ii=0;
    XS32 ch='(';

    if (fmt[0] == '\n')
    {
        offset = 0;
        return 0;
    }

    if (offset == 0)
    {
        if (fmt[0] == '%' && fmt[1] == 'd')
        return 0;
    }*/

    va_start(va, fmt);
    len = vsprintf(gspyCurrInfo, fmt, va);
    va_end(va);
    printf(gspyCurrInfo);

    return XSUCC;
/*    offset += len;

    if (0 == XOS_MemCmp(&gspyCurrInfo[offset-len],"IDLE",4))
    {        
        pdest = strrchr(&gspyCurrInfo[offset-len],ch);
        if (NULL == pdest)
        {
            return 0;
        }
        ii = pdest - &gspyCurrInfo[offset-len];
        XOS_StrNcpy(temp,&gspyCurrInfo[offset-len+ii-4],2);

        gspyCpuRate = atoi(temp);
    }

    if (offset > (256+5)*80 )//�������������Ϊ256
        offset = 0;
    return 0;*/
}
//TSP_TEMP_VER = 20110418  //�޸İ汾��Ϊ1.4.14
#ifdef XOS_TSP
//����*idletotalPerCentΪ���ۼƵ�CPU idleռ����
//*idleIncPerCentΪ��4s��CPU idleռ����
//printOnOffΪ��ӡ����1Ϊ��ӡ��0Ϊ����ӡ
extern int  idleOccupancyGet(int *idleTotalPerCent, int *idleIncPerCent ,XBOOL printOnOff);
#endif
#endif

/**************************************************************
������: XOS_GetCpuRate
����: ��������ϵͳ
����: rate---����CPUռ��������, rate% ,����rate
���: ��
����: �ɹ�:XSUCC         ʧ��:XERROR
˵��: 
***************************************************************/
extern void spyCommon (int freq, int ticksPerSec, FUNCPTR printRtn);
extern UINT spyTotalTicks;
extern UINT spyIdleTicks;

static void cpuBurnTask()
{
    int i;
    while (1)
    {
        for (i=0; i<500000; ++i) {}
        XOS_Sleep(100);
    }    
}

void XOS_BurnCpu(int count)
{
    int i = 0;
    for (; i<count; ++i)
    {
        taskSpawn("cpuBurnTask",255,VX_FP_TASK,1000,(FUNCPTR)cpuBurnTask,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}
void XOS_GetCpuRateT()
{
    int f1 = 0;
    if (XOS_GetCpuRate(&f1) == XSUCC)
    {
        printf("rate = %d\n", f1); 
    }
    else
    {
        printf("get error!\n");
    }
}

XS32 XOS_GetCpuRate(int* rate)
{
#ifdef XOS_TSP
    int i_cpurate;
#endif

    if (rate == NULL)
    {
        return XERROR;
    }
#ifdef XOS_TSP
    drv_GetCpuRate(&i_cpurate);
    *rate=((i_cpurate>0)?i_cpurate:0);
#else
    *rate=0;
#endif
    return XSUCC;
}

#if 0
XS32 XOS_GetCpuRate(int *rate)
{
    int totalper = 0;
    #if 0
    //����spyCommonע��һ��������spy��Ϣ������ַ���;
    //Ȼ����ַ�������ȡ��ǰcpuռ����.
    if(0 == ucSpyStartFlag)
    {
        //spyCommon(int, int,int (*) (...)));//����ԭ��
        //ע��spy��Ϣ������,δ���Ƿ���ֵ
        //spyCommon(2, 200,(int (*)(...))(XOS_spyInfoPrint));
        ucSpyStartFlag = 1;
    }
    if(gspyCpuRate <= 99 && gspyCpuRate > 0)
    {
        *rate = 100 - gspyCpuRate;
    }
    else
    {
        *rate = 0;
        return XERROR;
    }
    #endif
#if 0 //def XOS_TSP
    if(0 == idleOccupancyGet(&totalper, (int *)rate ,0))//0����ӡ
    {
        return XSUCC;
    }
    else
    {
        return XERROR;
    }
#endif

    //����δҪ��ʵ��
    *rate = 0;

    return XSUCC;
}
#endif

#ifndef XOS_ARCH_64

XS32 XOS_GetMemRateT()
{
    XS32 bytesAlloc, bytesTotal;

    XOS_GetMemInfo(&bytesAlloc, &bytesTotal);
    printf("mem :%d(kbAlloc) - %d(kbTotal)\n", bytesAlloc, bytesTotal);
    return XSUCC;
}

#endif

#ifdef XOS_ARCH_64

XS32 XOS_GetMemInfo(XU64* kbAlloc, XU64* kbTotal)
{
    MEM_PART_STATS partStats;
    if (kbAlloc == NULL || kbTotal == NULL)
    {
        return XERROR;
    }

    *kbAlloc = 0;
    *kbTotal = 0;
    if (memPartInfoGet(memSysPartId, &partStats) == ERROR)
    {
        return XERROR;
    }
    *kbTotal = (partStats.numBytesAlloc + partStats.numBytesFree)/1024;
    *kbAlloc = partStats.numBytesAlloc/1024;

    return XSUCC;
}
#else
XS32 XOS_GetMemInfo(XS32* kbAlloc, XS32* kbTotal)
{
    MEM_PART_STATS partStats;
    if (kbAlloc == NULL || kbTotal == NULL)
    {
        return XERROR;
    }

    *kbAlloc = 0;
    *kbTotal = 0;
    if (memPartInfoGet(memSysPartId, &partStats) == ERROR)
    {
        return XERROR;
    }
    *kbTotal = (partStats.numBytesAlloc + partStats.numBytesFree)/1024;
    *kbAlloc = partStats.numBytesAlloc/1024;

    return XSUCC;
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
    XS32  strLens;
    XCHAR tempbuf[256] = "Unknown errno!";
    XCHAR *szErrDes = NULL;

    if(NULL == errmsg || buflen == 0)
    {
        return NULL;
    }

    strerror_r(errnum,tempbuf);
    szErrDes = tempbuf;

    strLens = XOS_MIN((buflen-1), XOS_StrLen(szErrDes));
    errmsg[strLens] = '\0'; 
        
    return XOS_StrNcpy(errmsg,szErrDes,strLens);
#else
    XCHAR *szErrDes = NULL;

    switch(errnum)
    {
        case        0: szErrDes = "OK"; break;
        case        1: szErrDes = "S_errno_EPERM"; break;
        case        2: szErrDes = "S_errno_ENOENT"; break;
        case        3: szErrDes = "S_errno_ESRCH"; break;
        case        4: szErrDes = "S_errno_EINTR"; break;
        case        5: szErrDes = "S_errno_EIO"; break;
        case        6: szErrDes = "S_errno_ENXIO"; break;
        case        7: szErrDes = "S_errno_E2BIG"; break;
        case        8: szErrDes = "S_errno_ENOEXEC"; break;
        case        9: szErrDes = "S_errno_EBADF"; break;
        case       10: szErrDes = "S_errno_ECHILD"; break;
        case       11: szErrDes = "S_errno_EAGAIN"; break;
        case       12: szErrDes = "S_errno_ENOMEM"; break;
        case       13: szErrDes = "S_errno_EACCES"; break;
        case       14: szErrDes = "S_errno_EFAULT"; break;
        case       15: szErrDes = "S_errno_ENOTEMPTY"; break;
        case       16: szErrDes = "S_errno_EBUSY"; break;
        case       17: szErrDes = "S_errno_EEXIST"; break;
        case       18: szErrDes = "S_errno_EXDEV"; break;
        case       19: szErrDes = "S_errno_ENODEV"; break;
        case       20: szErrDes = "S_errno_ENOTDIR"; break;
        case       21: szErrDes = "S_errno_EISDIR"; break;
        case       22: szErrDes = "S_errno_EINVAL"; break;
        case       23: szErrDes = "S_errno_ENFILE"; break;
        case       24: szErrDes = "S_errno_EMFILE"; break;
        case       25: szErrDes = "S_errno_ENOTTY"; break;
        case       26: szErrDes = "S_errno_ENAMETOOLONG"; break;
        case       27: szErrDes = "S_errno_EFBIG"; break;
        case       28: szErrDes = "S_errno_ENOSPC"; break;
        case       29: szErrDes = "S_errno_ESPIPE"; break;
        case       30: szErrDes = "S_errno_EROFS"; break;
        case       31: szErrDes = "S_errno_EMLINK"; break;
        case       32: szErrDes = "S_errno_EPIPE"; break;
        case       33: szErrDes = "S_errno_EDEADLK"; break;
        case       34: szErrDes = "S_errno_ENOLCK"; break;
        case       35: szErrDes = "S_errno_ENOTSUP"; break;
        case       36: szErrDes = "S_errno_EMSGSIZE"; break;
        case       37: szErrDes = "S_errno_EDOM"; break;
        case       38: szErrDes = "S_errno_ERANGE"; break;
        case       40: szErrDes = "S_errno_EDESTADDRREQ"; break;
        case       41: szErrDes = "S_errno_EPROTOTYPE"; break;
        case       42: szErrDes = "S_errno_ENOPROTOOPT"; break;
        case       43: szErrDes = "S_errno_EPROTONOSUPPORT"; break;
        case       44: szErrDes = "S_errno_ESOCKTNOSUPPORT"; break;
        case       45: szErrDes = "S_errno_EOPNOTSUPP"; break;
        case       46: szErrDes = "S_errno_EPFNOSUPPORT"; break;
        case       47: szErrDes = "S_errno_EAFNOSUPPORT"; break;
        case       48: szErrDes = "S_errno_EADDRINUSE"; break;
        case       49: szErrDes = "S_errno_EADDRNOTAVAIL"; break;
        case       50: szErrDes = "S_errno_ENOTSOCK"; break;
        case       51: szErrDes = "S_errno_ENETUNREACH"; break;
        case       52: szErrDes = "S_errno_ENETRESET"; break;
        case       53: szErrDes = "S_errno_ECONNABORTED"; break;
        case       54: szErrDes = "S_errno_ECONNRESET"; break;
        case       55: szErrDes = "S_errno_ENOBUFS"; break;
        case       56: szErrDes = "S_errno_EISCONN"; break;
        case       57: szErrDes = "S_errno_ENOTCONN"; break;
        case       58: szErrDes = "S_errno_ESHUTDOWN"; break;
        case       59: szErrDes = "S_errno_ETOOMANYREFS"; break;
        case       60: szErrDes = "S_errno_ETIMEDOUT"; break;
        case       61: szErrDes = "S_errno_ECONNREFUSED"; break;
        case       62: szErrDes = "S_errno_ENETDOWN"; break;
        case       63: szErrDes = "S_errno_ETXTBSY"; break;
        case       64: szErrDes = "S_errno_ELOOP"; break;
        case       65: szErrDes = "S_errno_EHOSTUNREACH"; break;
        case       66: szErrDes = "S_errno_ENOTBLK"; break;
        case       67: szErrDes = "S_errno_EHOSTDOWN"; break;
        case       68: szErrDes = "S_errno_EINPROGRESS"; break;
        case       69: szErrDes = "S_errno_EALREADY"; break;
        case       70: szErrDes = "S_errno_EWOULDBLOCK"; break;
        case       71: szErrDes = "S_errno_ENOSYS"; break;
        case       72: szErrDes = "S_errno_ECANCELED"; break;
        case       74: szErrDes = "S_errno_ENOSR"; break;
        case       75: szErrDes = "S_errno_ENOSTR"; break;
        case       76: szErrDes = "S_errno_EPROTO"; break;
        case       77: szErrDes = "S_errno_EBADMSG"; break;
        case       78: szErrDes = "S_errno_ENODATA"; break;
        case       79: szErrDes = "S_errno_ETIME"; break;
        case       80: szErrDes = "S_errno_ENOMSG"; break;
        case       81: szErrDes = "S_errno_ERRMAX"; break;
        case    32768: szErrDes = "S_errno_ERRNO_PX_FLAG"; break;
        case   196709: szErrDes = "S_taskLib_NAME_NOT_FOUND"; break;
        case   196710: szErrDes = "S_taskLib_TASK_HOOK_TABLE_FULL"; break;
        case   196711: szErrDes = "S_taskLib_TASK_HOOK_NOT_FOUND"; break;
        case   196712: szErrDes = "S_taskLib_TASK_SWAP_HOOK_REFERENCED"; break;
        case   196713: szErrDes = "S_taskLib_TASK_SWAP_HOOK_SET"; break;
        case   196714: szErrDes = "S_taskLib_TASK_SWAP_HOOK_CLEAR"; break;
        case   196715: szErrDes = "S_taskLib_TASK_VAR_NOT_FOUND"; break;
        case   196716: szErrDes = "S_taskLib_TASK_UNDELAYED"; break;
        case   196717: szErrDes = "S_taskLib_ILLEGAL_PRIORITY"; break;
        case   786433: szErrDes = "S_ioLib_NO_DRIVER"; break;
        case   786434: szErrDes = "S_ioLib_UNKNOWN_REQUEST"; break;
        case   786435: szErrDes = "S_ioLib_DEVICE_ERROR"; break;
        case   786436: szErrDes = "S_ioLib_DEVICE_TIMEOUT"; break;
        case   786437: szErrDes = "S_ioLib_WRITE_PROTECTED"; break;
        case   786438: szErrDes = "S_ioLib_DISK_NOT_PRESENT"; break;
        case   786439: szErrDes = "S_ioLib_NO_FILENAME"; break;
        case   786440: szErrDes = "S_ioLib_CANCELLED"; break;
        case   786441: szErrDes = "S_ioLib_NO_DEVICE_NAME_IN_PATH"; break;
        case   786442: szErrDes = "S_ioLib_NAME_TOO_LONG"; break;
        case   786443: szErrDes = "S_ioLib_UNFORMATED"; break;
        case   786444: szErrDes = "S_ioLib_CANT_OVERWRITE_DIR"; break;
        case   851969: szErrDes = "S_iosLib_DEVICE_NOT_FOUND"; break;
        case   851970: szErrDes = "S_iosLib_DRIVER_GLUT"; break;
        case   851971: szErrDes = "S_iosLib_INVALID_FILE_DESCRIPTOR"; break;
        case   851972: szErrDes = "S_iosLib_TOO_MANY_OPEN_FILES"; break;
        case   851973: szErrDes = "S_iosLib_CONTROLLER_NOT_PRESENT"; break;
        case   851974: szErrDes = "S_iosLib_DUPLICATE_DEVICE_NAME"; break;
        case   851975: szErrDes = "S_iosLib_INVALID_ETHERNET_ADDRESS"; break;
        case   917505: szErrDes = "S_loadLib_ROUTINE_NOT_INSTALLED"; break;
        case   917506: szErrDes = "S_loadLib_TOO_MANY_SYMBOLS"; break;
        case  1114113: szErrDes = "S_memLib_NOT_ENOUGH_MEMORY"; break;
        case  1114114: szErrDes = "S_memLib_INVALID_NBYTES"; break;
        case  1114115: szErrDes = "S_memLib_BLOCK_ERROR"; break;
        case  1114116: szErrDes = "S_memLib_NO_PARTITION_DESTROY"; break;
        case  1114117: szErrDes = "S_memLib_PAGE_SIZE_UNAVAILABLE"; break;
        case  1310721: szErrDes = "S_rt11FsLib_VOLUME_NOT_AVAILABLE"; break;
        case  1310722: szErrDes = "S_rt11FsLib_DISK_FULL"; break;
        case  1310723: szErrDes = "S_rt11FsLib_FILE_NOT_FOUND"; break;
        case  1310724: szErrDes = "S_rt11FsLib_NO_FREE_FILE_DESCRIPTORS"; break;
        case  1310725: szErrDes = "S_rt11FsLib_INVALID_NUMBER_OF_BYTES"; break;
        case  1310726: szErrDes = "S_rt11FsLib_FILE_ALREADY_EXISTS"; break;
        case  1310727: szErrDes = "S_rt11FsLib_BEYOND_FILE_LIMIT"; break;
        case  1310728: szErrDes = "S_rt11FsLib_INVALID_DEVICE_PARAMETERS"; break;
        case  1310729: szErrDes = "S_rt11FsLib_NO_MORE_FILES_ALLOWED_ON_DISK"; break;
        case  1310730: szErrDes = "S_rt11FsLib_ENTRY_NUMBER_TOO_BIG"; break;
        case  1310731: szErrDes = "S_rt11FsLib_NO_BLOCK_DEVICE"; break;
        case  1441893: szErrDes = "S_semLib_INVALID_STATE"; break;
        case  1441894: szErrDes = "S_semLib_INVALID_OPTION"; break;
        case  1441895: szErrDes = "S_semLib_INVALID_QUEUE_TYPE"; break;
        case  1441896: szErrDes = "S_semLib_INVALID_OPERATION"; break;
        case  1835009: szErrDes = "S_symLib_SYMBOL_NOT_FOUND"; break;
        case  1835010: szErrDes = "S_symLib_NAME_CLASH"; break;
        case  1835011: szErrDes = "S_symLib_TABLE_NOT_EMPTY"; break;
        case  1835012: szErrDes = "S_symLib_SYMBOL_STILL_IN_TABLE"; break;
        case  1835020: szErrDes = "S_symLib_INVALID_SYMTAB_ID"; break;
        case  1835021: szErrDes = "S_symLib_INVALID_SYM_ID_PTR"; break;
        case  2293761: szErrDes = "S_usrLib_NOT_ENOUGH_ARGS"; break;
        case  2424833: szErrDes = "S_remLib_ALL_PORTS_IN_USE"; break;
        case  2424834: szErrDes = "S_remLib_RSH_ERROR"; break;
        case  2424835: szErrDes = "S_remLib_IDENTITY_TOO_BIG"; break;
        case  2424836: szErrDes = "S_remLib_RSH_STDERR_SETUP_FAILED"; break;
        case  2686977: szErrDes = "S_netDrv_INVALID_NUMBER_OF_BYTES"; break;
        case  2686978: szErrDes = "S_netDrv_SEND_ERROR"; break;
        case  2686979: szErrDes = "S_netDrv_RECV_ERROR"; break;
        case  2686980: szErrDes = "S_netDrv_UNKNOWN_REQUEST"; break;
        case  2686981: szErrDes = "S_netDrv_BAD_SEEK"; break;
        case  2686982: szErrDes = "S_netDrv_SEEK_PAST_EOF_ERROR"; break;
        case  2686983: szErrDes = "S_netDrv_BAD_EOF_POSITION"; break;
        case  2686984: szErrDes = "S_netDrv_SEEK_FATAL_ERROR"; break;
        case  2686985: szErrDes = "S_netDrv_WRITE_ONLY_CANNOT_READ"; break;
        case  2686986: szErrDes = "S_netDrv_READ_ONLY_CANNOT_WRITE"; break;
        case  2686987: szErrDes = "S_netDrv_READ_ERROR"; break;
        case  2686988: szErrDes = "S_netDrv_WRITE_ERROR"; break;
        case  2686989: szErrDes = "S_netDrv_NO_SUCH_FILE_OR_DIR"; break;
        case  2686990: szErrDes = "S_netDrv_PERMISSION_DENIED"; break;
        case  2686991: szErrDes = "S_netDrv_IS_A_DIRECTORY"; break;
        case  2686992: szErrDes = "S_netDrv_UNIX_FILE_ERROR"; break;
        case  2686993: szErrDes = "S_netDrv_ILLEGAL_VALUE"; break;
        case  2818049: szErrDes = "S_inetLib_ILLEGAL_INTERNET_ADDRESS"; break;
        case  2818050: szErrDes = "S_inetLib_ILLEGAL_NETWORK_NUMBER"; break;
        case  2883585: szErrDes = "S_routeLib_ILLEGAL_INTERNET_ADDRESS"; break;
        case  2883586: szErrDes = "S_routeLib_ILLEGAL_NETWORK_NUMBER"; break;
        case  2949121: szErrDes = "S_nfsDrv_INVALID_NUMBER_OF_BYTES"; break;
        case  2949122: szErrDes = "S_nfsDrv_BAD_FLAG_MODE"; break;
        case  2949123: szErrDes = "S_nfsDrv_CREATE_NO_FILE_NAME"; break;
        case  2949124: szErrDes = "S_nfsDrv_FATAL_ERR_FLUSH_INVALID_CACHE"; break;
        case  2949125: szErrDes = "S_nfsDrv_WRITE_ONLY_CANNOT_READ"; break;
        case  2949126: szErrDes = "S_nfsDrv_READ_ONLY_CANNOT_WRITE"; break;
        case  2949127: szErrDes = "S_nfsDrv_NOT_AN_NFS_DEVICE"; break;
        case  2949128: szErrDes = "S_nfsDrv_NO_HOST_NAME_SPECIFIED"; break;
        case  2949129: szErrDes = "S_nfsDrv_PERMISSION_DENIED"; break;
        case  2949130: szErrDes = "S_nfsDrv_NO_SUCH_FILE_OR_DIR"; break;
        case  2949131: szErrDes = "S_nfsDrv_IS_A_DIRECTORY"; break;
        case  3014657: szErrDes = "S_nfsLib_NFS_AUTH_UNIX_FAILED"; break;
        case  3014658: szErrDes = "S_nfsLib_NFS_INAPPLICABLE_FILE_TYPE"; break;
        case  3080192: szErrDes = "S_rpcLib_RPC_SUCCESS"; break;
        case  3080193: szErrDes = "S_rpcLib_RPC_CANTENCODEARGS"; break;
        case  3080194: szErrDes = "S_rpcLib_RPC_CANTDECODERES"; break;
        case  3080195: szErrDes = "S_rpcLib_RPC_CANTSEND"; break;
        case  3080196: szErrDes = "S_rpcLib_RPC_CANTRECV"; break;
        case  3080197: szErrDes = "S_rpcLib_RPC_TIMEDOUT"; break;
        case  3080198: szErrDes = "S_rpcLib_RPC_VERSMISMATCH"; break;
        case  3080199: szErrDes = "S_rpcLib_RPC_AUTHERROR"; break;
        case  3080200: szErrDes = "S_rpcLib_RPC_PROGUNAVAIL"; break;
        case  3080201: szErrDes = "S_rpcLib_RPC_PROGVERSMISMATCH"; break;
        case  3080202: szErrDes = "S_rpcLib_RPC_PROCUNAVAIL"; break;
        case  3080203: szErrDes = "S_rpcLib_RPC_CANTDECODEARGS"; break;
        case  3080204: szErrDes = "S_rpcLib_RPC_SYSTEMERROR"; break;
        case  3080205: szErrDes = "S_rpcLib_RPC_UNKNOWNHOST"; break;
        case  3080206: szErrDes = "S_rpcLib_RPC_PMAPFAILURE"; break;
        case  3080207: szErrDes = "S_rpcLib_RPC_PROGNOTREGISTERED"; break;
        case  3080208: szErrDes = "S_rpcLib_RPC_FAILED"; break;
        case  3145728: szErrDes = "S_nfsLib_NFS_OK"; break;
        case  3145729: szErrDes = "S_nfsLib_NFSERR_PERM"; break;
        case  3145730: szErrDes = "S_nfsLib_NFSERR_NOENT"; break;
        case  3145733: szErrDes = "S_nfsLib_NFSERR_IO"; break;
        case  3145734: szErrDes = "S_nfsLib_NFSERR_NXIO"; break;
        case  3145741: szErrDes = "S_nfsLib_NFSERR_ACCES"; break;
        case  3145745: szErrDes = "S_nfsLib_NFSERR_EXIST"; break;
        case  3145747: szErrDes = "S_nfsLib_NFSERR_NODEV"; break;
        case  3145748: szErrDes = "S_nfsLib_NFSERR_NOTDIR"; break;
        case  3145749: szErrDes = "S_nfsLib_NFSERR_ISDIR"; break;
        case  3145755: szErrDes = "S_nfsLib_NFSERR_FBIG"; break;
        case  3145756: szErrDes = "S_nfsLib_NFSERR_NOSPC"; break;
        case  3145758: szErrDes = "S_nfsLib_NFSERR_ROFS"; break;
        case  3145791: szErrDes = "S_nfsLib_NFSERR_NAMETOOLONG"; break;
        case  3145794: szErrDes = "S_nfsLib_NFSERR_NOTEMPTY"; break;
        case  3145797: szErrDes = "S_nfsLib_NFSERR_DQUOT"; break;
        case  3145798: szErrDes = "S_nfsLib_NFSERR_STALE"; break;
        case  3145827: szErrDes = "S_nfsLib_NFSERR_WFLUSH"; break;
        case  3211265: szErrDes = "S_errnoLib_NO_STAT_SYM_TBL"; break;
        case  3276801: szErrDes = "S_hostLib_UNKNOWN_HOST"; break;
        case  3276802: szErrDes = "S_hostLib_HOST_ALREADY_ENTERED"; break;
        case  3276803: szErrDes = "S_hostLib_INVALID_PARAMETER"; break;
        case  3473409: szErrDes = "S_if_sl_INVALID_UNIT_NUMBER"; break;
        case  3473410: szErrDes = "S_if_sl_UNIT_UNINITIALIZED"; break;
        case  3473411: szErrDes = "S_if_sl_UNIT_ALREADY_INITIALIZED"; break;
        case  3538945: szErrDes = "S_loginLib_UNKNOWN_USER"; break;
        case  3538946: szErrDes = "S_loginLib_USER_ALREADY_EXISTS"; break;
        case  3538947: szErrDes = "S_loginLib_INVALID_PASSWORD"; break;
        case  3604481: szErrDes = "S_scsiLib_DEV_NOT_READY"; break;
        case  3604482: szErrDes = "S_scsiLib_WRITE_PROTECTED"; break;
        case  3604483: szErrDes = "S_scsiLib_MEDIUM_ERROR"; break;
        case  3604484: szErrDes = "S_scsiLib_HARDWARE_ERROR"; break;
        case  3604485: szErrDes = "S_scsiLib_ILLEGAL_REQUEST"; break;
        case  3604486: szErrDes = "S_scsiLib_BLANK_CHECK"; break;
        case  3604487: szErrDes = "S_scsiLib_ABORTED_COMMAND"; break;
        case  3604488: szErrDes = "S_scsiLib_VOLUME_OVERFLOW"; break;
        case  3604489: szErrDes = "S_scsiLib_UNIT_ATTENTION"; break;
        case  3604490: szErrDes = "S_scsiLib_SELECT_TIMEOUT"; break;
        case  3604491: szErrDes = "S_scsiLib_LUN_NOT_PRESENT"; break;
        case  3604492: szErrDes = "S_scsiLib_ILLEGAL_BUS_ID"; break;
        case  3604493: szErrDes = "S_scsiLib_NO_CONTROLLER"; break;
        case  3604494: szErrDes = "S_scsiLib_REQ_SENSE_ERROR"; break;
        case  3604495: szErrDes = "S_scsiLib_DEV_UNSUPPORTED"; break;
        case  3604496: szErrDes = "S_scsiLib_ILLEGAL_PARAMETER"; break;
        case  3604497: szErrDes = "S_scsiLib_EARLY_PHASE_CHANGE"; break;
        case  3604498: szErrDes = "S_scsiLib_ABORTED"; break;
        case  3604499: szErrDes = "S_scsiLib_ILLEGAL_OPERATION"; break;
        case  3604500: szErrDes = "S_scsiLib_DEVICE_EXIST"; break;
        case  3604501: szErrDes = "S_scsiLib_DISCONNECTED"; break;
        case  3604502: szErrDes = "S_scsiLib_BUS_RESET"; break;
        case  3604503: szErrDes = "S_scsiLib_DATA_TRANSFER_TIMEOUT"; break;
        case  3604504: szErrDes = "S_scsiLib_SOFTWARE_ERROR"; break;
        case  3604505: szErrDes = "S_scsiLib_NO_MORE_THREADS"; break;
        case  3604506: szErrDes = "S_scsiLib_UNKNOWN_SENSE_DATA"; break;
        case  3604507: szErrDes = "S_scsiLib_INVALID_BLOCK_SIZE"; break;
        case  3670017: szErrDes = "S_dosFsLib_32BIT_OVERFLOW"; break;
        case  3670018: szErrDes = "S_dosFsLib_DISK_FULL"; break;
        case  3670019: szErrDes = "S_dosFsLib_FILE_NOT_FOUND"; break;
        case  3670020: szErrDes = "S_dosFsLib_NO_FREE_FILE_DESCRIPTORS"; break;
        case  3670021: szErrDes = "S_dosFsLib_NOT_FILE"; break;
        case  3670022: szErrDes = "S_dosFsLib_NOT_SAME_VOLUME"; break;
        case  3670023: szErrDes = "S_dosFsLib_NOT_DIRECTORY"; break;
        case  3670024: szErrDes = "S_dosFsLib_DIR_NOT_EMPTY"; break;
        case  3670025: szErrDes = "S_dosFsLib_FILE_EXISTS"; break;
        case  3670026: szErrDes = "S_dosFsLib_INVALID_PARAMETER"; break;
        case  3670027: szErrDes = "S_dosFsLib_NAME_TOO_LONG"; break;
        case  3670028: szErrDes = "S_dosFsLib_UNSUPPORTED"; break;
        case  3670029: szErrDes = "S_dosFsLib_VOLUME_NOT_AVAILABLE"; break;
        case  3670030: szErrDes = "S_dosFsLib_INVALID_NUMBER_OF_BYTES"; break;
        case  3670031: szErrDes = "S_dosFsLib_ILLEGAL_NAME"; break;
        case  3670032: szErrDes = "S_dosFsLib_CANT_DEL_ROOT"; break;
        case  3670033: szErrDes = "S_dosFsLib_READ_ONLY"; break;
        case  3670034: szErrDes = "S_dosFsLib_ROOT_DIR_FULL"; break;
        case  3670035: szErrDes = "S_dosFsLib_NO_LABEL"; break;
        case  3670036: szErrDes = "S_dosFsLib_NO_CONTIG_SPACE"; break;
        case  3670037: szErrDes = "S_dosFsLib_FD_OBSOLETE"; break;
        case  3670038: szErrDes = "S_dosFsLib_DELETED"; break;
        case  3670039: szErrDes = "S_dosFsLib_INTERNAL_ERROR"; break;
        case  3670040: szErrDes = "S_dosFsLib_WRITE_ONLY"; break;
        case  3670041: szErrDes = "S_dosFsLib_ILLEGAL_PATH"; break;
        case  3670042: szErrDes = "S_dosFsLib_ACCESS_BEYOND_EOF"; break;
        case  3670043: szErrDes = "S_dosFsLib_DIR_READ_ONLY"; break;
        case  3670044: szErrDes = "S_dosFsLib_UNKNOWN_VOLUME_FORMAT"; break;
        case  3670045: szErrDes = "S_dosFsLib_ILLEGAL_CLUSTER_NUMBER"; break;
        case  3735553: szErrDes = "S_selectLib_NO_SELECT_SUPPORT_IN_DRIVER"; break;
        case  3735554: szErrDes = "S_selectLib_NO_SELECT_CONTEXT"; break;
        case  3735555: szErrDes = "S_selectLib_WIDTH_OUT_OF_RANGE"; break;
        case  3801089: szErrDes = "S_hashLib_KEY_CLASH"; break;
        case  3866625: szErrDes = "S_qLib_Q_CLASS_ID_ERROR"; break;
        case  3997697: szErrDes = "S_objLib_OBJ_ID_ERROR"; break;
        case  3997698: szErrDes = "S_objLib_OBJ_UNAVAILABLE"; break;
        case  3997699: szErrDes = "S_objLib_OBJ_DELETED"; break;
        case  3997700: szErrDes = "S_objLib_OBJ_TIMEOUT"; break;
        case  3997701: szErrDes = "S_objLib_OBJ_NO_METHOD"; break;
        case  4063233: szErrDes = "S_qPriHeapLib_NULL_HEAP_ARRAY"; break;
        case  4128769: szErrDes = "S_qPriBMapLib_NULL_BMAP_LIST"; break;
        case  4259841: szErrDes = "S_msgQLib_INVALID_MSG_LENGTH"; break;
        case  4259842: szErrDes = "S_msgQLib_NON_ZERO_TIMEOUT_AT_INT_LEVEL"; break;
        case  4259843: szErrDes = "S_msgQLib_INVALID_QUEUE_TYPE"; break;
        case  4325377: szErrDes = "S_classLib_CLASS_ID_ERROR"; break;
        case  4325378: szErrDes = "S_classLib_NO_CLASS_DESTROY"; break;
        case  4390913: szErrDes = "S_intLib_NOT_ISR_CALLABLE"; break;
        case  4390914: szErrDes = "S_intLib_VEC_TABLE_WP_UNAVAILABLE"; break;
        case  4521985: szErrDes = "S_cacheLib_INVALID_CACHE"; break;
        case  4587521: szErrDes = "S_rawFsLib_VOLUME_NOT_AVAILABLE"; break;
        case  4587522: szErrDes = "S_rawFsLib_END_OF_DEVICE"; break;
        case  4587523: szErrDes = "S_rawFsLib_NO_FREE_FILE_DESCRIPTORS"; break;
        case  4587524: szErrDes = "S_rawFsLib_INVALID_NUMBER_OF_BYTES"; break;
        case  4587525: szErrDes = "S_rawFsLib_ILLEGAL_NAME"; break;
        case  4587526: szErrDes = "S_rawFsLib_NOT_FILE"; break;
        case  4587527: szErrDes = "S_rawFsLib_READ_ONLY"; break;
        case  4587528: szErrDes = "S_rawFsLib_FD_OBSOLETE"; break;
        case  4587529: szErrDes = "S_rawFsLib_NO_BLOCK_DEVICE"; break;
        case  4587530: szErrDes = "S_rawFsLib_BAD_SEEK"; break;
        case  4587531: szErrDes = "S_rawFsLib_INVALID_PARAMETER"; break;
        case  4587532: szErrDes = "S_rawFsLib_32BIT_OVERFLOW"; break;
        case  4653057: szErrDes = "S_arpLib_INVALID_ARGUMENT"; break;
        case  4653058: szErrDes = "S_arpLib_INVALID_HOST"; break;
        case  4653059: szErrDes = "S_arpLib_INVALID_ENET_ADDRESS"; break;
        case  4653060: szErrDes = "S_arpLib_INVALID_FLAG"; break;
        case  4718593: szErrDes = "S_smLib_MEMORY_ERROR"; break;
        case  4718594: szErrDes = "S_smLib_INVALID_CPU_NUMBER"; break;
        case  4718595: szErrDes = "S_smLib_NOT_ATTACHED"; break;
        case  4718596: szErrDes = "S_smLib_NO_REGIONS"; break;
        case  4784129: szErrDes = "S_bootpLib_INVALID_ARGUMENT"; break;
        case  4784130: szErrDes = "S_bootpLib_INVALID_COOKIE"; break;
        case  4784131: szErrDes = "S_bootpLib_NO_BROADCASTS"; break;
        case  4784132: szErrDes = "S_bootpLib_PARSE_ERROR"; break;
        case  4784133: szErrDes = "S_bootpLib_INVALID_TAG"; break;
        case  4784134: szErrDes = "S_bootpLib_TIME_OUT"; break;
        case  4784135: szErrDes = "S_bootpLib_MEM_ERROR"; break;
        case  4784136: szErrDes = "S_bootpLib_NOT_INITIALIZED"; break;
        case  4784137: szErrDes = "S_bootpLib_BAD_DEVICE"; break;
        case  4849665: szErrDes = "S_icmpLib_TIMEOUT"; break;
        case  4849666: szErrDes = "S_icmpLib_NO_BROADCAST"; break;
        case  4849667: szErrDes = "S_icmpLib_INVALID_INTERFACE"; break;
        case  4849668: szErrDes = "S_icmpLib_INVALID_ARGUMENT"; break;
        case  4915201: szErrDes = "S_tftpLib_INVALID_ARGUMENT"; break;
        case  4915202: szErrDes = "S_tftpLib_INVALID_DESCRIPTOR"; break;
        case  4915203: szErrDes = "S_tftpLib_INVALID_COMMAND"; break;
        case  4915204: szErrDes = "S_tftpLib_INVALID_MODE"; break;
        case  4915205: szErrDes = "S_tftpLib_UNKNOWN_HOST"; break;
        case  4915206: szErrDes = "S_tftpLib_NOT_CONNECTED"; break;
        case  4915207: szErrDes = "S_tftpLib_TIMED_OUT"; break;
        case  4915208: szErrDes = "S_tftpLib_TFTP_ERROR"; break;
        case  4980737: szErrDes = "S_proxyArpLib_INVALID_PARAMETER"; break;
        case  4980738: szErrDes = "S_proxyArpLib_INVALID_INTERFACE"; break;
        case  4980739: szErrDes = "S_proxyArpLib_INVALID_PROXY_NET"; break;
        case  4980740: szErrDes = "S_proxyArpLib_INVALID_CLIENT"; break;
        case  4980741: szErrDes = "S_proxyArpLib_INVALID_ADDRESS"; break;
        case  4980742: szErrDes = "S_proxyArpLib_TIMEOUT"; break;
        case  5111809: szErrDes = "S_smPktLib_SHARED_MEM_TOO_SMALL"; break;
        case  5111810: szErrDes = "S_smPktLib_MEMORY_ERROR"; break;
        case  5111811: szErrDes = "S_smPktLib_DOWN"; break;
        case  5111812: szErrDes = "S_smPktLib_NOT_ATTACHED"; break;
        case  5111813: szErrDes = "S_smPktLib_INVALID_PACKET"; break;
        case  5111814: szErrDes = "S_smPktLib_PACKET_TOO_BIG"; break;
        case  5111815: szErrDes = "S_smPktLib_INVALID_CPU_NUMBER"; break;
        case  5111816: szErrDes = "S_smPktLib_DEST_NOT_ATTACHED"; break;
        case  5111817: szErrDes = "S_smPktLib_INCOMPLETE_BROADCAST"; break;
        case  5111818: szErrDes = "S_smPktLib_LIST_FULL"; break;
        case  5111819: szErrDes = "S_smPktLib_LOCK_TIMEOUT"; break;
        case  5177345: szErrDes = "S_loadEcoffLib_HDR_READ"; break;
        case  5177346: szErrDes = "S_loadEcoffLib_OPTHDR_READ"; break;
        case  5177347: szErrDes = "S_loadEcoffLib_SCNHDR_READ"; break;
        case  5177348: szErrDes = "S_loadEcoffLib_READ_SECTIONS"; break;
        case  5177349: szErrDes = "S_loadEcoffLib_LOAD_SECTIONS"; break;
        case  5177350: szErrDes = "S_loadEcoffLib_RELOC_READ"; break;
        case  5177351: szErrDes = "S_loadEcoffLib_SYMHDR_READ"; break;
        case  5177352: szErrDes = "S_loadEcoffLib_EXTSTR_READ"; break;
        case  5177353: szErrDes = "S_loadEcoffLib_EXTSYM_READ"; break;
        case  5177354: szErrDes = "S_loadEcoffLib_GPREL_REFERENCE"; break;
        case  5177355: szErrDes = "S_loadEcoffLib_JMPADDR_ERROR"; break;
        case  5177356: szErrDes = "S_loadEcoffLib_NO_REFLO_PAIR"; break;
        case  5177357: szErrDes = "S_loadEcoffLib_UNRECOGNIZED_RELOCENTRY"; break;
        case  5177358: szErrDes = "S_loadEcoffLib_REFHALF_OVFL"; break;
        case  5177359: szErrDes = "S_loadEcoffLib_UNEXPECTED_SYM_CLASS"; break;
        case  5177360: szErrDes = "S_loadEcoffLib_UNRECOGNIZED_SYM_CLASS"; break;
        case  5177361: szErrDes = "S_loadEcoffLib_FILE_READ_ERROR"; break;
        case  5177362: szErrDes = "S_loadEcoffLib_FILE_ENDIAN_ERROR"; break;
        case  5242881: szErrDes = "S_loadAoutLib_TOO_MANY_SYMBOLS"; break;
        case  5373953: szErrDes = "S_bootLoadLib_ROUTINE_NOT_INSTALLED"; break;
        case  5439508: szErrDes = "S_loadLib_NO_RELOCATION_ROUTINE"; break;
        case  5505025: szErrDes = "S_vmLib_NOT_PAGE_ALIGNED"; break;
        case  5505026: szErrDes = "S_vmLib_BAD_STATE_PARAM"; break;
        case  5505027: szErrDes = "S_vmLib_BAD_MASK_PARAM"; break;
        case  5505028: szErrDes = "S_vmLib_ADDR_IN_GLOBAL_SPACE"; break;
        case  5505029: szErrDes = "S_vmLib_TEXT_PROTECTION_UNAVAILABLE"; break;
        case  5505030: szErrDes = "S_vmLib_NO_FREE_REGIONS"; break;
        case  5505031: szErrDes = "S_vmLib_ADDRS_NOT_EQUAL"; break;
        case  5570561: szErrDes = "S_mmuLib_INVALID_PAGE_SIZE"; break;
        case  5570562: szErrDes = "S_mmuLib_NO_DESCRIPTOR"; break;
        case  5570563: szErrDes = "S_mmuLib_INVALID_DESCRIPTOR"; break;
        case  5570565: szErrDes = "S_mmuLib_OUT_OF_PMEGS"; break;
        case  5570566: szErrDes = "S_mmuLib_VIRT_ADDR_OUT_OF_BOUNDS"; break;
        case  5636097: szErrDes = "S_moduleLib_MODULE_NOT_FOUND"; break;
        case  5636098: szErrDes = "S_moduleLib_HOOK_NOT_FOUND"; break;
        case  5636099: szErrDes = "S_moduleLib_BAD_CHECKSUM"; break;
        case  5636100: szErrDes = "S_moduleLib_MAX_MODULES_LOADED"; break;
        case  5701633: szErrDes = "S_unldLib_MODULE_NOT_FOUND"; break;
        case  5701634: szErrDes = "S_unldLib_TEXT_IN_USE"; break;
        case  5767169: szErrDes = "S_smObjLib_NOT_INITIALIZED"; break;
        case  5767170: szErrDes = "S_smObjLib_NOT_A_GLOBAL_ADRS"; break;
        case  5767171: szErrDes = "S_smObjLib_NOT_A_LOCAL_ADRS"; break;
        case  5767172: szErrDes = "S_smObjLib_SHARED_MEM_TOO_SMALL"; break;
        case  5767173: szErrDes = "S_smObjLib_TOO_MANY_CPU"; break;
        case  5767174: szErrDes = "S_smObjLib_LOCK_TIMEOUT"; break;
        case  5767175: szErrDes = "S_smObjLib_NO_OBJECT_DESTROY"; break;
        case  5832705: szErrDes = "S_smNameLib_NOT_INITIALIZED"; break;
        case  5832706: szErrDes = "S_smNameLib_NAME_TOO_LONG"; break;
        case  5832707: szErrDes = "S_smNameLib_NAME_NOT_FOUND"; break;
        case  5832708: szErrDes = "S_smNameLib_VALUE_NOT_FOUND"; break;
        case  5832709: szErrDes = "S_smNameLib_NAME_ALREADY_EXIST"; break;
        case  5832710: szErrDes = "S_smNameLib_DATABASE_FULL"; break;
        case  5832711: szErrDes = "S_smNameLib_INVALID_WAIT_TYPE"; break;
        case  5963777: szErrDes = "S_m2Lib_INVALID_PARAMETER"; break;
        case  5963778: szErrDes = "S_m2Lib_ENTRY_NOT_FOUND"; break;
        case  5963779: szErrDes = "S_m2Lib_TCPCONN_FD_NOT_FOUND"; break;
        case  5963780: szErrDes = "S_m2Lib_INVALID_VAR_TO_SET"; break;
        case  5963781: szErrDes = "S_m2Lib_CANT_CREATE_SYS_SEM"; break;
        case  5963782: szErrDes = "S_m2Lib_CANT_CREATE_IF_SEM"; break;
        case  5963783: szErrDes = "S_m2Lib_CANT_CREATE_ROUTE_SEM"; break;
        case  5963784: szErrDes = "S_m2Lib_ARP_PHYSADDR_NOT_SPECIFIED"; break;
        case  5963785: szErrDes = "S_m2Lib_IF_TBL_IS_EMPTY"; break;
        case  5963786: szErrDes = "S_m2Lib_IF_CNFG_CHANGED"; break;
        case  5963787: szErrDes = "S_m2Lib_TOO_BIG"; break;
        case  5963788: szErrDes = "S_m2Lib_BAD_VALUE"; break;
        case  5963789: szErrDes = "S_m2Lib_READ_ONLY"; break;
        case  5963790: szErrDes = "S_m2Lib_GEN_ERR"; break;
        case  6160385: szErrDes = "S_mountLib_ILLEGAL_MODE"; break;
        case  6291457: szErrDes = "S_loadSomCoffLib_HDR_READ"; break;
        case  6291458: szErrDes = "S_loadSomCoffLib_AUXHDR_READ"; break;
        case  6291459: szErrDes = "S_loadSomCoffLib_SYM_READ"; break;
        case  6291460: szErrDes = "S_loadSomCoffLib_SYMSTR_READ"; break;
        case  6291461: szErrDes = "S_loadSomCoffLib_OBJ_FMT"; break;
        case  6291462: szErrDes = "S_loadSomCoffLib_SPHDR_ALLOC"; break;
        case  6291463: szErrDes = "S_loadSomCoffLib_SPHDR_READ"; break;
        case  6291464: szErrDes = "S_loadSomCoffLib_SUBSPHDR_ALLOC"; break;
        case  6291465: szErrDes = "S_loadSomCoffLib_SUBSPHDR_READ"; break;
        case  6291466: szErrDes = "S_loadSomCoffLib_SPSTRING_ALLOC"; break;
        case  6291467: szErrDes = "S_loadSomCoffLib_SPSTRING_READ"; break;
        case  6291468: szErrDes = "S_loadSomCoffLib_INFO_ALLOC"; break;
        case  6291469: szErrDes = "S_loadSomCoffLib_LOAD_SPACE"; break;
        case  6291470: szErrDes = "S_loadSomCoffLib_RELOC_ALLOC"; break;
        case  6291471: szErrDes = "S_loadSomCoffLib_RELOC_READ"; break;
        case  6291472: szErrDes = "S_loadSomCoffLib_RELOC_NEW"; break;
        case  6291473: szErrDes = "S_loadSomCoffLib_RELOC_VERSION"; break;
        case  6356993: szErrDes = "S_loadElfLib_HDR_READ"; break;
        case  6356994: szErrDes = "S_loadElfLib_HDR_ERROR"; break;
        case  6356995: szErrDes = "S_loadElfLib_PHDR_MALLOC"; break;
        case  6356996: szErrDes = "S_loadElfLib_PHDR_READ"; break;
        case  6356997: szErrDes = "S_loadElfLib_SHDR_MALLOC"; break;
        case  6356998: szErrDes = "S_loadElfLib_SHDR_READ"; break;
        case  6356999: szErrDes = "S_loadElfLib_READ_SECTIONS"; break;
        case  6357000: szErrDes = "S_loadElfLib_LOAD_SECTIONS"; break;
        case  6357001: szErrDes = "S_loadElfLib_LOAD_PROG"; break;
        case  6357002: szErrDes = "S_loadElfLib_SYMTAB"; break;
        case  6357003: szErrDes = "S_loadElfLib_RELA_SECTION"; break;
        case  6357004: szErrDes = "S_loadElfLib_SCN_READ"; break;
        case  6357005: szErrDes = "S_loadElfLib_SDA_MALLOC"; break;
        case  6357007: szErrDes = "S_loadElfLib_SST_READ"; break;
        case  6357012: szErrDes = "S_loadElfLib_JMPADDR_ERROR"; break;
        case  6357013: szErrDes = "S_loadElfLib_GPREL_REFERENCE"; break;
        case  6357014: szErrDes = "S_loadElfLib_UNRECOGNIZED_RELOCENTRY"; break;
        case  6357015: szErrDes = "S_loadElfLib_RELOC"; break;
        case  6357016: szErrDes = "S_loadElfLib_UNSUPPORTED"; break;
        case  6357017: szErrDes = "S_loadElfLib_REL_SECTION"; break;
        case  6422529: szErrDes = "S_mbufLib_ID_INVALID"; break;
        case  6422530: szErrDes = "S_mbufLib_ID_EMPTY"; break;
        case  6422531: szErrDes = "S_mbufLib_SEGMENT_NOT_FOUND"; break;
        case  6422532: szErrDes = "S_mbufLib_LENGTH_INVALID"; break;
        case  6422533: szErrDes = "S_mbufLib_OFFSET_INVALID"; break;
        case  6488065: szErrDes = "S_pingLib_NOT_INITIALIZED"; break;
        case  6488066: szErrDes = "S_pingLib_TIMEOUT"; break;
        case  6750209: szErrDes = "S_tapeFsLib_NO_SEQ_DEV"; break;
        case  6750210: szErrDes = "S_tapeFsLib_ILLEGAL_TAPE_CONFIG_PARM"; break;
        case  6750211: szErrDes = "S_tapeFsLib_SERVICE_NOT_AVAILABLE"; break;
        case  6750212: szErrDes = "S_tapeFsLib_INVALID_BLOCK_SIZE"; break;
        case  6750213: szErrDes = "S_tapeFsLib_ILLEGAL_FILE_SYSTEM_NAME"; break;
        case  6750214: szErrDes = "S_tapeFsLib_ILLEGAL_FLAGS"; break;
        case  6750215: szErrDes = "S_tapeFsLib_FILE_DESCRIPTOR_BUSY"; break;
        case  6750216: szErrDes = "S_tapeFsLib_VOLUME_NOT_AVAILABLE"; break;
        case  6750217: szErrDes = "S_tapeFsLib_BLOCK_SIZE_MISMATCH"; break;
        case  6750218: szErrDes = "S_tapeFsLib_INVALID_NUMBER_OF_BYTES"; break;
        case  6815745: szErrDes = "S_snmpdLib_VIEW_CREATE_FAILURE"; break;
        case  6815746: szErrDes = "S_snmpdLib_VIEW_INSTALL_FAILURE"; break;
        case  6815747: szErrDes = "S_snmpdLib_VIEW_MASK_FAILURE"; break;
        case  6815748: szErrDes = "S_snmpdLib_VIEW_DEINSTALL_FAILURE"; break;
        case  6815749: szErrDes = "S_snmpdLib_VIEW_LOOKUP_FAILURE"; break;
        case  6815750: szErrDes = "S_snmpdLib_MIB_ADDITION_FAILURE"; break;
        case  6815751: szErrDes = "S_snmpdLib_NODE_NOT_FOUND"; break;
        case  6815752: szErrDes = "S_snmpdLib_INVALID_SNMP_VERSION"; break;
        case  6815753: szErrDes = "S_snmpdLib_TRAP_CREATE_FAILURE"; break;
        case  6815754: szErrDes = "S_snmpdLib_TRAP_BIND_FAILURE"; break;
        case  6815755: szErrDes = "S_snmpdLib_TRAP_ENCODE_FAILURE"; break;
        case  6815756: szErrDes = "S_snmpdLib_INVALID_OID_SYNTAX"; break;
        case  7012353: szErrDes = "S_resolvLib_NO_RECOVERY"; break;
        case  7012354: szErrDes = "S_resolvLib_TRY_AGAIN"; break;
        case  7012355: szErrDes = "S_resolvLib_HOST_NOT_FOUND"; break;
        case  7012356: szErrDes = "S_resolvLib_NO_DATA"; break;
        case  7012357: szErrDes = "S_resolvLib_BUFFER_2_SMALL"; break;
        case  7012358: szErrDes = "S_resolvLib_INVALID_PARAMETER"; break;
        case  7012359: szErrDes = "S_resolvLib_INVALID_ADDRESS"; break;
        case  7143425: szErrDes = "S_muxLib_LOAD_FAILED"; break;
        case  7143426: szErrDes = "S_muxLib_NO_DEVICE"; break;
        case  7143427: szErrDes = "S_muxLib_INVALID_ARGS"; break;
        case  7143428: szErrDes = "S_muxLib_ALLOC_FAILED"; break;
        case  7143429: szErrDes = "S_muxLib_ALREADY_BOUND"; break;
        case  7143430: szErrDes = "S_muxLib_UNLOAD_FAILED"; break;
        case  7143431: szErrDes = "S_muxLib_NOT_A_TK_DEVICE"; break;
        case  7143432: szErrDes = "S_muxLib_NO_TK_DEVICE"; break;
        case  7143433: szErrDes = "S_muxLib_END_BIND_FAILED"; break;
        case  7340033: szErrDes = "S_dhcpsLib_NOT_INITIALIZED"; break;
        case  7405569: szErrDes = "S_sntpcLib_INVALID_PARAMETER"; break;
        case  7405570: szErrDes = "S_sntpcLib_INVALID_ADDRESS"; break;
        case  7405571: szErrDes = "S_sntpcLib_TIMEOUT"; break;
        case  7405572: szErrDes = "S_sntpcLib_VERSION_UNSUPPORTED"; break;
        case  7405573: szErrDes = "S_sntpcLib_SERVER_UNSYNC"; break;
        case  7471105: szErrDes = "S_sntpsLib_INVALID_PARAMETER"; break;
        case  7471106: szErrDes = "S_sntpsLib_INVALID_ADDRESS"; break;
        case  7536641: szErrDes = "S_netBufLib_MEMSIZE_INVALID"; break;
        case  7536642: szErrDes = "S_netBufLib_CLSIZE_INVALID"; break;
        case  7536643: szErrDes = "S_netBufLib_NO_SYSTEM_MEMORY"; break;
        case  7536644: szErrDes = "S_netBufLib_MEM_UNALIGNED"; break;
        case  7536645: szErrDes = "S_netBufLib_MEMSIZE_UNALIGNED"; break;
        case  7536646: szErrDes = "S_netBufLib_MEMAREA_INVALID"; break;
        case  7536647: szErrDes = "S_netBufLib_MBLK_INVALID"; break;
        case  7536648: szErrDes = "S_netBufLib_NETPOOL_INVALID"; break;
        case  7536649: szErrDes = "S_netBufLib_INVALID_ARGUMENT"; break;
        case  7536650: szErrDes = "S_netBufLib_NO_POOL_MEMORY"; break;
        case  7602177: szErrDes = "S_cdromFsLib_ALREADY_INIT"; break;
        case  7602179: szErrDes = "S_cdromFsLib_DEVICE_REMOVED"; break;
        case  7602180: szErrDes = "S_cdromFsLib_SUCH_PATH_TABLE_SIZE_NOT_SUPPORTED"; break;
        case  7602181: szErrDes = "S_cdromFsLib_ONE_OF_VALUES_NOT_POWER_OF_2"; break;
        case  7602182: szErrDes = "S_cdromFsLib_UNKNOWN_FILE_SYSTEM"; break;
        case  7602183: szErrDes = "S_cdromFsLib_INVAL_VOL_DESCR"; break;
        case  7602184: szErrDes = "S_cdromFsLib_INVALID_PATH_STRING"; break;
        case  7602185: szErrDes = "S_cdromFsLib_MAX_DIR_HIERARCHY_LEVEL_OVERFLOW"; break;
        case  7602186: szErrDes = "S_cdromFsLib_NO_SUCH_FILE_OR_DIRECTORY"; break;
        case  7602187: szErrDes = "S_cdromFsLib_INVALID_DIRECTORY_STRUCTURE"; break;
        case  7602188: szErrDes = "S_cdromFsLib_INVALID_FILE_DESCRIPTOR"; break;
        case  7602189: szErrDes = "S_cdromFsLib_READ_ONLY_DEVICE"; break;
        case  7602190: szErrDes = "S_cdromFsLib_END_OF_FILE"; break;
        case  7602191: szErrDes = "S_cdromFsLib_INV_ARG_VALUE"; break;
        case  7602192: szErrDes = "S_cdromFsLib_SEMTAKE_ERROR"; break;
        case  7602193: szErrDes = "S_cdromFsLib_SEMGIVE_ERROR"; break;
        case  7602194: szErrDes = "S_cdromFsLib_VOL_UNMOUNTED"; break;
        case  7602195: szErrDes = "S_cdromFsLib_INVAL_DIR_OPER"; break;
        case  7602196: szErrDes = "S_cdromFsLib_READING_FAILURE"; break;
        case  7602197: szErrDes = "S_cdromFsLib_INVALID_DIR_REC_STRUCT"; break;
        case  7602199: szErrDes = "S_cdromFsLib_SESSION_NR_NOT_SUPPORTED"; break;
        case  7667713: szErrDes = "S_loadLib_FILE_READ_ERROR"; break;
        case  7667714: szErrDes = "S_loadLib_REALLOC_ERROR"; break;
        case  7667715: szErrDes = "S_loadLib_JMPADDR_ERROR"; break;
        case  7667716: szErrDes = "S_loadLib_NO_REFLO_PAIR"; break;
        case  7667717: szErrDes = "S_loadLib_GPREL_REFERENCE"; break;
        case  7667718: szErrDes = "S_loadLib_UNRECOGNIZED_RELOCENTRY"; break;
        case  7667719: szErrDes = "S_loadLib_REFHALF_OVFL"; break;
        case  7667720: szErrDes = "S_loadLib_FILE_ENDIAN_ERROR"; break;
        case  7667721: szErrDes = "S_loadLib_UNEXPECTED_SYM_CLASS"; break;
        case  7667722: szErrDes = "S_loadLib_UNRECOGNIZED_SYM_CLASS"; break;
        case  7667723: szErrDes = "S_loadLib_HDR_READ"; break;
        case  7667724: szErrDes = "S_loadLib_OPTHDR_READ"; break;
        case  7667725: szErrDes = "S_loadLib_SCNHDR_READ"; break;
        case  7667726: szErrDes = "S_loadLib_READ_SECTIONS"; break;
        case  7667727: szErrDes = "S_loadLib_LOAD_SECTIONS"; break;
        case  7667728: szErrDes = "S_loadLib_RELOC_READ"; break;
        case  7667729: szErrDes = "S_loadLib_SYMHDR_READ"; break;
        case  7667730: szErrDes = "S_loadLib_EXTSTR_READ"; break;
        case  7667731: szErrDes = "S_loadLib_EXTSYM_READ"; break;
        case  8060929: szErrDes = "S_miiLib_PHY_LINK_DOWN"; break;
        case  8060930: szErrDes = "S_miiLib_PHY_NULL"; break;
        case  8060931: szErrDes = "S_miiLib_PHY_NO_ABLE"; break;
        case  8060932: szErrDes = "S_miiLib_PHY_AN_FAIL"; break;
        case  8126465: szErrDes = "S_poolLib_ARG_NOT_VALID"; break;
        case  8126466: szErrDes = "S_poolLib_INVALID_POOL_ID"; break;
        case  8126467: szErrDes = "S_poolLib_NOT_POOL_ITEM"; break;
        case  8126468: szErrDes = "S_poolLib_POOL_IN_USE"; break;
        case  8126469: szErrDes = "S_poolLib_STATIC_POOL_EMPTY"; break;
        case  8192001: szErrDes = "S_setLib_LIB_INIT"; break;
        case  8192002: szErrDes = "S_setLib_LIB_NOT_INIT"; break;
        case  8192003: szErrDes = "S_setLib_ARG_NOT_VALID"; break;
        case  8192004: szErrDes = "S_setLib_OBJ_NOT_IN_SET"; break;
        case  8257537: szErrDes = "S_dmsLib_DMS_INIT"; break;
        case  8257538: szErrDes = "S_dmsLib_DMS_NOT_INIT"; break;
        case  8257539: szErrDes = "S_dmsLib_ARG_NOT_VALID"; break;
        case  8257540: szErrDes = "S_dmsLib_NAME_NOT_UNIQUE"; break;
        case  8257541: szErrDes = "S_dmsLib_NAME_UNKNOWN"; break;
        case  8257542: szErrDes = "S_dmsLib_DRIVER_UNKNOWN"; break;
        case  8257543: szErrDes = "S_dmsLib_BASIS_UNKNOWN"; break;
        case  8257544: szErrDes = "S_dmsLib_NO_CONNECT"; break;
        case  8257545: szErrDes = "S_dmsLib_NO_DISCONNECT"; break;
        case  8257546: szErrDes = "S_dmsLib_OBJ_ID_ERROR"; break;
        case  8257547: szErrDes = "S_dmsLib_ATTR_NOT_VALID"; break;
        case  8257548: szErrDes = "S_dmsLib_NOT_SUPPORTED"; break;
        case  8257549: szErrDes = "S_dmsLib_DRIVER_INACTIVE"; break;
        case  8257550: szErrDes = "S_dmsLib_INVALID_CLASS"; break;
        case  8454145: szErrDes = "S_devCfgLib_DCFG_INIT"; break;
        case  8454146: szErrDes = "S_devCfgLib_DCFG_NOT_INIT"; break;
        case  8454147: szErrDes = "S_devCfgLib_PARAM_VAL_NOT_VALID"; break;
        case  8454148: szErrDes = "S_devCfgLib_DEVICE_NOT_FOUND"; break;
        case  8454149: szErrDes = "S_devCfgLib_PARAM_NOT_FOUND"; break;
        case  8454150: szErrDes = "S_devCfgLib_NO_NVRAM_SPACE"; break;
        case  8454151: szErrDes = "S_devCfgLib_NO_NVRAM"; break;
        case  8716289: szErrDes = "S_cbioLib_INVALID_CBIO_DEV_ID"; break;
        case  8781825: szErrDes = "S_eventLib_TIMEOUT"; break;
        case  8781826: szErrDes = "S_eventLib_NOT_ALL_EVENTS"; break;
        case  8781827: szErrDes = "S_eventLib_ALREADY_REGISTERED"; break;
        case  8781828: szErrDes = "S_eventLib_EVENTSEND_FAILED"; break;
        case  8781829: szErrDes = "S_eventLib_ZERO_EVENTS"; break;
        case  8781830: szErrDes = "S_eventLib_TASK_NOT_REGISTERED"; break;
        case  8781831: szErrDes = "S_eventLib_NULL_TASKID_AT_INT_LEVEL"; break;
        case  8912897: szErrDes = "S_ftpLib_ILLEGAL_VALUE"; break;
        case  8912898: szErrDes = "S_ftpLib_TRANSIENT_RETRY_LIMIT_EXCEEDED"; break;
        case  8912899: szErrDes = "S_ftpLib_FATAL_TRANSIENT_RESPONSE"; break;
        case  8913117: szErrDes = "S_ftpLib_REMOTE_SERVER_STATUS_221"; break;
        case  8913122: szErrDes = "S_ftpLib_REMOTE_SERVER_STATUS_226"; break;
        case  8913153: szErrDes = "S_ftpLib_REMOTE_SERVER_STATUS_257"; break;
        case  8913318: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_422"; break;
        case  8913321: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_425"; break;
        case  8913346: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_450"; break;
        case  8913347: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_451"; break;
        case  8913348: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_452"; break;
        case  8913396: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_500"; break;
        case  8913397: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_501"; break;
        case  8913398: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_502"; break;
        case  8913399: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_503"; break;
        case  8913400: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_504"; break;
        case  8913416: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_520"; break;
        case  8913417: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_521"; break;
        case  8913426: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_530"; break;
        case  8913446: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_550"; break;
        case  8913447: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_551"; break;
        case  8913448: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_552"; break;
        case  8913449: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_553"; break;
        case  8913450: szErrDes = "S_ftpLib_REMOTE_SERVER_ERROR_554"; break;

        default      : szErrDes = "unknown errno!"; break;
    }

    return szErrDes;
#endif
}

#ifdef __cplusplus
}
#endif /*__cplusplus */

