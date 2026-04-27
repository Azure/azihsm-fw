// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1ErrorHandle.cpp
//! @brief  FpsCpu2 Error Handler
//!
//=============================================================================


//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu1.h"

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

extern Fps_t* rFps;

#define REG_FPS_BANK0_EVENT_STATUS_0      (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all)
#define REG_FPS_BANK0_EVENT_STATUS_1      (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus1.all)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_0 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus0SlotArrayQEmpty310)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_1 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus1SlotArrayQEmpty6332)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_2 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus2SlotArrayQEmpty9564)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_3 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus3SlotArrayQEmpty12796)
#define REG_FPS_SLOT_ARRAY_PI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueProducerIndex.all)
#define REG_FPS_SLOT_ARRAY_CI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueConsumerIndex.all)
#define REG_FPS_SLOT_ARRAY_SIZE_BASE      (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueSize.all)
#define REG_FPS_SLOT_ARRAY_STATUS_BASE    (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueStatus.all)
#define REG_FPS_INDIRECT_REG_WR_DISABLE   (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable)

#define CPU1_GET_DFL_BUF_ADDR(cdmaListNum, dflOffset) (cdmaListNum == CDMA_FP_DFL_3_LIST) ? (dflOffset + M7_FPS_CPU12_DFL_1_BUFF_ADDR) : (dflOffset + M7_FPS_CPU1_DFL_BUFF_ADDR)

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

void fpsCpu1::_SkipAndAbortWithSQid(uint16_t sqid,  CmdEntryStatus_t ceSts)
{
    uint8_t caIndex = pIbPhysicalId2Ca[sqid];
    uint32_t cmdArrayPiValue,  cmdArrayCiValue;
    uint32_t* pCmdArrayHwCi;
    uint32_t* pCmdArrayHwPi;

    pCmdArrayHwCi = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIndex << CMD_ARRAY_SHIFT));
    pCmdArrayHwPi = (uint32_t*)(REG_FPS_SLOT_ARRAY_PI_BASE + (caIndex << CMD_ARRAY_SHIFT));
    cmdArrayCiValue = readl(pCmdArrayHwCi);
    cmdArrayPiValue = readl(pCmdArrayHwPi);

    while (!QUEUE_EMPTY_EXTRA_BIT(cmdArrayPiValue,  cmdArrayCiValue))
    {
        uint16_t ceIndex = (caIndex << CA_SIZE_SHIFT) + (cmdArrayCiValue & CA_MASK);
        CmdEntry_t* pCe = _pCmdArrayBase + ceIndex;

        if (ceSts > pCe->Status)
        {
            pCe->Status = ceSts;
        }

        cmdArrayCiValue = QUEUE_INC(cmdArrayCiValue, CA_ROLLOVER_MASK);
    }

    uint8_t qbIndex = SQ_PID_2_QBIDX(sqid);
    QueueBlockInfo_t* pQBlockInfo = &_pQBlockInfoBase[qbIndex];

    if (pQBlockInfo->remainLen != 0)
    {
        uint16_t ceIndex = pQBlockInfo->remainCeIdx;
        if ((ceIndex >> CA_SIZE_SHIFT) ==  caIndex)
        {
            bool isNeededUpdateCmdArrayCi = true;
            isNeededUpdateCmdArrayCi = API_CDMASendAbort(ceIndex,   &cdmaSqPi,  0,  0,  &_cdmaSq, _cdmaFatalErrorFlag);
            if (isNeededUpdateCmdArrayCi)
            {
                _CPU1SubmitAbortInfo[caIndex] = ceIndex - (caIndex << CA_SIZE_SHIFT);

                DMB();

                _UpdateCmdArrayCi(ceIndex, pQBlockInfo);
            }
        }
    }

}

