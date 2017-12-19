/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     SPRCommFun.cpp
* Author:       luhaiyan
* Date:					10-12-2014
* OverView:     SPR�Ĺ�����������Ҫ�����ĸ�����Ŀ�н��б���
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
#include "SPRCommFun.h"

#include "SPRTaskCommFun.h"
#include <math.h>


#define MAX_DIGITAL_LEN             33          //�绰
#define DIGITAL_BUFF_LEN            32          /*���뻺��������*/
#define BCD_NUM_MASK                0x0f


/**************************************************************************
�� �� ��: GetCurSysTime
��������:  ��ȡϵͳʱ��
��    ��:  XS8 *pTimeStr Ҫ���ص�ʱ�䴮
�� �� ֵ:  XS32 ���
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
   ������������ ʮ������ תΪ �ַ���
ʾ����
    ���룺0x61��0x62��0x63��0x64
	�����ab��cd
                                                                  
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
������������ASCII�ַ�תΪʮ������
ʾ����
     ���� 0x61
	 ��� 0x0a
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
   ������������ �ַ��� תΪ ʮ������
ʾ����
    ���룺ab��cd
	�����0x61��0x62��0x63��0x64
                                                                  
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
   ������������XU64���͵������ֽ���Little-Endian��ת��Ϊ�����ֽ���Big-Endian��
   ���룺XU64 val����ת����XU64������ֵ
		 XBOOL BYTE_ORDER�����ؼ�������ֽ���Little-EndianΪ0��Big-EndianΪ1
   �����ת���ֽ�������ֵ
ʾ����
    ���룺0x7654321087654321LL
	�����0x2143658710325476LL
                                                                  
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
   ������������XU64���͵������ֽ���Big-Endian��ת��Ϊ�����ֽ���Little-Endian��
   ���룺XU64 val����ת����XU64������ֵ
		 XBOOL BYTE_ORDER�����ؼ�������ֽ���Little-EndianΪ0��Big-EndianΪ1
   �����ת���ֽ�������ֵ
ʾ����
    ���룺0x2143658710325476LL
	�����0x7654321087654321LL
                                                                  
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
 ������:ReverseBcd
 ����:BCD�뷴ת
 ����:
		XU8* strBcd��BCD��
		XU64 length��BCD��ĳ���	
 ���:	XU8* strRevBcd����ת���BCD��
 ����:
 ˵��:	"10086"��BCD����strBcdΪ0x0180f600000000000000000000000000
		length Ϊ3
		 ������strRevBcd Ϊ   0x10086f00000000000000000000000000
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
��������    : NumCvt2Bcd
��������    : ���ַ�����ʽ�ĺ���ת����BCD���ʽ
����        : XCHAR *pStr ����, �ַ�����ʽ
����        : XU8 *pBcd ���, BCD���ʽ����
����        : XU8 *pLen ����, �������
����ֵ      : XS32: BCD�볤�ȣ�XERROR��ʾ����
����:
        "33" => 0x33 
        "34" => 0x43
        "37" => 0x73
        "3468" => 0x4386
        "3768" => 0x7386

		����pStr ��10086��
		���pBcd  0x0180f600000000000000000000000000
		 
************************************/
XS32 NumCvt2Bcd(XU8 *pStr, XU8 *pBcd)
{   
    XU8 ucIdx = 0;
    XU8 ucBuf = 0;       /*BCD����������*/
    XU8 ucNum = 0;       /*ȡ���ĺ���*/
    XU8 ucMaxLen = 0;    /*ʵ��ȡ�ĳ���*/

    /*�����Ϸ��Լ��*/
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
        /*ucIdxΪż��ʱȡ����λ, Ϊ����ʱȡ����λ*/
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
��������    : BcdCvt2Num
��������    : ��BCD���ʽת�����ַ�����ʽ�ĺ���
����        : XCHAR *pBcd ����, BCD���ʽ����		   
			: XU8 *pLen ����, �������
���        : XU8 *strTel ���, �ַ�����ʽ
����ֵ      : XS64 
����:   		
		����pBcd  0x0180f600
		���strTel ��10086��
************************************/
XS64 BcdCvt2Num(XU8* pBcd, XU64 length, XU8* pStr)
{
	char reverBcd[16] ={0};
	memset(reverBcd, 0xff, 16);
	//��תbcd
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
��������    : IpAddrHex2Str
��������    : ��IpV4��ַ��ʮ������������ת��Ϊ����172.16.8.237�ĸ�ʽ
����        : XCHAR *IpHex ����, ������ʽ		   
			: XU8 *pLen ����, �������
���        : XU8 *IpStr ���, �ַ�����ʽ���������Ϊ 16���ȵ��ַ�����
����ֵ      : XS64 
����:   		
************************************/
XS64 IpAddrHex2Str(XU8* IpHex,XU8* IpStr)
{
	XS8 tempIp[16] ={0};
	for(int i =0; i<4; i++)//Ĭ��IpHex����Ϊ4
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
��������    : printMsgBuff
��������    : ��ӡ��Ϣ
����        : XU8* pOutBuf, �����buf��
              XU32 nOutLen�� �����Ϣ���洫�ݹ�����buf�ĳ���
			  XU8* pInBuf��������Ϣ 
			  XU32 nInLen ��������Ϣ����	   
			: 
����ֵ      : XVOID 
����:   		
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
��������    : InitTaskContextAry
��������    : ��ʼ��ʵ��������Ҫ��STaskContextAry��Ϣ
����        : STaskContext* pTaskContext	   
: 
����ֵ      : STaskContextAry *g_pMaTaskContextAry ,���û�о����´���һ��������pTaskContext��ֵ����Ӧ����Ϣ��
����:   		
************************************/
XS32 InitTaskContextAry(STaskContextAry **pPtrTaskContextAry, STaskContext* pTaskContext)
{
	if(pTaskContext == NULL)
	{
		return XERROR;
	}
	STaskContextAry* pTaskContextAry = *pPtrTaskContextAry;
	//����̣߳���һ���߳��ȳ�ʼ��
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
	//����n���̵߳���Ϣ����
	XOS_MemCpy(&pTaskContextAry->pTaskContext[pTaskContext->dbTaskId], pTaskContext, sizeof(STaskContext));
	return XSUCC;

}

/***************************************************************
������GetPresentTime
���ܣ���õ�ǰ��ʱ��
���룺
�����
���أ���ǰ��ʱ��    
/**************************************************************/
XU64 GetPresentTime(XVOID)
{
	XU64 sysTime = 0;
	struct tm fTimes;
	XU64 longTime = 0;
	XU64 diffTime = 0;
	struct timeb timeBuffer;

	longTime = 946656000;//��������дΪ�̶�ֵ����ÿ�ε��øú���ȡ�õ�ʱ��᲻һ��
	
	//��õ�ǰϵͳʱ��
	ftime( &timeBuffer );
	
	//��ǰʱ�������-2000�������
	diffTime = timeBuffer.time - longTime;

	//��ȷ������
	sysTime = diffTime*1000 + timeBuffer.millitm;//��+����

	return sysTime;
}
