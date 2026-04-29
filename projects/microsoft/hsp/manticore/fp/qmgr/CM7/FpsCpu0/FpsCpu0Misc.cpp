// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpSCpu0.cpp
//! @brief  FpSCpu0 Component Group
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu0.h"

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

bool FpsCpu0ChkFPMsgStsDone(CP2FPMsgSts sts)
{
    bool done = false;
    switch (sts)
    {
        case msgSuccess:
        case msgInvalidField:
        case msgVfInstalledAlready:
        case msgNotifyCpu1:
        case msgNotSupport:
        {
            done = true;
            break;
        }
        case msgInProgress:
        case msgNoEmptyEntry:
        {
            done = false;
            break;
        }
        default:
        {
            done = false;
            break;
        }
    }

    return done;
}

void fpsCpu0::FpsCpu0ReceiveFPMsgFiber(void* pObj)
{
    fpsCpu0* pThis = static_cast<fpsCpu0*>(pObj);

    uint32_t msgCPU2toCPU0Pi, msgCPU2toCPU0Ci;
    uint8_t modify = false;
    CP2FPMsgSts sts = msgSuccess;

    // Message Handling
    msgCPU2toCPU0Pi = readl(pThis->pCPU2toCPU0Pi);
    msgCPU2toCPU0Ci = readl(pThis->pCPU2toCPU0Ci);

    while (msgCPU2toCPU0Pi != msgCPU2toCPU0Ci) //msg from CPU2
    {
        sts = pThis->RecvFPMsg(&pThis->pCPU2toCPU0MsgQ[msgCPU2toCPU0Ci], cM7Core2);

        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP0 msgCPU2toCPU0Pi[0x%x], msgCPU2toCPU0Ci[0x%x]\n", ( msgCPU2toCPU0Pi| ( msgCPU2toCPU0Ci<< 0x10UL))), "16,16");
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP0 sts[0x%x]\n", sts), "32");
        if (FpsCpu0ChkFPMsgStsDone(sts))
        {
            msgCPU2toCPU0Ci = M7_QUEUE_INC(msgCPU2toCPU0Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            modify = true;
        }
        else
        {
            //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogInfo, ("FP0 sts[0x%x]\n", sts), "32");
            break;
        }
        //triggerCpu2 = 1;
    }

    if (modify)
    {
        pThis->CPU2toCPU0Ci = msgCPU2toCPU0Ci;
        writel(pThis->CPU2toCPU0Ci, pThis->pCPU2toCPU0Ci);
    }

    if (pThis->ChkRecvFPMsgFiberDone())
    {
        pThis->_fpsCpu0RecvFpMsgFiber.Wait();
        #ifdef IPC_SUPPORT
        IpcIntMaskClr(IPC_FP0, CPU1toCPU0_DESC);
        IpcIntMaskClr(IPC_FP0, CPU2toCPU0_DESC);
        #endif
    }
}

void fpsCpu0::CheckFPMsgFiberNeedResume(void* pObj)
{
    fpsCpu0* pThis = static_cast<fpsCpu0*>(pObj);
    {
        pThis->_fpsCpu0RecvFpMsgFiber.Resume();
    }
}

void fpsCpu0::_RestoreSchedulingStructure(uint8_t vfIndex)
{
    uint64_t vfQbEnBit = _pQBEnBitmap[vfIndex];
    for (uint16_t qbid = FindNextBit64(vfQbEnBit); vfQbEnBit != 0; \
         vfQbEnBit &= ~(BIT_ULL(qbid)),  qbid = FindNextBit64(vfQbEnBit))
    {
        uint16_t firstQid = QBIDX_2_HIGH_SQ_PID(qbid);
        uint16_t secondQid = QBIDX_2_LOW_SQ_PID(qbid);
        uint16_t caIdxFirst = pIbPhysicalId2Ca[firstQid];
        uint16_t caIdxSecond = pIbPhysicalId2Ca[secondQid];
        uint32_t* pCmdArrayHwCiFirst = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIdxFirst << CMD_ARRAY_SHIFT));
        uint32_t* pCmdArrayHwCiSecond = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIdxSecond << CMD_ARRAY_SHIFT));

        writel(cmdArrayCi[caIdxFirst],  pCmdArrayHwCiFirst);
        writel(cmdArrayCi[caIdxSecond],  pCmdArrayHwCiSecond);
    }

    if (_pQB65EnBitmap[vfIndex])
    {
        uint16_t qbid = FPS_QUEUE_BLOCK_65_INDEX;
        uint16_t firstQid = QBIDX_2_HIGH_SQ_PID(qbid);
        uint16_t secondQid = QBIDX_2_LOW_SQ_PID(qbid);
        uint16_t caIdxFirst = pIbPhysicalId2Ca[firstQid];
        uint16_t caIdxSecond = pIbPhysicalId2Ca[secondQid];
        uint32_t* pCmdArrayHwCiFirst = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIdxFirst << CMD_ARRAY_SHIFT));
        uint32_t* pCmdArrayHwCiSecond = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIdxSecond << CMD_ARRAY_SHIFT));

        writel(cmdArrayCi[caIdxFirst],  pCmdArrayHwCiFirst);
        writel(cmdArrayCi[caIdxSecond],  pCmdArrayHwCiSecond);
    }

}

CP2FPMsgSts fpsCpu0::HandleReSchedule()
{
    // 0->1 full?
    if (M7_QUEUE_FULL(CPU0toCPU1Pi, readl(pCPU0toCPU1Ci), (uint32_t)PSRAM_INTL_CPUX2CPUY_MSG_MASK))    //chk fp msg Q full wait q space
    {
        return msgInProgress;
    }

    uint64_t vfEnBitMap = *_pVFEnBitmap;
    for (uint8_t vfIndex = FindNextBit64(vfEnBitMap); vfEnBitMap != 0; \
         vfEnBitMap &= ~(BIT_ULL(vfIndex)), vfIndex = FindNextBit64(vfEnBitMap))
    {
        _RestoreSchedulingStructure(vfIndex);
    }

    if (_pVF65EnBitmap)
    {
        uint8_t vfIndex = MAX_VF_NUM;
        _RestoreSchedulingStructure(vfIndex);
    }

    CP2FPMsgSts sts = msgSuccess;
    CP2FPMsgSts msgSts;

    msgSts = SendFPMsg(cM7Core1, cdmaResetMsg, 0, msgSuccess, 0,  0);

    return sts;
}

void fpsCpu0::GetFwUpdateInfo(FWupdateBackupInfo* pFWupdateInfo, RecoverDataBlk dataType)
{
    FwUpdateDataHeader* pDataHeader = (FwUpdateDataHeader*)PSRAM_BACKUP_DATA_HEADER_ADDR;
    if (pDataHeader->signature != FW_UPDATE_SIGNATURE) // this will be from old structure to new
    {
        pFWupdateInfo->sts = cNoSignature;
        return;

    } // else do nothing
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

                if (dataType == cUCDData)
                {
                    uint32_t newDataLen = (uint32_t)PSRAM_DFL_BACKUP_SIZE;
                    if (dataLen > newDataLen)
                    {
                        FwSts = cOldVer;
                    }
                    pFWupdateInfo->addr = (uint32_t)(dataLenAddr + dataLenSize);
                    pFWupdateInfo->length = dataLen;
                    pFWupdateInfo->sts = FwSts;
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
