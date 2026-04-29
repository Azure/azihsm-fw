/*++

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hsprt.h

Abstract:

    This is the main include file for minimum hsp runtime (hsprt) library

Author:

    Navin Pai (navinp)

--*/
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"
#include "inc/port/hsp_addr_map.h"
#include "riscvcpu.h"
#include "printf.h"


//
// The main function. All executable needs to provide an implementation of ths
// function
//
NORETURN void main();


#ifdef __clang__
//
// The function is a weak reference. If it's defined, it will be called before
// any other initialization in HsprtMain()
//
__attribute__((naked)) void PreMain() __attribute__((weak));
#endif


//
//  Helper functions
//


uint8_t memcmp_s(pcchar_t Arr1, pcchar_t Arr2, const uint16_t Length);


static INLINE uint32_t ByteSwapUint32(uint32_t u)
{
    return (((u & 0xff) << 24) | ((u & 0xff00) << 8) | ((u & 0xff0000) >> 8) |
            ((u & 0xff000000) >> 24));
}


//
// defined in string.h
//
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
