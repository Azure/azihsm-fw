// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2FatalAndKeyErrorhandler.cpp
//! @brief  FpsCpu2 handle CDMA uncorr/corr key error and fatal error
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2.h"
extern "C"
{
#include "vicommon.h"
#include "crashdump.h"
}

void fpsCpu2::FpsCpu2CheckCDMAFatalErrorIrq(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    uint32_t interruptCause = HalCDMA_GetInterruptCause();

    if (interruptCause & CDMA_INT_CAUSE_KV_MEM_UNCORRECTABLE_ECC_ERR)
    {
        //In case of CDMA Key Vault Uncorrectable Error invoke the Crash Dump Handler
        Explicit_CrashCatcher_Entry();
        _uncorrectableKeyErrorOccurred = true;
    }

    if (interruptCause & CDMA_INT_CAUSE_KV_MEM_CORR_EXCEED_THRESHOLD_ERR)
    {
        if (_cdmaCorrectableKeyErrorHandleState == cCorrtableErrorhandlingWait)
        {
            HalCDMA_WriteOneClearInterruptCause(CDMA_INT_CAUSE_KV_MEM_CORR_EXCEED_THRESHOLD_ERR);
            APICDMA_ClearGlobalCheckEnable(KV_MEM_ECC_ERR_CHECK_EN); //clear bits in/from the CDMA global check enable register.
            _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingStart;
            _fpsCpu2HandleCDMAKeyCorrtableErrFiber.Resume();
        }
    }
    else if (interruptCause & CDMA_INT_CAUSE_FATAL_ERROR)
    {
        if (pThis->_cdmaFatalErrorHandleState == cFatalErrorHandlingWait)
        {
            writel(1, pThis->_cdmaFatalErrorFlag);
            _cdmaFatalErrorHandleState = cFatalErrorHandlingStart;
            _fpsCpu2HandleCDMAFatalErrorFiber.Resume();
        } //else do nothing
    }
    else
    {
        VicIrqEnable(CDMA_INT_1_NUM);
    }
}

void fpsCpu2::FpsCpu2HandleCDMAKeyCorrtableErrFiber(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    CDMACorrectableKeyErrorHandlingState_t curState = pThis->_cdmaCorrectableKeyErrorHandleState;

    switch (curState)
    {
        case cCorrtableErrorhandlingStart:
        {
            pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingNotifyCpu1;
        }
        case cCorrtableErrorhandlingNotifyCpu1:
        {
            CP2FPMsgSts msgSts = msgNoEmptyEntry;
            msgSts = pThis->SendFPMsg(cM7Core1, ConfigIOMsg, 0, msgSuccess, cPauseIO,  0);
            if (msgSts != msgSuccess)
            {
                return;
            }
            else
            {
                pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWaitCpu1Resp;
            }
            break;
        }
        case cCorrtableErrorhandlingWaitCpu1Resp:
        {
            return;
        }
        case cCorrtableErrorhandlingWaitCDMAIdle:
        {
            if (HalCDMA_ReadStatus() & CDMA_STS_CDMA_ACTIVE)
            {
                return;
            } //else do nothing
            pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingNotifyCP;
            break;
        }
        case cCorrtableErrorhandlingNotifyCP:
        {
            uint32_t errorCount = HalCDMA_GetCorrKeyErrCount(cCDMACorrKeyError);
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("cdma key corrtable err: key vault mem corrtable err, errCnt:0x%X\n", errorCount), "32");
            gFpsCpu2.MsgHandleSendHsmReq(msgSubOpKeyVaultReload);
            pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWaitCPResp;
            break;
        }
        case cCorrtableErrorhandlingWaitCPResp:
        {
            break;
        }
        case cCorrtableErrorhandlingNotifyCpu1ResumeIO:
        {
            CP2FPMsgSts msgSts = msgNoEmptyEntry;
            msgSts = pThis->SendFPMsg(cM7Core1, ConfigIOMsg, 0, msgSuccess, cResumeIO,  0);
            if (msgSts != msgSuccess)
            {
                return;
            }
            pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWaitCpu1Resume;
            break;
        }
        case cCorrtableErrorhandlingWaitCpu1Resume:
        {
            break;
        }
        case cCorrtableErrorhandlingDone:
        {
            pThis->_cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWait;
            pThis->_fpsCpu2HandleCDMAKeyCorrtableErrFiber.Wait();
            VicIrqEnable(CDMA_INT_1_NUM);
            break;
        }
        default:
        {
            break;
        }
    }
    return;

}

