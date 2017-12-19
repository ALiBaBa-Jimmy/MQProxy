
#include "cpp_number.h"


XU32 CNumber::CheckLength()
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN, 0);

    XU32 i = 0;
    for( i = 0 ; i < m_nLength ; i++)
    {
        if(m_aryData[i]>0x0c)
        {
            m_nLength = i;
            return m_nLength;
        }
    }
    return i;
}

bool CNumber:: Init( const XU8* pSrcNumber,XU32 nLength,XU32 &nSucceedLength,eNumberType eType /*= eBCD*/)
{
    Empty();

    return Replace(0,  0,  pSrcNumber, nLength,  nSucceedLength,  eType);
}

XVOID CNumber::Init(const CNumber &srcNumber)
{
    XU32  nSucceedLength = 0;

    Empty();

    Replace(0, 0, srcNumber, nSucceedLength);
}


XU32 CNumber::BCDToNumberString( XU8 * pNumberString ,const XU8 * pBCD,XU32 nBCDLength) const
{
    XU8 h;//高4位
    XU8 l;//低4位

    //begin zhangbotao modify for PC-LINT 2006-8-29
    if((NULL == pNumberString) || (NULL == pBCD))
    {
        return 0;
    }

    for( XU32 i = 0;i< nBCDLength/2;i++)
    {
        UnCompress(pBCD[i],h, l);

        if(l > 0x0c )
            return i * 2;

        pNumberString[  i * 2] = l;

        if( h > 0x0c )
            return i * 2 +1;

        pNumberString[  i * 2 +1] = h;
    }

    if( nBCDLength % 2 == 1)
    {
        UnCompress(pBCD[ nBCDLength/2  ],h, l);
        pNumberString[ nBCDLength - 1] = l;
    }
    return nBCDLength ;
}

XU32 CNumber::CharToNumberString( XU8 * pNumberString ,const XU8 * pChar,XU32 nBCDLength) const
{
    if((NULL == pNumberString) || (NULL == pChar))
    {
        return 0;
    }

    XU32 i = 0;
    for(i = 0;i< nBCDLength;i++)
    {
        if( ( pChar[i] >= '0' && pChar[i] <= '9' )
            ||pChar[i] == '*'
            ||pChar[i] == '#')
        {
            pNumberString[  i ] = ASCII2Int(pChar[i]);
        }
        else
        {
            return i;
        }
    }
    return i;
}

bool CNumber::Replace(XU32 nIndex,XU32 nLength,const XU8* pSrcNumber,XU32 nSrcLength,XU32 &nSucceedLength,eNumberType eType /*= eBCD*/)
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if( nIndex + nLength > m_nLength )
    {
        nSucceedLength = 0;
        //CPP_ASSERT_RV(false,false);
        return false;
    }

    nSucceedLength = nSrcLength;

    if(NULL == pSrcNumber)
    {
        return false;
    }


    if(( m_nLength - nLength) + nSrcLength > MAX_NUMBER_LEN )

    {
        nSucceedLength = (MAX_NUMBER_LEN - m_nLength) - nLength;
    }

    //BYTE buffLastNumber[MAX_NUMBER_LEN];//用于拷贝替换段后面的号码
    //UINT nLastNumberLength = m_nLength - nIndex - nLength;//替换段后面的号码长度
    XU8 buffLastNumber[MAX_NUMBER_LEN+1] = "0";
    XU32 nLastNumberLength = (m_nLength - nIndex) - nLength;


    if( nLastNumberLength > CNUMBER_ZERO )
        XOS_MemCpy(buffLastNumber, m_aryData + nIndex + nLength , nLastNumberLength );

    if( eType == eBCD)
    {
        nSucceedLength =  BCDToNumberString( m_aryData + nIndex,pSrcNumber,nSucceedLength);
    }
    else if( eType == eCHAR )
    {
        nSucceedLength =  CharToNumberString( m_aryData + nIndex,pSrcNumber,nSucceedLength);
    }
    else if( eType == eINT )
    {
        nSucceedLength = NumberStringToNumberString(m_aryData + nIndex, pSrcNumber, nSucceedLength);
    }

    if( nLastNumberLength > CNUMBER_ZERO )
        XOS_MemCpy(m_aryData + nIndex + nSucceedLength, buffLastNumber , nLastNumberLength );


    m_nLength = (m_nLength - nLength) + nSucceedLength;
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if( nSucceedLength < nSrcLength )
    {
        return false;
    }

    return true ;
}

bool CNumber::Replace(XU32 nIndex,XU32 nLength,const CNumber& srcNumber,XU32 &nSucceedLength)
{
    return Replace(nIndex,nLength,srcNumber.GetBuffer(),srcNumber.GetLength(),nSucceedLength,eINT);
}

