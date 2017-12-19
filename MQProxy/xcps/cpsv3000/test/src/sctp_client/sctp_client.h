#ifndef __SCTP_CLIENT_H
#define __SCTP_CLIENT_H

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
void SctpGetSendLenVar(XCHAR* pStr , XU32 *pOut);


#ifdef __cplusplus
}
#endif

#endif
