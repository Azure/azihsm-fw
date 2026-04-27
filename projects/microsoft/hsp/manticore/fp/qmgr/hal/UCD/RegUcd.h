// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @brief UCD Registers
//!
//=============================================================================

// Generated with Dullahan v2.2.6.03a6f27

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

//#include <stddef.h>
//#include <stdint.h>
//#include "SysTypes.h"
#include "List.h"
#include "RegUcdCpuPfVfNvmeController.h"
#include "RegUcdGenCmn.h"
#include "RegUcdGenCmnIbLgc2phys.h"
#include "RegUcdGenCmnObLgc2phys.h"
#include "RegUcdInbound.h"
#include "RegUcdOutbound.h"
#include "RegUcdHstPfVfNvmeController.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------

typedef struct
{
    UcdCpuPfNvmeControllerRegisters_t ucdCpuPfNvmeControllerRegisters;      // 0x0 : ucd_cpu_pf_nvme_controller_registers /
    uint8_t rsvd54[32684];  // 0x54 : rsvd_54 / rsvd_54 ///< 0x54 - 0x8000
    UcdCpuVfNvmeControllerRegisters_t ucdCpuVfNvmeControllerRegisters[64];  // 0x8000 : ucd_cpu_vf_nvme_controller_registers /
    uint8_t rsvd18000[950272];         // 0x18000 : rsvd_18000 / rsvd_18000  ///< 0x18000 - 0x100000
    UcdGenCmnRegisters_t ucdGenCmnRegisters;  // 0x100000 : ucd_gen_cmn_registers /
    uint8_t rsvd100fc4[60];  // 0x100FC4 : rsvd_100fc4 / rsvd_100fc4 ///< 0x100fc4 - 0x101000
    UcdGenCmnIbLgc2physRegisters_t ucdGenCmnIbLgc2physRegisters[264];       // 0x101000 : ucd_gen_cmn_ib_lgc2phys_registers /
    uint8_t rsvd101420[3040];           // 0x101420 : rsvd_101420 / rsvd_101420  ///< 0x101420 - 0x102000
    UcdGenCmnObLgc2physRegisters_t ucdGenCmnObLgc2physRegisters[264];       // 0x102000 : ucd_gen_cmn_ob_lgc2phys_registers /
    uint8_t rsvd102420[515040];      // 0x102420 : rsvd_102420 / rsvd_102420   ///< 0x102420 - 0x180000
    UcdInboundRegisters_t ucdInboundRegisters;   // 0x180000 : ucd_inbound_registers /
    //uint8_t rsvd7100to8000[3840]; ///< 0x7100 - 0x8000
    //uint8_t rsvd184848[229376];      // 0x184848 : rsvd_184848 / rsvd_184848   ///< 0x188000 - 0x1C0000
    uint8_t rsvd187100[233216]; ///< 0x187100 - 0x1C0000
    UcdOutboundRegisters_t ucdOutboundRegisters;    // 0x1C0000 : ucd_outbound_registers /
    //uint8_t rsvd7100to8000[3840]; ///< 0x7100 - 0x8000
    //uint8_t rsvd1c4628[229376];      // 0x1C4628 : rsvd_1c4628 / rsvd_1c4628  //uint8_t rsvd1c4628[244184]; ///< 0x1C8000 - 0x200000
    uint8_t rsvd1c7100[233216]; ///< 0x1C7100 - 0x200000
    UcdHstPfNvmeControllerRegisters_t ucdHstPfNvmeControllerRegisters;      // 0x200000 : ucd_hst_pf_nvme_controller_registers /
    uint8_t rsvd200054[1048492];      // 0x200054 : rsvd_200054 / rsvd_200054 ///< 0x200054 - 0x300000
    UcdHstVfNvmeControllerRegisters_t ucdHstVfNvmeControllerRegisters[64];  // 0x300000 : ucd_hst_vf_nvme_controller_registers /
} Ucd_t;

#if 1
static_assert(TYPE_OFFSET(Ucd_t, ucdCpuPfNvmeControllerRegisters) == 0x0, "check register structure offset 0x0");
static_assert(TYPE_OFFSET(Ucd_t, ucdCpuVfNvmeControllerRegisters) == 0x8000, "check register structure offset 0x8000");
static_assert((TYPE_OFFSET(Ucd_t, ucdGenCmnRegisters) == 0x100000), "check register structure offset 0x100000");
static_assert(TYPE_OFFSET(Ucd_t, ucdGenCmnIbLgc2physRegisters) == 0x101000, "check register structure offset 0x101000");
static_assert(TYPE_OFFSET(Ucd_t, ucdGenCmnObLgc2physRegisters) == 0x102000, "check register structure offset 0x102000");
static_assert(TYPE_OFFSET(Ucd_t, ucdInboundRegisters) == 0x180000, "check register structure offset 0x180000");
static_assert(TYPE_OFFSET(Ucd_t, ucdOutboundRegisters) == 0x1C0000, "check register structure offset 0x1C0000");
static_assert(TYPE_OFFSET(Ucd_t, ucdHstPfNvmeControllerRegisters) == 0x200000, "check register structure offset 0x200000");
static_assert(TYPE_OFFSET(Ucd_t, ucdHstVfNvmeControllerRegisters) == 0x300000, "check register structure offset 0x300000");
#else
COMPILE_ASSERT(offsetof(Ucd_t, ucdCpuPfNvmeControllerRegisters) == 0x0, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdCpuVfNvmeControllerRegisters) == 0x8000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdGenCmnRegisters) == 0x100000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdGenCmnIbLgc2physRegisters) == 0x101000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdGenCmnObLgc2physRegisters) == 0x102000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdInboundRegisters) == 0x180000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdOutboundRegisters) == 0x1C0000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdHstPfNvmeControllerRegisters) == 0x200000, "check register structure offset");
COMPILE_ASSERT(offsetof(Ucd_t, ucdHstVfNvmeControllerRegisters) == 0x300000, "check register structure offset");
#endif

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
