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
//! @brief FENCE_GSRAM Registers
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
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar0End_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar0Start_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar0Rd0_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar0Wr0_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar1End_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar1Start_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar1Rd0_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar1Wr0_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar2End_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar2Start_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar2Rd0_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar2Wr0_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar3End_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar3Start_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar3Rd0_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar3Wr0_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar4End_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar4Start_t;

/// @brief 0x88
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar4Rd0_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar4Wr0_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar5End_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar5Start_t;

/// @brief 0xA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar5Rd0_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar5Wr0_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar6End_t;

/// @brief 0xC4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar6Start_t;

/// @brief 0xC8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar6Rd0_t;

/// @brief 0xD0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar6Wr0_t;

/// @brief 0xE0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar7End_t;

/// @brief 0xE4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceGsrambar7Start_t;

/// @brief 0xE8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar7Rd0_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceGsrambar7Wr0_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} Bar8End_t;

/// @brief 0x104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} Bar8Start_t;

/// @brief 0x108
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} Bar8Rd0_t;

/// @brief 0x110
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} Bar8Wr0_t;

/// @brief 0x120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} Bar9End_t;

/// @brief 0x124
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} Bar9Start_t;

/// @brief 0x128
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} Bar9Rd0_t;

/// @brief 0x130
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} Bar9Wr0_t;

typedef struct
{
    FenceGsrambar0End_t bar0End;                                            // 0x0 : bar0_end / 
    FenceGsrambar0Start_t bar0Start;                                        // 0x4 : bar0_start / 
    FenceGsrambar0Rd0_t bar0Rd0;                                            // 0x8 : bar0_rd0 / 
    uint32_t bar0Rd1InitiatorEn;                                            // 0xC : bar0_rd1 / 
    FenceGsrambar0Wr0_t bar0Wr0;                                            // 0x10 : bar0_wr0 / 
    uint32_t bar0Wr1InitiatorEn;                                            // 0x14 : bar0_wr1 / 
    uint8_t rsvd18[8];                                                      // 0x18 : rsvd_18 / rsvd_18
    FenceGsrambar1End_t bar1End;                                            // 0x20 : bar1_end / 
    FenceGsrambar1Start_t bar1Start;                                        // 0x24 : bar1_start / 
    FenceGsrambar1Rd0_t bar1Rd0;                                            // 0x28 : bar1_rd0 / 
    uint32_t bar1Rd1InitiatorEn;                                            // 0x2C : bar1_rd1 / 
    FenceGsrambar1Wr0_t bar1Wr0;                                            // 0x30 : bar1_wr0 / 
    uint32_t bar1Wr1InitiatorEn;                                            // 0x34 : bar1_wr1 / 
    uint8_t rsvd38[8];                                                      // 0x38 : rsvd_38 / rsvd_38
    FenceGsrambar2End_t bar2End;                                            // 0x40 : bar2_end / 
    FenceGsrambar2Start_t bar2Start;                                        // 0x44 : bar2_start / 
    FenceGsrambar2Rd0_t bar2Rd0;                                            // 0x48 : bar2_rd0 / 
    uint32_t bar2Rd1InitiatorEn;                                            // 0x4C : bar2_rd1 / 
    FenceGsrambar2Wr0_t bar2Wr0;                                            // 0x50 : bar2_wr0 / 
    uint32_t bar2Wr1InitiatorEn;                                            // 0x54 : bar2_wr1 / 
    uint8_t rsvd58[8];                                                      // 0x58 : rsvd_58 / rsvd_58
    FenceGsrambar3End_t bar3End;                                            // 0x60 : bar3_end / 
    FenceGsrambar3Start_t bar3Start;                                        // 0x64 : bar3_start / 
    FenceGsrambar3Rd0_t bar3Rd0;                                            // 0x68 : bar3_rd0 / 
    uint32_t bar3Rd1InitiatorEn;                                            // 0x6C : bar3_rd1 / 
    FenceGsrambar3Wr0_t bar3Wr0;                                            // 0x70 : bar3_wr0 / 
    uint32_t bar3Wr1InitiatorEn;                                            // 0x74 : bar3_wr1 / 
    uint8_t rsvd78[8];                                                      // 0x78 : rsvd_78 / rsvd_78
    FenceGsrambar4End_t bar4End;                                            // 0x80 : bar4_end / 
    FenceGsrambar4Start_t bar4Start;                                        // 0x84 : bar4_start / 
    FenceGsrambar4Rd0_t bar4Rd0;                                            // 0x88 : bar4_rd0 / 
    uint32_t bar4Rd1InitiatorEn;                                            // 0x8C : bar4_rd1 / 
    FenceGsrambar4Wr0_t bar4Wr0;                                            // 0x90 : bar4_wr0 / 
    uint32_t bar4Wr1InitiatorEn;                                            // 0x94 : bar4_wr1 / 
    uint8_t rsvd98[8];                                                      // 0x98 : rsvd_98 / rsvd_98
    FenceGsrambar5End_t bar5End;                                            // 0xA0 : bar5_end / 
    FenceGsrambar5Start_t bar5Start;                                        // 0xA4 : bar5_start / 
    FenceGsrambar5Rd0_t bar5Rd0;                                            // 0xA8 : bar5_rd0 / 
    uint32_t bar5Rd1InitiatorEn;                                            // 0xAC : bar5_rd1 / 
    FenceGsrambar5Wr0_t bar5Wr0;                                            // 0xB0 : bar5_wr0 / 
    uint32_t bar5Wr1InitiatorEn;                                            // 0xB4 : bar5_wr1 / 
    uint8_t rsvdB8[8];                                                      // 0xB8 : rsvd_b8 / rsvd_b8
    FenceGsrambar6End_t bar6End;                                            // 0xC0 : bar6_end / 
    FenceGsrambar6Start_t bar6Start;                                        // 0xC4 : bar6_start / 
    FenceGsrambar6Rd0_t bar6Rd0;                                            // 0xC8 : bar6_rd0 / 
    uint32_t bar6Rd1InitiatorEn;                                            // 0xCC : bar6_rd1 / 
    FenceGsrambar6Wr0_t bar6Wr0;                                            // 0xD0 : bar6_wr0 / 
    uint32_t bar6Wr1InitiatorEn;                                            // 0xD4 : bar6_wr1 / 
    uint8_t rsvdD8[8];                                                      // 0xD8 : rsvd_d8 / rsvd_d8
    FenceGsrambar7End_t bar7End;                                            // 0xE0 : bar7_end / 
    FenceGsrambar7Start_t bar7Start;                                        // 0xE4 : bar7_start / 
    FenceGsrambar7Rd0_t bar7Rd0;                                            // 0xE8 : bar7_rd0 / 
    uint32_t bar7Rd1InitiatorEn;                                            // 0xEC : bar7_rd1 / 
    FenceGsrambar7Wr0_t bar7Wr0;                                            // 0xF0 : bar7_wr0 / 
    uint32_t bar7Wr1InitiatorEn;                                            // 0xF4 : bar7_wr1 / 
    uint8_t rsvdF8[8];                                                      // 0xF8 : rsvd_f8 / rsvd_f8
    Bar8End_t bar8End;                                                      // 0x100 : bar8_end / 
    Bar8Start_t bar8Start;                                                  // 0x104 : bar8_start / 
    Bar8Rd0_t bar8Rd0;                                                      // 0x108 : bar8_rd0 / 
    uint32_t bar8Rd1InitiatorEn;                                            // 0x10C : bar8_rd1 / 
    Bar8Wr0_t bar8Wr0;                                                      // 0x110 : bar8_wr0 / 
    uint32_t bar8Wr1InitiatorEn;                                            // 0x114 : bar8_wr1 / 
    uint8_t rsvd118[8];                                                     // 0x118 : rsvd_118 / rsvd_118
    Bar9End_t bar9End;                                                      // 0x120 : bar9_end / 
    Bar9Start_t bar9Start;                                                  // 0x124 : bar9_start / 
    Bar9Rd0_t bar9Rd0;                                                      // 0x128 : bar9_rd0 / 
    uint32_t bar9Rd1InitiatorEn;                                            // 0x12C : bar9_rd1 / 
    Bar9Wr0_t bar9Wr0;                                                      // 0x130 : bar9_wr0 / 
    uint32_t bar9Wr1InitiatorEn;                                            // 0x134 : bar9_wr1 / 
} FenceGsram_t;

