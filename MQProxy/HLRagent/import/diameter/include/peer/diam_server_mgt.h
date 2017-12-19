/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  filename:       diam_peermgt.h
**  description:
**  author:         panjian
**  data:           2014.07.16
**
***************************************************************
**                          history
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_SERVER_MGT_H__
#define __DIAM_SERVER_MGT_H__

#include <util/diam_ace.h>
#include <api/diam_define.h>
#include <peer/diam_peer.h>
#include <peer/diam_server.h>
#include <map>

/*******************************************************************************************
类功能：Diameter 服务端管理类
/*******************************************************************************************/
class DiamServerMgt
{
public:
    DiamServerMgt();
    ~DiamServerMgt();

public:
    DiamRetCode AddServer(DiamUINT32 serverId, const char* master_ip, const char* slave_ip, DiamUINT32 port, PROTOCOL_TYPE type);
    DiamRetCode ModServer(DiamUINT32 serverId, const char* master_ip, const char* slave_ip, DiamUINT32 port, PROTOCOL_TYPE type);
    DiamRetCode DelServer(DiamUINT32 serverId);
    DiamServer* QryServerByServerId(DiamUINT32 serverId);

    DiamRetCode Start(DiamUINT32 serverId);
    DiamRetCode Stop(DiamUINT32 serverId);
    DiamRetCode ReStart(DiamUINT32 serverId);
private:
    std::map<DiamUINT32, DiamServer*> m_ServerMap; //<serverId, DiamServer*>
};

typedef ACE_Singleton<DiamServerMgt, ACE_Recursive_Thread_Mutex> DIAM_SERVER_S;
#define DIAM_SERVER_MGT DIAM_SERVER_S::instance()


#endif //__DIAM_SERVER_MGT_H__

