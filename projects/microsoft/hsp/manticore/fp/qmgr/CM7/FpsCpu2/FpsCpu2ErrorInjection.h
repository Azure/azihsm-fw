// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2ErrorInjection.h
//! @brief  FpsCpu2 Error Injection function
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "SysTypes.h"
#include "platform.h"
#if defined (SUPPORT_ERROR_INJECTION) || defined (SUPPORT_MSGERROR_INJECTION)
#include "MessageHandler.h"
#include "FpsCpu2.h"
#endif

//-----------------------------------------------------------------------------
//  Class Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Interface Function Declarations
//-----------------------------------------------------------------------------

#ifdef SUPPORT_ERROR_INJECTION
/**
 *  @brief    Cpu2 fill error inejction info in CQE
 *
 *  @param    LionFPCmdMetaData_t* pFpCmd - Host command pointer
 *
 *  @return   None
 */
void FpsCpu2FillErrInjectInfo(LionFPCmdMetaData_t* pFpCmd);
/**
 *  @brief    Cpu2 error inject and and modify error status
 *
 *  @param    uint32_t* pCdmaErrorStatus0 - CDMA command slot error status in register 0
 *            uint32_t* pCdmaErrorStatus1 - CDMA command slot error status in register 1
 *            LionFPCmdMetaData_t* pFpCmd - Host command pointer
 *
 *  @return   None
 */
void FpsCpu2ErrorInjection_InjectErr(uint32_t* pCdmaErrorStatus0, uint32_t* pCdmaErrorStatus1, LionFPCmdMetaData_t* pFpCmd);
#elif defined (SUPPORT_MSGERROR_INJECTION)
/**
 *  @brief    Cpu2 error inject and set error type
 *
 *  @param    uint64_t errInjectBitmap - error inject bitmap
 *            uint8_t* pCdmaErrSts - pointer of cdma error status
 *            uint16_t dflIdx - DFL buffer index
 *            uint8_t errIdx - error index of error inject table
 *
 *  @return   None
 */
void Cpu2CdmaErrorInjectionSetErrorType(uint64_t errInjectBitmap, uint8_t* pCdmaErrSts, uint16_t dflIdx, uint8_t errIdx);

/**
 *  @brief    Cpu2 scan error inject table
 *
 *  @param    uint64_t errInjectBitmap - error inject bitmap
 *            uint16_t cmdId - host command id
 *            uint8_t vfId - VF id
 *            uint8_t ibPhyQId - inbound physical queue id
 *            uint16_t dflIdx - DFL buffer index
 *
 *  @return   error inject index of error inject table
 */
uint8_t Cpu2ScanErrInjectTable(uint64_t errInjectBitmap, uint16_t dflIdx);

/**
 *  @brief    Cpu2 handle error error inject
 *
 *  @param    uint64_t errInjectBitmap - error inject bitmap
 *            uint8_t* pCdmaErrSts - pointer of cdma error status
 *            uint16_t dflIdx - DFL buffer index
 *
 *  @return   None
 */
void Cpu2HandleErrInject(uint64_t errInjectBitmap, uint8_t* pCdmaErrSts, uint16_t dflIdx);

/**
 *  @brief    Cpu2 check error inject error type and error inject times are regular
 *
 *  @param    uint8_t errorType - error type
 *            uint8_t reErrInjectTimes - re error inject times
 *
 *  @return   True : error inject error type and error inject times are regular
 *            False : error inject error type and error inject times are not regular
 */
bool Cpu2ChkErrInjectErrType(uint8_t errorType, uint8_t reErrInjectTimes);

/**
 *  @brief    Cpu2 check VF and inbound phyicsal queue are regular
 *
 *  @param    CP2FPMsgDataMsgErrorInjection_t* pErrInjectData - error inject data from message
 *            uint8_t* pIbQ2ObQ - inbound queue and outbound queue mapping pointer
 *            QueueBlockInfo_t* pQueueBlockInfoBase - queue block information pointer
 *
 *  @return   True : VF and inbound phyicsal queue are regular
 *            False : VF and inbound phyicsal queue are not regular
 */
bool Cpu2ChkVFQ(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, uint8_t* pIbQ2ObQ, QueueBlockInfo_t* pQueueBlockInfoBase);

/**
 *  @brief    Cpu2 check error inject condition
 *
 *  @param    CP2FPMsgDataMsgErrorInjection_t* pErrInjectData - error inject data from message
 *            uint8_t* pIbQ2ObQ - inbound queue and outbound queue mapping pointer
 *            uint64_t* pVFEnBitmap - VF enable bit map pointer
 *            QueueBlockInfo_t* pQueueBlockInfoBase - queue block information pointer
 *
 *  @return   message status (CP2FPMsgSts)
 */
uint8_t Cpu2ChkErrInjectCondition(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, uint8_t* pIbQ2ObQ, uint64_t* pVFEnBitmap, QueueBlockInfo_t* pQueueBlockInfoBase);

/**
 *  @brief    Cpu2 add error inject to error inject table
 *
 *  @param    CP2FPMsgDataMsgErrorInjection_t* pErrInjectData - error inject data from message
 *            uint8_t* pIbQ2ObQ - inbound queue and outbound queue mapping pointer
 *            uint64_t* pVFEnBitmap - VF enable bit map pointer
 *            QueueBlockInfo_t* pQueueBlockInfoBase - queue block information pointer
 *
 *  @return   message status (CP2FPMsgSts)
 */
uint8_t Cpu2AddErrInject(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, uint8_t* pIbQ2ObQ, uint64_t* pVFEnBitmap, QueueBlockInfo_t* pQueueBlockInfoBase);

//-----------------------------------------------------------------------------
//  Inline Member Function Definitions
//-----------------------------------------------------------------------------

#endif
