// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu0.h
//! @brief  FpsCpu0 Component Group
//!
//=============================================================================
#ifndef FP3CORE_FPSCPU0_H_
#define FP3CORE_FPSCPU0_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7FiberDataTypes.h"
#include "M7Fiber.h"
#include "M7MemMap.h"
#include "../../hal/Common/HalHostLionMSCmd.h"
#include "FpMessageCommon.h"
extern "C"
{
#include "vicommon.h"
}
#include "MessageHandler.h"
#include "BitmapOp.h"
#include "RegCortexm7.h"
#include "RegTcon.h"

extern Fps_t* rFps;
extern Ucd_t* rUcd;
extern Cdma_t* rCdma;
extern ResetType_t gResetType;

#define REG_FPS_BANK0_EVENT_STATUS_0      (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all)
#define REG_FPS_SLOT_ARRAY_PI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueProducerIndex.all)
#define REG_FPS_SLOT_ARRAY_CI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueConsumerIndex.all)
#define REG_FPS_SLOT_ARRAY_SIZE_BASE      (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueSize.all)
#define REG_FPS_SLOT_ARRAY_STATUS_BASE    (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueStatus.all)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_0 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus0SlotArrayQEmpty310)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_1 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus1SlotArrayQEmpty6332)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_2 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus2SlotArrayQEmpty9564)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_3 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus3SlotArrayQEmpty12796)
#define REG_FPS_SLOT_ARRAY_EMPTY_STATUS_4 (uint32_t)&(rFps->fpsBank1RegRegisters.fpsBank1SlotArrayQueueEmptyStatus4.all)
#define REG_FPS_INDIRECT_REG_WR_DISABLE   (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable)
#define REG_GLOBAL_SYNC_COUNTER_LO (uint32_t)&(rTcon->timerLoTconTimerLo)
#define REG_GLOBAL_SYNC_COUNTER_HI (uint32_t)&(rTcon->timerHiTconTimerHi)
#define REG_SYSTICK_CONTROL_STATUS (uint32_t)&(rCortexm7->systemControl.systCsr)
#define REG_SYSTICK_RELOAD_VALUE (uint32_t)&(rCortexm7->systemControl.systRvr)
#define REG_SYSTICK_CURRENT_VALUE (uint32_t)&(rCortexm7->systemControl.systCvr)

//-----------------------------------------------------------------------------
//  Class Definitions
//-----------------------------------------------------------------------------

enum class fpsCpu0FiberId_t : M7FiberId_t
{
    cFpsCpu0RecvFpMsgFiberId,
    cFpsCpu0FpCmdHandlerFiberId,
    cFpsCpu0CheckHeartbeatFiberId,
    cNumberOfFibers
};

/**
 *  @brief Module fpsCpu0 class
 *
 *
 */
class fpsCpu0
{
public:
    uint32_t inBoundDflPi[DFL_END];      ///< Inbound DFL queue Pi local parameter
    uint32_t inBoundCqCi[IBCQ_END];       ///< Inbound cmpl queue Ci local parameter
    uint32_t outBoundCqCi[OBCQ_END];      ///< Outbound Cmpl queue Ci local parameter
    uint8_t cmdArrayPi[UCD_FP_IO_Q_NUM];    ///< Local pi of command array
    uint8_t cmdArrayCi[UCD_FP_IO_Q_NUM];    ///< Local ci of command array

    CP2FPMsgContext_t* pCP0toFPMsgQ;      ///< CP2FP message queue base address
    CP2FPMsgContext_t* pCP1toFPMsgQ;      ///< CP2FP message queue base address
    FPInterMsgHeader* pCPU0toCPU1MsgQ;  ///< CPU0_to_CPU1 message queue base address
    FPInterMsgHeader* pCPU1toCPU0MsgQ;  ///< CPU1_to_CPU0 message queue base address
    FPInterMsgHeader* pCPU0toCPU2MsgQ;  ///< CPU0_to_CPU2 message queue base address
    FPInterMsgHeader* pCPU2toCPU0MsgQ;  ///< CPU2_to_CPU0 message queue base address

    volatile uint32_t* pCPU2toCPU0Pi;   ///< CPU2_to_CPU0 pi address
    volatile uint32_t* pCPU2toCPU0Ci;   ///< CPU2_to_CPU0 ci address
    volatile uint32_t* pCPU0toCPU2Pi;   ///< CPU0_to_CPU2 pi address
    volatile uint32_t* pCPU0toCPU2Ci;   ///< CPU0_to_CPU2 ci address
    volatile uint32_t* pCPU0toCPU1Pi;   ///< CPU2_to_CPU1 pi address
    volatile uint32_t* pCPU0toCPU1Ci;   ///< CPU2_to_CPU1 ci address
    volatile uint32_t* pCPU1toCPU0Pi;   ///< CPU1_to_CPU2 pi address
    volatile uint32_t* pCPU1toCPU0Ci;   ///< CPU1_to_CPU2 ci address

    uint32_t CPU2toCPU0Ci;
    uint32_t CPU0toCPU2Pi;

    uint32_t CPU0toCPU1Pi;
    uint32_t CPU1toCPU0Ci;

    uint32_t* pCpuStatus;
    #ifdef SUPPORT_UPDATE_TIMESTAMP
    /// < Time sync parameter
    uint32_t gTimerCounterBase;
    uint32_t gTimerCounterLast;
    uint32_t gTimerCounterDelta;
    uint32_t gTimerCounterCovert;
    uint32_t gTimerCounterCount;
    #endif
    #ifdef SUPPORT_TELEMETRY
    uint32_t* pOutstandingIoCnt;
    uint64_t* pAccumulateIoCnt;
    #endif

    uint32_t retryCEQueuePi;
    uint32_t retryCEQueueCi;
    volatile uint32_t* pRetryCEQueuePi;
    volatile uint32_t* pRetryCEQueueCi;
    uint16_t* pRetryCeIndexQueue;;

    uint32_t ceForRefillDFLQueuePi;
    uint32_t ceForRefillDFLQueueCi;
    volatile uint32_t* pCeForRefillDFLQueuePi;
    volatile uint32_t* pCeForRefillDFLQueueCi;
    uint16_t* pCEforRefillDFLQueue;

    Tcon_t* rTcon;
    Cortexm7_t* rCortexm7;

    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t hwDflPiAddr[DFL_END];
    #endif
    uint8_t* pCa2IbPhysicalId;
    uint8_t* pIbPhysicalId2Ca;

    #ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
    uint32_t localTimeStamp;
    #endif

    uint64_t* pFLRRequestBitMapLocal;

    uint32_t ibDflPiMask[DFL_END];      ///< Inbound DFL queue Pi mask

    /**
     *  @brief Initializes fpsCpu0 object.
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
     *  @param   void* pObj - pointer to fpsCpu0 object
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
     *  @brief   Refill DFL address
     *
     *  @param   uint32_t dflAddr - DFL address
     *
     *  @return  none
     */
    void RefillDFL(uint32_t dflAddr);

    #ifdef LIONPERF_SUPPORT
    /**
     *  @brief   Reload DFL list after FW update boot
     *
     *  @param   pFWupdateInfo - Fw update Info
     *
     *  @return  none
     */
    void LoadDFL(FWupdateBackupInfo *pFWupdateInfo);
    #endif

    /**
     *  @brief   Check Q with running cmd and set slot sts
     *
     *  @param   uint8_t slotSts - slot status
     *           uint8_t sqPId - queue phsical ID
     *
     *  @return with running cmd : true / no running cmd : false
     */
    uint8_t ChkQisRunningSetSlotSts(uint8_t slotSts, uint8_t sqPId);

    /**
     *  @brief   Scan IBCQ and refill DFL buffer
     *
     *  @param   uint8_t sqPId - queue phsical ID
     *
     *  @return none
     */
    void IbCqRefillDFL(uint8_t sqPId);

