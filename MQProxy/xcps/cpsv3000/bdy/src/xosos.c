/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosos.c
**
**  description:  os��װ�Ĺ����ļ�
**
**  author: wulei
**
**  date:   2006.09.04
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            

**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xosos.h"
#include "xosfilesys.h"

#ifdef XOS_WIN32
#include <winbase.h>
const unsigned __int64 FILETIME_TO_TIMEVAL_SKEW=0x19db1ded53e8000;
#endif

#define MAX_PRI 6
#define MIN_PRI 0

#ifdef XOS_VXWORKS
    XOS_SERTIME_T gSetTime = {0,1,0,70,0,0,0};
    t_XOSTT gMsec = 0;
    extern unsigned long tickGet(void);
#endif

#ifdef XOS_LINUX
#include <sys/times.h>
#endif
#if 0
/************************************************************************
������: XOS_GetFileLen
���ܣ� ��ȡ�ļ��ĳ���
���룺 filename  -   Ҫ��ȡ�ļ������·����,����/G711/tone.g729
����� resultlen -   �ļ���ʵ�ʳ���
���أ�
XSUCC    -   �ɹ�
XERROR   -   ʧ��
˵����
************************************************************************/
XS32 XOS_GetFileLen(XU8 *filename ,XU32 *length)
{
#ifdef XOS_WIN32
    struct _stat fbuf;
#else
    struct stat fbuf; 
#endif

    /*ȡ���ļ���С,���ֽڼ���*/
#ifdef XOS_WIN32
    if(XERROR == _stat((const char *)filename,&fbuf))
#else        
    if(XERROR == stat((const char *)filename,&fbuf))
#endif
    {
        return XERROR;
    }
    *length = (XU32)fbuf.st_size;

    return XSUCC;
}


/************************************************************************
������: XOS_ReadVoxFile
���ܣ� ���ļ��ж�ȡ����
���룺 filename  -   Ҫ��ȡ�ļ������·����,����/G711/tone.g729
�����
result   -   ���������ļ����ݵ����ݿռ�
resultlen -   �ļ���ʵ�ʳ���
���أ�
XSUCC    -   �ɹ�
XERROR   -   ʧ��
˵��������result�Ŀռ���Ӧ���ṩ��Լ��һ�����ֵ
************************************************************************/
XS32 XOS_ReadVoxFile(XU8 *filename ,XU8 *result ,XU32 *resultlen)
{
#ifdef XOS_WIN32
    struct _stat fbuf;
#else
    struct stat fbuf; 
#endif
    FILE *fp;
    XU32 numb = 0;

    /*ȡ���ļ���С,���ֽڼ���*/
#ifdef XOS_WIN32
    if(XERROR == _stat((const char *)filename,&fbuf))
#else        
    if(XERROR == stat((const char *)filename,&fbuf))
#endif     
    {
        return XERROR;
    }
    *resultlen = (XU32)fbuf.st_size;

    if(XNULLP == (fp = XOS_Fopen((XCONST XU8 *)filename , "rb")))
    {
        return XERROR;
    }

    for( ;numb < *resultlen ; )
    {
        numb += fread((XVOID *)(result+numb) ,(size_t)sizeof(XU8) ,*resultlen - numb ,fp);
    }

    XOS_Fclose(fp);

    return XSUCC;
}


/************************************************************************
������: XOS_RemoveFile
���ܣ� ɾ���ļ�
���룺 filename  -   ���·����,����/G711/tone.g729
����� N/A
���أ�
XSUCC    -   �ɹ�
XERROR   -   ʧ��
˵���� ��ɾ���ļ�ǰ��Ҫ�Ȱ������õ����ļ��ľ�����ر�
************************************************************************/
XS32 XOS_RemoveFile( XU8 *filename )
{
    if(XERROR == remove((const char *)filename))
    {
        return XERROR;
    }
    return XSUCC;
}
#endif


