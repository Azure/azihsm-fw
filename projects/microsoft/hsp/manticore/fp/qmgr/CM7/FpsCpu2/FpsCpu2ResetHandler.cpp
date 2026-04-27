// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2ResetHandler.cpp
//! @brief  FpsCpu2 handle FLR and PERST request from CP
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2.h"
extern "C"
{
#include "vicommon.h"
}

void fpsCpu2::FpsCpu2CheckResetIrqCause(void)
{
    uint32_t IPCValue = IpcIntGetDescValue(ResetCP2FP);
    switch (IPCValue)
    {
        case VFLR:
        {
            if (_curResetRequest != PFLR && _curResetRequest != PERST)
            {
                _curResetRequest = VFLR;
                _resetHandlingState = cResetHandlingProcessTeardownMap;
                _fpsCpu2HandleResetFiber.Resume();
            }
            break;
        }
        case PFLR:
        {
            if (_curResetRequest != PERST)
            {
                _curResetRequest = PFLR;
                _resetHandlingState = cResetHandlingProcessTeardownMap;
                _fpsCpu2HandleResetFiber.Resume();
            }
            break;
        }
        case PERST:
        {
            _curResetRequest = PERST;
            if (_curResetRequest != PFLR)
            {
                _resetHandlingState = cResetHandlingProcessTeardownMap;
                _fpsCpu2HandleResetFiber.Resume();
            }
            break;
        }
        default:
        {
            break;
        }
    }

}

void fpsCpu2::FpsCpu2HandleResetFiber(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    ResetHandlingState_t curState = pThis->_resetHandlingState;
    ResetRequestMagicNumber_t flrRequest = pThis->_curResetRequest;

    switch (curState)
    {
        case cResetHandlingWait:
        {
            break;
        }

        case cResetHandlingProcessTeardownMap:
        {
            uint8_t vfId;
            uint64_t FLRRequestBitMap = 0;
            if (flrRequest == VFLR)
            {
                uint32_t flrBitMapAddr = (uint32_t)PSRAM_FLR_REQUEST_BITMAP;
                FLRRequestBitMap = readq(flrBitMapAddr);
            }
            else
            {
                FLRRequestBitMap = readq(pThis->_pVFEnBitmap);
            }

            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Handle function reset 0_31:0x%x\n", ((uint32_t)(FLRRequestBitMap) & 0xFFFFFFFF)), "32");
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Handle function reset 32_63:0x%x\n", ((uint32_t)(FLRRequestBitMap >> 32) & 0xFFFFFFFF)), "32");
            writeq(FLRRequestBitMap, pThis->pFLRRequestBitMapLocal);

            uint64_t curFLRRequestBitMap = FLRRequestBitMap;
            uint64_t flrQueueBlockBitMap = 0;
            uint8_t flrQueueBlock65BitMap = 0;
            for (vfId = FindNextBit64(curFLRRequestBitMap); curFLRRequestBitMap; curFLRRequestBitMap &= ~(BIT_ULL(vfId)), vfId = FindNextBit64(curFLRRequestBitMap))
            {
                vfId = MAP_FUNCTION_ID(vfId);
                if (BIT_ULL(vfId) & readq(pThis->_pVFEnBitmap))
                {
                    pThis->HandleTeardownSlotSts(vfId);
                    #ifdef QOS_LATENCY_ERROR_HANDLING
                    pThis->_qosPenaltyVfBitmap &= (~(BIT_ULL(vfId)));
                    #endif
                }

                VFNodeInfo_t* pVFInfo = &pThis->_pVfInfoBase[vfId];
                flrQueueBlockBitMap |= pVFInfo->queueBlkBitMap;
                flrQueueBlock65BitMap |= pVFInfo->queueBlk65BitMap;
            }
            if (flrRequest != VFLR && readl(pThis->_pVF65EnBitmap))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Handle function reset PF:0x%x\n", (PF_ID)), "32");
                uint8_t function65Id = PF_ID;
                pThis->HandleTeardownSlotSts(function65Id);
                #ifdef QOS_LATENCY_ERROR_HANDLING
                pThis->_qosPenaltyVf65Bitmap = 0;
                #endif
                VFNodeInfo_t* pVFInfo = &pThis->_pVfInfoBase[function65Id];
                flrQueueBlockBitMap |= pVFInfo->queueBlkBitMap;
                flrQueueBlock65BitMap |= pVFInfo->queueBlk65BitMap;
            }

            writeq(flrQueueBlockBitMap, pThis->pFLRQueueBlockMap);
            writeb(flrQueueBlock65BitMap, pThis->pFLRQueueBlock65Map);

            pThis->_resetHandlingState = cResetHandlingNotifyCPU0;

        }

        case cResetHandlingNotifyCPU0:
        {
            if (M7_QUEUE_FULL(pThis->CPU2toCPU0Pi, readl(pThis->pCPU2toCPU0Ci), PSRAM_INTL_CPUX2CPUY_MSG_MASK))
            {
                return;
            }

            pThis->SendFPMsg(cM7Core0, resetHandlingMsg, 0, msgSuccess, 0,  0);
            pThis->_resetHandlingState = cResetHandlingWaitCPU0Resp;

            break;
        }

        case cResetHandlingWaitCPU0Resp:
        {
            break;
        }

        case cResetHandlingNotifyCPU1:
        {
            if (M7_QUEUE_FULL(pThis->CPU2toCPU1Pi, readl(pThis->pCPU2toCPU1Ci), PSRAM_INTL_CPUX2CPUY_MSG_MASK))
            {
                return;
            }

            pThis->SendFPMsg(cM7Core1, resetHandlingMsg, 0, msgSuccess, 0,  0);
            pThis->_resetHandlingState = cResetHandlingWaitCPU1Resp;

            break;
        }

        case cResetHandlingWaitCPU1Resp:
        {
            break;
        }

        case cResetHandlingWaitTeardownDone:
        {
            if (flrRequest == VFLR)
            {
                if (readl(pThis->_pVFEnBitmap) & readl(PSRAM_FLR_REQUEST_BITMAP))
                {
                    return;
                }
            }
            else
            {
                if (readq(pThis->_pVFEnBitmap) || readl(pThis->_pVF65EnBitmap))
                {
                    return;
                }
            }

            pThis->_resetHandlingState = cResetHandlingNotifyCP;

        }

        case cResetHandlingNotifyCP:
        {
            writeq(0, pThis->pFLRRequestBitMapLocal);

            IpcDescTriggerWithValue(ResetFP2CP, pThis->_curResetRequest);
            pThis->_curResetRequest = noRequest;
            pThis->_resetHandlingState = cResetHandlingWait;
            writeq(0, pThis->pFLRQueueBlockMap);
            writeb(0, pThis->pFLRQueueBlock65Map);
            pThis->_fpsCpu2HandleResetFiber.Wait();
            // IpcIntMaskClr(IPC_FP2, ResetCP2FP);

            break;
        }

        default:
        {
            break;
        }

    }
}
