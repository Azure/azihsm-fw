// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2MessageHandler.cpp
//! @brief  FpsCpu2 Message Handler
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2.h"
#include "MemIo.h"
#include "SysTypes.h"
#include "LoggingDebug.h"
extern "C"
{
#include "irq.h"
#include "vicommon.h"
}

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

bool fpsCpu2::HandleTeardownSlotSts(uint8_t VFId)
{
    VFNodeInfo_t* pVFNodeInfo = &_pVfInfoBase[VFId];
    uint8_t qbIndex = 0;
    uint8_t sqPId = 0;
    uint64_t queueBlkBitMap;

    queueBlkBitMap = pVFNodeInfo->queueBlkBitMap;
    if (queueBlkBitMap)
    {
        for (qbIndex = FindNextBit64(queueBlkBitMap); (queueBlkBitMap != 0); \
             queueBlkBitMap &= ~(BIT_ULL(qbIndex)), qbIndex = FindNextBit64(queueBlkBitMap))
        {
            sqPId = QBIDX_2_HIGH_SQ_PID(qbIndex);
            if (_pSlotFlagSts[sqPId] & cStsTearDown)
            {
                return true;
            }

            if (_pSlotFlagSts[sqPId] & cStsValid)
            {
                _pSlotFlagSts[sqPId] |= cStsTearDown;
            }

            sqPId = QBIDX_2_LOW_SQ_PID(qbIndex);
            if (_pSlotFlagSts[sqPId] & cStsTearDown)
            {
                return true;
            }

            if (_pSlotFlagSts[sqPId] & cStsValid)
            {
                _pSlotFlagSts[sqPId] |= cStsTearDown;
            }

            DMB();
        }
    }

    queueBlkBitMap = (uint64_t)pVFNodeInfo->queueBlk65BitMap;
    if (queueBlkBitMap)
    {
        if (_pSlotFlagSts[QB65_HIGH_PHYSICAL_Q_INDEX] & cStsTearDown)
        {
            return true;
        }

        if (_pSlotFlagSts[QB65_HIGH_PHYSICAL_Q_INDEX] & cStsValid)
        {
            _pSlotFlagSts[QB65_HIGH_PHYSICAL_Q_INDEX] |= cStsTearDown;
        }

        if (_pSlotFlagSts[QB65_LOW_PHYSICAL_Q_INDEX] & cStsTearDown)
        {
            return true;
        }

        if (_pSlotFlagSts[QB65_LOW_PHYSICAL_Q_INDEX] & cStsValid)
        {
            _pSlotFlagSts[QB65_LOW_PHYSICAL_Q_INDEX] |= cStsTearDown;
        }

        DMB();
    }

    return false;

}

uint64_t fpsCpu2::GetVFBitmap(uint8_t VFId)
{
    uint64_t enBitmap = (VFId == MAX_VF_NUM) ? (uint64_t)(readl(_pVF65EnBitmap)) : readq(_pVFEnBitmap);
    return enBitmap;
}

bool fpsCpu2::ChkCPtoFPMsgFiberDone(void)
{
    uint32_t msgCP0toFP2ReqPi = readl(CP0toFPReqMsg.pMsgPi);
    uint32_t msgCP0toFP2ReqCi = CP0toFPReqMsg.localMsgCi;

    uint32_t msgCP1toFP2ReqPi = readl(CP1toFPReqMsg.pMsgPi);
    uint32_t msgCP1toFP2ReqCi = CP1toFPReqMsg.localMsgCi;

    uint32_t msgCP0toFP2ResPi = readl(CP0toFPResMsg.pMsgPi);
    uint32_t msgCP0toFP2ResCi = CP0toFPResMsg.localMsgCi;

    uint32_t msgCP1toFP2ResPi = readl(CP1toFPResMsg.pMsgPi);
    uint32_t msgCP1toFP2ResCi = CP1toFPResMsg.localMsgCi;

    volatile bool retval = true;
    if ((msgCP0toFP2ReqPi != msgCP0toFP2ReqCi) ||   //for req from CP0
        (msgCP1toFP2ReqPi != msgCP1toFP2ReqCi) ||   //for req from CP1
        (msgCP0toFP2ResPi != msgCP0toFP2ResCi) ||   //for res from CP0
        (msgCP1toFP2ResPi != msgCP1toFP2ResCi))     //for res from CP1
    {
        retval =  false;
    } // else do nothing
    return retval;
}

bool fpsCpu2::ChkRecvFPMsgFiberDone(void)
{
    uint32_t msgCPU0toCPU2Pi = readl(pCPU0toCPU2Pi);
    uint32_t msgCPU0toCPU2Ci = CPU0toCPU2Ci;

    uint32_t msgCPU1toCPU2Pi = readl(pCPU1toCPU2Pi);
    uint32_t msgCPU1toCPU2Ci = CPU1toCPU2Ci;
    if (msgCPU0toCPU2Pi != msgCPU0toCPU2Ci) // for req from CPU0
    {
        return false;
    } // else do nothing
    if (msgCPU1toCPU2Pi != msgCPU1toCPU2Ci) //for resp from CPU1
    {
        return false;
    } // else do nothing
    return true;
}

void fpsCpu2::MsgNotSupport(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    //no such case
    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("cp to fp msg: msg op out of range, msgOp:0x%X\n", (pMsgSQContext->msgOp & 0xFF)), "32");

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateNotSupportStart:
        {
            pMsgState[cp2FpMsgIdx] = stateNotSupportSendFP2CP;
        }
        case stateNotSupportSendFP2CP:
        {
            uint8_t sts = 0;
            pMsgSQContext->sts = msgNotSupport;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            else
            {
                break; //
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
            break;
    }
}

CP2FPMsgSts fpsCpu2::RecvFPMsg(FPInterMsgHeader* pFpMsgHeader, M7CoreId_t msgCpu)
{
    CP2FPMsgSts fpSts = (CP2FPMsgSts)pFpMsgHeader->sts;
    FpInterMsgOp fpMsgOp = (FpInterMsgOp)pFpMsgHeader->fpMsgOp;
    CP2FPMsgSts recvSts = msgSuccess;
    switch (fpMsgOp)
    {
        case cp2FpMsg:
        {
            // recv resp from other cpu
            uint8_t msgIdx = pFpMsgHeader->msgIdx;;
            CPCoreId_t msgSrc = (CPCoreId_t)pFpMsgHeader->msgSrc;
            CP2FPMessageInfo* pCp2FPMsgInfo = NULL;

            CP2FPMsgContext_t* pCP2FPMsg = NULL;
            // pCP2FPMsg = (CP2FPMsgContext_t*)(((uint32_t)CP0toFPReqMsg.pMsgQ) + (msgIdx * PSRAM_FP2CP_MSG_ELMNT_SIZE));
            switch (msgSrc)
            {
                case CP0:
                    pCp2FPMsgInfo = (CP2FPMessageInfo*)(&CP0toFPReqMsg);
                    break;
                case CP1:
                    pCp2FPMsgInfo = (CP2FPMessageInfo*)(&CP1toFPReqMsg);
                    break;
                default:
                    return msgInvalidField;
            }
            pCP2FPMsg = (CP2FPMsgContext_t*)((pCp2FPMsgInfo->pMsgQ)) + msgIdx;

            HandleFPRespMsgSts(pCp2FPMsgInfo, pCP2FPMsg, msgIdx, fpSts, msgCpu);
            recvSts = (CP2FPMsgSts)pCP2FPMsg->sts;
            (this->*pfCpu2MsgTable[pCP2FPMsg->msgOp])(pCp2FPMsgInfo, pCP2FPMsg, msgIdx, pCp2FPMsgInfo->msgState, msgSrc);
            break;
        }
        case errorCmdMsg:
        {
            recvSts = msgNotSupport;
            break;
        }
        case fatalErrorMsg:
        {
            #ifndef TDD
            if (HalCDMA_GetInterruptCause() & CDMA_INT_CAUSE_FATAL_ERROR)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("RecvFPMsg, InterruptCause[0x%X]\n", (uint32_t)HalCDMA_GetInterruptCause()), "32");
                HalCDMA_DisableSQ();
                HalCDMA_DisableCQ();
            }
            #endif
            _cdmaFatalErrorHandleState = cFatalErrorHandlingWaitHandleCompletion;
            break;
        }
        #ifdef QOS_LATENCY_ERROR_HANDLING
        case qosPenaltyMsg:
        {
            // recv resp from other cpu, CPU2 un-mark its local penalty VF bitmap
            uint16_t ceIdx = (pFpMsgHeader->skipAbort_ceIndex);
            uint16_t caIdx = (ceIdx >> CA_SIZE_SHIFT);
            uint16_t phyIbqId = pCa2IbPhysicalId[caIdx];
            uint8_t qbIndex = SQ_PID_2_QBIDX(phyIbqId);
            QueueBlockInfo_t* pQBlockInfo = &this->_pQueueBlockInfoBase[qbIndex];
            uint8_t vfId = pQBlockInfo->vfId;
            if (vfId == MAX_VF_NUM)
            {
                this->_qosPenaltyVf65Bitmap = 0;
            }
            else
            {
                this->_qosPenaltyVfBitmap &= ~(BIT_ULL(vfId));
            }
            break;
        }
        #endif
        case ConfigIOMsg:
        {
            if ((ConfigIOStatus_t)pFpMsgHeader->configIo == cPauseIO)
            {
                _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWaitCDMAIdle;
            }
            else
            {
                _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingDone;
            }

            break;

        }

        case resetHandlingMsg:
        {
            if (_resetHandlingState == cResetHandlingWaitCPU0Resp)
            {
                _resetHandlingState = cResetHandlingNotifyCPU1;
            }
            else
            {
                _resetHandlingState = cResetHandlingWaitTeardownDone;
            }
            break;
        }

        default:
            recvSts = msgNotSupport;
            break;
    }
    return recvSts;
}

CP2FPMsgSts fpsCpu2::SendFPMsg(M7CoreId_t cpu, uint8_t fpMsgOp, uint8_t resp, CP2FPMsgSts fpSts, uint8_t cmdSpecific0, uint8_t cmdSpecific1)
{
    CP2FPMsgSts sts = msgSuccess;
    FPInterMsgHeader* pFPMsgHeader;
    volatile uint32_t* pMsgPi, * pMsgCi;
    uint32_t msgPi, msgCi;
    FPInterMsgHeader fpInterMsgTmpHeader;
    fpInterMsgTmpHeader.fpMsgOp = fpMsgOp;
    fpInterMsgTmpHeader.resp = resp;
    fpInterMsgTmpHeader.sts = fpSts;
    fpInterMsgTmpHeader.cmdSpecific[0] = cmdSpecific0;
    fpInterMsgTmpHeader.cmdSpecific[1] = cmdSpecific1;
    switch (cpu)
    {
        case cM7Core0:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU2toCPU0MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU2toCPU0Pi;
            pMsgCi = (volatile uint32_t*)pCPU2toCPU0Ci;
            CPU2toCPU0Pi = readl(pMsgPi);
            msgPi = CPU2toCPU0Pi;
        }
        break;
        case cM7Core1:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU2toCPU1MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU2toCPU1Pi;
            pMsgCi = (volatile uint32_t*)pCPU2toCPU1Ci;
            CPU2toCPU1Pi = readl(pMsgPi);
            msgPi = CPU2toCPU1Pi;
        }
        break;
        default:
        {
            return msgNotSupport;
        }
        break;
    }
    msgCi = readl(pMsgCi);

    if (M7_QUEUE_FULL(msgPi, msgCi, PSRAM_INTL_CPUX2CPUY_MSG_MASK))    //chk fp msg Q full wait q space
    {
        return msgNoEmptyEntry;
    } // else do nothing
    M7_MEM_COPY(&pFPMsgHeader[msgPi], &fpInterMsgTmpHeader, sizeof(FPInterMsgHeader));
    DMB();
    msgPi = M7_QUEUE_INC(msgPi, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
    writel(msgPi, pMsgPi);

    sts = fpSts;
    switch (cpu)
    {
        case cM7Core0:
        {
            CPU2toCPU0Pi = msgPi;
        }
        break;
        case cM7Core1:
        {
            CPU2toCPU1Pi = msgPi;
        }
        break;
        default:
            break;
    }
    #ifdef IPC_SUPPORT
    switch (cpu)
    {
        case cM7Core0:
        {
            IpcDescTrigger(CPU2toCPU0_DESC, msgPi);
        }
        break;
        case cM7Core1:
        {
            IpcDescTrigger(CPU2toCPU1_DESC, msgPi);
        }
        break;
        default:
            break;
    }
    #endif //IPC_SUPPORT

    return sts;
}

CP2FPMsgSts fpsCpu2::FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone)
{
    if (change)
    {
        //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogInfo, ("FP CPU 2 Status Change requested. changeStatus [0x%x]\n", changeStatus), "32");
        switch (changeStatus)
        {
            case FP_STS_NORMAL_BOOT:
            {
                IpcDescTrigger(UPDATE_TIMESTAMP_CPU2_TO_3CPU, IPC_TRIGGER_VAL);

                if(gResetType == cPor)
                {
                    FpsCpuNormalBootInitialize();
                }
                break;
            }
            case FP_STS_FP_START:
            {
                break;
            }
            default:
            {
                *pDone = false;
                break;
            }
        }
    } // else do nothing
    if (*pDone)
    {
        writel(changeStatus, pCpuStatus);
        DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("FP CPU 2 Status Change Completed. Status [0x%x]\n", readl(pCpuStatus)), "32");
    }
    else
    {
        return msgInvalidField;
    }
    return msgSuccess;
}

