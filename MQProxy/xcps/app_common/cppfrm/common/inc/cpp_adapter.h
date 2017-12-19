/************************************************************************************
�ļ���  :cpp_adapter.h

�ļ�����:

����    :zzt

��������:2006/01/19

�޸ļ�¼:
luoyong    2014/05/19   �Ż����ϵ�ƽ̨

************************************************************************************/
#ifndef __CPP_PUBLIC_H_
#define __CPP_PUBLIC_H_


#include "xosshell.h"
#include "trace_agent.h"
#include "cpp_new.h"


#ifdef XOS_WIN32
#if (_MSC_VER < 1310)
#define typename
#endif

//��ʱ����4996�澯��add by liaod
#pragma warning(disable:4996)

#endif


//���δ���Ϊ��Ԫ������
const XU8 LOW_BYTE_FIRST  = 0;
const XU8 HIGH_BYTE_FIRST = 1;
#ifdef CPU_HIGH_BYTE
const XU8 LOCAL_ENDIAN = HIGH_BYTE_FIRST;
#else
const XU8 LOCAL_ENDIAN = LOW_BYTE_FIRST;
#endif


#ifdef _CPPUNIT_
//��CPPUINT�б���У�鴦�������˵����������⣬��CPPUINT�����п��ܳ����������⡣
//������ΪVC6.0��������ĳ�Ա������ʱ�������˺������ʼ���
#define private              public
#define protected            public
#endif


typedef XU32   BID;
typedef XU32   MDID;         //ģ��ID, Ҳ���ǹ���������ID
typedef XU32   FSMID;        //״̬��ID
typedef XU16   TMSGID;


//�����붨��
enum EerrNo
{
    ERR_OK,                 //�޴���
    ERR_NOT_INITIAL,        //����δ��ʼ��
    ERR_INITIAL_FAIL,       //�����ʼ��ʧ��
    ERR_INVALID_PARAM,      //�Ƿ�����
    ERR_INVALID_OBJ,        //�Ƿ�����
    ERR_INVALID_OPR,        //�Ƿ�����
    ERR_MEMORY,             //�ڴ�Խ����߲��ܷ����ڴ�
    ERR_OBJ_NOT_FOUND,      //����δ�ҵ�
    ERR_OBJ_EXISTED,        //����һ������,���������
    ERR_NO_RESOURCE,        //û����Դ
    ERR_NO_DEFINE           //δ�������
};



#define INVALID_OBJ_ID       0

//ģ��ID����
const XU32 CPPFRM_MODULE_ID = FID_USERMIN;

//��ϢID����
const TMSGID SYS_TIMEOUT_MSG  = 1025;


/*********************************************************************
�������� : GetLocalBoardId
�������� : ��ñ���ID

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
extern "C"  XU32 XOS_GetLocalPID( XVOID );

typedef XU32 BID;

inline BID GetLocalBoardId()
{
    return XOS_GetLocalPID();
}


/*********************************************************************
��������: SS_CliWriteStr, SS_CliWriteLine,SS_CliPrintf,SS_CliInforPrintf
��������: �ṩһ��cli��ӡ����
����ԭ��:
     XS32 XOS_CliExtWriteStr(CLI_ENV *pCliEnv, XCHAR *pBuf)
     XS32 XOS_CliExtWriteStrLine(CLI_ENV *pCliEnv, XCHAR *pBuf)
     XS32 XOS_CliExtPrintf( CLI_ENV* pCliEnv, XCHAR* pFmt, ...)
     XS32 XOS_CliInforPrintf( XCHAR* pFmt, ... )
��������:
     SS_CliWriteStr: ��cli�ϴ�ӡһ���ַ���,������
     SS_CliWriteLine: ��cli�ϴ�ӡһ��
     SS_CliPrintf: ��printf��������,ע��:���з�������"\r\n",�����������
     SS_CliInforPrintf: ��SS_CliPrintf����,�����ṩ�ض�����
**********************************************************************/
#define SS_CliWriteStr        XOS_CliExtWriteStr
#define SS_CliWriteLine       XOS_CliExtWriteStrLine
#define SS_CliPrintf          XOS_CliExtPrintf
#define SS_CliInforPrintf     XOS_CliInforPrintf