XU32 CNumber::Append(const CNumber & srcNumber)
{

    XU32 nResult;

    Replace(m_nLength,CNUMBER_ZERO,srcNumber.GetBuffer(),srcNumber.GetLength(),nResult,eINT) ;
    return nResult;
}

XU32 CNumber::Append(const XU8* pSrcNumber,XU32 nLength,eNumberType eType /*= eBCD*/)
{
    XU32 nResult;

    Replace(m_nLength,CNUMBER_ZERO,pSrcNumber,nLength,nResult,eType) ;
    return nResult;
}

bool CNumber::Insert(XU32 nIndex, const XU8* pSrcNumber,XU32 nLength,XU32 &nSucceedLength,eNumberType eType /*= eBCD*/)
{
    return Replace(nIndex,CNUMBER_ZERO,pSrcNumber,nLength,nSucceedLength,eType);
}

bool CNumber::Insert(XU32 nIndex, const CNumber& srcNumber,XU32 &nSucceedLength)
{
    return Replace(nIndex,CNUMBER_ZERO,srcNumber.GetBuffer(),srcNumber.GetLength(),nSucceedLength,eINT);
}

bool CNumber::Delete(XU32 nIndex, XU32 nLength /*= 1*/)
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);


  //问题单号21709
  //允许删除长度为0，对内存移动的移动长度修改 yuxiao 061110
  //如果删除长度和索引大于号码长度，可能会导致内存越界。
    if((nIndex + nLength) > m_nLength)
    {
        return false;
    }

    // modify luoy    2011.9.14
/*  if((nIndex + nLength) < m_nLength)
  {
     SYS_MEMMOVE(m_aryData + nIndex,  m_aryData + nIndex+nLength, ((m_nLength - nLength)-nIndex));
  };  */


    if((nIndex + nLength) <= m_nLength)
    {
        XOS_MemMove(m_aryData + nIndex,  m_aryData + nIndex+nLength, (MAX_NUMBER_LEN - nLength-nIndex));
    }

    m_nLength = m_nLength - nLength;

    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    return true;

}

bool CNumber::Compare( const XU8* pOther,XU32 nLength,XS32 &nIndex,eNumberType eType /*= eBCD*/)
{

    nIndex = 0;

    if(NULL == pOther)
    {
        return false;
    }

    if( eType == eBCD)
    {
         return  CompareWithBCD( pOther, nLength,nIndex );
    }
    else if( eType == eCHAR)
    {
        return CompareWithChar( pOther, nLength ,nIndex );
    }
    else if(  eType == eINT)
    {
        return  CompareWithInt( pOther, nLength ,nIndex  );
    }
    return false;
}


bool CNumber::ComparePart( const XU8* pOther,XU32 nLength,XS32 &nIndex)
{
    nIndex = 0;
    if(NULL == pOther)
    {
        return false;
    }

    if( m_nLength <= nLength )  //目前的number个数要比原来的大
    {
        return false;
    }

    XU32 i = 0;
    for(i = 0;i< nLength;i++)
    {
        if( m_aryData[  i ] != pOther[i])
        {
             nIndex =  (XS32)i;
             return false;
        }
    }

    nIndex =  (XS32)i;

    return true;
}


bool CNumber::Compare(const CNumber& other, XS32 &nIndex)
{
    return Compare(other.GetBuffer(), other.GetLength(), nIndex, eINT);

}

bool CNumber::ComparePart(const CNumber& other, XS32 &nIndex)
{
    return ComparePart(other.GetBuffer(), other.GetLength(), nIndex);
}

bool CNumber::GetData(XU32 nIndex, XU32 nLength, XU8*pDecBuffer,XU32 nBufferLength,eNumberType eType /*= eBCD*/) const
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if(NULL == pDecBuffer)
    {
        return false;
    }

    bool result;

    if( nIndex + nLength > m_nLength )
    {
        return false;
    }

    switch( eType )
    {
    case eBCD :
        if( nLength % 2 == 1 )
        {
            if( nBufferLength < ( nLength / 2 + 1) )
            {
                result = false;
            }
            else
            {
                NumberStringTOBCD(pDecBuffer, m_aryData + nIndex, nLength);

                result = true;
            }
        }
        else
        {
            if( nBufferLength < ( nLength / 2 ) )
            {
                result = false;
            }
            else
            {
                NumberStringTOBCD(pDecBuffer, m_aryData + nIndex, nLength);
                result = true;
            }
        }
        break;
    case eCHAR:
        if( nBufferLength < nLength )
        {
            result = false;
        }
        else
        {
            NumberStringToChar(pDecBuffer, nBufferLength,m_aryData + nIndex, nLength);
            result = true;
        }
        break;
    case eINT:
        if( nBufferLength < nLength )
        {
            result = false;
        }
        else
        {
            XOS_MemCpy(pDecBuffer, m_aryData + nIndex, nLength);
            result = true;
        }
        break;
    default:

        result = false;
    }
    return result;
}