void fpsCpu2::HandleFPRespMsgSts(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pCP2FPMsg, uint8_t cp2FpMsgIdx, uint8_t fpSts, M7CoreId_t msgCpu)
{
    uint8_t sts;
    sts = pCP2FPMsg->sts;   ///< get the CP2FP Msg sts
    pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap |= MSG_CPU(msgCpu);
    switch (sts)//original status in cp2fp Q
    {
        case msgSuccess:   ///< change the CP2FP Msg sts as Internal Msg sts
        case msgInvalidField:
        case msgVfInstalledAlready:
            sts = fpSts;
            pCP2FPMsg->sts = sts;
            break;
        default:
            break;
    }
}

void fpsCpu2::HandleCP2FPMsg(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    uint8_t msgOp;
    uint8_t tablesz = ARRAY_SIZE(pfCpu2MsgTable);
    //CP2FPMsgSts MsgSts = (CP2FPMsgSts)pMsgSQContext->sts;
    msgOp = pMsgSQContext->msgOp;

    if (msgOp < tablesz)
    {
        (this->*pfCpu2MsgTable[msgOp])(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
    }
    else
    {
        MsgNotSupport(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
    }

    return;
}

void fpsCpu2::ChkUpdateCptoFpMsgCi(CP2FPMessageInfo* pCp2FPMsgInfo, CPMsgQId msgQId)
{
    CP2FPMsgContext_t* pCPtoFPMsgQTemp;
    uint32_t msgPi = pCp2FPMsgInfo->localMsgPi;
    uint32_t msgCiTemp = pCp2FPMsgInfo->localMsgCi;
    uint8_t* pMsgState =  pCp2FPMsgInfo->msgState;
    uint8_t modifyCi = false;
    while (msgPi != msgCiTemp)
    {
        pCPtoFPMsgQTemp =  (CP2FPMsgContext_t*)(((uint32_t)pCp2FPMsgInfo->pMsgQ) + PSRAM_CP2FP_MSG_ELMNT_SIZE * msgCiTemp);
        if (pMsgState[msgCiTemp] == ALL_MSG_STATE_DONE)
        {
            modifyCi = true;
            pMsgState[msgCiTemp] = MSG_STATE_START;
        }
        else
        {
            //msgCiTemp = M7_QUEUE_INC(msgCiTemp, PSRAM_CP2FP_MSG_MASK);
            break;
        }
        if(msgQId == CP0ToFP_Req || msgQId == CP1ToFP_Req)
        {
            msgCiTemp = M7_QUEUE_INC(msgCiTemp, PSRAM_CP2FP_MSG_MASK);
        }
        else
        {
            msgCiTemp = M7_QUEUE_INC(msgCiTemp, PSRAM_CP2FP_REQ_RES_MSG_MASK);
        }
    }
    if (modifyCi)
    {
        pCp2FPMsgInfo->localMsgCi = msgCiTemp;
        writel(msgCiTemp, pCp2FPMsgInfo->pMsgCi);//update ci
    } // else do nothing
}

CP2FPMsgSts fpsCpu2::SendFP2CPMsg(CP2FPMsgContext_t* pMsgInfo, uint8_t* pData, uint8_t resp, uint8_t length, CPMsgQId msgCP) // data may be resp or req
{
    FP2CPMessageInfo* pFP2CPMsgInfo = NULL;
    uint32_t msgFp2CpMsgCi;
    IPC_DESC_t Fp2CpDesc;

    switch (msgCP)
    {
        case FPToCP0_Req:
            pFP2CPMsgInfo = &FPtoCP0ReqMsg;
            Fp2CpDesc = FPtoCP0_REQ_DESC;
            break;
        case FPToCP1_Req:
            pFP2CPMsgInfo = &FPtoCP1ReqMsg;
            Fp2CpDesc = FPtoCP1_REQ_DESC;
            break;

        case FPToCP0_Res:
            pFP2CPMsgInfo = &FPtoCP0ResMsg;
            Fp2CpDesc = FPtoCP0_RES_DESC;
            break;
        case FPToCP1_Res:
            pFP2CPMsgInfo = &FPtoCP1ResMsg;
            Fp2CpDesc = FPtoCP1_RES_DESC;
            break;
        default:
            return msgInvalidField;
    }

    msgFp2CpMsgCi = readl(pFP2CPMsgInfo->pMsgCi);
    //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogDebug, ("SendFP2CPMsg pi[0x%X] ci[0x%X]\n", pFP2CPMsgInfo->localMsgPi | (msgFp2CpMsgCi << 0x10UL)), "16,16");
    CP2FPMsgContext_t* tmpFP2CPMsg;

    if(msgCP == FPToCP0_Res || msgCP == FPToCP1_Res)
    {
        if (M7_QUEUE_FULL(pFP2CPMsgInfo->localMsgPi, msgFp2CpMsgCi, PSRAM_FP2CP_MSG_MASK))
        {
            return msgNoEmptyEntry;
        }
    }
    else // FPToCP0_Req and FPToCP1_Req
    {
        if (M7_QUEUE_FULL(pFP2CPMsgInfo->localMsgPi, msgFp2CpMsgCi, PSRAM_FP2CP_REQ_RES_MSG_MASK))
        {
            return msgNoEmptyEntry;
        }
    }

    tmpFP2CPMsg = (CP2FPMsgContext_t*)(pFP2CPMsgInfo->pMsgQ) + (pFP2CPMsgInfo->localMsgPi);
    M7_MEM_COPY(tmpFP2CPMsg, pMsgInfo, sizeof(CP2FPMsgContext_t));//copy message
    tmpFP2CPMsg->resp = resp;
    tmpFP2CPMsg->length = length;

    M7_MEM_COPY(tmpFP2CPMsg->data, pData, tmpFP2CPMsg->length);//copy data

    // trigger CP
    if(msgCP == FPToCP0_Res || msgCP == FPToCP1_Res)
    {
        pFP2CPMsgInfo->localMsgPi = M7_QUEUE_INC(pFP2CPMsgInfo->localMsgPi, PSRAM_FP2CP_MSG_MASK);
    }
    else //FPToCP0_Req and FPToCP1_Req
    {
        pFP2CPMsgInfo->localMsgPi = M7_QUEUE_INC(pFP2CPMsgInfo->localMsgPi, PSRAM_FP2CP_REQ_RES_MSG_MASK);
    }
    writel(pFP2CPMsgInfo->localMsgPi, pFP2CPMsgInfo->pMsgPi);

    IpcDescTrigger(Fp2CpDesc, pFP2CPMsgInfo->localMsgPi);

    return ((CP2FPMsgSts)pMsgInfo->sts);
}

bool fpsCpu2::ChkFpVfUpdate(CP2FPMsgContext_t* pMsgSQContext)
{
    bool msgIsFail = false;
    CP2FPMsgDataVfUpdate_t* ptmpData = (CP2FPMsgDataVfUpdate_t*)pMsgSQContext->data;
    uint8_t VFId = MAP_FUNCTION_ID(ptmpData->VFId);
    uint8_t bitIdx = GET_VF_IDX(VFId);
    uint64_t EnBitmap = GetVFBitmap(VFId);

    if (VFId >= MAX_SUPPORT_FUNC_NUM)
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf id out of range, vfId:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");

        // MSFT request, teardown action should not retrun error sts?
        if (ptmpData->Action == cActionVfInstall)
        {
            pMsgSQContext->sts = msgVfOutOfRange;
        }
        msgIsFail = true;
        return msgIsFail;
    }

    switch (ptmpData->Action)
    {
        case cActionVfInstall:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("Create VF request received (0x%X)\n", VFId), "32");
            if (EnBitmap & BIT_ULL(bitIdx))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf installed already, vfId:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
                pMsgSQContext->sts = msgVfInstalledAlready;
                msgIsFail = true;
            } // else do nothing
            break;
        }

        case cActionTearDown:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("Teardown VF request received (0x%X)\n", VFId), "32");
            if (!(EnBitmap & BIT_ULL(bitIdx)))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf not installed, vfId:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
                pMsgSQContext->sts = msgSuccess; // According to msft request
                msgIsFail = true;
            }
            else
            {
                if (HandleTeardownSlotSts(VFId) == true)
                {
                    break;
                }

                #ifdef QOS_LATENCY_ERROR_HANDLING
                uint8_t vfGroupIndex = VFId >> 5;
                switch (vfGroupIndex)
                {
                    case VF0_VF31:
                    case VF32_VF63:
                    {
                        this->_qosPenaltyVfBitmap &= (~(BIT_ULL(VFId)));
                        break;
                    }
                    case VF64:
                    {
                        this->_qosPenaltyVf65Bitmap = 0;
                        break;
                    }
                    default:
                        break;
                }
                #endif
            }
            break;
        }

        default: // no such action
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid action, action:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (ptmpData->Action & 0xFF))), "24,8");
            pMsgSQContext->sts = msgInvalidField;
            msgIsFail = true;
            break;
        }
    }

    return msgIsFail;

}

