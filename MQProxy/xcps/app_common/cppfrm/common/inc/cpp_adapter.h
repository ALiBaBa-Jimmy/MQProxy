/************************************************************************************
文件名  :cpp_adapter.h

文件描述:

作者    :zzt

创建日期:2006/01/19

修改记录:
luoyong    2014/05/19   优化整合到平台

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

//暂时过滤4996告警，add by liaod
#pragma warning(disable:4996)

#endif


//本段代码为单元测试用
const XU8 LOW_BYTE_FIRST  = 0;
const XU8 HIGH_BYTE_FIRST = 1;
#ifdef CPU_HIGH_BYTE
const XU8 LOCAL_ENDIAN = HIGH_BYTE_FIRST;
#else
const XU8 LOCAL_ENDIAN = LOW_BYTE_FIRST;
#endif


#ifdef _CPPUNIT_
//在CPPUINT中便于校验处理。包含了第三方代码或库，在CPPUINT工程中可能出现链接问题。
//这是因为VC6.0在重整类的成员函数名时，考虑了函数访问级别。
#define private              public
#define protected            public
#endif


typedef XU32   BID;
typedef XU32   MDID;         //模块ID, 也就是工厂管理者ID
typedef XU32   FSMID;        //状态机ID
typedef XU16   TMSGID;


//错误码定义
enum EerrNo
{
    ERR_OK,                 //无错误
    ERR_NOT_INITIAL,        //对象未初始化
    ERR_INITIAL_FAIL,       //对象初始化失败
    ERR_INVALID_PARAM,      //非法参数
    ERR_INVALID_OBJ,        //非法对象
    ERR_INVALID_OPR,        //非法操作
    ERR_MEMORY,             //内存越界或者不能分配内存
    ERR_OBJ_NOT_FOUND,      //对象未找到
    ERR_OBJ_EXISTED,        //插入一个对象,但对象存在
    ERR_NO_RESOURCE,        //没有资源
    ERR_NO_DEFINE           //未定义错误
};



#define INVALID_OBJ_ID       0

//模块ID定义
const XU32 CPPFRM_MODULE_ID = FID_USERMIN;

//消息ID定义
const TMSGID SYS_TIMEOUT_MSG  = 1025;


/*********************************************************************
函数名称 : GetLocalBoardId
功能描述 : 获得本板ID

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
extern "C"  XU32 XOS_GetLocalPID( XVOID );

typedef XU32 BID;

inline BID GetLocalBoardId()
{
    return XOS_GetLocalPID();
}


/*********************************************************************
函数名称: SS_CliWriteStr, SS_CliWriteLine,SS_CliPrintf,SS_CliInforPrintf
功能描述: 提供一组cli打印函数
函数原型:
     XS32 XOS_CliExtWriteStr(CLI_ENV *pCliEnv, XCHAR *pBuf)
     XS32 XOS_CliExtWriteStrLine(CLI_ENV *pCliEnv, XCHAR *pBuf)
     XS32 XOS_CliExtPrintf( CLI_ENV* pCliEnv, XCHAR* pFmt, ...)
     XS32 XOS_CliInforPrintf( XCHAR* pFmt, ... )
函数功能:
     SS_CliWriteStr: 在cli上打印一个字符串,不换行
     SS_CliWriteLine: 在cli上打印一行
     SS_CliPrintf: 和printf功能类似,注意:换行符必须是"\r\n",否则会有问题
     SS_CliInforPrintf: 和SS_CliPrintf类似,但是提供重定向功能
**********************************************************************/
#define SS_CliWriteStr        XOS_CliExtWriteStr
#define SS_CliWriteLine       XOS_CliExtWriteStrLine
#define SS_CliPrintf          XOS_CliExtPrintf
#define SS_CliInforPrintf     XOS_CliInforPrintf


