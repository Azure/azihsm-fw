// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpSCpu0MessageHandler.cpp
//! @brief  FpSCpu0 Message Handler
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu0.h"
extern "C"
{
#ifdef MCR_TEST_HOOKS
#include "crashdump.h"
#endif
}
//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

#define GET_IBCQ_INDEX_FROM_SQ_PID(sqPid)  (sqPid == 64 || sqPid == 129) ? IBCQ_0 : IBCQ_1
// Little Func for VF 65 modification start
bool fpsCpu0::ChkQBlkBitmap(VFNodeInfo_t* pVFNodeInfo)
{
    uint64_t queueBlkBitMap = pVFNodeInfo->queueBlkBitMap;
    bool bitmapIsClean = false;
    uint64_t queueBlk65BitMap = pVFNodeInfo->queueBlk65BitMap;
    if (!queueBlkBitMap && !queueBlk65BitMap)
    {
        bitmapIsClean =  true;
    }// else do nothing
    return bitmapIsClean;

}
void fpsCpu0::HandleTeardown(uint8_t VFId)
{
    VFNodeInfo_t* pVFNodeInfo = &_pVfInfoBase[VFId];
    uint8_t qbIndex = 0;
    uint8_t sqPId = 0;
    uint64_t queueBlkBitMap;

    //scan bit map
    if (ChkQBlkBitmap(pVFNodeInfo))
    {
        ClearVFSetting(pVFNodeInfo);
    }// else do nothing

    queueBlkBitMap = pVFNodeInfo->queueBlkBitMap;
    if (queueBlkBitMap)
    {
        for (qbIndex = FindNextBit64(queueBlkBitMap); (queueBlkBitMap != 0); \
             queueBlkBitMap &= ~(BIT_ULL(qbIndex)), qbIndex = FindNextBit64(queueBlkBitMap))
        {
            sqPId = QBIDX_2_HIGH_SQ_PID(qbIndex);
            if (_pSlotFlagSts[sqPId] & cStsValid)
            {
                IbCqRefillDFL(sqPId);
                HandleDeleteVFQblk(sqPId, VFId, cStsTearDown);
            }

            sqPId = QBIDX_2_LOW_SQ_PID(qbIndex);
            if (_pSlotFlagSts[sqPId] & cStsValid)
            {
                IbCqRefillDFL(sqPId);
                HandleDeleteVFQblk(sqPId, VFId, cStsTearDown);
            }

            DMB();
        }
    }

    queueBlkBitMap = (uint64_t)pVFNodeInfo->queueBlk65BitMap;
    if (queueBlkBitMap)
    {
        if (_pSlotFlagSts[QB65_HIGH_PHYSICAL_Q_INDEX] & cStsValid)
        {
            IbCqRefillDFL(QB65_HIGH_PHYSICAL_Q_INDEX);
            HandleDeleteVFQblk(QB65_HIGH_PHYSICAL_Q_INDEX, VFId, cStsTearDown);
        }

        if (_pSlotFlagSts[QB65_LOW_PHYSICAL_Q_INDEX] & cStsValid)
        {
            IbCqRefillDFL(QB65_LOW_PHYSICAL_Q_INDEX);
            HandleDeleteVFQblk(QB65_LOW_PHYSICAL_Q_INDEX, VFId, cStsTearDown);
        }

        DMB();
    }
}

void fpsCpu0::_InitDFLList()
{
    // Initialize DFL buffer
    // DFL 0
    uint32_t DflListVirAddr = (uint32_t)M7_FPS_CPU0_DFL_LIST_0_ADDR;
    uint64_t* pEntries = (uint64_t*)DflListVirAddr;
    uint64_t DflBuffAddr = (uint64_t)(getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR));
    uint64_t cmdAddr = (0x00000000FFFFFFFFULL & DflBuffAddr);

    for (uint32_t i = 0; i < UCD_DFL_Q_SIZE; i++)
    {
        writeq(cmdAddr, &pEntries[i]);
        cmdAddr += DFL_BUFF_ELMNT_SIZE;
    }

    // DFL 1
    DflListVirAddr = (uint32_t)M7_FPS_CPU0_DFL_LIST_1_ADDR;
    pEntries = (uint64_t*)DflListVirAddr;
    DflBuffAddr = (uint64_t)(getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR));
    cmdAddr = (0x00000000FFFFFFFFULL & DflBuffAddr);

    for (uint32_t i = 0; i < UCD_DFL_1_Q_SIZE; i++)
    {
        writeq(cmdAddr, &pEntries[i]);
        cmdAddr += DFL_BUFF_ELMNT_SIZE;
    }
}

