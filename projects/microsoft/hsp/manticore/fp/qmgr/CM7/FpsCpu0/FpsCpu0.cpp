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
#include "Heartbeat.h"

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
Ucd_t* rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
Cdma_t* rCdma = (Cdma_t*)CDMA_REG_ADDR;

#define GET_CDMA_LIST_NUM(dflListNum)     (dflListNum == DFL_0) ? CDMA_FP_DFL_0_LIST : CDMA_FP_DFL_3_LIST

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

ATTR_ALWAYS_INLINE void fpsCpu0::RefillDFL(uint32_t dflAddr)
{
    FpsUcdIbq_t* pUcdIbq = &_ucdIbq;
    uint64_t* pDflEntries;
    DFL_Index_t dflNum = GET_DFL_NUM_FROM_ADDR(dflAddr);

    pDflEntries = pUcdIbq->pDflEntries[dflNum];
    writel((uint32_t)dflAddr, &pDflEntries[inBoundDflPi[dflNum]]); // assume high bytes shall be zero
    inBoundDflPi[dflNum] = QUEUE_INC(inBoundDflPi[dflNum], ibDflPiMask[dflNum]);

    writel(inBoundDflPi[dflNum], pUcdIbq->pHwDflPi[dflNum]);

    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t disableBit = (dflNum == DFL_0) ? SOC_REG_0_WR_BIT : SOC_REG_2_WR_BIT;
    if (readl(REG_FPS_INDIRECT_REG_WR_DISABLE) & disableBit)
    {
        writel(inBoundDflPi[dflNum], hwDflPiAddr[dflNum]);
    }
    #endif
}

#ifdef LIONPERF_SUPPORT
ATTR_ALWAYS_INLINE void fpsCpu0::LoadDFL(FWupdateBackupInfo *pFWupdateInfo)
{
    GetFwUpdateInfo(pFWupdateInfo, cUCDData);

    uint32_t DflListVirAddr = (uint32_t)M7_FPS_CPU0_DFL_LIST_0_ADDR;
    uint32_t DFLListAddr;

    if (pFWupdateInfo->sts == cNoSignature)
    {
        DFLListAddr = (uint32_t)(OLD_PSRAM_DFL_LIST_BACKUP);
    }
    else
    {
        DFLListAddr = (uint32_t)pFWupdateInfo->addr;
    }

    uint16_t* dflCopyBase = (uint16_t*)(DFLListAddr);
    uint64_t* pEntries = (uint64_t*)DflListVirAddr;
    uint64_t DflBuffAddr = (uint64_t)(getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR));

    for (uint16_t i = 0; i < UCD_DFL_Q_SIZE; i++)
    {
        uint16_t dflIndex = readw(&dflCopyBase[i]);
        uint64_t curDFLAddr = DflBuffAddr + (uint64_t)(((uint64_t)dflIndex) << (uint64_t)DFL_BUF_SZ_SHIFT);
        writeq(curDFLAddr, &pEntries[i]);
    }

    pEntries = (uint64_t*)((uint32_t)M7_FPS_CPU0_DFL_LIST_1_ADDR);
    DflBuffAddr = (uint64_t)(getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR));

    for (uint16_t i = 0; i < UCD_DFL_1_Q_SIZE; i++)
    {
        uint16_t dflIndex = readw(&dflCopyBase[i + UCD_DFL_Q_SIZE]);
        uint64_t curDFLAddr = DflBuffAddr + (uint64_t)(((uint64_t)dflIndex) << (uint64_t)DFL_BUF_SZ_SHIFT);
        writeq(curDFLAddr, &pEntries[i]);
    }
}
#endif

void fpsCpu0::Initialize(M7CompGroupId_t compId)
{
    // Init fiber parameters
    InitializeFiber();
    RegisterComponentGroup(compId);
}

void fpsCpu0::RegisterComponentGroup(M7CompGroupId_t compId)
{
    // Register Component Group
    M7FiberScheduler_RegisterCompGroup(cM7Core0, compId,
                                       static_cast<M7FiberId_t>(fpsCpu0FiberId_t::cNumberOfFibers),
                                       reinterpret_cast<OnM7FiberSchedulerInitializedFptr_t>(&fpsCpu0::RegisterFibers),
                                       static_cast<void*>(this));
}

void fpsCpu0::RegisterFibers(void* pObj)
{
    //Get the Component instance
    fpsCpu0* pThis = static_cast<fpsCpu0*>(pObj);
    pThis->_fpsCpu0FpCmdHandlerFiber.Register(pThis, &fpsCpu0::FpsCpu0FpCmdHandlerFiber, "FpsCpu0FpCmdHandlerFiber", false);
    pThis->_fpsCpu0RecvFpMsgFiber.Register(pThis, &fpsCpu0::FpsCpu0ReceiveFPMsgFiber, "FpsCpu0ReceiveFPMsgFiber");

    pThis->_fpsCpu0RecvFpMsgFiber.Wait();
    pThis->_fpsCpu0CheckHeartbeatFiber.Register(pThis, &CheckHeartbeatFiber, "FpsCpu0CheckHeartbeatFiber");
    pThis->_fpsCpu0CheckHeartbeatFiber.Activate();
}

