/************************************************************************************
�ļ���  :cpp_number.h

�ļ�����:�����������ӿ�ͷ�ļ�

����    :hulianzhuang,@XINWEI.

��������:2006/06/01

�޸ļ�¼:
         2006-06-01,hulianzhuang,�����ļ�
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

//�����ŷ�ʽ
enum eNumberType
{
    eBCD,
    eCHAR,
    eINT
};


class CNumber
{
public:

    //���캯��
    //CNumber();

    XU32 CheckLength();

    XVOID Init()
    {
        this->Empty();
    }

    /*********************************************************************
    �������� :
    �������� : �������캯��

    �������� : const XU8* pSrcNumber       Դ���봮
               XU32 nLength                Դ���봮����
               eNumberType eType           Դ���봮�����ŷ�ʽ
    ������� :
    ����ֵ   :

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool  Init( const XU8* pSrcNumber, XU32 nLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    �������� :
    �������� : �������캯��

    �������� : const CNumber &srcNumber    Դ������
    ������� :
    ����ֵ   :

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XVOID Init(const CNumber &srcNumber);


    /*********************************************************************
    �������� :
    �������� : �ȺŲ�����

    �������� : const CNumber &srcNumber    Դ������
    ������� :
    ����ֵ   :

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    //CNumber& operator=(const CNumber& srcNumber);


    /*********************************************************************
    �������� :
    �������� : ��ָ�����봮�滻ԭ���봮

    �������� : XS32 nIndex              �滻��ʼλ��
               XS32 nLength             �滻�ĳ���,ע:���м�λ�����滻
               const XU8* pSrcNumber    Դ���봮
               XS32 nSrcLength          Դ���봮����,ע:���ó��ȴ����滻����,�����ܳ��Ƚ����䳤,���򽫱����
               eNumberType eType        Դ���봮�����ŷ�ʽ
    ������� :
    ����ֵ   : ���nIndex + nLength ����ʵ�ʺ��볤��,�򷵻�false,��ʾ�滻ʧ��,
               ����ǰ���볤�� - nLength + nSrcLength �������ų�(32) ,�򷵻�false,��ʾ�滻ʧ��,
               ���滻ʧ����˺�������ִ���κβ���
               ����true��ʾ�滻�ɹ�

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Replace(XU32 nIndex,XU32 nLength,const XU8* pSrcNumber,XU32 nSrcLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    �������� :
    �������� : ��ָ���������滻ԭ���봮

    �������� : XS32 nIndex                  �滻��ʼλ��
               XS32 nLength                 �滻�ĳ���,ע:���м�λ�����滻
               const CNumber& srcNumber     Դ������
    ������� :
    ����ֵ   : ���nIndex + nLength ����ʵ�ʺ��볤��,�򷵻�false,��ʾ�滻ʧ��,
               ����ǰ���볤�� - nLength + �滻���볤�� �������ų�(32) ,�򷵻�false,��ʾ�滻ʧ��,
               ���滻ʧ����˺�������ִ���κβ���
               ����true��ʾ�滻�ɹ�

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Replace(XU32 nIndex,XU32 nLength,const CNumber& srcNumber,XU32 &nSucceedLength);


    /*********************************************************************
    �������� :
    �������� : ��ָ����������ӵ�Դ���봮����

    �������� : const CNumber& srcNumber    Դ������
    ������� :
    ����ֵ   : ʵ����ӳɹ��ĳ���,�������쳣�򷵻�-1;

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 Append(const CNumber & srcNumber);


    /*********************************************************************
    �������� :
    �������� : ��ָ����������ӵ�Դ���봮����

    �������� : const XU8* pSrcNumber     Դ���봮
               XU32  nLength             Դ���봮�������
               eNumberType eType         Դ���봮�����ŷ�ʽ
    ������� :
    ����ֵ   : ʵ����ӳɹ��ĳ���,�������쳣�򷵻�-1;

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 Append(const XU8* pSrcNumber,XU32 nLength,eNumberType eType = eBCD);


    /*********************************************************************
    �������� :
    �������� : ��ָ�����봮����ԭ���봮,���볤�Ȼ�䳤

    �������� : XS32 nIndex               �滻��ʼλ��
               const XU8* pSrcNumber     Դ���봮
               XS32 nLength              Դ���봮�������
               eNumberType eType         Դ���봮�����ŷ�ʽ
    ������� :
    ����ֵ   : ���nIndex ����ʵ�ʺ��볤��,�򷵻�false,��ʾ����ʧ��,
               ����ǰ���볤�� - nLength + nSrcLength �������ų�(32) ,�򷵻�false,��ʾ����ʧ��,
               ������ʧ����˺�������ִ���κβ���
               ����true��ʾ�滻�ɹ�

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Insert(XU32 nIndex, const XU8* pSrcNumber,XU32 nLength,XU32 &nSucceedLength,eNumberType eType = eBCD);


    /*********************************************************************
    �������� :
    �������� : ��ָ�����������ԭ���봮,���볤�Ȼ�䳤

    �������� : XS32 nIndex                  �滻��ʼλ��
               XS32 nLength                 �滻�ĳ���,ע:���м�λ�����滻
               const CNumber& srcNumber     Դ������
    ������� :
    ����ֵ   : ���nIndex ����ʵ�ʺ��볤��,�򷵻�false,��ʾ����ʧ��,
               ����ǰ���볤�� - nLength + ������볤�� �������ų�(32) ,�򷵻�false,��ʾ����ʧ��,
               ������ʧ����˺�������ִ���κβ���
               ����true��ʾ�滻�ɹ�

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Insert(XU32 nIndex, const CNumber& srcNumber,XU32 &nSucceedLength);


    /*********************************************************************
    �������� :
    �������� : ɾ��ָ������ָ��λ�õĺ��봮,���볤�Ȼ���

    �������� : XS32 nIndex                  �滻��ʼλ��
               XS32 nLength                 �滻�ĳ���,ע:���м�λ�����滻
    ������� :
    ����ֵ   : ��nIndex + nLength ���� ��ǰ���볤�� ������ʧ��,����ִ���κβ���

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Delete(XU32 nIndex, XU32 nLength = 1);


    /*********************************************************************
    �������� :
    �������� : ��պ��봮

    �������� :
    ������� :
    ����ֵ   :

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XVOID Empty()
    {
        m_nLength = CNUMBER_ZERO;
        XOS_MemSet(m_aryData, 0, MAX_NUMBER_LEN);
    }


    /*********************************************************************
    �������� :
    �������� : ȡ�ú��볤��

    �������� :
    ������� :
    ����ֵ   : ���볤��

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU32 GetLength() const
    {
        return m_nLength;
    }


    /*********************************************************************
    �������� :
    �������� : ���ú��볤��

    �������� :
    ������� :
    ����ֵ   : �����õĺ��볤�ȴ��ڵ�ǰ���볤�Ƚ�����ʧ��

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool SetLength(XU32 nLength)
    {
        if (nLength > m_nLength)
            return false;
        m_nLength = nLength;
        return true;
    }

    /*********************************************************************
    �������� :
    �������� : ȡ��ָ������λ��ֵ

    �������� : XU32 nIndex   ����λ����
    ������� :
    ����ֵ   : �����õĺ��볤�ȴ��ڵ�ǰ���볤�Ƚ�����-1,���򷵻�ָ������λ��ֵ

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    XU8 GetAt(XU32 nIndex) const
    {
        CPP_ASSERT_RV((nIndex < m_nLength), 0xFF);
        return m_aryData[nIndex];
    }


    /*********************************************************************
    �������� :
    �������� : �Ƚϵ�ǰ���봮��ָ�����봮�Ƿ���ͬ

    �������� : const XU8* pOther   ���ڱȽϵĺ��봮
               XU32 nLength        ���ڱȽϵĺ��봮�ĺ������
               eNumberType eType   Դ���봮�����ŷ�ʽ
    ������� : XS32 &nIndex        ���Դ���봮��Ŀ�ĺ��봮����ͬ,��ֵΪ��һ������ͬ�ĺ���λ��,��������ͬ���ֵΪ���봮�ĳ���
    ����ֵ   : ����ͬ�򷵻�true,���򷵻�false

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Compare( const XU8* pOther,XU32 nLength,XS32 &nIndex,eNumberType eType = eBCD);
    bool ComparePart( const XU8* pOther,XU32 nLength,XS32 &nIndex);


    /*********************************************************************
    �������� :
    �������� : �Ƚϵ�ǰ���봮��ָ�����봮�Ƿ���ͬ

    �������� : const CNumber& other���ڱȽϵĺ�����
    ������� : XS32 &nIndex        ���Դ���봮��Ŀ�ĺ��봮����ͬ,��ֵΪ��һ������ͬ�ĺ���λ��,��������ͬ���ֵΪ���봮�ĳ���
    ����ֵ   : ����ͬ�򷵻�true,���򷵻�false

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Compare(const CNumber& other, XS32 &nIndex);
    bool ComparePart(const CNumber& other, XS32 &nIndex);

	inline bool operator==(const CNumber &_right)
	{
		int index = 0;
		return  CompareWithInt( _right.GetBuffer(), _right.GetLength(), index);
	}

    /*********************************************************************
    �������� :
    �������� : �жϺ����Ƿ�Ϊ��

    �������� :
    ������� :
    ����ֵ   : ��Ϊ���򷵻�true,���򷵻�false

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool IsEmpty() const
    {
        return  ( m_nLength == CNUMBER_ZERO);
    }


    /*********************************************************************
    �������� :
    �������� : ���ָ��λ�ú��봮

    �������� : XU32 nIndex          ��ȡ�������ʼλ��
               XU32 nLength         ����ĳ���
               const XU8*pDecBuffer Ŀ�껺����
               XU32 nBufferLength   Ŀ�껺������С
               eNumberType eType    Ŀ����봮�����ŷ�ʽ
    ������� :
    ����ֵ   : ��nIndex + nLength  ���� ��ǰ���볤�� �򷵻�false,nBufferLength ��С�����᷵��false,ת��ʧ�ܻ᷵��false,���򷵻�true

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool GetData(XU32 nIndex, XU32 nLength, XU8*pDecBuffer,XU32 nBufferLength,eNumberType eType = eBCD) const;


    /*********************************************************************
    �������� :
    �������� : ����ڲ�������(���Ƽ�ʹ��!!)(�ڲ��ݶ���UCHAR���������,һ��UCHAR��ʾһλ����,��������Ϊ16������)

    �������� :
    ������� :
    ����ֵ   : �����ڲ���������ַ

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    const XU8* GetBuffer() const
    {
        return m_aryData;
    }


    /*********************************************************************
    �������� :
    �������� : ����ڲ�������(���Ƽ�ʹ��!!)(�ڲ��ݶ���UCHAR���������,һ��UCHAR��ʾһλ����,��������Ϊ16������)

    �������� :
    ������� :
    ����ֵ   : �����ڲ���������ַ

    �޸���ʷ : Author        mm/dd/yy       Initial Writing
    **********************************************************************/
    bool Check( XU32 &nSucceedLength ) const;

private:
    //ASCII��ת��INT
    XU8 ASCII2Int(XU8 c) const
    {
        if( c == '*' )
            return 0x0b;
        if( c == '#' )
            return 0x0c;
        return c - 48;
    }

    //INTת��ASCII��
    XU8 Int2ASCII(XU8 i) const
    {
        if( i == 0x0b )
            return '*';
        if( i == 0x0c )
            return '#';
        return i + 48;
    }

    //����������ѹ����һλBCD��
    XU8 Compress( XU8 h, XU8 l) const
    {
        XU8 dst = 0;
        dst = (h << 4) | l; //h������λ��BCD��ĸ���λ��l��BCD��ĵ���λ
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
    XU32   m_nLength;   //��Ч���볤��
    XU8    m_aryData[MAX_NUMBER_LEN];
};

#endif



