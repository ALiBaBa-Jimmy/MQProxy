/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosos.c
**
**  description:  os封装的公共文件
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
函数名: XOS_GetFileLen
功能： 获取文件的长度
输入： filename  -   要读取文件的相对路径名,例如/G711/tone.g729
输出： resultlen -   文件的实际长度
返回：
XSUCC    -   成功
XERROR   -   失败
说明：
************************************************************************/
XS32 XOS_GetFileLen(XU8 *filename ,XU32 *length)
{
#ifdef XOS_WIN32
    struct _stat fbuf;
#else
    struct stat fbuf; 
#endif

    /*取得文件大小,以字节计算*/
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
函数名: XOS_ReadVoxFile
功能： 从文件中读取数据
输入： filename  -   要读取文件的相对路径名,例如/G711/tone.g729
输出：
result   -   用来保存文件内容的数据空间
resultlen -   文件的实际长度
返回：
XSUCC    -   成功
XERROR   -   失败
说明：这里result的空间由应用提供，约定一个最大值
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

    /*取得文件大小,以字节计算*/
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
函数名: XOS_RemoveFile
功能： 删除文件
输入： filename  -   相对路径名,例如/G711/tone.g729
输出： N/A
返回：
XSUCC    -   成功
XERROR   -   失败
说明： 在删除文件前，要先把所有用到此文件的句柄都关闭
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
函数名: XOS_StrNChr
功能： 在字符串string中搜索字符ch第一次出现的位置，搜索到string中第len个字符止
输入：
string    -   待搜索的字符串
ch    -   查找的字符（建议以'@'的方式输入）
len   -   字符串string的长度
输出：
返回值：
成功    -   指向第一个ch在string的位置指针，
失败    -   返回XNULL表示未找到
说明：1~255是标准字符库，各系统可能(可以)有自己定义的字符；
比如Windows下大于30000时又有对应的字符；
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
    if( *(string+len-1) != '\0')    /*也许改成strncpy更好*/
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
函数名: XOS_StrNStr
功能： 得到在stack中第一次包含needle字符串的位置，，搜索到str中第len个字符止
输入：
stack -   目标字符串
needle        -   要查找的字符串
stacklen  -   目标串stack的长度
ndlen -   待查找串needle的长度
输出：N/A
返回值：
成功    -   返回在stack中第一次包含needle字符串的位置指针,
失败    -   返回XNULL表示未找到

说明：这里result的空间由应用提供，约定一个最大值
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
函数名: XOS_StrToLow
功能： 转换字符串中的字符为小写
输入： string  -   需要转换的字符串
输出： string -   转换后的字符串
返回：
XSUCC    -   成功
XERROR   -   失败
说明：  重要申明，此函数的参数必须是以 char a[] = ""; 的数组形式由应用定义
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
函数名: XOS_StrNtoLow
功能： 转换字符串中的字符为小写（只转换前len个字符）
输入：
string  -   需要转换的字符串
len  -   字符串string的长度（内存空间的长度）
输出： string -   转换后的字符串
返回值：
XSUCC  -   成功
XERROR   -   失败
说明： 重要申明，此函数的参数string应该是可写的字符数组空间，
len为string的长度，由应用保证;
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
函数名: XOS_StrToUp
功能： 转换字符串中的字符为大写
输入： string  -   需要转换的字符串
输出： string -   转换后的字符串
返回：
XSUCC    -   成功
XERROR   -   失败
说明：  重要申明，此函数的参数必须是以 char a[] = ""; 的数组形式由应用定义
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
函数名: XOS_StrNtoUp
功能： 转换字符串中的字符为大写（只转换前len个字符）
输入：
string  -   需要转换的字符串
len  -   字符串string的长度（内存空间的长度）
输出： string -   转换后的字符串
返回值：
XSUCC  -   成功
XERROR   -   失败
说明：重要申明，此函数的参数string应该是可写的字符数组空间，
len为string的长度，由应用保证;
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
函数名: XOS_StrToNum
功能：字符串转换为数字
输入：
pStr  -   要转换的字符串
iValue - 返回的数字
输出：iValue - 返回的数字
返回：
XSUCC -   成功
XERROR    -   失败
说明：
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
        case 'X':       /*16进制*/
            iBase = 16;
            iOfs += 2;
            break;

        case 'o':
        case 'O':       /*8进制*/
            iBase = 8;
            iOfs += 2;
            break;

        case 'b':
        case 'B':       /* 2进制*/
            iBase = 2;
            iOfs += 2;
            break;

        default:        /*10进制*/
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
函数名: XOS_StrToLongNum
功能：字符串转换为64位数字
输入：
pStr  -   要转换的字符串
iValue - 返回的数字
输出：iValue - 返回的数字
返回：
XSUCC -   成功
XERROR    -   失败
说明：
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
        case 'X':       /*16进制*/
            iBase = 16;
            iOfs += 2;
            break;

        case 'o':
        case 'O':       /*8进制*/
            iBase = 8;
            iOfs += 2;
            break;

        case 'b':
        case 'B':       /* 2进制*/
            iBase = 2;
            iOfs += 2;
            break;

        default:        /*10进制*/
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
函数名: XOS_SetTime
功能：vxWorks下设置时间的初始值
输入：pTime -   时间初始值
输出：
返回：
XSUCC -   成功
XERROR    -   失败
说明：由于vxWorks下只能获取1970年1月1日起 加上 板子起来的时长(比如50秒)，
所以需要应用设置一个初始值(比如:07.1.18 15:36:58)以用来调整当前时间；
（但是，获取的时间可精确至10毫秒，所以初始化时可能会有几百毫秒的误差）
建议：此接口一般只是初始化或是网管同步 时间时才被调用
************************************************************************/
XS32 XOS_SetTime ( XOS_SERTIME_T  *pTime )
{
    t_XOSTT lt,ret,ret0;
    t_XOSTD *temp = XNULL;

    if(XNULL == pTime)
    {
        return XERROR;
    }
    /*保存初始时间值*/
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

    /*将传入的初始时间换算成系统时间结构*/
    temp->dt_year = gSetTime.ucYear+100+70 - temp->dt_year;
    temp->dt_mon =  gSetTime.ucMonth-1 - temp->dt_mon;
    temp->dt_mday =gSetTime.ucDay+1 - temp->dt_mday;
    temp->dt_hour = gSetTime.ucHour - temp->dt_hour;
    temp->dt_min = gSetTime.ucMinute -temp->dt_min;
    temp->dt_sec = gSetTime.ucSecond -temp->dt_sec;
    temp->dt_isdst = 0;

    if( (ret0 = mktime( (struct tm *)temp )) != (time_t)-1 )
    {
        gMsec = ret0 -ret;  /*计算初始时间与板子时间的差值（秒）*/
    }

    return XSUCC;
}
#endif


/************************************************************************
函数名: XOS_GetLocTime
功能：获取系统时间(本地时间，以字符串的形式返回)
输入：datet -   用来保存时间字串的字符数组
输出：datet -   获取的时间字串
返回：
XSUCC -   成功
XERROR  -   失败
说明：这里的输入参数是长度为30 的字符数组;
//vx下是1970年1月1日起 加上 板子起来的时长(比如加50秒)
注：若传入正确的初始时间，则可以获取当前的真实时间      
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
函数名: XOS_GetLocTimeByPri
功能：  获取系统时间(本地时间(包括毫秒)，以字符串的形式返回)
输入：  pri 毫秒的精度
        pri 精度范围0--6,为0时mSec返回0，为1时mSec返回一位值 为2时mSec返回2位值，以此类推
输出：  sec 返回秒
        mSec 毫秒
返回：
XSUCC -   成功
XERROR  -   失败
//vx下是1970年1月1日起 加上 板子起来的时长(比如加50秒)
注：若传入正确的初始时间，则可以获取当前的真实时间      
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
函数名: XOS_GetTmTime
功能：获得局部时间。该函数返回struct tm 的结构体
输入：timet -   用来保存获得时间的结构体变量
输出：timet -   获得的当前时间
返回值：
XSUCC   -   成功
XERROR  -   失败
说明：这里调用的是time() ，得到time_t结构后再用localtime 转成struct tm 结构
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
函数名: XOS_Clock
功能：获取进程所占用的CPU的大约时间
输入：clockt    -   用来保存获得时间的变量
输出：clockt    -   获取的时间
返回：
XSUCC -   成功
XERROR    -   失败
说明： clock获得从系统从开启这个程序进程到程序中调用clock()函
数时之间的CPU时钟计时单元(clock tick);
// 调用的是clock_t clock( void );，clock_t为XU32型;
注：This routine always returns -1 in VxWorks. VxWorks does not track per-task
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
函数名: XOS_Time
功能：获取从公元1970年1月1日的UTC时间从0时0分0秒算起到现在所经过的秒数
输入：times -   用来保存获得时间（秒数）
输出：times -   获得的当前时间（秒数）
返回：
XSUCC -   成功
XERROR    -   失败
说明：这里调用的是time();
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
函数名: XOS_MkTime
功能：将时间格式由struct tm 转换为time_t
输入：timetm    -   要转换的struct tm结构的时间
输出：times -   转换后的time_t格式的时间
返回：
XSUCC -   成功
XERROR    -   失败
说明：
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
函数名: XOS_LocalTime
功能：将时间格式由time_t 转化为struct tm
输入：times -   要转换的time_t结构的时间
输出：timetm    -   转换后struct tm 结构的时间
返回：
XSUCC -   成功
XERROR    -   失败
说明：
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
函数名: XOS_GetSysTicks
功能：获取时钟ticks
输入：无
输出： 
返回：
XSUCC  -   成功
XERROR -   失败
说明：
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
函数名: XOS_GetSysTicks
功能：将ticks转换成秒
输入：无
输出： 
返回：
XSUCC  -   成功
XERROR -   失败
说明：
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
函数名: XOS_TicksToMsec
功能：将ticks转换成毫秒
输入：无
输出： 
返回：
XSUCC  -   成功
XERROR -   失败
说明：
************************************************************************/
XU64 XOS_TicksToMsec(XU32 ulTicks)
{
    XU64 uSec = 0;
    XU64 umSec = 0;
    XU64 remainder = 0;
    XU32 ticks_per_sec = 0;
    #define MSEC_PER_SEC   (1000)  /* 1s 有多少ms */
    
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
函数名: XOS_TicksToSecByPri
功能：将ticks转换成秒,根据精度位数，输出秒的精度
输入：
    ulTicks   -   指定的tick数量
输出： 
    sec       -   返回秒
    mSec      -   毫秒
    pri       -   毫秒的精度
    pri 精度范围0--6,为0时mSec返回0，1时mSec返回一位值 2时mSec返回2位值，以此类推

返回：
XSUCC  -   成功
XERROR -   失败
说明：
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
函数名: XOS_Rand
功能：获取一个随机数
输入：
seed  -   生成随机数的种子
gnum    -   生成的随机数
输出：gnum    -   生成的随机数
返回：
XSUCC -   成功
XERROR    -   失败
说明：seed 必须是无符号整数
************************************************************************/
XU32 XOS_Rand(XU32 seed , XS32 *gnum)
{
    if(XNULL != seed)
        srand(seed);
    *gnum = rand();
    return XSUCC;
}


/************************************************************************
函数名: XOS_GetTimeOfDay
功能：获取系统当前时间。该函数返回struct timeval的结构体。
输入：  
输出：
    tv      -   时间结构体timeval
    tz      -   时区，实际在linux系统中已经废弃，windows和vxworks中没有使用此
    参数，使用时传递NULL即可
返回：
XSUCC -   成功
XERROR    -   失败
说明：
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
        tv->tv_usec = (tp.tv_nsec + 500) / 1000;  //将纳秒转成微妙
    }
#endif    
    return ret;
}

/************************************************************************
函数名: XOS_VsPrintf 
功能： 将可变参数的字符串格式化，根据写入的最大长度值
将最大maxlen-1长度的信息串(不包括'\0')写入到buffer中,最后一个字节总是为‘\0’
输入:
buffer--存储最终字符串的指针
maxlen--最大可存储的字符串长度值
format--可变参数的格式串
ap --va_list处理字符类型
输出：
返回值：成功返回buffer中保存的的最大字符数(不包括'\0')
        异常返回负值

说明：maxlen参数必须与buffer可写的长度一致，否则会造成内存异常
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
    /*对maxlen进行0判断，因为下面处理可变参数时，maxlen是要进行-1处理的*/
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
    if(length < 0) /* 在linux下libc2.1以下，当输入的strlen >= maxlen时，返回-1。2.1之上的返回你输入的str的len*/
    {
    #ifdef XOS_WIN32   /* 当输入的str的len > maxlen时，win32 返回-1，在此改正为，最后一个字节替换为'\0',返回buf字符串实际长度 */
        buffer[Maxlen-1] = '\0';
        length = strlen(buffer);
    #endif
        return length;
    }   

    if((XU32)length >= Maxlen)
    {
    #ifdef XOS_WIN32  /* 这里只会出现len == maxlen的情况，如果len > maxlen，_vsnprintf返回-1，不会走到这里 */
        buffer[Maxlen-1] = '\0';        
    #endif
        length = strlen(buffer);
    }

    return length;
}


#if 0
/************************************************************************
函数名: XOS_Randn
输入：num   -   用来保存获取的随机数
输出：num   -   生成的随机数
返回：
XSUCC -   成功
XERROR    -   失败
说明：用获取当前时间来做为seed
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

