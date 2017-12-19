#ifndef __UDP_SERVER_H
#define __UDP_SERVER_H

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

void UdpLinkStartByIndex(XU16 index);
void UdpStartSendTimer(void);


#ifdef __cplusplus
}
#endif

#endif