bool fpsCpu2::ChkFpVfSlotSq2CqMapUpdate(CP2FPMsgContext_t* pMsgSQContext)
{
    CP2FPMsgDataVfSlotSq2CqMapUpdate_t* ptmpData = (CP2FPMsgDataVfSlotSq2CqMapUpdate_t*)pMsgSQContext->data;
    bool msgIsFail = false;
    uint8_t VFId = MAP_FUNCTION_ID(ptmpData->VFId);
    uint8_t sqPId = ptmpData->SqPId;
    uint8_t qbIdx = SQ_PID_2_QBIDX(sqPId);

    // check if host id and queue id are valid
    if ((VFId > MAX_VF_NUM))
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf id out of range, vfId:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
        pMsgSQContext->sts = msgVfOutOfRange;
        msgIsFail = true;
        return msgIsFail;
    }

    if ((sqPId >= UCD_FP_IO_Q_NUM))
    {
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("sqpid out of range, sqPId:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (sqPId & 0xFF))), "24,8");
        pMsgSQContext->sts = msgQueueOutOfRange;
        msgIsFail = true;
        return msgIsFail;
    }

    // check if valid host id is already installed
    if (VFId == MAX_VF_NUM)
    {
        uint32_t Vf65EnableBitMap = readl(_pVF65EnBitmap);
        if (!Vf65EnableBitMap)
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf not installed, vfId:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
            pMsgSQContext->sts = msgVfNotInstalled;
            msgIsFail = true;
            return msgIsFail;
        } // else do nothing
    }
    else
    {
        uint64_t VfEnableBitMap = *(_pVFEnBitmap);
        if (!(VfEnableBitMap & BIT_ULL(VFId)))
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf not installed, vfId:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
            pMsgSQContext->sts = msgVfNotInstalled;
            msgIsFail = true;
            return msgIsFail;
        } // else do nothing
    }

    switch (ptmpData->Action)
    {
        case cActionCreate:
        {
            if (ptmpData->CqPId >= UCD_FP_IO_Q_NUM)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("cqpid out of range, vfId:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (ptmpData->CqPId & 0xFF))), "24,8");
                pMsgSQContext->sts = msgQueueOutOfRange;
                msgIsFail = true;
                break;
            }

            if ((_pIbQ2ObQ[sqPId] != QID_INVALID) || (_pSlotFlagSts[sqPId] != cStsInit))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("q installed already, sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x8UL) | (ptmpData->SqPId & 0xFF))), "8,16,8");
                pMsgSQContext->sts = msgQueueInstalledAlready;
                msgIsFail = true;
                break;
            } // else do nothing

            // check host id is the same as the currently VF/VM or not
            if ((_pQueueBlockInfoBase[qbIdx].vfId != VFID_INV) && (_pQueueBlockInfoBase[qbIdx].vfId != VFId))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("q installed already, vfId:0x%X sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x10UL) | ((ptmpData->SqPId & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,8,8,8");
                pMsgSQContext->sts = msgInvalidField;
                msgIsFail = true;
                break;
            } // else do nothing

            break;
        }

        case cActionRemove:
        {
            if ((_pQueueBlockInfoBase[qbIdx].vfId != VFId) || !(_pSlotFlagSts[sqPId] & cStsValid))
            {
                if (!(_pSlotFlagSts[sqPId] & cStsValid))
                {
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("q not installed, sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x8UL) | (sqPId & 0xFF))), "8,16,8");
                    pMsgSQContext->sts = msgQueueNotInstalled;
                }
                else
                {
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("mismatch vf id between msg data and corr qb info, vfId:0x%X sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x10UL) | ((ptmpData->SqPId & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,8,8,8");
                    pMsgSQContext->sts = msgInvalidField;
                }
                msgIsFail = true;
            }
            else
            {
                _pSlotFlagSts[sqPId] |= cStsDelete;
            }
            break;
        }

        case cActionForceCompletion:
        {
            if ((_pQueueBlockInfoBase[qbIdx].vfId != VFId) || !(_pSlotFlagSts[sqPId] & cStsValid))
            {
                if (!(_pSlotFlagSts[sqPId] & cStsValid))
                {
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("q not installed, sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x8UL) | (sqPId & 0xFF))), "8,16,8");
                    pMsgSQContext->sts = msgQueueNotInstalled;
                }
                else
                {
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("mismatch vf id between msg data and corr qb info, vfId:0x%X sqpid:0x%X action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | ((ptmpData->Action & 0xFF) << 0x10UL) | ((ptmpData->SqPId & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,8,8,8");
                    pMsgSQContext->sts = msgInvalidField;
                }
                msgIsFail = true;
            }
            else
            {
                _pSlotFlagSts[sqPId] |= cStsForceCompletion;
            }
            break;
        }
        default: // no such action
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid action, action:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (ptmpData->Action & 0xFF))), "24,8");
            pMsgSQContext->sts = msgInvalidField;
            msgIsFail = true;
            break;
        }
    }

    return msgIsFail;

}
#ifdef LIONPERF_SUPPORT
void fpsCpu2::HandleOpSetLogLevel(CP2FPMsgContext_t* pMsgSQContext)
{
    uint8_t data = (uint8_t)(pMsgSQContext->data[0]);
    LogExt_t _logExt;
    _logExt.LevelsEnabledAtRunTime = (LogLevel_t)data;
    LoggingUpdateLogExt(&_logExt, cLogExtSetLevelsEnabledAtRunTime);
}
uint32_t fpsCpu2::CalCheckSum(uint32_t* pData, uint32_t len, uint8_t check)
{
    uint32_t sum = 0;
    uint32_t* p = (uint32_t*)pData;
    uint32_t i = 0;
    for (i = 0; i < (len / 4); i++)
    {
        sum += p[i];
    }

    if (check)
    {
        return sum;
    } // else do nothing

    sum = (~sum) + 1;
    return sum;
}

