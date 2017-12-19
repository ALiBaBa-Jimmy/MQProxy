/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月8日
**************************************************************************/
#ifndef __DIAM_AVPTYPE_H__
#define __DIAM_AVPTYPE_H__

#include <boost/function/function0.hpp>
#include <boost/function/function1.hpp>
#include <parser/diam_datatype.h>
#include <parser/diam_avpcontainer.h>
#include <parser/diam_parser.h>

typedef boost::function0<DiamAvpValueParser*> AvpValueParserFunctor;
typedef boost::function1<CAvpContainerEntry*, AvpDataType> AvpContainerEntryFunctor;

/**************************************************************************
类    名: AvpContainerEntryCreator
类 功 能: 容器创建器模板类
时    间: 2012年5月8日
**************************************************************************/
template <class T>
class CAvpContainerEntryCreator
{
public:
    CAvpContainerEntry* operator()(int type) {
        return new T(type);
    }
};

/**************************************************************************
类    名: AvpValueParserCreator
类 功 能: 解析器创建器模板类
时    间: 2012年5月8日
**************************************************************************/
template <class T>
class CAvpValueParserCreator
{
public:
    DiamAvpValueParser* operator()() {
        return new T();
    }
};

/**************************************************************************
类    名: AvpType
类 功 能: 对象创建器,该类型绑定对应的解析器创建器和容器创建器
时    间: 2012年5月8日
**************************************************************************/
class CAvpType
{
public:
    CAvpType(const char* name,
             AvpDataType type,
             DiamUINT32 size,
             AvpValueParserFunctor parserCreator,
             AvpContainerEntryFunctor containerEntryCreator) :
        name(name),
        type(type),
        size(size),
        parserCreator(parserCreator),
        containerEntryCreator(containerEntryCreator)
    {}
    const char* getName(void)
    {
        return name;
    };

    AvpDataType getType(void)
    {
        return type;
    };

    DiamUINT32 getMinSize(void)
    {
        return size;
    };

    DiamAvpValueParser* createParser()
    {
        return parserCreator();
    }

    CAvpContainerEntry* createContainerEntry(AvpDataType type)
    {
        return containerEntryCreator(type);
    }

private:
    //类型名
    const char *name;
    //类型值
    AvpDataType type;
    //类型大小
    DiamUINT32 size;
    //解析器创建器
    AvpValueParserFunctor parserCreator;
    //容器创建器
    AvpContainerEntryFunctor containerEntryCreator;
};


#endif /**/

