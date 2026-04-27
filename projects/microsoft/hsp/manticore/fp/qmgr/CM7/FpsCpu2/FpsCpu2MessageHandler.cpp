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
#include "M7MemMap.h"
#include "LoggingDebug.h"
extern "C"
{
#include "vicommon.h"
#include "irq.h"
#ifdef MCR_TEST_HOOKS
#include "crashdump.h"
#include "cm7ikmcu.h"
#endif
}
#ifdef SUPPORT_MSGERROR_INJECTION
#include "FpsCpu2ErrorInjection.h"
#endif
// #ifdef SUPPORT_TELEMETRY
#include "Version.h"
// #endif
//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

extern Fps_t* rFps;
#ifdef LOGGING_NEW_SCHEME
extern Gdma_t* rGdma;
#endif

extern uint32_t gDrainTimerIntrCnt;
extern uint32_t gDrainTimerValue;
extern uint32_t gWakeUp0IrqCount;
//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

void fpsCpu2::MsgHandleFpStsChange(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataFpStsChange_t* ptmpData = (CP2FPMsgDataFpStsChange_t*)(pMsgSQContext->data);
    Fastpath_Status_t changeStatus = (Fastpath_Status_t)(ptmpData->ChangeSts);

    if (sts == msgInvalidField)
    {
        pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpFpStsChange & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
    } // else do nothing

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpStsStart:
        {
            pMsgState[cp2FpMsgIdx] = stateFpStsMsgtoCpu0;
            switch (changeStatus)
            {
                case FP_STS_NORMAL_BOOT:
                {
                    break;
                }
                case FP_STS_FP_START:
                {
                    FpsCpuStart();
                    break;
                }
                default:
                {
                    pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
                    pMsgSQContext->sts = msgInvalidField;
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid fp sts, sts:0x%X msgOp:0x%X\n", (((msgOpFpStsChange & 0xFF) << 0x18UL) | (changeStatus & 0xFF))), "24,8");
                    break;
                }
            }
        }
        case stateFpStsMsgtoCpu0:
        {
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateFpStsMsgtoCpu1;
            }
            else
            {
                break;
            }
        }
        case stateFpStsMsgtoCpu1:
        {
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateFpStsWaitOtherCpuDone;
            }
            else
            {
                break;
            }
        }
        case stateFpStsWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateFpStsCpu2Handle;
            }
            else
            {
                break;
            }
        }
        case stateFpStsCpu2Handle:
        {
            uint8_t change = 1;
            uint8_t done = 1;
            pMsgSQContext->sts = FpsCpuHandleStatusChange(changeStatus, change, &done);
            if (!done)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid fp sts change, sts:0x%X msgOp:0x%X\n", (((msgOpFpStsChange & 0xFF) << 0x18UL) | (changeStatus & 0xFF))), "24,8");
                pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
                break;
            } // else do nothing

            pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
            break;
        }
        case stateFpStsSendFP2CP:
        {
            //send to fp2cp if there is an empty entry in FP2CP Q set state to done
            sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Firmware Major Number. [0x%x]\n", LIONMS_FW_VER_MAJOR), "32");
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Firmware Minor Number. [0x%x]\n", LIONMS_FW_VER_MINOR), "32");
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Firmware OEM Number. [0x%x]\n", LIONMS_FW_VER_OEM), "32");
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Firmware Version Build. [0x%x]\n", LIONMS_FW_VER_BUILD), "32");
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpFpStsChange, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

void fpsCpu2::MsgHandleErrQset(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgOpErrQSet_t* pErrQSetData = (CP2FPMsgOpErrQSet_t*)(pMsgSQContext->data);
    switch (pErrQSetData->subOp)
    {
        #ifdef LIONPERF_SUPPORT
        case msgSubOpAdminAbort:
        {
            MsgErrQSetSubOpAdminAbort(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
            break;
        }
        #endif
        case msgSubOpKeyVaultReload:
        {
            if(pMsgSQContext->resp)
            {
                //Response from HSM received. Disable the Wake Up Timer.
                VicIrqDisable(TCON_INT_WAKE_TIMER_0_NUM);
                ClrPendingIrq(TCON_INT_WAKE_TIMER_0_NUM);
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleErrQset Received response from HSM _cdmaFatalErrorHandleState = %d \n", _cdmaFatalErrorHandleState), "32");
                if (_cdmaFatalErrorHandleState == cFatalErrorHandlingWaitCPReWriteKey)
                {
                    API_CDMAResume();
                    _cdmaFatalErrorHandleState = cFatalErrorHandlingNotifyCpu0;
                }
                else if (_cdmaCorrectableKeyErrorHandleState == cCorrtableErrorhandlingWaitCPResp)
                {
                    _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingNotifyCpu1ResumeIO;
                    pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
                } //else do nothing
            }
            else
            {
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleErrQset Received request from HSM pMsgSQContext->resp = %d\n", pMsgSQContext->resp), "32");
                FpsCpu2PrintInfoLogInvMsgState(msgOpErrQSet, pMsgState[cp2FpMsgIdx]);
            }
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpErrQSet, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

#ifdef LIONPERF_SUPPORT
void fpsCpu2::MsgErrQSetSubOpAdminAbort(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgAdminAbort_t* pAbortMsg = (CP2FPMsgAdminAbort_t*)(pMsgSQContext->data);

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateAbortStart:
        {
            pAbortMsg->adminAbortCompleted = 0;
            pAbortMsg->abortSts = abortInvalid;
            if ((_pSlotFlagSts[pAbortMsg->ibQId] & cStsDelete) || (_pSlotFlagSts[pAbortMsg->ibQId] & cStsTearDown))
            {
                pAbortMsg->abortSts = abortFailed;
                pMsgSQContext->sts = (_pSlotFlagSts[pAbortMsg->ibQId] & cStsDelete) ? msgQueueIsDelete : msgVFIsTeardown;
                pMsgState[cp2FpMsgIdx] = stateAbortSendFP2CP;
                break;
            }
            pMsgState[cp2FpMsgIdx] = stateAbortMsgToCpu0;
        }

        case stateAbortMsgToCpu0:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateAbortMsgToCpu1;
            }
            else
            {
                return;
            }
        }

        case stateAbortMsgToCpu1:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateAbortSendFP2CP;
            }
            else
            {
                return;
            }
        }

        case stateAbortSendFP2CP:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (pAbortMsg->abortSts == abortSuccess)
                {
                    if (pAbortMsg->adminAbortCompleted) // waiting for CPU2 receive abort completed
                    {
                        pMsgSQContext->sts = msgSuccess;
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    pAbortMsg->abortSts = abortFailed;
                    if (pMsgSQContext->sts == msgQueueIsDelete)
                    {
                        // TBD: error level logging
                        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("sq is being deleted, msgSubOp:0x%X msgOp:0x%X\n", (((msgOpErrQSet & 0xFF) << 0x18UL) | (msgSubOpAdminAbort & 0xFF))), "24,8");
                    }
                    else if (pMsgSQContext->sts == msgVFIsTeardown)
                    {
                        // TBD: error level logging
                        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("vf is being teardown, msgSubOp:0x%X msgOp:0x%X\n", (((msgOpErrQSet & 0xFF) << 0x18UL) | (msgSubOpAdminAbort & 0xFF))), "24,8");
                    }
                    else
                    {
                        pMsgSQContext->sts = msgCmdNotFound;
                        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("admin abort: io cmd not found, cid:0x%X ibqId:0x%X vfId:0x%X\n", (((pAbortMsg->vfId & 0xFF) << 0x18UL) | ((pAbortMsg->ibQId & 0xFF) << 0x10UL) | (pAbortMsg->cmdId & 0xFFFF))), "16,8,8");
                    }
                }

                CP2FPMsgSts sts = msgSuccess;
                sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId);
                if (sts != msgNoEmptyEntry)
                {
                    pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
                }
            }

            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }

        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgSubOpAdminAbort, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

void fpsCpu2::MsgHandleFpModeChange(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    if (sts == msgInvalidField)
    {
        pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpFpModeChange & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
    } // else do nothing

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpModeChangeStart:
        {
            CP2FPMsgDataVfModeChange_t* ptmpData = (CP2FPMsgDataVfModeChange_t*)(pMsgSQContext->data);
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleFpModeChange : stateFpModeChangeStart, ptmpData->VFMode [0x%X]\n", ptmpData->VFMode), "32");
            if (ptmpData->VFMode > FP_MODE_STRICT)
            {
                pMsgSQContext->sts = msgInvalidField;
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid field, VFMode:0x%X msgOp:0x%X\n", (((msgOpFpModeChange & 0xFF) << 0x18UL) | (ptmpData->VFMode & 0xFF))), "24,8");
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
                break;
            }
            pMsgState[cp2FpMsgIdx] = stateFpModeChangeMsgtoCpu1;
        }
        case stateFpModeChangeMsgtoCpu1:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateFpModeChangeWaitWaitOtherCpuDone;
            } // else do nothing
            break;
        }
        case stateFpModeChangeWaitWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateFpModeChangeWaitSendFP2CP;
            }
            else
            {
                break;
            }
            break;
        }
        case stateFpModeChangeWaitSendFP2CP:
        {
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpFpModeChange, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif

void fpsCpu2::MsgHandleFpVfSlotSq2CqMapUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataVfSlotSq2CqMapUpdate_t* ptmpData = (CP2FPMsgDataVfSlotSq2CqMapUpdate_t*)pMsgSQContext->data;
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;

    uint8_t SqPId = ptmpData->SqPId;

    if ((SqPId >= UCD_FP_IO_Q_NUM))
    {
        DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("MsgHandleFpVfSlotSq2CqMapUpdate: SqPId out of range, SqPId:0x%X\n", SqPId), "32");
        pMsgSQContext->sts = msgQueueOutOfRange;
        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
    }


    if (sts == msgInvalidField)
    {
        pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpVfSlotSQ2CQMapUpdate & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
    } // else do nothing

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpVfSlotSq2CqMapUpdateStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("stateFpVfSlotSq2CqMapUpdateStart, ptmpData->VFId[0x%X], ptmpData->Action[0x%X]\n", ptmpData->VFId | (ptmpData->Action << 0x10UL)), "16,16");
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("stateFpVfSlotSq2CqMapUpdateStart, ptmpData->SqPId[0x%X], ptmpData->CqPId[0x%X]\n", ptmpData->SqPId | (ptmpData->CqPId << 0x10UL)), "16,16");
            bool msgIsFail = ChkFpVfSlotSq2CqMapUpdate(pMsgSQContext);
            if (!msgIsFail)
            {
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateMsgtoCpu0;
            }
            else
            {
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
                break;
            }
        }
        case stateFpVfSlotSq2CqMapUpdateMsgtoCpu0:
        {
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitCpu0Done;
            } // else do nothing
            break;
        }
        case stateFpVfSlotSq2CqMapUpdateWaitCpu0Done:
        {
            uint8_t SqPId = ptmpData->SqPId;
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (ptmpData->Action == cActionForceCompletion)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitForceComplDone;
                    break;
                }
                else if (ptmpData->Action == cActionRemove)
                {
                    if (!(_pSlotFlagSts[SqPId] & cStsDelete))   ///< no any running IO
                    {
                        uint8_t qbIdx = SQ_PID_2_QBIDX(SqPId);
                        if ((_pIbQ2ObQ[QBIDX_2_HIGH_SQ_PID(qbIdx)] == QID_INVALID) && (_pIbQ2ObQ[QBIDX_2_LOW_SQ_PID(qbIdx)] == QID_INVALID))
                        {
                            pMsgSQContext->sts = msgNotifyCpu1;
                            pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateMsgtoCpu1;
                        }
                        else
                        {
                            pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
                        }
                    }
                    else   ///< running IO
                    {
                        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateMsgtoCpu1;
                    }
                    break;
                }
                else   ///< cActionCreate
                {
                    if (pMsgSQContext->sts == msgNotifyCpu1)
                    {
                        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateMsgtoCpu1;
                    }
                    else
                    {
                        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
                    }
                    break;
                }
            }
            else
            {
                break;
            }
        }
        case stateFpVfSlotSq2CqMapUpdateMsgtoCpu1:
        {
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                if (ptmpData->Action == cActionRemove)
                {
                    if (pMsgSQContext->sts == msgNotifyCpu1)
                    {
                        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitCpu1UpdateCreditDone;
                    }
                    else
                    {
                        pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitCpu1Done;
                    }
                }
                else   ///< cActionCreate
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitCpu1UpdateCreditDone;
                }
            } // else do nothing
            break;
        }
        case stateFpVfSlotSq2CqMapUpdateWaitCpu1Done:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (ptmpData->Action == cActionRemove)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateWaitDeleteQDone;
                    pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap = 0;
                    pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap = 0;
                } // else do nothing
            }
            else
            {
                break;
            }
        }
        case stateFpVfSlotSq2CqMapUpdateWaitDeleteQDone:
        {
            uint8_t SqPId = ptmpData->SqPId;
            // polling delete q sts to make sure no any runnning IO
            if (_pIbQ2ObQ[SqPId] == QID_INVALID)
            {
                uint8_t qbIdx = SQ_PID_2_QBIDX(SqPId);
                if ((_pIbQ2ObQ[QBIDX_2_HIGH_SQ_PID(qbIdx)] == QID_INVALID) && (_pIbQ2ObQ[QBIDX_2_LOW_SQ_PID(qbIdx)] == QID_INVALID))
                {
                    pMsgSQContext->sts = msgNotifyCpu1;
                    pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateMsgtoCpu1;
                }
                else
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
                }
            } // else do nothing
            break;
        }
        case stateFpVfSlotSq2CqMapUpdateWaitCpu1UpdateCreditDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (pMsgSQContext->sts == msgNotifyCpu1)
                {
                    pMsgSQContext->sts = msgSuccess;
                }
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
            }
            break;
        }
        case stateFpVfSlotSq2CqMapUpdateWaitForceComplDone:
        {
            uint8_t SqPId = ptmpData->SqPId;
            if (!(_pSlotFlagSts[SqPId] & cStsForceCompletion))
            {
                pMsgState[cp2FpMsgIdx] = stateFpVfSlotSq2CqMapUpdateSendFP2CP;
            }
            else
            {
                break;
            }
        }
        case stateFpVfSlotSq2CqMapUpdateSendFP2CP:
        {
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpVfSlotSQ2CQMapUpdate, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

#ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
void fpsCpu2::MsgHandleUpdateTimestampAddr(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sendSts = msgSuccess;
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateUpdateTimestampAddrStart:
        {
            CP2FPMsgDataUpdateTimestampAddr_t* ptmpData = (CP2FPMsgDataUpdateTimestampAddr_t*)(pMsgSQContext->data);
            gTimeSyncDone = 0;
            pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrMsgtoCpu0;
        }
        case stateUpdateTimestampAddrMsgtoCpu0:
        {
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrWaitCpu0Done;
            } // else do nothing
            break;
        }
        case stateUpdateTimestampAddrWaitCpu0Done:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrMsgtoCpu1;
            }
            else
            {
                break;
            }
        }
        case stateUpdateTimestampAddrMsgtoCpu1:
        {
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrWaitOtherCpuDone;
            } // else do nothing
            break;
        }
        case stateUpdateTimestampAddrWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrCpu2Handle;
                gTimerCounterBase = (readl(REG_GLOBAL_SYNC_COUNTER_LO)) & SYSTICK_MASK;
                writel(0x0, REG_SYSTICK_CONTROL_STATUS);
                writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
                writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
                writel(0x7, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
            }
            else
            {
                break;
            }
        }
        case stateUpdateTimestampAddrCpu2Handle:
        {
            if (gTimeSyncDone)
            {
                pMsgState[cp2FpMsgIdx] = stateUpdateTimestampAddrSendFP2CP;
            }
            else
            {
                break;
            }
        }
        case stateUpdateTimestampAddrSendFP2CP:
        {
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpUpdateTimestampAddr, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif

void fpsCpu2::MsgHandleFpVfUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataVfUpdate_t* ptmpVfUpdate = (CP2FPMsgDataVfUpdate_t*)pMsgSQContext->data;
    uint8_t VFId = ptmpVfUpdate->VFId;
    uint8_t bitIdx = GET_VF_IDX(VFId);
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;

    if (sts == msgInvalidField)
    {
        pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpVfUpdate & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
    } // else do nothing

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpVfUpdateStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleFpVfUpdate MsgHandleFpVfUpdate: ptmpVfUpdate->VFId [0x%X], ptmpVfUpdate->Action [0x%X]\n", ptmpVfUpdate->VFId |(ptmpVfUpdate->Action << 0x10UL)), "16,16");
            bool msgIsFail = ChkFpVfUpdate(pMsgSQContext);
            if (!msgIsFail)
            {
                pMsgState[cp2FpMsgIdx] = stateFpVfUpdateMsgtoCpu0;
            }
            else
            {
                pMsgState[cp2FpMsgIdx] = stateFpVfUpdateSendFP2CP;
                break;
            }
        }
        case stateFpVfUpdateMsgtoCpu0:
        {
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateFpVfUpdateWaitCpu0Done;
            } // else do nothing
            break;
        }
        case stateFpVfUpdateWaitCpu0Done:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (ptmpVfUpdate->Action == cActionTearDown)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfUpdateMsgtoCpu1;
                }
                else
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfUpdateSendFP2CP;
                    break;
                }
            }
            else
            {
                break;
            }
        }
        case stateFpVfUpdateMsgtoCpu1:
        {
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateFpVfUpdateWaitCpu1Done;
            } // else do nothing
            break;
        }
        case stateFpVfUpdateWaitCpu1Done:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                if (ptmpVfUpdate->Action == cActionTearDown)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfUpdateWaitVfTeardown;
                    pMsgSQContext->sts = msgInProgress;
                }
                else
                {
                    pMsgState[cp2FpMsgIdx] = stateFpVfUpdateSendFP2CP;
                }

            } // else do nothing
            break;
        }
        case stateFpVfUpdateWaitVfTeardown:
        {
            uint64_t vfEnBitmap = GetVFBitmap(VFId);
            if (!(vfEnBitmap & BIT_ULL(bitIdx)))
            {
                // HandleTeardownBitMap(VFId, bitIdx, false);
                pMsgSQContext->sts = msgSuccess;
                pMsgState[cp2FpMsgIdx] = stateFpVfUpdateSendFP2CP;
            } // else do nothing
            break;
        }
        case stateFpVfUpdateSendFP2CP:
        {
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpVfUpdate, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

void fpsCpu2::MsgHandleCpCdmaIo(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)PSRAM_CP_DFL_BUF_ADDR; //list2 host sqe
    LionNvmeSQDescriptor_t* pHostCmd = (LionNvmeSQDescriptor_t*)&(pFpCmd->sqe);

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateCpCdmaIoStart:
        {
            if (unlikely(pHostCmd->SrcDataLen & HOST_CMD_16B_ALIGN_MASK))
            {
                CPCDMAIOStatus = CDMA_CMD_ERROR;
                pMsgState[cp2FpMsgIdx] = stateCpCdmaIoSendFP2CP;
            }
            pMsgState[cp2FpMsgIdx] = stateCpCdmaIoMsgtoCpu1;
        }

        case stateCpCdmaIoMsgtoCpu1:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);

            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateCpCdmaIoWaitOtherCpuDone;
            }

            break;
        }

        case stateCpCdmaIoWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateCpCdmaIoWaitCdmaDone;
            }

            break;
        }

        case stateCpCdmaIoWaitCdmaDone:
        {
            // polling cdma done
            if (CPCDMAIODone == 1)
            {
                pMsgState[cp2FpMsgIdx] = stateCpCdmaIoSendFP2CP;
            }

            break;
        }

        case stateCpCdmaIoSendFP2CP:
        {
            CP2FPMsgDataOpCpCdmaIoResp_t* ptmpCpCdmaIo = (CP2FPMsgDataOpCpCdmaIoResp_t*)pMsgSQContext->data;
            CP2FPMsgSts sts = msgSuccess;
            pMsgSQContext->length = 3;
            ptmpCpCdmaIo->cmdListIdx = 0;
            ptmpCpCdmaIo->cmdListNum = 2;
            ptmpCpCdmaIo->cmdStatus = CPCDMAIOStatus;

            if (CPCDMAIOStatus & CP_CMD_ERR_MASK)
            {
                pMsgSQContext->sts = msgInvalidField;
            }

            //DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CPCDMAIOStatus = 0x%x, pMsgSQContext->sts = 0x%x ", CPCDMAIOStatus, pMsgSQContext->sts), "32", "32");
            CPCDMAIODone = 0;



            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;

                // clear the status once the msg send to CP
                CPCDMAIOStatus = 0;
                writel(0, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
                writel(0, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
            }

            DMB();

            break;
        }

        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }

        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpCpCdmaIo, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
    #else
        MsgNotSupport(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
    #endif
}

void fpsCpu2::MsgHandleKeyUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateKeypdateStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleFpVfUpdate stateKeypdateStart: ptmpVfUpdate->VFId [0x%X], ptmpVfUpdate->Action [0x%X]\n", ptmpVfUpdate->VFId |(ptmpVfUpdate->Action << 0x10UL)), "16,16");
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleFpVfUpdate stateKeypdateStart: ptmpVfUpdate->keySubIndex [0x%X], ptmpVfUpdate->resourceGroupId [0x%X]\n", ptmpVfUpdate->keySubIndex |(ptmpVfUpdate->resourceGroupId << 0x10UL)), "16,16");
            CP2FPMsgDataKeyUpdate_t* ptmpData = (CP2FPMsgDataKeyUpdate_t*)(pMsgSQContext->data);
            pMsgState[cp2FpMsgIdx] = stateKeypdateMsgtoCpu1;
        }
        case stateKeypdateMsgtoCpu1:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateKeypdateWaitOtherCpuDone;
            } // else do nothing
            break;
        }
        case stateKeypdateWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateKeypdateWaitSendFP2CP;
            }
            else
            {
                break;
            }
        }
        case stateKeypdateWaitSendFP2CP:
        {
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpKeyUpdate, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#ifdef LIONPERF_SUPPORT
void fpsCpu2::MsgHandleSetLogLevel(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sendSts = msgSuccess;

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateSetLogLevelStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleSetLogLevel stateSetLogLevelStart: ptmpVfUpdate->LogLevel [0x%X]\n", ptmpVfUpdate->LogLevel), "32");
            CP2FPMsgDataSetLogLevel_t* ptmpData = (CP2FPMsgDataSetLogLevel_t*)(pMsgSQContext->data);
            pMsgState[cp2FpMsgIdx] = stateSetLogLevelCpu2Handle;
        }
        case stateSetLogLevelCpu2Handle:
        {
            HandleOpSetLogLevel(pMsgSQContext);
            pMsgState[cp2FpMsgIdx] = stateSetLogLevelMsgtoCpu0;
        }
        case stateSetLogLevelMsgtoCpu0:
        {
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateSetLogLevelMsgtoCpu1;
            }
            else
            {
                break;
            }
        }
        case stateSetLogLevelMsgtoCpu1:
        {
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateSetLogLevelWaitOtherCpuDone;
            } // else do nothing
            break;
        }
        case stateSetLogLevelWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateSetLogLevelSendFP2CP;
            }
            else
            {
                break;
            }
        }
        case stateSetLogLevelSendFP2CP:
        {
            //send to fp2cp if there is an empty entry in FP2CP Q set state to done
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId) CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpSetLogLevel, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif
void fpsCpu2::MsgHandleSendHsmReq(uint32_t msgOpErrQsetSubOp)
{
    CP2FPMsgSts sts = msgSuccess;

    CP2FPMsgOpErrQSet_t* pErrQSetData;
    pErrQSetData->subOp = msgOpErrQsetSubOp;
    CP2FPMsgContext_t msgInfo;
    msgInfo.msgOp = msgOpErrQSet; //msgOpErrQSet opcode of FP to HSM request channel is used to indicate any error condition on FP, that HSM needs to know about.
    msgInfo.resp = 0;
    msgInfo.tag = 0;
    msgInfo.sts = msgSuccess;
    msgInfo.length = FP_TO_CP_ERR_MSG_LEN;
    M7_MEM_COPY(msgInfo.data, pErrQSetData, FP_TO_CP_ERR_MSG_LEN);
    uint8_t* pData = (uint8_t*)&msgInfo.data;

    DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Detected correctable error on CDMA, Sending message to HSM to reload key vault, [0x%X]\n", 1), "32");

    ClrPendingIrq(TCON_INT_WAKE_TIMER_0_NUM);
    // Enable the Wake Up Timer. Will only be hit in case of IPC timeout. Else will be disabled when IPC message is received.
    VicIrqEnable(TCON_INT_WAKE_TIMER_0_NUM);
    gWakeUp0IrqCount = MAX_WAKEUP0_HSM_IRQ_TIMEOUT_COUNT;

    sts = SendFP2CPMsg(&msgInfo, pData, ReqMsg,  msgInfo.length, FPToCP1_Req);

    if( sts != msgSuccess)
    {
        DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("Send FP to CP msg failed with, status:0x%X\n", sts), "32");
    }
}

void fpsCpu2::SetupHardwareOffloadRegister(uint8_t ucdQueueID, uint8_t ibIndex, uint8_t obIndex, uint8_t dflIndex, uint8_t oslIndex)
{
    // IBCQ
    rFps->fpsHwe2fpRegRegister[ibIndex].fpsHwe2fpHwEngineToFpQCiIndirectAddressPortHwe2fpQCiIndirectRegAddr = \
        (uint32_t)(&(fpsCpu2::pIbCmnReg[UCD_CORE_1]->ucdIbCmnCqRegisters[ucdQueueID].ucdIbCmnCqCompletionQueueCi.all));

    // OBCQ
    rFps->fpsHwe2fpRegRegister[obIndex].fpsHwe2fpHwEngineToFpQCiIndirectAddressPortHwe2fpQCiIndirectRegAddr = \
        (uint32_t)(&(fpsCpu2::pObCmnReg[UCD_CORE_1]->ucdObCmnCqRegisters[ucdQueueID].ucdObCmnCqOutboundCompletionQueueCi.all));

    // DFL pi
    rFps->fpsSocFwdRegRegisters[dflIndex].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr = \
        (uint32_t)(&(fpsCpu2::pIbCmnReg[UCD_CORE_1]->ucdIbCmnDflRegisters[ucdQueueID].ucdIbCmnDflInboundDestinationFreeListPi.all));

    // OSL pi
    rFps->fpsSocFwdRegRegisters[oslIndex].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr = \
        (uint32_t)(&(fpsCpu2::pObCmnReg[UCD_CORE_1]->ucdObCmnOslRegisters[ucdQueueID].ucdObCmnOslOutboundSourceListPi.all));

    // update DFL pi
    if(gResetType == cPor)
    {
        uint8_t queueDepth = (ucdQueueID == UCD_QUEUE_0) ? M7_IO_QUEUE_DEPTH_MASK : M7_IO_QUEUE_1_DEPTH_MASK;
        writel(queueDepth, readl(&rFps->fpsSocFwdRegRegisters[dflIndex].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr));
    }

    DMB();

}

void fpsCpu2::MsgHandleFpUcdQuery(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgDataUcdQuery_t* pFpUcdQueryData = (CP2FPMsgDataUcdQuery_t*)M7_FPS_CPU2_UCD_QUERY_DATA;
    CP2FPMsgDataUcdQuery_t* pQueryData = (CP2FPMsgDataUcdQuery_t*)(pMsgSQContext->data);

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpUcdQueryStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleFpUcdQuery, pMsgState[cp2FpMsgIdx][0x%X]\n", pMsgState[cp2FpMsgIdx]), "32");
            pMsgState[cp2FpMsgIdx] = stateFpUcdQueryFillQuery;
        }
        break;

        case stateFpUcdQueryFillQuery:
        {
            M7_MEM_COPY((void*)pFpUcdQueryData, pMsgSQContext->data, pMsgSQContext->length);

            uint8_t ucdCoreID = pQueryData->ucdCoreID;
            uint8_t ucdQueueID = pQueryData->queueID;

            if ((ucdCoreID != UCD_CORE_1) || (ucdQueueID > UCD_QUEUE_1))
            {
                pMsgSQContext->sts = msgInvalidField;
                pMsgState[cp2FpMsgIdx] = stateFpUcdQuerySendFP2CP;
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpUcdQuery, ucdCoreID[0x%X]\n", ucdCoreID), "32");
                break;
            }

            if (ucdQueueID == UCD_QUEUE_0)
            {
                UcdQueryPara_t ucdQryParam = {MAX_SUPPORT_CMD_NUM,
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_DFL_LIST_0_ADDR),
                                              getCPU2TCMPhysicalAddress((uint32_t)M7_FPS_CPU2_OSL_LIST_0_ADDR),
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_IBCQ_0_ADDR),
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_OBCQ_0_ADDR),
                                              getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR),
                                              getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsHwe2fpRegRegister[cHwe2FpWq00UcdIbCq0].fpsHwe2fpHwEngineToFpQPiShadow.all))),
                                              getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQPiShadow.all)))
                };

                M7_MEM_COPY((void*)pFpUcdQueryData, (void*)(&ucdQryParam), sizeof(UcdQueryPara_t));

                SetupHardwareOffloadRegister(ucdQueueID, cHwe2FpWq00UcdIbCq0, cHwe2FpWq02UcdObCq0, cFpSocFwd00Ucd1Dfl0, cFpSocFwd01Ucd1Osl0);
            }
            else // UCD_QUEUE_1
            {
                UcdQueryPara_t ucdQryParam = {MAX_SUPPORT_CMD_NUM_QUEUE_1,
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_DFL_LIST_1_ADDR),
                                              getCPU2TCMPhysicalAddress((uint32_t)M7_FPS_CPU2_OSL_LIST_1_ADDR),
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_IBCQ_1_ADDR),
                                              getCPU0TCMPhysicalAddress((uint32_t)M7_FPS_CPU0_OBCQ_1_ADDR),
                                              getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR),
                                              getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsHwe2fpRegRegister[cHwe2FpWq01UcdIbCq1].fpsHwe2fpHwEngineToFpQPiShadow.all))),
                                              getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQPiShadow.all)))
                };

                M7_MEM_COPY((void*)pFpUcdQueryData, (void*)(&ucdQryParam), sizeof(UcdQueryPara_t));

                SetupHardwareOffloadRegister(ucdQueueID, cHwe2FpWq01UcdIbCq1, cHwe2FpWq03UcdObCq1, cFpSocFwd02Ucd1Dfl3, cFpSocFwd03Ucd1Osl1);
            }

            // setup OSL CI address
            _ucdObq.pHwOslCi[ucdQueueID] = \
                (uint32_t*)(&(fpsCpu2::pObCmnReg[ucdCoreID]->ucdObCmnOslRegisters[ucdQueueID].ucdObCmnOslOutboundSourceListCi.all));
            if(gResetType == cFwUpdateWarmReset)
            {
                pMsgState[cp2FpMsgIdx] = stateFpUcdQueryRestore;
            }
            else
            {
                pMsgState[cp2FpMsgIdx] = stateFpUcdQueryMsgToCpu0;
            }

            DMB();
        }
        break;

        case stateFpUcdQueryRestore:
        {
            uint8_t ucdQueueID = pQueryData->queueID;

            outBoundOSLPi[ucdQueueID] = readl(&pObCmnReg[pQueryData->ucdCoreID]->ucdObCmnOslRegisters[ucdQueueID].ucdObCmnOslOutboundSourceListPi.all);
            writel(outBoundOSLPi[ucdQueueID], _ucdObq.pHwOslPi[ucdQueueID]);

            #ifdef DISABLE_INDIRECT_REG_WRITE
            uint32_t disableBit = (ucdQueueID == OSL_0) ? SOC_REG_1_WR_BIT : SOC_REG_3_WR_BIT;
            if (readl(REG_FPS_INDIRECT_REG_WR_DISABLE) & disableBit)
            {
                writel(outBoundOSLPi[ucdQueueID], hwOslPiAddr[ucdQueueID]);
            }
            #endif

            pMsgState[cp2FpMsgIdx] = stateFpUcdQueryMsgToCpu0;
        }
        break;

        case stateFpUcdQueryMsgToCpu0:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateFpUcdQueryWaitOtherCpuDone;
            }
        }
        break;

        case stateFpUcdQueryWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateFpUcdQuerySendFP2CP;
            }
            else
            {
                break;
            }
        }

        case stateFpUcdQuerySendFP2CP:
        {
            CP2FPMsgSts sts = msgNoEmptyEntry;
            #ifdef POST_RESET_FAULT_INJECTION_SUPPORT
            if (gResetType == cFwUpdateWarmReset)
            {
                // Dont send a response to CP to simulate UCD timeout error injection upon Firmware Update.
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpUcdQuery: Not responding to UCD Query to simulate POST_RESET_FAULT_INJECTION_SUPPORT when reset type = 0x%x\n", gResetType),"32");
            }
            else
            {
            #endif
                sts = SendFP2CPMsg(pMsgSQContext, (uint8_t*)pFpUcdQueryData, RespMsg, TYPE_OFFSET(CP2FPMsgDataUcdQuery_t, reserved), (CPMsgQId)CPSrcId);
            #ifdef POST_RESET_FAULT_INJECTION_SUPPORT
            }
            #endif
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }

            break;
        }

        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }

        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpUcdQuery, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

#ifdef LOGGING_NEW_SCHEME
void fpsCpu2::MsgHandleTelemetryQuery(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    uint8_t data[PSRAM_FP2CP_MSG_DATA_SIZE];
    CP2FPMsgDataTelemetryQuery_t* pQueryData = (CP2FPMsgDataTelemetryQuery_t*)(pMsgSQContext->data);
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateTelemetryQueryStart:
        {
            if (pQueryData->subOp == msgSubOpLogBufferingAddresses)
            {
                pMsgState[cp2FpMsgIdx] = stateTelemetryQuerySubOpBufferingAddresses;
            }
            else if (pQueryData->subOp == msgSubOpFirmwareInformation)
            {
                pMsgState[cp2FpMsgIdx] = stateTelemetryQuerySubOpFirmwareInformation;
            }
            else if (pQueryData->subOp == msgSubOpTelemetryCounters)
            {
                pMsgState[cp2FpMsgIdx] = stateTelemetryQuerySubOpTelemetryCounters;
            }
            #ifdef WEIGHT_ROUND_ROBIN
            else if (pQueryData->subOp == msgSubOpSetWeightRoundRobin)
            {
                pMsgState[cp2FpMsgIdx] = stateTelemetryQuerySubOpSetWeightRoundRobin;
            }
            #endif
            else
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid field, subOp:0x%X msgOp:0x%X\n", (((msgOpTelemetryQuery & 0xFF) << 0x18UL) | (pQueryData->subOp & 0xFF))), "24,8");
                pMsgSQContext->sts = msgInvalidField;
                pMsgState[cp2FpMsgIdx] = stateTelemetryQuerySendFP2CP;
            }
            break;
        }
        case stateTelemetryQuerySubOpBufferingAddresses:
        {
            CP2FPMsgLogBufferingAddresses_t* ptmpData = (CP2FPMsgLogBufferingAddresses_t*)data;
            M7_MEM_COPY(ptmpData, pMsgSQContext->data, pMsgSQContext->length);
            ptmpData->logBuffer0AddrFpsCpu0 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core0)));
            ptmpData->logBuffer1AddrFpsCpu0 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core0) + LOG_BUFFER_SIZE));
            ptmpData->logBuffer0AddrFpsCpu1 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core1)));
            ptmpData->logBuffer1AddrFpsCpu1 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core1) + LOG_BUFFER_SIZE));
            ptmpData->logBuffer0AddrFpsCpu2 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core2)));
            ptmpData->logBuffer1AddrFpsCpu2 = getpSRAMPhysicalAddress((uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core2) + LOG_BUFFER_SIZE));
            #ifdef SUPPORT_FPS_REGISTER
            ptmpData->gdmaInsCiShadowFpsCpu0 = \
                getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQCiShadow.all)));
            ptmpData->gdmaInsCiShadowFpsCpu1 = \
                getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQCiShadow.all)));
            ptmpData->gdmaInsCiShadowFpsCpu2 = \
                getFPSRegPhysicalAddress((uint32_t)(&(rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQCiShadow.all)));
            #endif

            // send fp2cp directly due to local var data
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, data, RespMsg, sizeof(CP2FPMsgLogBufferingAddresses_t), (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        case stateTelemetryQuerySubOpFirmwareInformation:
        {
            #ifdef SUPPORT_TELEMETRY
            CP2FPMsgFirmwareInformation_t* ptmpData = (CP2FPMsgFirmwareInformation_t*)data;

            M7_MEM_COPY(ptmpData, pMsgSQContext->data, pMsgSQContext->length);
            ptmpData->fwverBuildNo = LIONMS_FW_VER_BUILD;
            ptmpData->fwverOemNo = LIONMS_FW_VER_OEM;
            ptmpData->fwverMinorNo = LIONMS_FW_VER_MINOR;
            ptmpData->fwverMajorNo = LIONMS_FW_VER_MAJOR;
            ptmpData->fpmode = *(uint8_t*)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_FP_MODE_ADDR);
            ptmpData->loggingLevel = cLogInfo; //LoggingLevelAtRunTime();
            //ptmpData->logDataStatus = LogdataTransferAlreadyEnabled();
            ptmpData->resv = 0;
            ptmpData->injecCountNonFaultErr = TcInjecCountNonFatalErr;
            ptmpData->injecCountFaultErr = TcIInjecCountFatalErr;
            ptmpData->injecCountPoorSgl = TcInjecCountPoorSgl;
            ptmpData->systickThreshold = SYSTICK_THRESHOLD;

            // send fp2cp directly due to local var data
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, data, RespMsg, sizeof(CP2FPMsgFirmwareInformation_t), (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            #endif
            break;
        }
        case stateTelemetryQuerySubOpTelemetryCounters:
        {
            #ifdef SUPPORT_TELEMETRY
            CP2FPMsgTelemetryCounters_t* ptmpData = (CP2FPMsgTelemetryCounters_t*)data;
            M7_MEM_COPY(ptmpData, pMsgSQContext->data, pMsgSQContext->length);
            ptmpData->outstandingIOCnt = *(uint32_t*)CPU2_ACCESS_SHARE_TCM20_FROM_CPU0(M7_FPS_CPU20_OUTSTANDING_IO_CNT_ADDR);
            ptmpData->accumulateIOCnt = *(uint64_t*)CPU2_ACCESS_SHARE_TCM20_FROM_CPU0(M7_FPS_CPU20_ACCUMULATE_IO_CNT_ADDR);
            ptmpData->faultErrCnt = TcFaultErrCnt;
            ptmpData->nonFaultErrCnt = TcNonFaultErrCnt;
            ptmpData->poorConstructedSglCnt = TcPoorConstructedSglCnt;

            // send fp2cp directly due to local var data
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, data, RespMsg, sizeof(CP2FPMsgTelemetryCounters_t), (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            #endif
            break;
        }
        #ifdef WEIGHT_ROUND_ROBIN
        case stateTelemetryQuerySubOpSetWeightRoundRobin:
        {
            CP2FPMsgTelemetrySetWeightRoundRobin* ptmpData = (CP2FPMsgTelemetrySetWeightRoundRobin*)data;
            M7_MEM_COPY(ptmpData, pMsgSQContext->data, pMsgSQContext->length);

            if (ptmpData->configOp == 0) // Read weight and send back to CP
            {
                uint32_t exp = 0;
                if (*_pWeightRoundRobin > 0)
                {
                    exp = 31 - __builtin_clz(*_pWeightRoundRobin);
                }
                ptmpData->weightExp = exp;
            }
            else if (ptmpData->configOp == 1) // Write weight to Shared DTCM CPU12 for QMGR
            {
                if (ptmpData->weightExp < MAX_WEIGHT_EXP)
                {
                    *_pWeightRoundRobin = BIT(ptmpData->weightExp);
                }
            }

            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, data, RespMsg, sizeof(CP2FPMsgTelemetrySetWeightRoundRobin), (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        #endif
        case stateTelemetryQuerySendFP2CP:
        {
            //CP2FPMsgContext_t msgInfo;
            //M7_MEM_COPY(&msgInfo, pMsgSQContext, sizeof(CP2FPMsgContext_t));//copy message
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, data, RespMsg, sizeof(CP2FPMsgDataTelemetryQuery_t), (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpTelemetryQuery, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

#ifdef LIONPERF_SUPPORT
static uint32_t GetGdmaDqPiRegAddr(uint32_t gdmaInsFps)
{
    uint32_t gdmaDqPiRegAddrBase = 0;
    uint32_t offset = 0;

    // gdmaInsFps 0 ~ 3: 0x4C, 0x6C, 0x8C, 0xAC
    // gdmaInsFps 4 ~ 7: 0x24C, 0x26C, 0x28C, 0x2AC
    if (gdmaInsFps < 4)
    {
        gdmaDqPiRegAddrBase = (uint32_t)(&(rGdma->gdmaGdmaDeliveryQueue0ProducerIndex.all));
        offset = ((uint32_t)GDMA_REG_DELIVERY_QUEUE_OFFSET * gdmaInsFps);
        return (gdmaDqPiRegAddrBase + offset);
    }
    else
    {
        gdmaDqPiRegAddrBase = (uint32_t)(&(rGdma->gdmaGdmaDeliveryQueue4ProducerIndex.all));
        offset = ((uint32_t)GDMA_REG_DELIVERY_QUEUE_OFFSET * (uint32_t)(gdmaInsFps - 4));
        return (gdmaDqPiRegAddrBase + offset);
    }
}

void fpsCpu2::MsgHandleLogEnDisUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataLogEnDisUpdate_t* pData = (CP2FPMsgDataLogEnDisUpdate_t*)(pMsgSQContext->data);
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateLogEnDisUpdateStart:
        {
            if ((LogdataTransferAlreadyEnabled() && (pData->action == 1)) || (!LogdataTransferAlreadyEnabled() && (pData->action == 0)))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid field, action:0x%X msgOp:0x%X\n", (((msgOpLogEnDisUpdate & 0xFF) << 0x18UL) | (pData->action & 0xFF))), "24,8");
                sendSts = msgInvalidField;
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateSendFP2CP;
                break;
            }
            else if (pData->action == 0)
            {
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateCpu2UpdateLogExt;
                break;
            } // else do nothing
            else
            {
                // else do nothing
            }
            if ((pData->gdmaInsFpsCpu0 >= GDMA_INSTANCE_NUM) || (pData->gdmaInsFpsCpu1 >= GDMA_INSTANCE_NUM) ||
                (pData->gdmaInsFpsCpu2 >= GDMA_INSTANCE_NUM))
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid gdma qid, Ins0:0x%X Ins1:0x%X Ins2:0x%X msgOp:0x%X\n", (((msgOpLogEnDisUpdate & 0xFF) << 0x18UL) | ((pData->gdmaInsFpsCpu2 & 0xFF) << 0x10UL) | ((pData->gdmaInsFpsCpu1 & 0xFF) << 0x8UL) | (pData->gdmaInsFpsCpu0 & 0xFF))), "8,8,8,8");
                pMsgSQContext->sts = msgInvalidField;
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateSendFP2CP;
                break;
            } // else do nothing
            pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateCpu2ConfigGdmaFpReg;
            break;
        }
        case stateLogEnDisUpdateCpu2ConfigGdmaFpReg:
        {
            // config local gdma var and fp reg before enable logdata trandfer
            LoggingUpdateGdmaInfo(pData->gdmaQSizeFpsCpu2, pData->piInfoFpsCpu2, pData->pingPongIndexFpsCpu2);
            #ifdef SUPPORT_FPS_REGISTER
            // Setup FP2HWE registers for GDMA delivery queue for fps cpu0~2
            rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr = GetGdmaDqPiRegAddr(pData->gdmaInsFpsCpu0);
            rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr = GetGdmaDqPiRegAddr(pData->gdmaInsFpsCpu1);
            rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr = GetGdmaDqPiRegAddr(pData->gdmaInsFpsCpu2);

            // backup for fw update
            *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core0) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET) =
                rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr;
            *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core1) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET) =
                rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr;
            *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core2) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET) =
                rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr;
            #endif
            pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateCpu2UpdateLogExt;
            break;
        }
        case stateLogEnDisUpdateCpu2UpdateLogExt:
        {
            MsgUpdateLogExt(pMsgSQContext);
            pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateMsgtoCpu0;
            break;
        }
        case stateLogEnDisUpdateMsgtoCpu0:
        {
            sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateMsgtoCpu1;
            }
            else
            {
                break;
            }
        }
        case stateLogEnDisUpdateMsgtoCpu1:
        {
            sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core1);
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateWaitOtherCpuDone;
            } // else do nothing
            break;
        }
        case stateLogEnDisUpdateWaitOtherCpuDone:
        {
            if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
            {
                pMsgState[cp2FpMsgIdx] = stateLogEnDisUpdateSendFP2CP;
            }
            else
            {
                break;
            }
        }
        case stateLogEnDisUpdateSendFP2CP:
        {
            //send to fp2cp if there is an empty entry in FP2CP Q set state to done
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpLogEnDisUpdate, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

void fpsCpu2::MsgUpdateLogExt(CP2FPMsgContext_t* pMsgSQContext)
{
    uint8_t data = (uint8_t)(pMsgSQContext->data[0]);
    LogExt_t _logExt;
    _logExt.LogEnDisUpdate = data;
    LoggingUpdateLogExt(&_logExt, cLogExtSetLogEnDisUpdate);
}
#endif
#endif

#ifdef LIONPERF_SUPPORT
void fpsCpu2::MsgHandleOpCDMAStatSet(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgDataCdmaStatSet_t* pData = (CP2FPMsgDataCdmaStatSet_t*)(pMsgSQContext->data);
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateCDMAStatSetStart:
        {
            switch (pData->action)
            {
                case cActionResumeCdma:
                {
                    if (_cdmaFatalErrorHandleState == cFatalErrorHandlingWaitCPReWriteKey)
                    {
                        API_CDMAResume();
                        _cdmaFatalErrorHandleState = cFatalErrorHandlingNotifyCpu0;
                    }
                    else if (_cdmaCorrectableKeyErrorHandleState == cCorrtableErrorhandlingWaitCPResp)
                    {
                        _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingNotifyCpu1ResumeIO;
                    } //else do nothing
                    break;
                }
                #ifdef LIONMS_B0_V2_4_3
                case cActionQoSLatencyTimer:
                {
                    API_CDMASetTimerValue(cQoSLatencyTimerValue, pData->value);
                    break;
                }
                #endif
                case cActionKeyCorrErrThreshold:
                {
                    _CDMACorrectableKeyErrorThreshold = pData->value;
                    HalCDMA_SetCorrKeyErrThreshold(pData->value);
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleOpCDMAStatSet: _CDMACorrectableKeyErrorThreshold = %d\n", _CDMACorrectableKeyErrorThreshold), "32");
                    break;
                }
                default:
                {
                    break;
                }
            }
            pMsgState[cp2FpMsgIdx] = stateCDMAStatSetSendFP2CP;
        }
        case stateCDMAStatSetSendFP2CP:
        {
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpCDMAStatSet, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif

#ifdef QOS_LATENCY_ERROR_HANDLING
void fpsCpu2::MsgHandleOpQoSPenaltySetup(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    FpsCpu2PrintInfoLogInvMsgState(msgOpQoSPenalty, pMsgState[cp2FpMsgIdx]);
}
#endif

ATTR_NO_INLINE void fpsCpu2::FpsCpu2PrintInfoLogInvMsgState(uint32_t msgOpCode, uint32_t curMsgState)
{
    //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("invalid msg state, sts:0x%X msgOp:0x%X\n", (((msgOpCode & 0xFF) << 0x18UL) | (curMsgState & 0xFF))), "24,8");
}


//-----------------------------------------------------------------------------
//  Below function handler are not list in CP2FP or FP2CP Message OpCode Definition Table
//-----------------------------------------------------------------------------

#ifdef SUPPORT_MSGERROR_INJECTION
void fpsCpu2::MsgHandleMsgErrorInjection(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    uint64_t errInjectBitmap = *pErrInjectBitmap;
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateMsgErrorInjectionStart:
        {
            uint8_t sts = msgSuccess;
            CP2FPMsgDataMsgErrorInjection_t* ptmpData = (CP2FPMsgDataMsgErrorInjection_t*)(pMsgSQContext->data);
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("MsgHandleMsgErrorInjection, errInjectCnt[0x%X]\n", (uint32_t))errInjectCnt), "32");
            //chk err inject cnt
            if (errInjectCnt < M7_ERR_INJECT_DEPTH)
            {
                sts = Cpu2AddErrInject(ptmpData, _pIbQ2ObQ, _pVFEnBitmap, _pQueueBlockInfoBase);
            }
            else
            {
                sts = msgInvalidField;
            }

            if (errInjectBitmap || (sts == msgInvalidField))
            {
                pMsgSQContext->sts = sts;
                pMsgState[cp2FpMsgIdx] = stateMsgErrorInjectionSendFP2CP;
            }
            else
            {
                pMsgState[cp2FpMsgIdx] = stateMsgErrorInjectionMsgtoCpu1;//notify cpu1 flag
            }
        }
        break;
        case stateMsgErrorInjectionMsgtoCpu1:
        {
            CP2FPMsgSts sendSts = msgSuccess;
            sendSts = SendFPMsg(cM7Core1, errInjectMsg, 0, msgSuccess, 1, CPSrcId);
            if (sendSts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = stateMsgErrorInjectionSendFP2CP;
            } // else do nothing
        }
        break;
        case stateMsgErrorInjectionSendFP2CP:
        {
            //send to fp2cp if there is an empty entry in FP2CP Q set state to done
            CP2FPMsgSts sts = msgSuccess;
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            else
            {
                break;
            }
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpMsgErrorInjection, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif

#ifdef SUPPORT_CDMA_RESET_MSG
void fpsCpu2::HandleOpCDMAReset(CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState)
{
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateCDMAResetStart:
        {
            if (_cdmaFatalErrorHandleState == cFatalErrorHandlingWait)
            {
                duringCDMAResetMessage = true;
                TriggerCDMAResetFiber();
            }
            pMsgState[cp2FpMsgIdx] = stateCDMAStatSetSendFP2CP;
            break;
        }
        case stateCDMAResetSendFP2CP:
        {
            CP2FPMsgSts sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CP0FP2_Req);
            if (sts != msgNoEmptyEntry)
            {
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            } // else do nothing
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpCDMAReset, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}
#endif

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

void fpsCpu2::MsgHandleFpShutdownRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataShutdownReq_t* ptmpData = (CP2FPMsgDataShutdownReq_t*)(pMsgSQContext->data);
    Cortexm7_t* rCortexM7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
    bool ack = true;

    if (sts == msgInvalidField)
    {
        pMsgState[cp2FpMsgIdx] = stateFpStsSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpShutdownReq & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
    } // else do nothing

    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpShutdownStart:
        {
            pMsgState[cp2FpMsgIdx] = stateFpShutdownMsgtoCpu0;
            writel(FP_STS_FP_FW_UPDATE_START, pCpuStatus);    //writing to CPU status register to indicate that FW Update command is received.
            gDrainTimerValue = ptmpData->drainTime * ARM_SYSTICK_CLOCK;
            gDrainTimerIntrCnt = 0;
            // Start Timer
            writel(0x0, &rCortexM7->systemControl.systCsr);
            writel(SYSTICK_MASK, &rCortexM7->systemControl.systRvr);
            writel(0x0, &rCortexM7->systemControl.systCvr);   //any write to current val clears it.
            writel(0x7, &rCortexM7->systemControl.systCsr);   //enable systick, no intr and use processor clock
            if((readl(&_pIbCmnReg[UCD_CORE_1]->ucdIbCmnSnglInboundUcdStatus) & UCD_IB_CMN_SNGL_STATUS_BUSY) && ack)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, UCD Unpaused pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
                ack = false;
                pMsgState[cp2FpMsgIdx] = stateFpShutdownSendFP2CP;
            }
        }
        case stateFpShutdownMsgtoCpu0:
        {
            if(ack)
            {
                sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
                if (sendSts != msgNoEmptyEntry)
                {
                    pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                    pMsgState[cp2FpMsgIdx] = stateFpShutdownWaitOtherCpuDone;
                }
                break;
            }
        }
        case stateFpShutdownWaitOtherCpuDone:
        {
            if(ack)
            {
                if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpShutdownWaitFwUpdateDone;
                }
                else
                {
                    uint32_t currentTimestamp = SYSTICK_MASK - readl(REG_SYSTICK_CURRENT_VALUE) + (gDrainTimerIntrCnt * SYSTICK_MASK);
                    // check for timer expired
                    if(gDrainTimerValue <= currentTimestamp)
                    {
                        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, FP0 response Timeout  currentTimestamp %d\n", currentTimestamp), "32");
                        // Send NACK to CP core indicating IO Drain timeout
                        ack = false;
                        pMsgState[cp2FpMsgIdx] = stateFpShutdownSendFP2CP;
                        // Stop Timer
                        Cortexm7_t* rCortexM7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
                        writel(0x0, &rCortexM7->systemControl.systCsr);
                        writel(0, &rCortexM7->systemControl.systRvr);
                        writel(0x0, &rCortexM7->systemControl.systCvr);   //any write to current val clears it.
                        gDrainTimerValue = 0;
                        gDrainTimerIntrCnt = 0;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        case stateFpShutdownWaitFwUpdateDone:
        {
            if(ack)
            {
                bool ioDone = true;
                for (uint16_t i = 0; i  < UCD_FP_IO_Q_NUM; i++)
                {
                    if (_pSlotFlagSts[i] & cStsFwUpdate)
                    {
                        ioDone = false;
                        break;
                    } // else do nothing
                }
                if (ioDone)
                {
                    // Stop timer
                    writel(0x0, &rCortexM7->systemControl.systCsr);
                    writel(0, &rCortexM7->systemControl.systRvr);
                    writel(0x0, &rCortexM7->systemControl.systCvr);   //any write to current val clears it.
                    gDrainTimerValue = 0;
                    gDrainTimerIntrCnt = 0;
                    // data header
                    #ifdef LIONPERF_SUPPORT
                    FwUpdateDataHeader* pDataHeader = (FwUpdateDataHeader*)(PSRAM_BACKUP_DATA_HEADER_ADDR);
                    uint32_t checkSum = 0;
                    InitBackupDataHeader(pDataHeader);
                    // data blk 0
                    BackupDFLInfo();
                    // data blk 1
                    BackupLoggingInfo();
                    checkSum = CalCheckSum((uint32_t*)pDataHeader, pDataHeader->totalDataLength, 0);
                    pDataHeader->checkSum = checkSum;
                    DMB();
                    #endif
                    pMsgState[cp2FpMsgIdx] = stateFpShutdownSendFP2CP;

                }
                else
                {
                    uint32_t currentTimestamp = SYSTICK_MASK - readl(REG_SYSTICK_CURRENT_VALUE) + (gDrainTimerIntrCnt * SYSTICK_MASK);
                    // check for timer expired
                    if(gDrainTimerValue <= currentTimestamp)
                    {
                        for (uint16_t i = 0; i  < UCD_FP_IO_Q_NUM; i++)
                        {
                            if (_pSlotFlagSts[i] & cStsFwUpdate)
                            {
                                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, Slot statue fw update :0x%x \n", i), "32");
                            } // else do nothing
                        }
                        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, IO Timeout  currentTimestamp %d\n", currentTimestamp), "32");
                        // Send NACK to CP core indicating IO Drain timeout
                        ack = false;
                        pMsgState[cp2FpMsgIdx] = stateFpShutdownSendFP2CP;
                        // Stop Timer
                        Cortexm7_t* rCortexM7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
                        writel(0x0, &rCortexM7->systemControl.systCsr);
                        writel(0, &rCortexM7->systemControl.systRvr);
                        writel(0x0, &rCortexM7->systemControl.systCvr);   //any write to current val clears it.
                        gDrainTimerValue = 0;
                        gDrainTimerIntrCnt = 0;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        case stateFpShutdownSendFP2CP:
        {
            if((readl(&_pIbCmnReg[UCD_CORE_1]->ucdIbCmnSnglInboundUcdStatus) & UCD_IB_CMN_SNGL_STATUS_BUSY) && ack)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, UCD Unpaused pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
                ack = false;
            }
            if(!ack)
            {
                // Stop Timer
                Cortexm7_t* rCortexM7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
                writel(0x0, &rCortexM7->systemControl.systCsr);
                writel(0, &rCortexM7->systemControl.systRvr);
                writel(0x0, &rCortexM7->systemControl.systCvr);   //any write to current val clears it.
                gDrainTimerValue = 0;
                gDrainTimerIntrCnt = 0;
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, NACK Msg sent to CP pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
                pMsgSQContext->sts = msgFwUpdateTimeout; // Send NACK in case of IO Drain Timeout
            }
            #ifdef PRE_RESET_FAULT_INJECTION_SUPPORT
            pMsgSQContext->sts = msgFwUpdateTimeout; // Send NACK to simulate Drain timeout due.
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, returning NACK pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
            #endif
            sts = msgSuccess;
            //writing to CPU status register to indicate that FW Update command has been processed and responding with an ACK.
            writel(FP_STS_FP_FW_UPDATE_RESP , pCpuStatus);
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            if (sts != msgNoEmptyEntry)
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, Msg sent to CP pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            }
            else
            {
                //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("MsgHandleFpShutdownRequest, Msg Not sent to CP pMsgState[%x]:0x%x \n", (cp2FpMsgIdx,pMsgState[cp2FpMsgIdx])), "16,16");
            }
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpShutdownReq, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

#ifdef MCR_TEST_HOOKS

void fpsCpu2::HandleFpTriggerCrashRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    CP2FPMsgSts sendSts = msgSuccess;
    CP2FPMsgDataInjectErrorReq_t* ptmpData = (CP2FPMsgDataInjectErrorReq_t*)(pMsgSQContext->data);

    if(ptmpData->errorType == InjErrPanic || ptmpData->errorType > InjErrHang)
    {
        pMsgSQContext->sts = msgInvalidField;
        pMsgState[cp2FpMsgIdx] = stateFpTriggerCrashSendFP2CP;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpInjectErrorReq & 0xFF) << 0x18UL) | (pMsgSQContext->sts & 0xFF))), "24,8");
    }
    else{
        switch (pMsgState[cp2FpMsgIdx])
        {
            case stateFpTriggerCrashStart:
            {
                if(ptmpData->coreid == FP2)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpTriggerCrashSendFP2CP;
                    // Crash now with given crashcode;
                    TriggerCrash(ptmpData->errorType);
                    break;
                }
                else
                {
                    pMsgState[cp2FpMsgIdx] = stateFpTriggerCrashMsgToOtherCpu;
                }
            }
            case stateFpTriggerCrashMsgToOtherCpu:
            {
                if(ptmpData->coreid == FP0)
                {
                    sendSts = SendFPMsg(cM7Core0, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
                }
                else
                {
                    sendSts = SendFPMsg(cM7Core1, cp2FpMsg, 0, msgSuccess, cp2FpMsgIdx, CPSrcId);
                }

                if (sendSts != msgNoEmptyEntry)
                {
                    pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap |= MSG_CPU(cM7Core0);
                    pMsgState[cp2FpMsgIdx] = stateFpTriggerCrashWaitOtherCpuDone;
                }
                break;
            }
            case stateFpTriggerCrashWaitOtherCpuDone:
            {
                if (pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].completeMap == pCp2FPMsgInfo->msgBitmap[cp2FpMsgIdx].submitMap)
                {
                    pMsgState[cp2FpMsgIdx] = stateFpTriggerCrashSendFP2CP;
                }
                else
                {
                    break;
                }
            }
            case stateFpTriggerCrashSendFP2CP:
            {
                sendSts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
                pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
                break;
            }
            case ALL_MSG_STATE_DONE:
            {
                // Do nothing
                break;
            }
            default:
            {
                FpsCpu2PrintInfoLogInvMsgState(msgOpInjectErrorReq, pMsgState[cp2FpMsgIdx]);
                break;
            }
        }
    }
}

void fpsCpu2::HandleFpIoLvl1AbrtRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    switch (pMsgState[cp2FpMsgIdx])
    {
        case stateFpIoLvl1AbrtStart:
        {
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("L1AbortHandling, sts:0x%X msgOp:0x%X\n", (((msgOpInjectErrorReq & 0xFF) << 0x18UL) | (sts & 0xFF))), "24,8");
            level1AbortFlag = true;
            pMsgState[cp2FpMsgIdx] = stateFpIoLvl1AbrtSendFP2CP;
            break;
        }
        case stateFpIoLvl1AbrtSendFP2CP:
        {
            sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
            pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
            break;
        }
        case ALL_MSG_STATE_DONE:
        {
            // Do nothing
            break;
        }
        default:
        {
            FpsCpu2PrintInfoLogInvMsgState(msgOpInjectErrorReq, pMsgState[cp2FpMsgIdx]);
            break;
        }
    }
}

