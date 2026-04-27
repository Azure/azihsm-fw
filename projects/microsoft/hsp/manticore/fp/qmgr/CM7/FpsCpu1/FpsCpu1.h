// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1.h
//! @brief  FpsCpu1 Component Group
//!
//=============================================================================
#ifndef FP3CORE_FPS_CPU1_H_
#define FP3CORE_FPS_CPU1_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7FiberDataTypes.h"
#include "M7Fiber.h"
#include "M7MemMap.h"
#include "../../hal/Common/HalHostLionMSCmd.h"
#include "FpMessageCommon.h"
#include "RegCdma.h"
extern "C"
{
#include "vicommon.h"
}
#include "MessageHandler.h"
#include "CDMA.h"
extern "C"
{
#include "APICdmaErrorHandle.h"
}
#include "BitmapOp.h"
#if defined (SUPPORT_MSGERROR_INJECTION) || defined (SUPPORT_ERROR_INJECTION)
#include "FpsCpu1ErrorInjection.h"
#endif
#include "RegCortexm7.h"
#include "RegTcon.h"

extern Fps_t* rFps;
extern Ucd_t* rUcd;
extern Cdma_t* rCdma;
extern ResetType_t gResetType;

#define REG_FPS_BANK0_EVENT_STATUS_0      (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all)
#define REG_FPS_BANK0_EVENT_STATUS_1      (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus1.all)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_0 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus0SlotArrayQEmpty310)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_1 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus1SlotArrayQEmpty6332)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_2 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus2SlotArrayQEmpty9564)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_3 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus3SlotArrayQEmpty12796)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_4 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus4.all)
#define REG_FPS_SLOT_ARRAY_PI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueProducerIndex.all)
#define REG_FPS_SLOT_ARRAY_CI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueConsumerIndex.all)
#define REG_FPS_SLOT_ARRAY_SIZE_BASE      (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueSize.all)
#define REG_FPS_SLOT_ARRAY_STATUS_BASE    (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueStatus.all)
#define REG_FPS_INDIRECT_REG_WR_DISABLE   (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable)
#ifdef CDMA_CMD_COUNT
#define REG_FPS_CDMA_IO_COMMAND_QUEUE_STATUS (uint32_t)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueStatus.all)
#define REG_FPS_CDMA_IO_COMMAND_QUEUE_STATUS_WORD  (uint32_t)(((uint32_t)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueStatus.all)) + 2)
#endif // End of CDMA_CMD_COUNT

#define REG_GLOBAL_SYNC_COUNTER_LO (uint32_t)&(rTcon->timerLoTconTimerLo)
#define REG_GLOBAL_SYNC_COUNTER_HI (uint32_t)&(rTcon->timerHiTconTimerHi)
#define REG_SYSTICK_CONTROL_STATUS (uint32_t)&(rCortexm7->systemControl.systCsr)
#define REG_SYSTICK_RELOAD_VALUE (uint32_t)&(rCortexm7->systemControl.systRvr)
#define REG_SYSTICK_CURRENT_VALUE (uint32_t)&(rCortexm7->systemControl.systCvr)

//-----------------------------------------------------------------------------
//  Class Definitions
//-----------------------------------------------------------------------------

enum class fpsCpu1FiberId_t : M7FiberId_t
{
    cFpsCpu1RecvFpMsgFiberId,
    cFpsCpu1QueueManagerFiberId,
    cFpsCpu1CPCDMAIOFiberId,
    cFpsCpu1CheckHeartbeatFiberId,
    cNumberOfFibers
};

/**
 *  @brief Module fpsCpu1 class
 *
 *
 */
class fpsCpu1 {
public:
    uint32_t cdmaSqPi;
    uint32_t cdmaSqCi;
    uint8_t cmdArrayCi[UCD_FP_IO_Q_NUM];
    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t cdmaSqPiHwAddr;
    #endif

    CP2FPMsgContext_t* pCP0toFPMsgQ;      ///< CP2FP message queue base address
    CP2FPMsgContext_t* pCP1toFPMsgQ;      ///< CP2FP message queue base address

