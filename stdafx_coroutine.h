#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#ifdef WIN32 
#define COROUTINE_BASE __declspec(dllexport)
#else
#define COROUTINE_BASE __declspec(dllimport)
#endif