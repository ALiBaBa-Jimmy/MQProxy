/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRCommFun.cpp
* Author:       luhaiyan
* Date:					10-12-2014
* OverView:     SPR的公共函数，需要包含的各个项目中进行编译
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#include "SPRCommFun.h"

#include "SPRTaskCommFun.h"
#include <math.h>


#define MAX_DIGITAL_LEN             33          //电话
#define DIGITAL_BUFF_LEN            32          /*号码缓冲区长度*/
#define BCD_NUM_MASK                0x0f


/**************************************************************************
函 数 名: GetCurSysTime
函数功能:  获取系统时间
参    数:  XS8 *pTimeStr 要返回的时间串
返 回 值:  XS32 结果
**************************************************************************/
XS32 GetCurSysTime(XS8 *pTimeStr)
{
    t_XOSTT            timeT;
    t_XOSTD            timeTM;
    XS32               procRes;
    XU8                tempBuffer[256];

    if (XNULLP == pTimeStr)
    {
        return XERROR;
    }
    
    XOS_Time(&timeT);
    procRes = XOS_LocalTime(&timeT, &timeTM);
    if (XSUCC != procRes)
    {
        return XERROR;
    }


    //XOS_Sprintf((XS8*)tempBuffer, 256, "%04d%02d%02d%02d%02d%02d", 
	XOS_Sprintf((XS8*)tempBuffer, 256, "%04d-%02d", 
                timeTM.dt_year + 1900,
                timeTM.dt_mon + 1);
    XOS_MemCpy(pTimeStr, tempBuffer, LENGTH_OF_DATE);

    return XSUCC;
}


/************************************************************************
   功能描述：把 十六进制 转为 字符串
示例：
    输入：0x61，0x62，0x63，0x64
	输出：ab，cd
                                                                  
/************************************************************************/
XU64 USER_HexToStr(XU8 *pHex,XU8 * pStr,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
		pStr[i] = USER_ChrToHex(pHex[2*i]) * 16 + USER_ChrToHex(pHex[2*i+1]);
	}

	return XSUCC;
}

/***********************************
功能描述：把ASCII字符转为十六进制
示例：
     输入 0x61
	 输出 0x0a
************************************/
XU8 USER_ChrToHex(XU8 chr)
{
	XU8 temp1[17]="0123456789abcdef";
	XU8 temp2[7]="ABCDEF";
	XU8 i;

	for (i=0; i< 16;i++)
	{
		if (chr == temp1[i])
			return i;
	}
	for (i=0; i< 6;i++)
	{
		if (chr == temp2[i])
			return 10+i;
	}

	return -1;
}

/************************************************************************
   功能描述：把 字符串 转为 十六进制
示例：
    输入：ab，cd
	输出：0x61，0x62，0x63，0x64
                                                                  
/************************************************************************/
XU64 USER_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen)
{
	XU64 i;

	for (i=0; i< ulLen;i++)
	{
		pHex[2*i]   = ( pStr[i] / 16 > 9) ?(87 + pStr[i] / 16 ):(48 + pStr[i] / 16 );
		pHex[2*i+1] = ( pStr[i] % 16 > 9) ?(87 + pStr[i] % 16 ):(48 + pStr[i] % 16 );
	}

	return XSUCC;
}


/************************************************************************
   功能描述：把XU64类型的主机字节序（Little-Endian）转换为网络字节序（Big-Endian）
   输入：XU64 val：待转换的XU64类型数值
		 XBOOL BYTE_ORDER：本地计算机的字节序，Little-Endian为0，Big-Endian为1
   输出：转换字节序后的数值
示例：
    输入：0x7654321087654321LL
	输出：0x2143658710325476LL
                                                                  
/************************************************************************/
XU64 htonll(XU64 val, XBOOL byteOrder)
{
     if (!byteOrder)
     {
         return (((XU64 )htonl((XS32)((val << 32) >> 32))) << 32) | (XU32)htonl((XS32)(val >> 32));
     }
     else
     {
         return val;  
     }
 }

/************************************************************************
   功能描述：把XU64类型的网络字节序（Big-Endian）转换为主机字节序（Little-Endian）
   输入：XU64 val：待转换的XU64类型数值
		 XBOOL BYTE_ORDER：本地计算机的字节序，Little-Endian为0，Big-Endian为1
   输出：转换字节序后的数值
示例：
    输入：0x2143658710325476LL
	输出：0x7654321087654321LL
                                                                  
/************************************************************************/
XU64 ntohll(XU64 val, XBOOL byteOrder)
{
     if (!byteOrder)
     {
         return (((XU64 )htonl((XS32)((val << 32) >> 32))) << 32) | (XU32)htonl((XS32)(val >> 32));
     }
     else 
     {
         return val;  
      }
 }
