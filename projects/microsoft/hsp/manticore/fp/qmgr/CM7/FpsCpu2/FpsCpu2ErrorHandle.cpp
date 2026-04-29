// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2ErrorHandle.cpp
//! @brief  FpsCpu2 Error Handle
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2ErrorHandle.h"
#if defined (SUPPORT_MSGERROR_INJECTION) || defined (SUPPORT_ERROR_INJECTION)
#include "FpsCpu2ErrorInjection.h"
#endif

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------


#ifdef SUPPORT_MSGERROR_INJECTION
uint8_t isCmdErrorInjectionEnable = false;
#endif

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

void fpsCpu2::FpsCpu2SendCDMAAbortRequestToCpu1(uint16_t abortCeIndex)
{
    // Need not to check the queue is full, there is only 68 cdma slot.
    pCdmaSlotAbortQueue[cdmaSlotAbortQueuePi] = abortCeIndex;
    cdmaSlotAbortQueuePi = QUEUE_INC(cdmaSlotAbortQueuePi, 0x7f);
    DMB();

    writel(cdmaSlotAbortQueuePi, pCdmaSlotAbortQueuePi);
}

void fpsCpu2::FpsCpu2SendRetryCeRequestToCpu0(uint16_t retryCeIndex)
{
    pRetryCeIndexQueue[retryCEQueuePi] = retryCeIndex;
    retryCEQueuePi = QUEUE_INC(retryCEQueuePi, 0x1ff);

    DMB();

    writel(retryCEQueuePi, pRetryCEQueuePi);
}

void fpsCpu2::FpsCpu2SendRefillDFLRequestToCpu0(uint16_t ceIndex)
{
    pCEforRefillDFLQueue[ceForRefillDFLQueuePi] = ceIndex;
    ceForRefillDFLQueuePi = QUEUE_INC(ceForRefillDFLQueuePi, 0x1ff);

    DMB();

    writel(ceForRefillDFLQueuePi, pCeForRefillDFLQueuePi);
}

