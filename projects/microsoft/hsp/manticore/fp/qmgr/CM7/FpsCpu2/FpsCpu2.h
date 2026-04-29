// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2.h
//! @brief  FpsCpu2 Component Group
//!
//=============================================================================
#ifndef FP3CORE_FPSCPU2_H_
#define FP3CORE_FPSCPU2_H_
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "M7FiberDataTypes.h"
#include "M7Fiber.h"
#include "M7MemMap.h"
#include "MessageHandler.h"
#include "FpMessageCommon.h"
#include "HalHostLionMSCmd.h"
#include "common.h"

extern "C"
{

}
#ifdef LOGGING_NEW_SCHEME
#include "RegGdma.h"
#endif
extern "C"
{
#include "vicommon.h"
#include "APICdma.h"
}
#include "BitmapOp.h"
#include "FpsCpu2ErrorHandle.h"
#include "RegCortexm7.h"
#include "RegTcon.h"
#include "M7Partition.h"
enum OBSts
{
    OBNormal = 0,
    RefillDFL,
    ErrQFill,
    CdmaContiune,
};
extern Fps_t* rFps;
extern ResetType_t gResetType;

#define REG_GLOBAL_SYNC_COUNTER_LO (uint32_t)&(rTcon->timerLoTconTimerLo)
#define REG_GLOBAL_SYNC_COUNTER_HI (uint32_t)&(rTcon->timerHiTconTimerHi)
#define REG_SYSTICK_CONTROL_STATUS (uint32_t)&(rCortexm7->systemControl.systCsr)
#define REG_SYSTICK_RELOAD_VALUE (uint32_t)&(rCortexm7->systemControl.systRvr)
#define REG_SYSTICK_CURRENT_VALUE (uint32_t)&(rCortexm7->systemControl.systCvr)
#define REG_FPS_SLOT_ARRAY_CI_BASE        (uint32_t)&(rFps->fpsSlotArrayRegRegisters[0].fpsSlotArrayQueueConsumerIndex.all)
#define REG_FPS_INDIRECT_REG_WR_DISABLE   (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable)

//-----------------------------------------------------------------------------
//  Class Definitions
//-----------------------------------------------------------------------------

enum class fpsCpu2FiberId_t : M7FiberId_t
{
    //cFpsCpu2ProcessCdmaCqOslFiberId,
    cFpsCpu2CP2FPServiceFiberId,
    cFpsCpu2ProcessCdmaCqOslFiberId,
    cFpsCpu2RecvFpMsgFiberId,
    cFpsCpu2HandleCDMAFatalErrorFiberId,
    cFpsCpu2HandleCDMAKeyCorrtableErrFiberId,
    cFpsCpu2HandleResetFiberId,
    cFpsCpu2CheckHeartbeatFiberId,
    cNumberOfFibers
};

typedef enum CDMAFatalErrorHandlingState_t
{
    cFatalErrorHandlingWait = 0,
    cFatalErrorHandlingStart,
    cFatalErrorHandlingNotifyCpu1,
    cFatalErrorHandlingWaitCpu1Resp,
    cFatalErrorHandlingWaitHandleCompletion,
    cFatalErrorHandlingClassifyErrorSlots,
    cFatalErrorHandlingWaitOutboundProcessDone,
    cFatalErrorHandlingWaitCPReWriteKey,
    cFatalErrorHandlingNotifyCpu0,
    cFatalErrorHandlingWaitResetDone,
}CDMAFatalErrorHandlingState_t;

typedef enum CDMACorrectableKeyErrorHandlingState_t
{
    cCorrtableErrorhandlingWait = 0,
    cCorrtableErrorhandlingStart,
    cCorrtableErrorhandlingNotifyCpu1,
    cCorrtableErrorhandlingWaitCpu1Resp,
    cCorrtableErrorhandlingWaitCDMAIdle,
    cCorrtableErrorhandlingNotifyCP,
    cCorrtableErrorhandlingWaitCPResp,
    cCorrtableErrorhandlingNotifyCpu1ResumeIO,
    cCorrtableErrorhandlingWaitCpu1Resume,
    cCorrtableErrorhandlingDone,
}CDMACorrectableKeyErrorHandlingState_t;

typedef enum ResetHandlingState_t
{
    cResetHandlingWait = 0,
    cResetHandlingProcessTeardownMap,
    cResetHandlingNotifyCPU0,
    cResetHandlingWaitCPU0Resp,
    cResetHandlingNotifyCPU1,
    cResetHandlingWaitCPU1Resp,
    cResetHandlingWaitTeardownDone,
    cResetHandlingNotifyCP
}ResetHandlingState_t;

typedef struct CP2FPMessageBitmap
{
    union
    {
        uint8_t bitMap;
        struct
        {
            uint8_t submitMap : 2;
            uint8_t completeMap : 2;
            uint8_t reserved : 4;
        };

    };
}CP2FPMessageState;
typedef struct CP2FPMessageInfo
{
    CP2FPMsgContext_t* pMsgQ;
    volatile uint32_t* pMsgPi;
    volatile uint32_t* pMsgCi;
    uint32_t localMsgPi;
    uint32_t localMsgCi;
    uint8_t msgState[PSRAM_CP2FP_MSG_DEPTH];
    CP2FPMessageBitmap msgBitmap[PSRAM_CP2FP_MSG_DEPTH];
}CP2FPMessageInfo;

typedef struct FP2CPMessageInfo
{
    CP2FPMsgContext_t* pMsgQ;
    volatile uint32_t* pMsgPi;
    volatile uint32_t* pMsgCi;
    uint32_t localMsgPi;
}FP2CPMessageInfo;

typedef enum MessageType_t
{
    ReqMsg = 0,
    RespMsg,
}MessageType_t;

/**
 *  @brief Module FCP class
 *
 *
 */
class fpsCpu2
{
public:
    uint32_t cdmaCqCi;                   ///< CDMA Cmpl queue Ci value
    uint32_t outBoundOSLPi[OSL_END];     ///< Outbound OSL submission  queue Pi value
    uint32_t localObCqPi[OBCQ_END];

    CP2FPMessageInfo CP0toFPReqMsg;
    CP2FPMessageInfo CP1toFPReqMsg;

    FP2CPMessageInfo FPtoCP0ResMsg;
    FP2CPMessageInfo FPtoCP1ResMsg;

    FP2CPMessageInfo FPtoCP0ReqMsg;
    FP2CPMessageInfo FPtoCP1ReqMsg;

    CP2FPMessageInfo CP0toFPResMsg;
    CP2FPMessageInfo CP1toFPResMsg;

    FPInterMsgHeader* pCPU2toCPU0MsgQ;  ///< CPU2_to_CPU0 message queue base address
    FPInterMsgHeader* pCPU0toCPU2MsgQ;  ///< CPU0_to_CPU2 message queue base address
    FPInterMsgHeader* pCPU1toCPU2MsgQ;  ///< CPU1_to_CPU2 message queue base address
    FPInterMsgHeader* pCPU2toCPU1MsgQ;  ///< CPU2_to_CPU1 message queue base address

    volatile uint32_t* pCPU2toCPU0Pi;   ///< CPU2_to_CPU0 pi address
    volatile uint32_t* pCPU2toCPU0Ci;   ///< CPU2_to_CPU0 ci address
    volatile uint32_t* pCPU0toCPU2Pi;   ///< CPU0_to_CPU2 pi address
    volatile uint32_t* pCPU0toCPU2Ci;   ///< CPU0_to_CPU2 ci address

    volatile uint32_t* pCPU2toCPU1Pi;   ///< CPU2_to_CPU1 pi address
    volatile uint32_t* pCPU2toCPU1Ci;   ///< CPU2_to_CPU1 ci address
    volatile uint32_t* pCPU1toCPU2Pi;   ///< CPU1_to_CPU2 pi address
    volatile uint32_t* pCPU1toCPU2Ci;   ///< CPU1_to_CPU2 ci address


    uint32_t CPU2toCPU0Pi;
    uint32_t CPU0toCPU2Ci;

    uint32_t CPU2toCPU1Pi;
    uint32_t CPU1toCPU2Ci;

    #ifdef CDMA_CMD_COUNT
    uint32_t cdmaCmdSlotQueueCi;
    volatile uint32_t* pCdmaCmdSlotQueueCi;
    #endif // End of CDMA_CMD_COUNT

    uint32_t cdmaSlotAbortQueuePi;
    uint32_t cdmaSlotAbortQueueCi;
    volatile uint32_t* pCdmaSlotAbortQueuePi;
    volatile uint32_t* pCdmaSlotAbortQueueCi;
    uint16_t* pCdmaSlotAbortQueue;
    uint32_t retryCEQueuePi;
    uint32_t retryCEQueueCi;
    volatile uint32_t* pRetryCEQueuePi;
    volatile uint32_t* pRetryCEQueueCi;
    uint16_t* pRetryCeIndexQueue;
    uint32_t ceForRefillDFLQueuePi;
    uint32_t ceForRefillDFLQueueCi;
    volatile uint32_t* pCeForRefillDFLQueuePi;
    volatile uint32_t* pCeForRefillDFLQueueCi;
    uint16_t* pCEforRefillDFLQueue;
    uint32_t* pCpuStatus;

    uint8_t CPCDMAIODone;
    uint8_t CPCDMAIOStatus;
    #ifdef SUPPORT_CDMA_RESET_MSG
    uint8_t duringCDMAResetMessage;
    uint8_t Reserved;
    #else
    uint8_t Reserved[2];
    #endif

    #ifdef SUPPORT_TELEMETRY
    uint8_t TcInjecCountNonFatalErr;
    uint8_t TcIInjecCountFatalErr;
    uint8_t TcInjecCountPoorSgl;
    uint8_t Reserved2;
    uint64_t TcFaultErrCnt;
    uint64_t TcNonFaultErrCnt;
    uint64_t TcPoorConstructedSglCnt;
    #endif
    uint8_t* adminAbortCount;
    #ifdef MCR_TEST_HOOKS
    bool level1AbortFlag; ///< Flag to trigger a Level 1 abort via Error Injection Path. When this flag is enabled, it causes a command timeout of one I/O command.
    #endif
    uint8_t reserved2[2];
    #ifdef QOS_LATENCY_ERROR_HANDLING
    CP2FPMsgDataQoSPenalty_t* _pQosPenalty;
    uint32_t* _pQosVFBitmap[VF_MAX];
    uint64_t _qosPenaltyVfBitmap;
    uint8_t _qosPenaltyVf65Bitmap;
    #endif
    #ifdef WEIGHT_ROUND_ROBIN
    uint32_t* _pWeightRoundRobin;
    #endif
    uint8_t* CPU1SubmitAbortInfo;