void fpsCpu0::InitializeFiber()
{
    pCpuStatus = (uint32_t*)(PSRAM_FP_CPU0_STATUS_ADDR);
    writel(FP_STS_INIT_START, pCpuStatus);

    // Init fiber parameters fiberParamsfpsCpu0FpCmdHandlerFiber
    M7FiberParameters_t fiberParamsFpsCpu0FpCmdHandlerFiber = {0};
    fiberParamsFpsCpu0FpCmdHandlerFiber.fiberWeight = cFiberWeightFpsCpu0FpCmdHandlerFiber;

    // Initialize Fiber object fiberParamsfpsCpu0FpCmdHandlerFiber
    _fpsCpu0FpCmdHandlerFiber.Initialize(cM7Core0, cM7CompGroupIo, \
                                         static_cast<M7FiberId_t>(fpsCpu0FiberId_t::cFpsCpu0FpCmdHandlerFiberId), fiberParamsFpsCpu0FpCmdHandlerFiber);

    //Init fiber parameters object fiberParamsRecvFpMsg
    M7FiberParameters_t fiberParamsRecvFpMsg = {0};
    fiberParamsRecvFpMsg.fiberWeight = cFiberWeightRecvFPMsg;

    // Initialize Fiber object fpsCpu0RecvFpMsgFiber
    _fpsCpu0RecvFpMsgFiber.Initialize(cM7Core0, cM7CompGroupIo, \
                                      static_cast<M7FiberId_t>(fpsCpu0FiberId_t::cFpsCpu0RecvFpMsgFiberId), fiberParamsRecvFpMsg);

    //Init fiber parameters object CheckHeartbeat
    M7FiberParameters_t fiberParamsCheckHeartbeat = {0};
    fiberParamsCheckHeartbeat.fiberWeight = cFiberWeightCheckHeartbeatFiber;

    // Initialize Fiber object CheckHeartbeat
    _fpsCpu0CheckHeartbeatFiber.Initialize(cM7Core0, cM7CompGroupIo, \
                                      static_cast<M7FiberId_t>(fpsCpu0FiberId_t::cFpsCpu0CheckHeartbeatFiberId), fiberParamsCheckHeartbeat);

    rFps = (Fps_t*)FPS_REG_ADDR;
    rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
    rCdma = (Cdma_t*)CDMA_REG_ADDR;
    rTcon = (Tcon_t*)TCON_REG_ADDR;
    rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;

    pCP0toFPMsgQ = (CP2FPMsgContext_t*)(PSRAM_CP0toFP_REQ_MSG_ADDR);
    pCP1toFPMsgQ = (CP2FPMsgContext_t*)(PSRAM_CP1toFP_REQ_MSG_ADDR);
    pCPU2toCPU0Pi = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU0_PI_ADDR);
    pCPU2toCPU0Ci = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU0_CI_ADDR);
    pCPU2toCPU0MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU2_2_CPU0_MSG_ADDR);

    pCPU0toCPU2Pi = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU2_PI_ADDR);
    pCPU0toCPU2Ci = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU2_CI_ADDR);
    pCPU0toCPU2MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU0_2_CPU2_MSG_ADDR);

    pCPU0toCPU1Pi = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU1_PI_ADDR);
    pCPU0toCPU1Ci = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU1_CI_ADDR);
    pCPU0toCPU1MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU0_2_CPU1_MSG_ADDR);

    pCPU1toCPU0Pi = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU0_PI_ADDR);
    pCPU1toCPU0Ci = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU0_CI_ADDR);
    pCPU1toCPU0MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU1_2_CPU0_MSG_ADDR);

    CPU2toCPU0Ci = 0;
    CPU0toCPU2Pi = 0;

    CPU0toCPU1Pi = 0;
    CPU1toCPU0Ci = 0;

    M7_MEM_SET((void*)pCPU0toCPU2Pi, 0, 4);
    M7_MEM_SET((void*)pCPU2toCPU0Ci, 0, 4);

    M7_MEM_SET((void*)pCPU0toCPU1Pi, 0, 4);
    M7_MEM_SET((void*)pCPU1toCPU0Ci, 0, 4);

    // Setup FP hardware offload register.
    // UCD core 1, IB CQ 0
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq00UcdIbCq0].fpsHwe2fpHwEngineToFpQSize.all = 9; // 512 entries
    _ucdIbq.pHwIbCqCi[IBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq00UcdIbCq0].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    _ucdIbq.pHwIbCqPi[IBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq00UcdIbCq0].fpsHwe2fpHwEngineToFpQPiShadow.all);
    _ucdIbq.pHwStatus = &(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all);
    inBoundCqCi[IBCQ_0] = readl(_ucdIbq.pHwIbCqCi[IBCQ_0]);

    // UCD core 1, IB CQ 1
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq01UcdIbCq1].fpsHwe2fpHwEngineToFpQSize.all = 5; // 32 entries
    _ucdIbq.pHwIbCqCi[IBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq01UcdIbCq1].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    _ucdIbq.pHwIbCqPi[IBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq01UcdIbCq1].fpsHwe2fpHwEngineToFpQPiShadow.all);
    inBoundCqCi[IBCQ_1] = readl(_ucdIbq.pHwIbCqCi[IBCQ_1]);

    // UCD core 1, DFL 0, 512 elements
    _ucdIbq.pHwDflPi[DFL_0] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd00Ucd1Dfl0].fpsSocFwdSocIndirectDataPortSocIndirectRegData);

    // UCD core 1, DFL 3, 32 elements
    _ucdIbq.pHwDflPi[DFL_1] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd02Ucd1Dfl3].fpsSocFwdSocIndirectDataPortSocIndirectRegData);

    // UCD core 1, OB CQ 0, 512 elements
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQSize.all = 9;
    _ucdObq.pHwObCqPi[OBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQPiShadow.all);
    _ucdObq.pHwObCqCi[OBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    _ucdObq.pHwStatus = &(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all);
    outBoundCqCi[OBCQ_0] =  readl(_ucdObq.pHwObCqCi[OBCQ_0]);

    // UCD core 1, OB CQ 1, 32 elements
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQSize.all = 5;
    _ucdObq.pHwObCqPi[OBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQPiShadow.all);
    _ucdObq.pHwObCqCi[OBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    outBoundCqCi[OBCQ_1] =  readl(_ucdObq.pHwObCqCi[OBCQ_1]);

    // UCD core 1, OSL 0, 512 elements
    _ucdObq.pHwOslPi[OBCQ_0] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd01Ucd1Osl0].fpsSocFwdSocIndirectDataPortSocIndirectRegData);
    inBoundDflPi[OBCQ_0] = readl(_ucdIbq.pHwDflPi[DFL_0]);
    // UCD core 1, OSL 1, 32 elements
    _ucdObq.pHwOslPi[OBCQ_1] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd03Ucd1Osl1].fpsSocFwdSocIndirectDataPortSocIndirectRegData);
    inBoundDflPi[OBCQ_1] = readl(_ucdIbq.pHwDflPi[DFL_1]);

    // setup FP slot register to 4 elements
    for (uint8_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
    {
        writel(2, (REG_FPS_SLOT_ARRAY_SIZE_BASE + (i << 4)));
    }

    // setup UCD Inbound / Outbound queue / CE / CE tiny
    _ucdIbq.pIbCqe[IBCQ_0] = (UcdCqEntry_t*)(M7_FPS_CPU0_IBCQ_0_ADDR);
    _ucdIbq.pIbCqe[IBCQ_1] = (UcdCqEntry_t*)(M7_FPS_CPU0_IBCQ_1_ADDR);
    _ucdIbq.pDflEntries[DFL_0] = (uint64_t*)(M7_FPS_CPU0_DFL_LIST_0_ADDR);
    _ucdIbq.pDflEntries[DFL_1] = (uint64_t*)(M7_FPS_CPU0_DFL_LIST_1_ADDR);
    _ucdObq.pCqEntries[OBCQ_0] = (UcdCqEntry_t*)(M7_FPS_CPU0_OBCQ_0_ADDR);
    _ucdObq.pCqEntries[OBCQ_1] = (UcdCqEntry_t*)(M7_FPS_CPU0_OBCQ_1_ADDR);
    _pCmdEntryArray = (CmdEntry_t*)(M7_FPS_CPU01_CMD_ARRAY_BASE_ADDR);
    _pCmdEntryArrayTiny = (CmdEntryTiny_t*)(GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU20_CMD_ARRAY_TINY_BASE_ADDR));

    #ifdef LIONPERF_SUPPORT
    M7_MEM_SET((void*)M7_FPS_CPU0_DFL_LIST_0_ADDR, 0, IB_DFL_0_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU0_DFL_LIST_1_ADDR, 0, IB_DFL_1_SIZE);
    #else
    if(gResetType == cPor)
    {
        M7_MEM_SET((void*)M7_FPS_CPU0_DFL_LIST_0_ADDR, 0, IB_DFL_0_SIZE);
        M7_MEM_SET((void*)M7_FPS_CPU0_DFL_LIST_1_ADDR, 0, IB_DFL_1_SIZE);
    }
    #endif

    M7_MEM_SET((void*)M7_FPS_CPU0_IBCQ_0_ADDR, 0, IB_CQ_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU0_IBCQ_1_ADDR, 0, IB_CQ_1_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU0_OBCQ_0_ADDR, 0, OB_CQ_0_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU0_OBCQ_1_ADDR, 0, OB_CQ_1_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU01_CMD_ARRAY_BASE_ADDR, 0, M7_SHARE_CMD_ARRAY_SIZE);
    M7_MEM_SET((void*)_pCmdEntryArrayTiny, 0, M7_SHARE_CMD_ARRAY_TINY_SIZE);

    // Inbound / outbound mapping information, Cpu20 share TCM
    _pSlotFlagSts = (uint8_t*)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU20_SLOT_STATUS_ADDR);
    _pIbQ2ObQ = (uint8_t*)(CPU0AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_IBQ2OBQ_ADDR));
    _CPU1SubmitAbortInfo = (uint8_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_M7_CPU1_SUBMIT_ABORT_INFO));

    for (uint8_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
    {
        cmdArrayPi[i] = 0;
        cmdArrayCi[i] = 0;
        uint32_t* pCmdArrayHwPi = (uint32_t*)(REG_FPS_SLOT_ARRAY_PI_BASE + (i << CMD_ARRAY_SHIFT));
        uint32_t* pCmdArrayHwCi = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (i << CMD_ARRAY_SHIFT));
        DMB();
        writel(cmdArrayPi[i], pCmdArrayHwPi);
        writel(cmdArrayCi[i], pCmdArrayHwCi);
        _CPU1SubmitAbortInfo[i] = ABORT_NOT_SUBMIT;
    }
    if (gResetType == cPor)
    {
        for (uint8_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
        {
            _pIbQ2ObQ[i] = QID_INVALID;
            _pSlotFlagSts[i] = cStsInit;
        }
    }

    // CPU01 share TCM, CPU0 view
    _pVfInfoBase = (VFNodeInfo_t*)(M7_FPS_CPU01_VF_INFO_BASE);
    _pQueueBlockInfoBase = (QueueBlockInfo_t*)(CPU0AccessCPU1TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU01_QB_INFO_BASE)));
    _pVFEnBitmap = (uint64_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_ENABLE_BIT_MAP_ADDR));
    _pQBEnBitmap = (uint64_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_QB_ENABLE_BIT_MAP_ADDR));
    _pVfCmdExistBitMap = (uint64_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_CMD_EXIST_BIT_MAP_ADDR));
    _pVF65EnBitmap = (uint32_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_65_ENABLE_BIT_MAP_ADDR));
    _pQB65EnBitmap = (uint8_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_QB_65_ENABLE_BIT_MAP_ADDR));
    _pVf65CmdExistBitMap = (uint32_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_65_CMD_EXIST_BIT_MAP_ADDR));
    _pTotalCredit = (volatile int32_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_TOTAL_CREDIT_ADDR));
    _pVfCredit = (volatile int32_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_CREDIT_INFO_BASE));
    _pVfRemainCredit = (volatile int32_t*)(CPU0_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_REMAIN_CREDIT_INFO_BASE));

    if (gResetType == cPor)
    {
        M7_MEM_SET((void*)_pVfCredit, 0, M7_SHARE_VF_CREIDT_INFO_SIZE);
        M7_MEM_SET((void*)_pVfRemainCredit, 0, M7_SHARE_VF_REMAIN_CREIDT_INFO_SIZE);
        *_pTotalCredit = 0;
        *_pVFEnBitmap = 0;
        *_pVF65EnBitmap = 0;
    }
    *_pVfCmdExistBitMap = 0;
    *_pVf65CmdExistBitMap = 0;
    pFLRRequestBitMapLocal = (uint64_t*)(CPU0AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_FLR_REQUSET_BIT_MAP_LOCAL));

    if (gResetType == cPor)
    {
        for (uint32_t i = 0; i < (MAX_VF_NUM + MAX_PF_NUM); i++)
        {
            _pVfInfoBase[i].vfId = i;
            _pVfInfoBase[i].credit = 0;
            _pVfInfoBase[i].queueBlkBitMap = 0;
            _pVfInfoBase[i].queueBlk65BitMap = 0;
            _pQBEnBitmap[i] = 0;
            _pQB65EnBitmap[i] = 0;
            #ifdef QOS_LATENCY_ERROR_HANDLING
            _pVfInfoBase[i].qosPenaltyPeriod = 0;
            #endif
        }

        for (uint32_t i = 0; i < (FPS_QUEUE_BLOCK_NUM + FPS_QUEUE_BLOCK_65); i++)
        {
            _pQueueBlockInfoBase[i].queueBlockIndex = i;
            _pQueueBlockInfoBase[i].vfId = VFID_INV;
            _pQueueBlockInfoBase[i].remainLen = 0;
            _pQueueBlockInfoBase[i].remainCeIdx = 0;
        }
    }
    for (uint32_t i = 0; i < (FPS_QUEUE_BLOCK_NUM + FPS_QUEUE_BLOCK_65); i++)
    {
        _pQueueBlockInfoBase[i].remainLen = 0;
        _pQueueBlockInfoBase[i].remainCeIdx = 0;
    }

    _rgid2OwnerVfid = (uint8_t*)(M7_FPS_CPU01_KEY_TABLE_RGID_TO_OWNER_VFID_ADDR);
    _key2OwnerVfid = (uint8_t*)(M7_FPS_CPU01_KEY_TABLE_KEY_TO_OWNER_VFID_ADDR);

    if (gResetType == cPor)
    {
        for (uint8_t i = 0; i <= (MAX_FP_RGID_NUM); i++)
        {
            _rgid2OwnerVfid[i] = RGID_NO_OWNER_VF;
        }
        M7_MEM_SET((void*)_key2OwnerVfid, RGID_NO_OWNER_VF, 0x200);
    }

    _pIbCmnReg[UCD_CORE_0] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR);
    _pIbCmnReg[UCD_CORE_1] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR + UCD_IBOB_CORE_OFFSET);
    _pObCmnReg[UCD_CORE_0] = (UcdCore0ObCmnRegisters_t*)(UCD_OB_REGS_ADDR);
    _pObCmnReg[UCD_CORE_1] = (UcdCore0ObCmnRegisters_t*)(UCD_OB_REGS_ADDR + UCD_IBOB_CORE_OFFSET);

    #ifdef SUPPORT_UPDATE_TIMESTAMP
    gTimerCounterBase = 0;
    gTimerCounterLast = 0;
    gTimerCounterCovert = 0;
    gTimerCounterCount = 0;
    #endif
    #ifdef SUPPORT_TELEMETRY
    pOutstandingIoCnt = (uint32_t*)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU20_OUTSTANDING_IO_CNT_ADDR);
    *pOutstandingIoCnt = 0;
    pAccumulateIoCnt = (uint64_t*)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU20_ACCUMULATE_IO_CNT_ADDR);
    *pAccumulateIoCnt = 0;
    #endif

    // retry CE new scheme
    retryCEQueuePi = 0;
    retryCEQueueCi = 0;
    pRetryCEQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq02].fpsCpuxToCpuyQueueProducerIndex.all);
    pRetryCEQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq02].fpsCpuxToCpuyQueueConsumerIndex.all);;
    pRetryCeIndexQueue = (uint16_t*)(M7_FPS_CPU20_RETRY_CE_QUEUE + 0x10000);
    writel(0, pRetryCEQueuePi);
    writel(0, pRetryCEQueueCi);

    // refill DFL new scheme
    ceForRefillDFLQueuePi = 0;
    ceForRefillDFLQueueCi = 0;
    pCeForRefillDFLQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq03].fpsCpuxToCpuyQueueProducerIndex.all);
    pCeForRefillDFLQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq03].fpsCpuxToCpuyQueueConsumerIndex.all);
    pCEforRefillDFLQueue = (uint16_t*)(M7_FPS_CPU20_REFILL_CE_DFL_QUEUE + 0x10000);
    writel(0, pCeForRefillDFLQueuePi);
    writel(0, pCeForRefillDFLQueuePi);

    _adminAbortCount = (uint8_t*)(CPU0AccessCPU2TCMMem((uint32_t)M7_FPS_CPU20_ADMIN_ABORT_COUNT_ADDR));

    /*  The Mapping of Physical Queue ID and Command Array (CA)
     *
     *  [Original]
     *  IBQ : QB0 = [0,1]     QB1 = [2,3]     QB2 = [4,5]    ......  QB63 = [63, 127]      QB64 [128, 129]
     *  CA  : QB0 = [0,64]    QB1 = [1,65]    QB2 = [2,66]   ......  QB63 = [63, 127]      QB64 [128, 129]
     *
     *  [NEW_IB_PHY_Q_MAP for VF64]
     *  IBQ : QB0 = [0,64]    QB1 = [1,65]    QB2 = [2,66]   ......  QB63 = [63, 127]
     *  CA  : QB0 = [0,64]    QB1 = [1,65]    QB2 = [2,66]   ......  QB63 = [63, 127]
     *
     *  [NEW_IB_PHY_Q_MAP for VF65]
     *  IBQ : QB0 = [0,65]    QB1 = [1,66]    QB2 = [2,67]   ......  QB63 = [63, 128]      QB64[64, 129]
     *  CA  : QB0 = [0,64]    QB1 = [1,65]    QB2 = [2,66]   ......  QB63 = [63, 127]      QB64[128, 129]
     */
    pCa2IbPhysicalId = (uint8_t*)M7_FPS_CPU01_CA_2_IBPHYQID_TABLE;
    pIbPhysicalId2Ca = (uint8_t*)M7_FPS_CPU01_IBPHYQID_2_CA_TABLE;
    for (uint8_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
    {
        if (i < 64)
        {
            pIbPhysicalId2Ca[i] = i;
            pCa2IbPhysicalId[i] = i;
        }
        else if (i > 64)
        {
            pIbPhysicalId2Ca[i] = i - 1;
            pCa2IbPhysicalId[i] = i + 1;
        }
        pIbPhysicalId2Ca[64] = 128;
        pIbPhysicalId2Ca[129] = 129;
        pCa2IbPhysicalId[64] = 65;
        pCa2IbPhysicalId[128] = 64;
        pCa2IbPhysicalId[129] = 129;
    }

    ibDflPiMask[DFL_0] = DFL_BUF_INDEX_MASK;
    ibDflPiMask[DFL_1] = DFL_1_BUF_INDEX_MASK;

    #ifdef LIONPERF_SUPPORT
    if(gResetType == cFwUpdateWarmReset)
    {
        FWupdateBackupInfo FWupdateInfo;
        FwUdSts FwChecksumSts = CheckBackupData();
        uint8_t FwUpdateSts = GetFwUpdateStatus(FwChecksumSts);
        if( !FwUpdateSts )
        {
            GetFwUpdateInfo(&FWupdateInfo, cLoggingData);
            LoggingResumeBootInit(&FWupdateInfo);
            // LoadDFL() = RESUME_BOOT
            GetFwUpdateInfo(&FWupdateInfo, cUCDData);
            LoadDFL(&FWupdateInfo);
        }
        else
        {
            //DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("Firmware update failed, FwChecksumSts:0x%X FwUpdateSts:0x%X\n", FwChecksumSts, FwUpdateSts ), "32", "32");
        }
    }
    #endif

    writel(FP_STS_INIT_DONE, pCpuStatus);
}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0TearDownRefillDFL(uint16_t ceIndex, uint8_t caIndex, uint8_t ibPhyQId)
{
    uint32_t dflAddr = GET_DFL_PHYSICAL_BUF_ADDR(_pCmdEntryArray[ceIndex].cdmaListNum, (_pCmdEntryArray[ceIndex].DFLIdx << DFL_BUF_SZ_SHIFT));

    /* Refill DFL, free the CE / CE tiny resource, and complete an outstanding IO */
    RefillDFL(dflAddr);

    _pCmdEntryArray[ceIndex].Dw0 = 0;
    _pCmdEntryArrayTiny[ceIndex].Dw0 = 0;

    cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);

    #ifdef SUPPORT_TELEMETRY
    *pOutstandingIoCnt = *pOutstandingIoCnt - 1;
    #endif

    DMB();

    /* Check if there is outstanding IO in FP firmware. If there is no any command,
       try to clear internal data structure of queue block and PF/VF
     */
    uint8_t qbIndex = SQ_PID_2_QBIDX(ibPhyQId);
    HandleDeleteVFQblk(ibPhyQId, _pQueueBlockInfoBase[qbIndex].vfId, _pSlotFlagSts[ibPhyQId]);

}

