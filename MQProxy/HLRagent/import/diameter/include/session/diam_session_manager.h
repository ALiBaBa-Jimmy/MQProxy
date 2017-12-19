/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: diam_Session_manager.h
功    能: 管理会话,目前会话以功能类的方式进行,不包含任何会话状态
          如果需要包括会话状态,则可以扩展会话属性。
时    间: 2012年8月31日
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
    函 数 名: saveSessionId
    函数功能: 根据业务ID获取到一个appid
    参    数: DiameterUINT32 hh 业务ID
    返 回 值: sessionId     会话Id
    **************************************************************************/
    DiamBool saveSessionId(DiamUINT32 hh, std::string& sessionId);
    /**************************************************************************
    函 数 名: getSessionIdByHH,get后将删除
    函数功能: 根据业务ID获取到一个appid
    参    数: DiameterUINT32 hh 业务ID
    返 回 值: sessionId     会话Id
    **************************************************************************/
    DiamBool getSessionIdByHH(DiamUINT32 hh, std::string& sessionId);
    /**************************************************************************
    函 数 名: GetSession
    函数功能: 根据业务ID获取到一个appid
    参    数: DiameterUINT32 appid 业务ID
    返 回 值: CDiamSession* 会话处理对象
    **************************************************************************/
    DiamSession* getSession(DiamUINT32 appid);
    /**************************************************************************
    函 数 名: RegisterMsgFunction
    函数功能: 向会话中注册消息处理回调函数
    参    数:
    返 回 值:
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
