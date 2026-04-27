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
//! @brief IDEFUSE Registers
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
        uint32_t START                       :1;      ///<BIT [0] START
        uint32_t WRITE                       :1;      ///<BIT [1] WRITE
        uint32_t READ                        :1;      ///<BIT [2] READ
        uint32_t RSVD_2                      :1;      ///<BIT [3] RSVD_2
        uint32_t READ_BACK                   :1;      ///<BIT [4] READ_BACK
        uint32_t RSVD_1                      :1;      ///<BIT [5] RSVD_1
        uint32_t PDWN                        :1;      ///<BIT [6] PDWN
        uint32_t MARGIN_READ                 :1;      ///<BIT [7] MARGIN_READ
        uint32_t TEST_EN                     :1;      ///<BIT [8] TEST_EN
        uint32_t TEST_MODE                   :2;      ///<BIT [10:9] TEST_MODE
        uint32_t RSVD                        :21;     ///<BIT [31:11] RSVD_0
    } b;
} IdefuseCtrl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DONE                        :1;      ///<BIT [0] DONE
        uint32_t RSVD                        :31;     ///<BIT [31:1] RSVD_0
    } b;
} IdefuseInt_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DONE_MASK                   :1;      ///<BIT [0] DONE_MASK
        uint32_t RSVD                        :31;     ///<BIT [31:1] RSVD_0
    } b;
} IdefuseIntMsk_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t ROW_ADDR                    :10;     ///<BIT [9:0] ROW_ADDR
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} IdefuseRowAddr_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EN                          :1;      ///<BIT [0] ENABLE
        uint32_t WRITE_TIME                  :15;     ///<BIT [15:1] WRITE_TIME
        uint32_t RSVD                        :16;     ///<BIT [31:16] RSVD_0
    } b;
} IdefuseSclkCtrl_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EFUSE_CSB                   :1;      ///<BIT [0] EFUSE_CSB
        uint32_t EFUSE_STROBE                :1;      ///<BIT [1] EFUSE_STROBE
        uint32_t EFUSE_LOAD                  :1;      ///<BIT [2] EFUSE_LOAD
        uint32_t EFUSE_PGENB                 :1;      ///<BIT [3] EFUSE_PGENB
        uint32_t EFUSE_PS                    :1;      ///<BIT [4] EFUSE_PS
        uint32_t EFUSE_PD                    :1;      ///<BIT [5] EFUSE_PD
        uint32_t EFUSE_MR                    :1;      ///<BIT [6] EFUSE_MR
        uint32_t EFUSE_TRCS                  :1;      ///<BIT [7] EFUSE_TRCS
        uint32_t EFUSE_AT                    :2;      ///<BIT [9:8] EFUSE_AT
        uint32_t RSVD_1                      :6;      ///<BIT [15:10] RSVD_1
        uint32_t EFUSE_A                     :10;     ///<BIT [25:16] EFUSE_A
        uint32_t RSVD                        :5;      ///<BIT [30:26] RSVD_0
        uint32_t CPU_DIR_CTRL_EN             :1;      ///<BIT [31] CPU_DIR_CTRL_EN
    } b;
} IdefuseCpuDirCtrl_t;

/// @brief 0x114
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t REVID                       :4;      ///<BIT [3:0] REVID
        uint32_t RSVD                        :28;     ///<BIT [31:4] RSVD_0
    } b;
} ChipRevId_t;

/// @brief 0x118
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_TRIP_THR               :10;     ///<BIT [9:0] TSEN_TRIP_THR
        uint32_t RSVD_1                      :6;      ///<BIT [15:10] RSVD_1
        uint32_t TSEN_TRIP_THR_VLD           :1;      ///<BIT [16] TSEN_TRIP_THR_VLD
        uint32_t RSVD                        :15;     ///<BIT [31:17] RSVD_0
    } b;
} TsenTripTemp_t;

/// @brief 0x11C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t EFUSE_LOCK                  :1;      ///<BIT [0] EFUSE_LOCK
        uint32_t RSVD                        :31;     ///<BIT [31:1] RSVD_0
    } b;
} EfuseStatus_t;

typedef struct
{
    IdefuseCtrl_t idefuseCtrl;                                              // 0x0 : IDEFUSE_CTRL / 
    IdefuseInt_t idefuseInt;                                                // 0x4 : IDEFUSE_INT / 
    IdefuseIntMsk_t idefuseIntMsk;                                          // 0x8 : IDEFUSE_INT_MSK / 
    uint8_t rsvdC[4];                                                       // 0xC : rsvd_c / rsvd_c
    IdefuseRowAddr_t idefuseRowAddr;                                        // 0x10 : IDEFUSE_ROW_ADDR / 
    uint32_t idefuseDataData;                                               // 0x14 : IDEFUSE_DATA / 
    uint8_t rsvd18[32];                                                     // 0x18 : rsvd_18 / rsvd_18
    IdefuseSclkCtrl_t idefuseSclkCtrl;                                      // 0x38 : IDEFUSE_SCLK_CTRL / 
    uint8_t rsvd3c[4];                                                      // 0x3C : rsvd_3c / rsvd_3c
    IdefuseCpuDirCtrl_t idefuseCpuDirCtrl;                                  // 0x40 : IDEFUSE_CPU_DIR_CTRL / 
    uint32_t idefuseCpuDirCtrlDataEfuseData;                                // 0x44 : IDEFUSE_CPU_DIR_CTRL_DATA / 
    uint8_t rsvd48[184];                                                    // 0x48 : rsvd_48 / rsvd_48
    uint32_t chipUid0Uid0;                                                  // 0x100 : CHIP_UID_0 / 
    uint32_t chipUid1Uid1;                                                  // 0x104 : CHIP_UID_1 / 
    uint32_t chipUid2Uid2;                                                  // 0x108 : CHIP_UID_2 / 
    uint32_t chipSvc0Svc0;                                                  // 0x10C : CHIP_SVC_0 / 
    uint32_t chipSvc1Svc1;                                                  // 0x110 : CHIP_SVC_1 / 
    ChipRevId_t chipRevId;                                                  // 0x114 : CHIP_REV_ID / 
    TsenTripTemp_t tsenTripTemp;                                            // 0x118 : TSEN_TRIP_TEMP / 
    EfuseStatus_t efuseStatus;                                              // 0x11C : EFUSE_STATUS / 
} Idefuse_t;

COMPILE_ASSERT(offsetof(Idefuse_t,idefuseCtrl)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseInt)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseIntMsk)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseRowAddr)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseDataData)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseSclkCtrl)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseCpuDirCtrl)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,idefuseCpuDirCtrlDataEfuseData)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipUid0Uid0)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipUid1Uid1)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipUid2Uid2)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipSvc0Svc0)==0x10C,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipSvc1Svc1)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,chipRevId)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,tsenTripTemp)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(Idefuse_t,efuseStatus)==0x11C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Idefuse_t rIdefuse; ///< 0xB0002000
