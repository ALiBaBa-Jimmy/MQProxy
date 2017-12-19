/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename:       diameter_ua_binary_method.h
**  description:    diameter 基础类型与c结构体转换
**  author:         luozhongjie
**  data:           2014.10.21
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   luozhongjie     2014.10.21        create
**************************************************************/

#ifndef DIAMETER_UA_BINARY_METHOD_H_
#define DIAMETER_UA_BINARY_METHOD_H_
#include <ua/diameter_ua_common.h>
#include <api/diam_message.h>
#include <string>
#include <map>
#include <list>
#include <api/diam_messageoper.h>

namespace diameter_ua
{
typedef DiameterUaUndefinedIE UndefinedIE;
typedef DiameterUaBitmap Bitmap;
typedef DiameterUaListSize ListSize;

typedef DUA_UINT32 DiameterBaseType; // typedef AvpDataType DiameterBaseType;
typedef DiamBody DiameterBody;
typedef DUA_CONST CDictObjectAvp DiameterAVP;
typedef DUA_CONST CDictObjectQualAVP DiameterQualAVP;
typedef DUA_CONST CDictObjectGroupAvp DiameterGroupAVP;
typedef DUA_CONST CDictObjectCommand DiameterCommand;
typedef DUA_UINT32 TranslatedLen;

typedef enum
{
    kUaTypeBegin = 10000,
    kUaTypeResultCode , 

    kUaTypeEnd,
}UaType; // 与 AvpDataType 不能重复
#define BINARY_METHOD_PARA(TYPE) TYPE, #TYPE

DiameterCommand *SearchCommand(DiameterUaMsgInfo *msg_info);
DiameterGroupAVP *SearchGroup(DiameterAVP *avp);
#define DIAMETER_UA_NEW(ptr, obj, ret) {try{ptr = new obj; if(!ptr) return ret;}catch(...){ptr = DUA_NULL;return ret;}}

class CAvpBinaryMethod
{
    typedef ::std::map<DiameterBaseType, CAvpBinaryMethod * > MapAvpMethod;
    typedef MapAvpMethod::iterator MapAvpMethodIter;
public:
    DUA_STATIC CAvpBinaryMethod *GetInstanceByDiameterQualAvp(DiameterQualAVP *qual_avp);
     
    DUA_RETURN Encode(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN Decode(DiameterBody& diameter_body, DUA_UINT8 *binary);

    virtual DUA_VOID Display(DUA_UINT32 deepth);
    
    DUA_UINT32 IsList(){return max_size() > 1 ? 1 : 0;}
    DUA_UINT32 GetBlockLen()
    {
        if(binary_len_ == 0) 
            return 0;
        return IsList() ? (binary_len_ * max_size_ + sizeof(ListSize)) : binary_len_;
    }
    DUA_UINT32 IsGroup()
    {
        return type_==DIAM_AVP_TYPE_GROUPED ? 1 : 0;
    }
    virtual DUA_CONST DUA_INT8 *GetAvpName(); //group需要重新实现
    DiameterBaseType type(){return type_;}
    DUA_UINT32 binary_len(){return binary_len_;}
    DUA_UINT32 max_size(){return max_size_;}
    Bitmap bitmap(){return bitmap_;}
    DUA_UINT32 is_mandatory(){return is_mandatory_;}
    DiameterQualAVP *qualifiable_avp(){return qualifiable_avp_;}
    DUA_VOID set_is_mandatory(DUA_UINT32 is_mandatory){is_mandatory_ = is_mandatory;}
    DUA_VOID set_binary_len(int binary_len){binary_len_ = binary_len;}
    DUA_VOID set_bitmap(Bitmap bitmap){bitmap_ = bitmap;}
    DUA_CONST DUA_INT8* type_name(){ return type_name_.c_str();}
    virtual ~CAvpBinaryMethod(){};
protected:
    CAvpBinaryMethod(DiameterBaseType type_in, DUA_CONST DUA_INT8* type_name_in);
    
private:
    DUA_STATIC DUA_UINT32 Init();
    DUA_STATIC DUA_UINT32 RegistInstance(CAvpBinaryMethod *method);
    virtual CAvpBinaryMethod *NewInstance(DUA_CONST DiameterQualAVP *qual_avp) = 0; 
    virtual DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body ) = 0;
    virtual DUA_RETURN DiameterToBinary(DiameterBody& diameter_body,  DUA_UINT32 index, DUA_UINT8 *binary) = 0;

