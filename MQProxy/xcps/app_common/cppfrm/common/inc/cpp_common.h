/************************************************************************************
文件名  :cpp_common.h

文件描述:

作者    :zzt

创建日期:2006/01/19

修改记录:
luoyong    2014/05/19   优化整合到平台

************************************************************************************/
#ifndef __CPP_COMMON_H_
#define __CPP_COMMON_H_

#include "cpp_adapter.h"


/*********************************************************************
名称 : CRWBuff
职责 : 封装向一个缓存中的写入操作
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CRWBuff
{
public:
    CRWBuff()
    {
        m_pBuf        = XNULL;         //缓冲区的起始地址
        m_uiCurIndex  = 0;
        m_uiLen       = 0;             //整个缓冲区的长度
        m_chCPUOrder  = LOCAL_ENDIAN;  //默认为本板cpu字节顺序
    }
    XU32 GetLength()const
    {
        return m_uiLen;
    }
    XVOID SetBuf(XVOID *pvch,XU32 uiLen)
    {
        CPP_ASSERT_RN((pvch != XNULL) && (uiLen != 0));
        SetBuf(pvch);
        m_uiLen      = uiLen;
        m_uiCurIndex = 0;
    }
    XVOID SetBuf(XVOID *pvch)
    {
        m_pBuf = (XU8 *)pvch;
    }
    XVOID SetLen(XU32 uiLen)
    {
        m_uiLen = uiLen;
    }
    XVOID SetCPUOrder(XU8 uchCpuOder)
    {
        m_chCPUOrder = uchCpuOder;
    }
    bool IsSameCPUOder()const
    {
        return LOCAL_ENDIAN == m_chCPUOrder;
    }
    XVOID Reset()
    {
        m_uiCurIndex = 0;
    }
    //向缓冲区的尾端移动当前读写指针
    bool IncrementIndex(XU32 uiStp)
    {
        if(IsOverflow(uiStp))
        {
            return false;
        }
        m_uiCurIndex += uiStp;
        return true;
    }
    //向缓冲区的头部移动当前读写指针
    bool DecrementIndex(XU32 uiStp)
    {
        if(IsLowerOverflow(uiStp))
        {
            return false;
        }
        m_uiCurIndex -= uiStp;
        return true;
    }
    //是否越界.该越界表示是否越过了缓冲区的尾部
    bool IsOverflow(XU32 iLen)const
    {
        //这里为什么要 > ，而不是>=
        //因为m_CurIndex 始终指向下一个将要读写的内存块
        return m_uiCurIndex+iLen > GetLength();
    }
    //是否越界,这里主要表示是否越过了缓冲区的头部
    //这种情况主要发生在回退读写指针时
    bool IsLowerOverflow(XU32 uiStp)const
    {
        return m_uiCurIndex < uiStp;
    }
    XVOID* GetCurRWBuf()
    {
        return m_pBuf + m_uiCurIndex;
    }

protected:
    XU8    *m_pBuf;         //缓冲区的起始地址
    XU32    m_uiCurIndex;   //当前写入的指针
    XU32    m_uiLen;        //整个缓冲区的长度
    XU8     m_chCPUOrder;
};


/*********************************************************************
名称 : CWriter
职责 : 封装向一个缓存中的写入操作
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CWriter : public CRWBuff
{
private:
    template<class T> CWriter& WriterT(T tValue)
    {
        //CPP_ASSERT((XNULL != m_pBuf) && !IsOverflow(sizeof(tValue)));
        if(!IsSameCPUOder())
        {
            SWAP_BYTE_ODER(tValue);//将字节序倒换一下
        }

        // *((T*)GetCurRWBuf())  = tValue;
        // 由于solairs有字节序对齐问题，这里只能用memcpy --modify by liusj
        XOS_MemCpy((XVOID*)GetCurRWBuf(), (XVOID*)&tValue, sizeof(T));

        IncrementIndex(sizeof(T));

        return *this;
    }

public:
    CWriter& operator<<(XU32 uiValue)
    {
        return WriterT(uiValue);
    }
    CWriter& operator<<(XU16 usValue)
    {
        return WriterT(usValue);
    }
    CWriter& operator<<(XU8 ucValue)
    {
        return WriterT(ucValue);
    }

    //写一个32位的值到缓存中
    XVOID Write(XU32 uiValue)
    {
        (XVOID)WriterT(uiValue);
    }
    //写一个16位的值到缓存中
    XVOID Write(XU16 usValue)
    {
        (XVOID)WriterT(usValue);
    }
    //写一个8位的值到缓存中
    XVOID Write(XU8 ucValue)
    {
        (XVOID)WriterT(ucValue);
    }

    XVOID Write(XU8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)//允许写入0个字节长度
        {
            return ;
        }
        CPP_ASSERT_RN((uiLen != 0) && (pchValue != XNULL));
        CPP_ASSERT_RN((m_pBuf != XNULL)&&!IsOverflow(uiLen));
        XOS_MemCpy(GetCurRWBuf(),pchValue,uiLen);
        IncrementIndex(uiLen);
    }
    XVOID Write(XS8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)//允许写入0个字节长度
        {
            return ;
        }
        CPP_ASSERT_RN((uiLen != 0) &&(pchValue != XNULL));
        CPP_ASSERT_RN( (m_pBuf != XNULL)&&!IsOverflow(uiLen));
        XOS_MemCpy(GetCurRWBuf(),pchValue,uiLen);
        IncrementIndex(uiLen);
    }
};

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> inline T tRead(CRWBuff& refR,T*)
{
    //CPP_ASSERT(!refR.IsOverflow(sizeof(T)));
    //T tmp = *((T*)refR.GetCurRWBuf());//读取当前的内存上的数据
    T tmp;

    //由于solairs有字节序对齐问题，这里只能用memcpy --modify by liusj
    XOS_MemCpy((XVOID*)&tmp, (XVOID*)refR.GetCurRWBuf(), sizeof(T));

    (XVOID)refR.IncrementIndex(sizeof(T));
    return refR.IsSameCPUOder()? tmp:IntegerByteSwap(tmp);
}


/*********************************************************************
名称 : CReader
职责 : 封装向一个缓存中的读操作。该对象用在内部模块的消息流的读写
协作 :
历史 :
       修改者   日期          描述
       对于内部模块之间的消息,其消息格式是结构化的(非TLV格式)，并且
       是不允许越界的(即若发生越界,则表示内部模块发生了错误)。以后
       将该类的名字该为CInnerStrReader
**********************************************************************/
class CReader:public CRWBuff
{
private:
    template<class T>
    CReader& InnerRead(T& refValue)
    {
        refValue = tRead(*this,static_cast<T*>(XNULL));
        return *this;
    }

public:
    CReader& operator>>(XU32& refuiValue)
    {
        return InnerRead(refuiValue);
    }
    CReader& operator>>(XU16& refusValue)
    {
        return InnerRead(refusValue);
    }
    CReader& operator>>(XU8& urefcValue)
    {
        return InnerRead(urefcValue);
    }
    //到缓存中读一个32位的值
    XU32  ReadXU32()
    {
        return tRead(*this,static_cast<XU32*>(XNULL));
    }
    //到缓存中读一个16位的值
    XU16 ReadXU16()
    {
        return tRead(*this,static_cast<XU16*>(XNULL));
    }
    //到缓存中读一个8位的值
    XU8 ReadXU8()
    {
        return tRead(*this,static_cast<XU8*>(XNULL));
    }
    XVOID ReadXU8(XU8 * pchValue,XU32 uiLen)
    {
        CPP_ASSERT_RN((uiLen != 0) &&(pchValue != XNULL));
        CPP_ASSERT_RN(!IsOverflow(uiLen));
        XOS_MemCpy(pchValue,GetCurRWBuf(),uiLen);
        (XVOID)IncrementIndex(uiLen);
    }
};

/*********************************************************************
名称 :
职责 : 封装向一个缓存中的读操作。该对象用在读取
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class COuterTLVReader:public CRWBuff
{
private:
    template<class T>
    bool InnerRead(T& refValue)
    {
        if(IsOverflow(sizeof(refValue)))
        {
            return false;
        }
        refValue = tRead(*this,static_cast<T*>(XNULL));
        return true;
    }

public:
    bool operator>>(XU32& refuiValue)
    {
        return InnerRead(refuiValue);
    }
    bool operator>>(XS16& refusValue)
    {
        return InnerRead(refusValue);
    }
    bool operator>>(XU16& refusValue)
    {
        return InnerRead(refusValue);
    }
    bool operator>>(XU8& urefcValue)
    {
        return InnerRead(urefcValue);
    }
    //注意:这里允许读写0个字节的长度
    bool ReadXU8(XU8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)
        {
            return true; //若读取0个字节，则直接返回成功
        }
        CPP_ASSERT_RV(((uiLen != 0) &&(pchValue != XNULL)), false);

        if(IsOverflow(uiLen))
        {
            return false; //如果读越界
        }
        XOS_MemCpy(pchValue,GetCurRWBuf(),uiLen);
        (XVOID)IncrementIndex(uiLen);
        return true;
    }

    bool ReadXS8(XS8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)
        {
            return true; //若读取0个字节，则直接返回成功
        }
        CPP_ASSERT_RV(((uiLen != 0) &&(pchValue != XNULL)), false);

        if(IsOverflow(uiLen))
        {
            return false; //如果读越界
        }
        XOS_MemCpy(pchValue,GetCurRWBuf(),uiLen);
        (XVOID)IncrementIndex(uiLen);
        return true;
    }
};

/*********************************************************************
名称 : 读写外部TLV数据失败后的处理宏
职责 :
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
#define FAIL_RETURN_FALSE(rdexpr)                               \
if(!(rdexpr))                                                   \
{                                                               \
    return false;                                               \
}


/*********************************************************************
名称 :
职责 : 初始化读写缓冲
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
#define INIT_CRWBUFF()                                          \
XVOID InitCRWBuff(CRWBuff& wr)                                  \
{                                                               \
    wr.SetBuf(((XU8 *)this)-sizeof(*this), GetDataLen());       \
}

/*********************************************************************
名称 : FORBID_COPY_ASSIGN
职责 : 若一个类禁止拷贝和赋值，则用该宏在类体中声明
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
#define FORBID_COPY_ASSIGN(claName)                            \
private:                                                       \
    claName(const claName& other);                             \
    claName& operator=(const claName& other)


/*********************************************************************
名称 : CBaseCounter
职责 : 基本计数功能.若构造函数的puiBuf为0，这CBaseCounter对象自己分配内存
       否则使用puiBuf所
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
class CSimpleCounter
{
private:
    enum EFreeFlag
    {
        SelfFree,
        OtherFree
    };

public:
    CSimpleCounter(XU32 uiNum,XU32*puiBuf=XNULL)
        :m_uiLen(uiNum)
    {
        //CPP_ASSERT_RN(m_uiLen != 0);

        m_uiLen = uiNum;//赋值长度
        if(puiBuf == XNULL)
        {
            m_puiBuf       = new XU32[m_uiLen];
            m_bSelfFreeBuf = SelfFree;
        }
        else
        {
            m_puiBuf       = puiBuf;
            m_bSelfFreeBuf = OtherFree;
        }
        ClearAll();
    }
    ~CSimpleCounter()
    {
        if(SelfFree == m_bSelfFreeBuf)
        {
            delete[] m_puiBuf;
        }
        m_puiBuf = XNULL;
    }
    //设置某索引处的值
    XVOID Set(XU32 uiIdx, XU32 uiVal)
    {
        CPP_ASSERT_RN(IsValidIndex(uiIdx));
        m_puiBuf[uiIdx] = uiVal;
    }

    //通过索引获得计数值
    XU32 Get(XU32 uiIdx)const
    {
        CPP_ASSERT_RV((IsValidIndex(uiIdx)), 0);
        return m_puiBuf[uiIdx];
    }

    //通过索引将计数值加1
    XVOID Inc(XU32 uiIdx)
    {
        Set(uiIdx,Get(uiIdx)+1);
    }
    //通过引用增加几个值
    XVOID IncNum(XU32 uiIdx, XU32 uiVal)
    {
        Set(uiIdx,Get(uiIdx)+uiVal);
    }
    //清除某个索引上的计数值
    XVOID Clear(XU32 uiIdx)
    {
        Set(uiIdx,0);
    }

    //清除所有的计数值
    XVOID ClearAll()
    {
        XOS_MemSet(m_puiBuf ,0, m_uiLen *sizeof(m_puiBuf[0]));
    }
    XU32 GetLenth()const
    {
        return m_uiLen;
    }
    //打印信息
    XVOID Print()const;

private:
    FORBID_COPY_ASSIGN(CSimpleCounter);

private:
    bool IsValidIndex(XU32 uiIdx)const
    {
        return m_uiLen > uiIdx;
    }

private:
    XU32*       m_puiBuf;
    XU32        m_uiLen;//buf长度
    EFreeFlag   m_bSelfFreeBuf;
};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
class CCmpCounter
{
public:
    CCmpCounter(XU32 uiNum, XU8 *pucShutter = XNULL)
        :m_puiBuf(new XU32[uiNum*3]),m_Master(uiNum,m_puiBuf),m_Cnt0(uiNum,m_puiBuf+uiNum),m_Cnt1(uiNum,m_puiBuf+uiNum+uiNum)
    {
        m_pucShutter = pucShutter;
    }
    ~CCmpCounter()
    {
        delete[] m_puiBuf;
        m_puiBuf     = XNULL;
        m_pucShutter = XNULL;
    }
    XVOID Set(XU32 uiIdx, XU32 uiVal)
    {
        m_Master.Set(uiIdx,uiVal);
    }
    XU32 Get(XU32 uiIdx)const
    {
        return  m_Master.Get(uiIdx);
    }
    XVOID Inc(XU32 uiIdx)
    {
        m_Master.Inc(uiIdx);
        if(IsOperCnt1())
        {
            m_Cnt1.Inc(uiIdx);
        }
        else
        {
            m_Cnt0.Inc(uiIdx);
        }
    }
    XVOID IncNum(XU32 uiIdx, XU32 uiVal)
    {
        m_Master.IncNum(uiIdx,uiVal);
        if(IsOperCnt1())
        {
            m_Cnt1.IncNum(uiIdx,uiVal);
        }
        else
        {
            m_Cnt0.IncNum(uiIdx,uiVal);
        }
    }
    XU32 GetDelta(XU32 uiIdx)
    {
        XU32 uiDelta = 0;
        if(IsOperCnt1())
        {
            uiDelta = m_Cnt1.Get(uiIdx);
            m_Cnt1.Clear(uiIdx);
        }
        else
        {
            uiDelta = m_Cnt0.Get(uiIdx);
            m_Cnt0.Clear(uiIdx);
        }
        return uiDelta;
    }
    XVOID SetDelta(XU32 uiIdx, XU32 uiDelta)
    {
        if(IsOperCnt1())
        {
            m_Cnt1.Set(uiIdx,uiDelta);
        }
        else
        {
            m_Cnt0.Set(uiIdx,uiDelta);
        }

    }
    XVOID ClearDelta(XU32 uiIdx)
    {
        m_Cnt0.Clear(uiIdx);
        m_Cnt1.Clear(uiIdx);
    }
    XVOID ClearAllDelta()
    {
        m_Cnt0.ClearAll();
        m_Cnt1.ClearAll();
    }
    XVOID Clear(XU32 uiIdx)
    {
        m_Master.Clear(uiIdx);
        m_Cnt0.Clear(uiIdx);
        m_Cnt1.Clear(uiIdx);
    }
    XVOID ClearAll()
    {
        m_Master.ClearAll();
        m_Cnt0.ClearAll();
        m_Cnt1.ClearAll();
    }
    XVOID Print()const
    {

    }
    XU32 GetLenth()const
    {
        return m_Master.GetLenth();
    }

private:
    //禁止拷贝和赋值
    FORBID_COPY_ASSIGN(CCmpCounter);

private:
    //是否操作
    bool IsOperCnt1()const
    {
        return (XNULL != m_pucShutter)&&(0 != *m_pucShutter);
    }

private:
    //这个数据成员一定放在该对象的顶端，这是由C++的初始化顺序决定的.
    XU32*   m_puiBuf;
    XU8*    m_pucShutter;

    CSimpleCounter m_Master;
    CSimpleCounter m_Cnt0;
    CSimpleCounter m_Cnt1;
};


/*********************************************************************
名称 : LessOptChar
职责 : 字串l是否小于字串r(字串l和r是指C风格的)。该仿函数主要用在MAP中,当
       MAP的键是一个CHAR*时
协作 :
历史 :
        修改者     日期          描述
        王雪东    2006.04.17     初稿完成
*********************************************************************/
class LessOptChar
{
public:
    bool operator()(const XS8* pchL, const XS8* pchR)const
    {
        CPP_ASSERT_RV(((pchL != XNULL) && (pchR != XNULL)), false);
        return (XOS_StrCmp(pchL, pchR) < 0);
    }
};


