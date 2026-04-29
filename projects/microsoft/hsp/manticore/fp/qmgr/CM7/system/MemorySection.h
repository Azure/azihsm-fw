// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
 *
 * @file MemorySection.h
 *
 * Memory section macro definitions for code and data.
 *
 ***********************************************************************************/
#pragma once

/***********************************************************************************
 *                                                                                  *
 * Dependencies                                                                     *
 *                                                                                  *
 ***********************************************************************************/
#include "Attr.h"

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
/// @brief  Data attribute has to be in either ZI or RW. Since RW is default, the define is empty.
///         __XX__DATA macros below are private and shall not be used outside this header file.
#if defined (__clang__) || defined (__GNUC__) || defined (__GNUG__)
#define __ZI_DATA                           ".bss."
#define __RW_DATA
#define __RO_DATA
#else
#define __ZI_DATA                           zero_init
#define __RW_DATA
#define __RO_DATA
#endif

/// @brief  Shared code placement macros
#define __ATTR_SHARED_CODE(sec)                 ATTR_SECTION(sec) ATTR_USED

/// @brief  Shared data placement macros
#define __ATTR_SHARED_DATA(sec, param)          ATTR_SECTION_EX((sec), (param)) ATTR_USED

/// @brief  Macors for Arm interrupt vector table
#define VECTOR_CODE                             ATTR_SECTION(vector_code)

/// @brief  Memory section for fastest memory
#define FASTEST_CODE                            ATTR_SECTION(fastest_code)

#define FASTEST_DATA                            ATTR_SECTION(fastest_data)
#define FASTEST_DATA_CONST                      ATTR_SECTION(fastest_data_const)
#define FASTEST_DATA_RO                         ATTR_SECTION_EX(fastest_data_ro, __RO_DATA)
#define FASTEST_DATA_UI                         ATTR_SECTION_EX(fastest_data_ui, __ZI_DATA)
#define FASTEST_DATA_ZI                         ATTR_SECTION_EX(fastest_data, __ZI_DATA)

/// @brief  Memory section for fast memory
#define FAST_CODE                               ATTR_SECTION(fast_code)

#define FAST_DATA                               ATTR_SECTION(fast_data)
#define FAST_DATA_CONST                         ATTR_SECTION(fast_data_const)
#define FAST_DATA_RO                            ATTR_SECTION_EX(fast_data_ro, __RO_DATA)
#define FAST_DATA_UI                            ATTR_SECTION_EX(fast_data_ui, __ZI_DATA)
#define FAST_DATA_ZI                            ATTR_SECTION_EX(fast_data, __ZI_DATA)

/// @brief  Memory section for normal memory
#define NORMAL_CODE                             ATTR_SECTION(normal_code)

#define NORMAL_DATA                             ATTR_SECTION(normal_data)
#define NORMAL_DATA_CONST                       ATTR_SECTION(normal_data_const)
#define NORMAL_DATA_RO                          ATTR_SECTION_EX(normal_data_ro, __RO_DATA)
#define NORMAL_DATA_UI                          ATTR_SECTION_EX(normal_data_ui, __ZI_DATA)
#define NORMAL_DATA_ZI                          ATTR_SECTION_EX(normal_data, __ZI_DATA)

/// @brief  Memory section for slow memory
#define SLOW_CODE                               ATTR_SECTION(slow_code) ATTR_NO_INLINE

#define SLOW_DATA                               ATTR_SECTION(slow_data)
#define SLOW_DATA_CONST                         ATTR_SECTION(slow_data_const)
#define SLOW_DATA_RO                            ATTR_SECTION_EX(slow_data_ro, __RO_DATA)
#define SLOW_DATA_UI                            ATTR_SECTION_EX(slow_data_ui, __ZI_DATA)
#define SLOW_DATA_ZI                            ATTR_SECTION_EX(slow_data, __ZI_DATA)

/// @brief  Memory section for slowest memory
#define SLOWEST_CODE                            ATTR_SECTION(slowest_code) ATTR_NO_INLINE

#define SLOWEST_DATA                            ATTR_SECTION(slowest_data)
#define SLOWEST_DATA_CONST                      ATTR_SECTION(slowest_data_const)
#define SLOWEST_DATA_RO                         ATTR_SECTION_EX(slowest_data_ro, __RO_DATA)
#define SLOWEST_DATA_UI                         ATTR_SECTION_EX(slowest_data_ui, __ZI_DATA)
#define SLOWEST_DATA_ZI                         ATTR_SECTION_EX(slowest_data, __ZI_DATA)

/// @brief  Memory section for fast shared data
#define FAST_CODE_SHARED                        __ATTR_SHARED_CODE(fast_code_shared)

#define FAST_DATA_SHARED                        __ATTR_SHARED_DATA(fast_data_shared, __RW_DATA)
#define FAST_DATA_SHARED_CONST                  __ATTR_SHARED_DATA(fast_data_shared_const, __RO_DATA)
#define FAST_DATA_SHARED_RO                     __ATTR_SHARED_DATA(fast_data_shared_ro, __RO_DATA)
#define FAST_DATA_SHARED_UI                     __ATTR_SHARED_DATA(fast_data_shared_ui, __ZI_DATA)
#define FAST_DATA_SHARED_ZI                     __ATTR_SHARED_DATA(fast_data_shared, __ZI_DATA)

/// @brief  Memory section for fast shared data
#define NORMAL_CODE_SHARED                      __ATTR_SHARED_CODE(normal_code_shared)

#define NORMAL_DATA_SHARED                      __ATTR_SHARED_DATA(normal_data_shared, __RW_DATA)
#define NORMAL_DATA_SHARED_CONST                __ATTR_SHARED_DATA(normal_data_shared_const, __RO_DATA)
#define NORMAL_DATA_SHARED_RO                   __ATTR_SHARED_DATA(normal_data_shared_ro, __RO_DATA)
#define NORMAL_DATA_SHARED_UI                   __ATTR_SHARED_DATA(normal_data_shared_ui, __ZI_DATA)
#define NORMAL_DATA_SHARED_ZI                   __ATTR_SHARED_DATA(normal_data_shared, __ZI_DATA)

/// @brief  Memory section for slow shared data
#define SLOW_CODE_SHARED                        __ATTR_SHARED_CODE(slow_code_shared)

#define SLOW_DATA_SHARED                        __ATTR_SHARED_DATA(slow_data_shared, __RW_DATA)
#define SLOW_DATA_SHARED_CONST                  __ATTR_SHARED_DATA(slow_data_shared_const, __RO_DATA)
#define SLOW_DATA_SHARED_RO                     __ATTR_SHARED_DATA(slow_data_shared_ro, __RO_DATA)
#define SLOW_DATA_SHARED_UI                     __ATTR_SHARED_DATA(slow_data_shared_ui, __ZI_DATA)
#define SLOW_DATA_SHARED_ZI                     __ATTR_SHARED_DATA(slow_data_shared, __ZI_DATA)

/// @brief  Memory section for slowest shared data
#define SLOWEST_CODE_SHARED                     __ATTR_SHARED_CODE(slowest_code_shared)

#define SLOWEST_DATA_SHARED                     __ATTR_SHARED_DATA(slowest_data_shared, __RW_DATA)
#define SLOWEST_DATA_SHARED_CONST               __ATTR_SHARED_DATA(slowest_data_shared_const, __RO_DATA)
#define SLOWEST_DATA_SHARED_RO                  __ATTR_SHARED_DATA(slowest_data_shared_ro, __RO_DATA)
#define SLOWEST_DATA_SHARED_UI                  __ATTR_SHARED_DATA(slowest_data_shared_ui, __ZI_DATA)
#define SLOWEST_DATA_SHARED_ZI                  __ATTR_SHARED_DATA(slowest_data_shared, __ZI_DATA)

/// @brief  Initialization code & data
#define INIT_CODE                               ATTR_SECTION(init_code) ATTR_NO_INLINE
#define INIT_DATA                               ATTR_SECTION(init_data)

/// @brief  Firmware info data
#define DATA_SECTION_FW_INFO                    ATTR_SECTION(fw_info_data)

/// @brief  Specifies a section to be used for all subsequent functions or objects in a file
#define MODULE_SECTION_CODE_FASTEST             PRAGMA_SECTION(SECTION_TYPE_CODE, fastest_code)
#define MODULE_SECTION_CODE_FAST                PRAGMA_SECTION(SECTION_TYPE_CODE, fast_code)
#define MODULE_SECTION_CODE_NORMAL              PRAGMA_SECTION(SECTION_TYPE_CODE, normal_code)
#define MODULE_SECTION_CODE_SLOW                PRAGMA_SECTION(SECTION_TYPE_CODE, slow_code)
#define MODULE_SECTION_CODE_SLOWEST             PRAGMA_SECTION(SECTION_TYPE_CODE, slowest_code)
#define MODULE_SECTION_CODE_SLOWEST_SHARED      PRAGMA_SECTION(SECTION_TYPE_CODE, slowest_code_shared)

#define MODULE_SECTION_RODATA_FASTEST           PRAGMA_SECTION(SECTION_TYPE_RODATA, fastest_data_ro)
#define MODULE_SECTION_RODATA_FAST              PRAGMA_SECTION(SECTION_TYPE_RODATA, fast_data_ro)
#define MODULE_SECTION_RODATA_FAST_UI           PRAGMA_SECTION(SECTION_TYPE_RODATA, .bss.fast_data_ui)
#define MODULE_SECTION_RODATA_FAST_SHARED       PRAGMA_SECTION(SECTION_TYPE_RODATA, fast_data_shared_ro)
#define MODULE_SECTION_RODATA_NORMAL            PRAGMA_SECTION(SECTION_TYPE_RODATA, normal_data_ro)
#define MODULE_SECTION_RODATA_SLOW              PRAGMA_SECTION(SECTION_TYPE_RODATA, slow_data_ro)
#define MODULE_SECTION_RODATA_SLOW_SHARED       PRAGMA_SECTION(SECTION_TYPE_RODATA, slow_data_shared_ro)
#define MODULE_SECTION_RODATA_SLOWEST           PRAGMA_SECTION(SECTION_TYPE_RODATA, slowest_data)
#define MODULE_SECTION_RODATA_SLOWEST_SHARED    PRAGMA_SECTION(SECTION_TYPE_RODATA, slowest_data_shared_ro)