bool CNumber::CompareWithBCD( const XU8 * pBCD, XU32  nBCDLength, XS32 &nIndex)
{
    XU8 h;//高4位
    XU8 l;//低4位
    nIndex = 0;

    m_nLength = m_nLength;

    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if(NULL == pBCD)
    {
        return false;
    }

    for( XU32 i = 0;i< nBCDLength/2;i++)
    {
        UnCompress(pBCD[i],h, l);
        if( m_aryData[  i *2 ] != l)
        {
            nIndex = (XS32)i *2;
            return false;
        }

        if( m_aryData[  i * 2 +1] != h)
        {
            nIndex = (XS32)i *2 +1;
            return false;
        }
    }
    if( nBCDLength % 2 == 1)
    {
        UnCompress(pBCD[ nBCDLength/2 + 1 ],h, l);
        if( m_aryData[ nBCDLength -1 ] != l)
        {
            nIndex = (XS32)nBCDLength -1;
            return false;
        }
    }

    nIndex = (XS32)nBCDLength;
    if( nIndex == (XS32)m_nLength )
    {
    	return true;
    }
    return false;

}

bool CNumber::CompareWithChar(const XU8 * pChar, XU32  nBCDLength, XS32 &nIndex)
{
    m_nLength = m_nLength;
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if(NULL == pChar)
    {
        return false;
    }

    XU32 i = 0;
    for(i = 0;i< nBCDLength;i++)
    {
        if( m_aryData[  i ] != ASCII2Int(pChar[i]))
        {
            nIndex =  (XS32)i;
            return false;
        }

    }

    nIndex =  (XS32)i;
    if( nIndex == (XS32)m_nLength )
    {
    	return true;
    }

    return false;
}

bool CNumber::CompareWithInt(const XU8 * pChar, XU32  nBCDLength, XS32 &nIndex)
{
    m_nLength = m_nLength;
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,false);

    if(NULL == pChar)
    {
        return false;
    }

    XU32 i = 0;
    for(i = 0; i < nBCDLength; i++)
    {
        if( m_aryData[i] != pChar[i])
        {
            nIndex =  (XS32)i;
            return false;
        }
    }

    nIndex =  (XS32)i;

    if( nIndex == (XS32)m_nLength )
    {
        return true;
    }
    return false;
}

XU32 CNumber::NumberStringTOBCD( XU8 * pBCD  ,const XU8 * pNumberString,XU32 nLength) const
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,0);
    XU8 h;//高4位
    XU8 l;//低4位

    if((NULL == pBCD) || (NULL == pNumberString))
    {
        return 0;
    }

    for( XU32 i = 0;i< nLength/2;i++)
    {
        l = pNumberString[  i *2 ] ;
        h = pNumberString[  i * 2 +1];

        if( l > 0x0c  )
            return i*2;

        pBCD[i] = Compress( h, l);

        if( h > 0x0c  )
            return i*2 + 1;
    }

    if( nLength % 2 == 1)
    {
        l = pNumberString[ nLength -1 ];

        if( l > 0x0c  )
            return nLength-1;

        pBCD[ nLength/2 ] = Compress(0x0f, l);
    }

    return nLength;
}

XU32 CNumber::NumberStringToChar( XU8 * pChar ,XU32 nBufferLength,const XU8 * pNumberString,XU32 nLength) const
{
    CPP_ASSERT_RV(m_nLength <= MAX_NUMBER_LEN,0);

    if((NULL == pChar) || (NULL == pNumberString))
    {
        return 0;
    }

    XU32 i = 0;
    for(i = 0;i< nLength;i++)
    {
        if( pNumberString[i] > 0x0c  )
        {
            pChar[i] = 0;
            return i;
        }
        pChar[i] = Int2ASCII ( pNumberString[i] );
    }

   // if( nBufferLength >= i )	// nLength == nBufferLength 时会溢出
    if( nBufferLength > i )		// 2012-03-27
           pChar[i] = 0;
    return i;
}

XU32 CNumber::NumberStringToNumberString( XU8 * pDstString ,const XU8 * pSrcString,XU32 nLength) const
{
    if((NULL == pDstString) || (NULL == pSrcString))
    {
        return 0;
    }

    XU32 i = 0;
    for(i = 0;i< nLength;i++)
    {
        if( pSrcString[i] > 0x0c  )
        {
            return i;
        }

        pDstString[i] = pSrcString[i];
    }
    return i;
}

bool CNumber::Check( XU32 &nSucceedLength ) const
{
    nSucceedLength = CNUMBER_ZERO;

    //号码类的长度有效值 应该可以为最大值
    if ( m_nLength > MAX_NUMBER_LEN )
    	return false;

    for( nSucceedLength = CNUMBER_ZERO ; nSucceedLength < m_nLength ; nSucceedLength++ )
    {
    	if( m_aryData[ nSucceedLength ] > 0x0c )
    	{
            return false;
    	}
    }
    return true;
}