void fpsCpu2::InitBackupDataHeader(FwUpdateDataHeader* pDataHeader)
{

    pDataHeader->signature = FW_UPDATE_SIGNATURE;
    pDataHeader->dataBlkCnt = FW_UPDATE_BACKUP_DATA_BLK_CNT;
    pDataHeader->totalDataLength = (PSRAM_DATA_BACKUP_HEADER_SIZE + (PSRAM_DATA_BACKUP_LENGTH_SIZE * FW_UPDATE_BACKUP_DATA_BLK_CNT)  + \
                                    PSRAM_DFL_BACKUP_SIZE + PSRAM_LOGGING_BACKUP_SIZE);
    pDataHeader->checkSum = 0;
}

void fpsCpu2::BackupDFLInfo()
{
    uint32_t* pUcdBackupInfoLength = (uint32_t*)(PSRAM_DFL_LIST_BACKUP_LENGTH_ADDR);
    uint32_t DflListVirAddr0 = (uint32_t)(CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU0_DFL_LIST_0_ADDR));
    uint64_t* pEntries = (uint64_t*)DflListVirAddr0;
    uint16_t* dflCopyBase = (uint16_t*)(PSRAM_DFL_LIST_BACKUP);
    *pUcdBackupInfoLength = PSRAM_DFL_BACKUP_SIZE;
    for (uint16_t i = 0; i < UCD_DFL_Q_SIZE; i++)
    {
        uint64_t dflAddr = readq(&pEntries[i]);
        uint16_t dflIdx = (dflAddr - (getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR))) >> (uint8_t)DFL_BUF_SZ_SHIFT;
        dflCopyBase[i] = dflIdx;
    }

    uint32_t DflListVirAddr1 = (uint32_t)(CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU0_DFL_LIST_1_ADDR));
    pEntries = (uint64_t*)DflListVirAddr1;
    for (uint16_t i = 0; i < UCD_DFL_1_Q_SIZE; i++)
    {
        uint64_t dflAddr = readq(&pEntries[i]);
        uint16_t dflIdx = (dflAddr - (getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR))) >> (uint8_t)DFL_BUF_SZ_SHIFT;
        dflCopyBase[i + UCD_DFL_Q_SIZE] = dflIdx;
    }
}
#endif