void fpsCpu0::FpsCpuNormalBootInitialize()
{
    #ifdef LIONPERF_SUPPORT
    LoggingNormalBootInit();
    #endif

    inBoundCqCi[IBCQ_0] = 0;
    inBoundCqCi[IBCQ_1] = 0;

    outBoundCqCi[OBCQ_0] = 0;
    outBoundCqCi[OBCQ_1] = 0;

    inBoundDflPi[DFL_0] = UCD_DFL_Q_SIZE - 1;
    inBoundDflPi[DFL_1] = UCD_DFL_1_Q_SIZE - 1;
    _InitDFLList();

    #ifndef SUPPORT_FPS_REGISTER
    writel(inBoundDflPi[DFL_0], _ucdIbq.pHwDflPi[DFL_0]);
    writel(inBoundDflPi[DFL_1], _ucdIbq.pHwDflPi[DFL_1]);
    #endif
}

uint8_t fpsCpu0::ChkRecvFPMsgFiberDone()
{
    uint32_t msgCPU2toCPU0Pi = readl(pCPU2toCPU0Pi);
    uint32_t msgCPU2toCPU0Ci = readl(pCPU2toCPU0Ci);

    uint32_t msgCPU1toCPU0Pi = readl(pCPU1toCPU0Pi);
    uint32_t msgCPU1toCPU0Ci = readl(pCPU1toCPU0Ci);

    if (msgCPU2toCPU0Pi != msgCPU2toCPU0Ci) // for req from CPU2
    {
        return false;
    }

    if (msgCPU1toCPU0Pi != msgCPU1toCPU0Ci) //for req from CPU1
    {
        return false;
    }

    return true;
}

uint8_t fpsCpu0::ChkMsgHandleDone()
{
    uint32_t msgCPU2toCPU0Pi = readl(pCPU2toCPU0Pi);
    uint32_t msgCPU2toCPU0Ci = readl(pCPU2toCPU0Ci);
    uint32_t msgCPU1toCPU0Pi = readl(pCPU1toCPU0Pi);
    uint32_t msgCPU1toCPU0Ci = readl(pCPU1toCPU0Ci);

    if (msgCPU2toCPU0Pi != msgCPU2toCPU0Ci) // for req from CPU2
    {
        return false;
    }

    if (msgCPU1toCPU0Pi != msgCPU1toCPU0Ci) //for req from CPU1
    {
        return false;
    }

    return true;
}

uint8_t fpsCpu0::ChkQisRunningSetSlotSts(uint8_t slotSts, uint8_t sqPId)
{
    uint8_t runningQFlag = false;
    uint8_t ibcqIndex = GET_IBCQ_INDEX_FROM_SQ_PID(sqPId);
    uint16_t qMask = (ibcqIndex == IBCQ_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;

    if (slotSts & (cStsFwUpdate | cStsForceCompletion))
    {
        uint32_t ibCqPi = (uint32_t)readl(_ucdIbq.pHwIbCqPi[ibcqIndex]);
        uint32_t ibCqCi = inBoundCqCi[ibcqIndex];

        while (!QUEUE_EMPTY(ibCqPi, ibCqCi))
        {
            UcdCqEntry_t* pCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[ibCqCi]);

            if (pCqe->QPId == sqPId)
            {
                //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("[IO LOG] sqPId 0x%X\n", sqPId), "32");
                //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("[IO LOG] ChkQisRunningSetSlotSts ibCqPi 0x%X\n", ibCqPi), "32");
                //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("[IO LOG] ChkQisRunningSetSlotSts ibCqCi 0x%X\n", ibCqCi), "32");
                runningQFlag = true;
                break;
            }

            ibCqCi = QUEUE_INC(ibCqCi, qMask);
        }
    }

    uint8_t caIdx = pIbPhysicalId2Ca[sqPId];

    if ((!QUEUE_EMPTY_EXTRA_BIT(cmdArrayPi[caIdx], cmdArrayCi[caIdx])))
    {
        runningQFlag = true;
    }

    if (runningQFlag && (slotSts & cStsFwUpdate))
    {
        _pSlotFlagSts[sqPId] = (slotSts) ? (_pSlotFlagSts[sqPId] | slotSts) : slotSts;
    }

    return runningQFlag;

}

void fpsCpu0::IbCqRefillDFL(uint8_t sqPId)
{
    uint8_t ibcqIndex = GET_IBCQ_INDEX_FROM_SQ_PID(sqPId);
    uint16_t qMask = (ibcqIndex == IBCQ_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
    uint32_t ibCqPi = (uint32_t)readl(_ucdIbq.pHwIbCqPi[ibcqIndex]);
    uint32_t ibCqCi = inBoundCqCi[ibcqIndex];

    while (!QUEUE_EMPTY(ibCqPi, ibCqCi))
    {
        UcdCqEntry_t* pCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[ibCqCi]);
        if (pCqe->QPId == sqPId)
        {
            RefillDFL(pCqe->AddrLow);
            pCqe->AddrLow = DFL_BUFFER_INVALID;
        }

        ibCqCi = QUEUE_INC(ibCqCi, qMask);
    }
}

void fpsCpu0::ClearVFSetting(VFNodeInfo_t* pVFNodeInfo)
{
    uint64_t VfEnabledBitMap = *(_pVFEnBitmap);
    pVFNodeInfo->credit = 0;
    _pVfCredit[pVFNodeInfo->vfId] = 0;
    _pVfRemainCredit[pVFNodeInfo->vfId] = 0;

    #ifdef QOS_LATENCY_ERROR_HANDLING
    pVFNodeInfo->qosPenaltyPeriod = 0;
    #endif

    if (pVFNodeInfo->vfId == MAX_VF_NUM)
    {
        writel(0, (uint32_t)_pVF65EnBitmap);
    }
    else
    {
        VfEnabledBitMap &= ~(BIT_ULL(pVFNodeInfo->vfId));
        *(_pVFEnBitmap) = VfEnabledBitMap;
    }

    DMB();
}

void fpsCpu0::ResetMapDeleteQ(uint8_t ibPhyQId)
{
    uint8_t* pIbQ2ObQ = (uint8_t*)(CPU0AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_IBQ2OBQ_ADDR));
    _pSlotFlagSts[ibPhyQId] = cStsInit;
    pIbQ2ObQ[ibPhyQId] = QID_INVALID;

    DMB();

    // check if queue block has no valid sq
    uint8_t qbIdx = SQ_PID_2_QBIDX(ibPhyQId);
    QueueBlockInfo_t* pQBInfo = &_pQueueBlockInfoBase[qbIdx];
    if (!(_pSlotFlagSts[QBIDX_2_HIGH_SQ_PID(qbIdx)] & cStsValid) && !(_pSlotFlagSts[QBIDX_2_LOW_SQ_PID(qbIdx)] & cStsValid) && (pQBInfo->vfId != VFID_INV))
    {
        uint8_t VFId = pQBInfo->vfId;
        VFNodeInfo_t* pVFNodeInfo = &_pVfInfoBase[VFId];

        if (qbIdx == MAX_VF_NUM)
        {
            pVFNodeInfo->queueBlk65BitMap = 0;
            _pQB65EnBitmap[VFId] = 0;
        }
        else
        {
            _pQBEnBitmap[VFId] &= (~(BIT_ULL(qbIdx)));
            pVFNodeInfo->queueBlkBitMap &= (~(BIT_ULL(qbIdx)));
        }

        pQBInfo->vfId = VFID_INV;

        DMB();

        //DebugLogLvDbgInfo(cLogCPU0Common, cLogDebug, ("[MSG Q_RST] VFId:0x%X, ibPhyQId:0x%X, qbId:0x%X\n", ((((uint32_t)(qbIdx) & 0xFFFFUL) << 0x10UL) | (((uint32_t)ibPhyQId & 0xFFUL) << 0x8UL) | (uint32_t)(VFId))), "8,8,16");
    }

}

void fpsCpu0::HandleDeleteVFQblk(uint8_t ibPhyQId, uint8_t vfId, uint8_t slotSts)
{
    uint8_t runningQFlag = false;
    VFNodeInfo_t* pVFNodeInfo = &_pVfInfoBase[vfId];

    runningQFlag = ChkQisRunningSetSlotSts(slotSts, ibPhyQId);

    if ((!runningQFlag) && (slotSts & (cStsTearDown | cStsDelete)))
    {
        ResetMapDeleteQ(ibPhyQId);
    }// else do nothing


    if (ChkQBlkBitmap(pVFNodeInfo) && (slotSts & cStsTearDown))
    {
        ClearVFSetting(pVFNodeInfo);
    }// else do nothing

    DMB();
}

