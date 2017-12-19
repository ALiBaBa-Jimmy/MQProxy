/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRCommFun.h
* Author:       luhaiyan
* Date:					10-12-2014
* OverView:     SPRCommFun�Ĺ�����������Ҫ�����ĸ�����Ŀ�н��б���
*
* History:      ������ʷ�޸�����ǰ��
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*
* Revisor:      �޸�������
* Date:         MM-DD-YYYY
* Description:  �����޸Ĺ��ܵ�
*******************************************************************************/
#ifndef __SPR_COMMON_FUN_H
#define __SPR_COMMON_FUN_H

#include "agentinclude.h"
#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
�� �� ��: GetCurSysTime
��������:  ��ȡϵͳʱ��
��    ��:  XS8 *pTimeStr Ҫ���ص�ʱ�䴮
�� �� ֵ:  XS32 ���
**************************************************************************/
XS32 GetCurSysTime(XS8 *pTimeStr);


/************************************************************************
   ������������ ʮ������ תΪ �ַ���
ʾ����
    ���룺0x61��0x62��0x63��0x64
	�����ab��cd
                                                                  
/************************************************************************/
XU64 USER_HexToStr(XU8 *pHex, XU8 * pStr,XU64 ulLen);

/***********************************
������������ASCII�ַ�תΪʮ������
ʾ����
     ���� 0x61
	 ��� 0x0a
************************************/
XU8 USER_ChrToHex(XU8 chr);


/************************************************************************
   ������������ �ַ��� תΪ ʮ������
ʾ����
    ���룺ab��cd
	�����0x61��0x62��0x63��0x64
                                                                  
/************************************************************************/
XU64 USER_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen);



/************************************************************************
   ������������XU64���͵������ֽ���Little-Endian��ת��Ϊ�����ֽ���Big-Endian��
   ���룺XU64 val����ת����XU64������ֵ
		 XBOOL BYTE_ORDER�����ؼ�������ֽ���Little-EndianΪ0��Big-EndianΪ1
   �����ת���ֽ�������ֵ
ʾ����
    ���룺0x7654321087654321LL
	�����0x2143658710325476LL
                                                                  
/************************************************************************/
XU64 htonll(XU64 val, XBOOL byteOrder);

/************************************************************************
   ������������XU64���͵������ֽ���Big-Endian��ת��Ϊ�����ֽ���Little-Endian��
   ���룺XU64 val����ת����XU64������ֵ
		 XBOOL BYTE_ORDER�����ؼ�������ֽ���Little-EndianΪ0��Big-EndianΪ1
   �����ת���ֽ�������ֵ
ʾ����
    ���룺0x2143658710325476LL
	�����0x7654321087654321LL
                                                                  
/************************************************************************/
XU64 ntohll(XU64 val, XBOOL byteOrder);



XS32 User_StrtoInt(XS8 *pstr, XS32 len);


// ��BCD���ʽת�����ַ�����ʽ�ĺ���
XS64 BcdCvt2Num(XU8* pBcd, XU64 length, XU8* pStr);

// ���ַ�����ʽ�ĺ���ת����BCD���ʽ
XS32 NumCvt2Bcd(XU8 *pStr, XU8 *pBcd);

//BCD�뷴ת
XS64 ReverseBcd(XU8* strBcd, XU64 length, XU8* strRevBcd);


XS64 IpAddrHex2Str(XU8* IpHex,XU8* IpStr);

XVOID PrintMsgBuff(XU8* pOutBuf, XU32 nOutLen,XU8* pInBuf, XU32 nInLen );

/********************************** 
��������    : InitTaskContextAry
��������    : ��ʼ��ʵ��������Ҫ��STaskContextAry��Ϣ,����Ҫ���صľͿ������������Ҫ���ص���InitBalanceTaskContextAry
����        : STaskContext* pTaskContext	   
: 
����ֵ      : STaskContextAry *g_pMaTaskContextAry ,���û�о����´���һ��������pTaskContext��ֵ����Ӧ����Ϣ��
����:   		
************************************/
XS32 InitTaskContextAry(STaskContextAry **pPtrTaskContextAry, STaskContext* pTaskContext);

/***************************************************************
������GetPresentTime
���ܣ���õ�ǰ��ʱ��
���룺
�����
���أ���ǰ��ʱ��    
/**************************************************************/
XU64 GetPresentTime(XVOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif 