/*********************************************************************
���� :  PrintInfo
ְ�� :  ��ӡ��Ϣ�Ľӿ�
        ��ӡ�������£���ӡ�����ɸߵ���:
        PL_EXP,    //�쳣��ӡ����(�����ʹ�ӡ)
        PL_ERR,    //һ���Դ����ӡ����(�����ʹ�ӡ)
        PL_WARN,   //������Ϣ��ӡ����(���д�ӡ)
        PL_INFO,   //������Ϣ��ӡ����(�����ʹ�ӡ)
        PL_DBG,    //�ܵ͵Ĵ�ӡ����(�����ʹ�ӡ)

        һ�����ӡ���BCPģ���һ��ss_bcp_xx.cpp�ļ��д�ӡ XS32 a ����ֵ.
        PrintInfo(PA(BCP_MODULE_ID,PL_WARN),"a value is %d \n",a);
        PA��ǰ�涨��ĺ꣬BCP_MODULE_ID��BCP��ģ��ID��PL_WARN�Ǵ�ӡ����
        �����ڿ���̨�Ͽ�������ĳ��ģ��Ĵ�ӡ����,�������Կ��Ƹ�ģ��Ĵ�ӡ
        �����Ϣ������������BCPģ��Ĵ�ӡ����ΪPL_DBG,���ģ�������еĴ�ӡ
        ���Ĵ�ӡ��Ϣ�����ӡ������
        �����ø�ģ��Ĵ�ӡ����ΪPL_INFO�����ģ��������
        ������PL_EXP,PL_ERR,PL_WARN,PL_INFO�Ĵ�ӡ��䶼���ӡ���������
        ��ΪPL_DBG����Ϣ��
            ��֮������ģ��Ĵ�ӡ����֮��ƽֻ̨���ӡ��ģ���б����õĴ�ӡ
        ����ߵ���Ϣ

Э��:   1.XOS_Trace:
        XS8* FileName          �ļ���
        XU32 ulLineNum         ��ǰ��
        MDID  mid              ��ӡ���������ģ��ID
        EPrtLeve eLevel        ��ӡ����
        XS8 *cFormat, ...      ��printf�Ĳ�������һ��

        2.PA ��
��ʷ :
        �޸���   ����          ����
*********************************************************************/
#define PA(mid,eLevel)         MD(mid,eLevel)
#define PrintInfo              XOS_Trace

#define LA(mid,eLevel,logid)   MD(mid,eLevel),(logid)
#define LogPrint               XOS_LogTrace



#ifndef XOS_WIN32 /* Solaris��û������������ */

inline XS8 *itoa(XS32 value, XS8 *string, XS32 radix)
{
    if (radix == 10) // ʮ����
    {
        sprintf(string, "%d", value);
    }
    else if (radix == 16)  // ʮ������
    {
        sprintf(string, "%x", value);
    }
    else // �������Ʋ�֧��
    {
        string[0] = 0;
        //SS_ASSERT_RV(false, string);
    }

    return string;
}

inline XS8 *_ultoa(XU32 value, XS8 *string, XS32 radix)
{
    if (radix == 10) // ʮ����
    {
        sprintf(string, "%u", value);
    }
    else if (radix == 16)  // ʮ������
    {
        sprintf(string, "%x", value);
    }
    else // �������Ʋ�֧��
    {
        string[0] = 0;
        //SS_ASSERT_RV(false, string);
    }

    return string;
}

#endif