ATTR_ALWAYS_INLINE bool fpsCpu0::FpsCpu0RetryErrorCmd(uint16_t orgCeIndex, uint16_t newCeIndex, uint8_t caIndex, uint8_t ibPhyQId)
{
    bool retryFail = FALSE;
    uint32_t* pCmdArrayHwPi = NULL;

    /* use original CE, keep the status of CE, and keep the retry time of CE tiny */
    if (newCeIndex == orgCeIndex)
    {
        _pCmdEntryArrayTiny[newCeIndex].ErrStatus = 0;

        cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);
        cmdArrayPi[caIndex] = QUEUE_INC(cmdArrayPi[caIndex], CA_ROLLOVER_MASK);
        pCmdArrayHwPi = (uint32_t*)(REG_FPS_SLOT_ARRAY_PI_BASE + (caIndex << CMD_ARRAY_SHIFT));

        DMB();

        writel(cmdArrayPi[caIndex], pCmdArrayHwPi);
    }
    else
    {
        if (_pCmdEntryArray[newCeIndex].Status == cCEStsInValid)
        {
            _pCmdEntryArray[newCeIndex].Dw0 = _pCmdEntryArray[orgCeIndex].Dw0;

            _pCmdEntryArrayTiny[newCeIndex].ErrStatus = 0;
            _pCmdEntryArrayTiny[newCeIndex].RetryTimes = _pCmdEntryArrayTiny[orgCeIndex].RetryTimes;
            _pCmdEntryArrayTiny[newCeIndex].abortStatus = _pCmdEntryArrayTiny[orgCeIndex].abortStatus;

            _pCmdEntryArray[orgCeIndex].Dw0 = 0;
            _pCmdEntryArrayTiny[orgCeIndex].Dw0 = 0;

            cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);
            cmdArrayPi[caIndex] = QUEUE_INC(cmdArrayPi[caIndex], CA_ROLLOVER_MASK);
            pCmdArrayHwPi = (uint32_t*)(REG_FPS_SLOT_ARRAY_PI_BASE + (caIndex << CMD_ARRAY_SHIFT));

            DMB();

            writel(cmdArrayPi[caIndex], pCmdArrayHwPi);
        }
        else
        {
            retryFail = TRUE;
        }
    }

    return retryFail;

}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0CheckSlotStatusAndHandling(uint8_t slotSts, uint8_t ibPhyQId)
{
    if (unlikely(slotSts & (cStsDelete | cStsTearDown | cStsForceCompletion | cStsFwUpdate)))
    {
        if (slotSts & (cStsDelete | cStsTearDown))
        {
            uint8_t qbIndex = SQ_PID_2_QBIDX(ibPhyQId);
            uint8_t vfId = _pQueueBlockInfoBase[qbIndex].vfId;

            HandleDeleteVFQblk(ibPhyQId, vfId, slotSts);
        }

        if (slotSts & (cStsForceCompletion | cStsFwUpdate))
        {
            if (!ChkQisRunningSetSlotSts(slotSts, ibPhyQId))
            {
                if (slotSts & cStsForceCompletion)
                {
                    _pSlotFlagSts[ibPhyQId] &= (uint8_t)(~(cStsForceCompletion));
                }

                if (slotSts & cStsFwUpdate)
                {
                    _pSlotFlagSts[ibPhyQId] &= (uint8_t)(~(cStsFwUpdate));
                }
            }
        }
    }
}

