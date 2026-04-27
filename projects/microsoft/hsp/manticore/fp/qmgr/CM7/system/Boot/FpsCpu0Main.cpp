// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//
//! @file
//! @brief C main() routine for FPSCPU0
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
#include "API_GCMTagCorrect.h"
#include "HalFps.h"
}
#include "M7MemMap.h"
#include "MemorySection.h"
MODULE_SECTION_FAST

#include "M7FiberScheduler.h"
#include "../FpsCpu0/FpsCpu0.h"
#include "LoggingDebug.h"
fpsCpu0 gFpsCpu0;
ResetType_t gResetType = cInvalidResetType;

//-----------------------------------------------------------------------------
//  Private Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Private Data Type Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Private Function Declarations
//-----------------------------------------------------------------------------

static void M7Core0PreOneTimeInit();
static void M7Core0OneTimeInit();

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
extern "C" int main(void);
int main(void)
{
    Disable_Tcon_Wakeup1();
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);
    VicIrqDisable(TCON_INT_WAKE_TIMER_1_NUM);
    //Initialize Logging
    InitializeCoreLogging(cM7Core0);

    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    gResetType = cPor;
    #else
    gResetType = (ResetType_t)readl(PSRAM_RESET_TYPE);
    if(gResetType == cWarmReset)
    {
        gResetType = cPor;
        M7_MEM_SET((void*)FPS_CPU01_SHARE_DTCM_START, 0,  FPS_CPU01_SHARE_DTCM_SIZE);
    }
    #endif
    // FPS CPU DISABLE ITCM DTCM PROTECTION
    HalFps_InitCpuMemoryControl(cCore0);

    for (uint32_t i = FPS_CPU0_IRAM_START; i < FPS_CPU0_IRAM_END; i += 4)
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU0_DTCM_START; i < FPS_CPU0_DTCM_END; i += 4)
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU01_SHARE_DTCM_START; i < FPS_CPU0_STACK_PROTECT; i += 4)
    {
        writel(readl(i), i);
    }

    HalFps_EnableTcmProtectionCheck(cCore0,true);
    HalFps_InitFpsControlRegister();

    while (readl(PSRAM_FP_CPU2_STATUS_ADDR) != FP_STS_INIT_START)
    {
        // Do nothing
    }

    /// Do PreOneTimeInit before main().
    M7Core0PreOneTimeInit();

    /// Do OneTimeInit.
    M7Core0OneTimeInit();

    M7FiberScheduler_ServiceLoop(cM7Core0);

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
static void M7Core0OneTimeInit()
{
    gFpsCpu0.Initialize(cM7CompGroupIo);
    M7FiberScheduler_Initialize(cM7Core0);
    // CheckStackIntegrity(true);
}

/**
 * One time initialization for HAL related modules.
 * Note: Some initializations were moved from M7Core0PreOneTimeInit of cpu boot into cpu main source files.
 * If there are some routines which must be initialized before main function,
 * this function could be the first candidate.
 *
 * @param initMode        initialization mode
 */
void M7Core0PreOneTimeInit()
{
    #ifdef IPC_SUPPORT
    uint8_t irqReqResult = 0;

    IpcIntMaskClr(IPC_FP0, CPU1toCPU0_DESC);
    IpcIntMaskClr(IPC_FP0, CPU2toCPU0_DESC);

    irqReqResult = Irq_Request(FP0_INT_NUM, Irq_FpsCpu0MsgHandler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(FP0_INT_NUM);

        //init descrptor
        IpcIntEnableSet(IPC_FP0, CPU1toCPU0_DESC);
        IpcIntEnableSet(IPC_FP0, CPU2toCPU0_DESC);

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        IpcIntEnableSet(IPC_FP0, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
        #endif

    } // else do nothing
    //Setup TCON wakeup1 interrupt
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);
    irqReqResult = Irq_Request(TCON_INT_WAKE_TIMER_1_NUM, Irq_FpsWakeup1Handler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(TCON_INT_WAKE_TIMER_1_NUM);
    }
    #endif
}
#ifdef SUPPORT_UPDATE_TIMESTAMP
#ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
void FpsUpdateTimestamp(void)
{
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu0.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu0.rCortexm7;
    gFpsCpu0.gTimerCounterBase = (readl(REG_GLOBAL_SYNC_COUNTER_LO)) & SYSTICK_MASK;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x3, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
}
#endif //SUPPORT_UPDATE_TIMESTAMP_IPC
void FpsTriggerTimer(void)
{
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu0.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu0.rCortexm7;
    uint32_t tempTCONDelta = 0;
    uint32_t tempSYSTICKDelta = 0;
    uint32_t tempSYSTICK = 0;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    gFpsCpu0.gTimerCounterLast = readl(REG_GLOBAL_SYNC_COUNTER_LO) & SYSTICK_MASK;
    if (gFpsCpu0.gTimerCounterBase > gFpsCpu0.gTimerCounterLast)
    {
        tempTCONDelta = (SYSTICK_MASK + 1) + gFpsCpu0.gTimerCounterLast - gFpsCpu0.gTimerCounterBase;
    }
    else
    {
        tempTCONDelta = gFpsCpu0.gTimerCounterLast - gFpsCpu0.gTimerCounterBase;
    }
    tempSYSTICKDelta = (uint32_t)GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, tempTCONDelta, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    tempSYSTICK = (uint32_t)GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, gFpsCpu0.gTimerCounterLast, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    gFpsCpu0.gTimerCounterDelta = tempSYSTICKDelta - SYSTICK_TIMER_VALUE;
    gFpsCpu0.gTimerCounterCount++;
    gFpsCpu0.gTimerCounterCovert =  (tempSYSTICK - gFpsCpu0.gTimerCounterDelta) & SYSTICK_DELTA_MASK;

    writel(SYSTICK_MASK - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x1, REG_SYSTICK_CONTROL_STATUS);
}
#endif