XS32 User_StrtoInt(XS8 *pstr, XS32 len)
{
	if (len >8)
	{
		len = 8;
	}
	int Return_val = 0;
	for (int i=len-1;i>=0;--i)
	{
		if(*(pstr+i)>='a' && *(pstr+i)<='f')
		{
			Return_val = Return_val+((XS32)pow(16.0, len-1-i))*(*(pstr+i)-0x57);
		}
		else if (*(pstr+i)>='A' && *(pstr+i)<='F')
		{
			Return_val = Return_val+((XS32)pow(16.0, len-1-i))*(*(pstr+i)-0x37);
		}
		else if (*(pstr+i)>='0' && *(pstr+i)<='9')
		{
			Return_val = Return_val+((XS32)pow(16.0, len-1-i))*(*(pstr+i)-0x30);
		}
		else
		{
			return Return_val;
		}
	}

	return Return_val;

}

/************************************************************************
 函数名:ReverseBcd
 功能:BCD码反转
 输入:
		XU8* strBcd：BCD串
		XU64 length：BCD码的长度	
 输出:	XU8* strRevBcd：翻转后的BCD码
 返回:
 说明:	"10086"的BCD码流strBcd为0x0180f600000000000000000000000000
		length 为3
		 计算后的strRevBcd 为   0x10086f00000000000000000000000000
************************************************************************/
XS64  ReverseBcd(XU8* strBcd, XU64 length, XU8* strRevBcd)
{
	XU8 ch1 = 0x00, ch2 = 0x00, ch = 0x00;
	XU64 i = 0;
	if(NULL== strBcd || NULL == strRevBcd)
	{
		return XERROR;
	}

	for(i = 0; i < length; i++)
	{
		ch  = strBcd[i];
		ch1 = ch >> 4;
		ch2 = ch << 4;		
		strRevBcd[i] = ch1 + ch2;
	}

	return XSUCC;
}

/********************************** 
函数名称    : NumCvt2Bcd
功能描述    : 把字符串格式的号码转换到BCD码格式
参数        : XCHAR *pStr 输入, 字符串格式
参数        : XU8 *pBcd 输出, BCD码格式号码
参数        : XU8 *pLen 输入, 号码个数
返回值      : XS32: BCD码长度：XERROR表示错误
例如:
        "33" => 0x33 
        "34" => 0x43
        "37" => 0x73
        "3468" => 0x4386
        "3768" => 0x7386

		输入pStr “10086”
		输出pBcd  0x0180f600000000000000000000000000
		 
************************************/
XS32 NumCvt2Bcd(XU8 *pStr, XU8 *pBcd)
{   
    XU8 ucIdx = 0;
    XU8 ucBuf = 0;       /*BCD缓冲区索引*/
    XU8 ucNum = 0;       /*取出的号码*/
    XU8 ucMaxLen = 0;    /*实际取的长度*/

    /*参数合法性检查*/
    if( NULL == pStr || NULL == pBcd )
    {
        return XERROR;
    }

    ucMaxLen = (XU8)(strlen((const char*)pStr));
    if( MAX_DIGITAL_LEN <= ucMaxLen )
    {
        ucMaxLen = MAX_DIGITAL_LEN - 1;
    }

    ucBuf = 0;
    for( ucIdx=0; ucMaxLen>ucIdx; ++ucIdx )
    {
        if(('f' == pStr[ucIdx])||('F' == pStr[ucIdx]))
        {
            ucNum = 0x0F;
        }
        else
        {
            ucNum = (XU8)(pStr[ucIdx] - '0');
        }
        /*ucIdx为偶数时取低四位, 为奇数时取高四位*/
        if( 0 == (ucIdx & 0x01) )
        {
            pBcd[ucBuf] = ((BCD_NUM_MASK << 4) | (ucNum & BCD_NUM_MASK));
        }
        else
        {
            pBcd[ucBuf] &= (BCD_NUM_MASK);
            pBcd[ucBuf] |= ((ucNum & BCD_NUM_MASK) << 4);
            //XOS_Trace(MD(FID_SAAP,PL_DBG), "SAAP_NumCvt2Bcd>  %0x", pBcd[ucBuf]);
            ++ucBuf;
            if( DIGITAL_BUFF_LEN <= ucBuf )
            {
                break;
            }
        }
    }

	XS32 nBcdLen = ((ucIdx%2)==0)?(ucIdx/2):((ucIdx/2)+1);
  
    //XOS_Trace(MD(FID_SAAP,PL_DBG), "exit SAAP_NumCvt2Bcd>");
    return nBcdLen;
}

