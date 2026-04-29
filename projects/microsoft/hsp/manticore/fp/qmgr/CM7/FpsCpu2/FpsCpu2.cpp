// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2.cpp
//! @brief  FpsCpu2 Component Group
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2.h"
#include "MemIo.h"
#include "Version.h"
extern "C"
{
#include "crashdump.h"
#include "API_GCMTagCorrect.h"
#include "vicommon.h"
}
#if defined (SUPPORT_MSGERROR_INJECTION) || defined (SUPPORT_ERROR_INJECTION)
#include "FpsCpu2ErrorInjection.h"
#endif
#include "Heartbeat.h"

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------

Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
Ucd_t* rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
Cdma_t* rCdma = (Cdma_t*)CDMA_REG_ADDR;

#ifdef LOGGING_NEW_SCHEME
Gdma_t* rGdma = (Gdma_t*)GDMA_REG_ADDR;
#endif
// #define REG_FPS_INDIRECT_REG_WR_DISABLE   (uint32_t)&(rFps->fpsBank0RegRegisters.fpsBank0IndirectRegisterWriteDisableIndirectRegWriteFwdDisable)

//-----------------------------------------------------------------------------
//  Member Function Definitions
//-----------------------------------------------------------------------------

bool ChkFPMsgStsDone(CP2FPMsgSts sts)
{
    bool done = false;
    switch (sts)
    {
        case msgSuccess:
        case msgInvalidField:
        case msgVfInstalledAlready:   ///< msgQueueOutOfRange
        case msgNotSupport:
        case msgNotifyCpu1:
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

void fpsCpu2::FpsCpuNormalBootInitialize()
{
    #ifdef LIONPERF_SUPPORT
    // Clear the log buffers for Marvell Internal logging
    LoggingNormalBootInit();
    #endif

    outBoundOSLPi[OSL_0] = 0;
    outBoundOSLPi[OSL_1] = 0;

    localObCqPi[OBCQ_0] = 0;
    localObCqPi[OBCQ_1] = 0;
}

void fpsCpu2::CDMAInit()
{
    //Configure CDMA registers
    uint32_t list0DflAddr = getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU1_DFL_BUFF_ADDR);
    uint32_t list1DflAddr = getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_CDMA_LIST_1_DFL_BUFF_ADDR);
    uint64_t list2DflAddr = getpSRAMPhysicalAddress((uint32_t)PSRAM_CP_DFL_BUF_ADDR);
    uint32_t list3DflAddr = getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_DFL_1_BUFF_ADDR);

    HalCDMA_Reset();
    #ifdef SUPPORT_FPS_REGISTER
    API_CDMASqSetup((uint32_t)FPS_REG_CMDA_SQ_CI_PHY_ADDR);//(uint32_t)(&(rFpsReg->fpsFp2hweRegRegisters[cFp2HweWq04CdmaSq].fpsFp2hweFpToHweQCiShadow));
    #else
    API_CDMASqSetup(getCPU1TCMPhysicalAddress((uint32_t)M7_CDMA_SQ_CI_ADDR));
    #endif
    #ifdef SUPPORT_FPS_REGISTER
    API_CDMACqSetup((uint32_t)FPS_REG_CMDA_CQ_CI_PHY_ADDR);//(uint32_t)(&(rFpsReg->fpsHwe2fpRegRegister[cHwe2FpWq04CdmaCq].fpsHwe2fpHwEngineToFpQPiShadow));
    #else
    API_CDMACqSetup(getCPU2TCMPhysicalAddress((uint32_t)M7_CDMA_CQ_PI_ADDR));
    #endif
    API_CDMASetList(CDMA_FP_DFL_0_LIST, 0, list0DflAddr);
    API_CDMASetList(CDMA_FP_IDLE_DFL_LIST, 0, list1DflAddr);
    API_CDMASetList(CDMA_CP_DFL_LIST,  0,  list2DflAddr);
    API_CDMASetList(CDMA_FP_DFL_3_LIST, 0, list3DflAddr);
    // enable error checking register
    API_CDMASetCmdSlotErrorCheckEn(CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_0, CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_0_MASK);
    API_CDMASetCmdSlotErrorCheckEn(CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_1, CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_1_MASK);
    HalCDMA_SetInterruptEnable(CDMA_INTERRUPT_1, (CDMA_INT_CAUSE_FATAL_ERROR | CDMA_INT_CAUSE_KV_MEM_CORR_EXCEED_THRESHOLD_ERR | CDMA_INT_CAUSE_KV_MEM_UNCORRECTABLE_ECC_ERR));
    #ifdef SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
    API_CDMAMaxDescrElmntChkCfgEn(CDMA_REG_MAX_DESCR_ELMNT_COUNT_PER_CHUNK);
    #endif

    HalCDMA_SetGlobalCheckEnable((KV_MEM_ECC_ERR_CHECK_EN | AXI_WRITE_BRESP_CHECK_EN | AXI_READ_RRESP_CHECK_EN));

    HalCDMA_SetFatalErrorHaltEnable();

    _CDMACorrectableKeyErrorThreshold = CORRECTABLE_KEY_ERROR_THRESHOLD;

    HalCDMA_SetCorrKeyErrThreshold(_CDMACorrectableKeyErrorThreshold);

    API_CDMAInit(!_uncorrectableKeyErrorOccurred);
    if (_uncorrectableKeyErrorOccurred)
    {
        uint32_t errorCount = HalCDMA_GetCorrKeyErrCount(cCDMAUncorrKeyError);
        DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("CDMAInit: key vault mem uncorrectable err, errorCount:0x%X\n", errorCount), "32");
        _uncorrectableKeyErrorOccurred = false;
    }

}

void fpsCpu2::FpsCpuStart()
{
    #ifndef TDD
    CDMAInit();
    /* Disable indirect register in CPU state - FP_STS_FP_START
       Have to write physical hardware address from now on
     */
    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t data = 0;
    #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
    data |= (FP2HWE_Q_PI_04_WR_BIT | SOC_REG_0_WR_BIT | SOC_REG_1_WR_BIT | SOC_REG_2_WR_BIT | SOC_REG_3_WR_BIT);
    #else
    data |= (SOC_REG_0_WR_BIT | SOC_REG_1_WR_BIT | SOC_REG_2_WR_BIT | SOC_REG_3_WR_BIT);
    #endif
    writel(data, REG_FPS_INDIRECT_REG_WR_DISABLE);

    hwOslPiAddr[OSL_0] = (uint32_t)rFps->fpsSocFwdRegRegisters[cFpSocFwd01Ucd1Osl0].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr;
    hwOslPiAddr[OSL_1] = (uint32_t)rFps->fpsSocFwdRegRegisters[cFpSocFwd03Ucd1Osl1].fpsSocFwdSocIndirectAddressPortSocIndirectRegAddr;
    #endif

    _fpsCpu2ProcessCdmaCqOslFiber.Activate();
    #endif
}

void fpsCpu2::Initialize(M7CompGroupId_t compId)
{
    // Init fiber parameters
    InitializeFiber();
    RegisterComponentGroup(compId);
}

void fpsCpu2::RegisterComponentGroup(M7CompGroupId_t compId)
{
    // Register Component Group
    M7FiberScheduler_RegisterCompGroup(cM7Core2, compId,
                                       static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cNumberOfFibers),
                                       reinterpret_cast<OnM7FiberSchedulerInitializedFptr_t>(&fpsCpu2::RegisterFibers),
                                       static_cast<void*>(this));
}

void fpsCpu2::RegisterFibers(void* pObj)
{
    // Get the Component instance
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    pThis->_fpsCpu2ProcessCdmaCqOslFiber.Register(pThis, &fpsCpu2::FpsCpu2ProcessCdmaCqOslFiber, \
                                                  "FpsCpu2ProcessCdmaCqOslFiber", false);
    pThis->_fpsCpu2CP2FPServiceFiber.Register(pThis, &fpsCpu2::FpsCpu2CP2FPServiceFiber, "FpsCpu2CP2FPMsgFiber");
    pThis->_fpsCpu2RecvFpMsgFiber.Register(pThis, &fpsCpu2::FpsCpu2ReceiveFPMsgFiber, "FpsCpu2ReceiveFPMsgFiber");
    pThis->_fpsCpu2HandleCDMAFatalErrorFiber.Register(pThis, &fpsCpu2::FpsCpu2HandleCDMAFatalErrorFiber, \
                                                      "FpsCpu2HandleCDMAFatalErrorFiber");
    pThis->_fpsCpu2RecvFpMsgFiber.Wait();
    pThis->_fpsCpu2CP2FPServiceFiber.Wait();
    pThis->_fpsCpu2HandleCDMAFatalErrorFiber.Wait();
    pThis->_fpsCpu2HandleCDMAKeyCorrtableErrFiber.Register(pThis, &fpsCpu2::FpsCpu2HandleCDMAKeyCorrtableErrFiber, \
                                                           "FpsCpu2HandleCDMAKeyCorrtableErrFiber");
    pThis->_fpsCpu2HandleCDMAKeyCorrtableErrFiber.Wait();

    pThis->_fpsCpu2HandleResetFiber.Register(pThis, &fpsCpu2::FpsCpu2HandleResetFiber, \
                                             "FpsCpu2HandleResetFiber");
    pThis->_fpsCpu2HandleResetFiber.Wait();

    pThis->_fpsCpu2CheckHeartbeatFiber.Register(pThis, &CheckHeartbeatFiber, "FpsCpu2CheckHeartbeatFiber");
    pThis->_fpsCpu2CheckHeartbeatFiber.Activate();
}