void fpsCpu0::HandleAdminAbort(CP2FPMsgAdminAbort_t* abortMsg)
{
    uint8_t ibcqIndex = ((abortMsg->ibQId == 64) || (abortMsg->ibQId == 129)) ? IBCQ_1 : IBCQ_0;
    uint16_t qMask = (ibcqIndex == IBCQ_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
    uint32_t ibCqPi = readl(_ucdIbq.pHwIbCqPi[ibcqIndex]);
    uint32_t ibCqCi = inBoundCqCi[ibcqIndex];

    /* Search the host command in IB CQ */
    while (ibCqPi != ibCqCi)
    {
        UcdCqEntry_t* pIbCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[ibCqCi]);
        LionNvmeSQDescriptor_t* pHostCmd = (LionNvmeSQDescriptor_t*)pIbCqe->AddrLow;

        uint8_t ibPhyQId = pIbCqe->QPId;
        uint8_t qbIdx = SQ_PID_2_QBIDX(ibPhyQId);
        uint8_t vfId = _pQueueBlockInfoBase[qbIdx].vfId;

        if ((ibPhyQId == abortMsg->ibQId) && (pHostCmd->HostCid == abortMsg->cmdId) && (vfId == abortMsg->vfId))
        {
            pIbCqe->Abort = 1; // mark it to abort status, will process it in FpsCpu0FillCmdEntryAndSendToCpu1()

            abortMsg->abortSts = abortSuccess;

            DMB();

            return;
        }

        ibCqCi = QUEUE_INC(ibCqCi, qMask);

    }

    /* Search the host command in error retry command queue */
    uint16_t retryCePi = readw(0x40000B20);
    uint16_t retryCeCi = retryCEQueueCi;

    while (retryCePi != retryCeCi)
    {
        uint16_t ceIndex = pRetryCeIndexQueue[retryCeCi];
        uint16_t dflIdx = _pCmdEntryArray[ceIndex].DFLIdx;
        uint8_t cdmaListNum = _pCmdEntryArray[ceIndex].cdmaListNum;
        uint8_t ibPhyQId = _pCmdEntryArray[ceIndex].PhyIbqId;
        uint8_t qbIdx = SQ_PID_2_QBIDX(ibPhyQId);
        uint8_t vfId = _pQueueBlockInfoBase[qbIdx].vfId;
        LionNvmeSQDescriptor_t* pHostCmd = (LionNvmeSQDescriptor_t*)(CPU0AccessCPU1TCMMem( \
                                                                         GET_DFL_PHYSICAL_BUF_ADDR(cdmaListNum, (dflIdx << DFL_BUF_SZ_SHIFT))));

        if ((ibPhyQId == abortMsg->ibQId) && (pHostCmd->HostCid == abortMsg->cmdId) && (vfId == abortMsg->vfId))
        {
            _pCmdEntryArray[ceIndex].Status = cCEStsCdmaAbort;
            _pCmdEntryArrayTiny[ceIndex].abortStatus = cCETinyAdminAbort;

            abortMsg->abortSts = abortSuccess;

            DMB();

            return;
        }
        retryCeCi = QUEUE_INC(retryCeCi, 0x1ff);

    }
}

CP2FPMsgSts fpsCpu0::FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone)
{
    if (change)
    {
        switch (changeStatus)
        {
            case FP_STS_NORMAL_BOOT:
            {
                if(gResetType == cPor)
                {
                    FpsCpuNormalBootInitialize();
                }
            }
            break;

            case FP_STS_FP_START:
            {
                #ifdef DISABLE_INDIRECT_REG_WRITE
                hwDflPiAddr[DFL_0] = (uint32_t)rFps->fpsSocFwdRegRegisters[cFpSocFwd00Ucd1Dfl0].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr;
                hwDflPiAddr[DFL_1] = (uint32_t)rFps->fpsSocFwdRegRegisters[cFpSocFwd02Ucd1Dfl3].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr;
                #endif
                _fpsCpu0FpCmdHandlerFiber.Activate();
            }
            break;

            default:
                *pDone = false;
                break;

        }
    }

    if (*pDone)
    {
        writel(changeStatus, pCpuStatus);
        DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("FP CPU 0 Status Change Completed. Status [0x%x]\n", readl(pCpuStatus)), "32");
    }
    else
    {
        return msgInvalidField;
    }

    return msgSuccess;

}

void fpsCpu0::UpdateQRunningStatus(CP2FPMsgContext_t* pMsg)
{
    CP2FPMsgDataShutdownReq_t* ptmpData = (CP2FPMsgDataShutdownReq_t*)(pMsg->data);
    uint32_t drainTime = ptmpData->drainTime;
    while ((readl(&_pIbCmnReg[UCD_CORE_1]->ucdIbCmnSnglInboundUcdStatus) & UCD_IB_CMN_SNGL_STATUS_BUSY))
    {
        // Waiting
        // Inbound UCD Busy since Command Fetches are Pending.
    }

    uint64_t tmpVFEnMap = *_pVFEnBitmap;
    if (tmpVFEnMap)
    {
        for (uint16_t vfid = FindNextBit64(tmpVFEnMap); tmpVFEnMap != 0;
             tmpVFEnMap &= ~(BIT_ULL(vfid)),  vfid = FindNextBit64(tmpVFEnMap))
        {
            uint64_t tmpQBEnMap = _pQBEnBitmap[vfid];
            if (tmpQBEnMap)
            {
                for (uint16_t qbid = FindNextBit64(tmpQBEnMap); tmpQBEnMap != 0;
                     tmpQBEnMap &= ~(BIT_ULL(qbid)),  qbid = FindNextBit64(tmpQBEnMap))
                {
                    ChkQisRunningSetSlotSts(cStsFwUpdate,  QBIDX_2_HIGH_SQ_PID(qbid));
                    ChkQisRunningSetSlotSts(cStsFwUpdate,  QBIDX_2_LOW_SQ_PID(qbid));
                }
            }
            if (_pQB65EnBitmap[vfid])
            {
                ChkQisRunningSetSlotSts(cStsFwUpdate,  QB65_HIGH_PHYSICAL_Q_INDEX);
                ChkQisRunningSetSlotSts(cStsFwUpdate,  QB65_LOW_PHYSICAL_Q_INDEX);
            }
        }
    }
    if (_pVF65EnBitmap)
    {
        uint64_t tmpQBEnMap = _pQBEnBitmap[MAX_VF_NUM];
        if (tmpQBEnMap)
        {
            for (uint16_t qbid = FindNextBit64(tmpQBEnMap); tmpQBEnMap != 0;
                 tmpQBEnMap &= ~(BIT_ULL(qbid)),  qbid = FindNextBit64(tmpQBEnMap))
            {
                ChkQisRunningSetSlotSts(cStsFwUpdate,  QBIDX_2_HIGH_SQ_PID(qbid));
                ChkQisRunningSetSlotSts(cStsFwUpdate,  QBIDX_2_LOW_SQ_PID(qbid));
            }
        }

        if (_pQB65EnBitmap[MAX_VF_NUM])
        {
            ChkQisRunningSetSlotSts(cStsFwUpdate,  QB65_HIGH_PHYSICAL_Q_INDEX);
            ChkQisRunningSetSlotSts(cStsFwUpdate,  QB65_LOW_PHYSICAL_Q_INDEX);
        }
    }
}