ATTR_ALWAYS_INLINE bool fpsCpu0::FpsCpu0FillCmdEntryAndSendToCpu1(uint8_t ibcqIndex, uint16_t qMask)
{
    bool cmdStatus = TRUE;

    UcdCqEntry_t* pCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[inBoundCqCi[ibcqIndex]]);
    uint8_t ibPhyQId = pCqe->QPId;
    uint8_t caIndex = pIbPhysicalId2Ca[ibPhyQId];
    uint8_t caPi = cmdArrayPi[caIndex];
    uint16_t ceIndex = (caIndex << CA_SIZE_SHIFT) + (caPi & (uint8_t)CA_MASK);
    volatile CmdEntry_t* pCe = &(_pCmdEntryArray[ceIndex]);
    uint8_t cdmaListNum = GET_CDMA_LIST_NUM(pCqe->ListId);
    // extract the 16 bit DFL index
    uint16_t dflIdx = ((pCqe->AddrLow  & 0xffff) >> (uint8_t)DFL_BUF_SZ_SHIFT);

    if (likely(pCe->Status == cCEStsInValid))
    {
        pCe->Dw0 = (cdmaListNum << FPS_CE_DFL_NUM_SHIFT) | (dflIdx << FPS_CE_DFL_ID_SHIFT) | \
                   (pCqe->IFSel << FPS_CE_IFSEL_SHIFT) | (pCqe->QPId << FPS_CE_QPID_SHIFT) | cCEStsValid;

        /* Found admin abort host command in IB CQ, mark the abort status, send it to CDMA */
        if (unlikely(pCqe->Abort))
        {
            pCe->Status = cCEStsCdmaAbort;
            _pCmdEntryArrayTiny[ceIndex].abortStatus = cCETinyAdminAbort;
        }

        caPi = QUEUE_INC(caPi, CA_ROLLOVER_MASK);
        cmdArrayPi[caIndex] = caPi;

        uint32_t* pCmdArrayHwPi = (uint32_t*)(REG_FPS_SLOT_ARRAY_PI_BASE + (caIndex << CMD_ARRAY_SHIFT));

        DMB();
        writel(caPi, pCmdArrayHwPi);

        #ifndef DISABLE_IO_LOG
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("[IO LOG] Inbound cmd entry: sts:0x%X, ibqid:0x%02X, ifsel:0x%02X, dfl idx:0x%03X, list num:0x%X\n", \
                                                       pCe->Dw0), "4,8,8,10,2");
        #endif

        inBoundCqCi[ibcqIndex] = QUEUE_INC(inBoundCqCi[ibcqIndex], qMask);
        #ifdef SUPPORT_TELEMETRY
        *pOutstandingIoCnt = (*pOutstandingIoCnt) + 1;
        #endif
    }
    else
    {
        cmdStatus = FALSE;
    }

    return cmdStatus;

}

