//-----------------------------------------------------------------------------
//
// Copyright (c) 2022 Marvell. All rights reserved.
// The following file is subject to the limited use license agreement by and
// between Marvell and you, your employer or other entity on behalf of whom
// you act. In the absence of such license agreement the following file is
// subject to Marvell's standard Limited Use License Agreement.
//
//-----------------------------------------------------------------------------

//=============================================================================
//!
//! @brief POR Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.3.

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SharedStruct.h"
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RST_OSC                     :1;      ///<BIT [0] RST_OSC
        uint32_t RESERVED0                   :1;      ///<BIT [1] Reserved0
        uint32_t RST_CPUCP                   :1;      ///<BIT [2] RST_CPUCP
        uint32_t RST_CPU_JT                  :1;      ///<BIT [3] RST_CPU_JT
        uint32_t RST_GDMA                    :1;      ///<BIT [4] RST_GDMA
        uint32_t RST_BCP                     :1;      ///<BIT [5] RST_BCP
        uint32_t RST_APB                     :1;      ///<BIT [6] RST_APB
        uint32_t RST_NQM                     :1;      ///<BIT [7] RST_NQM
        uint32_t RST_GSRAM                   :1;      ///<BIT [8] RST_GSRAM
        uint32_t RESERVED1                   :2;      ///<BIT [10:9] Reserved1
        uint32_t RST_TRACE                   :1;      ///<BIT [11] RST_TRACE
        uint32_t RST_MINIAXI                 :1;      ///<BIT [12] RST_MINIAXI
        uint32_t RST_UART                    :1;      ///<BIT [13] RST_UART
        uint32_t RESERVED2                   :1;      ///<BIT [14] Reserved2
        uint32_t RST_TCON                    :1;      ///<BIT [15] RST_TCON
        uint32_t RESERVED3                   :1;      ///<BIT [16] Reserved3
        uint32_t RST_MCU                     :1;      ///<BIT [17] RST_MCU
        uint32_t RST_PCIE_MTX                :1;      ///<BIT [18] RST_PCIE_MTX
        uint32_t RST_CP0                     :1;      ///<BIT [19] RST_CP0
        uint32_t RST_CP1                     :1;      ///<BIT [20] RST_CP1
        uint32_t RST_CP_2X                   :1;      ///<BIT [21] RST_CP_2X
        uint32_t RST_FP0                     :1;      ///<BIT [22] RST_FP0
        uint32_t RST_FP1                     :1;      ///<BIT [23] RST_FP1
        uint32_t RST_FP2                     :1;      ///<BIT [24] RST_FP2
        uint32_t RST_SPI_TPM0                :1;      ///<BIT [25] RST_SPI_TPM0
        uint32_t RST_SPI_TPM1                :1;      ///<BIT [26] RST_SPI_TPM1
        uint32_t RESERVED4                   :5;      ///<BIT [31:27] Reserved4
    } b;
} ResetControl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t FP0_CPUWAIT                 :1;      ///<BIT [0] FP0_CPUWAIT
        uint32_t FP1_CPUWAIT                 :1;      ///<BIT [1] FP1_CPUWAIT
        uint32_t FP2_CPUWAIT                 :1;      ///<BIT [2] FP2_CPUWAIT
        uint32_t RESERVED                    :29;     ///<BIT [31:3] Reserved
    } b;
} FpRunstall_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t CP0_DBGETM_RST              :1;      ///<BIT [0] CP0_DBGETM_RST
        uint32_t CP1_DBGETM_RST              :1;      ///<BIT [1] CP1_DBGETM_RST
        uint32_t FP0_DBGETM_RST              :1;      ///<BIT [2] FP0_DBGETM_RST
        uint32_t FP1_DBGETM_RST              :1;      ///<BIT [3] FP1_DBGETM_RST
        uint32_t FP2_DBGETM_RST              :1;      ///<BIT [4] FP2_DBGETM_RST
        uint32_t RESERVED1                   :27;     ///<BIT [31:5] Reserved1
    } b;
} DbgetmResetControl_t;

/// @brief 0xC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RST_CRYPTO0                 :1;      ///<BIT [0] RST_CRYPTO0
        uint32_t RST_CRYPTO1                 :1;      ///<BIT [1] RST_CRYPTO1
        uint32_t RST_CRYPTO2                 :1;      ///<BIT [2] RST_CRYPTO2
        uint32_t RESERVED1                   :13;     ///<BIT [15:3] Reserved1
        uint32_t RST_UPKA                    :16;     ///<BIT [31:16] RST_UPKA
    } b;
} ResetControl1_t;

typedef struct
{
    ResetControl_t resetControl;                                            // 0x0 : Reset_Control / 
    FpRunstall_t fpRunstall;                                                // 0x4 : FP_Runstall / 
    DbgetmResetControl_t dbgetmResetControl;                                // 0x8 : DBGETM_Reset_Control / 
    ResetControl1_t resetControl1;                                          // 0xC : Reset_Control1 / 
} Por_t;

COMPILE_ASSERT(offsetof(Por_t,resetControl)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Por_t,fpRunstall)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Por_t,dbgetmResetControl)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Por_t,resetControl1)==0xC,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Por_t rPor; ///< 0xB0004000
