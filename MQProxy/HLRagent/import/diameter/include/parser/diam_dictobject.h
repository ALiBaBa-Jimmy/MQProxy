/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��7��
**************************************************************************/
#ifndef __DIAM_DICTOBJECT_H_
#define __DIAM_DICTOBJECT_H_

#include <list>
#include <api/diam_define.h>
#include <parser/diam_datatype.h>

class CDictObject {};
/**************************************************************************
��    ��: dictAVP
�� �� ��: �ֵ����ݶ��󣬶�Ӧ�ֵ��е�avp
ʱ    ��: 2012��5��7��
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
    DiamAVPCode   avpCode;  //avp����
    std::string   avpName;  //avp����
    AvpDataType   avpType;  //avp��������
    DiamVendorId  vendorId; //avp�����豸��
    DiamAvpFlag   flags;    //avp��־
    DiamUINT16    maxSize;  //UA Utf8String OctetString ��󳤶�
    DiamUINT16    resize;   //UA OctetString �Ƿ���Ҫ��ҵ��ָ�����ȣ�������0Ϊ��
};

/**************************************************************************
��    ��: dictQualAVP
�� �� ��:
ʱ    ��: 2012��5��7��
**************************************************************************/
typedef struct
{
    CDictObjectAvp *avp;
    DiamUINT16 min;
    DiamUINT16 max;
} CDictObjectQualAVP;

/**************************************************************************
��    ��: DiamQualifiedAvpList
�� �� ��: �ֵ��࣬��Ӧcommand���е�avp����
ʱ    ��: 2012��5��7��
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
��    ��: �������־λ
�� �� ��:
ʱ    ��: 2012��5��7��
**************************************************************************/
struct commandFlags
{
    DiamUINT8 r:1;
    DiamUINT8 p:1;
    DiamUINT8 e:1;
};
/**************************************************************************
��    ��: dictCommand
�� �� ��: �ֵ����ݶ��󣬶�Ӧ�ֵ��е�������
ʱ    ��: 2012��5��7��
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
��    ��:
�� �� ��:
ʱ    ��: 2012��5��7��
**************************************************************************/
class CDictObjectGroupAvp : public CDictObjectCommand {};

#endif /*__DIAM_DICTOBJECT_H_*/


