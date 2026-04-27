// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
*
* @file SysTypes.h
*
* System primitive data type, macro and constant definitions
*
***********************************************************************************/
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include "SysTypes.h"


#define LOG2(X)                 (31-__clz(X))                       ///< Log 2
typedef void (*CallbackNoParam_t)();  ///< Type for callback w/o any parameters

#define STRINGIFY(X)                _STRINGIFY(X)
#define _STRINGIFY(X)               #X

#ifdef __cplusplus
}
#endif