void fpsCpu2::InitializeUCD()
{
    pIbCmnReg[UCD_CORE_0] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR);
    pIbCmnReg[UCD_CORE_1] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR + UCD_IBOB_CORE_OFFSET);
    pObCmnReg[UCD_CORE_0] = (UcdCore0ObCmnRegisters_t*)(UCD_OB_REGS_ADDR);
    pObCmnReg[UCD_CORE_1] = (UcdCore0ObCmnRegisters_t*)(UCD_OB_REGS_ADDR + UCD_IBOB_CORE_OFFSET);

    _ucdObq.pHwOslPi[OSL_0] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd01Ucd1Osl0].fpsSocFwdSocIndirectDataPortSocIndirectRegData);
    _ucdObq.pHwOslPi[OSL_1] = &(rFps->fpsSocFwdRegRegisters[cFpSocFwd03Ucd1Osl1].fpsSocFwdSocIndirectDataPortSocIndirectRegData);

    _ucdObq.pHwObCqPi[OBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQPiShadow.all);
    _ucdObq.pHwObCqPi[OBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQPiShadow.all);

    _ucdObq.pHwObCqCi[OBCQ_0] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq02UcdObCq0].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    _ucdObq.pHwObCqCi[OBCQ_1] = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq03UcdObCq1].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);

    _ucdObq.pOslEntries[OSL_0] = (UcdOslEntry_t*)(M7_FPS_CPU2_OSL_LIST_0_ADDR);
    _ucdObq.pOslEntries[OSL_1] = (UcdOslEntry_t*)(M7_FPS_CPU2_OSL_LIST_1_ADDR);

    M7_MEM_SET((void*)M7_FPS_CPU2_OSL_LIST_0_ADDR, 0, OB_OSL_0_SIZE);
    M7_MEM_SET((void*)M7_FPS_CPU2_OSL_LIST_1_ADDR, 0, OB_OSL_1_SIZE);
}

void fpsCpu2::InitializeErrorHandlingDataStructure()
{
    #ifdef CDMA_CMD_COUNT
    cdmaCmdSlotQueueCi = 0;
    rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueConsumerIndex.all = 0;
    pCdmaCmdSlotQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq00].fpsCpuxToCpuyQueueConsumerIndex.all);
    writel(0, pCdmaCmdSlotQueueCi);
    #endif // End of CDMA_CMD_COUNT

    rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq01].fpsCpuxToCpuyQueueSize.all = 0x7;   // 2^7 = 128
    cdmaSlotAbortQueuePi = 0;
    cdmaSlotAbortQueueCi = 0;
    pCdmaSlotAbortQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq01].fpsCpuxToCpuyQueueProducerIndex.all);
    pCdmaSlotAbortQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq01].fpsCpuxToCpuyQueueConsumerIndex.all);
    pCdmaSlotAbortQueue = (uint16_t*)((uint32_t)CPU2AccessCPU1TCMMem(M7_FPS_CPU12_CDMA_SLOT_ABORT_QUEUE));
    M7_MEM_SET((void*)pCdmaSlotAbortQueue, 0, (M7_ERR_INJECTION_BITMAP_SIZE + M7_ERR_INJECT_TOTAL_SIZE));
    writel(0, pCdmaSlotAbortQueuePi);
    writel(0, pCdmaSlotAbortQueueCi);

    retryCEQueuePi = 0;
    retryCEQueueCi = 0;
    pRetryCEQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq02].fpsCpuxToCpuyQueueProducerIndex.all);
    pRetryCEQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq02].fpsCpuxToCpuyQueueConsumerIndex.all);
    pRetryCeIndexQueue = (uint16_t*)(((uint32_t)CPU2AccessCPU0TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU20_RETRY_CE_QUEUE))));
    M7_MEM_SET((void*)pRetryCeIndexQueue, 0, M7_SHARE_RETRY_CMD_ENTRY_INFO_SIZE);
    writel(0, pRetryCEQueuePi);
    writel(0, pRetryCEQueueCi);

    ceForRefillDFLQueuePi = 0;
    ceForRefillDFLQueueCi = 0;
    pCeForRefillDFLQueuePi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq03].fpsCpuxToCpuyQueueProducerIndex.all);
    pCeForRefillDFLQueueCi = (volatile uint32_t*)&(rFps->fpsCpuxToCpuyRegRegisters[cCpuX2CpuYWq03].fpsCpuxToCpuyQueueConsumerIndex.all);
    pCEforRefillDFLQueue = (uint16_t*)(((uint32_t)CPU2AccessCPU0TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1(M7_FPS_CPU20_REFILL_CE_DFL_QUEUE))));
    M7_MEM_SET((void*)pCEforRefillDFLQueue, 0, M7_SHARE_REFILL_CE_DFL_ENTRY_INFO_SIZE);
    writel(0, pCeForRefillDFLQueuePi);
    writel(0, pCeForRefillDFLQueueCi);
    #ifdef MCR_TEST_HOOKS
    level1AbortFlag = false;
    #endif
}

void fpsCpu2::InitializeCDMA()
{

    // CDMA CQ initialize
    _cdmaCq = {0};
    _cdmaCq.pCdmaCqBase = (CdmaCqCmdDescr_t*)(M7_FPS_CPU2_CDMA_CQ_ENTRY_ADDR);
    M7_MEM_SET((void*)M7_FPS_CPU2_CDMA_CQ_ENTRY_ADDR, 0, CDMA_CQ_SIZE);
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq04CdmaCq].fpsHwe2fpHwEngineToFpQSize.all = 6;
    rFps->fpsHwe2fpRegRegister[cHwe2FpWq04CdmaCq].fpsHwe2fpHwEngineToFpQCiIndirectAddressPortHwe2fpQCiIndirectRegAddr = \
        (uint32_t)(&(rCdma->completionQueue0ConsumerIndex.all));
    _cdmaCq.pHwCi = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq04CdmaCq].fpsHwe2fpHwEngineToFpQCiIndirectDataPort.all);
    _cdmaCq.pHwPi = &(rFps->fpsHwe2fpRegRegister[cHwe2FpWq04CdmaCq].fpsHwe2fpHwEngineToFpQPiShadow.all);
    _cdmaCq.pHwStatus = &(rFps->fpsBank0RegRegisters.fpsBank0EventStatus0.all);
    cdmaCqCi = 0;
    writel(0, _cdmaCq.pHwPi);
    writel(0, _cdmaCq.pHwCi);
}
void fpsCpu2::InitializeMessageQ()
{
    CP0toFPReqMsg.pMsgPi = (volatile uint32_t*)PSRAM_CP0toFP_REQ_PI_ADDR;
    CP0toFPReqMsg.pMsgCi = (volatile uint32_t*)PSRAM_CP0toFP_REQ_CI_ADDR;
    CP0toFPReqMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_CP0toFP_REQ_MSG_ADDR;

    FPtoCP0ResMsg.pMsgPi = (volatile uint32_t*)PSRAM_FPtoCP0_RES_PI_ADDR;
    FPtoCP0ResMsg.pMsgCi = (volatile uint32_t*)PSRAM_FPtoCP0_RES_CI_ADDR;
    FPtoCP0ResMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_FPtoCP0_RES_MSG_ADDR;

    CP1toFPReqMsg.pMsgPi = (volatile uint32_t*)PSRAM_CP1toFP_REQ_PI_ADDR;
    CP1toFPReqMsg.pMsgCi = (volatile uint32_t*)PSRAM_CP1toFP_REQ_CI_ADDR;
    CP1toFPReqMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_CP1toFP_REQ_MSG_ADDR;

    FPtoCP1ResMsg.pMsgPi = (volatile uint32_t*)PSRAM_FPtoCP1_RES_PI_ADDR;
    FPtoCP1ResMsg.pMsgCi = (volatile uint32_t*)PSRAM_FPtoCP1_RES_CI_ADDR;
    FPtoCP1ResMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_FPtoCP1_RES_MSG_ADDR;

    FPtoCP0ReqMsg.pMsgPi = (volatile uint32_t*)PSRAM_FPtoCP0_REQ_PI_ADDR;
    FPtoCP0ReqMsg.pMsgCi = (volatile uint32_t*)PSRAM_FPtoCP0_REQ_CI_ADDR;
    FPtoCP0ReqMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_FPtoCP0_REQ_MSG_ADDR;

    CP0toFPResMsg.pMsgPi = (volatile uint32_t*)PSRAM_CP0toFP_RES_PI_ADDR;
    CP0toFPResMsg.pMsgCi = (volatile uint32_t*)PSRAM_CP0toFP_RES_CI_ADDR;
    CP0toFPResMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_CP0toFP_RES_MSG_ADDR;

    FPtoCP1ReqMsg.pMsgPi = (volatile uint32_t*)PSRAM_FPtoCP1_REQ_PI_ADDR;
    FPtoCP1ReqMsg.pMsgCi = (volatile uint32_t*)PSRAM_FPtoCP1_REQ_CI_ADDR;
    FPtoCP1ReqMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_FPtoCP1_REQ_MSG_ADDR;

    CP1toFPResMsg.pMsgPi = (volatile uint32_t*)PSRAM_CP1toFP_RES_PI_ADDR;
    CP1toFPResMsg.pMsgCi = (volatile uint32_t*)PSRAM_CP1toFP_RES_CI_ADDR;
    CP1toFPResMsg.pMsgQ = (CP2FPMsgContext_t*)PSRAM_CP1toFP_RES_MSG_ADDR;

    pCPU2toCPU0Pi = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU0_PI_ADDR);
    pCPU2toCPU0Ci = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU0_CI_ADDR);
    pCPU2toCPU0MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU2_2_CPU0_MSG_ADDR);

    pCPU0toCPU2Pi = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU2_PI_ADDR);
    pCPU0toCPU2Ci = (volatile uint32_t*)(PSRAM_INTL_CPU0_2_CPU2_CI_ADDR);
    pCPU0toCPU2MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU0_2_CPU2_MSG_ADDR);

    pCPU2toCPU1Pi = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU1_PI_ADDR);
    pCPU2toCPU1Ci = (volatile uint32_t*)(PSRAM_INTL_CPU2_2_CPU1_CI_ADDR);
    pCPU2toCPU1MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU2_2_CPU1_MSG_ADDR);

    pCPU1toCPU2Pi = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU2_PI_ADDR);
    pCPU1toCPU2Ci = (volatile uint32_t*)(PSRAM_INTL_CPU1_2_CPU2_CI_ADDR);
    pCPU1toCPU2MsgQ = (FPInterMsgHeader*)(PSRAM_INTL_CPU1_2_CPU2_MSG_ADDR);

    M7_MEM_SET((void*)CP0toFPReqMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)CP0toFPReqMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)FPtoCP0ResMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)FPtoCP0ResMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)CP1toFPReqMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)CP1toFPReqMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)FPtoCP1ResMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)FPtoCP1ResMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)FPtoCP0ReqMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)FPtoCP0ReqMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)CP0toFPResMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)CP0toFPResMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)FPtoCP1ReqMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)FPtoCP1ReqMsg.pMsgCi, 0, 4);

    M7_MEM_SET((void*)CP1toFPResMsg.pMsgPi, 0, 4);
    M7_MEM_SET((void*)CP1toFPResMsg.pMsgCi, 0, 4);

    CP0toFPReqMsg.localMsgPi = 0;
    CP0toFPReqMsg.localMsgCi = 0;
    FPtoCP0ResMsg.localMsgPi = 0;

    CP1toFPReqMsg.localMsgPi = 0;
    CP1toFPReqMsg.localMsgCi = 0;
    FPtoCP1ResMsg.localMsgPi = 0;

    CP0toFPResMsg.localMsgPi = 0;
    CP0toFPResMsg.localMsgCi = 0;
    FPtoCP0ReqMsg.localMsgPi = 0;

    CP1toFPResMsg.localMsgPi = 0;
    CP1toFPResMsg.localMsgCi = 0;
    FPtoCP1ReqMsg.localMsgPi = 0;

    M7_MEM_SET((void*)pCPU2toCPU0Pi, 0, 4);
    M7_MEM_SET((void*)pCPU0toCPU2Ci, 0, 4);

    M7_MEM_SET((void*)pCPU2toCPU1Pi, 0, 4);
    M7_MEM_SET((void*)pCPU1toCPU2Ci, 0, 4);


    CPU2toCPU0Pi = 0;
    CPU0toCPU2Ci = 0;

    CPU2toCPU1Pi = 0;
    CPU1toCPU2Ci = 0;
    for (uint8_t i = 0; i < PSRAM_CP2FP_MSG_DEPTH; i++)
    {
        CP0toFPReqMsg.msgState[i] = MSG_STATE_START;
        CP1toFPReqMsg.msgState[i] = MSG_STATE_START;
        CP0toFPReqMsg.msgBitmap[i].bitMap = 0;
        CP1toFPReqMsg.msgBitmap[i].bitMap = 0;
    }

    for(uint8_t i = 0; i < PSRAM_CP2FP_REQ_RES_MSG_DEPTH; i++)
    {
        CP0toFPResMsg.msgState[i] = MSG_STATE_START;
        CP0toFPResMsg.msgBitmap[i].bitMap = 0;

        CP1toFPResMsg.msgState[i] = MSG_STATE_START;
        CP1toFPResMsg.msgBitmap[i].bitMap = 0;
    }

    pfCpu2MsgTable[msgOpFpStsChange] = &fpsCpu2::MsgHandleFpStsChange;

    pfCpu2MsgTable[msgOpErrQSet] = &fpsCpu2::MsgHandleErrQset;
    #ifdef LIONPERF_SUPPORT
    pfCpu2MsgTable[msgOpFpModeChange] = &fpsCpu2::MsgHandleFpModeChange;
    #endif
    pfCpu2MsgTable[msgOpVfSlotSQ2CQMapUpdate] = &fpsCpu2::MsgHandleFpVfSlotSq2CqMapUpdate;
    #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
    pfCpu2MsgTable[msgOpUpdateTimestampAddr] = &fpsCpu2::MsgHandleUpdateTimestampAddr;
    #else
    pfCpu2MsgTable[msgOpUpdateTimestampAddr] = &fpsCpu2::MsgNotSupport;
    #endif
    pfCpu2MsgTable[msgOpVfUpdate] = &fpsCpu2::MsgHandleFpVfUpdate;
    pfCpu2MsgTable[msgOpCpCdmaIo] = &fpsCpu2::MsgHandleCpCdmaIo;
    pfCpu2MsgTable[msgOpKeyUpdate] = &fpsCpu2::MsgHandleKeyUpdate;
    pfCpu2MsgTable[msgOpUcdQuery] = &fpsCpu2::MsgHandleFpUcdQuery;
    #ifdef LOGGING_NEW_SCHEME
    pfCpu2MsgTable[msgOpTelemetryQuery] = &fpsCpu2::MsgHandleTelemetryQuery;
    #ifdef LIONPERF_SUPPORT
    pfCpu2MsgTable[msgOpSetLogLevel] = &fpsCpu2::MsgHandleSetLogLevel;
    pfCpu2MsgTable[msgOpLogEnDisUpdate] = &fpsCpu2::MsgHandleLogEnDisUpdate;
    #endif
    #endif
    #ifdef LIONPERF_SUPPORT
    pfCpu2MsgTable[msgOpCDMAStatSet] = &fpsCpu2::MsgHandleOpCDMAStatSet;
    #endif
    #ifdef QOS_LATENCY_ERROR_HANDLING
    pfCpu2MsgTable[msgOpQoSPenalty] = &fpsCpu2::MsgHandleOpQoSPenaltySetup;
    #endif

    // No list in CP2FP or FP2CP Message OpCode Definition Table
    #ifdef SUPPORT_MSGERROR_INJECTION
    pfCpu2MsgTable[msgOpMsgErrorInjection] = &fpsCpu2::MsgHandleMsgErrorInjection;
    #endif
    #ifdef SUPPORT_CDMA_RESET_MSG
    pfCpu2MsgTable[msgOpCDMAReset] = &fpsCpu2::HandleOpCDMAReset;
    #endif
    pfCpu2MsgTable[msgOpShutdownReq] = &fpsCpu2::MsgHandleFpShutdownRequest;
    #ifdef MCR_TEST_HOOKS
    pfCpu2MsgTable[msgOpInjectErrorReq] = &fpsCpu2::MsgHandleFpInjectErrorRequest;
    #endif
}

void fpsCpu2::InitializeIOResource()
{
    // CE and CE Tiny initialize
    _pCmdEntryArrayBase = (CmdEntry_t*)(CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU01_CMD_ARRAY_BASE_ADDR));   //via CPU0
    _pCmdEntryArrayTinyBase = (CmdEntryTiny_t*)(M7_FPS_CPU20_CMD_ARRAY_TINY_BASE_ADDR);

    // The mapping of inbound to outbound physical queue
    _pIbQ2ObQ = (uint8_t*)(M7_FPS_CPU20_IBQ2OBQ_ADDR);
    pQAbortBitmap = (uint8_t*)(CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_ABORT_BITMAP_ADDR));
    CPU1SubmitAbortInfo = (uint8_t*)(CPU2_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_M7_CPU1_SUBMIT_ABORT_INFO));
    pCDMAIOAbortBit = (uint8_t*)(CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_CDMA_IO_ABORT_BIT_ADDR));
    _pSlotFlagSts = (uint8_t*)CPU2AccessCPU0TCMMem(GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU20_SLOT_STATUS_ADDR));
    _pVfInfoBase = (VFNodeInfo_t*)CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU01_VF_INFO_BASE);
    _pQueueBlockInfoBase = (QueueBlockInfo_t*)CPU2AccessCPU0TCMMem((uint32_t)M7_FPS_CPU01_QB_INFO_BASE);
    _pVFEnBitmap = (uint64_t*)(CPU2_ACCESS_SHARE_TCM01_FROM_CPU1((uint32_t)M7_FPS_CPU01_VF_ENABLE_BIT_MAP_ADDR));
    _pVF65EnBitmap = (uint32_t*)(CPU2_ACCESS_SHARE_TCM01_FROM_CPU1(M7_FPS_CPU01_VF_65_ENABLE_BIT_MAP_ADDR));
    // _VfTeardownBitmap = 0;
    // _Vf65TeardwonBitmap = 0;

    //Service Indicator initialize
    _pServiceIndicator = (uint8_t*)(M7_FPS_CPU20_FIPS_APPROVAL_STS_ARRAY);
}

