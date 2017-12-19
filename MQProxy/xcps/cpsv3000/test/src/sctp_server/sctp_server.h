#ifndef __SCTP_SERVER_H
#define __SCTP_SERVER_H

#include <signal.h>

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"
#include "xosntl.h"
#include "xosencap.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

//void NtlTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
//void NtlShowCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

void SctpLinkStartByIndex(XU16 index);
void SctpStartSendTimer(void);


#ifdef __cplusplus
}
#endif

#endif