#define MODULE_SECTION_RWDATA_FASTEST           PRAGMA_SECTION(SECTION_TYPE_DATA, fastest_data)
#define MODULE_SECTION_RWDATA_FAST              PRAGMA_SECTION(SECTION_TYPE_DATA, fast_data)
#define MODULE_SECTION_RWDATA_FAST_UI           PRAGMA_SECTION(SECTION_TYPE_DATA, .bss.fast_data_ui)
#define MODULE_SECTION_RWDATA_FAST_SHARED       PRAGMA_SECTION(SECTION_TYPE_DATA, fast_data_shared)
#define MODULE_SECTION_RWDATA_NORMAL            PRAGMA_SECTION(SECTION_TYPE_DATA, normal_data)
#define MODULE_SECTION_RWDATA_SLOW              PRAGMA_SECTION(SECTION_TYPE_DATA, slow_data)
#define MODULE_SECTION_RWDATA_SLOW_SHARED       PRAGMA_SECTION(SECTION_TYPE_DATA, slow_data_shared)
#define MODULE_SECTION_RWDATA_SLOWEST           PRAGMA_SECTION(SECTION_TYPE_DATA, slowest_data)
#define MODULE_SECTION_RWDATA_SLOWEST_SHARED    PRAGMA_SECTION(SECTION_TYPE_DATA, slowest_data_shared)

#define MODULE_SECTION_ZIDATA_FASTEST           PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.fastest_data)
#define MODULE_SECTION_ZIDATA_FAST              PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.fast_data)
#define MODULE_SECTION_ZIDATA_FAST_UI           PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.fast_data_ui)
#define MODULE_SECTION_ZIDATA_FAST_SHARED       PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.fast_data_shared)
#define MODULE_SECTION_ZIDATA_NORMAL            PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.normal_data)
#define MODULE_SECTION_ZIDATA_SLOW              PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.slow_data)
#define MODULE_SECTION_ZIDATA_SLOW_SHARED       PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.slow_data_shared)
#define MODULE_SECTION_ZIDATA_SLOWEST           PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.slowest_data)
#define MODULE_SECTION_ZIDATA_SLOWEST_SHARED    PRAGMA_SECTION(SECTION_TYPE_ZIDATA, .bss.slowest_data_shared)

#define MODULE_OPTIMIZE_FOR_MAX_SPEED           PRAGMA(Omax)

/// Memory section directive for fast code and fast data
#define MODULE_SECTION_FAST    \
    MODULE_SECTION_CODE_FAST   \
    MODULE_SECTION_RWDATA_FAST \
    MODULE_SECTION_ZIDATA_FAST \
    MODULE_SECTION_RODATA_FAST

/// Memory section directive for normal code and data
#define MODULE_SECTION_NORMAL    \
    MODULE_SECTION_CODE_NORMAL   \
    MODULE_SECTION_RWDATA_NORMAL \
    MODULE_SECTION_ZIDATA_NORMAL \
    MODULE_SECTION_RODATA_NORMAL

/// Memory section directive for slowest code and data
#define MODULE_SECTION_SLOWEST    \
    MODULE_SECTION_CODE_SLOWEST   \
    MODULE_SECTION_RWDATA_SLOWEST \
    MODULE_SECTION_ZIDATA_SLOWEST \
    MODULE_SECTION_RODATA_SLOWEST

/// Memory section directive for fast shared code and data
#define MODULE_SECTION_SHARED_FAST    \
    MODULE_SECTION_CODE_SHARED_FAST   \
    MODULE_SECTION_RWDATA_SHARED_FAST \
    MODULE_SECTION_ZIDATA_SHARED_FAST \
    MODULE_SECTION_RODATA_SHARED_FAST

/// Memory section directive for normal shared code and data
#define MODULE_SECTION_SHARED_NORMAL    \
    MODULE_SECTION_CODE_SHARED_NORMAL   \
    MODULE_SECTION_RWDATA_SHARED_NORMAL \
    MODULE_SECTION_ZIDATA_SHARED_NORMAL \
    MODULE_SECTION_RODATA_SHARED_NORMAL

/// Memory section directive for slowest shared code and data
#define MODULE_SECTION_SHARED_SLOWEST    \
    MODULE_SECTION_CODE_SLOWEST_SHARED   \
    MODULE_SECTION_RWDATA_SLOWEST_SHARED \
    MODULE_SECTION_ZIDATA_SLOWEST_SHARED \
    MODULE_SECTION_RODATA_SLOWEST_SHARED

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