/************************************************************************
������: XOS_StrNChr
���ܣ� ���ַ���string�������ַ�ch��һ�γ��ֵ�λ�ã�������string�е�len���ַ�ֹ
���룺
string    -   ���������ַ���
ch    -   ���ҵ��ַ���������'@'�ķ�ʽ���룩
len   -   �ַ���string�ĳ���
�����
����ֵ��
�ɹ�    -   ָ���һ��ch��string��λ��ָ�룬
ʧ��    -   ����XNULL��ʾδ�ҵ�
˵����1~255�Ǳ�׼�ַ��⣬��ϵͳ����(����)���Լ�������ַ���
����Windows�´���30000ʱ���ж�Ӧ���ַ���
************************************************************************/
XCHAR *XOS_StrNChr (XCHAR *string , XS32 ch , XU32 len)
{
    XU32 i = 0;
    char *str = XNULL;
    XCHAR *p = XNULL,*q = XNULL;

    XOS_UNUSED(i);

    if(string == XNULL|| len == 0 || (len+1) > 2097152)
    {
        return XNULL;
    }

    str = (char *)malloc(len+1);
    if(str == XNULL)
    {
        return XNULL;
    }

    strncpy(str,string,len);
    str[len] = '\0';

#if 0
    if( *(string+len-1) != '\0')    /*Ҳ��ĳ�strncpy����*/
    {
        for(i = 0; i <= len ; i++)
        {
            if(i == len)
                str[i] = '\0';
            else
                str[i] = *(string+i);            
        }
    }
    else
    {
        if(XNULL == (p = strchr(string, ch )))
        {
            free(str);
            return XNULL;
        }
        else
        {
            free(str);
            return p;
        }
    }
#endif

    if(XNULL == (p = strchr(str, ch )))
    {
        free(str);
        return XNULL;
    }
    else
    {
        q = string+(strlen(str)-strlen(p));
        free(str);
        return q;
    }
}


/************************************************************************
������: XOS_StrNStr
���ܣ� �õ���stack�е�һ�ΰ���needle�ַ�����λ�ã���������str�е�len���ַ�ֹ
���룺
stack -   Ŀ���ַ���
needle        -   Ҫ���ҵ��ַ���
stacklen  -   Ŀ�괮stack�ĳ���
ndlen -   �����Ҵ�needle�ĳ���
�����N/A
����ֵ��
�ɹ�    -   ������stack�е�һ�ΰ���needle�ַ�����λ��ָ��,
ʧ��    -   ����XNULL��ʾδ�ҵ�

˵��������result�Ŀռ���Ӧ���ṩ��Լ��һ�����ֵ
************************************************************************/
XCHAR *XOS_StrNStr (XCHAR *stack , XCHAR *needle , XU32 stacklen ,XU32 ndlen)
{
    XU32 i = 0;
    char *str1 = XNULL;
    char *str2 = XNULL;
    XCHAR *p = XNULL,*q = XNULL;

    XOS_UNUSED(i);

    if(stack == XNULL|| stacklen == 0 || needle == XNULL || ndlen == 0 || stacklen > 2097152 || ndlen > 1024)
    {
        return XNULL;
    }

    str1 = (char *)malloc(stacklen+1);
    if(str1 == XNULL )
    {
        return XNULL;
    }
    str2 = (char *)malloc(ndlen+1);
    if(str2 == XNULL )
    {
        free(str1);
        return XNULL;
    }

    strncpy(str2,needle,ndlen);
    str2[ndlen] = '\0';

    strncpy(str1,stack,stacklen);
    str1[stacklen] = '\0';

#if 0
    //if( *(needle+ndlen-1) != '\0')
    {
        for(i = 0; i <= ndlen ; i++)
        {
            if(i == ndlen)
                str2[i] = '\0';
            else
                str2[i] = *(needle+i);            
        }
    }

    if( *(stack+stacklen-1) != '\0')
    {
        for(i = 0; i <= stacklen ; i++)
        {
            if(i == stacklen)
                str1[i] = '\0';
            else
                str1[i] = *(stack+i);            
        }
    }
    else
    {
        if(XNULL == (p = strstr(stack, str2 )))
        {
            free(str2);
            free(str1);
            return XNULL;
        }
        else
        {
            free(str2);
            free(str1);
            return p;
        }
    }
#endif

    if(XNULL == (p = strstr(str1, str2 )))
    {
        free(str2);
        free(str1);
        return XNULL;
    }
    else
    {
        q = stack+(strlen(str1)-strlen(p));
        free(str2);
        free(str1);
        return q;
    }
}


/************************************************************************
������: XOS_StrToLow
���ܣ� ת���ַ����е��ַ�ΪСд
���룺 string  -   ��Ҫת�����ַ���
����� string -   ת������ַ���
���أ�
XSUCC    -   �ɹ�
XERROR   -   ʧ��
˵����  ��Ҫ�������˺����Ĳ����������� char a[] = ""; ��������ʽ��Ӧ�ö���
************************************************************************/
XS32 XOS_StrToLow(XU8 string[] )
{
    XU32 i = 0;
    if(XNULLP == string)
    {
        return XERROR;
    }
    for(i= 0 ; i <= XOS_StrLen(string); i++)
    {
        string[i] = XOS_ToLower(string[i]);
    }
    return XSUCC;
}


