/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月11日
**************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined (WIN32)
#  define DIAMETER_EXPORT __declspec(dllexport)
#else
#  define DIAMETER_EXPORT
#endif     /* WIN32 */

#endif //__CONFIG_H__
