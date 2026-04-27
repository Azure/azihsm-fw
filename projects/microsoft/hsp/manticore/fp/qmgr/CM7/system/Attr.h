// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
 *
 * @file Attr.h
 *
 * Header file for compiler-specific function, variable, and type attributes.
 *
 ***********************************************************************************/
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************************
 *                                                                                  *
 * Dependencies                                                                     *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Public Constant Definitions                                                      *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Public Macros Definitions                                                        *
 *                                                                                  *
 ***********************************************************************************/
/// @brief  Compiler-specific keywords and operators
#define __unaligned

#define ATTR_ALIGN_OF(x)            __alignof__(x)      ///< Inquires the alignment of a type or variable
#if defined (__clang__) || defined (__GNUC__) || defined (__GNUG__)
#define ATTR_UNALIGNED          __unaligned             ///< Tells the compiler the pointer or variable is unaligned
#else
#define ATTR_UNALIGNED          __packed                ///< Tells the compiler the pointer or variable is unaligned
#endif

/// @brief  Compiler-specific attribute
#define ATTRIBUTE(x)                __attribute__((x))

/// @brief  Inline
#define ATTR_ALWAYS_INLINE          __attribute__((always_inline))
#define ATTR_NO_INLINE              __attribute__((noinline))

/// @brief  Unused and used attribute
#define ATTR_WEAK                   __attribute__((weak))
#define ATTR_UNUSED                 __attribute__((unused))
#define ATTR_USED                   __attribute__((used))
#if defined (__clang__) || defined (__GNUC__) || defined (__GNUG__)
#define ATTR_NAKED                  __attribute__((naked))
#endif

/// @brief  Align
#define ATTR_ALIGNED(x)             __attribute__((aligned(x)))
#define ATTR_PACKED                 __attribute__((packed))

/// @brief  Interrupt function
#define ATTR_INT(x)                 __attribute__((interrupt(x)))

/// @brief  Specifies a variable must be placed in a particular memory
#define ATTR_SECTION(x)             __attribute__((section(# x)))     ///< Specify memory section of a function or varible
#if defined (__clang__)
#define ATTR_SECTION_EX(x, y)   __attribute__((section(y # x)))
#else
#define ATTR_SECTION_EX(x, y)   __attribute__((section(# x), ## y))
#endif

#if defined (__clang__)
#define __STR_LINK(x)               # x
#define __SECTION(_qual)            ".bss.ARM.__at_"__STR_LINK (_qual)
#define ATTR_AT(x)                  __attribute__((section(__SECTION(x))))
#else
#define ATTR_AT(x)                  __attribute__((section(x)))         ///< Specify the absolute address of a variable
#endif

/// @brief  Arm compiler-specific pragmas
#ifndef PRAGMA
#define PRAGMA(x)               _Pragma(# x)
#endif

/// @brief  Packed objects are read and written using unaligned accesses
#if defined (__clang__)
#define PRAGMA_PACK(n)          PRAGMA("pack("# n ")")
#define PRAGMA_PACK_PUSH        PRAGMA("pack(push)")
#define PRAGMA_PACK_POP         PRAGMA("pack(pop)")
#else
#define PRAGMA_PACK(n)          PRAGMA("pack("# n ")")
#define PRAGMA_PACK_PUSH        PRAGMA("push")
#define PRAGMA_PACK_POP         PRAGMA("pop")
#endif

/// @brief  Places subsequent functions, variables in the named section depending on the section type.
#if defined (__clang__)
#define PRAGMA_SECTION(t, x)    PRAGMA(clang section t = # x)
#else
#define PRAGMA_SECTION(t, x)    PRAGMA(arm section(t) = # x)
#endif

/// @brief  Valid section types for PRAGMA_SECTION
#if defined (__clang__)
#define SECTION_TYPE_CODE       text
#define SECTION_TYPE_DATA       data
#define SECTION_TYPE_ZIDATA     bss
#else
#define SECTION_TYPE_CODE       code
#define SECTION_TYPE_DATA       rwdata
#define SECTION_TYPE_ZIDATA     zidata
#endif
#define SECTION_TYPE_RODATA         rodata

/***********************************************************************************
 *                                                                                  *
 * Public Data Type Definitions                                                     *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Public Function Declarations                                                     *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Public Inline Function Definitions                                               *
 *                                                                                  *
 ***********************************************************************************/

#ifdef __cplusplus
}
#endif
