#ifndef DIAMETER_UA_BUILD_HEAD_FILE_H_
#define DIAMETER_UA_BUILD_HEAD_FILE_H_
#include <iostream>
#include <list>
#include <map>
#include <ua/diameter_ua_common.h>

namespace diameter_ua{
class CAbstractType //用于头文件生成
{
    typedef ::std::map<AvpDataType, CAbstractType* > MapType;
    typedef MapType::iterator MapTypeIter;
public:
    CAbstractType(DiameterQualAVP *qualifiable_avp);
    virtual ~CAbstractType();

    DUA_CONST DUA_INT8* GetTypeName(){return type_name_.c_str();}
    AvpDataType type(){return type_;}
    virtual DUA_CONST DUA_INT8 *GetAvpName() = 0;
private:
    AvpDataType type_;
    ::std::string type_name_;
    DiameterQualAVP *qualifiable_avp_;
    DUA_STATIC MapType* g_types_;
};
class CTypeGroup : public CAbstractType
{
public:
    explicit CTypeGroup(DiameterQualAVP *qualifiable_avp);
    explicit CTypeGroup(DiameterGroupAVP *group_avp);
    ~CTypeGroup();
    DUA_INT32 is_command(){return is_command_;}
private:
    DiameterGroupAVP *group_avp_;
    DUA_INT32 is_command_;
};

}
#endif