    FPInterMsgHeader* pCPU0toCPU1MsgQ;  ///< CPU0_to_CPU1 message queue base address
    FPInterMsgHeader* pCPU1toCPU0MsgQ;  ///< CPU1_to_CPU0 message queue base address
    FPInterMsgHeader* pCPU1toCPU2MsgQ;  ///< CPU1_to_CPU2 message queue base address
    FPInterMsgHeader* pCPU2toCPU1MsgQ;  ///< CPU2_to_CPU1 message queue base address

    volatile uint32_t* pCPU2toCPU1Pi;   ///< CPU2_to_CPU1 pi address
    volatile uint32_t* pCPU2toCPU1Ci;   ///< CPU2_to_CPU1 ci address
    volatile uint32_t* pCPU1toCPU2Pi;   ///< CPU1_to_CPU2 pi address
    volatile uint32_t* pCPU1toCPU2Ci;   ///< CPU1_to_CPU2 ci address
    volatile uint32_t* pCPU0toCPU1Pi;   ///< CPU0_to_CPU1 pi address
    volatile uint32_t* pCPU0toCPU1Ci;   ///< CPU0_to_CPU1 ci address
    volatile uint32_t* pCPU1toCPU0Pi;   ///< CPU1_to_CPU0 pi address
    volatile uint32_t* pCPU1toCPU0Ci;   ///< CPU1_to_CPU0 ci address
    #ifdef CDMA_CMD_COUNT
    volatile uint32_t* pCdmaCmdSlotQueuePi;
    volatile uint32_t* pCdmaCmdSlotQueueCi;
    #endif // End of CDMA_CMD_COUNT

    uint32_t cdmaSlotAbortQueuePi;
    uint32_t cdmaSlotAbortQueueCi;
    volatile uint32_t* pCdmaSlotAbortQueuePi;
    volatile uint32_t* pCdmaSlotAbortQueueCi;
    uint16_t* pCdmaSlotAbortQueue;

    uint32_t CPU2toCPU1Ci;
    uint32_t CPU1toCPU2Pi;

    uint32_t CPU0toCPU1Ci;
    uint32_t CPU1toCPU0Pi;

    uint32_t* pCpuStatus;

    CP2FPMsgDataOpCpCdmaIo_t* pCPCDMAdata;
    uint8_t CPCDMAStatus;
    uint8_t CPCDMAError;
    uint8_t hasCPCDMACmd;   ///< When set, indicates a CP CDMA IO is received.
    uint8_t Reserved[2];
    #ifdef SUPPORT_MSGERROR_INJECTION
    uint8_t errInjectFlag;
    uint64_t* pErrInjectBitmap;
    CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection;
    #endif
    #ifdef CDMA_CMD_COUNT
    uint32_t cdmaCmdSlotQueuePi;
    uint32_t cdmaCmdSlotQueueCi;
    #endif // End of CDMA_CMD_COUNT

    Tcon_t* rTcon;
    Cortexm7_t* rCortexm7;

    #ifdef SUPPORT_UPDATE_TIMESTAMP
    /// < Time sync parameter
    uint32_t gTimerCounterBase;
    uint32_t gTimerCounterLast;
    uint32_t gTimerCounterDelta;
    uint32_t gTimerCounterCovert;
    uint32_t gTimerCounterCount;
    #endif
    uint8_t* pCa2IbPhysicalId;
    uint8_t* pIbPhysicalId2Ca;

    #ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
    uint32_t localTimeStamp;
    #endif

    uint64_t* pFLRRequestBitMapLocal;

    uint64_t* pFLRQueueBlockMap;
    uint8_t* pFLRQueueBlock65Map;


    /**
     *  @brief Initializes fpsCpu1 object.
     *
     *  This function must be call before any other module interface function
     *  is called.
     *
     */
    void Initialize(M7CompGroupId_t compId);

    /**
     *  @brief   Member function that register all member's fiber.
     *
     *  @param   CompGroupId_t compId - Component Group ID
     *
     *  @return  None
     */
    void RegisterComponentGroup(M7CompGroupId_t compId);

    /**
     *  @brief   Register all member's fibers
     *
     *  @param   void* pObj - pointer to fpsCpu1 object
     *
     *  @return  None
     */
    static void RegisterFibers(void* pObj);

    /**
     *  @brief   Initialize all member's fibers.
     *
     *  @return  None
     */
    void InitializeFiber();

    /**
     *  @brief   Initialize resource when normal boot.
     *
     *  @return  None
     */
    void FpsCpuNormalBootInitialize();

    // message Q related functions
    /**
     *  @brief   Check all FP internal Q done
     *
     *  @return  done : true / undone : false
     */
    uint8_t ChkRecvFPMsgFiberDone();

    /**
     *  @brief   Manage Message from CP request
     *
     *  @param   uint8_t msgIdx - message index in CP2FPMsgQ
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts ManageCPMsg(uint8_t msgIdx, uint8_t msgSrc);

    /**
     *  @brief   Manage Message for CDMA abort
     *
     *  @param   uint16_t cpuCid - cpu command id (ceIndex) to skip and abort
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts ManageCDMAAbort(uint16_t cpuCid);

    #ifdef QOS_LATENCY_ERROR_HANDLING
    /**
     *  @brief   Manage Message for create/delete queue
     *
     *  @param   CP2FPMsgContext_t* pMsg - msg for manage queue create/delete
     *
     *  @return  cEcError: create/delete queue update fail / cEcNoError: if create/delete queue success
     */
    Error_t ManageQueueCfg(CP2FPMsgContext_t* pMsg);

    /**
     *  @brief   Check available FP internal Message Q resource
     *
     *  @param   M7CoreId_t cpu - cpu destination of message delivery.
     *
     *  @return  1: available resource / 0: no available resource
     */
    uint8_t ChkAvailableFPInterMsgRes(M7CoreId_t cpu);

    /**
     *  @brief   Handle Qos latency error
     *
     *  @param   vfGroupIndex - 0: VF0-31, 1: VF32-63, 2: VF64
     *
     *  @return  None
     */
    void QosLatencyPenaltyHandling(uint8_t vfGroupIndex);

    #else
    /**
     *  @brief   Manage Message for delete queue
     *
     *  @param   CP2FPMsgDataVfSlotSq2CqMapUpdate_t* pCtx - data for manage queue delete
     *
     *  @return  cEcError: delete queue update fail / cEcNoError: if delete queue success
     */
    Error_t ManageQueueDelete(CP2FPMsgDataVfSlotSq2CqMapUpdate_t* pCtx);
    #endif

    /**
     *  @brief   Manage Message for delete queue
     *
     *  @param   CP2FPMsgDataVfUpdate_t* pCtx - data for manage vf update
     *
     *  @return  cEcError: vf update fail / cEcNoError: vf update success
     */
    Error_t ManageVFUpdate(CP2FPMsgDataVfUpdate_t* pData);

    /**
     *  @brief   Send Message to other FP core by FP internal Q
     *
     *  @param   M7CoreId_t cpu - cpu destination of message delivery.
     *           uint8_t fpMsgOp  - FP internal message opcode
     *           uint8_t resp  - FP internal message response
     *           uint8_t fpSts - FP internal message status (CP2FPMsgSts)
     *           uint8_t cmdSpecific0 /cmdSpecific1 - FP internal message specific info according to different message opcode
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts SendFPMsg(M7CoreId_t cpu, uint8_t fpMsgOp, uint8_t resp, CP2FPMsgSts fpSts, uint8_t cmdSpecific0, uint8_t cmdSpecific1);

    /**
     *  @brief   Receive FP message from FP internal Q
     *
     *  @param   FPInterMsgHeader* pFpMsgHeader - pointer to  FP internal Q
     *           M7CoreId_t msgCpu  - cpu source of message delivery.
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts RecvFPMsg(FPInterMsgHeader* pFPMsgHeader, M7CoreId_t msgCpu);

    /**
     *  @brief   Clean key information
     *
     *  @param   uint16 keyIndex - The key index value
     *
     *  @return  None
     */
    void CleanKeyInfo(uint16_t keyIndex);

