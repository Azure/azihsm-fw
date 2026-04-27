// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1ErrorInjection.h
//! @brief  FpsCpu1 Error Injection function
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "SysTypes.h"
#include "platform.h"
#include "FpsCpu1.h"

#ifdef SUPPORT_ERROR_INJECTION

void Cpu1InsertErr(uint8_t errorType, volatile CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd);
void Cpu1HandleErrorInject(AesXtsCmd_t* pAesXtsCmd, volatile CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd);


#elif defined (SUPPORT_MSGERROR_INJECTION)

//-----------------------------------------------------------------------------
//  Class Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Interface Function Declarations
//-----------------------------------------------------------------------------

uint8_t Cpu1ScanErrInjectTable(uint64_t errInjectBitmap, CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection, \
                               uint16_t cmdId, uint8_t vfId, uint8_t ibPhyQId, uint16_t dflIdx);
void Cpu1InsertErr(CP2FPMsgDataMsgErrorInjection_t* pErrorInjection, CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd);

//-----------------------------------------------------------------------------
//  Inline Member Function Definitions
//-----------------------------------------------------------------------------

#endif
