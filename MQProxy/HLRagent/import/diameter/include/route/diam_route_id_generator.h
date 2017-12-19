#ifndef __ROUTE_ID_GENERATOR_H__
#define __ROUTE_ID_GENERATOR_H__

#include <time.h>
#include <stdlib.h>
#include <api/diam_datatype.h>
#include <util/diam_ace.h>

class CDiamIdGenerator
{
public:
    virtual DiamUINT32 Get() = 0;

protected:
    CDiamIdGenerator() : m_Id(0) { }
    virtual ~CDiamIdGenerator() { }
    DiamUINT32 m_Id;
};

class CDiam_HopByHopGenerator : public CDiamIdGenerator
{
public:
    DiamUINT32 Get()
    {
        if (m_Id == 0)
        {
#ifndef WIN32
            struct timeval tv;
            gettimeofday(&tv, 0);
            srand((unsigned int)(tv.tv_sec + tv.tv_usec));
#else
            srand((unsigned int)time(0));
#endif
            m_Id = rand();
        }
        return (++m_Id);
    }
};

class DIAM_EndToEndGenerator : public CDiamIdGenerator
{
public:
    DiamUINT32 Get()
    {
        if (m_Id == 0)
        {
#ifndef WIN32
            struct timeval tv;
            gettimeofday(&tv, 0);
            srand((unsigned int)(tv.tv_sec + tv.tv_usec));
            m_Id = (unsigned int)(tv.tv_sec) << 20; // set high 12 bit
#else
            srand((unsigned int)time(0));
            m_Id = (unsigned int)(rand()) << 20; // set high 12 bit
#endif
        }
        m_Id &= 0xFFF00000; // clear lower 20 bits
        m_Id |= rand() & 0x000FFFFF; // set low 20 bits
        return (m_Id);
    }
};

typedef ACE_Singleton<CDiam_HopByHopGenerator, ACE_Recursive_Thread_Mutex>
DIAM_HopByHopGenerator_S;
#define DIAM_HOPBYHOP_GEN() DIAM_HopByHopGenerator_S::instance()

typedef ACE_Singleton<DIAM_EndToEndGenerator, ACE_Recursive_Thread_Mutex>
DIAM_EndToEndGenerator_S;
#define DIAM_ENDTOEND_GEN() DIAM_EndToEndGenerator_S::instance()

#endif