COMPILE_ASSERT(offsetof(FenceGsram_t,bar0End)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar0Start)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar0Rd0)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar0Rd1InitiatorEn)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar0Wr0)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar0Wr1InitiatorEn)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1End)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1Start)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1Rd0)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1Rd1InitiatorEn)==0x2C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1Wr0)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar1Wr1InitiatorEn)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2End)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2Start)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2Rd0)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2Rd1InitiatorEn)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2Wr0)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar2Wr1InitiatorEn)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3End)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3Start)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3Rd0)==0x68,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3Rd1InitiatorEn)==0x6C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3Wr0)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar3Wr1InitiatorEn)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4End)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4Start)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4Rd0)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4Rd1InitiatorEn)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4Wr0)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar4Wr1InitiatorEn)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5End)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5Start)==0xA4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5Rd0)==0xA8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5Rd1InitiatorEn)==0xAC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5Wr0)==0xB0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar5Wr1InitiatorEn)==0xB4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6End)==0xC0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6Start)==0xC4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6Rd0)==0xC8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6Rd1InitiatorEn)==0xCC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6Wr0)==0xD0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar6Wr1InitiatorEn)==0xD4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7End)==0xE0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7Start)==0xE4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7Rd0)==0xE8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7Rd1InitiatorEn)==0xEC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7Wr0)==0xF0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar7Wr1InitiatorEn)==0xF4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8End)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8Start)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8Rd0)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8Rd1InitiatorEn)==0x10C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8Wr0)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar8Wr1InitiatorEn)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9End)==0x120,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9Start)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9Rd0)==0x128,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9Rd1InitiatorEn)==0x12C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9Wr0)==0x130,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceGsram_t,bar9Wr1InitiatorEn)==0x134,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile FenceGsram_t rFenceGsram; ///< 0xB0206000