void fpsCpu2::MsgHandleFpInjectErrorRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId)
{
    CP2FPMsgSts sts = (CP2FPMsgSts)pMsgSQContext->sts;
    CP2FPMsgDataInjectErrorReq_t* ptmpData = (CP2FPMsgDataInjectErrorReq_t*)(pMsgSQContext->data);

    if(ptmpData->errorType < InjErrFpIoLvl1Abrt || ptmpData->errorType>=InjErrMax)
    {
        pMsgSQContext->sts = msgInvalidField;
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("msg invalid field, sts:0x%X msgOp:0x%X\n", (((msgOpInjectErrorReq & 0xFF) << 0x18UL) | (pMsgSQContext->sts & 0xFF))), "24,8");
        sts = SendFP2CPMsg(pMsgSQContext, pMsgSQContext->data, RespMsg, pMsgSQContext->length, (CPMsgQId)CPSrcId); //send fp2cp msgfail
        pMsgState[cp2FpMsgIdx] = ALL_MSG_STATE_DONE;
    }
    else
    {
        switch(ptmpData->errorType)
        {
            case InjErrFpIoLvl1Abrt:
            {
                HandleFpIoLvl1AbrtRequest(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
                break;
            }
            case InjErrHardFault:
            case InjErrExplicitCrash:
            case InjErrPanic:
            case InjErrHang:
            {
                HandleFpTriggerCrashRequest(pCp2FPMsgInfo, pMsgSQContext, cp2FpMsgIdx, pMsgState, CPSrcId);
                break;
            }
            default:
            {
                FpsCpu2PrintInfoLogInvMsgState(msgOpInjectErrorReq, pMsgState[cp2FpMsgIdx]);
                break;
            }
        }
    }
}
#endif