    DUA_STATIC MapAvpMethod *g_avps_method;
    DiameterBaseType type_;
    DUA_UINT32 binary_len_;
    DUA_UINT32 max_size_;
    Bitmap bitmap_;
    DiameterQualAVP *qualifiable_avp_;
    DUA_UINT32 is_mandatory_; //仅对group有效
    std::string type_name_;
};

class CRecieverGroupIterator;
class CRecieverGroup : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;   
 public  :
    typedef ::std::list<CAvpBinaryMethod *> ListAvpMethod;
    typedef ListAvpMethod::iterator ListAvpMethodIter;

    CRecieverGroup* Init(DiameterQualAVP *qual_avp);
    DUA_UINT32 AppendAvp(CAvpBinaryMethod &avp_method);

    DUA_CONST DUA_INT8 *GetAvpName();
    DUA_VOID Display(DUA_UINT32 deepth);
    DUA_VOID set_group_avp(DiameterGroupAVP *group_avp) {group_avp_ = group_avp;}
    DUA_UINT32 IsMessage(){return group_avp_?1:0;}
    CRecieverGroupIterator GetIterator();
    ~CRecieverGroup();
 private:
    CRecieverGroup();
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);

    ListAvpMethod avps_method_;
    Bitmap current_bitmap_;
    DUA_UINT32 is_message_;
    DiameterGroupAVP *group_avp_;
};
class CRecieverGroupIterator
{
    friend class CRecieverGroup;
public:
    DUA_CONST CAvpBinaryMethod *GetNext();
    ~CRecieverGroupIterator();
private:
    CRecieverGroupIterator(CRecieverGroup::ListAvpMethod& avp_list);
    CRecieverGroup::ListAvpMethod& avp_list_;
    CRecieverGroup::ListAvpMethodIter iter_;
};
class CRecieverUnknown : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod; // 构造私有，只能由父类创建子类
private:
    CRecieverUnknown():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_UNKNOWN))
    {
        set_binary_len(sizeof(DUA_UINT32));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverUtf8String : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverUtf8String():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_UTF8STRING))
    {}
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary,  DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverOctetString : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverOctetString():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_OCTETSTRING))
    {}
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverInt32 : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverInt32():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_INT32))
    {
        set_binary_len(sizeof(DUA_INT32));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
    
};

class CRecieverUint32 : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverUint32():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_UINT32))
    {
        set_binary_len(sizeof(DUA_UINT32));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);

};

class CRecieverInt64 : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverInt64():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_INT64))
    {
        set_binary_len(sizeof(DUA_INT64));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);

};

class CRecieverUint64 : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverUint64():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_UINT64))
    {
        set_binary_len(sizeof(DUA_UINT64));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);

};
class CRecieverEnum : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod; 
private:
    CRecieverEnum():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_ENUM))
    {
        set_binary_len(sizeof(DUA_UINT32));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);

};

class CRecieverIdentity : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverIdentity():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_DIAMIDENTITY))
    {
        set_binary_len(sizeof(DUA_INT8 *));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverTime : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverTime():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_TIME))
    {
        set_binary_len(sizeof(DiamTime));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverAddress : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverAddress():CAvpBinaryMethod(BINARY_METHOD_PARA(DIAM_AVP_TYPE_ADDRESS))
    {
        set_binary_len(sizeof(DuaIp));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
};

class CRecieverUaResultCode : public CAvpBinaryMethod
{
    friend class CAvpBinaryMethod;
private:
    CRecieverUaResultCode():CAvpBinaryMethod(BINARY_METHOD_PARA(kUaTypeResultCode))
    {
        set_binary_len(sizeof(DUA_INT32));
    }
    CAvpBinaryMethod *NewInstance(DiameterQualAVP *qual_avp);
    DUA_RETURN BinaryToDiameter(DUA_UINT8 *binary, DiameterBody& diameter_body);
    DUA_RETURN DiameterToBinary(DiameterBody& diameter_body, DUA_UINT32 index, DUA_UINT8 *binary);
    
};

}
#endif