#ifdef LIONPERF_SUPPORT
void fpsCpu2::BackupLoggingInfo()
{
    // Note: when cpu2 is backing up, cpu0/1 may still have timestamp logging.
    // log buffer info and GDMA pi are stored in SHARE DTCM,
    // so there may be a crash issue when cpu2 R, cpu0/1 W, in the same time, due to SHARE DTCM dual channel access limitation.
    // cpu2 access SHARE DTCM through cpu0/1 can solve this issue.
    uint32_t* pLoggingBackInfoLength = (uint32_t*)PSRAM_LOGGING_BACKUP_LENGTH_ADDR;
    *pLoggingBackInfoLength = PSRAM_LOGGING_BACKUP_SIZE;
    #ifdef RESTORE_LOG_INFO_FROM_PSRAM_FOR_CPU0_CPU2
    // cpu0 (share 0/1)
    M7_MEM_COPY((uint8_t*)(LOG_BUFFER_INFO_BASE_ADDRESS + (LOG_BUFFER_INFO_SIZE * cM7Core0)), (uint8_t*)(CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU01_LOG_BUF_INFO_ADDR)), sizeof(struct LogBufferInfo_t));
    // cpu2 (share 2/0)
    M7_MEM_COPY((uint8_t*)(LOG_BUFFER_INFO_BASE_ADDRESS + (LOG_BUFFER_INFO_SIZE * cM7Core2)), (uint8_t*)(M7_FPS_CPU20_LOG_BUF_INFO_ADDR), sizeof(struct LogBufferInfo_t));
    #endif
    // cpu1 (share 1/2) must backup to psram
    M7_MEM_COPY((uint8_t*)(LOG_BUFFER_INFO_BASE_ADDRESS + (LOG_BUFFER_INFO_SIZE * cM7Core1)), (uint8_t*)(CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_LOG_BUF_INFO_ADDR)), sizeof(struct LogBufferInfo_t));
    // backup current gdma dq pi
    *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core0) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_CURRENT_PI_OFFSET) = *(uint32_t*)(CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU01_LOG_GDMA_DQ_PI));
    *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core1) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_CURRENT_PI_OFFSET) = *(uint32_t*)(CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_LOG_GDMA_DQ_PI));
    *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core2) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_CURRENT_PI_OFFSET) = *(uint32_t*)(M7_FPS_CPU20_LOG_GDMA_DQ_PI);
}

