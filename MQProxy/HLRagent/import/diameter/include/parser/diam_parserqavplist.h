#ifndef __DIAM_PARSERQAVPLIST_H__
#define __DIAM_PARSERQAVPLIST_H__

#include <parser/diam_msgbuffer.h>
#include <parser/diam_avpcontainer.h>
#include <parser/diam_dictobject.h>
#include <parser/diam_parser.h>

typedef CDiamParser<DiamMsgBlock*, DiamBody*, CDictObjectCommand*> QualifiedAvpListParser;

#endif //__DIAM_PARSERQAVPLIST_H__