/************************************************************************
������: XOS_StrNtoLow
���ܣ� ת���ַ����е��ַ�ΪСд��ֻת��ǰlen���ַ���
���룺
string  -   ��Ҫת�����ַ���
len  -   �ַ���string�ĳ��ȣ��ڴ�ռ�ĳ��ȣ�
����� string -   ת������ַ���
����ֵ��
XSUCC  -   �ɹ�
XERROR   -   ʧ��
˵���� ��Ҫ�������˺����Ĳ���stringӦ���ǿ�д���ַ�����ռ䣬
lenΪstring�ĳ��ȣ���Ӧ�ñ�֤;
************************************************************************/
XS32 XOS_StrNtoLow( XU8 *string , XU32 len )
{
    XU32 i = 0;
    if(XNULLP == string)
    {
        return XERROR;
    }
    for( i = 0; i < len && string[i] != '\0' ; i++)
    {
        string[i] = XOS_ToLower(string[i]);
    }
    return XSUCC;
}


/************************************************************************
������: XOS_StrToUp
���ܣ� ת���ַ����е��ַ�Ϊ��д
���룺 string  -   ��Ҫת�����ַ���
����� string -   ת������ַ���
���أ�
XSUCC    -   �ɹ�
XERROR   -   ʧ��
˵����  ��Ҫ�������˺����Ĳ����������� char a[] = ""; ��������ʽ��Ӧ�ö���
************************************************************************/
XS32 XOS_StrToUp(XU8 string[] )
{
    XU32 i = 0;
    if(XNULLP == string)
    {
        return XERROR;
    }
    for( i = 0; i <= XOS_StrLen(string); i++)
    {
        string[i] = XOS_ToUpper(string[i]);
    }
    return XSUCC;
}


/************************************************************************
������: XOS_StrNtoUp
���ܣ� ת���ַ����е��ַ�Ϊ��д��ֻת��ǰlen���ַ���
���룺
string  -   ��Ҫת�����ַ���
len  -   �ַ���string�ĳ��ȣ��ڴ�ռ�ĳ��ȣ�
����� string -   ת������ַ���
����ֵ��
XSUCC  -   �ɹ�
XERROR   -   ʧ��
˵������Ҫ�������˺����Ĳ���stringӦ���ǿ�д���ַ�����ռ䣬
lenΪstring�ĳ��ȣ���Ӧ�ñ�֤;
************************************************************************/
XS32 XOS_StrNtoUp(XU8 *string , XU32 len )
{
    XU32 i = 0;
    if(XNULLP == string)
    {
        return XERROR;
    }
    for( i = 0; i < len && string[i] != '\0' ; i++)
    {
        string[i] = XOS_ToUpper(string[i]);
    }
    return XSUCC;
}