    /**
     *  @brief   Clear VF setting
     *
     *  @param   VFNodeInfo_t* pVFNodeInfo - VF node information
     *
     *  @return  none
     */
    void ClearVFSetting(VFNodeInfo_t* pVFNodeInfo);

    /**
     *  @brief   Reset slot flag status, inbound Q to outbound Q map, and handle delete Q
     *
     *  @param   uint8_t ibPhyQId - queue phsical ID
     *
     *  @return  none
     */
    void ResetMapDeleteQ(uint8_t ibPhyQId);

    /**
     *  @brief   Handle Delete Q and Delete VF
     *
     *  @param   uint8_t ibPhyQId - queue phsical ID
     *           uint8_t vfId - VF ID
     *           uint8_t slotSts - slot status
     *
     *  @return  none
     */
    void HandleDeleteVFQblk(uint8_t ibPhyQId, uint8_t vfId, uint8_t slotSts);

    /**
     *  @brief   Handle ErrorQ Set subOp Abort cmd
     *
     *  @param   CP2FPMsgAdminAbort_t* abortMsg - admin abort msg
     *
     *  @return  none
     */
    void HandleAdminAbort(CP2FPMsgAdminAbort_t* abortMsg);

    /**
     *  @brief   Handle status change
     *
     *  @return  CP2FPMsgSts
     */
    CP2FPMsgSts FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone);

    /**
     *  @brief   Update queue running status
     *
     *  @param   CP2FPMsgContext_t* pMsg - CP to FP msg context
     *
     *  @return  None
     */
    void UpdateQRunningStatus(CP2FPMsgContext_t* pMsg);

    /**
     *  @brief   Handle VfSlotSq2CqMap Update
     *
     *  @return  None
     */
    Error_t FpsCpuVfSlotSq2CqMapUpdate(CP2FPMsgContext_t* pMsg);
    #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
    /**
     *  @brief   Handle update timestamp address
     *
     *  @param   CP2FPMsgDataUpdateTimestampAddr_t* pCtx - Update timestamp address data
     *
     *  @return  ErrorCode
     */
    Error_t FpsCpuUpdateTimestampAddr(CP2FPMsgDataUpdateTimestampAddr_t* pCtx);
    #endif
    /**
     *  @brief Handle Vf Update
     *
     *  @param   CP2FPMsgDataVfUpdate_t* pData - VfUpdate Message data
     *
     *  @return  ErrorCode
     */
    Error_t FpsCpuVfUpdate(CP2FPMsgDataVfUpdate_t* pData);

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
     *  @brief   Check all FP internal message
     *
     *  @return  done : true / undone : false
     */
    uint8_t ChkMsgHandleDone();

    /**
     *  @brief   Check Q block bitmap is clear or not
     *
     *  @param   VFNodeInfo_t* pVFNodeInfo - VF node information
     *
     *  @return  done : true / undone : false
     */
    bool ChkQBlkBitmap(VFNodeInfo_t* pVFNodeInfo);

    /**
     *  @brief   Check Q block bitmap is clear or not
     *
     *  @return  done : true / undone : false
     */
    void HandleTeardown(uint8_t VFId);

    /**
     *  @brief   Manage Message from CP request
     *
     *  @param   uint8_t msgIdx - message index in CP2FPMsgQ
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts ManageCPMsg(uint8_t msgIdx, uint8_t msgSrc);

    /**
     *  @brief   Send Message to other FP core by FP internal Q
     *
     *  @param   M7CoreId_t cpu - cpu destination of message delivery.
     *
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
     *  @brief   Handle re-schedule after CDMA reset due to fatal error
     *
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts HandleReSchedule();

    void FpsCpu0TearDownRefillDFL(uint16_t ceIndex, uint8_t caIndex, uint8_t ibPhyQId);
    bool FpsCpu0RetryErrorCmd(uint16_t orgCeIndex, uint16_t newCeIndex, uint8_t caIndex, uint8_t ibPhyQId);
    void FpsCpu0CheckSlotStatusAndHandling(uint8_t slotSts, uint8_t ibPhyQId);
    bool FpsCpu0FillCmdEntryAndSendToCpu1(uint8_t ibcqIndex, uint16_t qMask);
    void FpsCpu0ProcessObCqFailure(uint8_t obcqIndex);
    void FpsCpu0CheckIBCqeValidAndDFLAddress(uint32_t dflAddr, uint8_t ibcqIndex);
    void FpsCpu0InboundCompletionQueueHandler(uint8_t ibcqIndex);
    void FpsCpu0OutboundCompletionQueueHandler(uint8_t obcqIndex);
    void FpsCpu0ProcessRetryCeHandler(void);
    void FpsCpu0TearDownRefillDFLHandler(void);

    /**
     *  @brief   Entry point function for running the Fiber fpsCpu0FpCmdHandlerFiber
     *
     *  @param   void* pObj - pointer to fpsCpu0 object
     *
     *  @return  None
     */
    static void FpsCpu0FpCmdHandlerFiber(void* pObj);

    /**
     *  @brief   Entry point function for running the Fiber ReceiveFPMsg
     *
     *  @param   void* pObj - pointer to fpsCpu0 object
     *
     *  @return  None
     */
    static void FpsCpu0ReceiveFPMsgFiber(void* pObj);

    /**
     *  @brief   Entry point function for checking irq status in PCSim and resume fiber if need.
     *
     *  @param   void* pObj - pointer to fpsCpu0 object
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

    constexpr static uint8_t   cFiberWeightFpsCpu0FpCmdHandlerFiber = 1;
    constexpr static uint32_t   cFiberWeightRecvFPMsg = 1;
    constexpr static uint32_t   cFiberWeightCheckHeartbeatFiber = 1;

protected:
    // empty

private:

    /**
     *  @brief   To init DFL list.
     *
     *  @param   None
     *
     *  @return  None
     */
    void _InitDFLList();

    /**
     *  @brief   To restore schedule structure (command array hw ci, vf command exist bit map) after CDMA reset due to CDMA fatal error
     *
     *  @param   vfIndex Index of VF
     *
     *  @return  void
     */
    void _RestoreSchedulingStructure(uint8_t vfIndex);

    FpsUcdIbq_t _ucdIbq;
    FpsUcdObq_t _ucdObq;

    VFNodeInfo_t* _pVfInfoBase;
    uint64_t* _pVFEnBitmap;
    uint32_t* _pVF65EnBitmap;
    uint8_t* _pSlotFlagSts;
    uint8_t* _pIbQ2ObQ;
    uint64_t* _pVfCmdExistBitMap;
    uint32_t* _pVf65CmdExistBitMap;
    uint64_t* _pQBEnBitmap;
    uint8_t* _pQB65EnBitmap;
    uint8_t* _rgid2OwnerVfid;   ///< bit map of each key valid indication for each RG ID, Bit(n) represents Key slot(n).
    uint8_t* _key2OwnerVfid;   ///< bit map of each key valid indication for each RG ID, Bit(n) represents Key slot(n).
    QueueBlockInfo_t* _pQueueBlockInfoBase;
    CmdEntry_t* _pCmdEntryArray;
    CmdEntryTiny_t* _pCmdEntryArrayTiny;
    uint8_t* _CPU1SubmitAbortInfo;
    volatile int32_t* _pTotalCredit;
    volatile int32_t* _pVfCredit;
    volatile int32_t* _pVfRemainCredit;
    M7Fiber _fpsCpu0FpCmdHandlerFiber;
    M7Fiber _fpsCpu0RecvFpMsgFiber;
    M7Fiber _fpsCpu0CheckHeartbeatFiber;
    UcdCore0IbCmnRegisters_t* _pIbCmnReg[UCD_CORE_NUM];
    UcdCore0ObCmnRegisters_t* _pObCmnReg[UCD_CORE_NUM];

    uint8_t* _adminAbortCount;
};

#endif  // FP3CORE_fpsCpu0_H_
