/************************************************************************************
�ļ���  :cpp_common.h

�ļ�����:

����    :zzt

��������:2006/01/19

�޸ļ�¼:
luoyong    2014/05/19   �Ż����ϵ�ƽ̨

************************************************************************************/
#ifndef __CPP_COMMON_H_
#define __CPP_COMMON_H_

#include "cpp_adapter.h"


/*********************************************************************
���� : CRWBuff
ְ�� : ��װ��һ�������е�д�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CRWBuff
{
public:
    CRWBuff()
    {
        m_pBuf        = XNULL;         //����������ʼ��ַ
        m_uiCurIndex  = 0;
        m_uiLen       = 0;             //�����������ĳ���
        m_chCPUOrder  = LOCAL_ENDIAN;  //Ĭ��Ϊ����cpu�ֽ�˳��
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
    //�򻺳�����β���ƶ���ǰ��дָ��
    bool IncrementIndex(XU32 uiStp)
    {
        if(IsOverflow(uiStp))
        {
            return false;
        }
        m_uiCurIndex += uiStp;
        return true;
    }
    //�򻺳�����ͷ���ƶ���ǰ��дָ��
    bool DecrementIndex(XU32 uiStp)
    {
        if(IsLowerOverflow(uiStp))
        {
            return false;
        }
        m_uiCurIndex -= uiStp;
        return true;
    }
    //�Ƿ�Խ��.��Խ���ʾ�Ƿ�Խ���˻�������β��
    bool IsOverflow(XU32 iLen)const
    {
        //����ΪʲôҪ > ��������>=
        //��Ϊm_CurIndex ʼ��ָ����һ����Ҫ��д���ڴ��
        return m_uiCurIndex+iLen > GetLength();
    }
    //�Ƿ�Խ��,������Ҫ��ʾ�Ƿ�Խ���˻�������ͷ��
    //���������Ҫ�����ڻ��˶�дָ��ʱ
    bool IsLowerOverflow(XU32 uiStp)const
    {
        return m_uiCurIndex < uiStp;
    }
    XVOID* GetCurRWBuf()
    {
        return m_pBuf + m_uiCurIndex;
    }

protected:
    XU8    *m_pBuf;         //����������ʼ��ַ
    XU32    m_uiCurIndex;   //��ǰд���ָ��
    XU32    m_uiLen;        //�����������ĳ���
    XU8     m_chCPUOrder;
};


/*********************************************************************
���� : CWriter
ְ�� : ��װ��һ�������е�д�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CWriter : public CRWBuff
{
private:
    template<class T> CWriter& WriterT(T tValue)
    {
        //CPP_ASSERT((XNULL != m_pBuf) && !IsOverflow(sizeof(tValue)));
        if(!IsSameCPUOder())
        {
            SWAP_BYTE_ODER(tValue);//���ֽ��򵹻�һ��
        }

        // *((T*)GetCurRWBuf())  = tValue;
        // ����solairs���ֽ���������⣬����ֻ����memcpy --modify by liusj
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

    //дһ��32λ��ֵ��������
    XVOID Write(XU32 uiValue)
    {
        (XVOID)WriterT(uiValue);
    }
    //дһ��16λ��ֵ��������
    XVOID Write(XU16 usValue)
    {
        (XVOID)WriterT(usValue);
    }
    //дһ��8λ��ֵ��������
    XVOID Write(XU8 ucValue)
    {
        (XVOID)WriterT(ucValue);
    }

    XVOID Write(XU8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)//����д��0���ֽڳ���
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
        if(0 == uiLen)//����д��0���ֽڳ���
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> inline T tRead(CRWBuff& refR,T*)
{
    //CPP_ASSERT(!refR.IsOverflow(sizeof(T)));
    //T tmp = *((T*)refR.GetCurRWBuf());//��ȡ��ǰ���ڴ��ϵ�����
    T tmp;

    //����solairs���ֽ���������⣬����ֻ����memcpy --modify by liusj
    XOS_MemCpy((XVOID*)&tmp, (XVOID*)refR.GetCurRWBuf(), sizeof(T));

    (XVOID)refR.IncrementIndex(sizeof(T));
    return refR.IsSameCPUOder()? tmp:IntegerByteSwap(tmp);
}


/*********************************************************************
���� : CReader
ְ�� : ��װ��һ�������еĶ��������ö��������ڲ�ģ�����Ϣ���Ķ�д
Э�� :
��ʷ :
       �޸���   ����          ����
       �����ڲ�ģ��֮�����Ϣ,����Ϣ��ʽ�ǽṹ����(��TLV��ʽ)������
       �ǲ�����Խ���(��������Խ��,���ʾ�ڲ�ģ�鷢���˴���)���Ժ�
       ����������ָ�ΪCInnerStrReader
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
    //�������ж�һ��32λ��ֵ
    XU32  ReadXU32()
    {
        return tRead(*this,static_cast<XU32*>(XNULL));
    }
    //�������ж�һ��16λ��ֵ
    XU16 ReadXU16()
    {
        return tRead(*this,static_cast<XU16*>(XNULL));
    }
    //�������ж�һ��8λ��ֵ
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
���� :
ְ�� : ��װ��һ�������еĶ��������ö������ڶ�ȡ
Э�� :
��ʷ :
       �޸���   ����          ����
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
    //ע��:���������д0���ֽڵĳ���
    bool ReadXU8(XU8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)
        {
            return true; //����ȡ0���ֽڣ���ֱ�ӷ��سɹ�
        }
        CPP_ASSERT_RV(((uiLen != 0) &&(pchValue != XNULL)), false);

        if(IsOverflow(uiLen))
        {
            return false; //�����Խ��
        }
        XOS_MemCpy(pchValue,GetCurRWBuf(),uiLen);
        (XVOID)IncrementIndex(uiLen);
        return true;
    }

    bool ReadXS8(XS8 * pchValue,XU32 uiLen)
    {
        if(0 == uiLen)
        {
            return true; //����ȡ0���ֽڣ���ֱ�ӷ��سɹ�
        }
        CPP_ASSERT_RV(((uiLen != 0) &&(pchValue != XNULL)), false);

        if(IsOverflow(uiLen))
        {
            return false; //�����Խ��
        }
        XOS_MemCpy(pchValue,GetCurRWBuf(),uiLen);
        (XVOID)IncrementIndex(uiLen);
        return true;
    }
};

/*********************************************************************
���� : ��д�ⲿTLV����ʧ�ܺ�Ĵ����
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
#define FAIL_RETURN_FALSE(rdexpr)                               \
if(!(rdexpr))                                                   \
{                                                               \
    return false;                                               \
}


/*********************************************************************
���� :
ְ�� : ��ʼ����д����
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
#define INIT_CRWBUFF()                                          \
XVOID InitCRWBuff(CRWBuff& wr)                                  \
{                                                               \
    wr.SetBuf(((XU8 *)this)-sizeof(*this), GetDataLen());       \
}

/*********************************************************************
���� : FORBID_COPY_ASSIGN
ְ�� : ��һ�����ֹ�����͸�ֵ�����øú�������������
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
#define FORBID_COPY_ASSIGN(claName)                            \
private:                                                       \
    claName(const claName& other);                             \
    claName& operator=(const claName& other)


/*********************************************************************
���� : CBaseCounter
ְ�� : ������������.�����캯����puiBufΪ0����CBaseCounter�����Լ������ڴ�
       ����ʹ��puiBuf��
Э�� :
��ʷ :
       �޸���   ����          ����
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

        m_uiLen = uiNum;//��ֵ����
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
    //����ĳ��������ֵ
    XVOID Set(XU32 uiIdx, XU32 uiVal)
    {
        CPP_ASSERT_RN(IsValidIndex(uiIdx));
        m_puiBuf[uiIdx] = uiVal;
    }

    //ͨ��������ü���ֵ
    XU32 Get(XU32 uiIdx)const
    {
        CPP_ASSERT_RV((IsValidIndex(uiIdx)), 0);
        return m_puiBuf[uiIdx];
    }

    //ͨ������������ֵ��1
    XVOID Inc(XU32 uiIdx)
    {
        Set(uiIdx,Get(uiIdx)+1);
    }
    //ͨ���������Ӽ���ֵ
    XVOID IncNum(XU32 uiIdx, XU32 uiVal)
    {
        Set(uiIdx,Get(uiIdx)+uiVal);
    }
    //���ĳ�������ϵļ���ֵ
    XVOID Clear(XU32 uiIdx)
    {
        Set(uiIdx,0);
    }

    //������еļ���ֵ
    XVOID ClearAll()
    {
        XOS_MemSet(m_puiBuf ,0, m_uiLen *sizeof(m_puiBuf[0]));
    }
    XU32 GetLenth()const
    {
        return m_uiLen;
    }
    //��ӡ��Ϣ
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
    XU32        m_uiLen;//buf����
    EFreeFlag   m_bSelfFreeBuf;
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
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
    //��ֹ�����͸�ֵ
    FORBID_COPY_ASSIGN(CCmpCounter);

private:
    //�Ƿ����
    bool IsOperCnt1()const
    {
        return (XNULL != m_pucShutter)&&(0 != *m_pucShutter);
    }

private:
    //������ݳ�Աһ�����ڸö���Ķ��ˣ�������C++�ĳ�ʼ��˳�������.
    XU32*   m_puiBuf;
    XU8*    m_pucShutter;

    CSimpleCounter m_Master;
    CSimpleCounter m_Cnt0;
    CSimpleCounter m_Cnt1;
};


/*********************************************************************
���� : LessOptChar
ְ�� : �ִ�l�Ƿ�С���ִ�r(�ִ�l��r��ָC����)���÷º�����Ҫ����MAP��,��
       MAP�ļ���һ��CHAR*ʱ
Э�� :
��ʷ :
        �޸���     ����          ����
        ��ѩ��    2006.04.17     �������
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
GetSSPathRoot:   ��ȡSS ���ڸ�Ŀ¼
GetSSPathBin:    ��ȡSS ִ���ļ�����Ŀ¼
GetSSPathCfg:    ��ȡSS �����ļ�����Ŀ¼
GetSSPathLog:    ��ȡSS ��־�ļ�����Ŀ¼
GetSSPathDoc:    ��ȡSS �����ĵ�����Ŀ¼
GetSSPathCache:  ��ȡSS �����ļ�����Ŀ¼
GetSSPathTmp:    ��ȡSS ��ʱ�ļ�����Ŀ¼
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
���� : cspout
ְ�� : ��ӡ�����
Э�� :
��ʷ :
        �޸���     ����          ����
        ��ѩ��    2006.04.17     �������
*********************************************************************/
//�������ģʽ
#define SS_OUT_TO_NULL            0
#define SS_OUT_TO_SCREEN          1
#define SS_OUT_TO_FILE            2
#define SS_OUT_TO_ALL             3

//�������־
#define SS_OUT_TO_LOG             1

//����һ����������ֶ���󳤶�
#define MAX_SEG_LEN               200

namespace cspout
{
    //���б�־��
    class endlout
    {
    public:
    };


    //ռ���ַ���
    class setw
    {
    public:
         setw(XU32 uiWidth)
         {
             m_uiWidth = uiWidth;
         }

         XU32 m_uiWidth;
    };

    //��ӡ�����
    //����ֻ֧�������д�ӡ�����ļ������
    //����־��ӡ�����֧��
    class CPrint
    {
    public:
        //0�������  1����ӡ��� 2��������ļ�
        XU32 m_uiPrintFlg;     //��ӡ�����־
        XU32 m_uiBlankCnt;     //��һ��ӡ���ռ���ַ���
        XS8 * m_pFileName;

        CLI_ENV* m_pCliEnv;

        FILE *stream ;           //����ļ�

    public:
        /*********************************************************************
        �������� : CPrint
        �������� : ���캯��

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        //CPrint( XU32 uiPrintFlg ,XS8 * pFileName,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel,XS8 * pOperateMode)

        //��ӡ�����
        //����ֻ֧�������д�ӡ�����ļ������
        //����־��ӡ�����֧��
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

        //�������ģʽ��������ļ���������ļ��Ļ���"w+"(����д�ֿɶ�)��ʽ�����
        CPrint( XU32 uiPrintFlg ,XS8 * pFileName)
        {
            m_uiBlankCnt = 0;
            m_uiPrintFlg = uiPrintFlg;
            m_pFileName  = pFileName;
            stream       = XNULL;
            m_pCliEnv    = XNULL;
            OpenFile(pFileName);
        }

        //�������ļ�,���������ģʽ����������£���������ļ�������������ļ���
        CPrint( XU32 uiPrintFlg)
        {
            m_uiBlankCnt = 0;
            m_uiPrintFlg = uiPrintFlg;
            m_pFileName  = XNULL;
            stream       = XNULL;
            m_pCliEnv    = XNULL;
        }

        //�������ļ�,Ҳ���������Ļ
        CPrint()
        {
            m_uiBlankCnt  = 0;
            m_uiPrintFlg  = SS_OUT_TO_NULL;
            m_pFileName   = XNULL;
            stream        = XNULL;
            m_pCliEnv     = XNULL;
        }

        //���ļ����ɹ����򷵻�false,���򷵻�true
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

        //Ĭ����w+��ʽ���ļ�
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

        //����false:�ļ������ڣ�����true:�ļ����ڣ��ҹرճɹ�
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

        //�����������ģʽ
        XVOID SetPrintMode(XU32 uiPrintFlg)
        {
            m_uiPrintFlg = uiPrintFlg;
        }

        //��ȡ�������ģʽ
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
        �������� : ~CPrint
        �������� : ��������

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        ~CPrint()
        {
            CloseFile();
            m_pFileName = XNULL;
            m_pCliEnv = XNULL;
        }

        /*********************************************************************
        �������� : operator<<(XS8 * p)
        �������� : ��������غ���

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
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

            //�������Ļ
            if(SS_OUT_TO_SCREEN == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                    XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%d",t);
            }

            //������ļ�
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

            //�������Ļ���ļ�
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

            //�������Ļ
            if(SS_OUT_TO_SCREEN == m_uiPrintFlg)
            {
                for( i = 0; i< uiBlankLen; i++)
                {
                    XOS_CliExtPrintf(m_pCliEnv, " ");
                }
                XOS_CliExtPrintf(m_pCliEnv,"%u",t);
            }

            //������ļ�
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

            //�������Ļ���ļ�
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
     ��ӡ�������־���ļ�
    ****************************************/
    class CPrintInfo
    {
    public:
        //0�������  1����ӡ��� 2��������ļ�
        XU32 m_uiPrintFlg;       //��ӡ�����־
        XU32 m_uiBlankCnt;       //��һ��ӡ���ռ���ַ���
        XS8 * m_pFileName;
        //���ô���־ģʽ��ӡ
        XU32 m_uiModuleId;       //ģ��ID
        e_PRINTLEVEL  m_ePrintLevel; //��ӡ�������

        CLI_ENV* m_pCliEnv;

        FILE *stream ;           //����ļ�

    public:
        /*********************************************************************
        �������� : CPrintInfo
        �������� : ���캯��

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        //CPrint( XU32 uiPrintFlg ,XS8 * pFileName,XU32 uiModuleId,e_PRINTLEVEL ePrintLevel,XS8 * pOperateMode)

        //��ӡ�����
        //����־��ӡ���֧��
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


        //���ļ����ɹ����򷵻�false,���򷵻�true
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

        //Ĭ����w+��ʽ���ļ�
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

        //����false:�ļ������ڣ�����true:�ļ����ڣ��ҹرճɹ�
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

        //�����������ģʽ
        XVOID SetPrintMode(XU32 uiPrintFlg)
        {
            m_uiPrintFlg = uiPrintFlg;
        }

        //��ȡ�������ģʽ
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
        �������� : ~CPrintInfo
        �������� : ��������

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
        **********************************************************************/
        ~CPrintInfo()
        {
            CloseFile();
            m_pFileName = XNULL;
            m_pCliEnv = XNULL;
        }

        /*********************************************************************
        �������� : operator<<(XS8 * p)
        �������� : ��������غ���

        �������� :
        ������� :
        ����ֵ   :

        �޸���ʷ : Author        mm/dd/yy       Initial Writing
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

            //�������־
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

            //������ļ�
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

            //�������־
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

            //������ļ�
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


