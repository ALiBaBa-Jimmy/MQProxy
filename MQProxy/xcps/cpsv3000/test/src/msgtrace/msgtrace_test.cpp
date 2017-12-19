#include <string>
#include <map>
#include <vector>
#include <signal.h>

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"

#include "../fid_def.h"


#ifdef __cplusplus
extern "C" {
#endif

    XS32 MsgTraceTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 MsgTraceInit( XVOID*, XVOID*);
    XS8 MsgTraceClose(XVOID *Para1, XVOID *Para2);
    XS8 MsgProc(XVOID *Para1, XVOID *Para2);
    XS8 MsgOut(t_BACKPARA* para);

#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherMsgTraceFid ={
    { "FID_MSGTRACE_TEST",  NULL, FID_MSGTRACE_TEST,},
    { MsgTraceInit, NULL, MsgTraceClose,},
    { MsgProc, MsgOut}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 MsgTraceTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherMsgTraceFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_MSGTRACE_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_MSGTRACE_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    return XSUCC;
}

XS8 MsgTraceInit( XVOID*, XVOID*)
{
    MMInfo("MsgTraceInit begin");

    MMInfo("MsgTraceInit end");
    return XSUCC;
}

XS8 MsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;

    if(pMsg) {
        if(pMsg->length > 0 && pMsg->message) {
            XOS_Trace(MD(FID_MSGTRACE_TEST, PL_INFO), "RECV TRACE MSG, len=%d, msg:%s", pMsg->length, pMsg->message);
        } else {
            XOS_Trace(MD(FID_MSGTRACE_TEST, PL_INFO), "RECV TRACE MSG, len=%d", pMsg->length);
        }
    }
    return 0;
}

XS8 MsgOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_MSGTRACE_TEST, PL_INFO), "MsgOut -> %d", para->para1);

    return 0;
}

XS8 MsgTraceClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_MSGTRACE_TEST, PL_INFO), "MsgTraceClose ...");
    return 0;
}

