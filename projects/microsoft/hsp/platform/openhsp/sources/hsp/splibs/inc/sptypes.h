/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    sptypes.h

Abstract:

    Common types and defines used across HSP

Author:

    Peng Li (pengfeli)

--*/

#pragma once

#include <stdint.h>

typedef char* pchar_t;
typedef const char* pcchar_t;
typedef volatile char* pvchar_t;

typedef unsigned char uchar_t;
typedef unsigned char* puchar_t;
typedef volatile char* pvuchar_t;

typedef uint8_t* puint8_t;
typedef const uint8_t* pcuint8_t;
typedef volatile uint8_t* pvuint8_t;

typedef uint16_t* puint16_t;
typedef const uint16_t* pcuint16_t;
typedef volatile uint16_t* pvuint16_t;

typedef uint32_t* puint32_t;
typedef const uint32_t* pcuint32_t;
typedef volatile uint32_t* pvuint32_t;

typedef int32_t* pint32_t;
typedef const int32_t* pcint32_t;
typedef volatile int32_t* pvint32_t;

typedef uint64_t* puint64_t;
typedef const uint64_t* pcuint64_t;
typedef volatile uint64_t* pvuint64_t;

typedef void* pvoid_t;
typedef const void* pcvoid_t;


//
// All external to module functions should add this prefix
//
#define HSP_API


#ifdef PLATFORM_RISCV

typedef unsigned int uintptr_t;


//
// The naked attribute allows compiler to not provide prologue/epilogue for
// the function marked with this attribute. Only basic asm should be used
//
#define NAKED_FUNCTION              __attribute__((naked))

//
// Marks the function as noreturn.
//
#define NORETURN                    __attribute__((noreturn))

//
// Aligns the function at 2 to the power x byte boundary
//
#define ALIGN(x)                    __attribute__((aligned(x)))

//
// Specifies the section for the function
//
#define SECTION(n)                  __attribute__((section(n)))

//
// Specifies that the return value must be inspected
//
#define INSPECT_RETURN(x)           __attribute__((warn_unused_result))

//
// Specfies to the compiler that the code is unreachable
//
#define UNREACHABLE()               __builtin_unreachable()

//
// Interrupt handler attribute
//
#define INTERRUPT()                 __attribute__((interrupt))

//
// Specifies that the variable is used
//
#define GCC_USED()                  __attribute__((used))


#define OFFSETOF(x, y)              offsetof(x, y)

//
// __builtin_expect provides the compiler with branch prediction
// information. This allows compiler to generate code that does
// not thrash the pipeline
//
#define UNLIKELY(X)                 __builtin_expect((X), 0)
#define LIKELY(X)                   __builtin_expect((X), 1)

//
// Use to fallthrough case statement to avoid warning
//
#define FALL_THROUGH                __attribute__((fallthrough))

//
// Use to notify GCC to not reorder
//
#define COMPILER_BARRIER()          asm volatile("" ::: "memory")

//
// Force GCC to inline function
//
#define INLINE                      __attribute__((always_inline)) inline

//
// Force clang to not inline functions
//
#define NOINLINE                    __attribute__((__noinline__))

//
// Used to quiet down the compiler warnings
//
#define UNREFERENCED_PARAMETER(P)   (void)(P)

//
// compiletime asserts (failure results in error C2118: negative subscript)
//
#define C_ASSERT(e)                 typedef char __C_ASSERT__[(e) ? 1 : -1]


//
// Calculate the byte offset of a field in a structure of type type.
//
#define FIELD_OFFSET(type, field)   ((uint32_t) & (((type*)0)->field))


//
// Calculate the size of a field in a structure of type type, without
// knowing or stating the type of the field.
//
#define RTL_FIELD_SIZE(type, field) (sizeof(((type*)0)->field))


//
// Calculate the size of a structure of type type up through and
// including a field.
//
#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))


//
// Calculate the number of elements in an array
//
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

#ifdef assert
#undef assert
#define assert(__e)  ((__e) ? (void)0 : (__FILE__, __LINE__, __func__, #__e))
#endif

#define static_assert _Static_assert

#ifdef NULL
#undef NULL
#define NULL 0
#endif

#elif defined PLATFORM_WIN

#define UNLIKELY(X) (X)
#define LIKELY(X)   (X)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

#elif defined __unix__

#define UNLIKELY(X) (X)
#define LIKELY(X)   (X)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

//
// Force GCC to inline function
//
#define INLINE                      __attribute__((always_inline)) inline

#else
#error unknown platform
#endif