void fpsCpu0::FpsCpu0ProcessObCqFailure(uint8_t obcqIndex)
{
    UcdCqEntry_t* pObCqe = &((_ucdObq.pCqEntries[obcqIndex])[outBoundCqCi[obcqIndex]]);
    DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("[IO LOG] Outbound cq Fail: cqci:0x%X, ErrSts:0x%X\n", outBoundCqCi[obcqIndex], pObCqe->ErrSts));

}

void fpsCpu0::FpsCpu0CheckIBCqeValidAndDFLAddress(uint32_t dflAddr, uint8_t ibcqIndex)
{
    UcdCqEntry_t* pCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[inBoundCqCi[ibcqIndex]]);

    if (!pCqe->Good)
    {
        DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("[IO LOG] Inbound cq Fail: cqci:0x%X, ErrSts:0x%X\n", inBoundCqCi[ibcqIndex], pCqe->ErrSts));

        if (CHK_DFL_BUFF_ADDR(dflAddr))
        {
            RefillDFL(dflAddr);
        }

    }
    /*
    else
    {
        When tear down / queue delete, UCD may fetch host commands to DFL already, CPU0 will terminate these commands,
        refill DFL in IbCqRefillDFL(), and set the DFL address to 0xFFFF_FFFF.

        Just use this path to accumulate ci of IB CQ.
    }
    */
}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0InboundCompletionQueueHandler(uint8_t ibcqIndex)
{
    if(ibcqIndex >= IBCQ_END)
    {
        DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("InboundCompletionHandler: Invalid ibcqIndex: 0x%X\n", ibcqIndex),"32");
        return;
    }
    uint16_t qMask = (ibcqIndex == IBCQ_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
    uint32_t ibPi = readl(_ucdIbq.pHwIbCqPi[ibcqIndex]);
    inBoundCqCi[ibcqIndex]= readl(_ucdIbq.pHwIbCqCi[ibcqIndex]);

    while (ibPi != inBoundCqCi[ibcqIndex])
    {
        UcdCqEntry_t* pCqe = &((_ucdIbq.pIbCqe[ibcqIndex])[inBoundCqCi[ibcqIndex]]);
        uint32_t dflAddr = pCqe->AddrLow;

        if (unlikely(!pCqe->Good || !CHK_DFL_BUFF_ADDR(dflAddr)))
        {
            FpsCpu0CheckIBCqeValidAndDFLAddress(dflAddr, ibcqIndex);

            inBoundCqCi[ibcqIndex] = QUEUE_INC(inBoundCqCi[ibcqIndex], qMask);

            continue;
        }

        uint8_t ibPhyQId = pCqe->QPId;
        uint8_t slotSts = _pSlotFlagSts[ibPhyQId];
        if (unlikely((slotSts & (cStsTearDown | cStsDelete)) || !slotSts))
        {
            if ((_pIbQ2ObQ[ibPhyQId] == QID_INVALID) || (slotSts & (cStsTearDown | cStsDelete)))
            {
                RefillDFL(dflAddr);
                inBoundCqCi[ibcqIndex] = QUEUE_INC(inBoundCqCi[ibcqIndex], qMask); // update before ChkQisRunningSetSlotSts
                if ((slotSts & (cStsTearDown | cStsDelete)))
                {
                    uint8_t srcVfId = GET_VF_ID(pCqe->IFSel);
                    HandleDeleteVFQblk(ibPhyQId, srcVfId, slotSts);
                }

                continue;
            }
        }

        bool cmdSuccess = FpsCpu0FillCmdEntryAndSendToCpu1(ibcqIndex, qMask);
        if (!cmdSuccess)
        {
            break;
        }
    }

    writel(inBoundCqCi[ibcqIndex], _ucdIbq.pHwIbCqCi[ibcqIndex]);

}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0OutboundCompletionQueueHandler(uint8_t obcqIndex)
{
    if(obcqIndex >= OBCQ_END)
    {
        DebugLogLvDbgInfo(cLogCPU0Common, cLogError, ("OutboundCompletionHandler: Invalid obcqIndex: 0x%X\n", obcqIndex),"32");
        return;
    }
    uint16_t qMask = (obcqIndex == OBCQ_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
    uint32_t obCqPi = readl(_ucdObq.pHwObCqPi[obcqIndex]);
    outBoundCqCi[obcqIndex] = readl(_ucdObq.pHwObCqCi[obcqIndex]);

    while (obCqPi != outBoundCqCi[obcqIndex])
    {
        UcdCqEntry_t* pObCqe = &((_ucdObq.pCqEntries[obcqIndex])[outBoundCqCi[obcqIndex]]);
        uint16_t ceIndex = pObCqe->Tag;
        uint8_t caIndex = (ceIndex >> CA_SIZE_SHIFT);
        uint8_t ibPhyQId = pCa2IbPhysicalId[caIndex];
        uint8_t slotSts = _pSlotFlagSts[ibPhyQId];

        #ifndef DISABLE_IO_LOG
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogDebug, ("[IO LOG] Outbound cmd entry:                    sts:0x%X, obqid:0x%02X, ifsel:0x%02X, dfl idx:0x%03X, list num:0x%X\n", \
                    _pCmdEntryArray[ceIndex].Dw0), "4,8,8,10,2");
        #endif

        cmdArrayCi[caIndex] = QUEUE_INC(cmdArrayCi[caIndex], CA_ROLLOVER_MASK);

        if (likely(pObCqe->OB_Good))
        {
            #ifdef SUPPORT_TELEMETRY
            *pAccumulateIoCnt = (*pAccumulateIoCnt) + 1; // ensure success io count
            #endif
        }
        else
        {
            FpsCpu0ProcessObCqFailure(obcqIndex);
        }

        RefillDFL(pObCqe->Addr);

        #ifdef SUPPORT_TELEMETRY
        *pOutstandingIoCnt = (*pOutstandingIoCnt) - 1; // msut be minus after refill dfl
        #endif

        FpsCpu0CheckSlotStatusAndHandling(slotSts, ibPhyQId);

        outBoundCqCi[obcqIndex] = QUEUE_INC(outBoundCqCi[obcqIndex], qMask);
    }

    writel(outBoundCqCi[obcqIndex], _ucdObq.pHwObCqCi[obcqIndex]);
}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0TearDownRefillDFLHandler(void)
{
    /* process tear down - zero transfer but do not return cqe to host */
    uint32_t refillCePi = readl(pCeForRefillDFLQueuePi);
    while (refillCePi != ceForRefillDFLQueueCi)
    {
        uint16_t ceIndex = pCEforRefillDFLQueue[ceForRefillDFLQueueCi];
        uint8_t caIndex = ceIndex >> CA_SIZE_SHIFT;
        uint8_t ibPhyQId = pCa2IbPhysicalId[caIndex];

        FpsCpu0TearDownRefillDFL(ceIndex, caIndex, ibPhyQId);

        ceForRefillDFLQueueCi = QUEUE_INC(ceForRefillDFLQueueCi, 0x1ff);
        writel(ceForRefillDFLQueueCi, pCeForRefillDFLQueueCi);
    }
}

