#ifndef _XOS_NTPC_H_
#define _XOS_NTPC_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


XS32  XOS_FIDNTPC(HANDLE hDir,XS32 argc, XS8** argv);
XS8 NTPC_Init(XVOID*p1,XVOID*p2);
XS8 NTPC_msgProc(XVOID* pMsgP,XVOID*sb );
XS8 NTPC_timerProc( t_BACKPARA* pParam);
int ntpc_setTimerLen(int timerLen);
char *ntpc_setServerIp(char *szIp);
void ntpc_showIp();
int sntpc_SynTime(char * szIp);
void ntpc_showcfg(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void ntpcshowcfg(void);
int ntpc_getTimeZone(char *szTimeZone);





#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */

