// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1.cpp
//! @brief  FpsCpu1 Component Group
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu1.h"
#include "Heartbeat.h"
extern "C"
{
#include "crashdump.h"
#include "API_GCMTagCorrect.h"
}

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

uint32_t gQBCmdExistSnapBitMap[QB_MAX];
uint32_t gSq0HighSnapBitMap[QB_MAX], gSq1LowSnapBitMap[QB_MAX];
uint32_t gVFCmdExistBitMap[VF_MAX], gVFCreditRemainBitMap[VF_MAX];
uint32_t gTotalCredit = 0;
uint32_t gBanQBmap[QB_MAX] = {0};

#ifdef WEIGHT_ROUND_ROBIN
uint32_t gWeightRoundRobin = 0;
uint32_t gHighQueueHitCnt[MAX_SUPPORT_FUNC_NUM];
#endif

Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
Ucd_t* rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
Cdma_t* rCdma = (Cdma_t*)CDMA_REG_ADDR;

#ifdef QUEUE_BLOCK_BALANCE
uint16_t gQBCredit = 0;
#endif

#define GET_HIGH_PRIORITY_CA_INDEX(qbSelect, qbIndex) (qbSelect < QB64) ? qbIndex : QB64_HIGH_QUEUE
#define GET_LOW_PRIORITY_CA_INDEX(qbSelect, qbIndex)  (qbSelect < QB64) ? (FPS_SLOT_LOW_PRIORITY_START_INDEX + qbIndex) : QB64_LOW_QUEUE
#ifdef QOS_LATENCY_ERROR_HANDLING
uint32_t gQosLatencyErrVFBitmap[VF_MAX];
uint32_t gPenaltyQBmap[QB_MAX] = {0};
uint8_t gLastQBIndex[65] = {0};
#endif

#ifdef QOS_LATENCY_TEST
uint32_t x = 0;
uint32_t y = 1;
#endif

#define AAD_ALIGNMENT_CHECK 32

// The data block encrypted with AES-XTS will never exceed 2^{20} AES block i.e. 16MB
#define MAX_AES_XTS_DATA_LEN (0x10<<20)
//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

bool FpsCpu1ChkFPMsgStsDone(CP2FPMsgSts sts)
{
    bool done = false;
    switch (sts)
    {
        case msgSuccess:
        case msgInvalidField:
        case msgNotSupport:
        {
            done = true;
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

void fpsCpu1::Initialize(M7CompGroupId_t compId)
{
    // Init fiber parameters
    InitializeFiber();
    RegisterComponentGroup(compId);
}

void fpsCpu1::RegisterComponentGroup(M7CompGroupId_t compId)
{
    // Register Component Group
    M7FiberScheduler_RegisterCompGroup(cM7Core1, compId,
                                       static_cast<M7FiberId_t>(fpsCpu1FiberId_t::cNumberOfFibers),
                                       reinterpret_cast<OnM7FiberSchedulerInitializedFptr_t>(&fpsCpu1::RegisterFibers),
                                       static_cast<void*>(this));
}

void fpsCpu1::RegisterFibers(void* pObj)
{
    fpsCpu1* pThis = static_cast<fpsCpu1*>(pObj);
    pThis->_fpsCpu1QueueManagerFiber.Register(pThis, &fpsCpu1::FpsCpu1QueueManagerFiber, "FpsCpu1QueueManagerFiber", false);
    pThis->_fpsCpu1RecvFpMsgFiber.Register(pThis, &fpsCpu1::FpsCpu1ReceiveFPMsgFiber, "FpsCpu1RecvFPMsgFiber");
    pThis->_fpsCpu1CPCDMAIOFiber.Register(pThis, &fpsCpu1::FpsCpu1CPCDMAIOFiber, "FpsCpu1CPCDMAIOFiber");

    pThis->_fpsCpu1RecvFpMsgFiber.Wait();
    pThis->_fpsCpu1CheckHeartbeatFiber.Register(pThis, &CheckHeartbeatFiber, "FpsCpu1CheckHeartbeatFiber");
    pThis->_fpsCpu1CheckHeartbeatFiber.Activate();
}

void fpsCpu1::InitializeFiber()
{
    pCpuStatus = (uint32_t*)(PSRAM_FP_CPU1_STATUS_ADDR);
    writel(FP_STS_INIT_START, pCpuStatus);

    // Init fiber parameters FpsCpu1QueueManagerFiber
    M7FiberParameters_t fiberParamsFpsCpu1QueueManagerFiber = {0};
    fiberParamsFpsCpu1QueueManagerFiber.fiberWeight = cFiberWeightFpsCpu1QueueManagerFiber;

    // Initialize Fiber object FpsCpu1QueueManagerFiber
    _fpsCpu1QueueManagerFiber.Initialize(cM7Core1, cM7CompGroupIo, \
                                         static_cast<M7FiberId_t>(fpsCpu1FiberId_t::cFpsCpu1QueueManagerFiberId), fiberParamsFpsCpu1QueueManagerFiber);

    //Init fiber parameters object fpsCpu1RecvFpMsgFiber
    M7FiberParameters_t fiberParamsRecvFpMsgFiber = {0};
    fiberParamsRecvFpMsgFiber.fiberWeight = cFiberWeightfpsCpu1RecvFPMsgFiber;

    // Initialize Fiber object fpsCpu1RecvFpMsgFiber
    _fpsCpu1RecvFpMsgFiber.Initialize(cM7Core1, cM7CompGroupIo,                                             \
                                      static_cast<M7FiberId_t>(fpsCpu1FiberId_t::cFpsCpu1RecvFpMsgFiberId), \
                                      fiberParamsRecvFpMsgFiber);

    //Init fiber parameters object fpsCpu1CPCDMAIOFiber
    M7FiberParameters_t fiberParamsFpsCpu1CPCDMAIOFiber = {0};
    fiberParamsFpsCpu1CPCDMAIOFiber.fiberWeight = cFiberWeightfpsCpu1CPCDMAIOFiber;

    // Initialize Fiber object fpsCpu1CPCDMAIOFiber
    _fpsCpu1CPCDMAIOFiber.Initialize(cM7Core1, cM7CompGroupIo,                                            \
                                     static_cast<M7FiberId_t>(fpsCpu1FiberId_t::cFpsCpu1CPCDMAIOFiberId), \
                                     fiberParamsFpsCpu1CPCDMAIOFiber);
    //Init fiber parameters object CheckHeartbeat
    M7FiberParameters_t fiberParamsCheckHeartbeat = {0};
    fiberParamsCheckHeartbeat.fiberWeight = cFiberWeightCheckHeartbeatFiber;

    // Initialize Fiber object CheckHeartbeat
    _fpsCpu1CheckHeartbeatFiber.Initialize(cM7Core1, cM7CompGroupIo, \
                                      static_cast<M7FiberId_t>(fpsCpu1FiberId_t::cFpsCpu1CheckHeartbeatFiberId), fiberParamsCheckHeartbeat);


    rFps = (Fps_t*)FPS_REG_ADDR;
    rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
    rCdma = (Cdma_t*)CDMA_REG_ADDR;
    rTcon = (Tcon_t*)TCON_REG_ADDR;
    rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;

    pCP0toFPMsgQ = (CP2FPMsgContext_t*)(PSRAM_CP0toFP_REQ_MSG_ADDR);
    pCP1toFPMsgQ = (CP2FPMsgContext_t*)(PSRAM_CP1toFP_REQ_MSG_ADDR);

    pCPU2toCPU1Pi = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU1_PI_ADDR);
    pCPU2toCPU1Ci = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU1_CI_ADDR);
    pCPU2toCPU1MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU2_2_CPU1_MSG_ADDR);

    pCPU1toCPU2Pi = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU2_PI_ADDR);
    pCPU1toCPU2Ci = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU2_CI_ADDR);
    pCPU1toCPU2MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU1_2_CPU2_MSG_ADDR);

    pCPU0toCPU1Pi = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU1_PI_ADDR);
    pCPU0toCPU1Ci = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU1_CI_ADDR);
    pCPU0toCPU1MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU0_2_CPU1_MSG_ADDR);

    pCPU1toCPU0Pi = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU0_PI_ADDR);
    pCPU1toCPU0Ci = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU0_CI_ADDR);
    pCPU1toCPU0MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU1_2_CPU0_MSG_ADDR);

    M7_MEM_SET((void*)pCPU1toCPU2Pi, 0, 4);
    M7_MEM_SET((void*)pCPU2toCPU1Ci, 0, 4);

    M7_MEM_SET((void*)pCPU1toCPU0Pi, 0, 4);
    M7_MEM_SET((void*)pCPU0toCPU1Ci, 0, 4);

    CPU2toCPU1Ci = 0;
    CPU1toCPU2Pi = 0;
    CPU0toCPU1Ci = 0;
    CPU1toCPU0Pi = 0;

    for (uint8_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
    {
        cmdArrayCi[i] = 0;
    }

    #ifdef WEIGHT_ROUND_ROBIN
    for (uint8_t i = 0; i < MAX_SUPPORT_FUNC_NUM; i++)
    {
        gHighQueueHitCnt[i] = 0;
    }
    #endif

    // resource group and key resource init
    _rgid2keyValid = (uint8_t*)(M7_FPS_CPU12_KEY_TABLE_RGID_TO_VALID_ADDR);
    _rgid2OwnerVfid = (uint8_t*)((uint32_t)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)(M7_FPS_CPU01_KEY_TABLE_RGID_TO_OWNER_VFID_ADDR)));
    _key2OwnerVfid = (uint8_t*)((uint32_t)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)(M7_FPS_CPU01_KEY_TABLE_KEY_TO_OWNER_VFID_ADDR)));
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    _pKey2SessionID = (uint16_t*)((uint32_t)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)(M7_FPS_CPU01_KEY_SESSION_ID_ADDR)));
    _pKey2AppID = (uint8_t*)((uint32_t)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)(M7_FPS_CPU01_KEY_APP_ID_ADDR)));
    _pKeyIsEphemeral = (uint8_t*)((uint32_t)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)(M7_FPS_CPU01_KEY_FLAG_ADDR)));

    if(gResetType == cPor)
    {
        M7_MEM_SET((void*)_rgid2keyValid, 0, KEY_TABLE_RGID_TO_VALID_SIZE);
        M7_MEM_SET((void*)_pKey2SessionID, 0, M7_SHARE_KEY_SESSION_ID_SIZE);
        M7_MEM_SET((void*)_pKey2AppID, 0, M7_SHARE_KEY_APP_ID_SIZE);
        M7_MEM_SET((void*)_pKeyIsEphemeral, 1, M7_SHARE_KEY_FLAG_SIZE); // set all keys to ephemeral flag for testing
    }
    #endif

    // CDMA SQ init
    _cdmaSq = {0};
    cdmaSqPi = 0;
    cdmaSqCi = 0;
    _cdmaSq.pCdmaSqBase = (CdmaSqCmdDescr_t*)(M7_FPS_CPU12_CDMA_SQ_ENTRY_ADDR);
    M7_MEM_SET((void*)M7_FPS_CPU12_CDMA_SQ_ENTRY_ADDR, 0, CDMA_SQ_SIZE);

    rFps->fpsFp2hweRegRegisters[cFp2HweWq04CdmaSq].fpsFp2hweFpToHweQSize.all = FPS2HW_Q_SZ_POW; ///< 64 entries
    rFps->fpsFp2hweRegRegisters[cFp2HweWq04CdmaSq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr = \
        (uint32_t)(&(rCdma->deliveryQueue0ProducerIndex.all));
    #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
    cdmaSqPiHwAddr = (uint32_t)(&(rCdma->deliveryQueue0ProducerIndex.all));
    _cdmaSq.PiHwAddr = cdmaSqPiHwAddr;
    #endif
    _cdmaSq.pHwPi = (uint32_t*)&(rFps->fpsFp2hweRegRegisters[cFp2HweWq04CdmaSq].fpsFp2hweFpToHweQPiIndirectDataPort.all);
    _cdmaSq.pHwCi = (uint32_t*)&(rFps->fpsFp2hweRegRegisters[cFp2HweWq04CdmaSq].fpsFp2hweFpToHweQCiShadow.all);
    _cdmaSq.pHwStatus = (uint32_t*)&(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all);

    //Resetting CDMA SQ during initialization
    writel(0, _cdmaSq.pHwPi);
    writel(0, _cdmaSq.pHwCi);

    #ifdef SUPPORT_MSGERROR_INJECTION
    errInjectFlag = 0;
    pErrInjectBitmap = (uint64_t*)(CPU1AccessCPU2TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU12_ERR_INJECTION_BITMAP)));
    pMsgErrorInjection = (CP2FPMsgDataMsgErrorInjection_t*)(CPU1AccessCPU2TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU12_ERR_INJECTION_START)));

    #endif
    #ifdef CDMA_CMD_COUNT
    cdmaCmdSlotQueuePi = 0;
    cdmaCmdSlotQueueCi = 0;
    rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueSize.all = 0x7;   // 2^7 = 128
    pCdmaCmdSlotQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueProducerIndex.all);
    pCdmaCmdSlotQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueConsumerIndex.all);
    writel(0, pCdmaCmdSlotQueuePi);
    writel(0, pCdmaCmdSlotQueueCi);
    #endif // End of CDMA_CMD_COUNT

    cdmaSlotAbortQueuePi = 0;
    cdmaSlotAbortQueueCi = 0;
    pCdmaSlotAbortQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq01].fpsCpuxToCpuyQueueProducerIndex.all);
    pCdmaSlotAbortQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq01].fpsCpuxToCpuyQueueConsumerIndex.all);
    writel(0, pCdmaSlotAbortQueuePi);
    writel(0, pCdmaSlotAbortQueueCi);

    pCdmaSlotAbortQueue = (uint16_t*)M7_FPS_CPU12_CDMA_SLOT_ABORT_QUEUE;

    _pCmdArrayBase = (CmdEntry_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_CMD_ARRAY_BASE_ADDR));
    _pCmdArrayTinyBase = (CmdEntryTiny_t*)(CPU1AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_CMD_ARRAY_TINY_BASE_ADDR));   // via CPU2
    _pVfInfoBase = (VFNodeInfo_t*)(CPU1AccessCPU0TCMMem((uint32_t)(M7_FPS_CPU01_VF_INFO_BASE)));
    _pQBlockInfoBase = (QueueBlockInfo_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_QB_INFO_BASE));
    _pVFEnBitmap = (uint64_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_ENABLE_BIT_MAP_ADDR));
    _pQBEnBitmap = (bit64*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_QB_ENABLE_BIT_MAP_ADDR));
    _pVF65EnBitmap = (uint32_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_65_ENABLE_BIT_MAP_ADDR));
    _pQB65EnBitmap = (uint8_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_QB_65_ENABLE_BIT_MAP_ADDR));
    _pQAbortBitmap = (uint8_t*)(M7_FPS_CPU12_ABORT_BITMAP_ADDR);
    _CPU1SubmitAbortInfo = (uint8_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_M7_CPU1_SUBMIT_ABORT_INFO));
    _pCDMAIOAbortBit = (uint8_t*)(M7_FPS_CPU12_CDMA_IO_ABORT_BIT_ADDR);
    _pVfCmdExistBitMap = (uint64_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_CMD_EXIST_BIT_MAP_ADDR));
    _pVf65CmdExistBitMap = (uint32_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_65_CMD_EXIST_BIT_MAP_ADDR));

    _pTotalCredit = (volatile int32_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_TOTAL_CREDIT_ADDR));
    _pVfCredit = (volatile int32_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_CREDIT_INFO_BASE));
    _pVfRemainCredit = (volatile int32_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_VF_REMAIN_CREDIT_INFO_BASE));

    pCa2IbPhysicalId = (uint8_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_CA_2_IBPHYQID_TABLE));
    pIbPhysicalId2Ca = (uint8_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU01_IBPHYQID_2_CA_TABLE));

    M7_MEM_SET((void*)M7_FPS_CPU12_ABORT_BITMAP_ADDR, 0, M7_ABORT_BITMAP_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU12_CDMA_IO_ABORT_BIT_ADDR, 0, M7_FPS_CPU12_CDMA_IO_ABORT_BIT_SIZE);

    //init CPIO internal buffer
    pCPCDMAdata = (CP2FPMsgDataOpCpCdmaIo_t*)(M7_FPS_CPU12_CP_IO_INTERNAL_BUF_ADDR);
    CPCDMAStatus = 0;
    CPCDMAError = 0;
    M7_MEM_SET((void*)pCPCDMAdata, 0, sizeof(CP2FPMsgDataOpCpCdmaIo_t));
    hasCPCDMACmd = 0;

    _fpMode = (Fastpath_OP_Mode_t*)M7_FPS_CPU12_FP_MODE_ADDR;
    *_fpMode = DEFAULT_FP_MODE;

    _teardownQueueBlockBitMap = 0;
    _teardownQueueBlock65BitMap = 0;

    _cdmaFatalErrorFlag = (uint32_t*)((uint32_t)M7_FPS_CPU12_CDMA_FATAL_ERROR_OCCUR_FLAG);
    *_cdmaFatalErrorFlag = 0;
    #ifdef SUPPORT_UPDATE_TIMESTAMP
    gTimerCounterBase = 0;
    gTimerCounterLast = 0;
    gTimerCounterDelta = 0;
    gTimerCounterCovert = 0;
    gTimerCounterCount = 0;
    #endif

    gTotalCredit = 0;

    gQBCmdExistSnapBitMap[QB0_QB31] = 0;
    gSq0HighSnapBitMap[QB0_QB31] = 0;
    gSq1LowSnapBitMap[QB0_QB31] = 0;

    gQBCmdExistSnapBitMap[QB32_QB63] = 0;
    gSq0HighSnapBitMap[QB32_QB63] = 0;
    gSq1LowSnapBitMap[QB32_QB63] = 0;

    gQBCmdExistSnapBitMap[QB64] = 0;
    gSq0HighSnapBitMap[QB64] = 0;
    gSq1LowSnapBitMap[QB64] = 0;

    #ifdef QOS_LATENCY_ERROR_HANDLING
    _pQosPenalty = (CP2FPMsgDataQoSPenalty_t*)((uint32_t)M7_FPS_CPU12_QOS_PENALTY_ADDR);
    _pQosVFBitmap[VF0_VF31] = (uint32_t*)(M7_FPS_CPU12_QOS_VF_BITMAP_ADDR);
    _pQosVFBitmap[VF32_VF63] = _pQosVFBitmap[VF0_VF31] + 1;
    _pQosVFBitmap[VF64] = (uint32_t*)(M7_FPS_CPU12_QOS_VF_65_BITMAP_ADDR);
    #endif

    #ifdef WEIGHT_ROUND_ROBIN
    _pWeightRoundRobin = (uint32_t*)(M7_FPS_CPU12_WEIGHT_ROUND_ROBIN_ADDR);
    #endif

    pFLRRequestBitMapLocal = (uint64_t*)(CPU1AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_FLR_REQUSET_BIT_MAP_LOCAL));

    pFLRQueueBlockMap = (uint64_t*)(CPU1AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_FLR_QUEUE_BLOCK_BIT_MAP_LOCAL));
    pFLRQueueBlock65Map = (uint8_t*)(CPU1AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_FLR_QUEUE_BLOCK_65_BIT_MAP_LOCAL));

    #ifdef LIONPERF_SUPPORT
    if(gResetType == cFwUpdateWarmReset)
    {
        FwUdSts FwChecksumSts = CheckBackupData();
        uint8_t FwUpdateSts = GetFwUpdateStatus(FwChecksumSts);
        if( !FwUpdateSts )
        {
            FWupdateBackupInfo FWupdateInfo;
            GetFwUpdateInfo(&FWupdateInfo, cLoggingData);
            LoggingResumeBootInit(&FWupdateInfo);
        }
        else
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Firmware update failed, FwChecksumSts:0x%X FwUpdateSts:0x%X\n", (FwChecksumSts, FwUpdateSts )), "16,16");
        }
    }
    #endif

    writel(FP_STS_INIT_DONE, pCpuStatus);
}

void ExitQueueManagerFiber()
{
    gTotalCredit = 0;

    gQBCmdExistSnapBitMap[QB0_QB31] = 0;
    gQBCmdExistSnapBitMap[QB32_QB63] = 0;
    gQBCmdExistSnapBitMap[QB64] = 0;
}

/* FW sets the Chunk-Transfer Count to 2KB since the Dummy Slave port
   is an AHB port with 1/2 the total bandwidth of an AXI port
 */
ATTR_ALWAYS_INLINE void fpsCpu1::SendIdleCmd(int32_t credit)
{
    CdmaSq_t* pCdmaSq = &_cdmaSq;
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    volatile CdmaSqCmdDescr_t* pCdmaSqe;
    uint32_t chunkByte = CHUNK_SIZE;
    // NOTE:IF qos latency penalty are exceeds 50% of default credit(4K), the credit may be overflow.

    // Remaining credit must be 4k aligned
    while (credit > 0)
    {
        if (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
        {
            cdmaSqCi = readl(pCdmaSq->pHwCi);
            while (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
            {
                if (readl(_cdmaFatalErrorFlag))
                {
                    ExitQueueManagerFiber();
                    return;
                }
                cdmaSqCi = readl(pCdmaSq->pHwCi);
            }
        }

        {pCdmaSqe = pCdmaSqBase + cdmaSqPi;}
        pCdmaSqe->dw0 = CDMA_IDLE_CMD_CPU_ID | (CONTINUE_CMD << CDMA_SQE_DW0_CMD_STATE_SHIFT) | \
                        (CDMA_OPCODE_IDLE << CDMA_SQE_DW0_OPCODE_SHIFT) |                       \
                        (CDMA_LIST_1 << CDMA_SQE_DW0_CDMA_LIST_NUM_SHIFT);
        pCdmaSqe->dw1 = ((chunkByte >> 1) << CDMA_SQE_DW1_CHUNK_BCNT_SHIFT);
        pCdmaSqe->dw2 = 0;
        pCdmaSqe->dw3 = 0;

        DMB();

        cdmaSqPi = QUEUE_INC(cdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);
        writel(cdmaSqPi, pCdmaSq->pHwPi);
        #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
        writel(cdmaSqPi, cdmaSqPiHwAddr);
        #endif

        credit -= chunkByte;
    }
}

ATTR_ALWAYS_INLINE void SnapCommandExistStatus()
{
    uint32_t tmp = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_4));

    gSq0HighSnapBitMap[QB64] = tmp & 0x1;
    gSq1LowSnapBitMap[QB64] = (tmp >> 1) & 0x1;
    gQBCmdExistSnapBitMap[QB64] = (gSq0HighSnapBitMap[QB64] | gSq1LowSnapBitMap[QB64]);

    gSq0HighSnapBitMap[QB0_QB31] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_0));
    gSq0HighSnapBitMap[QB32_QB63] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_1));
    gSq1LowSnapBitMap[QB0_QB31] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_2));
    gSq1LowSnapBitMap[QB32_QB63] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_3));

    gQBCmdExistSnapBitMap[QB0_QB31] = gSq0HighSnapBitMap[QB0_QB31] | gSq1LowSnapBitMap[QB0_QB31];
    gQBCmdExistSnapBitMap[QB32_QB63] = gSq0HighSnapBitMap[QB32_QB63] | gSq1LowSnapBitMap[QB32_QB63];
}

#ifdef QOS_LATENCY_ERROR_HANDLING
ATTR_ALWAYS_INLINE void fpsCpu1::QosLatencyPenaltyHandling(uint8_t vfGroupIndex)
{
    VFNodeInfo_t* pVfInfo;
    uint8_t vfId, disBit;
    uint32_t lastRoundQBBitmapMask;

    while (gQosLatencyErrVFBitmap[vfGroupIndex])
    {
        vfId = FindNextBit32(gQosLatencyErrVFBitmap[vfGroupIndex]) + (vfGroupIndex << VF_QB_32_SHIFT);
        disBit = vfId & 0x1f;
        gQosLatencyErrVFBitmap[vfGroupIndex] &= ~(BIT(disBit));
        pVfInfo = &_pVfInfoBase[vfId];
        if (pVfInfo->qosPenaltyPeriod == 0)
        {
            /* recover original credit */
            *_pQosVFBitmap[vfGroupIndex] &= ~(BIT(disBit));
            *_pTotalCredit = *_pTotalCredit - _pVfCredit[vfId] + pVfInfo->credit;
            _pVfCredit[vfId] = pVfInfo->credit;

            continue;
        }

        if (gLastQBIndex[vfId] < 31)
        {
            lastRoundQBBitmapMask = ~(0xffffffff << (gLastQBIndex[vfId] + 1));
            gPenaltyQBmap[QB0_QB31]  |= (_pQBEnBitmap[vfId].dw[QB0_QB31] & lastRoundQBBitmapMask);
        }
        else if (gLastQBIndex[vfId] < 63)
        {
            lastRoundQBBitmapMask = ~(0xffffffff << ((gLastQBIndex[vfId] + 1) & 0x1f));
            gPenaltyQBmap[QB32_QB63] |= (_pQBEnBitmap[vfId].dw[QB32_QB63] & lastRoundQBBitmapMask);
            gPenaltyQBmap[QB0_QB31] |= _pQBEnBitmap[vfId].dw[QB0_QB31];
        }
        else
        {
            if (_pQB65EnBitmap[vfId] && gLastQBIndex[vfId] == 64)
            {
                gPenaltyQBmap[QB32_QB63] |= _pQBEnBitmap[vfId].dw[QB32_QB63];
                gPenaltyQBmap[QB0_QB31] |= _pQBEnBitmap[vfId].dw[QB0_QB31];
            }
        }

        pVfInfo->qosPenaltyPeriod = pVfInfo->qosPenaltyPeriod - 1;

    }

}
#endif

ATTR_ALWAYS_INLINE void fpsCpu1::QBHandler(uint8_t qbSelect, uint32_t dflBaseAddr)
{
    CdmaSq_t* pCdmaSq = &_cdmaSq;
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    volatile CdmaSqCmdDescr_t* pCdmaSqe;
    QueueBlockInfo_t* pQBlockInfo;
    CmdEntry_t* pCe;
    CmdEntryTiny_t* pCet;
    LionNvmeSQDescriptor_t* pHostCmd;
    #ifndef NEW_AES_KEY_VALIDATION_SUPPORT
    AesXtsCmd_t* pAesXtsCmd;
    #else
    KeyFlags_t* pKeyFlags1;
    KeyFlags_t* pKeyFlags2;
    #endif
    uint8_t caIndex = 0;
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint8_t qbIndex, vfIndex, cmdState, disBit;
    #else
    uint8_t qbIndex, vfIndex, cmdState, rgId1, rgId2, key1SubIdx, key2SubIdx, disBit;
    #endif
    uint16_t ceIndex;
    uint16_t currentChunkSize = CHUNK_SIZE;
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint32_t chunkByteCnt, dataLength, cdmaDw1;
    #else
    uint32_t chunkByteCnt, dataLength, hostKey1, hostKey2, cdmaDw1;
    #endif
    uint32_t* pCmdArrayHwCi = NULL;

    while (gQBCmdExistSnapBitMap[qbSelect])
    {
        qbIndex = FindNextBit32(gQBCmdExistSnapBitMap[qbSelect]) + (qbSelect << VF_QB_32_SHIFT);
        disBit = (qbIndex & QUEUE_BLOCK_OFFSET_MASK);
        gQBCmdExistSnapBitMap[qbSelect] &= ~(BIT(disBit));

        pQBlockInfo = &_pQBlockInfoBase[qbIndex];
        vfIndex = pQBlockInfo->vfId;
        currentChunkSize = CHUNK_SIZE;

        /* Shall keep this conditional statement, be careful compiler optimization.
         * Have to garantee it should not be negative number.  (_pVfRemainCredit[vfIndex] -= CREDIT_SIZE)
         */
        if (_pVfRemainCredit[vfIndex] > 0)
        {
            #ifdef QUEUE_BLOCK_BALANCE
            gQBCredit = CREDIT_SIZE;

            while (gQBCredit)
            #endif
            {
                /* Process NEW command */
                if (pQBlockInfo->remainLen == 0)
                {
                    #ifdef CDMA_CMD_COUNT
                    if (unlikely(((cdmaCmdSlotQueuePi - cdmaCmdSlotQueueCi) & FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK) >= \
                                 MAX_IO_CMD_SLOT_COUNT))
                    {
                        cdmaCmdSlotQueueCi = readl(pCdmaCmdSlotQueueCi);
                        if (((cdmaCmdSlotQueuePi - cdmaCmdSlotQueueCi) & FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK) >= \
                            MAX_IO_CMD_SLOT_COUNT)
                        {
                            ExitQueueManagerFiber();
                            return;
                        }
                    }
                    #endif
                    if (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
                    {
                        cdmaSqCi = readl(pCdmaSq->pHwCi);
                        while (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
                        {
                            if (readl(_cdmaFatalErrorFlag))
                            {
                                ExitQueueManagerFiber();
                                return;
                            }
                            cdmaSqCi = readl(pCdmaSq->pHwCi);
                        }
                    }

                    #ifndef WEIGHT_ROUND_ROBIN
                    if (gSq0HighSnapBitMap[qbSelect] & BIT(disBit))
                    {
                        caIndex = GET_HIGH_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                    }
                    else if (gSq1LowSnapBitMap[qbSelect] & BIT(disBit))
                    {
                        caIndex = GET_LOW_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                    }
                    else
                    {
                        // should not hit this case
                    }
                    #else
                    if ((gHighQueueHitCnt[qbIndex]++ & gWeightRoundRobin) == 0)
                    {
                        if (gSq0HighSnapBitMap[qbSelect] & BIT(disBit))
                        {
                            caIndex = GET_HIGH_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                        }
                        else if (gSq1LowSnapBitMap[qbSelect] & BIT(disBit))
                        {
                            caIndex = GET_LOW_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                        }
                        else
                        {
                            // should not hit this case
                        }
                    }
                    else
                    {
                        gHighQueueHitCnt[qbIndex] = 0;
                        if (gSq1LowSnapBitMap[qbSelect] & BIT(disBit))
                        {
                            caIndex = GET_LOW_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                        }
                        else if (gSq0HighSnapBitMap[qbSelect] & BIT(disBit))
                        {
                            caIndex = GET_HIGH_PRIORITY_CA_INDEX(qbSelect, qbIndex);
                        }
                        else
                        {
                            // should not hit this case
                        }
                    }
                    #endif

                    cmdState = NEW_CMD;

                    pCdmaSqe = pCdmaSqBase + cdmaSqPi;
                    ceIndex = (caIndex << CA_SIZE_SHIFT) + (cmdArrayCi[caIndex] & CA_MASK);
                    pCe = _pCmdArrayBase + ceIndex;
                    pCet = _pCmdArrayTinyBase + ceIndex;
                    pHostCmd = (LionNvmeSQDescriptor_t*)((pCe->DFLIdx << DFL_BUF_SZ_SHIFT) + dflBaseAddr);
                    dataLength = pHostCmd->SrcDataLen;

                    #ifndef NEW_AES_KEY_VALIDATION_SUPPORT
                    pAesXtsCmd = (AesXtsCmd_t*)(pHostCmd + 1);
                    #endif

                    if (unlikely(dataLength & HOST_CMD_16B_ALIGN_MASK))
                    {
                        if (pCe->Status < cCEStsInvalidXTSField)
                        {
                            if (pHostCmd->cipher)
                            {
                                pCe->Status = cCEStsInvalidXTSField;
                                pCet->HostErrCode = cCETinyHostErrSqeXtsLengthUnaligned16B;
                            }
                            else
                            {
                                pCe->Status = cCEStsInvalidGCMField;
                                pCet->HostErrCode = cCETinyHostErrSqeGcmLengthUnaligned16B;
                            }
                        }
                    }

                    /* key verification, NOTE: for better performance, do not write the algorithm to an inline function */
                    #ifdef MODE_NORMAL
                    if (pHostCmd->cipher) // XTS_CMD
                    {
                        #ifndef NEW_AES_KEY_VALIDATION_SUPPORT
                        hostKey1 = pAesXtsCmd->HostKey1Idx;
                        hostKey2 = pAesXtsCmd->HostKey2Idx;

                        rgId1 = hostKey1 >> HOST_SQE_KEY_RGID_SHIFT;
                        rgId2 = hostKey2 >> HOST_SQE_KEY_RGID_SHIFT;

                        key1SubIdx = hostKey1 & HOST_SQE_KEY_SUB_INDEX_MASK;
                        key2SubIdx = hostKey2 & HOST_SQE_KEY_SUB_INDEX_MASK;

                        hostKey1 = rgId1 * 7 + key1SubIdx;
                        hostKey2 = rgId2 * 7 + key2SubIdx;

                        if ((hostKey1 != hostKey2) && \
                            ((_key2OwnerVfid[hostKey1] == vfIndex) && (_key2OwnerVfid[hostKey2] == vfIndex)))
                        {
                            pCdmaSqe->dw2 = ((hostKey2 << CDMA_SQE_DW2_KEY2_INDEX_SHIFT) | hostKey1);
                        }
                        else
                        {
                            if (pCe->Status < cCEStsInvalidXTSField)
                            {
                                pCe->Status = cCEStsInvalidXTSField;
                                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X hostKey1:0x%X hostKey2:0x%X\n", ((hostKey2 & 0x3FF << 0x12UL) | (hostKey1 & 0x3FF << 0x8UL) | (vfIndex & 0xFF))), "8,10,14");
                            }
                        }
                        #else         
                        AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(pHostCmd + 1);
                        // Calculate the key indexes for two XTS Keys.
                        uint16_t keyIndex1 = (pAesXtsCmd->HostKeyIdx[0].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[0].keySubIndex;
                        uint16_t keyIndex2 = (pAesXtsCmd->HostKeyIdx[1].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[1].keySubIndex;
                        uint32_t sessionID = pHostCmd->sessionID;
                        uint8_t appID = pHostCmd->appID;

                        AesKeyVault_t* keyVaultArr;
                        keyVaultArr = (AesKeyVault_t*)(&(rCdma->aesKeyVaultAddr));

                        pKeyFlags1 = (KeyFlags_t*)&_pKeyIsEphemeral[keyIndex1];
                        pKeyFlags2 = (KeyFlags_t*)&_pKeyIsEphemeral[keyIndex2];
                        if (pKeyFlags1->keyType != cAesXts)
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey1Type;
                        }
                        else if (pKeyFlags2->keyType != cAesXts)
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey2Type;
                        }
                        else if (keyIndex1 >= KEY_INDEX_MAX)
                        {
                            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, hostKey1:0x%X\n", keyIndex1), "32");
                            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, RGID1:0x%X\n", pAesXtsCmd->HostKeyIdx[0].resourceGroupID), "32");
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey1;

                        }
                        else if (keyIndex2 >= KEY_INDEX_MAX)
                        {
                            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, hostKey2:0x%X\n", keyIndex2), "32");
                            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, RGID2:0x%X\n", pAesXtsCmd->HostKeyIdx[1].resourceGroupID), "32");
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey2;
                        }
                        else if (keyIndex1 == keyIndex2) // Same keys passed for XTS_CMD.
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsSameKeysPassed;
                        }
                        else if (_key2OwnerVfid[keyIndex1] != vfIndex) // VFIndex does not match the VFID that was saved at the key offset for Key1.
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidVfID;
                        }
                        else if (_key2OwnerVfid[keyIndex2] != vfIndex) // VFIndex does not match the VFID that was saved at the key offset for Key2.
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidVfID;
                        }
                        else if (pKeyFlags1->session_only != pKeyFlags2->session_only)
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsDiffEphemeralFlag;
                        }
                        else if (false == ValidateKeyVaultArrKey(keyVaultArr, keyIndex1, keyIndex2)) //Different Key Indexes might have the same key
                        {
                            pCe->Status = cCEStsInvalidXTSField;
                            pCet->HostErrCode = cCETinyHostErrSqeXtsSameKeysDiffIndex;
                        }
                        else
                        {
                            // Validate App ID as it is applicable for both ephemeral and persistent
                            if (_pKey2AppID[keyIndex1] != appID)
                            {
                                pCe->Status = cCEStsInvalidXTSField;
                                pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey1AppID;
                            }
                            else if (_pKey2AppID[keyIndex2] != appID)
                            {
                                pCe->Status = cCEStsInvalidXTSField;
                                pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey2AppID;
                            }
                            else
                            {
                                // Validate Ephemeral Keys
                                if (pKeyFlags1->session_only)
                                {
                                    // Validate the session ID as it is applicable only for ephemeral
                                    if (_pKey2SessionID[keyIndex1] != sessionID)
                                    {
                                        pCe->Status = cCEStsInvalidXTSField;
                                        pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey1SessionID;
                                    }
                                    else if (_pKey2SessionID[keyIndex2] != sessionID)
                                    {
                                        pCe->Status = cCEStsInvalidXTSField;
                                        pCet->HostErrCode = cCETinyHostErrSqeXtsInvalidKey2SessionID;
                                    }
                                    else
                                    {
                                        // Valid Session Key
                                        pCdmaSqe->Dw2.LocalKey1Idx = keyIndex1;
                                        pCdmaSqe->Dw2.LocalKey2Idx = keyIndex2;
                                    }
                                }
                                else
                                {
                                    // Valid App Key
                                    pCdmaSqe->Dw2.LocalKey1Idx = keyIndex1;
                                    pCdmaSqe->Dw2.LocalKey2Idx = keyIndex2;
                                }
                            }
                        }
                        //Check if XTS data length exceeds maximum size
                        if (unlikely(dataLength > MAX_AES_XTS_DATA_LEN))
                        {
                            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: AES XTS data length exceeds MAX length:0x%X\n", 1), "32");
                            if (pCe->Status < cCEStsInvalidXTSField)
                            {
                                pCe->Status = cCEStsInvalidXTSField;
                                pCet->HostErrCode = cCETinyHostErrSqeXtsDataLengthExceeded;
                            }
                        }
                        #endif
                        cdmaDw1 = (pCe->PhyIbqId << CDMA_SQE_DW1_IBQ_SHIFT) |                          \
                                  (pHostCmd->dataUnitLen << CDMA_SQE_DW1_XTS_DATA_UINT_LENGTH_SHIFT) | \
                                  (pHostCmd->prpSgl << CDMA_SQE_DW1_DATA_DESC_TYPE_SHIFT) +            \
                                  (XTS_CMD + (pHostCmd->EnDecrypt));
                    }
                    else // GCM_CMD
                    {
                        #ifndef NEW_AES_KEY_VALIDATION_SUPPORT
                        hostKey1 = pAesXtsCmd->HostKey1Idx;

                        rgId1 = hostKey1 >> HOST_SQE_KEY_RGID_SHIFT;
                        key1SubIdx = hostKey1 & HOST_SQE_KEY_SUB_INDEX_MASK;
                        hostKey1 = rgId1 * 7 + key1SubIdx;

                        if (likely(_key2OwnerVfid[hostKey1] == vfIndex))
                        {
                            pCdmaSqe->dw2 = hostKey1;
                        }
                        else
                        {
                            if (pCe->Status < cCEStsInvalidGCMField)
                            {
                                pCe->Status = cCEStsInvalidGCMField;
                                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X hostKey1:0x%X ownerVfId:0x%X\n", (((_key2OwnerVfid[hostKey1] & 0x7) << 0xBUL) | (((hostKey1 >> HOST_SQE_KEY_RGID_SHIFT) & 0x7) << 0x8UL) | (vfIndex & 0xFF))), "8,3,21");
                            }
                        }
                        #else
                        AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(pHostCmd + 1);
                        uint16_t keyIndex1 = (pAesXtsCmd->HostKeyIdx[0].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[0].keySubIndex;

                        AesGcmCmd_t* pAesGcmCmd = (AesGcmCmd_t*)(pHostCmd + 1);
                        pKeyFlags1 = (KeyFlags_t*)&_pKeyIsEphemeral[keyIndex1];

                        //Uncomment following logs for Tag correction debug
                        //Data from Host
                        // DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Data from Host: ceIndex: %x, EnDecrypt:%x", ceIndex,pHostCmd->EnDecrypt), "32","32");
                        // DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Data from host: UnalignedSrcDataLen: %x", pAesGcmCmd->UnalignedSrcDataLen), "32");
                        // DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Data from host: UnalignedDstDataLen: %x", pAesGcmCmd->UnalignedDstDataLen), "32");
                        // DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Data from host: UnpaddedAADLen: %x", pAesGcmCmd->UnpaddedAADLen), "32");

                        if(pKeyFlags1->keyType == cAesGcmApproved) 
                        {
                            if(pHostCmd->EnDecrypt == 0)
                            {
                                // // Get IV value from queue and set it
                                uint32_t ivStatus;
                                IVEntry_t entry = API_GetIVEntry(&ivStatus);

                                if(ivStatus == 0)
                                {
                                    M7_MEM_COPY(pAesGcmCmd->IV, entry.IV, sizeof(entry.IV));
                                }
                                else 
                                {
                                    // DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("Skipping current command since no IV available %x", ceIndex), "32" );
                                    // Reset total credits to 0 to exit this loop and terminate the current fiber,
                                    // since no IVs are available to process the command.
                                    // This allows other fibers (e.g., liveliness counter updates) to run.
                                    // The current command will be retried when QBHandler is invoked again.
                                    gTotalCredit = 0;
                                    return;
                                }
                            }
                        }
                        else if(pKeyFlags1->keyType != cAesGcmApproved && pKeyFlags1->keyType != cAesGcmUnapproved)
                        {
                            // DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("GCM: pKeyFlags1 Key Type :0x%X\n", pKeyFlags1->keyType), "32");
                            pCe->Status = cCEStsInvalidGCMField;
                            pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidKeyType;
                        }
                        if(pAesGcmCmd->AADLen % AAD_ALIGNMENT_CHECK)// Validate AAD length
                        {
                            // DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid AAD length :0x%X \n", (uint32_t)pAesGcmCmd->AADLen), "32");
                            pCe->Status = cCEStsInvalidGCMField;
                            pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidAadLength;
                        }
                        else if(keyIndex1 >= KEY_INDEX_MAX)
                        {
                            // Calculate the key index for the GCM Key.
                            // If key index are beyond range then log error.
                            // DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X KeyIndex:0x%X \n", ((keyIndex1 << 0x10UL) | vfIndex)), "16,16");
                            pCe->Status = cCEStsInvalidGCMField;
                            pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidKey;
                        }
                        else if((_key2OwnerVfid[keyIndex1] != vfIndex)) // VFIndex does not match the VFID that was saved at the key offset for Key1.
                        {
                            pCe->Status = cCEStsInvalidGCMField;
                            pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidVfID;
                        }
                        else if(likely(_key2OwnerVfid[keyIndex1] == vfIndex))
                        {
                            // Validate App ID as it is applicable for both ephermal and persistent
                            if (_pKey2AppID[keyIndex1] != pHostCmd->appID)
                            {
                                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X KeyIndex:0x%X \n", ((keyIndex1 << 0x10UL) | (vfIndex))), "16,16");
                                // DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, AppID:0x%X _pKey2AppID[keyIndex1]:0x%X\n", ((  _pKey2AppID[keyIndex1] << 10UL) | pHostCmd->appID)), "16,16");
                                pCe->Status = cCEStsInvalidGCMField;
                                pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidKeyAppID;
                            }
                            else
                            {
                                // Valid appID
                                if (pKeyFlags1->session_only)
                                {
                                    if (_pKey2SessionID[keyIndex1] != pHostCmd->sessionID)
                                    {
                                        pCe->Status = cCEStsInvalidGCMField;
                                        pCet->HostErrCode = cCETinyHostErrSqeGcmInvalidKeySessionID;
                                    }
                                    else
                                    {
                                        pCdmaSqe->dw2 = keyIndex1;
                                    }
                                }
                                else
                                {
                                    pCdmaSqe->dw2 = keyIndex1;
                                }
                            }
                        }
                        else
                        {
                            if (pCe->Status < cCEStsInvalidGCMField)
                            {
                                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, KeyIndex:0x%X \n", keyIndex1), "32");
                                // DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X _key2OwnerVfid[keyIndex1]:0x%X \n", ((_key2OwnerVfid[keyIndex1] << 0x10UL) | (vfIndex))), "16,16");
                                pCe->Status = cCEStsInvalidGCMField;
                                pCet->HostErrCode = cCETinyHostErrSqeGcmUndefinedErr;
                            }
                        }
                        #endif

                    cdmaDw1 = (pCe->PhyIbqId << CDMA_SQE_DW1_IBQ_SHIFT) |                          \
                                (pHostCmd->dataUnitLen << CDMA_SQE_DW1_XTS_DATA_UINT_LENGTH_SHIFT) | \
                                (pHostCmd->prpSgl << CDMA_SQE_DW1_DATA_DESC_TYPE_SHIFT) +            \
                                (GCM_CMD + (pHostCmd->EnDecrypt));
                    }
                    #endif

                    pCdmaSqe->dw3 = (pCe->IFSel) | (pCe->IFSel << CDMA_SQE_DW3_SDATA_ISEL_SHIFT) | \
                                    (pCe->IFSel << CDMA_SQE_DW3_DDESC_ISEL_SHIFT) |                \
                                    (pCe->IFSel << CDMA_SQE_DW3_DDATA_ISEL_SHIFT);

                    if (unlikely((pCe->Status == cCEStsPoorSGLRetry)))
                    {
                        currentChunkSize = CHUNK_SIZE_WITH_RETRY(pHostCmd->PoorSGLRetryTimes);
                    }

                    pHostCmd->chunkSize = currentChunkSize; // for poor SGL error handling
                    pHostCmd->PASID = pHostCmd->DW1;        // host CQE alignement

                    #ifdef CDMA_CMD_COUNT
                    cdmaCmdSlotQueuePi = QUEUE_INC(cdmaCmdSlotQueuePi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                    writel(cdmaCmdSlotQueuePi, pCdmaCmdSlotQueuePi);
                    #endif
                }
                else /* Process CONTINUE command */
                {
                    if (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
                    {
                        cdmaSqCi = readl(pCdmaSq->pHwCi);
                        while (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
                        {
                            if (readl(_cdmaFatalErrorFlag))
                            {
                                ExitQueueManagerFiber();
                                return;
                            }
                            cdmaSqCi = readl(pCdmaSq->pHwCi);
                        }
                    }

                    cmdState = CONTINUE_CMD;

                    pCdmaSqe = pCdmaSqBase + cdmaSqPi;
                    ceIndex = pQBlockInfo->remainCeIdx;
                    pCe = _pCmdArrayBase + ceIndex;
                    pHostCmd = (LionNvmeSQDescriptor_t*)((pCe->DFLIdx << DFL_BUF_SZ_SHIFT) + dflBaseAddr);
                    caIndex = ceIndex >> CA_SIZE_SHIFT;
                    dataLength = pQBlockInfo->remainLen;
                    cdmaDw1 = 0;

                    if (unlikely((pCe->Status == cCEStsPoorSGLRetry)))
                    {
                        currentChunkSize = CHUNK_SIZE_WITH_RETRY(pHostCmd->PoorSGLRetryTimes);
                    }
                }

                /* chunking host command */
                #ifndef MODE_DBYPASS
                if (currentChunkSize >= dataLength)
                {
                    cmdState |= LAST_CMD;
                    chunkByteCnt = dataLength;
                    pQBlockInfo->remainLen = 0;
                }
                else
                {
                    /* If the command should be chunk, gQBCredit could not be used before. */
                    #ifdef QUEUE_BLOCK_BALANCE
                    if (gQBCredit >= currentChunkSize)
                    #endif
                    {
                        chunkByteCnt = currentChunkSize;
                        pQBlockInfo->remainLen = dataLength - chunkByteCnt;
                        pQBlockInfo->remainCeIdx = ceIndex; // keep ceIndex for continue command in next round
                    }
                    #ifdef QUEUE_BLOCK_BALANCE
                    else
                    {
                        gQBCredit = 0;
                    }
                    #endif
                }
                #endif

                #if defined (MODE_DBYPASS)
                pCdmaSqe->dw1 = (pCe->PhyIbqId << CDMA_SQE_DW1_IBQ_SHIFT) |                          \
                                (pHostCmd->dataUnitLen << CDMA_SQE_DW1_XTS_DATA_UINT_LENGTH_SHIFT) | \
                                (pHostCmd->prpSgl << CDMA_SQE_DW1_DATA_DESC_TYPE_SHIFT);
                pHostCmd->DstDataLen = 0;
                cmdState = NEW_LAST_CMD;
                #elif defined (MODE_CBYPASS)
                pCdmaSqe->dw1 = (pCe->PhyIbqId << CDMA_SQE_DW1_IBQ_SHIFT) |                          \
                                (chunkByteCnt << CDMA_SQE_DW1_CHUNK_BCNT_SHIFT) |                    \
                                (pHostCmd->dataUnitLen << CDMA_SQE_DW1_XTS_DATA_UINT_LENGTH_SHIFT) | \
                                (pHostCmd->prpSgl << CDMA_SQE_DW1_DATA_DESC_TYPE_SHIFT);
                #else  // MODE_NORMAL
                pCdmaSqe->dw1 = (cdmaDw1 | chunkByteCnt << CDMA_SQE_DW1_CHUNK_BCNT_SHIFT);
                #endif

                pCdmaSqe->dw0 = (pCe->Dw0 & 0xfff00000) | \
                                (cmdState << CDMA_SQE_DW0_CMD_STATE_SHIFT) | ceIndex;

                #ifdef QUEUE_BLOCK_BALANCE
                gQBCredit -= chunkByteCnt;
                #endif

                #ifdef QOS_LATENCY_TEST
                if (x && (cmdState & NEW_CMD))
                {
                    x--;
                    if (!x)
                    {
                        // Send Reserved OpCode to test QOS Feature.
                        pCdmaSqe->dw0 |= (CDMA_OPCODE_RESERVED << CDMA_SQE_DW0_OPCODE_SHIFT);
                        if (y > 1)
                        {
                            x++;
                            y--;
                        }
                    }
                }
                #endif

                #ifdef SUPPORT_ERROR_INJECTION
                if (cmdState & NEW_CMD)
                {
                    Cpu1HandleErrorInject(pAesXtsCmd, pCdmaSqe, pHostCmd);
                }
                #elif defined (SUPPORT_MSGERROR_INJECTION)
                if (errInjectFlag && (cmdState & NEW_CMD))
                {
                    uint8_t errIdx;
                    uint8_t ibPhyQId = pThis->pCa2IbPhysicalId(caIndex);
                    uint64_t errInjectBitmap = readq(pErrInjectBitmap);
                    if (errInjectBitmap)
                    {
                        errIdx = Cpu1ScanErrInjectTable(errInjectBitmap, pMsgErrorInjection, pHostCmd->HostCid, vfNum, ibPhyQId, pCe->DFLIdx);
                        if (errIdx != INVALID_REFERENCE_WITH_BYTE)
                        {
                            Cpu1InsertErr(&pMsgErrorInjection[errIdx], pCdmaSqe, pHostCmd);
                        } // else do nothing
                    }
                    else
                    {
                        errInjectFlag = 0;
                    }
                }
                #endif

                // error handle. zero transfer length, and use up all credit on this QB
                if (unlikely((pCe->Status >= cCEStsInvalidXTSField)))
                {
                    pCdmaSqe->Dw1.ChunkByteCnt = 0;
                    pQBlockInfo->remainLen = 0;
                    #ifdef QUEUE_BLOCK_BALANCE
                    gQBCredit = 0;
                    #endif

                    if (cmdState & NEW_CMD)
                    {
                        pCet->ErrStatus = cCETinyStsZeroXfer; // for CPU2 to classify zero transfer
                        cmdState = NEW_LAST_CMD;
                        pCdmaSqe->Dw0.CmdState = NEW_LAST_CMD;
                        pHostCmd->SrcDataLen = 0;
                        pHostCmd->DstDataLen = 0;
                        if(!(pHostCmd->cipher)) // GCM command
                        {
                            //If error detected. Don't send to CP for tag correction.
                            AesGcmCmd_t* pAesGcmCmd = (AesGcmCmd_t*)(pHostCmd + 1);
                            pAesGcmCmd->UnalignedSrcDataLen = 0;
                            pAesGcmCmd->UnalignedDstDataLen = 0;
                            pAesGcmCmd->UnpaddedAADLen = 0;
                        }
                    }
                    else if (cmdState & CONTINUE_CMD)
                    {
                        cmdState = CONTINUE_LAST_CMD;
                        pCdmaSqe->Dw0.CmdState = CONTINUE_LAST_CMD;
                        pCdmaSqe->Dw0.CmdOpcode = CDMA_OPCODE_ABORT;
                    }
                }

                cdmaSqPi = QUEUE_INC(cdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);

                DMB();

                writel(cdmaSqPi, pCdmaSq->pHwPi);
                #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
                writel(cdmaSqPi, cdmaSqPiHwAddr);
                #endif

                #ifndef DISABLE_IO_LOG
                //DebugLogLvDbgInfoInline(cLogCPU1Common, cLogDebug, ("[IO LOG] CDMA SQE dw0:  ce id:0x%03X, cmd sts:0x%X, op:0x%X, cdma sq dw1[3:0]:0x%X:, dfl idx:0x%03X, list num:0x%X\n", \
                                                               (pCdmaSqe->dw0 & 0xFFF0FFFFUL) | ((pCdmaSqe->dw1 & 0xFUL) << 0x10UL)), "10,3,3,4,10,2");
                #endif

                if (cmdState & LAST_CMD)
                {
                    pCmdArrayHwCi = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (caIndex << CMD_ARRAY_SHIFT));
                    cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);
                    writel(cmdArrayCi[caIndex], pCmdArrayHwCi);

                    /* If did not used up qBcredit on 1st cmd, it may handle 2nd cmd directly.
                       Therefore, should update cmd status to indicate command completed.
                       Otherwise, will send the same command on next round
                     */
                    #ifdef QUEUE_BLOCK_BALANCE
                    if (gQBCredit)
                    {
                        if (qbSelect == QB64)
                        {
                            uint32_t regSts = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_4));
                            gSq0HighSnapBitMap[QB64] = regSts & 0x1;
                            gSq1LowSnapBitMap[QB64] = (regSts >> 1) & 0x1;
                        }
                        else
                        {
                            gSq0HighSnapBitMap[qbSelect] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_0 + (qbSelect << 2)));
                            gSq1LowSnapBitMap[qbSelect] = ~(readl(REG_FPS_SLOT_ARRAY_EMPTY_STATUS_2 + (qbSelect << 2)));
                        }
                    }
                    #endif
                }
            }

            _pVfRemainCredit[vfIndex] -= CREDIT_SIZE;
            gTotalCredit -= CREDIT_SIZE;

            #ifdef QOS_LATENCY_ERROR_HANDLING
            gLastQBIndex[vfIndex] = qbIndex;    // save the QB index everytime
            #endif

            if (_pVfRemainCredit[vfIndex] == 0)
            {
                gBanQBmap[QB0_QB31] |= _pQBEnBitmap[vfIndex].dw[QB0_QB31];
                gBanQBmap[QB32_QB63] |= _pQBEnBitmap[vfIndex].dw[QB32_QB63];
                gBanQBmap[QB64] |= _pQB65EnBitmap[vfIndex];
            }
        }
        else
        {
            gBanQBmap[QB0_QB31] |= _pQBEnBitmap[vfIndex].dw[QB0_QB31];
            gBanQBmap[QB32_QB63] |= _pQBEnBitmap[vfIndex].dw[QB32_QB63];
            gBanQBmap[QB64] |= _pQB65EnBitmap[vfIndex];
        }
    }
}

ATTR_ALWAYS_INLINE void fpsCpu1::FpsCpu1FreeCDMAErrorSlot(void)
{
    uint32_t abortQueuePi = readl(pCdmaSlotAbortQueuePi);
    uint32_t cdmaSlotAbortQueueCi = readl(pCdmaSlotAbortQueueCi);

    while (abortQueuePi != cdmaSlotAbortQueueCi)
    {
        if (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
        {
            cdmaSqCi = readl(_cdmaSq.pHwCi);
            while (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
            {
                if (readl(_cdmaFatalErrorFlag))
                {
                    return;
                }

                cdmaSqCi = readl(_cdmaSq.pHwCi);
            }
        }

        uint16_t abortCeIndex = pCdmaSlotAbortQueue[cdmaSlotAbortQueueCi];
        uint8_t caIdx = (abortCeIndex >> CA_SIZE_SHIFT);

        if (readl(_cdmaFatalErrorFlag))
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Fatal error occured CP_CDMA_IO Command 0x%x\n", abortCeIndex), "32");
            break;
        }

        // CPIO occurs error, abort this command and clear the setting
        if (abortCeIndex == CP_CDMA_IO_CMD_ID)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Aborting CP_CDMA_IO Command 0x%x\n", abortCeIndex), "32");
            hasCPCDMACmd = 0;
        }
        // else if regular IO command then dont abort here it will be aborted as part of _SkipAndAbortWithSQid call
        else if (readb(&_CPU1SubmitAbortInfo[caIdx]) == (abortCeIndex - (caIdx << CA_SIZE_SHIFT)))
        {
            cdmaSlotAbortQueueCi = QUEUE_INC(cdmaSlotAbortQueueCi, 0x7f);
            writel(cdmaSlotAbortQueueCi, pCdmaSlotAbortQueueCi);
            continue;
        }

        volatile CdmaSqCmdDescr_t* pCdmaSqe = &_cdmaSq.pCdmaSqBase[cdmaSqPi];

        pCdmaSqe->dw0 = (uint32_t)abortCeIndex | (CONTINUE_LAST_CMD << CDMA_SQE_DW0_CMD_STATE_SHIFT) | \
                        ((uint32_t)CDMA_OPCODE_ABORT << CDMA_SQE_DW0_OPCODE_SHIFT);
        pCdmaSqe->dw1 = 0;
        pCdmaSqe->dw2 = 0;
        pCdmaSqe->dw3 = 0;

        cdmaSqPi =  QUEUE_INC(cdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);

        DMB();

        writel(cdmaSqPi, _cdmaSq.pHwPi);
        #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
        writel(cdmaSqPi, _cdmaSq.PiHwAddr);
        #endif

        cdmaSlotAbortQueueCi = QUEUE_INC(cdmaSlotAbortQueueCi, 0x7f);
        writel(cdmaSlotAbortQueueCi, pCdmaSlotAbortQueueCi);

        if (abortCeIndex != CP_CDMA_IO_CMD_ID)
        {
            _UpdateCmdArrayCiAfterAbort(abortCeIndex);
        }
    }
}

void fpsCpu1::FpsCpu1QueueManagerFiber(void* pObj)
{
    fpsCpu1* pThis = static_cast<fpsCpu1*>(pObj);

    pThis->FpsCpu1FreeCDMAErrorSlot();

    #ifdef QOS_LATENCY_ERROR_HANDLING
    bool firstLoop = TRUE;
    #endif

    gBanQBmap[QB0_QB31] = 0;
    gBanQBmap[QB32_QB63] = 0;
    gBanQBmap[QB64] = 0;

    #ifdef QOS_LATENCY_ERROR_HANDLING
    gQosLatencyErrVFBitmap[VF0_VF31] = *pThis->_pQosVFBitmap[VF0_VF31];
    gQosLatencyErrVFBitmap[VF32_VF63] = *pThis->_pQosVFBitmap[VF32_VF63];

    gPenaltyQBmap[QB0_QB31] = 0;
    gPenaltyQBmap[QB32_QB63] = 0;
    gPenaltyQBmap[QB64] = 0;
    gQosLatencyErrVFBitmap[VF64] = *pThis->_pQosVFBitmap[VF64];

    pThis->QosLatencyPenaltyHandling(VF0_VF31);
    pThis->QosLatencyPenaltyHandling(VF32_VF63);
    pThis->QosLatencyPenaltyHandling(VF64);
    #endif

    gTotalCredit = pThis->_pTotalCredit[0];
    M7_MEM_COPY((void*)pThis->_pVfRemainCredit, (const void*)pThis->_pVfCredit, 0x104);
    #ifdef WEIGHT_ROUND_ROBIN
    gWeightRoundRobin = pThis->_pWeightRoundRobin[0];
    #endif

    while (gTotalCredit)
    {
        SnapCommandExistStatus();

        #ifdef QOS_LATENCY_ERROR_HANDLING
        if (firstLoop)
        {
            gQBCmdExistSnapBitMap[QB0_QB31] &= ~(gPenaltyQBmap[QB0_QB31]);
            gQBCmdExistSnapBitMap[QB32_QB63] &= ~(gPenaltyQBmap[QB32_QB63]);
            gQBCmdExistSnapBitMap[QB64] &= ~(gPenaltyQBmap[QB64]);

            firstLoop = FALSE;
        }
        else
        #endif
        {
            gQBCmdExistSnapBitMap[QB0_QB31] &= ~(gBanQBmap[QB0_QB31]);
            gQBCmdExistSnapBitMap[QB32_QB63] &= ~(gBanQBmap[QB32_QB63]);
            gQBCmdExistSnapBitMap[QB64] &= ~(gBanQBmap[QB64]);
        }

        /* After service all VF and QB once, if there are remaining credits, snap commands and service next round */
        if ((gQBCmdExistSnapBitMap[QB0_QB31] | gQBCmdExistSnapBitMap[QB32_QB63] | gQBCmdExistSnapBitMap[QB64]) == NO_EXIST_CMD)
        {
            break;
        }

        pThis->QBHandler(QB0_QB31, (uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR);
        pThis->QBHandler(QB32_QB63, (uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR);
        pThis->QBHandler(QB64, (uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR);
    }

    if (*pThis->_fpMode == FP_MODE_STRICT && gTotalCredit)
    {
        pThis->SendIdleCmd(gTotalCredit);
    }

    #ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
    uint32_t currentTimestamp = 0xFFFFFF - readl(&pThis->rCortexm7->systemControl.systCvr);
    currentTimestamp = (currentTimestamp + pThis->gTimerCounterCovert) & SYSTICK_MASK;
    gTimeStampBase = pThis->gTimerCounterCovert;

    if ((pThis->localTimeStamp & TimestampMaskBit23_14) != (currentTimestamp & TimestampMaskBit23_14)) // compare 10 bits [23:14]
    {
        // update local timestamp and record in log buffer
        pThis->localTimeStamp = currentTimestamp;

        // Note: //DebugLogLvDbgInfoInline's param logCategory cannot use variable, due to tokenize tool check the enum "text" but "value".
        //DebugLogLvDbgInfoInline(cLogCPU1Common, cLogInfo, ("logging timestamp: 0x%x\n", pThis->localTimeStamp));
    }
    #endif

}

void fpsCpu1::FpsCpu1ReceiveFPMsgFiber(void* pObj)
{
    fpsCpu1* pThis = static_cast<fpsCpu1*>(pObj);

    uint32_t msgCPU2toCPU1Pi, msgCPU2toCPU1Ci;
    bool modifyCpu2ToCpu1 = false;
    CP2FPMsgSts sts = msgSuccess;

    msgCPU2toCPU1Pi = readl(pThis->pCPU2toCPU1Pi);
    msgCPU2toCPU1Ci = readl(pThis->pCPU2toCPU1Ci);
    while (msgCPU2toCPU1Pi != msgCPU2toCPU1Ci) //msg from CPU2
    {
        sts = pThis->RecvFPMsg(&pThis->pCPU2toCPU1MsgQ[msgCPU2toCPU1Ci], cM7Core2);
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP1 msgCPU2toCPU1Pi[0x%x], msgCPU2toCPU1Ci [0x%x]\n", ( msgCPU2toCPU1Pi| ( msgCPU2toCPU1Ci<< 0x10UL))), "16,16");
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("FP1 pCPU2toCPU1MsgQ[msgCPU2toCPU1Ci].sts [0x%x]\n", pThis->pCPU2toCPU1MsgQ[msgCPU2toCPU1Ci].sts), "32");
        //if (FpsCpu1ChkFPMsgStsDone(sts))
        if( sts != msgNoEmptyEntry) // Send response passed.
        {
            msgCPU2toCPU1Ci = M7_QUEUE_INC(msgCPU2toCPU1Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            modifyCpu2ToCpu1 = true;
        }
        else
        {
            #ifdef DEBUG_BUILD
            writel(0xB, pThis->pCpuStatus);
            Explicit_CrashCatcher_Entry();
            #endif
            break;
        }
    }

    if (modifyCpu2ToCpu1)
    {
        pThis->CPU2toCPU1Ci = msgCPU2toCPU1Ci;
        writel(pThis->CPU2toCPU1Ci, pThis->pCPU2toCPU1Ci);
    }

    bool modifyCpu0ToCpu1 = false;
    uint32_t msgCPU0toCPU1Pi = readl(pThis->pCPU0toCPU1Pi);
    uint32_t msgCPU0toCPU1Ci = pThis->CPU0toCPU1Ci;
    while (msgCPU0toCPU1Pi != msgCPU0toCPU1Ci) //msg from CPU0
    {
        sts = pThis->RecvFPMsg(&pThis->pCPU0toCPU1MsgQ[msgCPU0toCPU1Ci], cM7Core0);

        if (FpsCpu1ChkFPMsgStsDone(sts))
        {
            msgCPU0toCPU1Ci = M7_QUEUE_INC(msgCPU0toCPU1Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            modifyCpu0ToCpu1 = true;
        }
        else
        {
            break;
        }
    }

    if (modifyCpu0ToCpu1)
    {
        pThis->CPU0toCPU1Ci = msgCPU0toCPU1Ci;
        writel(pThis->CPU0toCPU1Ci, pThis->pCPU0toCPU1Ci);
    }

    if ((modifyCpu0ToCpu1 || modifyCpu2ToCpu1) && pThis->ChkRecvFPMsgFiberDone())
    {
        pThis->_fpsCpu1RecvFpMsgFiber.Wait();
        #ifdef IPC_SUPPORT
        IpcIntMaskClr(IPC_FP1, CPU0toCPU1_DESC);
        IpcIntMaskClr(IPC_FP1, CPU2toCPU1_DESC);
        #endif
    }

}

#ifdef NEW_AES_KEY_VALIDATION_SUPPORT
// Validate two XTS keys held in the CDMA vault.
//   return true  -> the two keys DIFFER (valid for XTS)
//   return false -> the two keys are IDENTICAL (XTS violation), OR
//                   either index is out of range.
ATTR_ALWAYS_INLINE bool fpsCpu1::ValidateKeyVaultArrKey(AesKeyVault_t* keyVaultArr, uint16_t keyIndex1, uint16_t keyIndex2)
{
    if (keyIndex1 > KEY_INDEX_MAX || keyIndex2 > KEY_INDEX_MAX)
    {
        return false;
    }

    // Compare 8 u32 words (vault is 32-bit-only MMIO). Return on the
    // first differing word: keys differ -> valid for XTS.
    const uint32_t* k1 = keyVaultArr[keyIndex1].key;
    const uint32_t* k2 = keyVaultArr[keyIndex2].key;
    for (uint32_t w = 0; w < AES_KEY_LEN_IN_WORDS; w++)
    {
        if (k1[w] != k2[w])
        {
            return true;
        }
    }

    // All 8 words matched -> keys are identical -> invalid for XTS.
    //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: same keys at different index, hostKey1:0x%X hostKey2:0x%X\n", ((keyIndex2 << 0x10UL) | (keyIndex1))), "16,16");
    return false;
}
#endif

void fpsCpu1::FpsCpu1CPCDMAIOFiber(void* pObj)
{
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    fpsCpu1* pThis = static_cast<fpsCpu1*>(pObj);
    CdmaSq_t* pCdmaSq = &pThis->_cdmaSq;
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    CdmaSqCmdDescr_t* pCdmaSqe;
    CP2FPMsgDataOpCpCdmaIo_t* pCPCDMAdataTemp = (CP2FPMsgDataOpCpCdmaIo_t*)pThis->pCPCDMAdata;
    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)PSRAM_CP_DFL_BUF_ADDR; //list2 host sqe
    uint8_t rgId1, rgId2, key1SubIdx, key2SubIdx, vfNum;
    uint32_t hostKey1, hostKey2;
    KeyFlags_t* pKeyFlags1;
    KeyFlags_t* pKeyFlags2;
    uint32_t dataLength;
    uint32_t cmdState;
    // vfid is taken from the IPC message received from CP
    vfNum = pCPCDMAdataTemp->CdmaSqe.Dw0.vfid;
    uint16_t currentChunkSize = CHUNK_SIZE;
    uint32_t chunkByteCnt;
    LionNvmeSQDescriptor_t* pHostCmd = NULL;

    if (pThis->hasCPCDMACmd == 0)
    {
        pThis->CPCDMAStatus = 0;
        pThis->CPCDMAError = 0;
        pThis->_fpsCpu1CPCDMAIOFiber.Wait();
        return;
    }
    #ifdef CDMA_CMD_COUNT
    if (unlikely(((pThis->cdmaCmdSlotQueuePi - pThis->cdmaCmdSlotQueueCi) & FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK) >= MAX_IO_CMD_SLOT_COUNT))
    {
        pThis->cdmaCmdSlotQueueCi = readl(pThis->pCdmaCmdSlotQueueCi);
        if (unlikely(((pThis->cdmaCmdSlotQueuePi - pThis->cdmaCmdSlotQueueCi) & FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK) >= MAX_IO_CMD_SLOT_COUNT))
        {
            return;
        }
    }
    #endif

    if (QUEUE_FULL(pThis->cdmaSqPi, pThis->cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
    {
        pThis->cdmaSqCi = readl(pCdmaSq->pHwCi);
        while (QUEUE_FULL(pThis->cdmaSqPi, pThis->cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
        {
            if (readl(pThis->_cdmaFatalErrorFlag))
            {
                ExitQueueManagerFiber();
                return;
            }

            pThis->cdmaSqCi = readl(pCdmaSq->pHwCi);
        }
    }
    /* Process NEW command */
    cmdState = NEW_LAST_CMD;

    pCdmaSqe = pCdmaSqBase + pThis->cdmaSqPi;
    pHostCmd = (LionNvmeSQDescriptor_t*)&(pFpCmd->sqe);
    dataLength = pHostCmd->SrcDataLen;
    if (dataLength > CHUNK_SIZE)
    {
        dataLength = CHUNK_SIZE;
        // Max datalength supported for CDMA IO is 4K.
        // Return error if datalength is more than 4K
        chunkByteCnt = 0;
        // Error handling will be done on CP
        pThis->CPCDMAStatus = cCEStsCpCdmaError;
        pThis->CPCDMAError = cCETinyHostErrDefaultErrorCode;
    }
    else
    {
        chunkByteCnt = dataLength;
    }

    if (unlikely(dataLength & HOST_CMD_16B_ALIGN_MASK))
    {
        if (pHostCmd->cipher)
        {
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsLengthUnaligned16B;
        }
        else
        {
            pThis->CPCDMAStatus = cCEStsInvalidGCMField;
            pThis->CPCDMAError = cCETinyHostErrSqeGcmLengthUnaligned16B;
        }
        DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO:  cCEStsLengthUnalign16B [0x%x]\n", pThis->CPCDMAStatus), "32");
    }

    /* key verification, NOTE: for better performance, do not write the algorithm to an inline function */
    if (pHostCmd->cipher) // XTS_CMD
    {
        AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(pHostCmd + 1);
        // Calculate the key indexes for two XTS Keys.
        uint16_t keyIndex1 = (pAesXtsCmd->HostKeyIdx[0].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[0].keySubIndex;
        uint16_t keyIndex2 = (pAesXtsCmd->HostKeyIdx[1].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[1].keySubIndex;
        uint32_t sessionID = pHostCmd->sessionID;
        uint8_t appID = pHostCmd->appID;

        AesKeyVault_t* keyVaultArr;
        keyVaultArr = (AesKeyVault_t*)(&(rCdma->aesKeyVaultAddr));

        pKeyFlags1 = (KeyFlags_t*)&pThis->_pKeyIsEphemeral[keyIndex1];
        pKeyFlags2 = (KeyFlags_t*)&pThis->_pKeyIsEphemeral[keyIndex2];
        if (pKeyFlags1->keyType != cAesXts)
        {
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey1Type;
        }
        else if (pKeyFlags2->keyType != cAesXts)
        {
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey2Type;
        }
        else if (keyIndex1 >= KEY_INDEX_MAX) // There are two separate keys passed for XTS_CMD.
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS key index out of bound, keyIndex1:0x%X\n", keyIndex1), "32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey1;
        }
        else if (keyIndex2 >= KEY_INDEX_MAX) // There are two separate keys passed for XTS_CMD.
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS key index out of bound, keyIndex2:0x%X\n", keyIndex2), "32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey2;
        }
        else if (keyIndex1 == keyIndex2)
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS keyindex match error, keyIndex1:0x%X keyIndex2:0x%X\n", keyIndex1, keyIndex2), "32","32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsSameKeysPassed;
        }
        else if (pThis->_key2OwnerVfid[keyIndex1] != vfNum) // the VFID for the two keys should match.
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS vfid mismatch error, stored vfid for keyIndex1:0x%X, vfid in command:0x%X\n", pThis->_key2OwnerVfid[keyIndex1], vfNum), "32","32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidVfID;
        }
        else if (pThis->_key2OwnerVfid[keyIndex2] != vfNum)
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS vfid mismatch error, stored vfid for keyIndex2:0x%X, vfid in command:0x%X\n", pThis->_key2OwnerVfid[keyIndex2], vfNum), "32","32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidVfID;
        }
        else if (pKeyFlags1->session_only != pKeyFlags2->session_only)// - the ephemeral flag for the two keys is the same.
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS ephermal flag mismatch error, ephemeral flag for keyIndex1:0x%X ephemeral flag for keyIndex2:0x%X\n", pThis->_pKeyIsEphemeral[keyIndex1] , pThis->_pKeyIsEphemeral[keyIndex2]), "32", "32");
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsDiffEphemeralFlag;
        }
        else if (false == pThis->ValidateKeyVaultArrKey(keyVaultArr, keyIndex1, keyIndex2))
        {
            pThis->CPCDMAStatus = cCEStsInvalidXTSField;
            pThis->CPCDMAError = cCETinyHostErrSqeXtsSameKeysDiffIndex;
        }
        else
        {
            // Validate App ID as it is applicable for both ephermal and persistent
            if (pThis->_pKey2AppID[keyIndex1] != appID)
            {
                DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS appID mismatch, stored appID for keyIndex1:0x%X appID in command:0x%X\n", pThis->_pKey2AppID[keyIndex1], appID), "32", "32");
                pThis->CPCDMAStatus = cCEStsInvalidXTSField;
                pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey1AppID;
            }
            else if (pThis->_pKey2AppID[keyIndex2] != appID)
            {
                DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS appID mismatch, stored appID for keyIndex2:0x%X appID in command:0x%X\n", pThis->_pKey2AppID[keyIndex2], appID), "32", "32");
                pThis->CPCDMAStatus = cCEStsInvalidXTSField;
                pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey2AppID;
            }
            else
            {
                if (pKeyFlags1->session_only)
                {
                    // Validate the session ID as it is applicable only for ephermal
                    if (pThis->_pKey2SessionID[keyIndex1] != sessionID)
                    {
                        DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS sessionID mismatch, stored sessionID for keyIndex1:0x%X sessionID in command:0x%X\n", pThis->_pKey2SessionID[keyIndex1], sessionID), "32", "32");
                        pThis->CPCDMAStatus = cCEStsInvalidXTSField;
                        pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey1SessionID;
                    }
                    else if (pThis->_pKey2SessionID[keyIndex2] != sessionID)
                    {
                        DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS sessionID mismatch, stored sessionID for keyIndex2:0x%X sessionID in command:0x%X\n", pThis->_pKey2SessionID[keyIndex1], sessionID), "32", "32");
                        pThis->CPCDMAStatus = cCEStsInvalidXTSField;
                        pThis->CPCDMAError = cCETinyHostErrSqeXtsInvalidKey2SessionID;
                    }
                    else
                    {
                        // Valid
                        pCdmaSqe->Dw2.LocalKey1Idx = keyIndex1;
                        pCdmaSqe->Dw2.LocalKey2Idx = keyIndex2;
                    }
                }
                else
                {
                    // Valid
                    pCdmaSqe->Dw2.LocalKey1Idx = keyIndex1;
                    pCdmaSqe->Dw2.LocalKey2Idx = keyIndex2;
                }
            }
            //Check if XTS data length exceeds maximum size
            if (unlikely(dataLength > MAX_AES_XTS_DATA_LEN))
            {
                DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: XTS AES XTS data length exceeds MAX length (Max Length is 16MB), length:0x%X\n", dataLength), "32");
                if (pThis->CPCDMAStatus < cCEStsInvalidXTSField)
                {
                    pThis->CPCDMAStatus = cCEStsInvalidXTSField;
                    pThis->CPCDMAError = cCETinyHostErrSqeXtsDataLengthExceeded;
                }
            }
            pCdmaSqe->Dw1.DataProcessType = XTS_CMD + (pHostCmd->EnDecrypt);
        }
    }
    else // GCM_CMD
    {
        AesXtsCmd_t* pAesXtsCmd = (AesXtsCmd_t*)(pHostCmd + 1);
        uint16_t keyIndex1 = (pAesXtsCmd->HostKeyIdx[0].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pAesXtsCmd->HostKeyIdx[0].keySubIndex;
        // Validate AAD length
        AesGcmCmd_t* pAesGcmCmd = (AesGcmCmd_t*)(pHostCmd + 1);
        pKeyFlags1 = (KeyFlags_t*)&pThis->_pKeyIsEphemeral[keyIndex1];

        if(pKeyFlags1->keyType != cAesGcmUnapproved)
        {
            pThis->CPCDMAStatus = cCEStsInvalidGCMField;
            pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidKeyType;
        }
        else if (pAesGcmCmd->AADLen % AAD_ALIGNMENT_CHECK)
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM unaligned AAD length, length:0x%X \n", (uint32_t)pAesGcmCmd->AADLen), "32");
            pThis->CPCDMAStatus = cCEStsInvalidGCMField;
            pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidAadLength;
        }
        else if (keyIndex1 >= KEY_INDEX_MAX)
        {
            // Calculate the key index for the GCM Key.
            // If key index are beyond range then log error.
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM key index out of bound, keyIndex:0x%X \n", keyIndex1 << 0x10UL), "32");
            pThis->CPCDMAStatus = cCEStsInvalidGCMField;
            pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidKey;
        }
        else if ((pThis->_key2OwnerVfid[keyIndex1] != vfNum))
        {
            DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM vfid mismatch error, stored vfid for keyIndex:0x%X, vfid in command:0x%X\n", pThis->_key2OwnerVfid[keyIndex1], vfNum), "32", "32");
            pThis->CPCDMAStatus = cCEStsInvalidGCMField;
            pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidVfID;
        }
        else if (likely(pThis->_key2OwnerVfid[keyIndex1] == vfNum))
        {
            if (pKeyFlags1->session_only)
            {
                if (pThis->_pKey2SessionID[keyIndex1] != pHostCmd->sessionID)
                {
                    DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM ephemeral key sessionID mismatch, stored sessionID for keyIndex:0x%X sessionID in command:0x%X\n", pThis->_pKey2SessionID[keyIndex1], pHostCmd->sessionID), "32", "32");
                    pThis->CPCDMAStatus = cCEStsInvalidGCMField;
                    pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidKeySessionID;
                }
                else if (pThis->_pKey2AppID[keyIndex1] != pHostCmd->appID)
                {
                    DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM ephemeral key appID mismatch, stored appID for keyIndex:0x%X appID in command:0x%X\n", pThis->_pKey2AppID[keyIndex1], pHostCmd->appID), "32", "32");
                    pThis->CPCDMAStatus = cCEStsInvalidGCMField;
                    pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidKeyAppID;
                }
                else
                {
                    // Both session ID and application ID are Valid
                    pCdmaSqe->dw2 = keyIndex1;
                }
            }
            else
            {
                // Incase of persistent key validate application ID
                if (pThis->_pKey2AppID[keyIndex1] == pHostCmd->appID)
                {
                    pCdmaSqe->dw2 = keyIndex1;
                }
                else
                {
                    if (pThis->CPCDMAStatus < cCEStsInvalidGCMField)
                    {
                        DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM persistent key appID mismatch, stored appID for keyIndex:0x%X appID in command:0x%X\n", pThis->_pKey2AppID[keyIndex1], pHostCmd->appID), "32", "32");
                        pThis->CPCDMAStatus = cCEStsInvalidGCMField;
                        pThis->CPCDMAError = cCETinyHostErrSqeGcmInvalidKeyAppID;
                    }
                }
            }
        }
        else
        {
            if (pThis->CPCDMAStatus < cCEStsInvalidGCMField)
            {
                DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM invalid key, KeyIndex:0x%X \n", keyIndex1), "32");
                DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO: GCM invalid key, vfId:0x%X pThis->_key2OwnerVfid[keyIndex1]:0x%X \n", ((pThis->_key2OwnerVfid[keyIndex1] << 0x10UL) | (vfNum))), "16,16");
                pThis->CPCDMAStatus = cCETinyHostErrSqeGcmUndefinedErr;
            }
        }
        pCdmaSqe->Dw1.DataProcessType = GCM_CMD + (pHostCmd->EnDecrypt);
    }

    pCdmaSqe->Dw1.DataDescrType = pHostCmd->prpSgl;
    pCdmaSqe->Dw1.DataContinuation = 0;
    pCdmaSqe->Dw1.XtsDataUnitLength = pHostCmd->dataUnitLen;
    pCdmaSqe->Dw1.DataXfrAttrReserved = 0;
    pCdmaSqe->Dw1.Reserved = 0;
    // UcdIqId is zero, as this is an internal command
    pCdmaSqe->Dw1.UcdIqId = 0;
    // Interface Select is taken from the IPC message received from CP
    pCdmaSqe->Dw3.SrcDescrIfSel = pCPCDMAdataTemp->CdmaSqe.Dw1.srcDescInterfaceSel;
    pCdmaSqe->Dw3.SrcDataIfSel = pCPCDMAdataTemp->CdmaSqe.Dw1.srcDataInterfaceSel;
    pCdmaSqe->Dw3.DstDescrIfSel = pCPCDMAdataTemp->CdmaSqe.Dw1.destDescInterfaceSel;
    pCdmaSqe->Dw3.DstDataIfSel = pCPCDMAdataTemp->CdmaSqe.Dw1.destDataInterfaceSel;
    pHostCmd->PoorSGLRetryTimes = 0;
    pHostCmd->chunkSize = 0;
    // Alignment for completion CQE
    pHostCmd->PASID = pHostCmd->DW1;

    pCdmaSqe->Dw1.ChunkByteCnt = chunkByteCnt;
    pCdmaSqe->Dw0.CmdId = CP_CDMA_IO_CMD_ID;
    pCdmaSqe->Dw0.CmdState = cmdState;
    pCdmaSqe->Dw0.CmdOpcode = CDMA_OPCODE_DATA_TRANSFER;
    pCdmaSqe->Dw0.Reserved = 0;
    // Only 1 command at a time, so no need to support multiple DFL Index.
    pCdmaSqe->Dw0.CmdDflIdx = 0;
    pCdmaSqe->Dw0.DflNum = CDMA_CP_DFL_LIST;

    // error handle. set zero transfer length
    if (unlikely((pThis->CPCDMAStatus >= cCEStsInvalidXTSField)))
    {
        pCdmaSqe->Dw1.ChunkByteCnt = 0;
        // No data to transfer, only need completion on FP2
        pHostCmd->SrcDataLen = 0;
        pHostCmd->DstDataLen = 0;
    }

    //Submit entry to CDMA
    pThis->cdmaSqPi = QUEUE_INC(pThis->cdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);
    DMB();
    writel(pThis->cdmaSqPi, pCdmaSq->pHwPi);
    #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
    writel(pThis->cdmaSqPi, pThis->cdmaSqPiHwAddr);
    #endif
    // Set to zero to indicate that comment has been submitted to CDMA.
    pThis->hasCPCDMACmd = 0;
    writel(pThis->CPCDMAStatus, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
    writel(pThis->CPCDMAError, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
    pThis->CPCDMAStatus = 0;
    pThis->CPCDMAError = 0;

     pThis->_fpsCpu1CPCDMAIOFiber.Wait();
    #ifdef CDMA_CMD_COUNT
     pThis->cdmaCmdSlotQueuePi = QUEUE_INC(pThis->cdmaCmdSlotQueuePi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
    #endif
    #endif
}

void fpsCpu1::CheckFPMsgFiberNeedResume(void* pObj)
{
    fpsCpu1* pThis = static_cast<fpsCpu1*>(pObj);
    {
        pThis->_fpsCpu1RecvFpMsgFiber.Resume();
    } // else do nothing
}
