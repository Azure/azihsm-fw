// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//
//! @file
//! @brief C main() routine for FPSCPU1
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
#include "FpsCpu1.h"
fpsCpu1 gFpsCpu1;
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

static void M7Core1PreOneTimeInit();
static void M7Core1OneTimeInit();

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
    InitializeCoreLogging(cM7Core1);

    #ifdef WA_ZEROOUT_PSRAM_IN_FP
    gResetType = cPor;
    #else
    gResetType = (ResetType_t)readl(PSRAM_RESET_TYPE);
    if(gResetType == cWarmReset)
    {
        gResetType = cPor;
        M7_MEM_SET((void*)FPS_CPU12_SHARE_DTCM_START, 0,  FPS_CPU12_SHARE_DTCM_SIZE);
    }
    #endif

    HalFps_InitCpuMemoryControl(cCore1);

    for (uint32_t i = FPS_CPU1_IRAM_START; i < FPS_CPU1_IRAM_END; i += 4)
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU1_DTCM_START; i < FPS_CPU1_DTCM_END; i += 4)
    {
        writel(readl(i), i);
    }
    for (uint32_t i = FPS_CPU01_SHARE_DTCM_START; i < FPS_CPU1_STACK_PROTECT; i += 4)
    {
        writel(readl(i), i);
    }

    HalFps_EnableTcmProtectionCheck(cCore1, true);

    HalFps_InitFpsControlRegister();

    while (readl(PSRAM_FP_CPU2_STATUS_ADDR) != FP_STS_INIT_START)
    {
        // Do nothing
    }

    /// Do PreOneTimeInit before main().
    M7Core1PreOneTimeInit();

    /// Do OneTimeInit.
    M7Core1OneTimeInit();

    M7FiberScheduler_ServiceLoop(cM7Core1);

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
static void M7Core1OneTimeInit()
{
    //GCM IV Queue one-time init
    API_GcmIvQueueOneTimeInitByFp1();

    gFpsCpu1.Initialize(cM7CompGroupIo);
    M7FiberScheduler_Initialize(cM7Core1);
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
void M7Core1PreOneTimeInit()
{
    #ifdef IPC_SUPPORT
    uint8_t irqReqResult = 0;

    IpcIntMaskClr(IPC_FP1, CPU0toCPU1_DESC);
    IpcIntMaskClr(IPC_FP1, CPU2toCPU1_DESC);

    irqReqResult = Irq_Request(FP1_INT_NUM, Irq_FpsCpu1MsgHandler, 0, NULL);
    if (irqReqResult == 0)
    {
        VicIrqEnable(FP1_INT_NUM);
        //VicIrqEnable(132);
        //init descriptor
        IpcIntEnableSet(IPC_FP1, CPU0toCPU1_DESC);
        IpcIntEnableSet(IPC_FP1, CPU2toCPU1_DESC);

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        IpcIntEnableSet(IPC_FP1, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
        #endif

    } // else do nothing
    //Setup TCON wakeup1 Interrupt
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
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu1.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu1.rCortexm7;
    gFpsCpu1.gTimerCounterBase = (readl(REG_GLOBAL_SYNC_COUNTER_LO)) & SYSTICK_MASK;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x3, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
}
#endif //SUPPORT_UPDATE_TIMESTAMP_IPC
void FpsTriggerTimer(void)
{
    Tcon_t* rTcon = (Tcon_t*)gFpsCpu1.rTcon;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)gFpsCpu1.rCortexm7;
    uint32_t tempTCONDelta = 0;
    uint32_t tempSYSTICKDelta = 0;
    uint32_t tempSYSTICK = 0;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    gFpsCpu1.gTimerCounterLast = readl(REG_GLOBAL_SYNC_COUNTER_LO) & SYSTICK_MASK;
    if (gFpsCpu1.gTimerCounterBase > gFpsCpu1.gTimerCounterLast)
    {
        tempTCONDelta = (SYSTICK_MASK + 1) + gFpsCpu1.gTimerCounterLast - gFpsCpu1.gTimerCounterBase;
    }
    else
    {
        tempTCONDelta = gFpsCpu1.gTimerCounterLast - gFpsCpu1.gTimerCounterBase;
    }
    tempSYSTICKDelta = (uint32_t)GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, tempTCONDelta, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    tempSYSTICK = (uint32_t)GLOBAL_TICK_TO_SYSTICK(GLOBAL_SYNC_COUNTER_CLOCK, gFpsCpu1.gTimerCounterLast, ARM_SYSTICK_CLOCK, SYSTICK_THRESHOLD);
    gFpsCpu1.gTimerCounterDelta = tempSYSTICKDelta - SYSTICK_TIMER_VALUE;
    gFpsCpu1.gTimerCounterCount++;
    gFpsCpu1.gTimerCounterCovert =  (tempSYSTICK - gFpsCpu1.gTimerCounterDelta) & SYSTICK_DELTA_MASK;

    writel(SYSTICK_MASK - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x1, REG_SYSTICK_CONTROL_STATUS);
}
#endif
