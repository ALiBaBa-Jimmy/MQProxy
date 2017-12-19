#ifndef __UDT_TEST_H
#define __UDT_TEST_H

#include <signal.h>

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"
#include "xosencap.h"
//#include "udtlib.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void UdtTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

#ifdef __cplusplus
}
#endif

#endif

