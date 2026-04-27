// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2ErrorInjection.cpp
//! @brief  FpsCpu2 Error Injection
//!
//=============================================================================


//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2ErrorInjection.h"
#if defined (SUPPORT_ERROR_INJECTION) || defined (SUPPORT_MSGERROR_INJECTION)
extern fpsCpu2 gFpsCpu2;
#endif

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

#ifdef SUPPORT_ERROR_INJECTION
void FpsCpu2FillErrInjectInfo(LionFPCmdMetaData_t* pFpCmd)
{
    AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(&pFpCmd->meta.AesXtsCmd);
    if (pAesXtsCmd->Signature == MSFT_SIGNATURE)
    {
        if (pAesXtsCmd->ErrorType0 != cNoErr)
        {
            pFpCmd->cqe.ErrTypeNum = pAesXtsCmd->ErrorType0;
            pFpCmd->cqe.ErrInjectionTimes = 0;
            pAesXtsCmd->ErrorType0 = cNoErr;
        }
        else
        {
            pFpCmd->cqe.ErrTypeNum = pAesXtsCmd->ErrorType1;
            pFpCmd->cqe.ErrInjectionTimes = 1;
            pAesXtsCmd->ErrorType1 = cNoErr;
        }
    } // else do nothing

}

void FpsCpu2ErrorInjection_InjectErr(uint32_t* pCdmaErrorStatus0, uint32_t* pCdmaErrorStatus1, LionFPCmdMetaData_t* pFpCmd)
{
    AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(&pFpCmd->meta.AesXtsCmd);
    uint8_t errorType = cNoErr;
    // uint32_t cdmaErrorStatus0 = readl(pCdmaErrorStatus0);
    // uint32_t cdmaErrorStatus1 = readl(pCdmaErrorStatus1);
    uint32_t cdmaErrorStatus0 = 0;
    uint32_t cdmaErrorStatus1 = 0;
    if (pAesXtsCmd->ErrorType0 != cNoErr)
    {
        errorType = pAesXtsCmd->ErrorType0;
    }
    else if (pAesXtsCmd->ErrorType1 != cNoErr)
    {
        errorType = pAesXtsCmd->ErrorType1;
    }
    else
    {
        // else do nothing
    }

    switch (errorType)
    {
        case cDescmDestAxiRdErr:
        {
            cdmaErrorStatus0 = DESCM_DEST_AXI_RD_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cDescmSrcAxiRdErr:
        {
            cdmaErrorStatus0 = DESCM_SRC_AXI_RD_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmdeUnexpectedCmdPhaseErr:
        {
            cdmaErrorStatus0 = CMDE_UNEXPECTED_CMD_PHASE_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmdeInvalidOpcodeErr:
        {
            cdmaErrorStatus0 = CDMA_INVALID_OPCODE_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmdeAxiRdErr:
        {
            cdmaErrorStatus0 = CMDE_AXI_RD_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmpleAxiWrErr:
        {
            cdmaErrorStatus1 = CMPLE_AXI_WR_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmdTransferLengthUnderrunErr:
        {
            cdmaErrorStatus1 = CMD_TRANSFER_LENGTH_UNDERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCmdTransferLengthOverrunErr:
        {
            cdmaErrorStatus1 = CMD_TRANSFER_LENGTH_OVERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cDoeBufferRdParityErr:
        {
            cdmaErrorStatus1 = DOE_BUFFER_RD_PARITY_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCryptoeTextOutReadErr:
        {
            cdmaErrorStatus1 = CRYPTOE_TEXT_OUT_READ_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cCryptoeRedundancyMismatchErr:
        {
            cdmaErrorStatus1 = CRYPTOE_REDUNDANCY_MISMATCH_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cDbmBufferRdParityErr:
        {
            cdmaErrorStatus1 = DBM_BUFFER_RD_PARITY_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cDbmAxiRdErr:
        {
            cdmaErrorStatus1 = DBM_AXI_RD_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cDoeOverrunErr:
        {
            cdmaErrorStatus1 = DOE_OVERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cDoeUnderrunErr:
        {
            cdmaErrorStatus1 = DOE_UNDERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cDoeAxiWrErr:
        {
            cdmaErrorStatus1 = DOE_AXI_WR_ERR;
            pFpCmd->sqe.SrcDataPtr[0].Lo = pFpCmd->sqe.SrcDataPtr[0].Lo - 1;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cCryptoeFatalErr:
        {
            cdmaErrorStatus1 = CRYPTOE_FATAL_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cCryptoeKeyVaultMemRdErr:
        {
            cdmaErrorStatus1 = CRYPTOE_KEY_VAULT_MEM_RD_ERR;
            pFpCmd->sqe.SrcDataPtr[0].Lo = pFpCmd->sqe.SrcDataPtr[0].Lo - 1;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cDfeOverrunErr:
        {
            cdmaErrorStatus1 = DFE_OVERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        case cDfeUnderrrunErr:
        {
            cdmaErrorStatus1 = DFE_UNDERRUN_ERR;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        break;
        #ifdef LIONMS_B0
        case cCommandSlotErrSts1:
        {
            cdmaErrorStatus1 = COMMAND_SLOT_ERROR_STATUS;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        #endif
        default:
            break;
    }
    if ((cdmaErrorStatus0 & (NON_FATAL_RETRY_MASK_REG_0)) || (cdmaErrorStatus1 & (NON_FATAL_RETRY_MASK_REG_1))) // retry case
    {
        if ((errorType == pAesXtsCmd->ErrorType0) && (pAesXtsCmd->ErrorType1 != cNoErr)) // will execute second err injection
        {
            pAesXtsCmd->ErrorType0 = cNoErr; // clear errtype 0
        }// else do nothing
    }// else do nothing
    writel(cdmaErrorStatus0, pCdmaErrorStatus0);
    writel(cdmaErrorStatus1, pCdmaErrorStatus1);
}
#elif defined (SUPPORT_MSGERROR_INJECTION)
void Cpu2CdmaErrorInjectionSetErrorType(uint64_t errInjectBitmap, uint8_t* pCdmaErrSts, uint16_t dflIdx, uint8_t errIdx)
{
    CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection = &(gFpsCpu2.pMsgErrorInjection[errIdx]);
    uint64_t* pErrInjectBitmap = gFpsCpu2.pErrInjectBitmap;

    switch (pMsgErrorInjection->errorType)
    {
        case cMsgErrInjectCdmaNonFatalErr:
        {
            *pCdmaErrSts = cCETinyStsNonFatalErr;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountNonFatalErr++;
            #endif
        }
        break;
        case cMsgErrInjectCdmaPoorSGLErr:
        {
            *pCdmaErrSts = cCETinyStsPoorSGLErr;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcInjecCountPoorSgl++;
            #endif
        }
        break;
        case cMsgErrInjectCdmaFatalErr:
        {
            *pCdmaErrSts = cCETinyStsFatalErr;
            #ifdef SUPPORT_TELEMETRY
            gFpsCpu2.TcIInjecCountFatalErr++;
            #endif
        }
        default:
            DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Cpu2CdmaErrorInjectionSetErrorType: invalid error type:0x%X\n", pMsgErrorInjection->errorType), "32");
            break;
    }
    pMsgErrorInjection->reErrInjectTimes--;
    if (!pMsgErrorInjection->reErrInjectTimes)
    {
        CP2FPMsgSts sendSts = msgNoEmptyEntry;
        pMsgErrorInjection->injectSts = Cpu1NoInject;
        pMsgErrorInjection->dflIdx = INVALID_REFERENCE_WITH_DOUBLE;
        errInjectBitmap &= ~(BIT_ULL(errIdx));
        writeq(errInjectBitmap, pErrInjectBitmap);
        while (sendSts == msgNoEmptyEntry && !(errInjectBitmap))
        {
            sendSts = gFpsCpu2.SendFPMsg(cM7Core1, errInjectMsg, 0, msgSuccess, 0, 0);// disable cpu1 errinject flag
        }
        gFpsCpu2.errInjectCnt--;

    }
    else
    {
        pMsgErrorInjection->injectSts = Cpu1WaitInject;
        pMsgErrorInjection->dflIdx = dflIdx;
    }
}

uint8_t Cpu2ScanErrInjectTable(uint64_t errInjectBitmap, uint16_t dflIdx)
{
    CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection = gFpsCpu2.pMsgErrorInjection;
    uint8_t errInjectIdx = INVALID_REFERENCE_WITH_BYTE;
    uint8_t errInjectCnt;
    uint16_t errCmdId;
    uint8_t errVfId, errPhyQId;
    for (errInjectCnt = FindNextBit64(errInjectBitmap); (errInjectBitmap != 0); \
         errInjectBitmap &= ~(BIT_ULL(errInjectCnt)), errInjectCnt = FindNextBit64(errInjectBitmap))
    {
        CP2FPMsgDataMsgErrorInjection_t* pErrorInjection = &pMsgErrorInjection[errInjectCnt];

        if ((pErrorInjection->injectSts == Cpu1Inject) && (pErrorInjection->dflIdx == dflIdx))
        {
            errInjectIdx = errInjectCnt;
            break;
        } // else do nothing
    }
    return errInjectIdx;
}

void Cpu2HandleErrInject(uint64_t errInjectBitmap, uint8_t* pCdmaErrSts, LionFPCmdMetaData_t* pFpCmd)
{
    uint8_t errIdx;
    errIdx = Cpu2ScanErrInjectTable(errInjectBitmap, dflIdx);
    if (errIdx != INVALID_REFERENCE_WITH_BYTE)
    {
        Cpu2CdmaErrorInjectionSetErrorType(errInjectBitmap, pCdmaErrSts, dflIdx, errIdx);
    } // else do nothing, not error inject case. It's real error case.
}

bool Cpu2ChkErrInjectErrType(uint8_t errorType, uint8_t reErrInjectTimes)
{

    switch (errorType)
    {
        case cMsgErrInjectCdmaNonFatalErr:
        {
            if (reErrInjectTimes > CMD_NON_FATAL_ERROR_MAX_INJECT_TIMES)
            {
                return false;
            } // else do nothing
        }
        break;
        case cMsgErrInjectCdmaPoorSGLErr:
        {
            #if 1
            return false;
            #else
            if (reErrInjectTimes > CMD_POOR_SGL_MAX_INJECT_TIMES)
            {
                return false;
            } // else do nothing
            #endif
        }
        break;
        case cMsgErrInjectCdmaFatalErr:
        {
            if (reErrInjectTimes > CMD_FATAL_ERROR_MAX_INJECT_TIMES)
            {
                return false;
            } // else do nothing
        }
        break;
        default:
            DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Cpu2ChkErrInjectErrType: invalid error type:0x%X\n", errorType), "32");
            break;
    }
    return true;

}

bool Cpu2ChkVFQ(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, uint8_t* pIbQ2ObQ, QueueBlockInfo_t* pQueueBlockInfoBase)
{
    uint8_t vfId =  pErrInjectData->vfId;
    uint8_t ibPhyQId = pErrInjectData->ibPhyQId;

    uint8_t queueblk = SQ_PID_2_QBIDX(ibPhyQId);
    if (pIbQ2ObQ[ibPhyQId] == QID_INVALID)
    {
        return false;
    } // else do nothing
    if ((vfId != INVALID_REFERENCE_WITH_BYTE) && (pQueueBlockInfoBase[queueblk].vfId != vfId))
    {
        return false;
    } // else do nothing
    return true;

}
uint8_t Cpu2ChkErrInjectCondition(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, uint8_t* pIbQ2ObQ, uint64_t* pVFEnBitmap, QueueBlockInfo_t* pQueueBlockInfoBase)
{
    uint16_t cmdId = pErrInjectData->cmdId;
    uint8_t vfId =  pErrInjectData->vfId;
    uint8_t ibPhyQId = pErrInjectData->ibPhyQId;
    uint64_t vfEnBitmap = readq(pVFEnBitmap);

    if ((cmdId == INVALID_REFERENCE_WITH_DOUBLE) && (ibPhyQId == INVALID_REFERENCE_WITH_BYTE) && (vfId == INVALID_REFERENCE_WITH_BYTE))
    {
        return msgInvalidField;
    } // else do nothing

    if ((vfId != INVALID_REFERENCE_WITH_BYTE) && !(vfEnBitmap & BIT_ULL(vfId))) //chk vf valid
    {
        return msgInvalidField;
    } // else do nothing

    if ((ibPhyQId != INVALID_REFERENCE_WITH_BYTE) && !Cpu2ChkVFQ(pErrInjectData, pIbQ2ObQ, pQueueBlockInfoBase)) //  chk vf + Q
    {
        return msgInvalidField;
    } // else do nothing

    if (!Cpu2ChkErrInjectErrType(pErrInjectData->errorType, pErrInjectData->reErrInjectTimes))
    {
        return msgInvalidField;
    } // else do nothing

    return msgSuccess;
}
uint8_t Cpu2AddErrInject(CP2FPMsgDataMsgErrorInjection_t* pErrInjectData, \
                         uint8_t* pIbQ2ObQ, uint64_t* pVFEnBitmap, QueueBlockInfo_t* pQueueBlockInfoBase)
{
    CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection = gFpsCpu2.pMsgErrorInjection;
    uint64_t* pErrInjectBitmap = gFpsCpu2.pErrInjectBitmap;
    uint8_t errInjectResult;
    errInjectResult = Cpu2ChkErrInjectCondition(pErrInjectData, pIbQ2ObQ, pVFEnBitmap, pQueueBlockInfoBase);

    if (errInjectResult == msgSuccess)
    {
        uint64_t errInjectBitmap = readq(pErrInjectBitmap);
        uint8_t i;
        for (i = 0; i < M7_ERR_INJECT_DEPTH; i++)
        {
            if (!(errInjectBitmap & BIT_ULL(i)))
            {
                errInjectBitmap |= BIT_ULL(i);
                break;
            } // else do nothing
        }

        pErrInjectData->injectSts = Cpu1WaitInject;
        pErrInjectData->dflIdx = INVALID_REFERENCE_WITH_DOUBLE;
        M7_MEM_COPY(&pMsgErrorInjection[i], pErrInjectData, sizeof(CP2FPMsgDataMsgErrorInjection_t));
        writeq(errInjectBitmap, pErrInjectBitmap);
        //*(gFpsCpu2.pErrInjectBitmap) = errInjectBitmap;
        gFpsCpu2.errInjectCnt++;
        gFpsCpu2.totalErrInjectCnt++;

    } // else do nothing
    return errInjectResult;

}


//-----------------------------------------------------------------------------
//  Interface Function Definitions
//-----------------------------------------------------------------------------

#endif
