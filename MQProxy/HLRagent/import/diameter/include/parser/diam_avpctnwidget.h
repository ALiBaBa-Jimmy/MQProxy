/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月27日
**************************************************************************/
#ifndef __DIAM_AVPCNT_WIGGET_H__
#define __DIAM_AVPCNT_WIGGET_H__

#include <string.h>
#include <api/diam_message.h>
#include <parser/diam_datatype.h>
#include <parser/diam_dictobject.h>
#include <parser/diam_avpcontainer.h>
#include <parser/diam_ctnmanager.h>
/**************************************************************************
 类    名: CAvpContainerWidget
 类 功 能:
 时    间: 2012年5月10日
 **************************************************************************/
template<class D, AvpDataType T>
class CAvpContainerWidget
{
public:
    CAvpContainerWidget(const char *name)
    {
        CAvpContainerManager cm;
        m_cAvp = cm.acquire(name);
    }

    CAvpContainerWidget(const DiamUINT32 code)
    {
        CAvpContainerManager cm;
        m_cAvp = cm.acquire(code);
    }

    CAvpContainerWidget(const char *name, D &value)
    {
        CAvpContainerManager cm;
        m_cAvp = cm.acquire(name);
        Get() = value;
    }

    CAvpContainerWidget(const DiamUINT32 code, D &value)
    {
        CAvpContainerManager cm;
        m_cAvp = cm.acquire(code);
        Get() = value;
    }

    CAvpContainerWidget(CAvpContainer *avp):m_cAvp(avp)
    {
    }

    ~CAvpContainerWidget() {}

    D &Get()
    {
        CAvpContainerEntryManager em;
        CAvpContainerEntry *e = em.acquire(T);
        m_cAvp->add(e);
        return e->dataRef(Type2Type<D>());
    }

    CAvpContainer *operator()()
    {
        return m_cAvp;
    }

    bool empty()
    {
        return (m_cAvp->size() == 0);
    }

private:
    CAvpContainer *m_cAvp;
};

/**************************************************************************
类    名: CAvpContainerListWidget
类 功 能: 对containerList进行封装,提供向container中添加/删除avp等操作
时    间: 2012年5月10日
**************************************************************************/
template<class D, AvpDataType t>
class CAvpContainerListWidget
{
public:
    CAvpContainerListWidget(DiamBody &lst): m_List(lst) {};

    D *GetAvp(const char *name, unsigned int index=0)
    {
        CAvpContainer* c = m_List.search(name);
        if (c && (index < c->size()))
        {
            CAvpContainerEntry *e = (*c)[index];
            return e->dataPtr(Type2Type<D>());
        }
        return (0);
    }

    D *GetAvp(const DiamUINT32 code, unsigned int index=0)
    {
        CAvpContainer* c = m_List.search(code);
        if (c && (index < c->size()))
        {
            CAvpContainerEntry *e = (*c)[index];
            return e->dataPtr(Type2Type<D>());
        }
        return (0);
    }


    D &AddAvp(const char *name, bool append = false)
    {
        CAvpContainer* c = m_List.search(name);
        if (! c)
        {
            CAvpContainerWidget<D, t> avpWidget(name);
            m_List.add(avpWidget());
            return avpWidget.Get();
        }
        else if ((c->size() == 0) || append)
        {
            CAvpContainerWidget<D, t> avpWidget(c);
            return avpWidget.Get();
        }
        else
        {
            return (*c)[0]->dataRef(Type2Type<D>());
        }
    }

    D &AddAvp(const DiamUINT32 code, bool append = false)
    {
        CAvpContainer* c = m_List.search(code);
        if (! c)
        {
            CAvpContainerWidget<D, t> avpWidget(code);
            m_List.add(avpWidget());
            return avpWidget.Get();
        }
        else if ((c->size() == 0) || append)
        {
            CAvpContainerWidget<D, t> avpWidget(c);
            return avpWidget.Get();
        }
        else
        {
            return (*c)[0]->dataRef(Type2Type<D>());
        }
    }

    void AddAvp(CAvpContainerListWidget<D, t> &avp)
    {
        m_List.add(avp());
    }

    void DelAvp(const char *name)
    {
        std::list<CAvpContainer*>::iterator i;
        for (i=m_List.begin(); i!=m_List.end(); i++)
        {
            CAvpContainer *c = *i;
            if (strcmp(c->getAvpName(), name) == 0)
            {
                m_List.erase(i);
                CAvpContainerManager cm;
                cm.release(c);
                break;
            }
        }
    }

    void DelAvp(DiamUINT32 code)
    {
        std::list<CAvpContainer*>::iterator i;
        for (i=m_List.begin(); i!=m_List.end(); i++)
        {
            CAvpContainer *c = *i;
            if (c->getAvpCode() == code)
            {
                m_List.erase(i);
                CAvpContainerManager cm;
                cm.release(c);
                break;
            }
        }
    }