/********************************** 
函数名称    : BcdCvt2Num
功能描述    : 把BCD码格式转换到字符串格式的号码
参数        : XCHAR *pBcd 输入, BCD码格式号码		   
			: XU8 *pLen 输入, 号码个数
输出        : XU8 *strTel 输出, 字符串格式
返回值      : XS64 
例如:   		
		输入pBcd  0x0180f600
		输出strTel “10086”
************************************/
XS64 BcdCvt2Num(XU8* pBcd, XU64 length, XU8* pStr)
{
	char reverBcd[16] ={0};
	memset(reverBcd, 0xff, 16);
	//翻转bcd
	ReverseBcd((XU8*)pBcd, length,(XU8*)reverBcd);
	USER_StrToHex((XU8*)reverBcd,(XU8*)pStr,16);
	for(int i =0; i<MAX_DIGITAL_LEN; i++)
	{
		if((pStr[i]== 'f') ||(pStr[i]== 'F'))
		{
			pStr[i] =0;
		}
	}

	return 0;

}


/********************************** 
函数名称    : IpAddrHex2Str
功能描述    : 把IpV4地址由十六机制码流，转化为类似172.16.8.237的格式
参数        : XCHAR *IpHex 输入, 码流格式		   
			: XU8 *pLen 输入, 号码个数
输出        : XU8 *IpStr 输出, 字符串形式，输入参数为 16长度的字符数组
返回值      : XS64 
例如:   		
************************************/
XS64 IpAddrHex2Str(XU8* IpHex,XU8* IpStr)
{
	XS8 tempIp[16] ={0};
	for(int i =0; i<4; i++)//默认IpHex长度为4
	{
		if (i == 0)
		{
			sprintf(tempIp, "%d", IpHex[i]);
		}
		else
		{
			sprintf(tempIp, "%s.%d", tempIp, IpHex[i]);
		}
		
	}

	memcpy(IpStr, tempIp, sizeof(tempIp));

	return 0;

}
/********************************** 
函数名称    : printMsgBuff
功能描述    : 打印消息
参数        : XU8* pOutBuf, ：输出buf六
              XU32 nOutLen： 输出消息外面传递过来的buf的长度
			  XU8* pInBuf：输入消息 
			  XU32 nInLen ：输入消息长度	   
			: 
返回值      : XVOID 
例如:   		
************************************/
XVOID PrintMsgBuff(XU8* pOutBuf, XU32 nOutLen,XU8* pInBuf, XU32 nInLen )
{

	int j = 0;
	for(XU32 i=0; i<nInLen; i++)
	{
		XS8 temp[4] ={0};
		if((i*3) >= nOutLen)
		{
			break;
		}
		sprintf(temp, "%02X ", pInBuf[i]);		
		XOS_StrCat(pOutBuf,temp);
		j+=3;
	}
	

}

/********************************** 
函数名称    : InitTaskContextAry
功能描述    : 初始化实际任务需要的STaskContextAry信息
参数        : STaskContext* pTaskContext	   
: 
返回值      : STaskContextAry *g_pMaTaskContextAry ,如果没有就重新创建一个，否则将pTaskContext赋值到对应的信息中
例如:   		
************************************/
XS32 InitTaskContextAry(STaskContextAry **pPtrTaskContextAry, STaskContext* pTaskContext)
{
	if(pTaskContext == NULL)
	{
		return XERROR;
	}
	STaskContextAry* pTaskContextAry = *pPtrTaskContextAry;
	//多个线程，第一个线程先初始化
	if(XNULL == pTaskContextAry)
	{		
		pTaskContextAry = (STaskContextAry *)XOS_Malloc(sizeof(STaskContextAry));
		if(XNULL == pTaskContextAry)
		{
			return XERROR;
		}		
		pTaskContextAry->taskcount		= pTaskContext->taskcount;
		pTaskContextAry->startFid		= pTaskContext->startFid;
		pTaskContextAry->endFid			= pTaskContext->startFid + pTaskContext->taskcount;
		pTaskContextAry->dispatchFid	= pTaskContext->startFid;
		pTaskContextAry->pTaskContext	= (STaskContext *)XOS_Malloc(sizeof(STaskContext)
			* pTaskContext->taskcount);
		XOS_MemSet(pTaskContextAry->pTaskContext,0,(sizeof(STaskContext) * pTaskContext->taskcount));	
		*pPtrTaskContextAry = pTaskContextAry;
	}
	//将第n个线程的信息拷贝
	XOS_MemCpy(&pTaskContextAry->pTaskContext[pTaskContext->dbTaskId], pTaskContext, sizeof(STaskContext));
	return XSUCC;

}

/***************************************************************
函数：GetPresentTime
功能：获得当前的时间
输入：
输出：
返回：当前的时间    
/**************************************************************/
XU64 GetPresentTime(XVOID)
{
	XU64 sysTime = 0;
	struct tm fTimes;
	XU64 longTime = 0;
	XU64 diffTime = 0;
	struct timeb timeBuffer;

	longTime = 946656000;//将该秒数写为固定值否则，每次调用该函数取得的时间会不一致
	
	//获得当前系统时间
	ftime( &timeBuffer );
	
	//当前时间的秒数-2000年的秒数
	diffTime = timeBuffer.time - longTime;

	//精确毫秒数
	sysTime = diffTime*1000 + timeBuffer.millitm;//秒+毫秒

	return sysTime;
}
