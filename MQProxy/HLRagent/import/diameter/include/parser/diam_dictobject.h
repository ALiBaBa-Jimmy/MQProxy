/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月7日
**************************************************************************/
#ifndef __DIAM_DICTOBJECT_H_
#define __DIAM_DICTOBJECT_H_

#include <list>
#include <api/diam_define.h>
#include <parser/diam_datatype.h>

class CDictObject {};
/**************************************************************************
类    名: dictAVP
类 功 能: 字典数据对象，对应字典中的avp
时    间: 2012年5月7日
**************************************************************************/
class CDictObjectAvp
{
public:
    CDictObjectAvp(DiamAVPCode code,
                   const char *name,
                   AvpDataType type,
                   DiamVendorId id,
                   DiamAvpFlag flg);

public:
    DiamAVPCode   avpCode;  //avp编码
    std::string   avpName;  //avp名称
    AvpDataType   avpType;  //avp数据类型
    DiamVendorId  vendorId; //avp所属设备商
    DiamAvpFlag   flags;    //avp标志
    DiamUINT16    maxSize;  //UA Utf8String OctetString 最大长度
    DiamUINT16    resize;   //UA OctetString 是否需要由业务指定长度，不等于0为真
};

/**************************************************************************
类    名: dictQualAVP
类 功 能:
时    间: 2012年5月7日
**************************************************************************/
typedef struct
{
    CDictObjectAvp *avp;
    DiamUINT16 min;
    DiamUINT16 max;
} CDictObjectQualAVP;

/**************************************************************************
类    名: DiamQualifiedAvpList
类 功 能: 字典类，对应command码中的avp规则
时    间: 2012年5月7日
**************************************************************************/
class CDiamQualifiedAvpList: public std::list<CDictObjectQualAVP*>
{
public:
    CDiamQualifiedAvpList(DiamAvpParseType pt);

    ~CDiamQualifiedAvpList();

    void add(CDictObjectQualAVP* q_avp);

    unsigned getMinSize(void);

    DiamAvpParseType& getParseType(void);

private:
    DiamAvpParseType parseType;
};

/**************************************************************************
类    名: 命令码标志位
类 功 能:
时    间: 2012年5月7日
**************************************************************************/
struct commandFlags
{
    DiamUINT8 r:1;
    DiamUINT8 p:1;
    DiamUINT8 e:1;
};
/**************************************************************************
类    名: dictCommand
类 功 能: 字典数据对象，对应字典中的命令码
时    间: 2012年5月7日
**************************************************************************/
class CDictObjectCommand : public CDictObject
{
public:
    DiamCommandCode    code;
    std::string            name;
    struct commandFlags    flags;
    DiamAppId  appId;
    DiamVendorId       vendorId;
    CDiamQualifiedAvpList* fixed;
    CDiamQualifiedAvpList* required;
    CDiamQualifiedAvpList* optional;
};

/**************************************************************************
类    名:
类 功 能:
时    间: 2012年5月7日
**************************************************************************/
class CDictObjectGroupAvp : public CDictObjectCommand {};

#endif /*__DIAM_DICTOBJECT_H_*/


