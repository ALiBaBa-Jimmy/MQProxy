//#ifdef XOS_LINUX || XOS_VXWORKS ||XOS_WIN32

#ifndef __SNPRINTF_TEST_H
#define __SNPRINTF_TEST_H

#include <stdarg.h>
#include <stdio.h>

#include "xosos.h"
#include "xosshell.h"
#include "xosmmgt.h"
#include "xmlparser.h"
#include "xosmodule.h"
#include "xostype.h"
#include "xosencap.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

XS32 SnprintfTest(HANDLE hdir, XS32 argc, XCHAR** argv);

void SnprintfTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
#ifdef __cplusplus
}
#endif

#endif /* __SNPRINTF_TEST_H */

//#endif /* XOS_VXWORKS */

