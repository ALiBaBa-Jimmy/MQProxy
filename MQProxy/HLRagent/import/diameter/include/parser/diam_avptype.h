/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��8��
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
��    ��: AvpContainerEntryCreator
�� �� ��: ����������ģ����
ʱ    ��: 2012��5��8��
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
��    ��: AvpValueParserCreator
�� �� ��: ������������ģ����
ʱ    ��: 2012��5��8��
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
��    ��: AvpType
�� �� ��: ���󴴽���,�����Ͱ󶨶�Ӧ�Ľ�����������������������
ʱ    ��: 2012��5��8��
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
    //������
    const char *name;
    //����ֵ
    AvpDataType type;
    //���ʹ�С
    DiamUINT32 size;
    //������������
    AvpValueParserFunctor parserCreator;
    //����������
    AvpContainerEntryFunctor containerEntryCreator;
};


#endif /**/