/*********************************************************************
���� :
ְ�� : �ַ��������ӿ�
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
//����дת��ΪСд
#define SYS_STRLWR(pchar)                  strlwr((pchar))
//��Сдת��Ϊ��д
#define SYS_STRUPR(pchar)                  strupr((pchar))

//string->intת��
#define SYS_ATOI(pStr)                     atoi((pStr))
//string->longת��
#define SYS_ATOL(pStr)                     atol((pStr))
//string->doubleת��
#define SYS_ATOF(pStr)                     atof((pStr))

//int->stringת��
#define SYS_ITOA(iValue,pStr,nRadix)       itoa((iValue),(pStr),(nRadix))
//long->stringת��
#define SYS_ULTOA(ulValue,pStr,nRadix)     _ultoa((ulValue),(pStr),(nRadix))


/*********************************************************************
���� :
ְ�� : XOS����ֵ��FRM����ֵ֮��ӳ��
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
#define RST_XOS_SS(rstXOS)   ((rstXOS) == XSUCC  ? ERR_OK : ERR_NO_DEFINE)
#define RST_SS_XOS(rstSS)    ((rstSS)  == ERR_OK ? XSUCC  : XERROR)


/*********************************************************************
���� : ASSERT
ְ�� : ��
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
#define CPP_ASSERT_RN(expr)                                                                  \
if(!(expr))                                                                                 \
{                                                                                           \
   PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Assert Failed, expr is {%s}\r\n", #expr);         \
   return;                                                                                  \
}

#define CPP_ASSERT_RV(expr, reVal)                                                           \
if(!(expr))                                                                                 \
{                                                                                           \
   PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"Assert Failed, expr is {%s}\r\n", #expr);         \
   return reVal;                                                                            \
}


/*********************************************************************
���� :
ְ�� : �����͵����ݽ����ֽ���ת��
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
inline XS8 IntegerByteSwap(XS8 cValue)
{
    return cValue;
}
inline XU8 IntegerByteSwap(XU8 ucValue)
{
    return ucValue;
}
inline XU16 IntegerByteSwap(XU16 usValue)
{
    return (XU16)((usValue & 0x00ff)<<8) |((usValue & 0xff00)>>8);
}
inline XS16 IntegerByteSwap(XS16 sValue)
{
    return (XS16) IntegerByteSwap((XU16)sValue);
}
inline XU32 IntegerByteSwap(XU32 uiValue)
{
    return( ((uiValue&0x000000FF)<<24) |         \
            ((uiValue&0x0000FF00)<<8)  |         \
            ((uiValue&0x00FF0000)>>8)  |         \
            ((uiValue&0xFF000000)>>24));
}
inline XS32 IntegerByteSwap(XS32 iValue)
{
    return  (XS32) IntegerByteSwap((XU32)iValue);
}
inline XU64 IntegerByteSwap(XU64 udwValue)
{
    return ( (((XU64)(IntegerByteSwap((XU32)((udwValue<<32)>>32)))) <<32)  |    \
             ((XU64)(IntegerByteSwap((XU32)(udwValue>>32)))) );
}
inline XS64 IntegerByteSwap(XS64 dwValue)
{
    return ( (XS64)IntegerByteSwap((XU64)dwValue) );
}

#define SWAP_BYTE_ODER(value) value = IntegerByteSwap((value))



/*********************************************************************
���½ӿ�����ƽ̨����Ϣ���ͻ��ƣ��������µķ���:
            1.������ͷ���Ϣ�Ļ���
            2.������Ϣ�ӿ�
��ƽ̨��Ϣͷ�йص�һЩӳ��
���ǽ�ƽ̨��t_XOSMSGHEAD::PID��Ϊ��ID����t_XOSMSGHEAD::FID��Ϊģ��ID����
��Ϣ����ID
**********************************************************************/
typedef  t_XOSCOMMHEAD   CCpsMsgHead;  //ƽ̨��Ϣͷ
typedef  t_XOSUSERID     CAddrCpsMsg;  //ƽ̨��Ϣ��ַ

const XU32 CPS_MSGHEAD_LENGTH = sizeof(CCpsMsgHead); //ƽ̨��Ϣͷ�Ĵ�С

inline bool operator==(const CAddrCpsMsg& left,const CAddrCpsMsg& right)
{
   return (left.PID == right.PID)&&(left.FID == right.FID);
}

