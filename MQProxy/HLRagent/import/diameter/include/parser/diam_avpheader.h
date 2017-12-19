/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月7日
**************************************************************************/
#ifndef __AVP_HEADER_H__
#define __AVP_HEADER_H__

#include <list>
#include <string>
#include <parser/diam_parser.h>
#include <parser/diam_errstatus.h>

/*矫正4字节边界*/
#define adjust_word_boundary(len)  ((DiamUINT32)((((len-1)>>2)+1)<<2))

class CDiamAvpHeaderList : public std::list<DiamAvpHeader>
{
public:
    CDiamAvpHeaderList() {}

    void create(DiamMsgBlock *aBuffer);
};

#endif // __AVP_HEADER_H__