Error_t fpsCpu0::FpsCpuVfSlotSq2CqMapUpdate(CP2FPMsgContext_t* pMsg)
{
    CP2FPMsgDataVfSlotSq2CqMapUpdate_t* pCtx = (CP2FPMsgDataVfSlotSq2CqMapUpdate_t*)(&pMsg->data[0]);
    uint8_t VFId = MAP_FUNCTION_ID(pCtx->VFId);

    if ((VFId > MAX_VF_NUM))
    {
        DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("FpsCpuVfSlotSq2CqMapUpdate: vfId out of range, vfId:0x%X\n", VFId ), "32");
        return cEcError;
    }

    if ((pCtx->SqPId >= UCD_FP_IO_Q_NUM))
    {
        DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("FpsCpuVfSlotSq2CqMapUpdate: SqPId out of range, SqPId:0x%X\n", pCtx->SqPId), "32");
        return cEcError;
    }

    switch (pCtx->Action)
    {
        case cActionCreate:
        {
            // check if the corresponding QB is assigned to the VF or not
            uint8_t* pIbQ2ObQ = (uint8_t*)(CPU0AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_IBQ2OBQ_ADDR));
            uint8_t qbIdx = SQ_PID_2_QBIDX(pCtx->SqPId);
            QueueBlockInfo_t* pQBInfo = &_pQueueBlockInfoBase[qbIdx];
            if (pQBInfo->vfId != VFID_INV)
            {
                // directly go to proceed queue installation
                pMsg->sts = msgSuccess;
            }
            else
            {
                VFNodeInfo_t* pVFNodeInfo = &_pVfInfoBase[VFId];
                if (pQBInfo->queueBlockIndex == 64)
                {
                    pVFNodeInfo->queueBlk65BitMap = 1;
                    _pQB65EnBitmap[VFId] = 1;
                }
                else
                {
                    pVFNodeInfo->queueBlkBitMap |= BIT_ULL(pQBInfo->queueBlockIndex);
                    _pQBEnBitmap[VFId] |= BIT_ULL(pQBInfo->queueBlockIndex);
                }

                pQBInfo->vfId = VFId;
                pMsg->sts = msgNotifyCpu1;   ///< to update credit info

            }
            uint8_t caIdx = pIbPhysicalId2Ca[pCtx->SqPId];  // need to support 128/129
            _CPU1SubmitAbortInfo[caIdx] = ABORT_NOT_SUBMIT;
            DMB();

            // proceed queue installation
            _pSlotFlagSts[pCtx->SqPId] |= cStsValid;   ///< set sqpid to valid
            pIbQ2ObQ[pCtx->SqPId] = pCtx->CqPId;       ///< update relative cqpid

            //DebugLogLvDbgInfo(cLogCPU0Common, cLogDebug, ("[MSG Q_C] vfId:0x%X, QBIdx:0x%X, SqPId:0x%X, CqPId:0x%X, SlotFlagSts:0x%X\n",                                                                                                     \
                                                          ((((uint32_t)(pCtx->CqPId) & 0xFFUL) << 0x18UL) | (((uint32_t)(pCtx->SqPId) & 0xFFUL) << 0x10UL) | (((uint32_t)(pQBInfo->queueBlockIndex) & 0xFFUL) << 0x8UL) | (uint32_t)(VFId)), \
                                                          (((uint32_t)_pSlotFlagSts[pCtx->SqPId]) & 0xFFUL)), "8,8,8,8", "32");
        }
        break;

        // Dirk VF65 todo
        case cActionRemove:
        {
            //DebugLogLvDbgInfo(cLogCPU0Common, cLogDebug, ("[MSG Q_D] vfId:0x%X, SqPId:0x%X, Action:0x%X\n", \
                                                          ((((uint32_t)(pCtx->Action) & 0xFFUL) << 0x18UL) |  (((uint32_t)(pCtx->SqPId) & 0xFFUL) << 0x8UL) | (uint32_t)(pCtx->VFId))), "8,16,8");

            uint8_t SqPId = pCtx->SqPId;
            IbCqRefillDFL(SqPId);
            HandleDeleteVFQblk(SqPId, VFId, cStsDelete);
        }
        break;

        case cActionForceCompletion:
        {
            bool runningFlag = false;
            runningFlag = ChkQisRunningSetSlotSts(cStsForceCompletion, pCtx->SqPId);
            if (!runningFlag)
            {
                _pSlotFlagSts[pCtx->SqPId] &= (uint8_t)(~(cStsForceCompletion));
            }
        }
        break;
    }

    if (pMsg->sts)
    {
        return cEcInProgress;
    }

    return cEcNoError;

}