/*********************************************************************
�������� :MallocMsgBufCps
�������� :���亯�� ������ƽ��������Ϣ�ڴ�Ľӿ�
          �ú���������ڴ�ֻ����FreeMsgBufCps�ͷ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID *MallocMsgBufCps(XU32 uiSize,MDID mid)
{
    return XOS_MsgMemMalloc(mid, uiSize);
}

/*********************************************************************
�������� : FreeMsgBufCps
�������� :
          ���亯�� ������ƽ���ͷ���Ϣ�ڴ�Ľӿڣ�ֻ���ͷ���
          MallocMsgBufCps������ڴ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID FreeMsgBufCps(XVOID *msghead,MDID mid)
{
   XOS_MsgMemFree(mid,(CCpsMsgHead *)msghead);
}

/*********************************************************************
�������� : SendMsgCps
�������� :
          ���亯�� ������ƽ��������Ϣ�Ľӿ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo SendMsgCps(CCpsMsgHead *msghead)
{
   return RST_XOS_SS(XOS_MsgSend(msghead));
}

/*********************************************************************
�������� : GetCPsSizeFromFullLength
�������� : ƽ̨����Ϣ����ķ��亯���Ĳ�����С�ǲ�����ƽ̨ͷ�Ĵ�С�ġ�
          �����ڴ����ʹ�ÿ�ܵ���Ϣͷʱ(CMsg)��ϰ����ֱ�Ӵ�CMsg�̳й���
          �����ڷ����ڴ�ʱ��ϰ�߰���ͷ�Ĵ�С��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XU32 GetCPsLenFromFullLength(XU32 uiFullSize)
{
    CPP_ASSERT_RV(uiFullSize > CPS_MSGHEAD_LENGTH, 0);
    return uiFullSize - CPS_MSGHEAD_LENGTH;
}



/*********************************************************************
���½ӿ�����ƽ̨�Ķ�ʱ������ؽӿڡ�
**********************************************************************/
/*********************************************************************
�������� : ClearTimerHandCps
�������� : ��ʼ��ƽ̨��ʱ�����

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID ClearTimerHandCps(PTIMER& refHand)
{
    XOS_INIT_THDLE(refHand);
}

/*********************************************************************
�������� : TimerCreate
�������� : ������ʱ����ֻ���ǵ;��ȶ�ʱ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerCreateCps(MDID mid,PTIMER& tHand, e_TIMERTYPE tt, XVOID * para)
{
    t_BACKPARA t_paRatmp = {(XPOINT)para,0,0,0};
    return RST_XOS_SS(XOS_TimerCreate(mid,&tHand,tt,TIMER_PRE_LOW,&t_paRatmp));
}

/*********************************************************************
�������� : TimerStart
�������� : ������ʱ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerStartCps(MDID mid,PTIMER tHandle, XU32 uiLen)
{
    // ��ʱ��������SS_LOWTIMER_CLCUNIT�ı���
    CPP_ASSERT_RV(((uiLen % LOC_LOWTIMER_CLCUNIT) == 0), ERR_NOT_INITIAL);

    // ��ʱ�����������С�̶�
    CPP_ASSERT_RV((uiLen >= LOC_LOWTIMER_CLCUNIT), ERR_NOT_INITIAL);

     return RST_XOS_SS(XOS_TimerBegin(mid,tHandle,uiLen));
}

/*********************************************************************
�������� : TimerStopCps
�������� : ֹͣ��ʱ��,��ֹͣ������Ҫ����ƽ̨��ֹͣ������ƽ̨��ʵ�൱һ��ɾ������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerStopCps(MDID mid, PTIMER pTimer)
{
    return RST_XOS_SS(XOS_TimerEnd(mid,pTimer));
}

/*********************************************************************
�������� : TimerDeleteCps
�������� : ɾ����ʱ��,��ֹͣ�ٷ����������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/

inline EerrNo TimerDeleteCps(MDID mid,PTIMER pTimer)
{
    return RST_XOS_SS(XOS_TimerDelete(mid,pTimer));
}

/*********************************************************************
�������� : TimerNumRegCps
�������� : ��ʱ��ע�ắ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo  TimerNumRegCps( MDID mId, XU32 uiMaxTimerNum)
{
    return  RST_XOS_SS(XOS_TimerReg(mId, LOC_LOWTIMER_CLCUNIT,uiMaxTimerNum, 0));
}


typedef e_TIMERTYPE TimerType;

struct CTm
{
    XU32 tm_sec;     // �� �C ȡֵ����Ϊ[0,59]
    XU32 tm_min;     // �� - ȡֵ����Ϊ[0,59]
    XU32 tm_hour;    // ʱ - ȡֵ����Ϊ[0,23]
    XU32 tm_mday;    // һ�����е����� - ȡֵ����Ϊ[1,31]
    XU32 tm_mon;     // �·ݣ���һ�¿�ʼ��ȡֵ����Ϊ[1,12]
    XU32 tm_year;    // ��ǰ��ݣ����統ǰ��2006
    XU32 tm_wday;    // ���� �C ȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ�����
    XU32 tm_yday;    // ��ÿ���1��1�տ�ʼ������ �C ȡֵ����Ϊ[0,365]������0����1��1�գ�1����1��2�գ��Դ�����
    XU32 tm_isdst;   // ����ʱ��ʶ����ʵ������ʱ��ʱ��tm_isdstΪ������ʵ������ʱ��tm_isdstΪ0�����˽����ʱ��tm_isdst()Ϊ����
};

/*********************************************************************
�������� :
�������� : ��ø�������ʱ�� GetStandTimer()

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID BuildCTm(CTm& refCtm,const tm& refSysTime)
{
    XOS_MemCpy(&refCtm, &refSysTime, sizeof(struct tm));
    refCtm.tm_year += 1900;    //tm�е�ʱ����������1900�Ĳ�ֵ
    refCtm.tm_mon  += 1;       //tm�е��·�
}

/*********************************************************************
�������� :
�������� : ��ø�������ʱ�� GetStandTimer()

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID BuildTm(tm& refSysTime, const CTm& refCtm)
{
    XOS_MemCpy(&refSysTime, &refCtm, sizeof(struct tm));
    refSysTime.tm_year -=1900;    //tm�е�ʱ����������1900�Ĳ�ֵ
    refSysTime.tm_mon  -=1;       //tm�е��·�
}

/*********************************************************************
�������� :
�������� :���ϵͳ��1970.01.01.0.0��ʼ������������������(Microsoft C/C++ 7.0
          ��ʱ����ֵ�Ǵ�1899��12��31��0ʱ0��0�뵽��ʱ���
          ������������)����ֵ��һ��ULONG���͵ģ���2038��1��18��19ʱ14��07�뽫���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline t_XOSTT GetSecondTime()
{
    return (t_XOSTT)time(XNULL);
}

/*********************************************************************
�������� : GetClock
�������� : ���ϵͳ�ӿ������������̵������е���clock()����ʱ֮���
           CPUʱ�Ӽ�ʱ��Ԫ.CPUʱ�Ӽ�ʱ��Ԫ��CPU�������й�. X86��1msΪ
           һ��ʱ�Ӽ�ʱ��Ԫ��
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline t_XOSCT GetClock()
{
    t_XOSCT clockt = 0;
    (XVOID)XOS_Clock(&clockt);

    return clockt;
}

/*********************************************************************
��������: GetLocalTime
��������: ��þֲ�ʱ�䣬����ϵͳ���õ�ʱ���й�ϵ,�������ǻ����ϵ��Ǳ���ʱ�䡣

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID  GetLocalTime(CTm& refCtm, t_XOSTT ulSecond)
{
    t_XOSTT scondtime = static_cast<t_XOSTT>(ulSecond);
    struct tm *local = localtime((const time_t *) &scondtime);
    BuildCTm(refCtm, *local);
}

/*********************************************************************
�������� : GetLocalTime
�������� : ��þֲ�ʱ�䣬����ϵͳ���õ�ʱ���й�ϵ,�������ǻ����ϵ��Ǳ���ʱ�䡣

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID  GetLocalTime(CTm& refCtm)
{
    GetLocalTime(refCtm,GetSecondTime());
}

/*********************************************************************
�������� : GetRunTime
�������� : ���ϵͳ���е�ʱ�䣬����ϵͳ���õ�ʱ���й�ϵ,�������ǻ����ϵ��Ǳ���ʱ�䡣

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID GetRunTime(CTm& refCtm, t_XOSTT ulSecond)
{
    t_XOSTT scondtime = static_cast<t_XOSTT>(ulSecond);
    struct tm *local = localtime((const time_t *) &scondtime);
    BuildCTm(refCtm, *local);
}

/*********************************************************************
�������� : GetRunTime
�������� : ���ϵͳ���е�ʱ�䣬����ϵͳ���õ�ʱ���й�ϵ,�������ǻ����ϵ��Ǳ���ʱ�䡣

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID GetRunTime(CTm& refCtm)
{
    GetRunTime(refCtm, XOS_TicksToSec(XOS_GetSysTicks()));
}

/*********************************************************************
�������� : SetTraceLevelByFid
�������� : ����Ĭ��TraceLevel��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline bool SetTraceLevelByFid(XU32 fid, e_PRINTLEVEL level)
{
    t_FIDTRACEINFO* pTraceInfo = MOD_getFidTraceInfo(fid);
    if (NULL == pTraceInfo)
    {
        return false;
    }

    pTraceInfo->traceLevel = level;
    return true;
}

#endif