/************************************************************************
������: XOS_StrToNum
���ܣ��ַ���ת��Ϊ����
���룺
pStr  -   Ҫת�����ַ���
iValue - ���ص�����
�����iValue - ���ص�����
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����
************************************************************************/
XS32 XOS_StrToNum(XCHAR *pStr, XU32 *iValue)
{
    XS32    iOfs = 0, iBase;
    XS32    iFlg = 0;
    XCHAR  ucCh = 0;

    while(' ' == pStr[iOfs])
    {
        iOfs++;
    }

    if('0' > pStr[iOfs] || '9' < pStr[iOfs])
    {
        return(XERROR);
    }

    if('0' == pStr[iOfs])
    {
        switch(pStr[iOfs + 1])
        {
        case 'x':
        case 'X':       /*16����*/
            iBase = 16;
            iOfs += 2;
            break;

        case 'o':
        case 'O':       /*8����*/
            iBase = 8;
            iOfs += 2;
            break;

        case 'b':
        case 'B':       /* 2����*/
            iBase = 2;
            iOfs += 2;
            break;

        default:        /*10����*/
            iBase = 10;
            break;
        }
    }
    else
    {
        iBase = 10;
    }

    *iValue = 0;

    while('\0' != pStr[iOfs])
    {
        switch(iBase)
        {
        case 2:
            if('0' > pStr[iOfs] || '1' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 8:
            if('0' > pStr[iOfs] || '7' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 10:
            if('0' > pStr[iOfs] || '9' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 16:
            if('0' > pStr[iOfs] || '9' < pStr[iOfs])
            {
                if('a' > pStr[iOfs] || 'f' < pStr[iOfs])
                {
                    if('A' > pStr[iOfs] || 'F' < pStr[iOfs])
                    {
                        iFlg = 1;
                    }
                    else
                    {
                        ucCh = pStr[iOfs] - 'A';
                        ucCh += 10;
                    }
                }
                else
                {
                    ucCh = pStr[iOfs] - 'a';
                    ucCh += 10;
                }
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        default:
            iFlg = 1;
            break;
        }
        
        if(1 == iFlg)
        {
            break;
        }
        else
        {
            *iValue = *iValue * iBase + ucCh;
            iOfs ++;
        }
    }

    return(XSUCC);
}
/************************************************************************
������: XOS_StrToLongNum
���ܣ��ַ���ת��Ϊ64λ����
���룺
pStr  -   Ҫת�����ַ���
iValue - ���ص�����
�����iValue - ���ص�����
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����
************************************************************************/
XS32 XOS_StrToLongNum(XCHAR *pStr, XU64 *iValue)
{
    XS32    iOfs = 0, iBase;
    XS32    iFlg = 0;
    XCHAR  ucCh = 0;

    while(' ' == pStr[iOfs])
    {
        iOfs++;
    }

    if('0' > pStr[iOfs] || '9' < pStr[iOfs])
    {
        return(XERROR);
    }

    if('0' == pStr[iOfs])
    {
        switch(pStr[iOfs + 1])
        {
        case 'x':
        case 'X':       /*16����*/
            iBase = 16;
            iOfs += 2;
            break;

        case 'o':
        case 'O':       /*8����*/
            iBase = 8;
            iOfs += 2;
            break;

        case 'b':
        case 'B':       /* 2����*/
            iBase = 2;
            iOfs += 2;
            break;

        default:        /*10����*/
            iBase = 10;
            break;
        }
    }
    else
    {
        iBase = 10;
    }

    *iValue = 0;

    while('\0' != pStr[iOfs])
    {
        switch(iBase)
        {
        case 2:
            if('0' > pStr[iOfs] || '1' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 8:
            if('0' > pStr[iOfs] || '7' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 10:
            if('0' > pStr[iOfs] || '9' < pStr[iOfs])
            {
                iFlg = 1;
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        case 16:
            if('0' > pStr[iOfs] || '9' < pStr[iOfs])
            {
                if('a' > pStr[iOfs] || 'f' < pStr[iOfs])
                {
                    if('A' > pStr[iOfs] || 'F' < pStr[iOfs])
                    {
                        iFlg = 1;
                    }
                    else
                    {
                        ucCh = pStr[iOfs] - 'A';
                        ucCh += 10;
                    }
                }
                else
                {
                    ucCh = pStr[iOfs] - 'a';
                    ucCh += 10;
                }
            }
            else
            {
                ucCh = pStr[iOfs] - '0';
            }
            break;
        default:
            iFlg = 1;
            break;
        }
        
        if(1 == iFlg)
        {
            break;
        }
        else
        {
            *iValue = *iValue * iBase + ucCh;
            iOfs ++;
        }
    }

    return(XSUCC);
}


#ifdef XOS_VXWORKS
/************************************************************************
������: XOS_SetTime
���ܣ�vxWorks������ʱ��ĳ�ʼֵ
���룺pTime -   ʱ���ʼֵ
�����
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵��������vxWorks��ֻ�ܻ�ȡ1970��1��1���� ���� ����������ʱ��(����50��)��
������ҪӦ������һ����ʼֵ(����:07.1.18 15:36:58)������������ǰʱ�䣻
�����ǣ���ȡ��ʱ��ɾ�ȷ��10���룬���Գ�ʼ��ʱ���ܻ��м��ٺ������
���飺�˽ӿ�һ��ֻ�ǳ�ʼ����������ͬ�� ʱ��ʱ�ű�����
************************************************************************/
XS32 XOS_SetTime ( XOS_SERTIME_T  *pTime )
{
    t_XOSTT lt,ret,ret0;
    t_XOSTD *temp = XNULL;

    if(XNULL == pTime)
    {
        return XERROR;
    }
    /*�����ʼʱ��ֵ*/
    gSetTime.ucYear = pTime->ucYear;
    gSetTime.ucMonth = pTime->ucMonth;
    gSetTime.ucDay = pTime->ucDay;
    gSetTime.ucHour = pTime->ucHour;
    gSetTime.ucMinute = pTime->ucMinute;
    gSetTime.ucSecond = pTime->ucSecond;

    lt = time(XNULLP);
    ret = lt;
    ret0 = lt;

    if(XNULL == (temp = (t_XOSTD *)localtime((const time_t *)&lt)))
    {
        return XERROR;
    }

    /*������ĳ�ʼʱ�任���ϵͳʱ��ṹ*/
    temp->dt_year = gSetTime.ucYear+100+70 - temp->dt_year;
    temp->dt_mon =  gSetTime.ucMonth-1 - temp->dt_mon;
    temp->dt_mday =gSetTime.ucDay+1 - temp->dt_mday;
    temp->dt_hour = gSetTime.ucHour - temp->dt_hour;
    temp->dt_min = gSetTime.ucMinute -temp->dt_min;
    temp->dt_sec = gSetTime.ucSecond -temp->dt_sec;
    temp->dt_isdst = 0;

    if( (ret0 = mktime( (struct tm *)temp )) != (time_t)-1 )
    {
        gMsec = ret0 -ret;  /*�����ʼʱ�������ʱ��Ĳ�ֵ���룩*/
    }

    return XSUCC;
}
#endif


/************************************************************************
������: XOS_GetLocTime
���ܣ���ȡϵͳʱ��(����ʱ�䣬���ַ�������ʽ����)
���룺datet -   ��������ʱ���ִ����ַ�����
�����datet -   ��ȡ��ʱ���ִ�
���أ�
XSUCC -   �ɹ�
XERROR  -   ʧ��
˵�����������������ǳ���Ϊ30 ���ַ�����;
//vx����1970��1��1���� ���� ����������ʱ��(�����50��)
ע����������ȷ�ĳ�ʼʱ�䣬����Ի�ȡ��ǰ����ʵʱ��      
************************************************************************/
XS32 XOS_GetLocTime(XCHAR *datet)
{
    struct tm *ptr;
    time_t lt;

    lt = time(XNULLP);
#ifdef XOS_VXWORKS
    lt = lt + gMsec;
#endif    
    if(XNULL == (ptr = (struct tm *)localtime(&lt)))
    {
        return XERROR;
    }
    XOS_MemCpy(datet,asctime(ptr),XOS_StrLen(asctime(ptr))+1);
    return XSUCC;
}
/************************************************************************
������: XOS_GetLocTimeByPri
���ܣ�  ��ȡϵͳʱ��(����ʱ��(��������)�����ַ�������ʽ����)
���룺  pri ����ľ���
        pri ���ȷ�Χ0--6,Ϊ0ʱmSec����0��Ϊ1ʱmSec����һλֵ Ϊ2ʱmSec����2λֵ���Դ�����
�����  sec ������
        mSec ����
���أ�
XSUCC -   �ɹ�
XERROR  -   ʧ��
//vx����1970��1��1���� ���� ����������ʱ��(�����50��)
ע����������ȷ�ĳ�ʼʱ�䣬����Ի�ȡ��ǰ����ʵʱ��      
************************************************************************/
XS32 XOS_GetLocTimeByPri(XU32* sec, XU32* mSec, int pri)
{
    struct timeval tval;
    int i = 0;
    XU32 val = 0;
    XS32 mod = 1;

    memset(&tval, 0, sizeof(struct timeval));
    
    XOS_GetTimeOfDay(&tval, NULL);

    if(sec) {
        *sec = tval.tv_sec;
    }

    if(mSec) {
        *mSec = 0;
        if(pri >= MIN_PRI && pri <= MAX_PRI) {
            for(i = 0; i < (MAX_PRI - pri); i++) {
                mod *= 10;
            }
            val = tval.tv_usec / mod;
            *mSec= val;
        }
    }

#ifdef XOS_VXWORKS
    *sec = *sec + gMsec;
#endif

    return XSUCC;
}

/************************************************************************
������: XOS_GetTmTime
���ܣ���þֲ�ʱ�䡣�ú�������struct tm �Ľṹ��
���룺timet -   ����������ʱ��Ľṹ�����
�����timet -   ��õĵ�ǰʱ��
����ֵ��
XSUCC   -   �ɹ�
XERROR  -   ʧ��
˵����������õ���time() ���õ�time_t�ṹ������localtime ת��struct tm �ṹ
************************************************************************/
XS32 XOS_GetTmTime ( t_XOSTD *timet )
{
    t_XOSTT lt;
    t_XOSTD *temp = XNULL;

    if(XNULLP == timet)
    {
        return XERROR;
    }

    lt = time(XNULLP);
#ifdef XOS_VXWORKS
    lt = lt + gMsec;
#endif

    if(XNULL == (temp = (t_XOSTD *)localtime((const time_t *)&lt)))
    {
        return XERROR;
    }

    XOS_MemCpy(timet, temp, sizeof(t_XOSTD));
    //*timet = *temp;

    return XSUCC;
}


#ifndef XOS_VXWORKS
/************************************************************************
������: XOS_Clock
���ܣ���ȡ������ռ�õ�CPU�Ĵ�Լʱ��
���룺clockt    -   ����������ʱ��ı���
�����clockt    -   ��ȡ��ʱ��
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵���� clock��ô�ϵͳ�ӿ������������̵������е���clock()��
��ʱ֮���CPUʱ�Ӽ�ʱ��Ԫ(clock tick);
// ���õ���clock_t clock( void );��clock_tΪXU32��;
ע��This routine always returns -1 in VxWorks. VxWorks does not track per-task
time or system idle time. There is no method of determining how long a task
or the entire system has been doing work.       
************************************************************************/
XS32 XOS_Clock(t_XOSCT *clockt)
{
    if(XNULLP == clockt)
    {
        return XERROR;
    }

    if((t_XOSCT)(-1) == (*clockt = (t_XOSCT)clock()))
    {
        return XERROR;
    }
    return XSUCC;
}
#else
XS32 XOS_Clock(t_XOSCT *clockt)
{
    if(XNULLP == clockt)
    {
        return XERROR;
    }

    if((t_XOSCT)(-1) == (*clockt = (t_XOSCT)tickGet()))
    {
        return XERROR;
    }
    return XSUCC;
}
#endif


/************************************************************************
������: XOS_Time
���ܣ���ȡ�ӹ�Ԫ1970��1��1�յ�UTCʱ���0ʱ0��0����������������������
���룺times -   ����������ʱ�䣨������
�����times -   ��õĵ�ǰʱ�䣨������
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����������õ���time();
************************************************************************/
XS32 XOS_Time ( t_XOSTT *times )
{
    if(XNULLP == times)
    {
        return XERROR;
    }

    time((time_t *)times);
#ifdef XOS_VXWORKS
    *times = *times + gMsec;
#endif

    return XSUCC;
}


/************************************************************************
������: XOS_MkTime
���ܣ���ʱ���ʽ��struct tm ת��Ϊtime_t
���룺timetm    -   Ҫת����struct tm�ṹ��ʱ��
�����times -   ת�����time_t��ʽ��ʱ��
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����
************************************************************************/
XS32 XOS_MkTime(t_XOSTD *timetm , t_XOSTT *times)
{
    if(XNULLP == times || XNULLP == timetm)
    {
        return XERROR;
    }

    if((t_XOSTT)(-1) == (*times = (t_XOSTT)mktime((struct tm *)timetm)))
    {
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������: XOS_LocalTime
���ܣ���ʱ���ʽ��time_t ת��Ϊstruct tm
���룺times -   Ҫת����time_t�ṹ��ʱ��
�����timetm    -   ת����struct tm �ṹ��ʱ��
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����
************************************************************************/
XS32 XOS_LocalTime(XCONST t_XOSTT *times  , t_XOSTD *timetm)
{
    t_XOSTD *temp = XNULL;
    if(XNULLP == times || XNULLP == timetm)
    {
        return XERROR;
    }

    if(XNULL == (temp = (t_XOSTD *)localtime((const time_t *)times)))
    {
        return XERROR;
    }

    *timetm = *temp;
    return XSUCC;
}

extern XU32 g_startSec;
XU32 XOS_GetRunTime(void)
{
    XS32 ret = XOS_TicksToSec(XOS_GetSysTicks()) - g_startSec;

    return (ret < 0) ? 0 : ret;
}

/************************************************************************
������: XOS_GetSysTicks
���ܣ���ȡʱ��ticks
���룺��
����� 
���أ�
XSUCC  -   �ɹ�
XERROR -   ʧ��
˵����
************************************************************************/
XU32 XOS_GetSysTicks(void)
{
#ifdef XOS_WIN32     
    return GetTickCount();
#endif

#ifdef XOS_LINUX
    return  times(NULL);
#endif

#ifdef XOS_VXWORKS
    return tickGet();
#endif

    return 0;
}

extern int sysClkRateGet (void);
/************************************************************************
������: XOS_GetSysTicks
���ܣ���ticksת������
���룺��
����� 
���أ�
XSUCC  -   �ɹ�
XERROR -   ʧ��
˵����
************************************************************************/
XU32 XOS_TicksToSec(XU32 ulTicks)
{
    XU32 ulSeconds = 0;
#ifdef XOS_LINUX
    ulSeconds = ulTicks/sysconf(_SC_CLK_TCK);
#endif

#ifdef XOS_WIN32
    ulSeconds = ulTicks/1000;
#endif

#ifdef XOS_VXWORKS
    ulSeconds = ulTicks/sysClkRateGet();
#endif
    return ulSeconds;
}
/************************************************************************
������: XOS_TicksToMsec
���ܣ���ticksת���ɺ���
���룺��
����� 
���أ�
XSUCC  -   �ɹ�
XERROR -   ʧ��
˵����
************************************************************************/
XU64 XOS_TicksToMsec(XU32 ulTicks)
{
    XU64 uSec = 0;
    XU64 umSec = 0;
    XU64 remainder = 0;
    XU32 ticks_per_sec = 0;
    #define MSEC_PER_SEC   (1000)  /* 1s �ж���ms */
    
#ifdef XOS_LINUX
    ticks_per_sec = sysconf(_SC_CLK_TCK);
#endif

#ifdef XOS_WIN32
    ticks_per_sec = 1000;
#endif

#ifdef XOS_VXWORKS
    ticks_per_sec = sysClkRateGet();
#endif

    uSec = ulTicks / ticks_per_sec;
    remainder = ulTicks % ticks_per_sec;
    umSec = remainder * MSEC_PER_SEC / ticks_per_sec; 

    umSec = uSec*1000 + umSec;

    return umSec;
}

/************************************************************************
������: XOS_TicksToSecByPri
���ܣ���ticksת������,���ݾ���λ���������ľ���
���룺
    ulTicks   -   ָ����tick����
����� 
    sec       -   ������
    mSec      -   ����
    pri       -   ����ľ���
    pri ���ȷ�Χ0--6,Ϊ0ʱmSec����0��1ʱmSec����һλֵ 2ʱmSec����2λֵ���Դ�����

���أ�
XSUCC  -   �ɹ�
XERROR -   ʧ��
˵����
************************************************************************/
XU32 XOS_TicksToSecByPri(XU32 ulTicks, XU32* sec, XU32* mSec, XS32 pri)
{
    XU32 uSec = 0;
    XU32 umSec = 0;
    XS32 mod = 1;
    XU32 val = 0;
    XS32 num = 0;
    XS32 i = 0;
#ifdef XOS_LINUX
    uSec = ulTicks / sysconf(_SC_CLK_TCK);
    umSec = ulTicks % sysconf(_SC_CLK_TCK); 
#endif

#ifdef XOS_WIN32
    uSec = ulTicks / 1000;
    umSec = ulTicks % 1000; 
#endif

#ifdef XOS_VXWORKS
    uSec = ulTicks / sysClkRateGet();
    umSec = ulTicks % sysClkRateGet(); 
#endif
    if(sec) {
        *sec = (XU32)uSec;
    }
    if(mSec) {
        *mSec = 0;
        if(pri >= MIN_PRI && pri <= MAX_PRI) {
            val = umSec;
            for(num = 0; val != 0; num++, val /= 10){}
            
            for(i = 0; i < (num - pri); i++) {
                mod *= 10;
            }
            val = umSec / mod;
            *mSec= val;
        }
    }
    return XSUCC;
}

/************************************************************************
������: XOS_Rand
���ܣ���ȡһ�������
���룺
seed  -   ���������������
gnum    -   ���ɵ������
�����gnum    -   ���ɵ������
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����seed �������޷�������
************************************************************************/
XU32 XOS_Rand(XU32 seed , XS32 *gnum)
{
    if(XNULL != seed)
        srand(seed);
    *gnum = rand();
    return XSUCC;
}


/************************************************************************
������: XOS_GetTimeOfDay
���ܣ���ȡϵͳ��ǰʱ�䡣�ú�������struct timeval�Ľṹ�塣
���룺  
�����
    tv      -   ʱ��ṹ��timeval
    tz      -   ʱ����ʵ����linuxϵͳ���Ѿ�������windows��vxworks��û��ʹ�ô�
    ������ʹ��ʱ����NULL����
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵����
************************************************************************/
XS32 XOS_GetTimeOfDay(struct timeval *tv, void *tz)
{
    int ret = XSUCC;
    
#ifdef XOS_WIN32
    SYSTEMTIME st;
    FILETIME ft;
    ULARGE_INTEGER ui;
#endif

#ifdef XOS_VXWORKS
    struct timespec tp;
#endif

#ifdef XOS_WIN32
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    ui.QuadPart -= FILETIME_TO_TIMEVAL_SKEW;
    tv->tv_sec = (long)(ui.QuadPart / (10000 * 1000));
    tv->tv_usec = (long)((ui.QuadPart % (10000 * 1000)) / 10);
#endif

#ifdef XOS_LINUX
    gettimeofday(tv, tz);
#endif

#ifdef XOS_VXWORKS
    if  ( (ret=clock_gettime(CLOCK_REALTIME, &tp))==0)  {
        tv->tv_sec  = tp.tv_sec;
        tv->tv_usec = (tp.tv_nsec + 500) / 1000;  //������ת��΢��
    }
#endif    
    return ret;
}

/************************************************************************
������: XOS_VsPrintf 
���ܣ� ���ɱ�������ַ�����ʽ��������д�����󳤶�ֵ
�����maxlen-1���ȵ���Ϣ��(������'\0')д�뵽buffer��,���һ���ֽ�����Ϊ��\0��
����:
buffer--�洢�����ַ�����ָ��
maxlen--���ɴ洢���ַ�������ֵ
format--�ɱ�����ĸ�ʽ��
ap --va_list�����ַ�����
�����
����ֵ���ɹ�����buffer�б���ĵ�����ַ���(������'\0')
        �쳣���ظ�ֵ

˵����maxlen����������buffer��д�ĳ���һ�£����������ڴ��쳣
************************************************************************/
XS32 XOS_VsPrintf (XCHAR *buffer, XU32 Maxlen, const XCHAR *format, va_list ap)
{
    XS32 length;

    /*
    va_list ap;
    va_start( ap, format );
    */
    if ( XNULL == buffer || XNULL == format )
    {
        return XNULL;
    }
    /*��maxlen����0�жϣ���Ϊ���洦��ɱ����ʱ��maxlen��Ҫ����-1�����*/
    if ( 0 == Maxlen )
    {
        return XNULL;
    }

    //#if ( XOS_WIN32  )
#ifdef XOS_WIN32
    length =  _vsnprintf(buffer,Maxlen,format, ap);
#endif
    //#if ( XOS_LINUX  || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )
    length = vsnprintf(buffer, Maxlen,format, ap);
#endif
    //#if ( XOS_VXWORKS || XOS_VTA )
#if ( defined(XOS_VXWORKS) || defined(XOS_VTA) )
    length = vx_vsnprintf(buffer,Maxlen,format,ap);
#endif

    /*va_end( ap );*/
    if(length < 0) /* ��linux��libc2.1���£��������strlen >= maxlenʱ������-1��2.1֮�ϵķ����������str��len*/
    {
    #ifdef XOS_WIN32   /* �������str��len > maxlenʱ��win32 ����-1���ڴ˸���Ϊ�����һ���ֽ��滻Ϊ'\0',����buf�ַ���ʵ�ʳ��� */
        buffer[Maxlen-1] = '\0';
        length = strlen(buffer);
    #endif
        return length;
    }   

    if((XU32)length >= Maxlen)
    {
    #ifdef XOS_WIN32  /* ����ֻ�����len == maxlen����������len > maxlen��_vsnprintf����-1�������ߵ����� */
        buffer[Maxlen-1] = '\0';        
    #endif
        length = strlen(buffer);
    }

    return length;
}


#if 0
/************************************************************************
������: XOS_Randn
���룺num   -   ���������ȡ�������
�����num   -   ���ɵ������
���أ�
XSUCC -   �ɹ�
XERROR    -   ʧ��
˵�����û�ȡ��ǰʱ������Ϊseed
************************************************************************/
XS32 XOS_Randn(XU32 *num)
{
    if(num == XNULL)
    {
        return XERROR;
    }

    *num = rand();
    return XSUCC;
}
#endif



#ifdef __cplusplus
}
#endif /* __cplusplus */

