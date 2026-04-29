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
//! @brief INTC_IPC Registers
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


typedef struct
{
    uint32_t intPendClr0;                                                   // 0x0 : INT_PEND_CLR_0 / 
    uint32_t intPendClr1;                                                   // 0x4 : INT_PEND_CLR_1 / 
    uint32_t intPendClr2;                                                   // 0x8 : INT_PEND_CLR_2 / 
    uint32_t intPendClr3;                                                   // 0xC : INT_PEND_CLR_3 / 
    uint32_t intPendClr4;                                                   // 0x10 : INT_PEND_CLR_4 / 
    uint32_t intPendClr5;                                                   // 0x14 : INT_PEND_CLR_5 / 
    uint32_t intPendSet0;                                                   // 0x18 : INT_PEND_SET_0 / 
    uint32_t intPendSet1;                                                   // 0x1C : INT_PEND_SET_1 / 
    uint32_t intPendSet2;                                                   // 0x20 : INT_PEND_SET_2 / 
    uint32_t intPendSet3;                                                   // 0x24 : INT_PEND_SET_3 / 
    uint32_t intPendSet4;                                                   // 0x28 : INT_PEND_SET_4 / 
    uint32_t intPendSet5;                                                   // 0x2C : INT_PEND_SET_5 / 
    uint32_t intMaskClr0;                                                   // 0x30 : INT_MASK_CLR_0 / 
    uint32_t intMaskClr1;                                                   // 0x34 : INT_MASK_CLR_1 / 
    uint32_t intMaskClr2;                                                   // 0x38 : INT_MASK_CLR_2 / 
    uint32_t intMaskClr3;                                                   // 0x3C : INT_MASK_CLR_3 / 
    uint32_t intMaskClr4;                                                   // 0x40 : INT_MASK_CLR_4 / 
    uint32_t intMaskClr5;                                                   // 0x44 : INT_MASK_CLR_5 / 
    uint32_t intMaskSet0;                                                   // 0x48 : INT_MASK_SET_0 / 
    uint32_t intMaskSet1;                                                   // 0x4C : INT_MASK_SET_1 / 
    uint32_t intMaskSet2;                                                   // 0x50 : INT_MASK_SET_2 / 
    uint32_t intMaskSet3;                                                   // 0x54 : INT_MASK_SET_3 / 
    uint32_t intMaskSet4;                                                   // 0x58 : INT_MASK_SET_4 / 
    uint32_t intMaskSet5;                                                   // 0x5C : INT_MASK_SET_5 / 
    uint32_t intEnabClr0;                                                   // 0x60 : INT_ENAB_CLR_0 / 
    uint32_t intEnabClr1;                                                   // 0x64 : INT_ENAB_CLR_1 / 
    uint32_t intEnabClr2;                                                   // 0x68 : INT_ENAB_CLR_2 / 
    uint32_t intEnabClr3;                                                   // 0x6C : INT_ENAB_CLR_3 / 
    uint32_t intEnabClr4;                                                   // 0x70 : INT_ENAB_CLR_4 / 
    uint32_t intEnabClr5;                                                   // 0x74 : INT_ENAB_CLR_5 / 
    uint32_t intEnabSet0;                                                   // 0x78 : INT_ENAB_SET_0 / 
    uint32_t intEnabSet1;                                                   // 0x7C : INT_ENAB_SET_1 / 
    uint32_t intEnabSet2;                                                   // 0x80 : INT_ENAB_SET_2 / 
    uint32_t intEnabSet3;                                                   // 0x84 : INT_ENAB_SET_3 / 
    uint32_t intEnabSet4;                                                   // 0x88 : INT_ENAB_SET_4 / 
    uint32_t intEnabSet5;                                                   // 0x8C : INT_ENAB_SET_5 / 
    uint8_t rsvd90[128];                                                    // 0x90 : rsvd_90 / rsvd_90
    uint32_t desc00DescReg00;                                               // 0x110 : DESC_REG_00 / 
    uint32_t desc01DescReg01;                                               // 0x114 : DESC_REG_01 / 
    uint32_t desc02DescReg02;                                               // 0x118 : DESC_REG_02 / 
    uint32_t desc03DescReg03;                                               // 0x11C : DESC_REG_03 / 
    uint32_t desc04DescReg04;                                               // 0x120 : DESC_REG_04 / 
    uint32_t desc05DescReg05;                                               // 0x124 : DESC_REG_05 / 
    uint32_t desc06DescReg06;                                               // 0x128 : DESC_REG_06 / 
    uint32_t desc07DescReg07;                                               // 0x12C : DESC_REG_07 / 
    uint32_t desc08DescReg08;                                               // 0x130 : DESC_REG_08 / 
    uint32_t desc09DescReg09;                                               // 0x134 : DESC_REG_09 / 
    uint32_t desc10DescReg10;                                               // 0x138 : DESC_REG_10 / 
    uint32_t desc11DescReg11;                                               // 0x13C : DESC_REG_11 / 
    uint32_t desc12DescReg12;                                               // 0x140 : DESC_REG_12 / 
    uint32_t desc13DescReg13;                                               // 0x144 : DESC_REG_13 / 
    uint32_t desc14DescReg14;                                               // 0x148 : DESC_REG_14 / 
    uint32_t desc15DescReg15;                                               // 0x14C : DESC_REG_15 / 
    uint32_t desc16DescReg16;                                               // 0x150 : DESC_REG_16 / 
    uint32_t desc17DescReg17;                                               // 0x154 : DESC_REG_17 / 
    uint32_t desc18DescReg18;                                               // 0x158 : DESC_REG_18 / 
    uint32_t desc19DescReg19;                                               // 0x15C : DESC_REG_19 / 
    uint32_t desc20DescReg20;                                               // 0x160 : DESC_REG_20 / 
    uint32_t desc21DescReg21;                                               // 0x164 : DESC_REG_21 / 
    uint32_t desc22DescReg22;                                               // 0x168 : DESC_REG_22 / 
    uint32_t desc23DescReg23;                                               // 0x16C : DESC_REG_23 / 
    uint32_t desc24DescReg24;                                               // 0x170 : DESC_REG_24 / 
    uint32_t desc25DescReg25;                                               // 0x174 : DESC_REG_25 / 
    uint32_t desc26DescReg26;                                               // 0x178 : DESC_REG_26 / 
    uint32_t desc27DescReg27;                                               // 0x17C : DESC_REG_27 / 
    uint32_t desc28DescReg28;                                               // 0x180 : DESC_REG_28 / 
    uint32_t desc29DescReg29;                                               // 0x184 : DESC_REG_29 / 
    uint32_t desc30DescReg30;                                               // 0x188 : DESC_REG_30 / 
    uint32_t desc31DescReg31;                                               // 0x18C : DESC_REG_31 / 
} IntcIpc_t;

COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr0)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr1)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr2)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr3)==0xC,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr4)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendClr5)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet0)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet1)==0x1C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet2)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet3)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet4)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intPendSet5)==0x2C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr0)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr1)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr2)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr3)==0x3C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr4)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskClr5)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet0)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet1)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet2)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet3)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet4)==0x58,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intMaskSet5)==0x5C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr0)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr1)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr2)==0x68,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr3)==0x6C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr4)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabClr5)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet0)==0x78,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet1)==0x7C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet2)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet3)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet4)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,intEnabSet5)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc00DescReg00)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc01DescReg01)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc02DescReg02)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc03DescReg03)==0x11C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc04DescReg04)==0x120,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc05DescReg05)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc06DescReg06)==0x128,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc07DescReg07)==0x12C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc08DescReg08)==0x130,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc09DescReg09)==0x134,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc10DescReg10)==0x138,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc11DescReg11)==0x13C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc12DescReg12)==0x140,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc13DescReg13)==0x144,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc14DescReg14)==0x148,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc15DescReg15)==0x14C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc16DescReg16)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc17DescReg17)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc18DescReg18)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc19DescReg19)==0x15C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc20DescReg20)==0x160,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc21DescReg21)==0x164,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc22DescReg22)==0x168,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc23DescReg23)==0x16C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc24DescReg24)==0x170,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc25DescReg25)==0x174,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc26DescReg26)==0x178,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc27DescReg27)==0x17C,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc28DescReg28)==0x180,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc29DescReg29)==0x184,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc30DescReg30)==0x188,"check register structure offset");
COMPILE_ASSERT(offsetof(IntcIpc_t,desc31DescReg31)==0x18C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile IntcIpc_t rIntcIpc; ///< 0xB0006000