void fpsCpu2::Cpu2CdmaErrorCmdHandler(void)
{
    CdmaCqCmdDescr_t* pCdmaCqe = &_cdmaCq.pCdmaCqBase[cdmaCqCi];
    API_CDMASetDiagnosticControl(((uint32_t)pCdmaCqe->Dw1.CmdSlot));
    volatile uint32_t cdmaErrorStatus0 = API_CDMAGetCmdSlotErrorStatus(CDMA_CMD_SLOT_ERR_STS_REG_ID_0);
    volatile uint32_t cdmaErrorStatus1 = API_CDMAGetCmdSlotErrorStatus(CDMA_CMD_SLOT_ERR_STS_REG_ID_1);

    #ifdef QOS_LATENCY_TEST
    if (!cdmaErrorStatus1 && (cdmaErrorStatus0 & CDMA_INVALID_OPCODE_ERR)) // non fatal error inject
    {
        cdmaErrorStatus0 = QOS_LATENCY_TO_ERR;
    }
    #endif

    if (cdmaErrorStatus0)
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("[IO LOG] cid:0x%X, ceIndex:0x%X, cdmaErrSt0:0x%X\n", \
                                                 (((uint32_t)(pCdmaCqe->Dw0.CmdId) << 0x10UL) | ((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL)), cdmaErrorStatus0), "16,16", "32");
    }

    if (cdmaErrorStatus1)
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("[IO LOG] cid:0x%X, ceIndex:0x%X, cdmaErrSt1:0x%X\n", \
                                                 (((uint32_t)(pCdmaCqe->Dw0.CmdId) << 0x10UL) | ((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL)), cdmaErrorStatus1), "16,16", "32");
    }

    uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;
    if(ceIndex != CP_CDMA_IO_CMD_ID)
    {
        CmdEntry_t* pCmdEntry = &_pCmdEntryArrayBase[ceIndex];
        CmdEntryTiny_t* pCmdEntryTiny = &_pCmdEntryArrayTinyBase[ceIndex];
        uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCdmaCqe->Dw0.DflNum, (pCdmaCqe->Dw0.CmdDflIdx << DFL_BUF_SZ_SHIFT));
        LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));
        if (cdmaErrorStatus1 & CDMA_CMD_SLOT_ERR_STS_REG_POORLY_SGL_ERR_MASK)
        {

            pCmdEntryTiny->ErrStatus = cCETinyStsPoorSGLErr;
            pCmdEntry->Status = cCEStsPoorSGLRetry;

            pCmdEntryTiny->RetryTimes = pCmdEntryTiny->RetryTimes + 1;
            pFpCmd->sqe.PoorSGLRetryTimes = pCmdEntryTiny->RetryTimes;

#ifdef SUPPORT_TELEMETRY
            TcPoorConstructedSglCnt++;
#endif
        }
        else
        {
            if ((cdmaErrorStatus0 & (NON_FATAL_RETRY_MASK_REG_0)) || (cdmaErrorStatus1 & (NON_FATAL_RETRY_MASK_REG_1)))
            {
                pCmdEntryTiny->ErrStatus = cCETinyStsNonFatalErr;
                pCmdEntry->Status = cCEStsRetry;

                pCmdEntryTiny->RetryTimes = pCmdEntryTiny->RetryTimes + 1;
                pFpCmd->sqe.PoorSGLRetryTimes = pCmdEntryTiny->RetryTimes;
            }
            else
            {
#ifdef QOS_LATENCY_ERROR_HANDLING
                if (cdmaErrorStatus0 & QOS_LATENCY_TO_ERR)
                {
                    pCmdEntryTiny->ErrStatus = cCETinyStsQosErr;
                    pCmdEntry->Status = cCEStsQoSError;
                    pCmdEntryTiny->HostErrCode = CQE_DEFAULT_ERROR_CODE;
                }
                else
#endif
                {
                    if(cdmaErrorStatus0 || cdmaErrorStatus1)
                    {
                        pCmdEntryTiny->ErrStatus = cCETinyStsReportHost;
                        pCmdEntry->Status = NonFatalStatusCode(cdmaErrorStatus0, cdmaErrorStatus1);
                        pCmdEntryTiny->HostErrCode = NonFatalErrorCode(cdmaErrorStatus0,cdmaErrorStatus1);
                    }
                    else
                    {
                        DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CDMA cmd returned error, but Slot Error Registers are not set [0x%x]\n", cdmaErrorStatus0 ), "32");
                    }
                }
            }

#ifdef SUPPORT_TELEMETRY
            if ((cdmaErrorStatus0 & NON_DEFINED_ERROR_MASK_REG_0) || (cdmaErrorStatus1 & NON_DEFINED_ERROR_MASK_REG_1))
            {
                TcFaultErrCnt++;
            }
            else
            {
                TcNonFaultErrCnt++;
            }
#endif

#ifdef DEBUG_BUILD
            FpsCpu2PrintErrInfoLogByErrStsType(pFpCmd, pCmdEntry, pCmdEntryTiny->ErrStatus, cdmaErrorStatus0, cdmaErrorStatus1);
#endif
        }
    }
    else
    {
        if((cdmaErrorStatus1 & CDMA_CMD_SLOT_ERR_STS_REG_POORLY_SGL_ERR_MASK) || (cdmaErrorStatus1 & NON_FATAL_RETRY_MASK_REG_1)){
            writel(CQE_SC_CRYPTO_ENGINE_ERROR, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
            writel(NonFatalErrorCode(cdmaErrorStatus0,cdmaErrorStatus1), PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Error Path StsCode = 0x%x, ErrCode = 0x%x", readl(PSRAM_FP_CPIO_CDMA_STATUS_ADDR), readl(PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR)), "32","32");

        }
        else if(cdmaErrorStatus0 & NON_FATAL_RETRY_MASK_REG_0){
            if (cdmaErrorStatus0 & CMDE_AXI_RD_ERR ){
                writel(CQE_SC_CRYPTO_ENGINE_ERROR, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
                writel(NonFatalErrorCode(cdmaErrorStatus0,cdmaErrorStatus1), PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
            }
            else{
                writel(CQE_SC_FETCH_ERROR, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
                writel(NonFatalErrorCode(cdmaErrorStatus0,cdmaErrorStatus1), PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
            }
        }
#ifdef QOS_LATENCY_TEST
        else if(cdmaErrorStatus0 & QOS_LATENCY_TO_ERR){
            writel(CQE_SC_QOS_LATENCY_ERROR, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
            writel(CQE_DEFAULT_ERROR_CODE, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
        }
#endif
        else if(cdmaErrorStatus0 & DEST_DATA_XFR_TO_ERR){
            writel(CQE_SC_DATA_TRANSFER_TIMEOUT, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
            writel(CQE_DEST_XFER_DATA_TIMEOUT, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
        }
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Sending Abort Request to FP1 0x%x", ceIndex), "32");
        CPCDMAIOStatus = CDMA_CMD_ERROR;

    }

    DMB();
}

CmdEntryStatus_t fpsCpu2::NonFatalStatusCode(uint32_t cdmaErrorStatus0, uint32_t cdmaErrorStatus1)
{
    CmdEntryStatus_t statusCode = cCEStsValid;

    if(cdmaErrorStatus0)
    {
        if(cdmaErrorStatus0 & (CMDE_AXI_RD_ERR | DESCM_DEST_AXI_RD_ERR | DESCM_SRC_AXI_RD_ERR))
        {
            statusCode = cCEStsCryptoEngineError;
        }
        else if(cdmaErrorStatus0 & DEST_DATA_XFR_TO_ERR)
        {
            statusCode = cCEStsDataTranferTimeout;
        }
        else
        {
            statusCode = cCEStsFetchError;
        }

        return statusCode;
    }

    if(cdmaErrorStatus1)
    {
        if(cdmaErrorStatus1 & (CMD_TRANSFER_LENGTH_UNDERRUN_ERR | CMD_TRANSFER_LENGTH_OVERRUN_ERR))
        {
            statusCode = cCEStsFetchError;
        }
        else
        {
            statusCode = cCEStsCryptoEngineError;
        }

        return statusCode;
    }
}

CmdEntryTinyHostErrCode_t fpsCpu2::NonFatalErrorCode(uint32_t cdmaErrorStatus0,uint32_t cdmaErrorStatus1)
{
    CmdEntryTinyHostErrCode_t errCode = cCETinyHostErrDefaultErrorCode;

    if(cdmaErrorStatus0)
    {
        if(cdmaErrorStatus0 & DEST_DATA_XFR_TO_ERR)
        {
            errCode = cCETinyHostErrCqeDestXferDataTimeout;
        }
        else if(cdmaErrorStatus0 & DESCM_DEST_DESCR_SGL_SEG_ERR)
        {
            errCode = cCETinyHostErrCqeDestSglFetchError;
        }
        else if(cdmaErrorStatus0 & DESCM_SRC_DESCR_SGL_SEG_ERR)
        {
            errCode = cCETinyHostErrCqeSrcSglFetchError;
        }
        else if(cdmaErrorStatus0 & CMDE_UNEXPECTED_CMD_PHASE_ERR)
        {
            errCode = cCETinyHostErrCqeCmdeUnexpectedCmdPhaseErr;
        }
        else if(cdmaErrorStatus0 & CMDE_INVALID_OPCODE_ERR)
        {
            errCode = cCETinyHostErrCqeCmdeInvalidOpcodeErr;
        }
        else if(cdmaErrorStatus0 & CMDE_STRUCTURE_ERR )
        {
            errCode = cCETinyHostErrCqeCmdeStructureErr;
        }
        else if(cdmaErrorStatus0 & DESCM_DEST_AXI_RD_ERR)
        {
            errCode = cCETinyHostErrCqeDescmDestAxiRdErr;
        }
        else if(cdmaErrorStatus0 & DESCM_SRC_AXI_RD_ERR)
        {
            errCode = cCETinyHostErrCqeDescmSrcAxiRdErr;
        }
        else if(cdmaErrorStatus0 & FUNC_IN_ERR_STATE)
        {
            errCode = cCETinyHostErrCqeSrcVfFetchError;
        }

        return errCode;
    }

    if(cdmaErrorStatus1){
        if(cdmaErrorStatus1 & CMD_TRANSFER_LENGTH_UNDERRUN_ERR)
        {
            errCode = cCETinyHostErrCqeCmdTransferLengthUnderrunErr;
        }
        else if(cdmaErrorStatus1 & CMD_TRANSFER_LENGTH_OVERRUN_ERR)
        {
            errCode = cCETinyHostErrCqeCmdTransferLengthOverrunErr;
        }
        else if(cdmaErrorStatus1 & CMPLE_AXI_WR_ERR)
        {
            errCode = cCETinyHostErrCqeCmpleAxiWrErr;
        }
        else if(cdmaErrorStatus1 & DOE_MAX_ELMNT_COUNT_ERR)
        {
            errCode = cCETinyHostErrCqeDoeMaxElmntCountErr;
        }
        else if(cdmaErrorStatus1 & DOE_BUFFER_RD_PARITY_ERR)
        {
            errCode = cCETinyHostErrCqeDoeBufferRdParityErr;
        }
        else if(cdmaErrorStatus1 & DOE_AXI_WR_ERR)
        {
            errCode = cCETinyHostErrCqeDoeAxiWrErr;
        }
        else if(cdmaErrorStatus1 & CRYPTOE_TAG_MISMATCH_ERR)
        {
            errCode = cCETinyHostErrCqeCryptoeTagMismatchErr;
        }
        else if(cdmaErrorStatus1 & CRYPTOE_TEXT_OUT_READ_ERR)
        {
            errCode = cCETinyHostErrCqeCryptoeTextOutReadErr;
        }
        else if(cdmaErrorStatus1 & CRYPTOE_REDUNDANCY_MISMATCH_ERR)
        {
            errCode = cCETinyHostErrCqeCryptoeRedundancyMismatchErr;
        }
        else if(cdmaErrorStatus1 & DBM_BUFFER_RD_PARITY_ERR)
        {
            errCode = cCETinyHostErrCqeDbmBufferRdParityErr;
        }
        else if(cdmaErrorStatus1 & DBM_AXI_RD_ERR)
        {
            errCode = cCETinyHostErrCqeDbmAxiRdErr;
        }
        else if(cdmaErrorStatus1 & DFE_MAX_ELMNT_COUNT_ERR)
        {
            errCode = cCETinyHostErrCqeDfeMaxElmntCountErr;
        }

        return errCode;
    }
}

#ifdef DEBUG_BUILD
ATTR_NO_INLINE void fpsCpu2::FpsCpu2PrintErrInfoLogByErrStsType(LionFPCmdMetaData_t* pFpCmd, CmdEntry_t* pCmdEntry, uint8_t errStsType, uint32_t cdmaErrSts0, uint32_t cdmaErrSts1)
{
    switch (errStsType)
    {
        case cCETinyStsNonFatalErr:
        {
            DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: detect non-fatal err, hiuId:0x%X cid:0x%X\n", ((((uint32_t)pFpCmd->cqe.CmdId & 0xFFFF) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,24");
            break;
        }
        case cCETinyStsPoorSGLErr:
        {
            DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: detect poorly sgl err, hiuId:0x%X cid:0x%X\n", ((((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,24");
            break;
        }
        case cCETinyStsReportHost:
        {
            if (cdmaErrSts1 & BIT(13))
            {
                DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: detect invalid gcm tag, hiuId:0x%X cid:0x%X\n", ((((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,24");
            }

            #ifdef QOS_LATENCY_ERROR_HANDLING
            else if (cdmaErrSts0 & QOS_LATENCY_TO_ERR)
            {
                DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: detect qos err, hiuId:0x%X cid:0x%X\n", ((((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,24");
            }
            #endif

            DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: classify err report host, hiuId:0x%X cid:0x%X\n", ((((uint32_t)pFpCmd->cqe.CmdId & 0xFFFFUL) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,24");
            break;
        }
        default:
        {
            DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CdmaCmdError: unexpected sts, hiuId:0x%X cid:0x%X cdmaErrSts:0x%X\n", (((errStsType & 0xFF) << 0x18UL) | ((pFpCmd->cqe.CmdId & 0xFFFF) << 0x8UL) | (UCD_IFSEL_TO_HIUID(pCmdEntry->IFSel) & 0xFF))), "8,16,8");
            break;
        }
    }
}
#endif

#ifdef LIONPERF_SUPPORT
void fpsCpu2::FpsCpu2SearchAbortMsgInMsgQueue(void)
{
    uint32_t msgPi = readl(CP0toFPReqMsg.pMsgPi);
    uint32_t msgCi = CP0toFPReqMsg.localMsgCi;
    CdmaCqCmdDescr_t* pCdmaCqe = &(_cdmaCq.pCdmaCqBase[cdmaCqCi]);
    uint8_t ibPhyQId = pCdmaCqe->Dw1.UcdIqId;
    uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;
    uint8_t vfId = MAP_FUNCTION_ID((GET_VF_ID(_pCmdEntryArrayBase[ceIndex].IFSel)));
    uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCdmaCqe->Dw0.DflNum, (pCdmaCqe->Dw0.CmdDflIdx << DFL_BUF_SZ_SHIFT));
    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));

    while (!QUEUE_EMPTY(msgPi, msgCi))
    {
        CP2FPMsgContext_t* pCP0toFPMsgQContext = (CP2FPMsgContext_t*)(((uint32_t)CP0toFPReqMsg.pMsgQ) + (PSRAM_CP2FP_MSG_ELMNT_SIZE * msgCi));
        CP2FPMsgAdminAbort_t* abortMsg = (CP2FPMsgAdminAbort_t*)pCP0toFPMsgQContext->data;

        if ((pCP0toFPMsgQContext->msgOp == msgOpErrQSet) && (abortMsg->subOp == msgSubOpAdminAbort))
        {
            if ((abortMsg->ibQId == ibPhyQId) && (abortMsg->vfId == vfId) && (abortMsg->cmdId == pFpCmd->cqe.CmdId))
            {
                abortMsg->adminAbortCompleted = 1;

                DMB();

                break;
            }
        }

        msgCi = M7_QUEUE_INC(msgCi, PSRAM_CP2FP_MSG_MASK);
    }

}
#endif

#ifdef QOS_LATENCY_ERROR_HANDLING
void fpsCpu2::Cpu2HandleQosLatencyTimeoutError(uint16_t ceIndex)
{
    CmdEntry_t* pCmdEntry = &_pCmdEntryArrayBase[ceIndex];
    uint8_t vfId = MAP_FUNCTION_ID((uint8_t)GET_VF_ID((uint8_t)(pCmdEntry->IFSel)));
    uint8_t isNeedToNotifyQoSErr = 0;
    uint8_t vfGroupIndex = vfId >> 5;

    // Condition 1: check if CP already config QoS penalty?
    #ifdef QOS_LATENCY_GLOBAL_UNIQUE
    CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &_pQosPenalty[0];
    #else
    CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &_pQosPenalty[vfId];
    #endif
    if ((pQosPenaltyInfo->Cfg.qosPenaltyPeriod == 0) || (pQosPenaltyInfo->Cfg.qosPenaltyCreditRatio >= DEFAULT_QOS_CREDIT_RATIO))
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("qos: cp didn't cfg the setting, period:0x%X ratio:0x%X\n", pQosPenaltyInfo->Cfg.qosPenaltyPeriod, pQosPenaltyInfo->Cfg.qosPenaltyCreditRatio), "32", "32");
        return;
    } // else do nothing

    // Condition 2: check if VF_n already in QoS penalty progres?
    if ((*_pQosVFBitmap[vfGroupIndex] & BIT((vfId & 0x1f))) == 0)
    {
        // Condition 3: check if FP_CPU2 already notify FP_CPU1 that VF_n needs to do the QoS penalty progress?
        switch (vfGroupIndex)
        {
            case VF0_VF31:
            case VF32_VF63:
            {
                if (!(_qosPenaltyVfBitmap & BIT_ULL(vfId)))
                {
                    isNeedToNotifyQoSErr = 1;
                } // else do nothing
                break;
            }
            case VF64:
            {
                if (_qosPenaltyVf65Bitmap == 0)
                {
                    isNeedToNotifyQoSErr = 1;
                } // else do nothing
                break;
            }
            default:
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("qos: invalid para, vfGroupIndex:0x%X\n", vfGroupIndex), "32");
                break;
            }
        }
    }// else do nothing

    if (isNeedToNotifyQoSErr)
    {
        // CPU2 mark its local penalty VF bitmap
        if (vfId == MAX_VF_NUM)
        {
            _qosPenaltyVf65Bitmap = 1;
        }
        else
        {
            _qosPenaltyVfBitmap |= BIT_ULL(vfId);
        }

        // CPU2 notify CPU1 which VF has QoS latency timeout error
        CP2FPMsgSts msgSts = msgNoEmptyEntry;
        msgSts = SendFPMsg(cM7Core1, qosPenaltyMsg, 0, msgSuccess, ((uint8_t)(ceIndex & 0x00FFU)), ((uint8_t)((ceIndex >> FPS_INTL_MSG_CMD_SPECIFIC_SHIFT) & 0x00FFU)));
        if (msgSts != msgSuccess)
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("qos: cpu2 to cpu1 msg q full, msgSts:0x%X\n", msgSts), "32");
        } // else do nothing
    }// else do nothing
}
#endif

LionFPCQEStatusCode FpsCpu2FillHostStatusCode(uint8_t ceStatus)
{
    volatile LionFPCQEStatusCode cqeStatus = CQE_SC_SUCCESS;

    switch (ceStatus)
    {
        case cCEStsDelQ:
        {
            cqeStatus = CQE_SC_DELETE_QUEUE;
        }
        break;

        case cCEStsInvalidXTSField:
        {
            cqeStatus = CQE_SC_INVALID_FIELD_XTS;
        }
        break;

        case cCEStsInvalidGCMField:
        {
            cqeStatus = CQE_SC_INVALID_FIELD_GCM;
        }
        break;

        case cCEStsCryptoEngineError:
        {
            cqeStatus = CQE_SC_CRYPTO_ENGINE_ERROR;
        }
        break;

        case cCEStsFetchError:
        {
            cqeStatus = CQE_SC_FETCH_ERROR;
        }
        break;

        case cCEStsDataTranferTimeout:
        {
            cqeStatus = CQE_SC_DATA_TRANSFER_TIMEOUT;
        }
        break;

        case cCEStsQoSError:
        {
            cqeStatus = CQE_SC_QOS_LATENCY_ERROR;
        }
        break;

        default:
            break;
    }

    return cqeStatus;

}

LionFPCQEErrorCode FpsCpu2FillHostGcmErrorCode(AesGcmExtRespErr respSts)
{
    volatile LionFPCQEErrorCode errCode = CQE_DEFAULT_ERROR_CODE;

    switch(respSts)
    {
        case INVALID_AES_GCM_REQUEST_PTR:
        {
            errCode = CP_GCM_INVALID_AES_GCM_REQUEST_PTR;
        }
        break;
        case INVALID_SQE_ADDR_PTR:
        {
            errCode = CP_GCM_INVALID_SQE_ADDR_PTR;
        }
        break;
        case INVALID_UNALIGNED_SRC_DATA_PTR:
        {
            errCode = CP_GCM_INVALID_UNALIGNED_SRC_DATA_PTR;
        }
        break;
        case INVALID_UNALIGNED_DST_DATA_PTR:
        {
            errCode = CP_GCM_INVALID_UNALIGNED_DST_DATA_PTR;
        }
        break;
        case INVALID_PCIE_FN:
        {
            errCode = CP_GCM_INVALID_PCIE_FN;
        }
        break;
        case DMA_MEM_ALLOC_FAILED:
        {
            errCode = CP_GCM_DMA_MEM_ALLOC_FAILED;
        }
        break;
        case DMA_IN_OPERATION_ERR:
        {
            errCode = CP_GCM_DMA_IN_OPERATION_ERR;
        }
        break;
        case DMA_OUT_OPERATION_ERR:
        {
            errCode = CP_GCM_DMA_OUT_OPERATION_ERR;
        }
        break;
        case AES_GCM_KEY_BLOB_READ_FAILED:
        {
            errCode = CP_GCM_KEY_BLOB_READ_FAILED;
        }
        break;
        case AES_GCM_TAG_CORRECTION_FAILED:
        {
            errCode = CP_GCM_TAG_CORR_FAILED;
        }
        break;
        case AES_GCM_INVALID_DECRYPT_TAG:
        {
            errCode = CP_GCM_INVALID_DECRYPT_TAG;
        }
        break;
        case INVALID_SQE_INDEX:
        {
            errCode = CP_GCM_INVALID_SQE_INDEX;
        }
        break;
        case INVALID_UNALIGNED_DATA_LEN:
        {
            errCode = CP_GCM_INVALID_UNALIGNED_DATA_LEN;
        }
        break;

        default:
            break;
    }

    return errCode;

}

uint8_t fpsCpu2::ChkRetryTimesExceeded(uint16_t ceIndex)
{
    uint8_t exceedFlag = false;
    CmdEntryTiny_t* pCmdEntryTiny = &_pCmdEntryArrayTinyBase[ceIndex];

    if ((((pCmdEntryTiny->ErrStatus == cCETinyStsNonFatalErr) && (pCmdEntryTiny->RetryTimes > CMD_NON_FATAL_ERROR_MAX_RETRY_TIMES)) || \
         ((pCmdEntryTiny->ErrStatus == cCETinyStsPoorSGLErr) && (pCmdEntryTiny->RetryTimes > CMD_POOR_SGL_MAX_RETRY_TIMES))         || \
         ((pCmdEntryTiny->ErrStatus == cCETinyStsFatalErr) && (pCmdEntryTiny->RetryTimes > CMD_FATAL_ERROR_MAX_RETRY_TIMES))
         ))
    {
        exceedFlag = true;
    }

    return exceedFlag;
}
