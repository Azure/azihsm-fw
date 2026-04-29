// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//
//! @file
//! @brief C main() routine for FPSCPU2
//!
//=============================================================================


//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

extern "C"
{
#include "vicommon.h"
#include "irq.h"
#include "crashdump.h"
#include "APILogging.h"
#include "HalFps.h"
#include "API_GCMTagCorrect.h"
}
#include "M7MemMap.h"
#include "MemorySection.h"
MODULE_SECTION_FAST

#include "LoggingDebug.h"
#include "M7FiberScheduler.h"
#include "FpsCpu2.h"
fpsCpu2 gFpsCpu2;
ResetType_t gResetType = cInvalidResetType;
uint32_t gDrainTimerIntrCnt = 0;
uint32_t gDrainTimerValue = 0;

//-----------------------------------------------------------------------------
//  Private Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Private Data Type Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Private Function Declarations
//-----------------------------------------------------------------------------

static void M7Core2PreOneTimeInit();
static void M7Core2OneTimeInit();

//-----------------------------------------------------------------------------
//  Private Variable Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Function Definitions
//-----------------------------------------------------------------------------

/**
 * main() for CPU0\n
 *  - start Core codes.\n
 *  - One time initialization for all Core modules.\n
 *  - Start fiber for CPU0\n
 *
 *  @return     shall be 0
 */
int main(void)
{
    Disable_Tcon_Wakeup1();
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);
    VicIrqDisable(TCON_INT_WAKE_TIMER_1_NUM);
    //Initialize Logging
    InitializeCoreLogging(cM7Core2);

    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    gResetType = cPor;
    #else
    gResetType = (ResetType_t)readl(PSRAM_RESET_TYPE);
    if(gResetType == cWarmReset)
    {
        gResetType = cPor;
        M7_MEM_SET((void*)FPS_CPU20_SHARE_DTCM_START, 0,  FPS_CPU20_SHARE_DTCM_SIZE);
    }
    #endif
    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    bool needClearPsram = (gResetType != cFwUpdateWarmReset) ? true : false;
    #endif

    HalFps_InitCpuMemoryControl(cCore2);

    for (uint32_t i = FPS_CPU2_IRAM_START; i < FPS_CPU2_IRAM_END; i += sizeof(uint32_t))
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU2_DTCM_START; i < FPS_CPU2_DTCM_END; i += sizeof(uint32_t))
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU01_SHARE_DTCM_START; i < FPS_CPU2_STACK_PROTECT; i += sizeof(uint32_t))
    {
        writel(readl(i), i);
    }

    HalFps_EnableTcmProtectionCheck(cCore2, true);

    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    // Config psram initial pattern, enable psram Read-Modift-Write for ecc
    // ECC threshold is not set at Init
    HalFps_ConfigureFpsMemoryControlRegister(cFPS_MEM_CTRL_CORR_THRESHOLD_0);

    if (needClearPsram)
    {
        //disable protection check, use ecc
        HalFps_InitPsramMemoryControl();
    } //else do nothing

    //enable parity for fabric(data path, use parity), psram(data path, use parity)
    HalFps_EnableFabricParity();
    #endif

    // Set FPS PARITY settings
    HalFps_InitFpsControlRegister();
    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    if (needClearPsram)
    {
        M7_MEM_SET((void*)PSRAM_START, 0,  PSRAM_SIZE);
    } //else do nothing

    if (needClearPsram)
    {
        //enable protection check, use ecc
        HalFps_EnablePsramProtectionCheck(true);
    } //else do nothing
    #endif

    /// Do PreOneTimeInit before main().
    M7Core2PreOneTimeInit();

    /// Do OneTimeInit.
    M7Core2OneTimeInit();

    M7FiberScheduler_ServiceLoop(cM7Core2);

    return 0;
}

//-----------------------------------------------------------------------------
//  Private Function Definitions
//-----------------------------------------------------------------------------

/**
 *   One time initialization for core modules.\n
 *
 *    - Serial Flash module\n
 *    - IRQ(Interrupt Request) modue\n
 *    - IPC(Inter Process Comuniation) module\n
 *    - Kernel and Kernel Synchronization objects\n
 *    - Create tasks\n
 *
 *   @param[in]  initMode        initialization mode
 **/
static void M7Core2OneTimeInit()
{
    //GCM Tag correction queues one-time init
    API_GcmReqResQueuesOneTimeInitByFp2();

    gFpsCpu2.Initialize(cM7CompGroupIo);
    M7FiberScheduler_Initialize(cM7Core2);
    // CheckStackIntegrity(true);
}

/**
 * One time initialization for HAL related modules.
 * Note: Some initializations were moved from M7PreOneTimeInit of cpu boot into cpu main source files.
 * If there are some routines which must be initialized before main function,
 * this function could be the first candidate.
 *
 * @param initMode        initialization mode
 */
