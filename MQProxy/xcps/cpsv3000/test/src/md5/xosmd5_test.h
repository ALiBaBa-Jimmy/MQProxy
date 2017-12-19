
#ifndef __XOSMD5_TEST_H
#define __XOSMD5_TEST_H

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"
#include "xosmd5.h"
#include "xosencap.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void Md5TestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

#ifdef __cplusplus
}
#endif

#endif