/*********************************************************************
名称 :  PrintInfo
职责 :  打印信息的接口
        打印级别如下，打印级别由高到低:
        PL_EXP,    //异常打印级别(错误型打印)
        PL_ERR,    //一般性错误打印级别(错误型打印)
        PL_WARN,   //运行信息打印级别(运行打印)
        PL_INFO,   //调试信息打印级别(调试型打印)
        PL_DBG,    //很低的打印级别(调试型打印)

        一个例子。在BCP模块的一个ss_bcp_xx.cpp文件中打印 XS32 a 的数值.
        PrintInfo(PA(BCP_MODULE_ID,PL_WARN),"a value is %d \n",a);
        PA是前面定义的宏，BCP_MODULE_ID是BCP的模块ID，PL_WARN是打印级别，
        我们在控制台上可以设置某个模块的打印级别,这样可以控制该模块的打印
        输出信息量，比如设置BCP模块的打印级别为PL_DBG,这该模块中所有的打印
        语句的打印信息都会打印出来，
        若设置该模块的打印级别为PL_INFO，则该模块中所有
        级别是PL_EXP,PL_ERR,PL_WARN,PL_INFO的打印语句都会打印，但不会打级
        别为PL_DBG的信息。
            总之，设置模块的打印级别之后，平台只会打印该模块中比设置的打印
        级别高的信息

协作:   1.XOS_Trace:
        XS8* FileName          文件名
        XU32 ulLineNum         当前行
        MDID  mid              打印语句所属的模块ID
        EPrtLeve eLevel        打印级别
        XS8 *cFormat, ...      与printf的参数含义一样

        2.PA 宏
历史 :
        修改者   日期          描述
*********************************************************************/
#define PA(mid,eLevel)         MD(mid,eLevel)
#define PrintInfo              XOS_Trace

#define LA(mid,eLevel,logid)   MD(mid,eLevel),(logid)
#define LogPrint               XOS_LogTrace



#ifndef XOS_WIN32 /* Solaris下没有这两个函数 */

inline XS8 *itoa(XS32 value, XS8 *string, XS32 radix)
{
    if (radix == 10) // 十进制
    {
        sprintf(string, "%d", value);
    }
    else if (radix == 16)  // 十六进制
    {
        sprintf(string, "%x", value);
    }
    else // 其他进制不支持
    {
        string[0] = 0;
        //SS_ASSERT_RV(false, string);
    }

    return string;
}

inline XS8 *_ultoa(XU32 value, XS8 *string, XS32 radix)
{
    if (radix == 10) // 十进制
    {
        sprintf(string, "%u", value);
    }
    else if (radix == 16)  // 十六进制
    {
        sprintf(string, "%x", value);
    }
    else // 其他进制不支持
    {
        string[0] = 0;
        //SS_ASSERT_RV(false, string);
    }

    return string;
}

#endif




/*********************************************************************
名称 :
职责 : 字符串操作接口
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
//将大写转换为小写
#define SYS_STRLWR(pchar)                  strlwr((pchar))
//将小写转换为大写
#define SYS_STRUPR(pchar)                  strupr((pchar))

//string->int转换
#define SYS_ATOI(pStr)                     atoi((pStr))
//string->long转换
#define SYS_ATOL(pStr)                     atol((pStr))
//string->double转换
#define SYS_ATOF(pStr)                     atof((pStr))

//int->string转换
#define SYS_ITOA(iValue,pStr,nRadix)       itoa((iValue),(pStr),(nRadix))
//long->string转换
#define SYS_ULTOA(ulValue,pStr,nRadix)     _ultoa((ulValue),(pStr),(nRadix))


/*********************************************************************
名称 :
职责 : XOS返回值和FRM返回值之间映射
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
#define RST_XOS_SS(rstXOS)   ((rstXOS) == XSUCC  ? ERR_OK : ERR_NO_DEFINE)
#define RST_SS_XOS(rstSS)    ((rstSS)  == ERR_OK ? XSUCC  : XERROR)


/*********************************************************************
名称 : ASSERT
职责 : 宏
协作 :
历史 :
       修改者   日期          描述
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
名称 :
职责 : 对整型的数据进行字节序转换
协作 :
历史 :
       修改者   日期          描述
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
以下接口适配平台的消息发送机制，包括如下的方面:
            1.分配和释放消息的缓存
            2.发送消息接口
与平台消息头有关的一些映射
我们将平台的t_XOSMSGHEAD::PID称为板ID，将t_XOSMSGHEAD::FID称为模块ID，或
消息队列ID
**********************************************************************/
typedef  t_XOSCOMMHEAD   CCpsMsgHead;  //平台消息头
typedef  t_XOSUSERID     CAddrCpsMsg;  //平台消息地址

