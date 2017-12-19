#ifndef _XOSHA_H_
#define _XOSHA_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/

#include "xostype.h"
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define MAX_HA_CALLBACK 32

typedef int ( * HA_CALLBACK_FUNC )(int,void *,void *);
typedef struct ha_register_para_t {
    int module_id;
    HA_CALLBACK_FUNC  p_notify_cbf;
    void              *app_arg;
}t_ha_register_para;

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/
typedef enum
{
  eHaBoardGood =0,
  eHaBoardBad
}e_xosha_bdstatus;

typedef enum
{    
    eHaCmdUser = 0,
    eHaCmdDriver
}e_xosha_cmd;

typedef enum
{    
    eHaStateNull = 0,
    eHaStateActive,
    eHaStateStandby,
    
    eHaStateSingle = 0x80 /*上层应用事件*/
} e_xosha_state;

typedef enum
{    
    eHaErrNo=0,
    eHaErrSuspend,
    eHaErrUser,
    eHaErr3
}e_xosha_errcode;

/*
HA事件描述
NULL=未知或初始化状态
START_SERVICE=Master Board
STOP_SERVICE=Slave Board
START_BACKUP=开始主备数据同步
PEER_CONN_FAIL=主备通讯Fail
PEER_CONN_OK=主备通讯OK
STANDBY_UP=备板启动成功
STANDBY_LOST=备板丢失
*/
typedef enum
{
    HA_NOTIFY_EVENT_NULL=0,
    HA_NOTIFY_EVENT_START_SERVICE,
    HA_NOTIFY_EVENT_STOP_SERVICE,
    HA_NOTIFY_EVENT_START_BACKUP,
    HA_NOTIFY_EVENT_PEER_CONN_FAIL,
    HA_NOTIFY_EVENT_PEER_CONN_OK,
    HA_NOTIFY_EVENT_STANDBY_UP,
    HA_NOTIFY_EVENT_STANDBY_LOST
}e_ha_event;

typedef struct ha_info_tag
{
  e_xosha_state hastate;
  int  tskerrno;
  int  tskid;
  char tskname[32];
  char ocurtime[20];
  char desc[32];
}t_xosha_info;

typedef enum
{
  XRUN_LED =1,
  XALARM_LED,
  XEPI_LED,
  XOCT_LED  
}e_ha_ledtype;
/*机框管理*/
/* board type */
#define CHASSSID_ID_DEFAULT  0x01
#define BOARD_VTA_MAXNUM     11
#define BOARD_TYPE_CSP  0x00
#define BOARD_TYPE_TSP  0x01
#define BOARD_TYPE_OCI  0x02
#define BOARD_TYPE_EPI  0x03
#define BOARD_TYPE_NSP  0x04
#define BOARD_TYPE_OCT  0x05
#define BOARD_TYPE_AMP600  0x06
#define BOARD_TYPE_MAX  0X0a

/*
VTA：Versatile Telecom Architecture  通用电信平台
TSP：TDM Service Process board  TDM业务处理板
CSP：Control and Switch Process board  控制交换处理板
NSP2000：Network Service Process board 2000  网络业务处理板2000
EPI：E1 Pool Interface board   E1接口板
OCI：OC3 Interface board       OC3接口板
DBP：Data BackPlane board      数据背板
GPP：General Processor Process board  通用处理器处理板
AMP600：Audiocodes Media Process board 600  奥科媒体处理板600
*/
typedef struct chasisinfo_tag
{
    int ChassisId;
    int SlotNo;
    int boardType;/*csp(0);tsp(1);oci(2);epi(3);nsp(4);oct(5);amp600(6)*/
    int SubId;
    int HAMode; /*oneAndOne(1);nAndM(2);onlyone(3) */
    int HAState;/*Initial(0);Master(1);Slave(2);Reserved(3)*/
    int RunState;/*good board(0);board bad(1)*/
    XU32 PhyIp;
    XU32 RoutingGateway;
    XU32 PhyNetmask;
    XU32 LogicIp;
    XU32 LogicIPmask;
    char Version[256];
    char Desc[64];
    char strtimevalue[32];
    XS32 snmpRowStatus;
    XS32 registerState;
}t_chassisinfo;
/*机框管理*/

int ha_app_register(t_ha_register_para *t_ha_para);
extern t_xosha_info g_sysha_context;

#ifdef VXWORKS 
  int ha_call_callbacks(int cmd, void * hainfo_arg);
  extern int xos_hainit();
  extern XS32 ha_setsystime(XCHAR* strTimeIn);
  extern XCHAR* ha_getsystime();
  extern int ha_oam_settime(int parain);
  extern XCHAR* ha_oam_gettime();
  extern XS32 ha_ledrun(XS32 LedType,XBOOL bActive);
  extern XS32 ha_checkBoardMain(void);
  extern XS32 getBoardIp( XU32 *ip,XU32 *mask,XU32 *gateway);

#endif
/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/
extern XU32 ha_getboardslot();
extern int  chassis_board_register(t_chassisinfo board_info);
extern int  chassis_board_unregister(t_chassisinfo board_info);
extern int  chassis_board_check(t_chassisinfo board_info);
extern t_chassisinfo g_chassis_board[BOARD_TYPE_MAX];

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif 