    unsigned int GetAvpCount(char *name)
    {
        CAvpContainer* c = m_List.search(name);
        return (c) ? c->size() : 0;
    }

    unsigned int GetAvpCount(DiamUINT32 code)
    {
        CAvpContainer* c = m_List.search(code);
        return (c) ? c->size() : 0;
    }

private:
    DiamBody &m_List;
};


//对外提供接口
typedef CAvpContainerWidget<DiamIdentity, DIAM_AVP_TYPE_DIAMIDENTITY>  AvpWidget_Identity;
typedef CAvpContainerWidget<DiamAddress,DIAM_AVP_TYPE_ADDRESS>         AvpWidget_Address;
typedef CAvpContainerWidget<DiamINT16,DIAM_AVP_TYPE_INT32>             AvpWidget_Int16;
typedef CAvpContainerWidget<DiamUINT16,DIAM_AVP_TYPE_UINT32>           AvpWidget_UInt16;
typedef CAvpContainerWidget<DiamINT32,DIAM_AVP_TYPE_INT32>             AvpWidget_Int32;
typedef CAvpContainerWidget<DiamUINT32,DIAM_AVP_TYPE_UINT32>           AvpWidget_UInt32;
typedef CAvpContainerWidget<DiamINT64,DIAM_AVP_TYPE_INT64>             AvpWidget_Int64;
typedef CAvpContainerWidget<DiamUINT64,DIAM_AVP_TYPE_UINT64>           AvpWidget_UInt64;
typedef CAvpContainerWidget<DiamUTF8String,DIAM_AVP_TYPE_UTF8STRING>   AvpWidget_Utf8;
typedef CAvpContainerWidget<DiamGroup,DIAM_AVP_TYPE_GROUPED>           AvpWidget_Grouped;
typedef CAvpContainerWidget<DiamOctetString,DIAM_AVP_TYPE_OCTETSTRING> AvpWidget_String;
typedef CAvpContainerWidget<DiamURI,DIAM_AVP_TYPE_DIAMURI>             AvpWidget_DiamURL;
typedef CAvpContainerWidget<DiamEnum,DIAM_AVP_TYPE_ENUM>               AvpWidget_Enum;
typedef CAvpContainerWidget<DiamTime,DIAM_AVP_TYPE_TIME>               AvpWidget_Time;

//对外提供接口
typedef CAvpContainerListWidget<DiamIdentity, DIAM_AVP_TYPE_DIAMIDENTITY>   AvpListWidget_Identity;
typedef CAvpContainerListWidget<DiamAddress, DIAM_AVP_TYPE_ADDRESS>         AvpListWidget_Address;
typedef CAvpContainerListWidget<DiamINT16, DIAM_AVP_TYPE_INT32>             AvpListWidget_Int16;
typedef CAvpContainerListWidget<DiamUINT16, DIAM_AVP_TYPE_UINT32>           AvpListWidget_UInt16;
typedef CAvpContainerListWidget<DiamINT32, DIAM_AVP_TYPE_INT32>             AvpListWidget_Int32;
typedef CAvpContainerListWidget<DiamUINT32, DIAM_AVP_TYPE_UINT32>           AvpListWidget_UInt32;
typedef CAvpContainerListWidget<DiamINT64, DIAM_AVP_TYPE_INT64>             AvpListWidget_Int64;
typedef CAvpContainerListWidget<DiamUINT64, DIAM_AVP_TYPE_UINT64>           AvpListWidget_UInt64;
typedef CAvpContainerListWidget<DiamUTF8String, DIAM_AVP_TYPE_UTF8STRING>   AvpListWidget_Utf8;
typedef CAvpContainerListWidget<DiamGroup, DIAM_AVP_TYPE_GROUPED>           AvpListWidget_Grouped;
typedef CAvpContainerListWidget<DiamOctetString, DIAM_AVP_TYPE_OCTETSTRING> AvpListWidget_OctetString;
//typedef CAvpContainerListWidget<DiamOctetString, DIAM_AVP_TYPE_COMMONSTRING> AvpListWidget_CommString;
typedef CAvpContainerListWidget<DiamURI, DIAM_AVP_TYPE_DIAMURI>             AvpListWidget_DiamURI;
typedef CAvpContainerListWidget<DiamEnum, DIAM_AVP_TYPE_ENUM>               AvpListWidget_Enum;
typedef CAvpContainerListWidget<DiamTime, DIAM_AVP_TYPE_TIME>               AvpListWidget_Time;

#endif /* __DIAM_AVPCONTAINERWIGGET_H__ */

