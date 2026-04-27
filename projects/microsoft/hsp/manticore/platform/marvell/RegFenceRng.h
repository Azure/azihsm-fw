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
//! @brief FENCE_RNG Registers
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
} FenceRngbar0End_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar0Start_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar0Rd0_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar0Wr0_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar1End_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar1Start_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar1Rd0_t;

/// @brief 0x30
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar1Wr0_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar2End_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar2Start_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar2Rd0_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar2Wr0_t;

/// @brief 0x60
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar3End_t;

/// @brief 0x64
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar3Start_t;

/// @brief 0x68
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar3Rd0_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar3Wr0_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar4End_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar4Start_t;

/// @brief 0x88
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar4Rd0_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar4Wr0_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar5End_t;

/// @brief 0xA4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar5Start_t;

/// @brief 0xA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar5Rd0_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar5Wr0_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar6End_t;

/// @brief 0xC4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar6Start_t;

/// @brief 0xC8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar6Rd0_t;

/// @brief 0xD0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar6Wr0_t;

/// @brief 0xE0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar7End_t;

/// @brief 0xE4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ADDR11_0                    :12;     ///<BIT [11:0] addr11_0
        uint32_t ADDR31_12                   :20;     ///<BIT [31:12] addr31_12
    } b;
} FenceRngbar7Start_t;

/// @brief 0xE8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar7Rd0_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t HSP_EN                      :1;      ///<BIT [0] hsp_en
        uint32_t INITIATOR_EN                :31;     ///<BIT [31:1] initiator_en
    } b;
} FenceRngbar7Wr0_t;

