#ifndef __NTL_TEST_H
#define __NTL_TEST_H

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

void AddHighTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void AddLowTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void AutoTestLowTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void AutoTestHighTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void RunTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void StopTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void GetLocalTimeT(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void GetTickTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void TestCreateTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
void TestBeginTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

#ifdef __cplusplus
}
#endif

#endif