void fpsCpu2::FpsCpu2HandleCDMAFatalErrorFiber(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    CDMAFatalErrorHandlingState_t curState = pThis->_cdmaFatalErrorHandleState;

    switch (curState)
    {
        case cFatalErrorHandlingStart:
        {
            pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingNotifyCpu1;
        }
        case cFatalErrorHandlingNotifyCpu1:
        {
            CP2FPMsgSts msgSts = msgNoEmptyEntry;

            msgSts = pThis->SendFPMsg(cM7Core1, fatalErrorMsg, 0, msgSuccess, cPauseIO,  0);

            if (msgSts != msgSuccess)
            {
                return;
            }
            else
            {
                pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWaitCpu1Resp;
            }
        }
        break;
        case cFatalErrorHandlingWaitCpu1Resp:
        {
            return;
        }
        break;
        case cFatalErrorHandlingWaitHandleCompletion:
        {

            //Disable CDMA SQ/CQ
            if (HalCDMA_ReadStatus() & CDMA_STS_CDMA_ACTIVE)
            {
                return;
            } //else do nothing
            if (pThis->cdmaCqCi == readl(pThis->_cdmaCq.pHwPi))
            {
                pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingClassifyErrorSlots;
                pThis->_fpsCpu2ProcessCdmaCqOslFiber.Wait();
                #ifdef SUPPORT_CDMA_RESET_MSG
                if (pThis->duringCDMAResetMessage)
                {
                    pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWaitOutboundProcessDone;
                }
                #endif
            }
            else
            {
                return;
            }
        }
        break;
        case cFatalErrorHandlingClassifyErrorSlots:
        {
            pThis->_HandleFatalErrorSlotStatus(pThis, CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0);
            pThis->_HandleFatalErrorSlotStatus(pThis, CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1);
            pThis->_HandleFatalErrorSlotStatus(pThis, CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2);
            pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWaitOutboundProcessDone;
        }
        case cFatalErrorHandlingWaitOutboundProcessDone:
        {
            if ((pThis->outBoundOSLPi[OSL_0] == readl(pThis->_ucdObq.pHwOslCi[OSL_0]))  &&            \
                (pThis->outBoundOSLPi[OSL_1] == readl(pThis->_ucdObq.pHwOslCi[OSL_1])) &&             \
                (readl(pThis->_ucdObq.pHwOslCi[OSL_0]) == readl(pThis->_ucdObq.pHwObCqCi[OBCQ_0])) && \
                (readl(pThis->_ucdObq.pHwOslCi[OSL_1]) == readl(pThis->_ucdObq.pHwObCqCi[OBCQ_1])) && \
                (readl(pThis->pRetryCEQueuePi) == readl(pThis->pRetryCEQueueCi)))
            {
                if (pThis->_uncorrectableKeyErrorOccurred)
                {
                    pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWaitCPReWriteKey;
                }
                else
                {
                    pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingNotifyCpu0;
                }
                pThis->CDMAInit();
                pThis->cdmaCqCi = 0;
                #ifdef CDMA_CMD_COUNT
                pThis->cdmaCmdSlotQueueCi = 0;
                writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
                #endif // End of CDMA_CMD_COUNT
                writel(0, pThis->_cdmaCq.pHwPi);
                writel(0, pThis->_cdmaCq.pHwCi);
                pThis->cdmaSlotAbortQueuePi = 0;
                writel(0, pThis->pCdmaSlotAbortQueuePi);
                pThis->_fpsCpu2ProcessCdmaCqOslFiber.Resume();
            }
            else
            {
                break;
            }
            break;
        }
        case cFatalErrorHandlingWaitCPReWriteKey:
        {
            return;
        }
        case cFatalErrorHandlingNotifyCpu0:
        {
            CP2FPMsgSts msgSts = msgNoEmptyEntry;
            msgSts = pThis->SendFPMsg(cM7Core0, cdmaResetMsg, 0, msgSuccess, 0,  0);
            if (msgSts == msgNoEmptyEntry)
            {
                break;
            }
            else
            {
                pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWaitResetDone;
                break;
            }
        }
        case cFatalErrorHandlingWaitResetDone:
        {
            if (readl(pThis->_cdmaFatalErrorFlag) == 1)
            {
                break;
            }//else do nothing

            #ifdef SUPPORT_CDMA_RESET_MSG
            pThis->duringCDMAResetMessage = false;
            #endif
            pThis->_cdmaFatalErrorHandleState = cFatalErrorHandlingWait;
            pThis->_fpsCpu2HandleCDMAFatalErrorFiber.Wait();
            VicIrqEnable(CDMA_INT_1_NUM);
            break;
        }

        default:
            break;

    }

    return;
}