void fpsCpu2::GetFwUpdateInfo(FWupdateBackupInfo* pFWupdateInfo, RecoverDataBlk dataType)
{
    FwUpdateDataHeader* pDataHeader = (FwUpdateDataHeader*)PSRAM_BACKUP_DATA_HEADER_ADDR;
    if (pDataHeader->signature != FW_UPDATE_SIGNATURE) // this will be from old structure to new
    {
        pFWupdateInfo->sts = cNoSignature;
        return;
    }

    FwUdSts FwSts = cNewVer;
    // chk data blk cnt
    uint32_t blkcnt = (uint32_t)dataType + 1;
    uint32_t blkIdx = (uint32_t)cUCDData;
    uint32_t HeaderStartAddr = (uint32_t)(PSRAM_BACKUP_DATA_HEADER_ADDR);
    uint32_t dataOffset = (uint32_t)(PSRAM_DATA_BACKUP_HEADER_SIZE);
    uint32_t dataLen = 0;
    uint32_t dataLenSize = (uint32_t)PSRAM_DATA_BACKUP_LENGTH_SIZE;

    // blk 0
    for (blkIdx = (uint32_t)cUCDData; blkIdx < blkcnt; blkIdx++)
    {
        switch (blkIdx)
        {
            case cUCDData: // UCD data
            {
                uint32_t dataLenAddr = (uint32_t)(HeaderStartAddr + dataOffset);
                dataLen = readl(dataLenAddr);
                uint32_t newDataLen = (uint32_t)PSRAM_DFL_BACKUP_SIZE;
                if (dataLen != newDataLen)
                {
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("fw update: data len mismatch, blkIdx:0x%X\n", cUCDData), "32");
                }
                dataOffset += PSRAM_DATA_BACKUP_LENGTH_SIZE;
            }
            break;

            case cLoggingData: // logging data
            {
                uint32_t dataLenAddr = (uint32_t)(HeaderStartAddr + dataOffset);
                dataLen = readl(dataLenAddr);

                if (dataType == cLoggingData)
                {
                    uint32_t newDataLen = (uint32_t)PSRAM_LOGGING_BACKUP_SIZE;
                    if (dataLen != newDataLen)
                    {
                       //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("fw update: data len mismatch, blkIdx:0x%X\n", cLoggingData), "32");
                    }
                    if (dataLen > newDataLen)
                    {
                        FwSts = cOldVer;
                    }
                    pFWupdateInfo->addr = (uint32_t)(dataLenAddr + dataLenSize);
                    pFWupdateInfo->length = dataLen;
                    pFWupdateInfo->sts = FwSts;
                }
                dataOffset += PSRAM_LOGGING_BACKUP_LENGTH_SIZE;
            }
            break;

            default:
                break;
        }

        dataOffset += (dataLen);

    }
}
#endif