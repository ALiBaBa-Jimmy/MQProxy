
#ifndef __MEM_TEST_H
#define __MEM_TEST_H

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"
#include "xosencap.h"
#include "xosmem.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void MemTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

#ifdef __cplusplus
}
#endif

#endif