Error_t fpsCpu0::FpsCpuVfUpdate(CP2FPMsgDataVfUpdate_t* pData)
{
    uint8_t VFId = MAP_FUNCTION_ID(pData->VFId);
    uint64_t EnBitmap = *_pVFEnBitmap;

    switch (pData->Action)
    {
        case cActionVfInstall:
        {
            //if VF is not enabled
            if (VFId == MAX_VF_NUM)
            {
                if (readl(((uint32_t)_pVF65EnBitmap)))
                {
                    //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("vf install already, vfId:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
                    return cEcError;   ///< msgVfInstalledAlready
                }
                else
                {
                    writel(1, ((uint32_t)_pVF65EnBitmap));
                }
            }
            else
            {
                if (!(EnBitmap & BIT_ULL(VFId)))
                {
                    EnBitmap |= BIT_ULL(VFId);
                    *_pVFEnBitmap = EnBitmap;
                }
                else
                {
                    //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("vf install already, vfId:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
                    return cEcError;   ///< msgVfInstalledAlready
                }
            }

            break;
        }

        case cActionTearDown:
        {
            HandleTeardown(VFId);

            break;
        }

        default:
            break;
    }

    return cEcNoError;

}

#ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
Error_t fpsCpu0::FpsCpuUpdateTimestampAddr(CP2FPMsgDataUpdateTimestampAddr_t* pData)
{
    // init system tick
    gTimerCounterBase = (readl(REG_GLOBAL_SYNC_COUNTER_LO)) & SYSTICK_MASK;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x7, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
    return cEcNoError;
}
#endif

CP2FPMsgSts fpsCpu0::ManageCPMsg(uint8_t msgIdx, uint8_t msgSrc)
{
    CP2FPMsgContext_t* pMsgTemp;
    switch (msgSrc)
    {
        case CP0:
            pMsgTemp = (CP2FPMsgContext_t*)(((uint32_t)pCP0toFPMsgQ) + msgIdx * PSRAM_CP2FP_MSG_ELMNT_SIZE);
            break;
        case CP1:
            pMsgTemp = (CP2FPMsgContext_t*)(((uint32_t)pCP1toFPMsgQ) + msgIdx * PSRAM_CP2FP_MSG_ELMNT_SIZE);
            break;
        default:
            return msgInvalidField;
    }

    uint8_t msgOp = pMsgTemp->msgOp;
    uint8_t* pData = pMsgTemp->data;
    CP2FPMsgSts msgSts = msgSuccess;

    switch (msgOp)
    {
        case msgOpFpStsChange:
        {
            Fastpath_Status_t changeStatus;
            uint8_t change = 1;
            uint8_t done = 1;
            changeStatus = (Fastpath_Status_t)(*pData);
            msgSts = FpsCpuHandleStatusChange(changeStatus, change, &done);

            break;
        }

        case msgOpShutdownReq:
        {
            UpdateQRunningStatus(pMsgTemp);
            break;
        }

        #ifdef MCR_TEST_HOOKS
        case msgOpInjectErrorReq:
        {
            CP2FPMsgDataInjectErrorReq_t* ptmpData = (CP2FPMsgDataInjectErrorReq_t*)(pData);
            if(ptmpData->errorType > InjErrFpIoLvl1Abrt && ptmpData->errorType <= InjErrHang){
                TriggerCrash(ptmpData->errorType);
            }
            break;
        }
        #endif

        case msgOpErrQSet:
        {
            uint32_t subOp = *(uint32_t*)pData;
            switch (subOp)
            {
                case msgSubOpAdminAbort:
                {
                    HandleAdminAbort((CP2FPMsgAdminAbort_t*)(pData));
                    break;
                }

                default:
                {
                    msgSts = msgInvalidField;
                    break;
                }
            }

            break;
        }

        case msgOpVfSlotSQ2CQMapUpdate:
        {
            Error_t errCode;
            errCode = FpsCpuVfSlotSq2CqMapUpdate(pMsgTemp);
            if (cEcNoError != errCode)
            {
                msgSts = (CP2FPMsgSts)(pMsgTemp->sts);   ///< modify msg sts in FpsCpuVfSlotSq2CqMapUpdate
            }

            break;
        }

        #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
        case msgOpUpdateTimestampAddr:
        {
            Error_t errCode;
            errCode = FpsCpuUpdateTimestampAddr((CP2FPMsgDataUpdateTimestampAddr_t*)(pData));

            break;
        }
        #endif

        case msgOpVfUpdate:
        {
            Error_t errCode = FpsCpuVfUpdate((CP2FPMsgDataVfUpdate_t*)(pData));
            if (errCode != cEcNoError) ///< current only case cEcError with msgVfInstalledAlready
            {
                msgSts = msgVfInstalledAlready;   ///< msg failure cause
            }

            break;
        }

        #ifdef LIONPERF_SUPPORT
        case msgOpLogEnDisUpdate:
        {
            CP2FPMsgDataLogEnDisUpdate_t* ptmpData = (CP2FPMsgDataLogEnDisUpdate_t*)(pData);
            if (ptmpData->action == 1)
            {
                LoggingUpdateGdmaInfo(ptmpData->gdmaQSizeFpsCpu0, ptmpData->piInfoFpsCpu0, ptmpData->pingPongIndexFpsCpu0);
            }

            LoggingUpdateLogExtByLogExtShared();

            break;
        }

        case msgOpSetLogLevel:
        {
            LoggingUpdateLogExtByLogExtShared();

            break;
        }
        #endif

        case msgOpUcdQuery:
        {
            CP2FPMsgDataUcdQuery_t* pQueryData = (CP2FPMsgDataUcdQuery_t*)(pData);
            uint8_t ucdCoreID = pQueryData->ucdCoreID;
            uint8_t ucdQueueID = pQueryData->queueID;

            if(ucdCoreID >= UCD_CORE_NUM)
            {
                DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("ucdCoreID out of range [0x%x]\n", ucdCoreID), "32");
                msgSts = msgInvalidField;
                break;
            }
            else if(ucdQueueID >= IBCQ_END )
            {
                DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("ucdQueueID out of range [0x%x]\n", ucdQueueID), "32");
                msgSts = msgInvalidField;
                break;
            }


            //HW pi retains the value after idfu so copying it to IBCQ & OBCQ indirect ci regs to avoid issues caused due to FPS reg reset. 
            inBoundCqCi[ucdQueueID] = readl(&_pIbCmnReg[ucdCoreID]->ucdIbCmnCqRegisters[ucdQueueID].ucdIbCmnCqCompletionQueuePi);
            writel(inBoundCqCi[ucdQueueID], _ucdIbq.pHwIbCqCi[ucdQueueID]);

            outBoundCqCi[ucdQueueID] = readl(&_pObCmnReg[ucdCoreID]->ucdObCmnCqRegisters[ucdQueueID].ucdObCmnCqOutboundCompletionQueuePi);
            writel(outBoundCqCi[ucdQueueID], _ucdObq.pHwObCqCi[ucdQueueID]);

            inBoundDflPi[ucdQueueID] = readl(&_pIbCmnReg[ucdCoreID]->ucdIbCmnDflRegisters[ucdQueueID].ucdIbCmnDflInboundDestinationFreeListPi);
            writel(inBoundDflPi[ucdQueueID], _ucdIbq.pHwDflPi[ucdQueueID]);

#ifdef DISABLE_INDIRECT_REG_WRITE // ideally it should be disabled in CPU state - FP_STS_FP_START
            uint32_t disableBit = (ucdQueueID == DFL_0) ? SOC_REG_0_WR_BIT : SOC_REG_2_WR_BIT;
            if (readl(REG_FPS_INDIRECT_REG_WR_DISABLE) & disableBit)
            {
                writel(inBoundDflPi[ucdQueueID], hwDflPiAddr[ucdQueueID]);
            }
#endif

            writel(readl(&_pIbCmnReg[ucdCoreID]->ucdIbCmnCqRegisters[ucdQueueID].ucdIbCmnCqCompletionQueuePi),
                    _ucdIbq.pHwIbCqPi[ucdQueueID]);

            writel(readl(&_pObCmnReg[ucdCoreID]->ucdObCmnCqRegisters[ucdQueueID].ucdObCmnCqOutboundCompletionQueuePi),
                    _ucdObq.pHwObCqPi[ucdQueueID]);


            break;
        }
        default:
            msgSts = msgNotSupport;
            break;

    }

    return msgSts;

}

