/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��6��11��
**************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined (WIN32)
#  define DIAMETER_EXPORT __declspec(dllexport)
#else
#  define DIAMETER_EXPORT
#endif     /* WIN32 */

#endif //__CONFIG_H__