void M7Core2PreOneTimeInit()
{
    #ifdef IPC_SUPPORT
    uint8_t irqReqResult = 0;
    IpcIntMaskClr(IPC_FP2, CPU0toCPU2_DESC);
    IpcIntMaskClr(IPC_FP2, CPU1toCPU2_DESC);
    irqReqResult = Irq_Request(FP2_INT_NUM, Irq_FpsCpu2MsgHandler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(FP2_INT_NUM);
        //init descrptor
        IpcIntEnableSet(IPC_FP2, CP0toFP_REQ_DESC);
        IpcIntEnableSet(IPC_FP2, CP1toFP_REQ_DESC);
        IpcIntEnableSet(IPC_FP2, CP0toFP_RES_DESC);
        IpcIntEnableSet(IPC_FP2, CP1toFP_RES_DESC);
        IpcIntEnableSet(IPC_FP2, CPU0toCPU2_DESC);
        IpcIntEnableSet(IPC_FP2, CPU1toCPU2_DESC);

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        IpcIntEnableSet(IPC_FP2, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
        #endif

        IpcIntEnableSet(IPC_FP2, ResetCP2FP);

    } // else do nothing
    irqReqResult = Irq_Request(CDMA_INT_1_NUM, Irq_FpsCpu2CDMAIrqHandler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(CDMA_INT_1_NUM);
    } // else do nothing
    //Setup TCON wakeup0 interrupt
    irqReqResult = Irq_Request(TCON_INT_WAKE_TIMER_0_NUM, Irq_FpsWakeup0Handler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqDisable(TCON_INT_WAKE_TIMER_0_NUM);
    }
    //Setup TCON wakeup1 interrupt
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);
    irqReqResult = Irq_Request(TCON_INT_WAKE_TIMER_1_NUM, Irq_FpsWakeup1Handler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(TCON_INT_WAKE_TIMER_1_NUM);
    } // else do nothing
    #endif

}
#ifdef SUPPORT_UPDATE_TIMESTAMP
#ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
void FpsUpdateTimestamp(void)
{
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu2.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu2.rCortexm7;
    gFpsCpu2.gTimerCounterBase = (readl(REG_GLOBAL_SYNC_COUNTER_LO)) & SYSTICK_MASK;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x3, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
}
#endif //SUPPORT_UPDATE_TIMESTAMP_IPC
void FpsTriggerTimer(void)
{
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu2.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu2.rCortexm7;
    uint32_t tempTCONDelta = 0;
    uint32_t tempSYSTICKDelta = 0;
    uint32_t tempSYSTICK = 0;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    gFpsCpu2.gTimerCounterLast = readl(REG_GLOBAL_SYNC_COUNTER_LO) & SYSTICK_MASK;
    if (gFpsCpu2.gTimerCounterBase > gFpsCpu2.gTimerCounterLast)
    {
        tempTCONDelta = (SYSTICK_MASK + 1) + gFpsCpu2.gTimerCounterLast - gFpsCpu2.gTimerCounterBase;
    }
    else
    {
        tempTCONDelta = gFpsCpu2.gTimerCounterLast - gFpsCpu2.gTimerCounterBase;
    }
    tempSYSTICKDelta = GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, tempTCONDelta, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    tempSYSTICK = GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, gFpsCpu2.gTimerCounterLast, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    gFpsCpu2.gTimerCounterDelta = tempSYSTICKDelta - SYSTICK_TIMER_VALUE;
    gFpsCpu2.gTimerCounterCount++;
    gFpsCpu2.gTimerCounterCovert =  (tempSYSTICK - gFpsCpu2.gTimerCounterDelta) & SYSTICK_DELTA_MASK;

    writel(SYSTICK_MASK - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x1, REG_SYSTICK_CONTROL_STATUS);
    #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
    gFpsCpu2.gTimeSyncDone = 1;
    #endif
}
#endif //SUPPORT_UPDATE_TIMESTAMP_IPC

void FpsDrainTimer()
{
    if(gDrainTimerValue > 0)
    {
        Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu2.rCortexm7;
        gDrainTimerIntrCnt++;
        writel(0x0, &rCortexm7->systemControl.systCsr);
        // VM: TODO: Add logic to calculate ticks reload value based on remaining time
        uint32_t currentTimestamp = SYSTICK_MASK - readl(REG_SYSTICK_CURRENT_VALUE) + (gDrainTimerIntrCnt * SYSTICK_MASK);
        uint32_t setTime = SYSTICK_MASK;
        if(gDrainTimerValue - currentTimestamp < SYSTICK_MASK)
        {
            setTime = gDrainTimerValue - currentTimestamp;
        }
        writel(setTime, &rCortexm7->systemControl.systRvr);
        writel(0x0, &rCortexm7->systemControl.systCvr);   //any write to current val clears it.
        writel(0x7, &rCortexm7->systemControl.systCsr);
        //DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("IDFU: in intr=%d\n", gDrainTimerIntrCnt), "32"); // debug print can be removed during release
    }
}
