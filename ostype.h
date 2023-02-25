#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#pragma warning(disable: 4996)
#pragma warning(disable: 4251)
#pragma comment(lib, "ws2_32.lib")

#include "stdafx_coroutine.h"

#include <objbase.h>
#include <pthread.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <MSWSock.h>
#include <WinBase.h>
#include <Windows.h>
#include <direct.h>
#include <unordered_map>

#if _MSC_VER == 1600
#include <stdint.h>
#endif


typedef short				int16_t;
typedef int					int32_t;
typedef	long long			int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef	unsigned long long	uint64_t;
typedef int					socklen_t;
typedef unsigned char		uchar_t;
typedef int					net_handle_t;
typedef int					conn_handle_t;

#define NETLIB_INVALID_HANDLE	-1
#define DEBUG_MODEL		0
#define SERVICE_MODEL	1

const uint32_t INVALID_UINT32 = (uint32_t) - 1;
const uint32_t INVALID_VALUE = 0;

#endif	//__OS_TYPE_H__
