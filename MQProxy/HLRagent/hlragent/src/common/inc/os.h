#pragma once



#ifdef WIN32

#include "Winsock2.h"
#include <windows.h>

#else
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#include "string.h"
#include <stdio.h>