/**********************************************************************
GetSSPathRoot:   获取SS 所在根目录
GetSSPathBin:    获取SS 执行文件所在目录
GetSSPathCfg:    获取SS 配置文件所在目录
GetSSPathLog:    获取SS 日志文件所在目录
GetSSPathDoc:    获取SS 帮助文档所在目录
GetSSPathCache:  获取SS 缓冲文件所在目录
GetSSPathTmp:    获取SS 临时文件所在目录
**********************************************************************/
#define SS_PATH_MAX     MMGT_PATH_MAX

class CPathSS
{
public:
    enum DIR_TYPE
    {
        DIR_ROOT = 0,
        DIR_BIN,
        DIR_CFG,
        DIR_LOG,
        DIR_DOC,
        DIR_CACHE,
        DIR_TMP,
        DIR_CDR
    };
public:
    CPathSS(DIR_TYPE  type, const XS8* filename);
    CPathSS(DIR_TYPE  type);
    CPathSS();

    ~CPathSS()
    {
    }

    operator XS8*()
    {
        return sFileName;
    }
private:

    XVOID InitPathRoot()
    {
        if (XOS_StrLen(sRootPath) == 0)
        {
#ifdef XOS_EW_START
            XOS_GetSysPath(sRootPath, SS_PATH_MAX);
#else
            XOS_StrCpy(sRootPath, "./");
#endif
        }
    }

private:
    static XS8 sRootPath[SS_PATH_MAX];
    static XS8* sDir[];
    XS8 sFileName[SS_PATH_MAX];
};