void fpsCpu2::_HandleFatalErrorSlotStatus(void* pObj, uint8_t errorSlotId)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    uint32_t errorSlot = 0;
    uint32_t slotIdOffset = 0;
    uint32_t slotId;
    switch (errorSlotId)
    {
        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0:
        {
            errorSlot = HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0);
            slotIdOffset = 0;
        }
        break;
        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1:
        {
            errorSlot = HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1);
            slotIdOffset = 32;
        }
        break;
        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2:
        {
            errorSlot = HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2) & 0xF;
            slotIdOffset = 64;
        }
        break;
        default:
            return;
    }
    while (errorSlot)
    {
        slotId = FindNextBit32(errorSlot);
        HandleFatalErrorSlot(pThis, slotId + slotIdOffset);
        errorSlot &= ~(BIT(slotId));
    }
    return;
}

void fpsCpu2::HandleFatalErrorSlot(void* pObj, uint32_t slotId)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    API_CDMASetDiagnosticControl(slotId);
    uint32_t ceIndex = HalCDMA_GetDiagnosticCommandSlotStatus() & CDMA_DIAGNOSTIC_CMD_SLOT_STATUS_CPUID_MASK;
    volatile uint32_t cdmaErrorStatus0 = API_CDMAGetCmdSlotErrorStatus(CDMA_CMD_SLOT_ERR_STS_REG_ID_0);
    volatile uint32_t cdmaErrorStatus1 = API_CDMAGetCmdSlotErrorStatus(CDMA_CMD_SLOT_ERR_STS_REG_ID_1);

    if(cdmaErrorStatus1 & CRYPTOE_FATAL_ERR)
    {
        //In case of Fatal Error invoke the Crash Dump Handler on Specific Fatal Errors
        DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("HandleFatalErrorSlot: cdmaErrorStatus0 = 0x%X cdmaErrorStatus1 = 0x%X\n", cdmaErrorStatus0, cdmaErrorStatus1), "32","32");
        Explicit_CrashCatcher_Entry();
    }
    else
    {
        if (ceIndex == CP_CDMA_IO_CMD_ID && ((cdmaErrorStatus0 & FATAL_ERROR_MASK_REG_0) || (cdmaErrorStatus1 & FATAL_ERROR_MASK_REG_1)))
        {
            //CP CDMA IO Fatal error handling
            writel(cCEStsFatalError, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
            CmdEntryTinyHostErrCode_t errCode = FatalErrorErrCode(cdmaErrorStatus0, cdmaErrorStatus1);
            writel(errCode, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
            return;
        }//else do nothing

        CmdEntry_t* pCmdEntry = &pThis->_pCmdEntryArrayBase[ceIndex];
        uint8_t caIdx = (ceIndex >> CA_SIZE_SHIFT);
        uint8_t phyQId = pCa2IbPhysicalId[caIdx];
        if (pThis->_pSlotFlagSts[phyQId] & (cStsDelete | cStsTearDown))
        {
            return;
        }

        #ifdef SUPPORT_ERROR_INJECTION
        LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem((uint32_t)GET_DFL_BUFF_ADDR_FROM_CDMA_LIST(pCmdEntry->cdmaListNum)) + \
                (pCmdEntry->DFLIdx << DFL_BUF_SZ_SHIFT));
        if (cdmaErrorStatus0 & (DESCM_SRC_DESCR_STRUCTURE_ERR | DESCM_SRC_PRP_ALIGN_ERR))
        {
            FpsCpu2ErrorInjection_InjectErr(&cdmaErrorStatus0, &cdmaErrorStatus1, pFpCmd);
        } // else do nothing
        #elif SUPPORT_MSGERROR_INJECTION
        uint64_t errInjectBitmap = readq(pThis->pErrInjectBitmap);
        if (errInjectBitmap && (cdmaErrorStatus0 & (DESCM_SRC_DESCR_STRUCTURE_ERR | DESCM_SRC_PRP_ALIGN_ERR))) //check error is from cpu1 error injection
        {
            uint8_t cdmaErrSts = 0;
            Cpu2HandleErrInject(errInjectBitmap, &cdmaErrSts, pCmdEntry->DFLIdx);
        } // else do nothing, not error inject case. It's real error case.
        #endif

        if ((cdmaErrorStatus0 & (FATAL_ERROR_MASK_REG_0 | NON_DEFINED_ERROR_MASK_REG_0)) || \
                (cdmaErrorStatus1 & (FATAL_ERROR_MASK_REG_1 | NON_DEFINED_ERROR_MASK_REG_1)))
        {
            CmdEntryTiny_t* pCmdEntryTiny = &_pCmdEntryArrayTinyBase[ceIndex];
            pCmdEntryTiny->ErrStatus = cCETinyStsFatalErr;
            pCmdEntryTiny->RetryTimes += 1;

            #ifdef SUPPORT_TELEMETRY
            TcFaultErrCnt++;
            #endif

            if (((cdmaErrorStatus1 & FATAL_ERROR_MASK_REG_1) & FATAL_ERROR_RETRY_MASK_REG_1)  && !ChkRetryTimesExceeded(ceIndex))
            {
                if (pCmdEntry->Status < cCEStsFatalError)
                {
                    pCmdEntry->Status = cCEStsRetry;
                } // else do nothing
            }
            else
            {
                //Non-Retry case
                if (pCmdEntry->Status < cCEStsFatalError)
                {
                    if(cdmaErrorStatus0 & FATAL_ERROR_MASK_REG_0)
                    {
                        pCmdEntry->Status = cCEStsFetchError;
                        pCmdEntryTiny->HostErrCode = FatalErrorErrCode(cdmaErrorStatus0, cdmaErrorStatus1);
                    }
                    else if(cdmaErrorStatus1 & FATAL_ERROR_MASK_REG_1)
                    {
                        pCmdEntry->Status = cCEStsCryptoEngineError;
                        pCmdEntryTiny->HostErrCode = FatalErrorErrCode(cdmaErrorStatus0, cdmaErrorStatus1);
                    }
                }
                else
                {
                    DebugLogLvDbgInfo(cLogCPU2Common,cLogError, ("Entered because of Non-defined error mask reg, cdmaErrorStatus0:[0x%X] cdmaErrorStatus1:[0x%X]\n", \
                    cdmaErrorStatus0, cdmaErrorStatus1), "32", "32");
                }

            }
        }
    }
}

