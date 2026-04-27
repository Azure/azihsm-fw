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
//! @brief 39 register declaration
//!
//=============================================================================

// Generated with Dullahan v2.4.3.

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "RegGdma.h"
#include "RegCdma.h"
#include "RegUcd.h"
#include "RegMsix.h"
#include "RegFps.h"
#include "RegIdefuse.h"
#include "RegPorHspOnly.h"
#include "RegPor.h"
#include "RegTcon.h"
#include "RegIntcIpc.h"
#include "RegApb.h"
#include "RegTsen.h"
#include "RegUart.h"
#include "RegSpis0.h"
#include "RegSpis1.h"
#include "RegGsram.h"
#include "RegComphy0Soc.h"
#include "RegComphy1Soc.h"
#include "RegPcieTop.h"
#include "RegPcieEp.h"
#include "RegPcieTdispCfg.h"
#include "RegPcieTdispSec.h"
#include "RegPcieAssist.h"
#include "RegPcieDoe.h"
#include "RegPcieIde.h"
#include "RegPcieIdeAes.h"
#include "RegDualCpM7.h"
#include "RegFenceApb.h"
#include "RegFenceDualCp.h"
#include "RegFenceNqm.h"
#include "RegFenceBcp.h"
#include "RegFenceGdma.h"
#include "RegFenceGsram.h"
#include "RegFencePcie.h"
#include "RegFenceUpkab0.h"
#include "RegFenceUpkab1.h"
#include "RegFenceHssha.h"
#include "RegFenceAes.h"
#include "RegFenceRng.h"

//-----------------------------------------------------------------------------
//  Macros definitions
//-----------------------------------------------------------------------------

#define __STR_LINK(x)               # x
#define __SECTION(_qual)            ".bss.ARM.__at_"__STR_LINK(_qual)
#define ATTR_AT(x)                  __attribute__((section(__SECTION(x))))

//-----------------------------------------------------------------------------
//  Register declaration: Private or Public
//-----------------------------------------------------------------------------

/// @brief SoC register declaration

volatile Gdma_t                                  rGdma                                   ATTR_AT(0xA0000000);
volatile Cdma_t                                  rCdma                                   ATTR_AT(0xA0C00000);
volatile Ucd_t                                   rUcd                                    ATTR_AT(0xA1100000);
volatile Msix_t                                  rMsix                                   ATTR_AT(0xA1800000);
volatile Fps_t                                   rFps                                    ATTR_AT(0xA1E00000);
volatile Idefuse_t                               rIdefuse                                ATTR_AT(0xB0002000);
volatile PorHspOnly_t                            rPorHspOnly                             ATTR_AT(0xB0003000);
volatile Por_t                                   rPor                                    ATTR_AT(0xB0004000);
volatile Tcon_t                                  rTcon                                   ATTR_AT(0xB0005000);
volatile IntcIpc_t                               rIntcIpc                                ATTR_AT(0xB0006000);
volatile Apb_t                                   rApb                                    ATTR_AT(0xB0007000);
volatile Tsen_t                                  rTsen                                   ATTR_AT(0xB0008000);
volatile Uart_t                                  rUart                                   ATTR_AT(0xB0009000);
volatile Spis0_t                                 rSpis0                                  ATTR_AT(0xB000A000);
volatile Spis1_t                                 rSpis1                                  ATTR_AT(0xB000B000);
volatile Gsram_t                                 rGsram                                  ATTR_AT(0xB000C000);
volatile Comphy0Soc_t                            rComphy0Soc                             ATTR_AT(0xB0100000);
volatile Comphy1Soc_t                            rComphy1Soc                             ATTR_AT(0xB0140000);
volatile PcieTop_t                               rPcieTop                                ATTR_AT(0xB0160000);
volatile PcieEp_t                                rPcieEp                                 ATTR_AT(0xB0180000);
volatile PcieTdispCfg_t                          rPcieTdispCfg                           ATTR_AT(0xB01A0000);
volatile PcieTdispSec_t                          rPcieTdispSec                           ATTR_AT(0xB01B0000);
volatile PcieAssist_t                            rPcieAssist                             ATTR_AT(0xB01C0000);
volatile PcieDoe_t                               rPcieDoe                                ATTR_AT(0xB01D0000);
volatile PcieIde_t                               rPcieIde                                ATTR_AT(0xB01E0000);
volatile PcieIdeAes_t                            rPcieIdeAes                             ATTR_AT(0xB01F0000);
volatile DualCpM7_t                              rDualCpM7                               ATTR_AT(0xB0200000);
volatile FenceApb_t                              rFenceApb                               ATTR_AT(0xB0201000);
volatile FenceDualCp_t                           rFenceDualCp                            ATTR_AT(0xB0202000);
volatile FenceNqm_t                              rFenceNqm                               ATTR_AT(0xB0203000);
volatile FenceBcp_t                              rFenceBcp                               ATTR_AT(0xB0204000);
volatile FenceGdma_t                             rFenceGdma                              ATTR_AT(0xB0205000);
volatile FenceGsram_t                            rFenceGsram                             ATTR_AT(0xB0206000);
volatile FencePcie_t                             rFencePcie                              ATTR_AT(0xB0207000);
volatile FenceUpkab0_t                           rFenceUpkab0                            ATTR_AT(0xB0208000);
volatile FenceUpkab1_t                           rFenceUpkab1                            ATTR_AT(0xB0209000);
volatile FenceHssha_t                            rFenceHssha                             ATTR_AT(0xB020A000);
volatile FenceAes_t                              rFenceAes                               ATTR_AT(0xB020B000);
volatile FenceRng_t                              rFenceRng                               ATTR_AT(0xB020C000);