/*********************************************************************
名称 : cspout
职责 : 打印输出类
协作 :
历史 :
        修改者     日期          描述
        王雪东    2006.04.17     初稿完成
*********************************************************************/
//定义输出模式
#define SS_OUT_TO_NULL            0
#define SS_OUT_TO_SCREEN          1
#define SS_OUT_TO_FILE            2
#define SS_OUT_TO_ALL             3

//输出到日志
#define SS_OUT_TO_LOG             1

//定义一次性输出的字段最大长度
#define MAX_SEG_LEN               200

namespace cspout
{
    //换行标志类
    class endlout
    {
    public:
    };


    //占用字符类
    class setw
    {
    public:
         setw(XU32 uiWidth)
         {
             m_uiWidth = uiWidth;
         }

         XU32 m_uiWidth;
    };

    //打印输出类
    //该类只支持命令行打印，和文件输出。
    //对日志打印输出不支持
    class CPrint
    {
    public:
        //0：不输出  1：打印输出 2：输出到文件
        XU32 m_uiPrintFlg;     //打印输出标志
        XU32 m_uiBlankCnt;     //下一打印输出占用字符数
        XS8 * m_pFileName;

        CLI_ENV* m_pCliEnv;

        FILE *stream ;           //输出文件

    public:
        /*********************************************************************
        函数名称 : CPrint
        功能描述 : 构造函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        //CPrint( XU32 uiPrintFlg ,XS8 * pFileName,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel,XS8 * pOperateMode)

        //打印输出类
        //该类只支持命令行打印，和文件输出。
        //对日志打印输出不支持
        CPrint( XU32 uiPrintFlg ,XS8 * pFileName,XS8 * pOperateMode)
        {
            m_uiBlankCnt = 0;
            m_uiPrintFlg = uiPrintFlg;
            m_pFileName  = pFileName;
            //m_uiModuleId  = uiModuleId;
            //m_ePrintLevel = ePrintLevel;
            stream       = XNULL;
            m_pCliEnv    = XNULL;
            OpenFile(pFileName,pOperateMode);
        }

        //设置输出模式，和输出文件，如果是文件的话以"w+"(即可写又可读)方式输出。
        CPrint( XU32 uiPrintFlg ,XS8 * pFileName)
        {
            m_uiBlankCnt = 0;
            m_uiPrintFlg = uiPrintFlg;
            m_pFileName  = pFileName;
            stream       = XNULL;
            m_pCliEnv    = XNULL;
            OpenFile(pFileName);
        }

        //并不打开文件,可设置输出模式，这种情况下，如果不打开文件，则不能输出到文件。
        CPrint( XU32 uiPrintFlg)
        {
            m_uiBlankCnt = 0;
            m_uiPrintFlg = uiPrintFlg;
            m_pFileName  = XNULL;
            stream       = XNULL;
            m_pCliEnv    = XNULL;
        }

        //并不打开文件,也不输出到屏幕
        CPrint()
        {
            m_uiBlankCnt  = 0;
            m_uiPrintFlg  = SS_OUT_TO_NULL;
            m_pFileName   = XNULL;
            stream        = XNULL;
            m_pCliEnv     = XNULL;
        }

        //打开文件不成功，则返回false,否则返回true
        bool OpenFile(XS8 * pFileName,XS8 * pOperateMode )
        {
            if ((XNULL == pFileName) || (XNULL == pOperateMode))
            {
                return false;
            }

            if(stream != XNULL)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
                stream = XNULL;
                return false;
            }

            if((stream = XOS_Fopen(pFileName,pOperateMode)) == XNULL)
            {
                return false;
            }
            m_pFileName = pFileName;
            return true;
        }

        //默认以w+方式打开文件
        bool OpenFile(XS8 * pFileName)
        {
            if (XNULL == pFileName)
            {
                return false;
            }

            if(stream != XNULL)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
                stream = XNULL;
                return false;
            }

            if((stream = XOS_Fopen(pFileName,"w+")) == XNULL)
            {
                return false;
            }

            m_pFileName = pFileName;
            return true;
        }

        //返回false:文件不存在，返回true:文件存在，且关闭成功
        bool CloseFile()
        {
            if(XNULL != stream )
            {
                XOS_Fclose(stream);
                stream = XNULL;
                return true;
            }
            return false;
        }

        //设置输出控制模式
        XVOID SetPrintMode(XU32 uiPrintFlg)
        {
            m_uiPrintFlg = uiPrintFlg;
        }

        //获取输出控制模式
        int GetPrintMode()
        {
            return m_uiPrintFlg;
        }

        XVOID SetPrintEnv(CLI_ENV* pCliEnv)
        {
            if(XNULL !=pCliEnv)
            {
               m_pCliEnv = pCliEnv;
            }
        }


        bool GetFileName(XS8 * pFileName)
        {
            if (XNULL == pFileName)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
               return false;
            }
            pFileName =  m_pFileName;
            return true;
        }

        /*********************************************************************
        函数名称 : ~CPrint
        功能描述 : 析构函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        ~CPrint()
        {
            CloseFile();
            m_pFileName = XNULL;
            m_pCliEnv = XNULL;
        }

        /*********************************************************************
        函数名称 : operator<<(XS8 * p)
        功能描述 : 运算符重载函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        CPrint & operator<<(XS8 * p)
        {
            //SS_ASSERT(XNULL != p);
            XU32 uiTempLen = XOS_StrLen(p);
            XU32 uiBlankLen = 0;
            XU32 i = 0;
            if(m_uiBlankCnt > uiTempLen)
            {
                uiBlankLen = m_uiBlankCnt - uiTempLen;
            }

            if(SS_OUT_TO_SCREEN == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                   XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv, p);
            }

            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( p, sizeof( char ), XOS_StrLen(p), stream );
                    fflush(stream);
                }
            }

            if(SS_OUT_TO_ALL == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                   XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv, p);

                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( p, sizeof( char ), XOS_StrLen(p), stream );
                    fflush(stream);
                }
            }
            m_uiBlankCnt = 0;
            return *this;
        }

        CPrint & operator<<(XS32 t)
        {
            XS8 list[100] ;
            XU32 i = 0;
            XU32 len = sprintf( list,  "%d", t );

            XU32 uiBlankLen = 0;
            if(m_uiBlankCnt > len)
            {
                uiBlankLen = m_uiBlankCnt - len;
            }

            //输出到屏幕
            if(SS_OUT_TO_SCREEN == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                    XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%d",t);
            }

            //输出到文件
            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {

                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }

            //输出到屏幕和文件
            if(SS_OUT_TO_ALL == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                    XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%d",t);

                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }
            m_uiBlankCnt = 0;
            return *this;
        }

        CPrint & operator<<(XU32 t)
        {
            XS8 list[100] ;
            XU32 i = 0;
            XU32 len = sprintf( list,  "%d", t );

            XU32 uiBlankLen = 0;
            if(m_uiBlankCnt > len)
            {
                uiBlankLen = m_uiBlankCnt - len;
            }

            //输出到屏幕
            if(SS_OUT_TO_SCREEN == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                    XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%u",t);
            }

            //输出到文件
            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }

            //输出到屏幕和文件
            if(SS_OUT_TO_ALL == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                   XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%u",t);

                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }

            m_uiBlankCnt = 0;
            return *this;
        }

        CPrint & operator<<(endlout& pendl)
        {
            *this<<"\r\n";
            return *this;
        }

        CPrint & operator<<(const setw& set)
        {
            m_uiBlankCnt = set.m_uiWidth;
            return *this;
        }
    };

    /****************************************
     打印输出到日志和文件
    ****************************************/
    class CPrintInfo
    {
    public:
        //0：不输出  1：打印输出 2：输出到文件
        XU32 m_uiPrintFlg;       //打印输出标志
        XU32 m_uiBlankCnt;       //下一打印输出占用字符数
        XS8 * m_pFileName;
        //采用打日志模式打印
        XU32 m_uiModuleId;       //模块ID
        e_PRINTLEVEL  m_ePrintLevel; //打印输出级别