CP2FPMsgSts fpsCpu0::SendFPMsg(M7CoreId_t cpu, uint8_t fpMsgOp, uint8_t resp, CP2FPMsgSts fpSts, uint8_t cmdSpecific0, uint8_t cmdSpecific1)
{
    FPInterMsgHeader* pFPMsgHeader;
    volatile uint32_t* pMsgPi, * pMsgCi;
    uint32_t msgPi, msgCi;
    CP2FPMsgSts sts = fpSts;
    FPInterMsgHeader fpInterMsgTmpHeader;
    fpInterMsgTmpHeader.fpMsgOp = fpMsgOp;
    fpInterMsgTmpHeader.resp = resp;
    fpInterMsgTmpHeader.sts = fpSts;
    fpInterMsgTmpHeader.cmdSpecific[0] = cmdSpecific0;
    fpInterMsgTmpHeader.cmdSpecific[1] = cmdSpecific1;
    switch (cpu)
    {
        case cM7Core1:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU0toCPU1MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU0toCPU1Pi;
            pMsgCi = (volatile uint32_t*)pCPU0toCPU1Ci;
            CPU0toCPU1Pi = readl(pMsgPi);
            msgPi = CPU0toCPU1Pi;
        }
        break;
        case cM7Core2:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU0toCPU2MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU0toCPU2Pi;
            pMsgCi = (volatile uint32_t*)pCPU0toCPU2Ci;
            CPU0toCPU2Pi = readl(pMsgPi);
            msgPi = CPU0toCPU2Pi;
        }
        break;
        default:
        {
            return msgNotSupport;
        }
        break;
    }
    msgCi = readl(pMsgCi);

    if (M7_QUEUE_FULL(msgPi, msgCi, (uint32_t)PSRAM_INTL_CPUX2CPUY_MSG_MASK))    //chk fp msg Q full wait q space
    {
        //DebugLogLvDbgInfo(cLogCPU0Common, cLogDebug, ("FP0 Queue Full msgCi[0x%x], msgPi[0x%x]\n", (msgCi | (msgPi << 0x10UL))), "16,16");
        //DebugLogLvDbgInfo(cLogCPU0Common, cLogDebug, ("FP0 opcode [0x%x]\n", pFPMsgHeader[msgPi].fpMsgOp), "32");
        return msgNoEmptyEntry;
    }

    M7_MEM_COPY(&pFPMsgHeader[msgPi], &fpInterMsgTmpHeader, sizeof(FPInterMsgHeader));

    msgPi = M7_QUEUE_INC(msgPi, (uint32_t)PSRAM_INTL_CPUX2CPUY_MSG_MASK);
    writel(msgPi, pMsgPi);

    sts = fpSts;
    switch (cpu)
    {
        case cM7Core1:
        {
            CPU0toCPU1Pi = msgPi;
        }
        break;
        case cM7Core2:
        {
            CPU0toCPU2Pi = msgPi;
        }
        break;
        default:
            break;
    }

    #ifdef IPC_SUPPORT
    switch (cpu)
    {
        case cM7Core1:
        {
            IpcDescTrigger(CPU0toCPU1_DESC, msgPi);
        }
        break;
        case cM7Core2:
        {
            IpcDescTrigger(CPU0toCPU2_DESC, msgPi);
        }
        break;
        default:
            break;
    }
    #endif //IPC_SUPPORT
    //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP0 opcode[0x%x], msgPi[0x%x]\n", (pFPMsgHeader[msgPi].fpMsgOp | (msgPi << 0x10UL))), "16,16");
    //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP0 sts[0x%x]\n", sts), "32");

    return sts;

}
CP2FPMsgSts fpsCpu0::RecvFPMsg(FPInterMsgHeader* pFPMsgHeader, M7CoreId_t msgCpu)
{
    FpInterMsgOp fpMsgOp;
    CP2FPMsgSts fpSts = msgSuccess;
    CP2FPMsgSts recvSts = msgSuccess;
    fpMsgOp = (FpInterMsgOp)pFPMsgHeader->fpMsgOp;

    switch (fpMsgOp)
    {
        case cp2FpMsg:
        {
            fpSts = ManageCPMsg(pFPMsgHeader->msgIdx, pFPMsgHeader->msgSrc);
            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);
            break;
        }

        case cdmaResetMsg:
        {
            recvSts = HandleReSchedule();

            break;
        }

        case resetHandlingMsg:
        {
            uint32_t resetCause = IpcIntGetDescValue(ResetCP2FP);
            uint64_t FLRRequestBitMap = readq(pFLRRequestBitMapLocal);
            uint8_t vfId;

            for (vfId = FindNextBit64(FLRRequestBitMap); FLRRequestBitMap; FLRRequestBitMap &= ~(BIT_ULL(vfId)), vfId = FindNextBit64(FLRRequestBitMap))
            {
                vfId = MAP_FUNCTION_ID(vfId);
                //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("Handle function reset ID:0x%x \n", (vfId)), "32");

                HandleTeardown(vfId);
            }

            if (resetCause != VFLR && readl(_pVF65EnBitmap))
            {
                //DebugLogLvDbgInfo(cLogCPU0Common, cLogInfo, ("Handle function reset ID:0x%x \n", (PF_ID)), "32");
                HandleTeardown(PF_ID);
            }

            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);

            break;
        }

        default:
            recvSts = msgNotSupport;

            break;
    }

    return recvSts;

}