    /**
     *  @brief   Handle status change
     *
     *  @return  None
     */
    CP2FPMsgSts FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone);

    /**
     *  @brief   Handle key update
     *
     *  @param   CP2FPMsgDataKeyUpdate_t* pCtx - Key update message data
     *
     *  @return  cEcError: if key update fail / cEcNoError: if key update success
     */
    Error_t KeyUpdate(CP2FPMsgDataKeyUpdate_t* pKeyUpdate);

    /**
     *  @brief   Handle FP mode change
     *
     *  @param   CP2FPMsgDataVfModeChange_t* pCtx - FP mode change message data
     *
     *  @return  cEcError: if mode change receive invalid input / cEcNoError: if FP mode change success
     */
    Error_t FpModeChange(CP2FPMsgDataVfModeChange_t* pCtx);

    #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
    /**
     *  @brief   Handle Update timestamp address
     *
     *  @param   CP2FPMsgDataUpdateTimestampAddr_t* pCtx - Update timestamp address data
     *
     *  @return  cEcError: if update timestamp addr receive invalid input / cEcNoError: if update timestamp addr success
     */
    Error_t FpsCpuUpdateTimestampAddr(CP2FPMsgDataUpdateTimestampAddr_t* pCtx);
    #endif

    /**
     *  @brief   Handle CDMA IO
     *
     *  @param   CP2FPMsgDataCDMAIO_t* pCtx - CP CDMAIO data
     *
     *  @return  None
     */
    void CpCdmaIO(CP2FPMsgDataOpCpCdmaIo_t* pCtx);

    /**
     *  @brief   Visit Queue Block
     *
     *  @param   qbSelect - 0: QB0-QB31,  1: QB32-QB63, 2: QB64
     *           dflBaseAddr - DFL base address
     *
     *  @return  None
     */
    void QBHandler(uint8_t qbSelect,      uint32_t dflBaseAddr);

    void FpsCpu1FreeCDMAErrorSlot(void);

    /**
     *  @brief   Submit idle commands to CDMA engine
     *
     *  @param   credit - credit number
     *
     *  @return  None
     */
    void SendIdleCmd(int32_t credit);

    /**
     *  @brief   Setup idle commands to CDMA engine
     *
     *  @param   None
     *
     *  @return  None
     */
    void SetupIdleCmd();

    /**
     *  @brief   Entry point function for running FpsCpu1QueueManagerFiber
     *
     *  @param   void* pObj - pointer to FPS CPU1Fiber object
     *
     *  @return  None
     */
    static void FpsCpu1QueueManagerFiber(void* pObj);

    /**
     *  @brief   Entry point function for receiving FastPath internal message fiber.
     *
     *  @param   void* pObj - pointer to FPS CPU1Fiber object
     *
     *  @return  None
     */
    static void FpsCpu1ReceiveFPMsgFiber(void* pObj);

    /**
     *  @brief   Entry point function for handle CP CDMA IO fiber.
     *
     *  @param   void* pObj - pointer to FPS CPU1Fiber object
     *
     *  @return  None
     */
    static void FpsCpu1CPCDMAIOFiber(void* pObj);

    /**
     *  @brief   Entry point function for checking irq status in PCSim and resume fiber if need.
     *
     *  @param   void* pObj - pointer to Cpu1 object
     *
     *  @return  None
     */
    static void CheckFPMsgFiberNeedResume(void* pObj);

    /**
     *  @brief   Get Fw update Info.
     *
     *  @param   FWupdateBackupInfo* pFWupdateInfo - Fw update Info
     *           RecoverDataBlk dataType - data blk type
     *  @return  None
     */
    void GetFwUpdateInfo(FWupdateBackupInfo* pFWupdateInfo, RecoverDataBlk dataType);

    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    /**
     *  @brief   Function to validate the key in Keyvault Array
     *
     *  @param   AesKeyVault_t* keyVaultArr - Pointer to keyVaultArr
     *  @param   keyIndex1
     *  @param   keyIndex2
     *
     *  @return  true : validation success / false : validation failure
     */
    bool ValidateKeyVaultArrKey(AesKeyVault_t* keyVaultArr, uint16_t keyIndex1, uint16_t keyIndex2);
    #endif

    constexpr static uint32_t   cFiberWeightFpsCpu1QueueManagerFiber = 1;
    constexpr static uint32_t   cFiberWeightfpsCpu1RecvFPMsgFiber = 1;
    constexpr static uint32_t   cFiberWeightfpsCpu1CPCDMAIOFiber = 1;
    constexpr static uint32_t   cFiberWeightCheckHeartbeatFiber = 1;

