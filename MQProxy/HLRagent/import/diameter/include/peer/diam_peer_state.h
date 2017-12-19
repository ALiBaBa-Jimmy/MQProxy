/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_peerdata.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_PEER_STATE_H__
#define __DIAM_PEER_STATE_H__

#include <api/diam_message.h>
#include <util/diam_sharedptr.h>
#include <peer/diam_peer_define.h>

typedef int Diam_State;

class DiamPeer;
class DiamPeerState
{
public:
    DiamPeerState();
    virtual ~DiamPeerState();

    void Message(SharedPtr<DiamMsg>& msg, DiamPeer* peer);
    void Event(DiamUINT32 id, EDiamPeerEvent ev);
    bool IsOpen();

    Diam_State getState() const
    {
        return state_;
    }
    void StatusNofify(DiamUINT32 linkid, DiamLinkEvent ev);

protected:
    Diam_State           state_;
    diamLinkEventFunc    event_notify_;
    ACE_RW_Mutex         rw_mutex_;
};

#endif //__DIAM_PEER_STATE_H__

