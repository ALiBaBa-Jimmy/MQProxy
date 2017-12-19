/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRCommFun.h
* Author:       luhaiyan
* Date:					10-12-2014
* OverView:     SPRCommFun的公共函数，需要包含的各个项目中进行编译
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
#ifndef __SPR_COMMON_FUN_H
#define __SPR_COMMON_FUN_H

#include "agentinclude.h"
#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
函 数 名: GetCurSysTime
函数功能:  获取系统时间
参    数:  XS8 *pTimeStr 要返回的时间串
返 回 值:  XS32 结果
**************************************************************************/
XS32 GetCurSysTime(XS8 *pTimeStr);


/************************************************************************
   功能描述：把 十六进制 转为 字符串
示例：
    输入：0x61，0x62，0x63，0x64
	输出：ab，cd
                                                                  
/************************************************************************/
XU64 USER_HexToStr(XU8 *pHex, XU8 * pStr,XU64 ulLen);

/***********************************
功能描述：把ASCII字符转为十六进制
示例：
     输入 0x61
	 输出 0x0a
************************************/
XU8 USER_ChrToHex(XU8 chr);


/************************************************************************
   功能描述：把 字符串 转为 十六进制
示例：
    输入：ab，cd
	输出：0x61，0x62，0x63，0x64
                                                                  
/************************************************************************/
XU64 USER_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen);



/************************************************************************
   功能描述：把XU64类型的主机字节序（Little-Endian）转换为网络字节序（Big-Endian）
   输入：XU64 val：待转换的XU64类型数值
		 XBOOL BYTE_ORDER：本地计算机的字节序，Little-Endian为0，Big-Endian为1
   输出：转换字节序后的数值
示例：
    输入：0x7654321087654321LL
	输出：0x2143658710325476LL
                                                                  
/************************************************************************/
XU64 htonll(XU64 val, XBOOL byteOrder);

/************************************************************************
   功能描述：把XU64类型的网络字节序（Big-Endian）转换为主机字节序（Little-Endian）
   输入：XU64 val：待转换的XU64类型数值
		 XBOOL BYTE_ORDER：本地计算机的字节序，Little-Endian为0，Big-Endian为1
   输出：转换字节序后的数值
示例：
    输入：0x2143658710325476LL
	输出：0x7654321087654321LL
                                                                  
/************************************************************************/
XU64 ntohll(XU64 val, XBOOL byteOrder);



XS32 User_StrtoInt(XS8 *pstr, XS32 len);


// 把BCD码格式转换到字符串格式的号码
XS64 BcdCvt2Num(XU8* pBcd, XU64 length, XU8* pStr);

// 把字符串格式的号码转换到BCD码格式
XS32 NumCvt2Bcd(XU8 *pStr, XU8 *pBcd);

//BCD码反转
XS64 ReverseBcd(XU8* strBcd, XU64 length, XU8* strRevBcd);


XS64 IpAddrHex2Str(XU8* IpHex,XU8* IpStr);

XVOID PrintMsgBuff(XU8* pOutBuf, XU32 nOutLen,XU8* pInBuf, XU32 nInLen );

/********************************** 
函数名称    : InitTaskContextAry
功能描述    : 初始化实际任务需要的STaskContextAry信息,不需要负载的就可以用这个，需要负载的用InitBalanceTaskContextAry
参数        : STaskContext* pTaskContext	   
: 
返回值      : STaskContextAry *g_pMaTaskContextAry ,如果没有就重新创建一个，否则将pTaskContext赋值到对应的信息中
例如:   		
************************************/
XS32 InitTaskContextAry(STaskContextAry **pPtrTaskContextAry, STaskContext* pTaskContext);

/***************************************************************
函数：GetPresentTime
功能：获得当前的时间
输入：
输出：
返回：当前的时间    
/**************************************************************/
XU64 GetPresentTime(XVOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif 