typedef struct
{
    FenceRngbar0End_t bar0End;                                              // 0x0 : bar0_end / 
    FenceRngbar0Start_t bar0Start;                                          // 0x4 : bar0_start / 
    FenceRngbar0Rd0_t bar0Rd0;                                              // 0x8 : bar0_rd0 / 
    uint32_t bar0Rd1InitiatorEn;                                            // 0xC : bar0_rd1 / 
    FenceRngbar0Wr0_t bar0Wr0;                                              // 0x10 : bar0_wr0 / 
    uint32_t bar0Wr1InitiatorEn;                                            // 0x14 : bar0_wr1 / 
    uint8_t rsvd18[8];                                                      // 0x18 : rsvd_18 / rsvd_18
    FenceRngbar1End_t bar1End;                                              // 0x20 : bar1_end / 
    FenceRngbar1Start_t bar1Start;                                          // 0x24 : bar1_start / 
    FenceRngbar1Rd0_t bar1Rd0;                                              // 0x28 : bar1_rd0 / 
    uint32_t bar1Rd1InitiatorEn;                                            // 0x2C : bar1_rd1 / 
    FenceRngbar1Wr0_t bar1Wr0;                                              // 0x30 : bar1_wr0 / 
    uint32_t bar1Wr1InitiatorEn;                                            // 0x34 : bar1_wr1 / 
    uint8_t rsvd38[8];                                                      // 0x38 : rsvd_38 / rsvd_38
    FenceRngbar2End_t bar2End;                                              // 0x40 : bar2_end / 
    FenceRngbar2Start_t bar2Start;                                          // 0x44 : bar2_start / 
    FenceRngbar2Rd0_t bar2Rd0;                                              // 0x48 : bar2_rd0 / 
    uint32_t bar2Rd1InitiatorEn;                                            // 0x4C : bar2_rd1 / 
    FenceRngbar2Wr0_t bar2Wr0;                                              // 0x50 : bar2_wr0 / 
    uint32_t bar2Wr1InitiatorEn;                                            // 0x54 : bar2_wr1 / 
    uint8_t rsvd58[8];                                                      // 0x58 : rsvd_58 / rsvd_58
    FenceRngbar3End_t bar3End;                                              // 0x60 : bar3_end / 
    FenceRngbar3Start_t bar3Start;                                          // 0x64 : bar3_start / 
    FenceRngbar3Rd0_t bar3Rd0;                                              // 0x68 : bar3_rd0 / 
    uint32_t bar3Rd1InitiatorEn;                                            // 0x6C : bar3_rd1 / 
    FenceRngbar3Wr0_t bar3Wr0;                                              // 0x70 : bar3_wr0 / 
    uint32_t bar3Wr1InitiatorEn;                                            // 0x74 : bar3_wr1 / 
    uint8_t rsvd78[8];                                                      // 0x78 : rsvd_78 / rsvd_78
    FenceRngbar4End_t bar4End;                                              // 0x80 : bar4_end / 
    FenceRngbar4Start_t bar4Start;                                          // 0x84 : bar4_start / 
    FenceRngbar4Rd0_t bar4Rd0;                                              // 0x88 : bar4_rd0 / 
    uint32_t bar4Rd1InitiatorEn;                                            // 0x8C : bar4_rd1 / 
    FenceRngbar4Wr0_t bar4Wr0;                                              // 0x90 : bar4_wr0 / 
    uint32_t bar4Wr1InitiatorEn;                                            // 0x94 : bar4_wr1 / 
    uint8_t rsvd98[8];                                                      // 0x98 : rsvd_98 / rsvd_98
    FenceRngbar5End_t bar5End;                                              // 0xA0 : bar5_end / 
    FenceRngbar5Start_t bar5Start;                                          // 0xA4 : bar5_start / 
    FenceRngbar5Rd0_t bar5Rd0;                                              // 0xA8 : bar5_rd0 / 
    uint32_t bar5Rd1InitiatorEn;                                            // 0xAC : bar5_rd1 / 
    FenceRngbar5Wr0_t bar5Wr0;                                              // 0xB0 : bar5_wr0 / 
    uint32_t bar5Wr1InitiatorEn;                                            // 0xB4 : bar5_wr1 / 
    uint8_t rsvdB8[8];                                                      // 0xB8 : rsvd_b8 / rsvd_b8
    FenceRngbar6End_t bar6End;                                              // 0xC0 : bar6_end / 
    FenceRngbar6Start_t bar6Start;                                          // 0xC4 : bar6_start / 
    FenceRngbar6Rd0_t bar6Rd0;                                              // 0xC8 : bar6_rd0 / 
    uint32_t bar6Rd1InitiatorEn;                                            // 0xCC : bar6_rd1 / 
    FenceRngbar6Wr0_t bar6Wr0;                                              // 0xD0 : bar6_wr0 / 
    uint32_t bar6Wr1InitiatorEn;                                            // 0xD4 : bar6_wr1 / 
    uint8_t rsvdD8[8];                                                      // 0xD8 : rsvd_d8 / rsvd_d8
    FenceRngbar7End_t bar7End;                                              // 0xE0 : bar7_end / 
    FenceRngbar7Start_t bar7Start;                                          // 0xE4 : bar7_start / 
    FenceRngbar7Rd0_t bar7Rd0;                                              // 0xE8 : bar7_rd0 / 
    uint32_t bar7Rd1InitiatorEn;                                            // 0xEC : bar7_rd1 / 
    FenceRngbar7Wr0_t bar7Wr0;                                              // 0xF0 : bar7_wr0 / 
    uint32_t bar7Wr1InitiatorEn;                                            // 0xF4 : bar7_wr1 / 
} FenceRng_t;

COMPILE_ASSERT(offsetof(FenceRng_t,bar0End)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar0Start)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar0Rd0)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar0Rd1InitiatorEn)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar0Wr0)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar0Wr1InitiatorEn)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1End)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1Start)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1Rd0)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1Rd1InitiatorEn)==0x2C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1Wr0)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar1Wr1InitiatorEn)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2End)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2Start)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2Rd0)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2Rd1InitiatorEn)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2Wr0)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar2Wr1InitiatorEn)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3End)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3Start)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3Rd0)==0x68,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3Rd1InitiatorEn)==0x6C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3Wr0)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar3Wr1InitiatorEn)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4End)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4Start)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4Rd0)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4Rd1InitiatorEn)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4Wr0)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar4Wr1InitiatorEn)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5End)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5Start)==0xA4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5Rd0)==0xA8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5Rd1InitiatorEn)==0xAC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5Wr0)==0xB0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar5Wr1InitiatorEn)==0xB4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6End)==0xC0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6Start)==0xC4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6Rd0)==0xC8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6Rd1InitiatorEn)==0xCC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6Wr0)==0xD0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar6Wr1InitiatorEn)==0xD4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7End)==0xE0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7Start)==0xE4,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7Rd0)==0xE8,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7Rd1InitiatorEn)==0xEC,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7Wr0)==0xF0,"check register structure offset");
COMPILE_ASSERT(offsetof(FenceRng_t,bar7Wr1InitiatorEn)==0xF4,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile FenceRng_t rFenceRng; ///< 0xB020C000
