#ifndef __DIAM_PARSERAVP_H__
#define __DIAM_PARSERAVP_H__

#include <parser/diam_dictlib.h>
#include <parser/diam_parser.h>
#include <parser/diam_avpheader.h>

class CDiamAvpRawData
{
public:
    union
    {
        DiamMsgBlock *msg;
        CDiamAvpHeaderList *ahl;
    };
};
//avp ͷ������
typedef CDiamParser<CDiamAvpRawData*, DiamAvpHeader*, CDictObjectAvp*> CDiamAvpHeaderParser;
//avp ������
typedef CDiamParser<CDiamAvpRawData*, CAvpContainer*, CDictObjectAvp*> CDiamAvpParser;

#endif // __DIAM_PARSERAVP_H__
