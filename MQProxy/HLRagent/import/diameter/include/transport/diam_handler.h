/**************************************************************************
��    ��: CDiamACETransport
�� �� ��: socket������,������ӵĽ����������շ�
ʱ    ��: 2012��8��7��
**************************************************************************/
#ifndef __DIAM_HANDLER_H__
#define __DIAM_HANDLER_H__

#include <api/diam_datatype.h>
#include <api/diam_message.h>

#define  RX_BUFFER_SIZE                 2048
#define  MSG_COLLECTOR_MAX_MSG_LENGTH   2048
#define  MSG_COLLECTOR_MAX_MSG_BLOCK    10

class DiamRangedValue
{
public:
    enum
    {
        DEFAULT_LOW  = 0,
        DEFAULT_HIGH = 3,
    };

public:
    DiamRangedValue(int level = DEFAULT_LOW,
                    int low = DEFAULT_LOW,
                    int high = DEFAULT_HIGH)
    {
        Reset(level, low, high);
    }
    virtual ~DiamRangedValue()
    {
    }
    virtual int operator++()
    {
        m_CurrentLevel += 1;
        return (m_CurrentLevel > m_HighThreshold) ? true : false;
    }
    virtual int operator--()
    {
        m_CurrentLevel -= 1;
        return (m_CurrentLevel < m_LowThreshold) ? true : false;
    }
    virtual int operator()()
    {
        return m_CurrentLevel;
    }
    virtual bool InRange()
    {
        return ((m_CurrentLevel > m_LowThreshold) &&
                (m_CurrentLevel < m_HighThreshold));
    }
    void Reset(int level = DEFAULT_LOW,
               int low = DEFAULT_LOW,
               int high = DEFAULT_HIGH)
    {
        m_CurrentLevel = level;
        m_LowThreshold = low;
        m_HighThreshold = high;
    }

private:
    int m_CurrentLevel;
    int m_LowThreshold;
    int m_HighThreshold;
};


class DiamPeer;
class DiamHandler
{
public:
    DiamHandler();
    ~DiamHandler();

public:
    int ProcDiamMsg(void *data, int length, DiamPeer* peer);

private:
    void ProcDiamHeader();
    void ProcDiamBody();

    void SendErrorMsg(DiamMsg* diammsg);

    DiamUINT32 SimpleMemShow(const char *title, 
        const void *mem, DiamUINT32 len, 
        DiamChar*  outPutBuf, DiamUINT32 outPutBufLen);

    void MemShow(const char *title, const void *mem, DiamUINT32 len);

private:
    DiamChar*  buffer_;     // buffe`r of unprocessed received data //��û�д���Ľ��յ�������
    DiamINT32  offset_;     // current read offset from buffer      //��ǰ��buffer�ж�ȡ���ĳ���
    DiamINT32  bufsize_;    // allocated size of rdBuffer           //��ǰbuffer�ܳ���
    DiamINT32  msglen_;     // current message length

    DiamRangedValue m_PersistentError;
};

#endif //
