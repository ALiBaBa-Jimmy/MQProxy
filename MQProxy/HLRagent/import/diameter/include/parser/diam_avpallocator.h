/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年12月13日
**************************************************************************/
#ifndef __DIAM_AVPALLOCATOR_H__
#define __DIAM_AVPALLOCATOR_H__

#include <util/diam_config.h>
#include <stddef.h>

DIAMETER_EXPORT void* containerMalloc(size_t s);

DIAMETER_EXPORT void containerFree(void *p);

#endif //__DIAM_AVPALLOCATOR_H__
