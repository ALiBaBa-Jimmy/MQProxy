/************************************************************************************
文件名  :cpp_number.h

文件描述:定义号码操作接口头文件

作者    :hulianzhuang,@XINWEI.

创建日期:2006/06/01

修改记录:
         2006-06-01,hulianzhuang,创建文件
******************************************************************************/
#ifndef __CPP_NUMBER_H_
#define __CPP_NUMBER_H_

#include "cpp_adapter.h"


#define CNUMBER_ZERO               (0x00)
#define CNUMBER_FAILE              (-1)
#define MAX_NUMBER_LEN             (32)

#define NUMBER_STORAGE_STAR        (0x0b)
#define NUMBER_STORAGE_JING        (0x0c)
#define NUMBER_DISPLAY_STAR        ('*')
#define NUMBER_DISPLAY_JING        ('#')

//号码存放方式
enum eNumberType
{
    eBCD,
    eCHAR,
    eINT
};


class CNumber
{
public:

    //构造函数
    //CNumber();

    XU32 CheckLength();

    XVOID Init()
    {
        this->Empty();
    }

    /*********************************************************************
    函数名称 :
    功能描述 : 拷贝构造函数

    参数输入 : const XU8* pSrcNumber       源号码串
               XU32 nLength                源号码串长度
               eNumberType eType           源号码串号码存放方式
    参数输出 :
    返回值   :

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool  Init( const XU8* pSrcNumber, XU32 nLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    函数名称 :
    功能描述 : 拷贝构造函数

    参数输入 : const CNumber &srcNumber    源号码类
    参数输出 :
    返回值   :

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XVOID Init(const CNumber &srcNumber);


    /*********************************************************************
    函数名称 :
    功能描述 : 等号操作符

    参数输入 : const CNumber &srcNumber    源号码类
    参数输出 :
    返回值   :

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    //CNumber& operator=(const CNumber& srcNumber);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码串替换原号码串

    参数输入 : XS32 nIndex              替换起始位置
               XS32 nLength             替换的长度,注:即有几位将被替换
               const XU8* pSrcNumber    源号码串
               XS32 nSrcLength          源号码串长度,注:若该长度大于替换长度,号码总长度将被变长,否则将被变短
               eNumberType eType        源号码串号码存放方式
    参数输出 :
    返回值   : 如果nIndex + nLength 大于实际号码长度,则返回false,表示替换失败,
               若当前号码长度 - nLength + nSrcLength 大于最大号长(32) ,则返回false,表示替换失败,
               若替换失败则此函数不会执行任何操作
               返回true表示替换成功

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Replace(XU32 nIndex,XU32 nLength,const XU8* pSrcNumber,XU32 nSrcLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码类替换原号码串

    参数输入 : XS32 nIndex                  替换起始位置
               XS32 nLength                 替换的长度,注:即有几位将被替换
               const CNumber& srcNumber     源号码类
    参数输出 :
    返回值   : 如果nIndex + nLength 大于实际号码长度,则返回false,表示替换失败,
               若当前号码长度 - nLength + 替换号码长度 大于最大号长(32) ,则返回false,表示替换失败,
               若替换失败则此函数不会执行任何操作
               返回true表示替换成功

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Replace(XU32 nIndex,XU32 nLength,const CNumber& srcNumber,XU32 &nSucceedLength);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码类添加到源号码串后面

    参数输入 : const CNumber& srcNumber    源号码类
    参数输出 :
    返回值   : 实际添加成功的长度,若出现异常则返回-1;

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 Append(const CNumber & srcNumber);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码类添加到源号码串后面

    参数输入 : const XU8* pSrcNumber     源号码串
               XU32  nLength             源号码串号码个数
               eNumberType eType         源号码串号码存放方式
    参数输出 :
    返回值   : 实际添加成功的长度,若出现异常则返回-1;

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 Append(const XU8* pSrcNumber,XU32 nLength,eNumberType eType = eBCD);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码串插入原号码串,号码长度会变长

    参数输入 : XS32 nIndex               替换起始位置
               const XU8* pSrcNumber     源号码串
               XS32 nLength              源号码串号码个数
               eNumberType eType         源号码串号码存放方式
    参数输出 :
    返回值   : 如果nIndex 大于实际号码长度,则返回false,表示插入失败,
               若当前号码长度 - nLength + nSrcLength 大于最大号长(32) ,则返回false,表示插入失败,
               若插入失败则此函数不会执行任何操作
               返回true表示替换成功

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Insert(XU32 nIndex, const XU8* pSrcNumber,XU32 nLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    函数名称 :
    功能描述 : 用指定号码类插入原号码串,号码长度会变长

    参数输入 : XS32 nIndex                  替换起始位置
               XS32 nLength                 替换的长度,注:即有几位将被替换
               const CNumber& srcNumber     源号码类
    参数输出 :
    返回值   : 如果nIndex 大于实际号码长度,则返回false,表示插入失败,
               若当前号码长度 - nLength + 插入号码长度 大于最大号长(32) ,则返回false,表示插入失败,
               若插入失败则此函数不会执行任何操作
               返回true表示替换成功

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Insert(XU32 nIndex, const CNumber& srcNumber,XU32 &nSucceedLength);


    /*********************************************************************
    函数名称 :
    功能描述 : 删除指定长度指定位置的号码串,号码长度会变短

    参数输入 : XS32 nIndex                  替换起始位置
               XS32 nLength                 替换的长度,注:即有几位将被替换
    参数输出 :
    返回值   : 若nIndex + nLength 大于 当前号码长度 将返回失败,不会执行任何操作

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Delete(XU32 nIndex, XU32 nLength = 1);


    /*********************************************************************
    函数名称 :
    功能描述 : 清空号码串

    参数输入 :
    参数输出 :
    返回值   :

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XVOID Empty()
    {
        m_nLength = CNUMBER_ZERO;
        XOS_MemSet(m_aryData, 0, MAX_NUMBER_LEN);
    }


    /*********************************************************************
    函数名称 :
    功能描述 : 取得号码长度

    参数输入 :
    参数输出 :
    返回值   : 号码长度

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 GetLength() const
    {
        return m_nLength;
    }


    /*********************************************************************
    函数名称 :
    功能描述 : 设置号码长度

    参数输入 :
    参数输出 :
    返回值   : 若设置的号码长度大于当前号码长度将返回失败

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool SetLength(XU32 nLength)
    {
        if (nLength > m_nLength)
            return false;
        m_nLength = nLength;
        return true;
    }

    /*********************************************************************
    函数名称 :
    功能描述 : 取得指定号码位的值

    参数输入 : XU32 nIndex   号码位索引
    参数输出 :
    返回值   : 若设置的号码长度大于当前号码长度将返回-1,否则返回指定号码位的值

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU8 GetAt(XU32 nIndex) const
    {
        CPP_ASSERT_RV((nIndex < m_nLength), 0xFF);
        return m_aryData[nIndex];
    }


    /*********************************************************************
    函数名称 :
    功能描述 : 比较当前号码串和指定号码串是否相同

    参数输入 : const XU8* pOther   用于比较的号码串
               XU32 nLength        用于比较的号码串的号码个数
               eNumberType eType   源号码串号码存放方式
    参数输出 : XS32 &nIndex        如果源号码串和目的号码串不相同,此值为第一个不相同的号码位置,若号码相同则此值为号码串的长度
    返回值   : 若相同则返回true,否则返回false

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Compare( const XU8* pOther,XU32 nLength,XS32 &nIndex,eNumberType eType = eBCD);
    bool ComparePart( const XU8* pOther,XU32 nLength,XS32 &nIndex);


    /*********************************************************************
    函数名称 :
    功能描述 : 比较当前号码串和指定号码串是否相同

    参数输入 : const CNumber& other用于比较的号码类
    参数输出 : XS32 &nIndex        如果源号码串和目的号码串不相同,此值为第一个不相同的号码位置,若号码相同则此值为号码串的长度
    返回值   : 若相同则返回true,否则返回false

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Compare(const CNumber& other, XS32 &nIndex);
    bool ComparePart(const CNumber& other, XS32 &nIndex);

	inline bool operator==(const CNumber &_right)
	{
		int index = 0;
		return  CompareWithInt( _right.GetBuffer(), _right.GetLength(), index);
	}

    /*********************************************************************
    函数名称 :
    功能描述 : 判断号码是否为空

    参数输入 :
    参数输出 :
    返回值   : 若为空则返回true,否则返回false

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool IsEmpty() const
    {
        return  ( m_nLength == CNUMBER_ZERO);
    }


    /*********************************************************************
    函数名称 :
    功能描述 : 获得指定位置号码串

    参数输入 : XU32 nIndex          获取号码的起始位置
               XU32 nLength         号码的长度
               const XU8*pDecBuffer 目标缓冲区
               XU32 nBufferLength   目标缓冲区大小
               eNumberType eType    目标号码串号码存放方式
    参数输出 :
    返回值   : 若nIndex + nLength  大于 当前号码长度 则返回false,nBufferLength 大小不够会返回false,转换失败会返回false,否则返回true

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool GetData(XU32 nIndex, XU32 nLength, XU8*pDecBuffer,XU32 nBufferLength,eNumberType eType = eBCD) const;


    /*********************************************************************
    函数名称 :
    功能描述 : 获得内部缓冲区(不推荐使用!!)(内部暂定用UCHAR来保存号码,一个UCHAR表示一位号码,号码类型为16进制数)

    参数输入 :
    参数输出 :
    返回值   : 返回内部缓冲区地址

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    const XU8* GetBuffer() const
    {
        return m_aryData;
    }


    /*********************************************************************
    函数名称 :
    功能描述 : 获得内部缓冲区(不推荐使用!!)(内部暂定用UCHAR来保存号码,一个UCHAR表示一位号码,号码类型为16进制数)

    参数输入 :
    参数输出 :
    返回值   : 返回内部缓冲区地址

    修改历史 : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Check( XU32 &nSucceedLength ) const;

private:
    //ASCII码转成INT
    XU8 ASCII2Int(XU8 c) const
    {
        if( c == '*' )
            return 0x0b;
        if( c == '#' )
            return 0x0c;
        return c - 48;
    }

    //INT转成ASCII码
    XU8 Int2ASCII(XU8 i) const
    {
        if( i == 0x0b )
            return '*';
        if( i == 0x0c )
            return '#';
        return i + 48;
    }

    //将两个整数压缩成一位BCD码
    XU8 Compress( XU8 h, XU8 l) const
    {
        XU8 dst = 0;
        dst = (h << 4) | l; //h左移四位作BCD码的高四位，l作BCD码的低四位
        return dst;
    }

    XVOID UnCompress( XU8 bcd, XU8 &h, XU8 &l) const
    {
        h = bcd >> 4;
        l  = bcd & 0x0f;
    }

    XU32 BCDToNumberString( XU8 * pNumberString ,const XU8 * pBCD,XU32 nBCDLength) const;
    XU32 CharToNumberString( XU8 * pNumberString ,const XU8 * pChar,XU32 nBCDLength) const;
    XU32 NumberStringTOBCD( XU8 * pBCD  ,const XU8 * pNumberString,XU32 nLength) const;
    XU32 NumberStringToChar( XU8 * pChar ,XU32 nBufferLength,const XU8 * pNumberString,XU32 nLength) const;
    XU32 NumberStringToNumberString( XU8 * pDstString ,const XU8 * pSrcString,XU32 nLength) const;

    bool CompareWithBCD( const XU8 * pBCD, XU32  nBCDLength, XS32 &nIndex);
    bool CompareWithChar(const XU8 * pChar, XU32  nBCDLength, XS32 &nIndex);
    bool CompareWithInt(const XU8 * pChar, XU32  nBCDLength, XS32 &nIndex);

private:
    XU32   m_nLength;   //有效号码长度
    XU8    m_aryData[MAX_NUMBER_LEN];
};

#endif