void fpsCpu2::InitializeFiber()
{
    pCpuStatus = (uint32_t*)(PSRAM_FP_CPU2_STATUS_ADDR);
    writel(FP_STS_INIT_START, pCpuStatus);

    // Init fiber parameters fpsCpu2ProcessCdmaCqOslFiber
    M7FiberParameters_t fiberParamsFpsCpu2ProcessCdmaCqOslFiber = {0};
    fiberParamsFpsCpu2ProcessCdmaCqOslFiber.fiberWeight = cFiberWeightFpsCpu2ProcessCdmaCqOslFiber;

    // Initialize Fiber object fpsCpu2ProcessCdmaCqOslFiber
    _fpsCpu2ProcessCdmaCqOslFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                             static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2ProcessCdmaCqOslFiberId), fiberParamsFpsCpu2ProcessCdmaCqOslFiber);
    //Init fiber parameters CP2FP
    M7FiberParameters_t fiberParamsFpsCpu2CP2FP = {0};
    fiberParamsFpsCpu2CP2FP.fiberWeight = cFiberWeightFpsCpu2CP2FPServiceFiber; //??

    // Initialize Fiber object CP2FP
    _fpsCpu2CP2FPServiceFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                         static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2CP2FPServiceFiberId), fiberParamsFpsCpu2CP2FP);

    //Init fiber parameters object RecvFpMsg
    M7FiberParameters_t fiberParamsRecvFpMsg = {0};
    fiberParamsRecvFpMsg.fiberWeight = cFiberWeightFpsCpu2RecvFPMsgFiber;

    // Initialize Fiber object RecvFpMsg
    _fpsCpu2RecvFpMsgFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                      static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2RecvFpMsgFiberId), fiberParamsRecvFpMsg);

    M7FiberParameters_t fiberParamsHandleCDMAFatalError = {0};
    fiberParamsHandleCDMAFatalError.fiberWeight = cFiberWeightFpsCpu2HandleCDMAFatalErrorFiber;
    _fpsCpu2HandleCDMAFatalErrorFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                                 static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2HandleCDMAFatalErrorFiberId), fiberParamsHandleCDMAFatalError);

    M7FiberParameters_t fiberParamsCorrtableKeyErr = {0};
    fiberParamsCorrtableKeyErr.fiberWeight = cFiberWeightFpsCpu2HandleCDMAKeyCorrtableErrFiber;

    // Initialize Fiber object _fpsCpu2AliveCheckFiber
    _fpsCpu2HandleCDMAKeyCorrtableErrFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                                      static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2HandleCDMAKeyCorrtableErrFiberId), fiberParamsCorrtableKeyErr);

    M7FiberParameters_t fiberParamsReset = {0};
    fiberParamsReset.fiberWeight = cFiberWeightFpsCpu2HandleResetFiber;

    // Initialize Fiber object _fpsCpu2AliveCheckFiber
    _fpsCpu2HandleResetFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                        static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2HandleResetFiberId), fiberParamsReset);

    //Init fiber parameters object CheckHeartbeat
    M7FiberParameters_t fiberParamsCheckHeartbeat = {0};
    fiberParamsCheckHeartbeat.fiberWeight = cFiberWeightCheckHeartbeatFiber;

    // Initialize Fiber object CheckHeartbeat
    _fpsCpu2CheckHeartbeatFiber.Initialize(cM7Core2, cM7CompGroupIo, \
                                      static_cast<M7FiberId_t>(fpsCpu2FiberId_t::cFpsCpu2CheckHeartbeatFiberId), fiberParamsCheckHeartbeat);

    rFps = (Fps_t*)FPS_REG_ADDR;
    rUcd = (Ucd_t*)UCD_CPU_CREG_PF_ADDR_START;
    rCdma = (Cdma_t*)CDMA_REG_ADDR;
    rTcon = (Tcon_t*)TCON_REG_ADDR;
    rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;

    _pIbCmnReg[UCD_CORE_0] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR);
    _pIbCmnReg[UCD_CORE_1] = (UcdCore0IbCmnRegisters_t*)(UCD_IB_REGS_ADDR + UCD_IBOB_CORE_OFFSET);

    InitializeUCD();
    InitializeErrorHandlingDataStructure();
    InitializeCDMA();
    InitializeIOResource();
    InitializeMessageQ();


    #ifdef SUPPORT_MSGERROR_INJECTION
    errInjectCnt = 0;
    totalErrInjectCnt = 0;
    pErrInjectBitmap = (uint64_t*)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU12_ERR_INJECTION_BITMAP);
    pMsgErrorInjection = (CP2FPMsgDataMsgErrorInjection_t*)GET_FPS_BANK2_ADDR_FROM_BANK1((uint32_t)M7_FPS_CPU12_ERR_INJECTION_START);
    #endif

    // CP CDMA IO
    CPCDMAIODone = 0;
    CPCDMAIOStatus = 0;

    #ifdef SUPPORT_TELEMETRY
    // Telemetry counter
    TcInjecCountNonFatalErr = 0;
    TcIInjecCountFatalErr = 0;
    TcInjecCountPoorSgl = 0;
    TcFaultErrCnt = 0;
    TcNonFaultErrCnt = 0;
    TcPoorConstructedSglCnt = 0;
    #endif

    #ifdef SUPPORT_CDMA_RESET_MSG
    duringCDMAResetMessage = false;
    #endif

    _cdmaCorrectableKeyErrorHandleState = cCorrtableErrorhandlingWait;
    _cdmaFatalErrorHandleState = cFatalErrorHandlingWait;
    _curResetRequest = noRequest;
    _cdmaFatalErrorFlag = (uint32_t*)((uint32_t)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_CDMA_FATAL_ERROR_OCCUR_FLAG));
    _uncorrectableKeyErrorOccurred = false;
    pFLRRequestBitMapLocal = (uint64_t*)((uint32_t)M7_FPS_CPU20_FLR_REQUSET_BIT_MAP_LOCAL);

    pFLRQueueBlockMap = (uint64_t*)((uint32_t)M7_FPS_CPU20_FLR_QUEUE_BLOCK_BIT_MAP_LOCAL);
    *pFLRQueueBlockMap = 0;
    pFLRQueueBlock65Map = (uint8_t*)((uint32_t)M7_FPS_CPU20_FLR_QUEUE_BLOCK_65_BIT_MAP_LOCAL);
    *pFLRQueueBlock65Map = 0;

    // admin abort
    adminAbortCount = (uint8_t*)M7_FPS_CPU20_ADMIN_ABORT_COUNT_ADDR;
    M7_MEM_SET((void*)M7_FPS_CPU20_ADMIN_ABORT_COUNT_ADDR, 0, M7_FPS_CPU20_ADMIN_ABORT_COUNT_SIZE);

    pCa2IbPhysicalId = (uint8_t*)M7_FPS_CPU20_CA_2_IBPHYQID_TABLE;
    pIbPhysicalId2Ca = (uint8_t*)M7_FPS_CPU20_IBPHYQID_2_CA_TABLE;
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

    #ifdef SUPPORT_VF65_QB65_UCD_QUERY_TWICE
    init_ucd_parameters();
    #endif // SUPPORT_VF65_QB65_UCD_QUERY_TWICE

    while ((readl(PSRAM_FP_CPU1_STATUS_ADDR) != FP_STS_INIT_DONE) || (readl(PSRAM_FP_CPU0_STATUS_ADDR) != FP_STS_INIT_DONE))
    {
        //Debug_Log(cLogMonitor, cLogInfo, ("FP cpu 2 wait cpu0 status %x, cpu1 status %x\n", readl(PSRAM_FP_CPU0_STATUS_ADDR), readl(PSRAM_FP_CPU1_STATUS_ADDR)));
    }
    writel(0, _cdmaCq.pHwPi);
    writel(0, _cdmaCq.pHwCi);

    #ifdef SUPPORT_UPDATE_TIMESTAMP
    gTimerCounterBase = 0;
    gTimerCounterLast = 0;
    gTimerCounterDelta = 0;
    gTimerCounterCovert = 0;
    gTimerCounterCount = 0;
    gTimeSyncDone = 0;
    #endif

    #ifdef QOS_LATENCY_ERROR_HANDLING
    _pQosPenalty = (CP2FPMsgDataQoSPenalty_t*)((uint32_t)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_PENALTY_ADDR));
    M7_MEM_SET((void*)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_PENALTY_ADDR), 0, M7_QOS_PENALTY_SIZE);
    for (uint8_t i = 0; i < MAX_SUPPORT_FUNC_NUM; i++)
    {
        _pQosPenalty[i].Cfg.qosPenaltyCreditRatio = DEFAULT_QOS_CREDIT_RATIO;
    }
    _pQosVFBitmap[VF0_VF31] = (uint32_t*)((uint32_t)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_VF_BITMAP_ADDR));
    M7_MEM_SET((void*)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_VF_BITMAP_ADDR), 0, M7_SHARE_QOS_VF_BITMAP_SIZE);
    _pQosVFBitmap[VF32_VF63] = _pQosVFBitmap[VF0_VF31] + 1;
    _qosPenaltyVfBitmap = 0;
    _pQosVFBitmap[VF64] = (uint32_t*)((uint32_t)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_VF_65_BITMAP_ADDR));
    M7_MEM_SET((void*)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_QOS_VF_65_BITMAP_ADDR), 0, M7_SHARE_QOS_VF_65_BITMAP_SIZE);
    _qosPenaltyVf65Bitmap = 0;
    #endif

    #ifdef WEIGHT_ROUND_ROBIN
    _pWeightRoundRobin = (uint32_t*)((uint32_t)CPU2AccessCPU1TCMMem((uint32_t)M7_FPS_CPU12_WEIGHT_ROUND_ROBIN_ADDR));
    *_pWeightRoundRobin = DEFAULT_WEIGHT;
    #endif

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
            //DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("Firmware update failed, FwChecksumSts:0x%X FwUpdateSts:0x%X\n", (FwChecksumSts, FwUpdateSts)), "16,16");
        }
    }
    #endif

    if(gResetType==cPor)
    {
        M7_MEM_SET((void*)M7_FPS_CPU20_FIPS_APPROVAL_STS_ARRAY, 0, M7_FPS_CPU20_FIPS_APPROVAL_STS_ARRAY_SIZE);
    }

    writel(FP_STS_INIT_DONE, pCpuStatus);
}

bool fpsCpu2::FpsCpu2ReturnErrorCommandToHost(LionFPCQEStatusCode cqeStatus, LionFPCQEErrorCode cqeError)
{
    bool returnHost = TRUE;
    CdmaCqCmdDescr_t* pCdmaCqe = &(_cdmaCq.pCdmaCqBase[cdmaCqCi]);
    uint8_t oslIndex = (pCdmaCqe->Dw0.DflNum == CDMA_FP_DFL_0_LIST) ? OSL_0 : OSL_1;
    uint16_t qMask = (oslIndex == OSL_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;

    if (QUEUE_FULL(outBoundOSLPi[oslIndex], localObCqPi[oslIndex], qMask))
    {
        localObCqPi[oslIndex] = readl(_ucdObq.pHwObCqPi[oslIndex]);
        if (QUEUE_FULL(outBoundOSLPi[oslIndex], localObCqPi[oslIndex], qMask))
        {
            returnHost = FALSE;
        }
    }

    uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCdmaCqe->Dw0.DflNum, (pCdmaCqe->Dw0.CmdDflIdx << DFL_BUF_SZ_SHIFT));
    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));

    pFpCmd->cqe.StsCode = cqeStatus;
    pFpCmd->cqe.ErrCode = cqeError;

    pFpCmd->cqe.SqId = pCdmaCqe->Dw1.UcdIqId;

    return returnHost;

}

bool fpsCpu2::FpsCpu2ReturnCPRespErrCmdToHost(void* pObj, uint16_t ceIndex, LionFPCQEErrorCode cqeError)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    CmdEntry_t* pCe = pThis->_pCmdEntryArrayBase + ceIndex;
    uint8_t oslIndex = (pCe->cdmaListNum == CDMA_FP_DFL_0_LIST) ? OSL_0 : OSL_1;
    uint16_t qMask = (oslIndex == OSL_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;

    uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCe->cdmaListNum, (pCe->DFLIdx << DFL_BUF_SZ_SHIFT));
    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));

    pFpCmd->cqe.StsCode = CQE_SC_INVALID_FIELD_GCM;
    pFpCmd->cqe.ErrCode = cqeError;

    pFpCmd->cqe.SqId = pCe->PhyIbqId;
}

void fpsCpu2::FpsCpu2ProcessCdmaCqOslFiber(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    uint8_t* _pKeyFlag; //KeyFlag
    uint32_t cdmaHwCiAddr = (uint32_t)pThis->_cdmaCq.pHwCi;
    uint32_t cdmaCqPi = readl(pThis->_cdmaCq.pHwPi);
    pThis->cdmaCqCi = readl(pThis->_cdmaCq.pHwCi);

    pThis->FpsCpu2ProcessGcmCmdFiber(pThis);

    while (cdmaCqPi != pThis->cdmaCqCi)
    {
        CdmaCqCmdDescr_t* pCdmaCqe = &(pThis->_cdmaCq.pCdmaCqBase[pThis->cdmaCqCi]);
        uint8_t cdmaListNum = pCdmaCqe->Dw0.DflNum;
        uint8_t oslIndex = (cdmaListNum == CDMA_FP_DFL_0_LIST) ? OSL_0 : OSL_1;
        uint16_t qMask = (oslIndex == OSL_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
        uint32_t oslPi = pThis->outBoundOSLPi[oslIndex];
        uint8_t cmdStatus = pCdmaCqe->Dw0.CmdStatus;
        // AES-GCM Tag correction flags
        bool aesGcmSendToCP = false;
        bool aesGcmTagInvalid = false;

        #ifdef LIONPERF_SUPPORT
        /* Found admin abort host command in CPU0 / CPU1. Mark the status in corresponding msg,
           MsgErrQSetSubOpAdminAbort() will send the result to CP
         */
        if (unlikely(pThis->_pCmdEntryArrayTinyBase[pCdmaCqe->Dw0.CmdId].abortStatus == cCETinyAdminAbort))
        {
            pThis->FpsCpu2SearchAbortMsgInMsgQueue();
        }
        #endif
        uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCdmaCqe->Dw0.DflNum, (pCdmaCqe->Dw0.CmdDflIdx << DFL_BUF_SZ_SHIFT));
        switch (cdmaListNum)
        {
            case CDMA_FP_DFL_0_LIST:
            case CDMA_FP_DFL_3_LIST:
            {
                if (cmdStatus & CDMA_CMD_COMPLETED)
                {
                    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));
                    pFpCmd->cqe.DataLen = pFpCmd->cqe.DW5;
                    pFpCmd->cqe.DW14 = pFpCmd->cqe.DW6;
                    pFpCmd->cqe.DW15 = pFpCmd->cqe.DW7;

                    // Clear all the remaining Dwords
                    pFpCmd->cqe.DW5 = 0;
                    pFpCmd->cqe.DW6 = 0;
                    pFpCmd->cqe.DW7 = 0;
                    pFpCmd->cqe.DW8 = 0;
                    pFpCmd->cqe.DW9 = 0;
                    pFpCmd->cqe.DW10 = 0;
                    pFpCmd->cqe.DW11 = 0;

                    uint8_t phyQId = pCdmaCqe->Dw1.UcdIqId;
                    uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;

                    /* When CDMA cqe complete properly,
                       if slot status is tear down ==>  Ask CPU0 to refill DFL, do not return CQE to host, complete the command internally.
                       if slot status is delete Q  ==>  process it as normal flow, return CQE to host.
                     */
                    if (unlikely(pThis->_pSlotFlagSts[phyQId] & cStsTearDown))
                    {
                        pThis->FpsCpu2SendRefillDFLRequestToCpu0(ceIndex);

                        #ifdef CDMA_CMD_COUNT
                        pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                        writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
                        #endif

                        pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                        writel(pThis->cdmaCqCi, cdmaHwCiAddr);

                        continue;
                    }
                    //Moving it under LIONPERF_SUPPORT (disabled code) as we never hit this case
                    #ifdef LIONPERF_SUPPORT
                    /*
                       admin abort command cdma cqe may either zerotransfer or normal complete,
                       return CQE to host with CQE_SC_ABORT_REQ
                     */
                    if (unlikely(pThis->_pCmdEntryArrayTinyBase[ceIndex].abortStatus == cCETinyAdminAbort))
                    {
                        bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(CQE_SC_ABORT_REQ, CQE_DEFAULT_ERROR_CODE);
                        if (returnHost)
                        {
                            break;  // return CQE to host
                        }
                        else
                        {
                            return;
                        }
                    }
                    #endif
                    if (unlikely(pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus == cCETinyStsZeroXfer))
                    {
                        LionFPCQEStatusCode cqeStatus = FpsCpu2FillHostStatusCode(pThis->_pCmdEntryArrayBase[ceIndex].Status);
                        LionFPCQEErrorCode cqeError =  (LionFPCQEErrorCode)pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode;

                        bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(cqeStatus, cqeError);

                        if (returnHost)
                        {
                            break;  // return CQE to host
                        }
                        else
                        {
                            return;
                        }
                    }

                    // check if OSL is full ,  have to think about if abort handling return here, but change some variable?  -- Dirk
                    if (QUEUE_FULL(oslPi, pThis->localObCqPi[oslIndex], qMask))
                    {
                        pThis->localObCqPi[oslIndex] = readl(pThis->_ucdObq.pHwObCqPi[oslIndex]);
                        if (QUEUE_FULL(oslPi, pThis->localObCqPi[oslIndex], qMask))
                        {
                            return;
                        }
                    }

                    #ifndef DISABLE_IO_LOG
                    //DebugLogLvDbgInfo(cLogCPU2Common, cLogDebug, ("[IO LOG] CDMA CQE dw0:  ce id:0x%03X, obqid:0x%02X, dfl idx:0x%03X, list num:0x%X\n", \
                                                                   ((pCdmaCqe->dw0) & 0xFFF003FFUL) | (((uint32_t)pThis->_pIbQ2ObQ[phyQId]) << 0xAUL)), "10,10,10,2");
                    #endif

                    if(!(pFpCmd->cqe.cipher))
                    {
                        if((pFpCmd->meta.AesGcmCmd.UnpaddedAADLen < pFpCmd->meta.AesGcmCmd.AADLen || pFpCmd->meta.AesGcmCmd.UnalignedSrcDataLen || !pFpCmd->sqe.SrcDataLen))
                        {
                            aesGcmSendToCP = true;
                            aesGcmTagInvalid = false;

                            if((!(pFpCmd->sqe.SrcDataLen) && pFpCmd->meta.AesGcmCmd.UnalignedSrcDataLen) || 
                            ((pFpCmd->meta.AesGcmCmd.AADLen == pFpCmd->sqe.SrcDataLen) && !(pFpCmd->meta.AesGcmCmd.UnpaddedAADLen)))
                            {
                                aesGcmTagInvalid = true; //No AAD and No Aligned data available
                            }
                        }
                    }

                    break;     // return CQE to host

                }
                else
                {
                    /* send the CPU-id information to FP core 1 to process abort commands, free the error slot of CDMA
                       Need not to care delete queue and tear down in this phase, will take care them when abort complete
                     */

                    if (cmdStatus & CDMA_CMD_ERROR)
                    {
                        pThis->Cpu2CdmaErrorCmdHandler();
                        uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;
                        // DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CDMA_CMD_ERROR Ceindex:%x", ceIndex), "32");
                        // DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CDMA slot err Status0: %x, Status1:%x", readl(0xA0C00140),readl(0xA0C00144)), "32", "32");

                        pThis->FpsCpu2SendCDMAAbortRequestToCpu1(pCdmaCqe->Dw0.CmdId);
                        pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                        writel(pThis->cdmaCqCi, cdmaHwCiAddr);
                        continue;

                    }

                    /* The handling after CDMA free the error slot.
                       In this phase, have to determine if the slot is in tear down, delete or admin abort status first.
                       If it does, just complete the command, do not execute any retry.

                       For non-fatal error and poor SGL, send the retry request to CPU 0.
                     */
                    if (cmdStatus & CDMA_CMD_ABORTED)
                    {
                        uint8_t phyQId = pCdmaCqe->Dw1.UcdIqId;
                        uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;

                        LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));
                        pFpCmd->cqe.DataLen = pFpCmd->cqe.DW5;
                        pFpCmd->cqe.DW14 = pFpCmd->cqe.DW6;
                        pFpCmd->cqe.DW15 = pFpCmd->cqe.DW7;

                        // Clear all the remaining Dwords
                        pFpCmd->cqe.DW5 = 0;
                        pFpCmd->cqe.DW6 = 0;
                        pFpCmd->cqe.DW7 = 0;
                        pFpCmd->cqe.DW8 = 0;
                        pFpCmd->cqe.DW9 = 0;
                        pFpCmd->cqe.DW10 = 0;
                        pFpCmd->cqe.DW11 = 0;

                        /* if the queue is in tear down process, do not return any CQE to host, just complete it internally, ask CPU 0 to refill DFL. */
                        if (pThis->_pSlotFlagSts[phyQId] & cStsTearDown)
                        {
                            pThis->FpsCpu2SendRefillDFLRequestToCpu0(ceIndex);

                            #ifdef CDMA_CMD_COUNT
                            pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                            writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
                            #endif

                            pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                            writel(pThis->cdmaCqCi, cdmaHwCiAddr);

                            continue;
                        }

                        /* if the queue is in delete process, return error CQE to host, CPU0 will refill DFL in OB CQ of CPU 0 */
                        if (pThis->_pSlotFlagSts[phyQId] & cStsDelete)
                        {
                            bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(CQE_SC_DELETE_QUEUE, CQE_DEFAULT_ERROR_CODE);
                            if (returnHost)
                            {
                                break;  // return CQE to host
                            }
                            else
                            {
                                return;
                            }
                        }

                        //Moving it under LIONPERF_SUPPORT (disabled code) as we never hit this case
                        #ifdef LIONPERF_SUPPORT
                        if (unlikely(pThis->_pCmdEntryArrayTinyBase[ceIndex].abortStatus == cCETinyAdminAbort))
                        {
                            bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(CQE_SC_ABORT_REQ, CQE_DEFAULT_ERROR_CODE);
                            if (returnHost)
                            {
                                break;  // return CQE to host
                            }
                            else
                            {
                                return;
                            }
                        }
                        #endif

                        if (unlikely(pThis->_pCmdEntryArrayBase[ceIndex].Status == cCEStsCorrKeyErrHandling))
                        {
                            pThis->_pCmdEntryArrayBase[ceIndex].Status = cCEStsValid;
                            #ifdef CDMA_CMD_COUNT
                            pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                            writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
                            #endif

                            pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                            writel(pThis->cdmaCqCi, cdmaHwCiAddr);
                            continue;
                        }

                        #ifdef QOS_LATENCY_ERROR_HANDLING
                        if (pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus == cCETinyStsQosErr)
                        {
                            /* Check if it needs to notify CPU1 to do the qos penalty process. */
                            if (M7_QUEUE_FULL(readl(pThis->pCPU2toCPU1Pi), readl(pThis->pCPU2toCPU1Ci), PSRAM_INTL_CPUX2CPUY_MSG_MASK))
                            {
                                return;
                            } // else do nothing

                            pThis->Cpu2HandleQosLatencyTimeoutError(ceIndex);

                            pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus = cCETinyStsReportHost;
                        }
                        #endif

                        if (!(pFpCmd->cqe.cipher) && pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode == cCETinyHostErrCqeCryptoeTagMismatchErr &&
                            ((pFpCmd->meta.AesGcmCmd.UnpaddedAADLen  < pFpCmd->meta.AesGcmCmd.AADLen) ||
                            (pFpCmd->meta.AesGcmCmd.UnalignedSrcDataLen > 0)))
                        {
                            // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("CDMA_CMD_ABORTED Ceindex :0x%X\n", ceIndex), "32");
                            aesGcmSendToCP = true;
                            if(pFpCmd->sqe.SrcDataLen == 0)
                            {
                                aesGcmTagInvalid = true; //No AAD and No Aligned data available
                            }
                            else
                            {
                                aesGcmTagInvalid = false;
                            }
                            // Clear error codes before sending to CP
                            pThis->_pCmdEntryArrayBase[ceIndex].Status = cCEStsValid;
                            pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus = 0;
                            pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode = 0;
                            break;
                        }

                        /*  return error CQE to host, CPU 0 will refill DFL when process OB CQ*/
                        if (pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus == cCETinyStsReportHost)
                        {
                            LionFPCQEStatusCode cqeStatus = FpsCpu2FillHostStatusCode(pThis->_pCmdEntryArrayBase[ceIndex].Status);
                            LionFPCQEErrorCode cqeError =  (LionFPCQEErrorCode)pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode;

                            bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(cqeStatus, cqeError);
                            if (returnHost)
                            {
                                break;  // return CQE to host
                            }
                            else
                            {
                                return;
                            }
                        }

                        /* If the command have to retry, check if it execeed the retry time */
                        uint8_t isRetryTimeExceed = pThis->ChkRetryTimesExceeded(ceIndex);
                        if (isRetryTimeExceed)
                        {
                            LionFPCQEStatusCode cqeStatus = CQE_SC_CMD_RETRY_TIMES_EXCEEDED;
                            bool returnHost = pThis->FpsCpu2ReturnErrorCommandToHost(cqeStatus, CQE_DEFAULT_ERROR_CODE);
                            if (returnHost)
                            {
                                break;  // return CQE to host
                            }
                            else
                            {
                                return;
                            }
                        }
                        // DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CDMA_CMD_ABORTED: ceIndex:0x%X, CA Status: 0x%x\n", ceIndex, pThis->_pCmdEntryArrayBase[ceIndex].Status), "32", "32");
                        // DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("CDMA_CMD_ABORTED: Sts code:0x%X, Err code:0x%X\n", pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus, pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode), "32", "32");
                        // clear errors before retrying the command.
                        pThis->_pCmdEntryArrayBase[ceIndex].Status = cCEStsValid;
                        pThis->_pCmdEntryArrayTinyBase[ceIndex].ErrStatus = 0;
                        pThis->_pCmdEntryArrayTinyBase[ceIndex].HostErrCode = 0;

                        /* send retry request to CPU 0 */
                        pThis->FpsCpu2SendRetryCeRequestToCpu0(ceIndex);

                        #ifdef CDMA_CMD_COUNT
                        pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                        writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
                        #endif

                        pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                        writel(pThis->cdmaCqCi, cdmaHwCiAddr);

                        continue;

                    }
                }
            }
            break;

            case CDMA_CP_DFL_LIST:   // list 2 for CP CDMA IO, need not execute OSL
            {
                if (cmdStatus & CDMA_CMD_ERROR)
                {
                    uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;
                    pThis->Cpu2CdmaErrorCmdHandler();
                    pThis->FpsCpu2SendCDMAAbortRequestToCpu1(ceIndex);
                    pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                    DMB();
                    writel(pThis->cdmaCqCi, cdmaHwCiAddr);
                }
                else
                {
                    pThis->CPCDMAIOStatus = cmdStatus;
                    LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)PSRAM_CP_DFL_BUF_ADDR;
                    pFpCmd->cqe.SqId = pCdmaCqe->Dw1.UcdIqId;
                    #ifndef LIONPERF_SUPPORT
                    // FW Workaround for GCM Limitation
                    // IV is to be generated within FIPS boundary so CQE is extended to 16DW from 8DW.
                    // IV should be written to DW5, DW6, DW7.
                    // So, move DataLen, Sqid, SqHead, Status, phase bit and error info accordingly, to free up DW5, DW6 and DW7.
                    pFpCmd->cqe.DataLen = pFpCmd->cqe.DW5;
                    pFpCmd->cqe.DW14 = pFpCmd->cqe.DW6;
                    pFpCmd->cqe.DW15 = pFpCmd->cqe.DW7;

                    // Clear all the remaining Dwords
                    pFpCmd->cqe.DW5 = 0;
                    pFpCmd->cqe.DW6 = 0;
                    pFpCmd->cqe.DW7 = 0;
                    pFpCmd->cqe.DW8 = 0;
                    pFpCmd->cqe.DW9 = 0;
                    pFpCmd->cqe.DW10 = 0;
                    pFpCmd->cqe.DW11 = 0;
                    pFpCmd->cqe.DW12 = 0;
                    #endif
                    uint32_t ceStatus = readl(PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
                    uint32_t ceError = readl(PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);

                    if (pThis->_pSlotFlagSts[pCdmaCqe->Dw1.UcdIqId] & cStsDelete)
                    {
                        writel(CQE_SC_DELETE_QUEUE, PSRAM_FP_CPIO_CDMA_STATUS_ADDR);
                        writel(0, PSRAM_FP_CPIO_CDMA_ERR_CODE_ADDR);
                    }
                    else if (ceStatus == cCEStsInvalidXTSField)
                    {
                        pFpCmd->cqe.StsCode = CQE_SC_INVALID_FIELD_XTS;
                        pFpCmd->cqe.ErrCode = ceError;
                    }
                    else if (ceStatus == cCEStsInvalidGCMField)
                    {
                        pFpCmd->cqe.StsCode = CQE_SC_INVALID_FIELD_GCM;
                        pFpCmd->cqe.ErrCode = ceError;
                    }
                    else
                    {
                        pFpCmd->cqe.StsCode = ceStatus; // Previously Set in error for nonfatal case and interrupt in fatal case
                        pFpCmd->cqe.ErrCode = ceError;
                    }

                    if(!pFpCmd->cqe.cipher) // GCM commands
                    {
                        // FW Workaround for GCM Limitation
                        // IV is to be generated within FIPS boundary so CQE is extended to 16DW from 8DW.
                        // IV should be written to DW5, DW6, DW7.
                        // So, move DataLen, Sqid, SqHead, Status, phase bit and error info accordingly, to free up DW5, DW6 and DW7.
                        pFpCmd->cqe.DataLen = pFpCmd->cqe.DW5;
                        pFpCmd->cqe.DW14 = pFpCmd->cqe.DW6;
                        pFpCmd->cqe.DW15 = pFpCmd->cqe.DW7;

                        // Clear all the remaining Dwords
                        pFpCmd->cqe.DW5 = 0;
                        pFpCmd->cqe.DW6 = 0;
                        pFpCmd->cqe.DW7 = 0;
                        pFpCmd->cqe.DW8 = 0;
                        pFpCmd->cqe.DW9 = 0;
                        pFpCmd->cqe.DW10 = 0;
                        pFpCmd->cqe.DW11 = 0;

                        // Populate the IV in the CQE
                        M7_MEM_COPY(pFpCmd->cqe.IV, pFpCmd->meta.AesGcmCmd.IV, sizeof(pFpCmd->meta.AesGcmCmd.IV));
                    }

#ifdef CDMA_CMD_COUNT
                    pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
                    writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
#endif
                    pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
                    DMB();
                    writel(pThis->cdmaCqCi, cdmaHwCiAddr);
                    pThis->CPCDMAIODone = 1;
                }

                continue;
            }
            break;

            default:
                DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Invalid DFL List :0x%X\n", cdmaListNum), "32");
                break;
        }

        LionFPCmdMetaData_t* pFpCmd = (LionFPCmdMetaData_t*)(CPU2AccessCPU1TCMMem(dflBuffPhysicalAddr));
        uint16_t ceIndex = pCdmaCqe->Dw0.CmdId;
        uint8_t vfId = MAP_FUNCTION_ID((GET_VF_ID(pThis->_pCmdEntryArrayBase[ceIndex].IFSel)));

        #ifdef CDMA_CMD_COUNT
        pThis->cdmaCmdSlotQueueCi = QUEUE_INC(pThis->cdmaCmdSlotQueueCi, FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK);
        writel(pThis->cdmaCmdSlotQueueCi, pThis->pCdmaCmdSlotQueueCi);
        #endif

        pThis->cdmaCqCi = QUEUE_INC(pThis->cdmaCqCi, FPS_CDMA_QUEUE_DEPTH_MASK);
        writel(pThis->cdmaCqCi, cdmaHwCiAddr);

        #ifdef MCR_TEST_HOOKS
        //One completion IO timeouts but remaining IOs  pass. This will trigger a level 1 abort.
        if(!(pThis->level1AbortFlag))
        {
        #endif

        _pKeyFlag = (uint8_t*)((uint32_t)CPU2AccessCPU0TCMMem((uint32_t)(M7_FPS_CPU01_KEY_FLAG_ADDR)));
        uint16_t keyIndex1 = (pFpCmd->meta.AesXtsCmd.HostKeyIdx[0].resourceGroupID * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pFpCmd->meta.AesXtsCmd.HostKeyIdx[0].keySubIndex;
        KeyFlags_t* pKeyFlag1 = (KeyFlags_t*)&_pKeyFlag[keyIndex1];

        #ifndef LIONPERF_SUPPORT

        if(pKeyFlag1->keyType == cAesGcmApproved)
        {
            pFpCmd->cqe.ServiceIndicator = (pThis->_pServiceIndicator[vfId] & pKeyFlag1->keyType);
        }
        else if(pKeyFlag1->keyType == cAesGcmUnapproved)
        {
            pFpCmd->cqe.ServiceIndicator = 0;
        }
        else
        {
            pFpCmd->cqe.ServiceIndicator = pThis->_pServiceIndicator[vfId];
        }

        if(!(pFpCmd->cqe.cipher))
        {
            // Populate the IV in the CQE
            M7_MEM_COPY(pFpCmd->cqe.IV, pFpCmd->meta.AesGcmCmd.IV, sizeof(pFpCmd->meta.AesGcmCmd.IV));

            if(aesGcmSendToCP)
            {
                pThis->FpsCpu2ProcessGcmCmdFiber(pThis); //Handle response queue.
                uint16_t cmdTimeOutCtr = 0;
                while(API_GcmReqQueueFull())
                {
                    //Explicit crash if Req queue is full after timeout.
                    if(cmdTimeOutCtr++ == 5000)
                    {
                        // DebugLogLvDbgInfo(cLogCPU2Common, cLogError, ("FP2 crashed due to GcmReqQueue full after timeout, Cmd id: 0x%x",ceIndex), "32");
                        Explicit_CrashCatcher_Entry();
                    }
                };

            //Uncomment following logs for Tag correction debug
            //Request to CP
            // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Request sent to CP EnDecrypt:0x%X, ceindex:%X\n", pFpCmd->cqe.EnDecrypt, ceIndex), "32", "32");
            API_AddGCMTagCorrect(ceIndex, aesGcmTagInvalid, vfId, dflBuffPhysicalAddr);
            continue;
            }
        }
        #endif
        UcdOslEntry_t* pOslEntry = &((pThis->_ucdObq.pOslEntries[oslIndex])[oslPi]);
        dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(cdmaListNum, (pCdmaCqe->Dw0.CmdDflIdx << DFL_BUF_SZ_SHIFT));
        uint8_t phyQId = pCdmaCqe->Dw1.UcdIqId;

        pOslEntry->addrLow = dflBuffPhysicalAddr;
        pOslEntry->addrHi = 0;
        pOslEntry->Dw3 = (((uint32_t)pThis->_pIbQ2ObQ[phyQId]) << UCD_OSL_ENTRY_DW3_QPID_SH) | \
                        (UCD_CREDIT_CNT_NUM << UCD_OSL_ENTRY_DW3_CREDIT_SH) | (uint32_t)phyQId;
        pOslEntry->Dw4 = ceIndex;

        pThis->_pCmdEntryArrayBase[ceIndex].Dw0 = 0;
        pThis->_pCmdEntryArrayTinyBase[ceIndex].Dw0 = 0;

        pThis->outBoundOSLPi[oslIndex] = QUEUE_INC(oslPi, qMask);

        DMB();

        writel(pThis->outBoundOSLPi[oslIndex], pThis->_ucdObq.pHwOslPi[oslIndex]);
        #ifdef DISABLE_INDIRECT_REG_WRITE
        writel(pThis->outBoundOSLPi[oslIndex], pThis->hwOslPiAddr[oslIndex]);
        #endif

        #ifdef MCR_TEST_HOOKS
        }
        else
        {
            DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("L1 abort triggered, msgOp:0x%X\n", (((msgOpInjectErrorReq & 0xFF) << 0x18UL))), "32");
            pThis->level1AbortFlag = false;
        }
        #endif
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
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("logging timestamp: 0x%x\n", pThis->localTimeStamp), "32");
    }
    #endif
}