ATTR_ALWAYS_INLINE void fpsCpu0::FpsCpu0ProcessRetryCeHandler(void)
{
    uint32_t retryCePi = readl(pRetryCEQueuePi);

    while (retryCePi != retryCEQueueCi)
    {
        // release the original CE and get a new CE to retry it.
        // If there is no new CE, at least there must be an original CE index could be used.
        uint16_t orgCeIndex = pRetryCeIndexQueue[retryCEQueueCi];
        uint8_t caIndex = orgCeIndex >> CA_SIZE_SHIFT;
        uint16_t newCeIndex = (caIndex << CA_SIZE_SHIFT) + (cmdArrayPi[caIndex] & CA_MASK);
        uint8_t ibPhyQId = pCa2IbPhysicalId[caIndex];

        if (_pSlotFlagSts[ibPhyQId] & cStsTearDown)
        {
            FpsCpu0TearDownRefillDFL(orgCeIndex, caIndex, ibPhyQId);

            retryCEQueueCi = QUEUE_INC(retryCEQueueCi, 0x1ff);
            writel(retryCEQueueCi, pRetryCEQueueCi);

            continue;
        }

        bool retryFail = FpsCpu0RetryErrorCmd(orgCeIndex, newCeIndex, caIndex, ibPhyQId);
        if (retryFail)
        {
            return;
        }

        retryCEQueueCi = QUEUE_INC(retryCEQueueCi, 0x1ff);
        writel(retryCEQueueCi, pRetryCEQueueCi);

    }

}

