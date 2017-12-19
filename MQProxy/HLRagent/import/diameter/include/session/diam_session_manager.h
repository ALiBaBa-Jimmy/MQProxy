/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: diam_Session_manager.h
��    ��: ����Ự,Ŀǰ�Ự�Թ�����ķ�ʽ����,�������κλỰ״̬
          �����Ҫ�����Ự״̬,�������չ�Ự���ԡ�
ʱ    ��: 2012��8��31��
**************************************************************************/
#ifndef __DIAM_SESSION_MANAGE_H_
#define __DIAM_SESSION_MANAGE_H_

#include <map>
#include <session/diam_session.h>
#include <api/diam_interface.h>
#include <util/diam_ace.h>

class CDiamSessionManager
{
public:
    CDiamSessionManager();
    virtual ~CDiamSessionManager();

public:
    /**************************************************************************
    �� �� ��: saveSessionId
    ��������: ����ҵ��ID��ȡ��һ��appid
    ��    ��: DiameterUINT32 hh ҵ��ID
    �� �� ֵ: sessionId     �ỰId
    **************************************************************************/
    DiamBool saveSessionId(DiamUINT32 hh, std::string& sessionId);
    /**************************************************************************
    �� �� ��: getSessionIdByHH,get��ɾ��
    ��������: ����ҵ��ID��ȡ��һ��appid
    ��    ��: DiameterUINT32 hh ҵ��ID
    �� �� ֵ: sessionId     �ỰId
    **************************************************************************/
    DiamBool getSessionIdByHH(DiamUINT32 hh, std::string& sessionId);
    /**************************************************************************
    �� �� ��: GetSession
    ��������: ����ҵ��ID��ȡ��һ��appid
    ��    ��: DiameterUINT32 appid ҵ��ID
    �� �� ֵ: CDiamSession* �Ự�������
    **************************************************************************/
    DiamSession* getSession(DiamUINT32 appid);
    /**************************************************************************
    �� �� ��: RegisterMsgFunction
    ��������: ��Ự��ע����Ϣ����ص�����
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    DiamRetCode registerMsgFunction(DiamUINT32 appid, diamMsgFunc func);
private:
    //
    std::map<DiamUINT32, DiamSession*> app_session_;
    std::map<DiamUINT32, std::string>  hh_session_;
    ACE_RW_Mutex                       mutex_;
};

typedef ACE_Singleton<CDiamSessionManager, ACE_Recursive_Thread_Mutex> CDiamSessionManager_S;
#define DiamSessionManager() (CDiamSessionManager_S::instance())

#endif //__DIAM_SESSION_MANAGE_H_