void fpsCpu2::FpsCpu2ProcessGcmCmdFiber(void* pObj){
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    
    uint32_t pIsEmpty;
    GcmResponseEntry_t resp = API_GetGCMTagCorrect(&pIsEmpty);

    while(!pIsEmpty){
        //Uncomment following logs for Tag correction debug
        //Response from CP
        // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Resp status:0x%X\n", resp.status), "32");
        if(resp.status)
        {
            //Set Status and error
            LionFPCQEErrorCode cqeError =  FpsCpu2FillHostGcmErrorCode((AesGcmExtRespErr)resp.status);
            pThis->FpsCpu2ReturnCPRespErrCmdToHost(pThis, resp.ceIndex, cqeError);
        }
        uint16_t ceIndex = resp.ceIndex;
        CmdEntry_t* pCe = pThis->_pCmdEntryArrayBase + ceIndex;
        uint8_t cdmaListNum = pCe->cdmaListNum;
        uint8_t oslIndex = (cdmaListNum == CDMA_FP_DFL_0_LIST) ? OSL_0 : OSL_1;
        uint16_t qMask = (oslIndex == OSL_0) ? FPS_IO_QUEUE_DEPTH_MASK : FPS_IO_QUEUE_1_DEPTH_MASK;
        uint32_t oslPi = pThis->outBoundOSLPi[oslIndex];
        
        uint32_t dflBuffPhysicalAddr = GET_DFL_PHYSICAL_BUF_ADDR(pCe->cdmaListNum, (pCe->DFLIdx << DFL_BUF_SZ_SHIFT));

        if (QUEUE_FULL(oslPi, pThis->localObCqPi[oslIndex], qMask))
        {
            pThis->localObCqPi[oslIndex] = readl(pThis->_ucdObq.pHwObCqPi[oslIndex]);
            if (QUEUE_FULL(oslPi, pThis->localObCqPi[oslIndex], qMask))
            {
                // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Failed to sent to OSL, QUEUE_FULL Ceindex:0x%X\n", ceIndex), "32");
                return;
            }
        }
        
        UcdOslEntry_t* pOslEntry = &((pThis->_ucdObq.pOslEntries[oslIndex])[oslPi]);
        uint8_t phyQId = pCe->PhyIbqId;
        // In case of teardown dont send completion to host
        if (pThis->_pSlotFlagSts[phyQId] & cStsTearDown)
        {
            // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("cStsTearDown pThis->_pSlotFlagSts[phyQId] :0x%X, ceIndex :0x%X\n", pThis->_pSlotFlagSts[phyQId],ceIndex), "32", "32");
            pThis->FpsCpu2SendRefillDFLRequestToCpu0(ceIndex);
            resp = API_GetGCMTagCorrect(&pIsEmpty);
            continue;
        }

        // In case of queue delete send status as queue delete in progress to host
        if (pThis->_pSlotFlagSts[phyQId] & cStsDelete)
        {
            // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("cStsDelete pThis->_pSlotFlagSts[phyQId] :0x%X, ceIndex :0x%X\n", pThis->_pSlotFlagSts[phyQId],ceIndex), "32", "32");
            pThis->FpsCpu2ReturnErrorCommandToHost(CQE_SC_DELETE_QUEUE, CQE_DEFAULT_ERROR_CODE);
        }
        pOslEntry->addrLow = dflBuffPhysicalAddr;
        pOslEntry->addrHi = 0;
        pOslEntry->Dw3 = (((uint32_t)pThis->_pIbQ2ObQ[phyQId]) << UCD_OSL_ENTRY_DW3_QPID_SH) | \
                        (UCD_CREDIT_CNT_NUM << UCD_OSL_ENTRY_DW3_CREDIT_SH) | (uint32_t)phyQId;
        pOslEntry->Dw4 = ceIndex;
        pThis->_pCmdEntryArrayBase[ceIndex].Dw0 = 0;
        pThis->_pCmdEntryArrayTinyBase[ceIndex].Dw0 = 0;
        pThis->outBoundOSLPi[oslIndex] = QUEUE_INC(oslPi, qMask);
        DMB();
        writel(pThis->outBoundOSLPi[oslIndex], pThis->_ucdObq.pHwOslPi[oslIndex]);
        #ifdef DISABLE_INDIRECT_REG_WRITE
        writel(pThis->outBoundOSLPi[oslIndex], pThis->hwOslPiAddr[oslIndex]);
        #endif
        //Uncomment following logs for Tag correction debug
        // DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("Command sent to OSL, Ceindex:0x%X\n", ceIndex), "32");
        resp = API_GetGCMTagCorrect(&pIsEmpty);
    }
}