protected:
    // empty

private:
    uint64_t* _pVfCmdExistBitMap;
    uint32_t* _pVf65CmdExistBitMap;
    CmdEntry_t* _pCmdArrayBase;
    CmdEntryTiny_t* _pCmdArrayTinyBase;

    VFNodeInfo_t* _pVfInfoBase;
    QueueBlockInfo_t* _pQBlockInfoBase;
    CdmaSq_t _cdmaSq;
    uint64_t _teardownQueueBlockBitMap;
    uint32_t _teardownQueueBlock65BitMap;
    uint64_t* _pVFEnBitmap;
    uint32_t* _pVF65EnBitmap;
    bit64* _pQBEnBitmap;
    uint8_t* _pQB65EnBitmap;
    uint8_t* _rgid2keyValid;    ///< bit map of each key valid indication for each RG ID, Bit(n) represents Key slot(n).
    uint8_t* _rgid2OwnerVfid;   ///< map of rgid to the vfid which own the resource group
    uint8_t* _key2OwnerVfid;
    uint16_t* _pKey2SessionID;  ///< map of key to session ID for ephemeral Keys
    uint8_t* _pKey2AppID;       ///< map of key to application ID for both ephemeral and persistent keys
    uint8_t* _pKeyIsEphemeral;  ///< ephemeral flag for each key in each resource group
    uint32_t* _cdmaFatalErrorFlag;
    uint8_t* _pQAbortBitmap;
    uint8_t* _CPU1SubmitAbortInfo;
    uint8_t* _pCDMAIOAbortBit;
    M7Fiber _fpsCpu1QueueManagerFiber;
    M7Fiber _fpsCpu1RecvFpMsgFiber;
    M7Fiber _fpsCpu1CPCDMAIOFiber;
    M7Fiber _fpsCpu1CheckHeartbeatFiber;
    #ifdef QOS_LATENCY_ERROR_HANDLING
    CP2FPMsgDataQoSPenalty_t* _pQosPenalty;
    uint32_t* _pQosVFBitmap[VF_MAX];
    #endif
    #ifdef WEIGHT_ROUND_ROBIN
    uint32_t* _pWeightRoundRobin;
    #endif
    volatile int32_t* _pTotalCredit;
    volatile int32_t* _pVfCredit;
    volatile int32_t* _pVfRemainCredit;

    Fastpath_OP_Mode_t* _fpMode; ///< 0: greedy, 1:strict

    /**
     *  @brief After send abort command, checking whether the command array is now processing such command
     *  If so, we increase ci and rest remainLen of queue block info
     *
     *  @param    cpuCid - cpu command id to check
     *
     *  @return    None
     */
    void _UpdateCmdArrayCiAfterAbort(uint16_t cpuCid);

    /**
     *  @brief     Update Ci Value after abort, and clear remain length of queue block info
     *
     *  @param     cpuCid - cpu command id to update ci
     *  @param     pQBlockInfo Pointer to queue block info to update
     *
     *  @return    None
     */
    void _UpdateCmdArrayCi(uint16_t cpuCid, QueueBlockInfo_t* pQBlockInfo);

    /**
     *  @brief     To abort command when queue deletion or VF teardown
     *
     *  @param     sqid qid to delete/teardown
     *  @param     ceSts status to update to command entry
     *
     *  @return    None
     */
    void _SkipAndAbortWithSQid(uint16_t sqid,  CmdEntryStatus_t ceSts);

    /**
     *  @brief     To iterate over queue block of VF when teardown
     *
     *  @param     None
     *
     *  @return    None
     */
    void _ProcessTeardownBitMap();


    /**
     *  @brief     To update information to handle function teardown
     *
     *  @param     vfId function ID to teardown
     *
     *  @return    None
     */
    void _HandleTeardown(uint8_t vfId);

    /**
     *  @brief     Process admin abort command
     *
     *  @param     data structure of abort message
     *
     *  @return    None
     */
    void HandleAdminAbort(CP2FPMsgAdminAbort_t* abortMsg);

};

#endif  // FP3CORE_fpsCpu1_fpsCpu1_H_