void fpsCpu0::FpsCpu0FpCmdHandlerFiber(void* pObj)
{
    fpsCpu0* pThis = static_cast<fpsCpu0*>(pObj);

    pThis->FpsCpu0TearDownRefillDFLHandler();

    pThis->FpsCpu0ProcessRetryCeHandler();

    pThis->FpsCpu0InboundCompletionQueueHandler(IBCQ_0);
    pThis->FpsCpu0InboundCompletionQueueHandler(IBCQ_1);

    pThis->FpsCpu0OutboundCompletionQueueHandler(OBCQ_0);
    pThis->FpsCpu0OutboundCompletionQueueHandler(OBCQ_1);

    #ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
    uint32_t currentTimestamp = 0xFFFFFF - readl(&pThis->rCortexm7->systemControl.systCvr);
    currentTimestamp = (currentTimestamp + pThis->gTimerCounterCovert) & SYSTICK_MASK;
    gTimeStampBase = pThis->gTimerCounterCovert;

    if ((pThis->localTimeStamp & TimestampMaskBit23_14) != (currentTimestamp & TimestampMaskBit23_14)) // compare 10 bits [23:14]
    {
        // update local timestamp and record in log buffer
        pThis->localTimeStamp = currentTimestamp;

        // Note: //DebugLogLvDbgInfoInline's param logCategory cannot use variable, due to tokenize tool check the enum "text" but "value".
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogInfo, ("logging timestamp: 0x%x\n", pThis->localTimeStamp));
    }
    #endif
}