const XU32 CPS_MSGHEAD_LENGTH = sizeof(CCpsMsgHead); //平台消息头的大小

inline bool operator==(const CAddrCpsMsg& left,const CAddrCpsMsg& right)
{
   return (left.PID == right.PID)&&(left.FID == right.FID);
}

/*********************************************************************
函数名称 :MallocMsgBufCps
功能描述 :适配函数 。适配平他分配消息内存的接口
          该函数分配的内存只能由FreeMsgBufCps释放

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID *MallocMsgBufCps(XU32 uiSize,MDID mid)
{
    return XOS_MsgMemMalloc(mid, uiSize);
}

/*********************************************************************
函数名称 : FreeMsgBufCps
功能描述 :
          适配函数 。适配平他释放消息内存的接口，只能释放由
          MallocMsgBufCps分配的内存

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID FreeMsgBufCps(XVOID *msghead,MDID mid)
{
   XOS_MsgMemFree(mid,(CCpsMsgHead *)msghead);
}

/*********************************************************************
函数名称 : SendMsgCps
功能描述 :
          适配函数 。适配平他发送消息的接口

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo SendMsgCps(CCpsMsgHead *msghead)
{
   return RST_XOS_SS(XOS_MsgSend(msghead));
}

/*********************************************************************
函数名称 : GetCPsSizeFromFullLength
功能描述 : 平台的消息缓存的分配函数的参数大小是不包含平台头的大小的。
          但由于大家在使用框架的消息头时(CMsg)都习惯用直接从CMsg继承过来
          所以在分配内存时都习惯包含头的大小。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XU32 GetCPsLenFromFullLength(XU32 uiFullSize)
{
    CPP_ASSERT_RV(uiFullSize > CPS_MSGHEAD_LENGTH, 0);
    return uiFullSize - CPS_MSGHEAD_LENGTH;
}



/*********************************************************************
如下接口适配平台的定时器的相关接口。
**********************************************************************/
/*********************************************************************
函数名称 : ClearTimerHandCps
功能描述 : 初始化平台定时器句柄

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID ClearTimerHandCps(PTIMER& refHand)
{
    XOS_INIT_THDLE(refHand);
}

/*********************************************************************
函数名称 : TimerCreate
功能描述 : 创建定时器，只能是低精度定时器

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerCreateCps(MDID mid,PTIMER& tHand, e_TIMERTYPE tt, XVOID * para)
{
    t_BACKPARA t_paRatmp = {(XPOINT)para,0,0,0};
    return RST_XOS_SS(XOS_TimerCreate(mid,&tHand,tt,TIMER_PRE_LOW,&t_paRatmp));
}

/*********************************************************************
函数名称 : TimerStart
功能描述 : 启动定时器

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerStartCps(MDID mid,PTIMER tHandle, XU32 uiLen)
{
    // 定时器必须是SS_LOWTIMER_CLCUNIT的倍数
    CPP_ASSERT_RV(((uiLen % LOC_LOWTIMER_CLCUNIT) == 0), ERR_NOT_INITIAL);

    // 定时器必须大于最小刻度
    CPP_ASSERT_RV((uiLen >= LOC_LOWTIMER_CLCUNIT), ERR_NOT_INITIAL);

     return RST_XOS_SS(XOS_TimerBegin(mid,tHandle,uiLen));
}

/*********************************************************************
函数名称 : TimerStopCps
功能描述 : 停止定时器,在停止操作中要调用平台地停止操作。平台其实相当一个删除操作

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo TimerStopCps(MDID mid, PTIMER pTimer)
{
    return RST_XOS_SS(XOS_TimerEnd(mid,pTimer));
}

/*********************************************************************
函数名称 : TimerDeleteCps
功能描述 : 删除定时器,先停止再放入空闲链中

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/

inline EerrNo TimerDeleteCps(MDID mid,PTIMER pTimer)
{
    return RST_XOS_SS(XOS_TimerDelete(mid,pTimer));
}

/*********************************************************************
函数名称 : TimerNumRegCps
功能描述 : 定时器注册函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline EerrNo  TimerNumRegCps( MDID mId, XU32 uiMaxTimerNum)
{
    return  RST_XOS_SS(XOS_TimerReg(mId, LOC_LOWTIMER_CLCUNIT,uiMaxTimerNum, 0));
}


typedef e_TIMERTYPE TimerType;

struct CTm
{
    XU32 tm_sec;     // 秒 – 取值区间为[0,59]
    XU32 tm_min;     // 分 - 取值区间为[0,59]
    XU32 tm_hour;    // 时 - 取值区间为[0,23]
    XU32 tm_mday;    // 一个月中的日期 - 取值区间为[1,31]
    XU32 tm_mon;     // 月份（从一月开始，取值区间为[1,12]
    XU32 tm_year;    // 当前年份，比如当前是2006
    XU32 tm_wday;    // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推
    XU32 tm_yday;    // 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推
    XU32 tm_isdst;   // 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的tm_isdst为0；不了解情况时，tm_isdst()为负。
};

/*********************************************************************
函数名称 :
功能描述 : 获得格林尼治时间 GetStandTimer()

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID BuildCTm(CTm& refCtm,const tm& refSysTime)
{
    XOS_MemCpy(&refCtm, &refSysTime, sizeof(struct tm));
    refCtm.tm_year += 1900;    //tm中的时间的年份是与1900的差值
    refCtm.tm_mon  += 1;       //tm中的月份
}

/*********************************************************************
函数名称 :
功能描述 : 获得格林尼治时间 GetStandTimer()

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID BuildTm(tm& refSysTime, const CTm& refCtm)
{
    XOS_MemCpy(&refSysTime, &refCtm, sizeof(struct tm));
    refSysTime.tm_year -=1900;    //tm中的时间的年份是与1900的差值
    refSysTime.tm_mon  -=1;       //tm中的月份
}

/*********************************************************************
函数名称 :
功能描述 :获得系统从1970.01.01.0.0开始到现在所经历的秒数(Microsoft C/C++ 7.0
          中时间点的值是从1899年12月31日0时0分0秒到该时间点
          所经过的秒数)，该值是一个ULONG类型的，在2038年1月18日19时14分07秒将溢出

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline t_XOSTT GetSecondTime()
{
    return (t_XOSTT)time(XNULL);
}

/*********************************************************************
函数名称 : GetClock
功能描述 : 获得系统从开启这个程序进程到程序中调用clock()函数时之间的
           CPU时钟计时单元.CPU时钟计时单元与CPU的类型有关. X86是1ms为
           一个时钟计时单元。
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline t_XOSCT GetClock()
{
    t_XOSCT clockt = 0;
    (XVOID)XOS_Clock(&clockt);

    return clockt;
}

/*********************************************************************
函数名称: GetLocalTime
功能描述: 获得局部时间，这与系统设置的时区有关系,比如我们机器上的是北京时间。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID  GetLocalTime(CTm& refCtm, t_XOSTT ulSecond)
{
    t_XOSTT scondtime = static_cast<t_XOSTT>(ulSecond);
    struct tm *local = localtime((const time_t *) &scondtime);
    BuildCTm(refCtm, *local);
}

/*********************************************************************
函数名称 : GetLocalTime
功能描述 : 获得局部时间，这与系统设置的时区有关系,比如我们机器上的是北京时间。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID  GetLocalTime(CTm& refCtm)
{
    GetLocalTime(refCtm,GetSecondTime());
}

/*********************************************************************
函数名称 : GetRunTime
功能描述 : 获得系统运行的时间，这与系统设置的时区有关系,比如我们机器上的是北京时间。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID GetRunTime(CTm& refCtm, t_XOSTT ulSecond)
{
    t_XOSTT scondtime = static_cast<t_XOSTT>(ulSecond);
    struct tm *local = localtime((const time_t *) &scondtime);
    BuildCTm(refCtm, *local);
}

/*********************************************************************
函数名称 : GetRunTime
功能描述 : 获得系统运行的时间，这与系统设置的时区有关系,比如我们机器上的是北京时间。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID GetRunTime(CTm& refCtm)
{
    GetRunTime(refCtm, XOS_TicksToSec(XOS_GetSysTicks()));
}

/*********************************************************************
函数名称 : SetTraceLevelByFid
功能描述 : 设置默认TraceLevel。

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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