        CLI_ENV* m_pCliEnv;

        FILE *stream ;           //输出文件

    public:
        /*********************************************************************
        函数名称 : CPrintInfo
        功能描述 : 构造函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        //CPrint( XU32 uiPrintFlg ,XS8 * pFileName,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel,XS8 * pOperateMode)

        //打印输出类
        //对日志打印输出支持
        CPrintInfo( XU32 uiPrintFlg ,XS8 * pFileName,XS8 * pOperateMode,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel = PL_ERR)
        {
            m_uiBlankCnt  = 0;
            m_uiPrintFlg  = uiPrintFlg;
            m_pFileName   = pFileName;

            stream        = XNULL;
            m_pCliEnv     = XNULL;
            OpenFile(pFileName,pOperateMode);
            m_uiModuleId  = uiModuleId;
            m_ePrintLevel = ePrintLevel;
        }

        CPrintInfo( XU32 uiPrintFlg ,XS8 * pFileName,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel = PL_ERR)
        {
            m_uiBlankCnt  = 0;
            m_uiPrintFlg  = uiPrintFlg;
            m_pFileName   = pFileName;

            stream        = XNULL;
            m_pCliEnv     = XNULL;
            OpenFile(pFileName);
            m_uiModuleId  = uiModuleId;
            m_ePrintLevel = ePrintLevel;
        }


        //打开文件不成功，则返回false,否则返回true
        bool OpenFile(XS8 * pFileName,XS8 * pOperateMode )
        {
            if ((XNULL == pFileName) || (XNULL == pOperateMode))
            {
                return false;
            }

            if(stream != XNULL)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
                stream = XNULL;
                return false;
            }

            if((stream = XOS_Fopen(pFileName,pOperateMode)) == XNULL)
            {
                return false;
            }
            m_pFileName = pFileName;
            return true;
        }

        //默认以w+方式打开文件
        bool OpenFile(XS8 * pFileName)
        {
            if (XNULL == pFileName)
            {
                return false;
            }

            if(stream != XNULL)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
                stream = XNULL;
                return false;
            }

            if((stream = XOS_Fopen(pFileName,"w+")) == XNULL)
            {
                return false;
            }

            m_pFileName = pFileName;
            return true;
        }

        //返回false:文件不存在，返回true:文件存在，且关闭成功
        bool CloseFile()
        {
            if(XNULL != stream )
            {
                XOS_Fclose(stream);
                stream = XNULL;
                return true;
            }
            return false;
        }

        //设置输出控制模式
        XVOID SetPrintMode(XU32 uiPrintFlg)
        {
            m_uiPrintFlg = uiPrintFlg;
        }

        //获取输出控制模式
        int GetPrintMode()
        {
            return m_uiPrintFlg;
        }

        XVOID SetPrintEnv(CLI_ENV* pCliEnv)
        {
            if(XNULL !=pCliEnv)
            {
               m_pCliEnv = pCliEnv;
            }
        }

        bool GetFileName(XS8 * pFileName)
        {
            if (XNULL == pFileName)
            {
                return false;
            }

            if(0 == strlen(pFileName))
            {
               return false;
            }
            pFileName =  m_pFileName;
            return true;
        }

        /*********************************************************************
        函数名称 : ~CPrintInfo
        功能描述 : 析构函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        ~CPrintInfo()
        {
            CloseFile();
            m_pFileName = XNULL;
            m_pCliEnv = XNULL;
        }

        /*********************************************************************
        函数名称 : operator<<(XS8 * p)
        功能描述 : 运算符重载函数

        参数输入 :
        参数输出 :
        返回值   :

        修改历史 : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        CPrintInfo & operator<<(XS8 * p)
        {
            XU32 uiTempLen  = XOS_StrLen(p);
            XU32 uiBlankLen = 0;
            XU32 i = 0;
            XU32 uiMaxLen = uiTempLen;
            if(m_uiBlankCnt > uiTempLen)
            {
                uiBlankLen = m_uiBlankCnt - uiTempLen;
                uiMaxLen =  m_uiBlankCnt;
            }

            if(SS_OUT_TO_LOG == m_uiPrintFlg)
            {
                if(uiMaxLen < MAX_SEG_LEN)
                {
                    XS8 tmp[MAX_SEG_LEN] = {0};
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        //PrintInfo(PA(m_uiModuleId, PL_ERR),  " ");
                        XOS_StrCat(tmp, " ");
                    }
                    //PrintInfo(PA(m_uiModuleId, PL_ERR),  p);
                    XOS_StrCat(tmp, p);
                    XOS_CliInforPrintf(tmp);
                }
                else
                {
                    XOS_CliInforPrintf(p);
                }

            }

            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( p, sizeof( char ), XOS_StrLen(p), stream );
                    fflush(stream);
                }
            }

            m_uiBlankCnt = 0;
            return *this;
        }

        CPrintInfo & operator<<(XS32 t)
        {
            XS8 list[100] ;
            XU32 i = 0;
            XU32 len = sprintf( list,  "%d", t );

            XU32 uiBlankLen = 0;
            if(m_uiBlankCnt > len)
            {
                uiBlankLen = m_uiBlankCnt - len;
            }

            //输出到日志
            if(SS_OUT_TO_LOG == m_uiPrintFlg)
            {
                if(uiBlankLen < MAX_SEG_LEN)
                {
                    XS8 tmp[MAX_SEG_LEN] = {0};
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        //PrintInfo(PA(m_uiModuleId, PL_ERR),  " ");
                        XOS_StrCat(tmp, " ");
                    }
                    //PrintInfo(PA(m_uiModuleId, PL_ERR),  p);
                    XOS_CliInforPrintf(tmp);

                }
                XOS_CliInforPrintf("%d",t);
            }

            //输出到文件
            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {

                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }

            m_uiBlankCnt = 0;
            return *this;
        }

        CPrintInfo & operator<<(XU32 t)
        {
            XS8 list[100] ;
            XU32 i = 0;
            XU32 len = sprintf( list,  "%d", t );

            XU32 uiBlankLen = 0;
            if(m_uiBlankCnt > len)
            {
                uiBlankLen = m_uiBlankCnt - len;
            }

            //输出到日志
            if(SS_OUT_TO_LOG == m_uiPrintFlg)
            {
                if(uiBlankLen < MAX_SEG_LEN)
                {
                    XS8 tmp[MAX_SEG_LEN] = {0};
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        //PrintInfo(PA(m_uiModuleId, PL_ERR),  " ");
                        XOS_StrCat(tmp, " ");
                    }
                    //PrintInfo(PA(m_uiModuleId, PL_ERR),  p);
                    XOS_CliInforPrintf(tmp);

                }
                XOS_CliInforPrintf( "%u",t);
            }

            //输出到文件
            if(SS_OUT_TO_FILE == m_uiPrintFlg)
            {
                if(XNULL != stream )
                {
                    for( i = 0; i< uiBlankLen; i++)
                    {
                        XOS_Fwrite( " ", sizeof( char ), 1, stream );
                    }
                    XOS_Fwrite( list, sizeof(XS8),len, stream );
                    fflush(stream);
                }
            }

            m_uiBlankCnt = 0;
            return *this;
        }

        CPrintInfo & operator<<(endlout& pendl)
        {
            *this<<"\r\n";
            return *this;
        }

        CPrintInfo & operator<<(const setw& set)
        {
            m_uiBlankCnt = set.m_uiWidth;
            return *this;
        }
    };


    extern endlout endl;
}



#endif


