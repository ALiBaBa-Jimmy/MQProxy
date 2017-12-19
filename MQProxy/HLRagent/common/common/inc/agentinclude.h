#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "xostype.h"
#include "xosshell.h"
#include "fid_def.h"
#include "SPRTraceStruct.h"
#include "taskcommon.h"
#include "SPRDbErrorCode.h"

#if 0

#ifdef WIN32
	#ifdef AGENTHARDDB_EXPORTS
	#define AGENTHARDDB_API __declspec(dllexport)
	#else
	#define AGENTHARDDB_API __declspec(dllimport)
	#endif
#else
	#define AGENTHARDDB_API
 #endif
 
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */



