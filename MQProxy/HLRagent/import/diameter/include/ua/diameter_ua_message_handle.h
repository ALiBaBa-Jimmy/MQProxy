/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename:       diameter_ua_message_handle.h
**  description:    diameter ua 消息管理
**  author:         luozhongjie
**  data:           2014.10.21
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   luozhongjie     2014.10.21        create
**************************************************************/
#ifndef DIAMETER_UA_MESSAGE_HANDLE_H_
#define DIAMETER_UA_MESSAGE_HANDLE_H_
#include <ua/diameter_ua_common.h>
#include <map>
#include <string>
#include <util/diam_config.h>
#include <api/diam_messageoper.h>

namespace diameter_ua
{
class DiameterGroupAvpInfo;
class CRecieverGroup;
class CAppContext;
class CMsgHandle;
typedef void (*DecodeHook)(DiamMsg& diameter_msg, CMsgHandle &msg_handle);
class DIAMETER_EXPORT CMsgHandle
{
 typedef ::std::map< ::std::string,  DiameterGroupAvpInfo > MapCommandAvpInfo;
 typedef MapCommandAvpInfo::iterator MapCommandAvpInfoIter;
 friend class CAppContext;
 public:
    DUA_STATIC DUA_RETURN Init(DiameterUaMsgInfo *msg_info, DUA_UINT32 msg_size, DecodeHook func);
    DUA_STATIC CMsgHandle *GetMsgHandle(DUA_UINT32 app_id, DUA_UINT32 msg_id);

    DUA_RETURN DecodeMsg( DiamMsg &diameter_msg, DUA_UINT8 *msg_buff, DUA_UINT32 msg_buff_len);
    DUA_RETURN EncodeMsg(DUA_UINT8 *msg_buff, DUA_UINT32 msg_buff_len, DiamMsg &diameter_msg );
    DUA_UINT32 GetMsgSize(){return msg_info_.size;}
    DUA_UINT32 GetFid(){return msg_info_.peer_fid;}
    DUA_UINT32 GetMsgId(){return msg_info_.msg_id;}
    DUA_UINT32 GetAppId(){return msg_info_.app_id;}
    DUA_VOID Display();

 private:
    CMsgHandle(DiameterUaMsgInfo& msg_info, CAppContext *app_context);
    ~CMsgHandle();
    DUA_STATIC CMsgHandle *GetMsgHandleByMsg(DiamMsg &diameter_msg);
    DUA_VOID InitMsgAvp();
    DUA_UINT32 InitMapAvps(MapCommandAvpInfo& map_avps);
    CRecieverGroup *group_;
    DiameterUaMsgInfo msg_info_;
    CAppContext *app_context_;
};

class DiameterGroupAvpInfo;
class CUaAvp
{
    typedef std::map<std::string, DiameterGroupAvpInfo *> MapUaAvp;
    typedef MapUaAvp::iterator MapUaAvpIter;
public:
    // type定义见 UaType， AvpDataType
    DUA_STATIC void RegistAvp(DUA_CONST DUA_INT8* name, DUA_UINT32 type);
protected:
private:
    friend class CMsgHandle;
    DUA_STATIC void Init();
    DUA_STATIC DiameterGroupAvpInfo* Find(DUA_CONST DUA_INT8* name);
    DUA_STATIC MapUaAvp* g_ua_avps_;
};

}
#endif
