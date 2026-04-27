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
//! @brief PCIE_TDISP_SEC Registers
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
        uint32_t LOCK_TDISP_CFG              :1;      ///<BIT [0] LOCK_TDISP_CFG
        uint32_t LOCK_IDE_CFG                :1;      ///<BIT [1] LOCK_IDE_CFG
        uint32_t LOCK_IDE_KEY                :1;      ///<BIT [2] LOCK_IDE_KEY
        uint32_t RSVD_3_31                   :29;     ///<BIT [31:3] rsvd_3_31
    } b;
} TdispCtrlS_t;

/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TDISP_ST                    :4;      ///<BIT [3:0] TDISP_ST
        uint32_t RSVD_4_31                   :28;     ///<BIT [31:4] rsvd_4_31
    } b;
} TdispSt_t;

/// @brief 0x10
typedef struct
{
    TdispSt_t tdispSt;                    //TDISP_ST
} TdispStGrp_t;

typedef struct
{
    TdispCtrlS_t tdispCtrlS;                                                // 0x0 : TDISP_CTRL_S / 
    uint8_t rsvd4[12];                                                      // 0x4 : rsvd_4 / rsvd_4
    TdispStGrp_t tdispStGrp[65];                                            // 0x10 : TDISP_ST_GRP / 
} PcieTdispSec_t;

COMPILE_ASSERT(offsetof(PcieTdispSec_t,tdispCtrlS)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(PcieTdispSec_t,tdispStGrp)==0x10,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile PcieTdispSec_t rPcieTdispSec; ///< 0xB01B0000
