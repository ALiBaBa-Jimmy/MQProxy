/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月8日
**************************************************************************/
#ifndef __DIAM_AVPCONTAINER_H__
#define __DIAM_AVPCONTAINER_H__

#include <vector>
#include <list>
#include <string>
#include <api/diam_datatype.h>
#include <util/diam_config.h>
#include <parser/diam_dictobject.h>
#include <parser/diam_avpallocator.h>

template <class T>
struct Type2Type
{
    typedef T OriginalType;
};

/**************************************************************************
类    名:
类 功 能:
时    间: 2012年5月8日
**************************************************************************/
class DIAMETER_EXPORT CAvpContainerEntry
{
    friend class CAvpContainerEntryManager;
    friend class CAvpContainer;

public:
    AvpDataType& dataType()
    {
        return this->type;
    }

    template <class T> inline T& dataRef(Type2Type<T>)
    {
        return *((T*)data);
    }

    template <class T> inline T* dataPtr(Type2Type<T>)
    {
        return (T*)data;
    }

protected:
    CAvpContainerEntry(int type) : type((AvpDataType)type) {}

    virtual ~CAvpContainerEntry() {}

    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }

    void operator delete(void *p)
    {
        containerFree(p);
    }
    AvpDataType type;

    void*       data;
};


template <class T>
class DIAMETER_EXPORT CSpecificAvpContainerEntry : public CAvpContainerEntry
{
public:
    CSpecificAvpContainerEntry(int type) : CAvpContainerEntry(type)
    {
        //data = (new(MemoryManagerInstance::instance()->malloc(sizeof(T))) T);
        data = (new (containerMalloc(sizeof(T))) T);
    }

    ~CSpecificAvpContainerEntry()
    {
        ((T*)data)->T::~T();
        //MemoryManagerInstance::instance()->free(data);
        containerFree(data);
    }

    inline T* dataPtr() const
    {
        return (T*)data;
    }

    inline T& dataRef() const
    {
        return *((T*)data);
    }

    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }

    void operator delete(void *p)
    {
        containerFree(p);
    }

};

/**************************************************************************
类    名: CAvpContainer
类 功 能:
时    间: 2012年12月13日
**************************************************************************/
class DIAMETER_EXPORT CAvpContainer : public std::vector<CAvpContainerEntry*>
{
    friend class DiamBody;

public:

    CAvpContainer() : avpCode(0), avpName(""), flag(false), parseType(PARSE_TYPE_OPTIONAL)
    {}

    ~CAvpContainer() {}

    void releaseEntries();

    inline void add(CAvpContainerEntry* e)
    {
        resize(size()+1, e);
    }

    void remove(CAvpContainerEntry* e);

    inline const char* getAvpName() const
    {
        return avpName.c_str();
    }

    inline void setAvpName(const char* name)
    {
        avpName = std::string(name);
    }

    inline const DiamUINT32 getAvpCode() const
    {
        return avpCode;
    }

    inline void setAvpCode(DiamUINT32 code)
    {
        avpCode = code;
    }

    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }

    void operator delete(void *p)
    {
        containerFree(p);
    }

    inline DiamAvpParseType& ParseType()
    {
        return parseType;
    }

protected:
    std::string avpName;

    DiamUINT32 avpCode;

    DiamBool flag;

    DiamAvpParseType parseType;
};

class DIAMETER_EXPORT DiamBody : public std::list<CAvpContainer*>
{
public:

    DiamBody() {}
    ~DiamBody();

public:
    inline void add(CAvpContainer* c)
    {
        push_back(c);
    }

    inline void prepend(CAvpContainer* c)
    {
        push_front(c);
    }

    void releaseContainers();

    void* operator new(size_t s)
    {
        return containerMalloc(s);
    }

    void* operator new(size_t s, void*p)
    {
        return p;
    }

    void operator delete(void *p)
    {
        containerFree(p);
    }

    void operator delete(void *p, void*q)
    {
        containerFree(p);
    }

    CAvpContainer* search(const char* name);

    CAvpContainer* search(DiamUINT32 code);

    CAvpContainer* search(CDictObjectAvp* avp);

    void reset();

private:
    CAvpContainer* search(const char*, bool);
    CAvpContainer* search(DiamUINT32, bool);
    //根据avpcode和avpname查找
    CAvpContainer* search(DiamUINT32 code, const char* name, bool b);
    CAvpContainer* search(bool);
};

//
typedef CSpecificAvpContainerEntry<DiamINT32>         AvpContainerEntry_Integer32;
typedef CSpecificAvpContainerEntry<DiamUINT32>        AvpContainerEntry_Unsigned32;
typedef CSpecificAvpContainerEntry<DiamINT64>         AvpContainerEntry_Integer64;
typedef CSpecificAvpContainerEntry<DiamUINT64>        AvpContainerEntry_Unsigned64;
typedef CSpecificAvpContainerEntry<DiamTime>          AvpContainerEntry_Time;
typedef CSpecificAvpContainerEntry<DiamBody>          AvpContainerEntry_Grouped;
typedef CSpecificAvpContainerEntry<std::string>       AvpContainerEntry_String;
typedef CSpecificAvpContainerEntry<DiamOctetString>   AvpContainerEntry_OctetString;
typedef CSpecificAvpContainerEntry<DiamURI>           AvpContainerEntry_DiamURI;
typedef CSpecificAvpContainerEntry<DiamIPFilterRule>  AvpContainerEntry_IPFilterRule;
typedef CSpecificAvpContainerEntry<DiamAddress>       AvpContainerEntry_Address;


#endif /*__DIAM_AVPCONTAINER_H__*/