void fpsCpu1::_ProcessTeardownBitMap()
{
    uint8_t qbIndex, sqPId;

    if (_teardownQueueBlockBitMap)
    {
        for (qbIndex = FindNextBit64(_teardownQueueBlockBitMap); (_teardownQueueBlockBitMap != 0); \
             _teardownQueueBlockBitMap &= ~(BIT_ULL(qbIndex)), qbIndex = FindNextBit64(_teardownQueueBlockBitMap))
        {
            sqPId = QBIDX_2_HIGH_SQ_PID(qbIndex);
            _SkipAndAbortWithSQid(sqPId,  cCEStsTearDown);

            sqPId = QBIDX_2_LOW_SQ_PID(qbIndex);
            _SkipAndAbortWithSQid(sqPId,  cCEStsTearDown);

            DMB();
        }
    }

    if (_teardownQueueBlock65BitMap)
    {
        _SkipAndAbortWithSQid(QB65_HIGH_PHYSICAL_Q_INDEX,  cCEStsTearDown);
        _SkipAndAbortWithSQid(QB65_LOW_PHYSICAL_Q_INDEX,  cCEStsTearDown);

        _teardownQueueBlock65BitMap = 0;

        DMB();
    }
}

ATTR_ALWAYS_INLINE void fpsCpu1::_UpdateCmdArrayCi(uint16_t cpuCid, QueueBlockInfo_t* pQBlockInfo)
{
    uint8_t caIndex = (cpuCid >> CA_SIZE_SHIFT);
    uint32_t* pCmdArrayHwCi;

    pQBlockInfo->remainLen = 0;

    pCmdArrayHwCi = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIndex << CMD_ARRAY_SHIFT));
    cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);

    DMB();

    writel(cmdArrayCi[caIndex], pCmdArrayHwCi);
}

ATTR_ALWAYS_INLINE void fpsCpu1::_UpdateCmdArrayCiAfterAbort(uint16_t cpuCid)
{
    uint8_t caIndex = (cpuCid >> CA_SIZE_SHIFT);
    uint16_t sqid = pCa2IbPhysicalId[caIndex];
    uint8_t qbIndex = SQ_PID_2_QBIDX(sqid);

    QueueBlockInfo_t* pQBlockInfo = &_pQBlockInfoBase[qbIndex];
    if (pQBlockInfo->remainCeIdx == cpuCid && pQBlockInfo->remainLen != 0)
    {
        _UpdateCmdArrayCi(cpuCid, pQBlockInfo);
    }
}

void fpsCpu1::HandleAdminAbort(CP2FPMsgAdminAbort_t* abortMsg)
{
    uint8_t caIndex = pIbPhysicalId2Ca[abortMsg->ibQId];
    uint8_t caCi = cmdArrayCi[caIndex];
    uint8_t caPi = (uint8_t)(readl(REG_FPS_SLOT_ARRAY_PI_BASE + (caIndex << CMD_ARRAY_SHIFT)));

    while (caPi != caCi)
    {
        uint16_t ceIndex = (caIndex << CA_SIZE_SHIFT) + (caCi & CA_MASK);
        uint16_t dflIdx = _pCmdArrayBase[ceIndex].DFLIdx;
        uint8_t cdmaListNum = _pCmdArrayBase[ceIndex].cdmaListNum;
        uint8_t ibPhyQId = _pCmdArrayBase[ceIndex].PhyIbqId;
        uint8_t qbIdx = SQ_PID_2_QBIDX(ibPhyQId);
        uint8_t vfId = _pQBlockInfoBase[qbIdx].vfId;

        LionNvmeSQDescriptor_t* pHostCmd = (LionNvmeSQDescriptor_t*)(CPU1_GET_DFL_BUF_ADDR(cdmaListNum, (dflIdx << DFL_BUF_SZ_SHIFT)));

        if ((ibPhyQId == abortMsg->ibQId) && (pHostCmd->HostCid == abortMsg->cmdId) && (vfId == abortMsg->vfId))
        {
            _pCmdArrayBase[ceIndex].Status = cCEStsCdmaAbort;
            _pCmdArrayTinyBase[ceIndex].abortStatus = cCETinyAdminAbort; // access via CPU2

            abortMsg->abortSts = abortSuccess;

            DMB();

            return;
        }

        caCi = QUEUE_INC(caCi, CA_ROLLOVER_MASK);

    }

}