void fpsCpu2::FpsCpu2CP2FPServiceFiber(void* pObj)
{
    // Get the Component instance?? what?
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    CP2FPMsgContext_t* pCPtoFPMsgQTemp;
    uint32_t msgCi, msgPi, msgCiTemp;
    uint8_t msgSrc = CP0;
    uint8_t* pMsgState;
    CP2FPMessageInfo* pCp2FPMsgInfo;

    while (msgSrc < MsgQNum)
    {
        switch (msgSrc)
        {
            case CP0ToFP_Req:
                pCp2FPMsgInfo = &pThis->CP0toFPReqMsg;
                break;
            case CP1ToFP_Req:
                pCp2FPMsgInfo = &pThis->CP1toFPReqMsg;
                break;
            case CP0ToFP_Res:
                pCp2FPMsgInfo = &pThis->CP0toFPResMsg;
                break;
            case CP1ToFP_Res:
                pCp2FPMsgInfo = &pThis->CP1toFPResMsg;
                break;
            default:
                return;

        }
        pCp2FPMsgInfo->localMsgPi = readl(pCp2FPMsgInfo->pMsgPi);
        msgPi = pCp2FPMsgInfo->localMsgPi;
        msgCi = pCp2FPMsgInfo->localMsgCi;
        pMsgState = pCp2FPMsgInfo->msgState;
        msgCiTemp = msgCi;
        while (msgPi != msgCiTemp)
        {
            pCPtoFPMsgQTemp =  (CP2FPMsgContext_t*)(((uint32_t)pCp2FPMsgInfo->pMsgQ) + PSRAM_CP2FP_MSG_ELMNT_SIZE * msgCiTemp);
            if (pMsgState[msgCiTemp] == MSG_STATE_START)
            {
                //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogInfo, ("FpsCpu2CP2FPServiceFiber msgPi[0x%X] msgCiTemp[0x%X]\n", msgPi | (msgCiTemp << 0x10UL)), "16,16");
                pCPtoFPMsgQTemp->sts = msgSuccess;

                // pCPtoFPMsgQTemp->submitMap = 0;
                // pCPtoFPMsgQTemp->completeMap = 0;

                pCp2FPMsgInfo->msgBitmap[msgCiTemp].bitMap = 0;


            } // else do nothing
            if (msgSrc <= CP1ToFP_Req)
            {
                pThis->HandleCP2FPMsg(pCp2FPMsgInfo, pCPtoFPMsgQTemp, msgCiTemp, pMsgState, (CPCoreId_t)msgSrc);
            }
            else //response message q
            {
                pThis->HandleCP2FPMsg(pCp2FPMsgInfo, pCPtoFPMsgQTemp, msgCiTemp, pMsgState, (CPCoreId_t)msgSrc);
            }

            if(msgSrc == CP0ToFP_Req || msgSrc == CP1ToFP_Req)
            {
                msgCiTemp = M7_QUEUE_INC(msgCiTemp, PSRAM_CP2FP_MSG_MASK);
            }
            else
            {
                msgCiTemp = M7_QUEUE_INC(msgCiTemp, PSRAM_CP2FP_REQ_RES_MSG_MASK);
            }
        }
        pThis->ChkUpdateCptoFpMsgCi(pCp2FPMsgInfo,(CPMsgQId)msgSrc);
        msgSrc++;

    }

    if (pThis->ChkCPtoFPMsgFiberDone())
    {
        pThis->_fpsCpu2CP2FPServiceFiber.Wait();
        #if defined (IPC_SUPPORT)
        IpcIntMaskClr(IPC_FP2, CP0toFP_REQ_DESC);
        IpcIntMaskClr(IPC_FP2, CP1toFP_REQ_DESC);
        IpcIntMaskClr(IPC_FP2, CP0toFP_RES_DESC);
        IpcIntMaskClr(IPC_FP2, CP1toFP_RES_DESC);
        #endif
    }
}

void fpsCpu2::FpsCpu2ReceiveFPMsgFiber(void* pObj)
{
    //Receive from other FPs
    //Send to CP //chk is need to be deliver to CP
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    CP2FPMsgSts sts;
    uint32_t msgCPU0toCPU2Pi, msgCPU0toCPU2Ci;
    uint32_t msgCPU1toCPU2Pi, msgCPU1toCPU2Ci;
    uint8_t modify;

    modify = false;
    msgCPU0toCPU2Pi = readl(pThis->pCPU0toCPU2Pi);
    msgCPU0toCPU2Ci = readl(pThis->pCPU0toCPU2Ci);

    while (msgCPU0toCPU2Pi != msgCPU0toCPU2Ci)
    {
        sts = pThis->RecvFPMsg(&pThis->pCPU0toCPU2MsgQ[msgCPU0toCPU2Ci], cM7Core0);
        if (ChkFPMsgStsDone(sts))
        {
            msgCPU0toCPU2Ci = M7_QUEUE_INC(msgCPU0toCPU2Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            modify = true;
        }
        else
        {
            break;
        }
        //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogInfo, ("FpsCpu2CP2FPServiceFiber msgCPU0toCPU2Pi[0x%X] msgCPU0toCPU2Ci[0x%X]\n", msgCPU0toCPU2Pi | (msgCPU0toCPU2Ci << 0x10UL)), "16,16");
    }

    if (modify)
    {
        pThis->CPU0toCPU2Ci = msgCPU0toCPU2Ci;
        writel(msgCPU0toCPU2Ci, pThis->pCPU0toCPU2Ci);
    } // else do nothing

    //req from cpu1
    msgCPU1toCPU2Pi = readl(pThis->pCPU1toCPU2Pi);
    msgCPU1toCPU2Ci = pThis->CPU1toCPU2Ci;
    modify = false;

    while (msgCPU1toCPU2Pi != msgCPU1toCPU2Ci)
    {
        sts = pThis->RecvFPMsg(&pThis->pCPU1toCPU2MsgQ[msgCPU1toCPU2Ci], cM7Core1);
        //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogInfo, ("FpsCpu2CP2FPServiceFiber sts[0x%X]\n", sts), "32");
        if (ChkFPMsgStsDone(sts))
        {
            msgCPU1toCPU2Ci = M7_QUEUE_INC(msgCPU1toCPU2Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            modify = true;
        }
        else
        {
            break;
        }
    }

    if (modify)
    {
        pThis->CPU1toCPU2Ci = msgCPU1toCPU2Ci;
        writel(msgCPU1toCPU2Ci, pThis->pCPU1toCPU2Ci);
    } //else do nothing
    if (pThis->ChkRecvFPMsgFiberDone())
    {
        pThis->_fpsCpu2RecvFpMsgFiber.Wait();
        #ifdef IPC_SUPPORT
        IpcIntMaskClr(IPC_FP2, CPU0toCPU2_DESC);
        IpcIntMaskClr(IPC_FP2, CPU1toCPU2_DESC);
        #endif
    } //else do nothing

}

void fpsCpu2::CheckFPMsgFiberNeedResume(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    {
        pThis->_fpsCpu2RecvFpMsgFiber.Resume();
    }

}

void fpsCpu2::CheckCP2FPMsgFiberNeedResume(void* pObj)
{
    fpsCpu2* pThis = static_cast<fpsCpu2*>(pObj);
    {
        pThis->_fpsCpu2CP2FPServiceFiber.Resume();
    }

}