    void (fpsCpu2::*pfCpu2MsgTable[msgOpNum])(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    Tcon_t* rTcon;
    Cortexm7_t* rCortexm7;
    #ifdef SUPPORT_FPS_REGISTER
    UcdCore0IbCmnRegisters_t* pIbCmnReg[UCD_CORE_NUM];
    UcdCore0ObCmnRegisters_t* pObCmnReg[UCD_CORE_NUM];
    #endif
    #ifdef SUPPORT_MSGERROR_INJECTION
    CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection;
    uint64_t* pErrInjectBitmap;
    uint8_t errInjectCnt;
    uint8_t totalErrInjectCnt;
    #endif
    #ifdef SUPPORT_UPDATE_TIMESTAMP
    /// < Time sync parameter
    uint32_t gTimerCounterBase;
    uint32_t gTimerCounterLast;
    uint32_t gTimerCounterDelta;
    uint32_t gTimerCounterCovert;
    uint32_t gTimerCounterCount;
    uint32_t gTimeSyncDone;
    #endif
    uint64_t gTimerChkAlive;
    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t hwOslPiAddr[OSL_END];
    #endif

    uint8_t* pQAbortBitmap;
    uint8_t* pCDMAIOAbortBit;
    uint8_t* pCa2IbPhysicalId;
    uint8_t* pIbPhysicalId2Ca;
    volatile uint32_t* pErrorPendingQPi;   ///< error pending queue pi address
    volatile uint32_t* pErrorPendingQCi;   ///< error pending queue ci address
    #ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
    uint32_t localTimeStamp;
    #endif

    uint64_t* pFLRRequestBitMapLocal;

    uint64_t* pFLRQueueBlockMap;
    uint8_t* pFLRQueueBlock65Map;

    UcdCore0IbCmnRegisters_t* _pIbCmnReg[UCD_CORE_NUM];

    /**
     *  @brief   Initialize resource when normal boot.
     *
     *  @return  None
     */
    void FpsCpuNormalBootInitialize();

    /**
     *  @brief   Initialize resource when normal boot.
     *
     *  @return  None
     */
    void FpsCpuStart();

    /**
     *  @brief   Initializes fpsCpu2 object.
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
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void RegisterFibers(void* pObj);
    /**
     *  @brief   Initialize UCD.
     *
     *  @return  None
     */
    void InitializeUCD();
    /**
     *  @brief   Initialize ErrorHandlingDataStructure
     *
     *  @return  None
     */
    void InitializeErrorHandlingDataStructure();
    /**
     *  @brief   Initialize CDMA.
     *
     *  @return  None
     */
    void InitializeCDMA();
    /**
     *  @brief   Initialize Message queue.
     *
     *  @return  None
     */
    void InitializeMessageQ();

    /**
     *  @brief   Initialize IO resources.
     *
     *  @return  None
     */
    void InitializeIOResource();

    /**
     *  @brief   Initialize IO resources.
     *
     *  @return  None
     */
    void InitializeRegisterBaseAddress();
    /**
     *  @brief   Initialize all member's fibers.
     *
     *  @return  None
     */
    void InitializeFiber();

    /**
     *  @brief   message msgOpFpStsChange handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleFpStsChange(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message MsgErrQSet handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleErrQset(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message MsgErrQSetSubOpAdminAbort handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgErrQSetSubOpAdminAbort(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    
    #ifdef LIONPERF_SUPPORT
    /**
     *  @brief   message msgOpFpModeChange handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleFpModeChange(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    
    /**
     *  @brief   message msgOpSetLogLevel handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleSetLogLevel(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);    
   
    /**
     *  @brief   Handle opcode msgOpSetLogLevel
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - CP2FP Message
     *
     *  @return  None
     */
    void HandleOpSetLogLevel(CP2FPMsgContext_t* pMsgSQContext);    

    #endif

    /**
     *  @brief   message handle msgOpVfSlotSQ2CQMapUpdate
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleFpVfSlotSq2CqMapUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
    /**

     *  @brief   message msgOpUpdateTimestampAddr handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleUpdateTimestampAddr(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    #endif

    /**
     *  @brief   message  handle msgOpVfUpdate
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleFpVfUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message msgOpCpCdmaIo handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleCpCdmaIo(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message msgOpKeyUpdate handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleKeyUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);


    /**
     *  @brief   Send message to HSM (CP1) Core. (this opcode is for FP to HSM core Message)
     *
     *  @param   msgOpErrQsetSubOp  - subopcode to be added to message to sent to HSM core
     *
     *  @return  void
     */
    void MsgHandleSendHsmReq(uint32_t msgOpErrQsetSubOp);

    /**
     *  @brief   message msgOpUcdQuery handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleFpUcdQuery(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    #ifdef LOGGING_NEW_SCHEME
    /**
     *  @brief   message msgOpTelemetryQuery handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleTelemetryQuery(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    #ifdef LIONPERF_SUPPORT
    /**
     *  @brief   message msgOpLogEnDisUpdate handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleLogEnDisUpdate(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    #endif

    /**
     *  @brief   Handle opcode msgOpLogEnDisUpdate - update log ext
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - CP2FP Message
     *
     *  @return  None
     */
    void MsgUpdateLogExt(CP2FPMsgContext_t* pMsgSQContext);
    #endif

    #ifdef LIONPERF_SUPPORT
    /**
     *    @brief     Handle CDMA status set message
     *
     *    @param     CP2FPMsgContext_t* pMsgSQContext - pointer to    CP2FP message entry
     *             uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *    @return  None
     */
    void MsgHandleOpCDMAStatSet(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    #endif

    #ifdef QOS_LATENCY_ERROR_HANDLING
    /**
     *    @brief     Handle QoS Penalty
     *
     *
     *    @return  None
     */
    void MsgHandleOpQoSPenaltySetup(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   Handle QoS Latency Timeout Error
     *
     *
     *  @return  None
     */
    void Cpu2HandleQosLatencyTimeoutError(uint16_t ceIndex);
    #endif
    #ifdef SUPPORT_MSGERROR_INJECTION
    /**
     *  @brief   message msgOpMsgErrorInjection handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgHandleMsgErrorInjection(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    #endif

    #ifdef SUPPORT_CDMA_RESET_MSG
    /**
     *  @brief   Handle CDMA reset
     *
     *
     *  @return  None
     */
    void HandleOpCDMAReset(CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx);
    #endif

    /**
     *  @brief   message msgOpShutdownReq handle
     *
     *  @param   CP2FPMessageInfo* pCp2FPMsgInfo - pointer to CP2FP message info
     *           CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *           CPCoreId_t CPSrcId - CP core ID
     *
     *  @return  None
     */
    void MsgHandleFpShutdownRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    #ifdef MCR_TEST_HOOKS
    /**
     *  @brief   message L1 Abort handle
     *
     *  @param   CP2FPMessageInfo* pCp2FPMsgInfo - pointer to CP2FP message info
     *           CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *           CPCoreId_t CPSrcId - CP core ID
     *
     *  @return  None
     */
    void HandleFpIoLvl1AbrtRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message Trigger Crashdump handle
     *
     *  @param   CP2FPMessageInfo* pCp2FPMsgInfo - pointer to CP2FP message info
     *           CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *           CPCoreId_t CPSrcId - CP core ID
     *
     *  @return  None
     */
    void HandleFpTriggerCrashRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   message msgOpInjectErrorReq handle
     *
     *  @param   CP2FPMessageInfo* pCp2FPMsgInfo - pointer to CP2FP message info
     *           CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *           CPCoreId_t CPSrcId - CP core ID
     *
     *  @return  None
     */
    void MsgHandleFpInjectErrorRequest(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);
    #endif

    /**
     *  @brief   not support message handle
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContext - pointer to  CP2FP message entry
     *           uint32_t cp2FpMsgIdx - index in CP2FP message Q
     *           uint8_t* pMsgState - CP2FP message state in message handling
     *
     *  @return  None
     */
    void MsgNotSupport(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   Handle slotsts when teardown occur
     *
     *  @param   uint8_t VFId - VF Id
     *
     *  @return  none
     */
    bool HandleTeardownSlotSts(uint8_t VFId);
    /**
     *  @brief   Get VF bitmap
     *
     *  @return  VF bit map
     */
    uint64_t GetVFBitmap(uint8_t VFId);

    /**
     *  @brief   Check all CP2FP Q done
     *
     *  @return  done : true / undone : false
     */
    bool ChkCPtoFPMsgFiberDone(void);

    /**
     *  @brief   Check all FP internal Q done
     *
     *  @return  done : true / undone : false
     */
    bool ChkRecvFPMsgFiberDone(void);

    /**
     *  @brief   Receive FP message from FP internal Q
     *
     *  @param   FPInterMsgHeader* pFpMsgHeader - pointer to  FP internal Q
     *           M7CoreId_t msgCpu  - cpu source of message delivery.
     *
     *  @return  sts with enum CP2FPMsgSts
     */
    CP2FPMsgSts RecvFPMsg(FPInterMsgHeader* pFpMsgHeader, M7CoreId_t msgCpu);

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
     *  @brief   Handle status change
     *
     *  @return  None
     */
    CP2FPMsgSts FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone);

    /**
     *  @brief   Handle FP internal Q Response message status
     *
     *  @param   CP2FPMsgContext_t* pCP2FPMsg - message entry in CP2FPMsgQ
     *           uuint8_t fpSts  - status in FP internal message entry
     *           M7CoreId_t msgCpu - cpu source of message delivery.
     *
     *  @return  None
     */
    void HandleFPRespMsgSts(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pCP2FPMsg, uint8_t cp2FpMsgIdx, uint8_t fpSts, M7CoreId_t msgCpu);

    /**
     *  @brief   Handle CP2FP message
     *
     *  @param   CP2FPMsgContext_t* pMsgSQContent - CP2FP Message
     *           uint32_t cp2FpMsgIdx - Index in CP2FP message queue
     *
     *  @return  None
     */
    void HandleCP2FPMsg(CP2FPMessageInfo* pCp2FPMsgInfo, CP2FPMsgContext_t* pMsgSQContext, uint32_t cp2FpMsgIdx, uint8_t* pMsgState, CPCoreId_t CPSrcId);

    /**
     *  @brief   Check CP2FP message done and update CP2FP message Ci
     *
     *  @param   none
     *
     *  @return  none
     */
    void ChkUpdateCptoFpMsgCi(CP2FPMessageInfo* pCp2FPMsgInfo, CPMsgQId msgQId);

    /**
     *  @brief   Send FP2CP message
     *
     *  @param   CP2FPMsgContext_t* pMsgInfo - CP2FP Message
     *           uint8_t* pData - Data pointer contains data and will be transferred to CP.
     *           uint8_t resp - if resp is 1, it means response data
     *           uint8_t length - Data length
     *
     *  @return  message status (CP2FPMsgSts)
     */
    CP2FPMsgSts SendFP2CPMsg(CP2FPMsgContext_t* pMsgInfo, uint8_t* pData, uint8_t resp, uint8_t length, CPMsgQId msgCP);//means data from cp and return cp sts

    /**
     *  @brief   Check VF update message condition
     *
     *  @param   CP2FPMsgDataVfUpdate_t* ptmpData - Data structure of CP2FPMsgDataVfUpdate_t
     *
     *  @return  true : condition is incorrect / false : condition is correct
     */
    bool ChkFpVfUpdate(CP2FPMsgContext_t* pMsgSQContext);

    /**
     *  @brief   Check VF slot Sq to Cq map update condition
     *
     *  @param   CP2FPMsgDataVfSlotSq2CqMapUpdate_t* ptmpData - Data structure of CP2FPMsgDataVfSlotSq2CqMapUpdate_t
     *
     *  @return  true : condition is incorrect / false : condition is correct
     */
    bool ChkFpVfSlotSq2CqMapUpdate(CP2FPMsgContext_t* pMsgSQContext);

    /**
     *  @brief   Initial CDMA register
     *
     *
     *  @return  None
     */
    void CDMAInit();

    /**
     *  @brief   Handle CDMA fatal error slot
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *  @param   uint32_t slotId - slot id that error occur
     *
     *  @return  None
     */
    void HandleFatalErrorSlot(void* pObj, uint32_t slotId);

    /**
     *  @brief   Entry point function for running the Fiber fpsCpu2ProcessCdmaCqOslFiber
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void FpsCpu2ProcessCdmaCqOslFiber(void* pObj);

    /**
     *  @brief   Entry point function for running the Fiber fpsCpu2ProcessGcmCmdFiber
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void FpsCpu2ProcessGcmCmdFiber(void* pObj);

    /**
     *  @brief   Entry point function for FpsCpu2CP2FPServiceFiber
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void FpsCpu2CP2FPServiceFiber(void* pObj);

    /**
     *  @brief   Entry point function for FpsCpu2ReceiveFPMsgFiber
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void FpsCpu2ReceiveFPMsgFiber(void* pObj);

    /**
     *  @brief   Entry point function for FpsCpu2HandleCDMAFatalError
     *
     *  @param   void* pObj - pointer to fpsCpu2 object
     *
     *  @return  None
     */
    static void FpsCpu2HandleCDMAFatalErrorFiber(void* pObj);

    /**
     *  @brief   Entry point function for resuming FpsCpu2ReceiveFPMsgFiber if need
     *
     *  @param   void* pObj - pointer to Cpu2 object
     *
     *  @return  None
     */
    void CheckFPMsgFiberNeedResume(void* pObj);

    /**
     *  @brief   Entry point function for resuming FpsCpu2CP2FPServiceFiber if need.
     *
     *  @param   void* pObj - pointer to Cpu2 object
     *
     *  @return  None
     */
    void CheckCP2FPMsgFiberNeedResume(void* pObj);

    /**
     *  @brief   Entry point function for start checking cdma fatal error.
     *
     *  @param   void* pObj - pointer to Cpu2 object
     *
     *  @return  None
     */
    void FpsCpu2CheckCDMAFatalErrorIrq(void* pObj);

    void FpsCpu2SendCDMAAbortRequestToCpu1(uint16_t abortCeIndex);
    void FpsCpu2SendRetryCeRequestToCpu0(uint16_t retryCeIndex);
    void FpsCpu2SendRefillDFLRequestToCpu0(uint16_t ceIndex);
    void Cpu2CdmaErrorCmdHandler(void);
    uint8_t ChkRetryTimesExceeded(uint16_t ceIndex);
    CmdEntryTinyHostErrCode_t FatalErrorErrCode(uint32_t cdmaErrorStatus0,uint32_t cdmaErrorStatus1);
    CmdEntryStatus_t NonFatalStatusCode(uint32_t cdmaErrorStatus0, uint32_t cdmaErrorStatus1);
    CmdEntryTinyHostErrCode_t NonFatalErrorCode(uint32_t cdmaErrorStatus0,uint32_t cdmaErrorStatus1);
    bool FpsCpu2ReturnErrorCommandToHost(LionFPCQEStatusCode cqeStatus, LionFPCQEErrorCode cqeError);
    bool FpsCpu2ReturnCPRespErrCmdToHost(void* pObj, uint16_t ceIndex, LionFPCQEErrorCode cqeError);
    void FpsCpu2SearchAbortMsgInMsgQueue(void);
    void SetupHardwareOffloadRegister(uint8_t ucdQueueID, uint8_t ibIndex, uint8_t obIndex, uint8_t dflIndex, uint8_t oslIndex);

    /**
     *  @brief   To check the cause of reset requset
     *
     *
     *
     *  @return  None
     */
    void FpsCpu2CheckResetIrqCause(void);


    /**
     *  @brief     To handle CDMA key vault memory uncorrectable and correctable error
     *
     *  @param     void* pObj - pointer to Cpu2 object
     *
     *  @return    None
     */
    static void FpsCpu2HandleCDMAKeyCorrtableErrFiber(void* pObj);

    /**
     *  @brief     To handle FLR and PERST request from CP
     *
     *  @param     void* pObj - pointer to Cpu2 object
     *
     *  @return    None
     */
    static void FpsCpu2HandleResetFiber(void* pObj);

    /**
     *  @brief     To resume CDMA reset and error fatal error handling fiber
     *
     *
     *  @return    None
     */
    void TriggerCDMAResetFiber();
    #ifdef LIONPERF_SUPPORT
    /**
     *  @brief     Initialize Fw update backup header
     *
     *  @param     FwUpdateDataHeader* pDataHeader - Fw update header pointer
     *
     *  @return    checksum value
     */
    void InitBackupDataHeader(FwUpdateDataHeader* pDataHeader);

    /**
     *  @brief    Calculate check sum
     *
     *  @param     uint32_t* pData - Calculated data pointer
     *             uint32_t len -  data length
     *
     *  @return    checksum value
     */
    uint32_t CalCheckSum(uint32_t* pData, uint32_t len, uint8_t check);

    /**
     *  @brief   Backup UCD DFL info.
     *
     *  @param   None
     *
     *  @return  None
     */
    void BackupDFLInfo();

    /**
     *  @brief   Backup Logging info.
     *
     *  @param   None
     *
     *  @return  None
     */
    void BackupLoggingInfo();

    /**
     *  @brief   Get Fw update Info.
     *
     *  @param   FWupdateBackupInfo* pFWupdateInfo - Fw update Info
     *           RecoverDataBlk dataType - data blk type
     *  @return  None
     */
    void GetFwUpdateInfo(FWupdateBackupInfo* pFWupdateInfo, RecoverDataBlk dataType);
    #endif
    #ifdef SUPPORT_VF65_QB65_UCD_QUERY_TWICE
    /**
     * @brief   Initialize parameters.
     *
     * @return  None.
     */
    void init_ucd_parameters();
    #endif // End of SUPPORT_VF65_QB65_UCD_QUERY_TWICE

    void FpsCpu2PrintInfoLogInvMsgState(uint32_t msgOpCode, uint32_t curMsgState);

    #ifdef DEBUG_BUILD
    void FpsCpu2PrintErrInfoLogByErrStsType(LionFPCmdMetaData_t* pFpCmd, CmdEntry_t* pCmdEntry, uint8_t errStsType, uint32_t cdmaErrSts0, uint32_t cdmaErrSts1);
    #endif

    constexpr static uint32_t   cFiberWeightFpsCpu2ProcessCdmaCqOslFiber = 1;
    constexpr static uint32_t   cFiberWeightFpsCpu2CP2FPServiceFiber = 1;
    constexpr static uint32_t   cFiberWeightFpsCpu2RecvFPMsgFiber = 1;
    constexpr static uint32_t   cFiberWeightFpsCpu2HandleCDMAFatalErrorFiber = 1;
    constexpr static uint32_t   cFiberWeightFpsCpu2HandleResetFiber = 1;
    constexpr static uint32_t   cFiberWeightFpsCpu2HandleCDMAKeyCorrtableErrFiber = 1;
    constexpr static uint32_t   cFiberWeightCheckHeartbeatFiber = 1;

protected:
    // empty

private:
    FpsUcdObq_t _ucdObq;
    CmdEntry_t* _pCmdEntryArrayBase;
    CmdEntryTiny_t* _pCmdEntryArrayTinyBase;
    CdmaCq_t _cdmaCq;
    uint8_t* _pIbQ2ObQ;
    uint8_t* _pSlotFlagSts;
    VFNodeInfo_t* _pVfInfoBase;
    QueueBlockInfo_t* _pQueueBlockInfoBase;
    ErrorQueueContext_t* _pErrorPendingQ;   ///< error pending queue base address
    uint8_t* _pServiceIndicator;

    uint64_t* _pVFEnBitmap;
    uint32_t* _pVF65EnBitmap;
    uint64_t _VfTeardownBitmap;
    uint8_t _Vf65TeardwonBitmap;
    // uint8_t* _pQAbortBitmap;
    // uint32_t* _cdmaFatalErrorFlag;
    bool _uncorrectableKeyErrorOccurred;
    uint32_t _CDMACorrectableKeyErrorThreshold;
    ResetRequestMagicNumber_t _curResetRequest;

    M7Fiber     _fpsCpu2ProcessCdmaCqOslFiber;
    M7Fiber     _fpsCpu2CP2FPServiceFiber;
    M7Fiber     _fpsCpu2RecvFpMsgFiber;
    M7Fiber     _fpsCpu2HandleCDMAFatalErrorFiber;
    M7Fiber     _fpsCpu2HandleCDMAKeyCorrtableErrFiber;
    M7Fiber     _fpsCpu2HandleResetFiber;
    M7Fiber     _fpsCpu2CheckHeartbeatFiber;

    uint32_t* _cdmaFatalErrorFlag;

    CDMAFatalErrorHandlingState_t _cdmaFatalErrorHandleState;

    CDMACorrectableKeyErrorHandlingState_t _cdmaCorrectableKeyErrorHandleState;
    ResetHandlingState_t _resetHandlingState;

    /**
     *  @brief    Handle CDMA fatal error slot status
     *
     *  @param    void* pObj - pointer to Cpu2 object
     *
     *  @param    errorSlotId - The ID of error slot register
     *
     *  @return   None
     */
    void _HandleFatalErrorSlotStatus(void* pObj, uint8_t errorSlotId);

};

extern fpsCpu2 gFpsCpu2;
#endif  // FP3CORE_FPSCPU2_H_