CmdEntryTinyHostErrCode_t fpsCpu2::FatalErrorErrCode(uint32_t cdmaErrorStatus0, uint32_t cdmaErrorStatus1)
{
    CmdEntryTinyHostErrCode_t errCode = cCETinyHostErrDefaultErrorCode;

    if(cdmaErrorStatus0)
    {
        if((cdmaErrorStatus0 & (DESCM_DEST_PRP_ALIGN_ERR | DESCM_DEST_PRP_OFST_ERR)))
        {
            errCode = cCETinyHostErrCqeDestPrpFetchError;
        }
        else if((cdmaErrorStatus0 & (DESCM_DEST_SGL_LENGTH_ALIGNMENT_ERR | DESCM_DEST_SGL_TBL_LENGTH_ERR | \
            DESCM_DEST_SGL_UNDEFINED_DSCRPTR_TYPE | DESCM_DEST_SGL_ILLEGAL_DSCRPTR_ERR | DESCM_DEST_SGL_CROSS_4K_ERR)))
        {
            errCode = cCETinyHostErrCqeDestSglFetchError;
        }
        else if((cdmaErrorStatus0 & (DESCM_SRC_PRP_ALIGN_ERR | DESCM_SRC_PRP_OFST_ERR)))
        {
            errCode = cCETinyHostErrCqeSrcPrpFetchError;
        }
        else if((cdmaErrorStatus0 & (DESCM_SRC_SGL_LENGTH_ALIGNMENT_ERR | DESCM_SRC_SGL_TBL_LENGTH_ERR | \
            DESCM_SRC_SGL_UNDEFINED_DSCRPTR_TYPE | DESCM_SRC_SGL_ILLEGAL_DSCRPTR_ERR | DESCM_SRC_SGL_CROSS_4K_ERR)))
        {
            errCode = cCETinyHostErrCqeSrcSglFetchError;
        }
        else if(cdmaErrorStatus0 & DESCM_DEST_DESCR_STRUCTURE_ERR)
        {
            errCode = cCETinyHostErrCqeDescmDestDescrStructureErr;
        }
        else if(cdmaErrorStatus0 & DESCM_SRC_DESCR_STRUCTURE_ERR)
        {
            errCode = cCETinyHostErrCqeDescmSrcDescrStructureErr;
        }

        return errCode;
    }

    if(cdmaErrorStatus1){

        if((cdmaErrorStatus1 & DOE_OVERRUN_ERR))
        {
            errCode = cCETinyHostErrCqeDoeOverrunErr;
        }
        if((cdmaErrorStatus1 & DOE_UNDERRUN_ERR))
        {
            errCode = cCETinyHostErrCqeDoeUnderrunErr;
        }
        if((cdmaErrorStatus1 & CRYPTOE_FATAL_ERR))
        {
            errCode = cCETinyHostErrCqeCryptoeFatalErr;
        }
        if((cdmaErrorStatus1 & CRYPTOE_KEY_VAULT_MEM_RD_ERR))
        {
            errCode = cCETinyHostErrCqeCryptoeKeyVaultMemRdErr;
        }
        if((cdmaErrorStatus1 & DFE_OVERRUN_ERR))
        {
            errCode = cCETinyHostErrCqeDfeOverrunErr;
        }
        if((cdmaErrorStatus1 & DFE_UNDERRUN_ERR))
        {
            errCode = cCETinyHostErrCqeDfeUnderrunErr;
        }

        return errCode;
    }
}

void fpsCpu2::TriggerCDMAResetFiber()
{
    if (_cdmaFatalErrorHandleState == cFatalErrorHandlingWait)
    {
        _cdmaFatalErrorHandleState = cFatalErrorHandlingStart;
        _fpsCpu2HandleCDMAFatalErrorFiber.Resume();
    }
}
