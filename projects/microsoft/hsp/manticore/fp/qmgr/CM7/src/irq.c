// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "SysTypes.h"
#include "crashdump.h"
#include "irq.h"
#include "M7MemMap.h"
#include "LoggingDebug.h"
#include "RegTcon.h"

struct irqRegistration_t irqRegs[MAX_IRQ_NUM] = {{0}};
#if defined (CPU2)
extern uint32_t gDrainTimerValue;
#endif
extern void Handler_For_Other_CPU_HardFault(void);

uint32_t gIrqFiber_need_resume = 0;
uint32_t gWakeUp0IrqCount = 0;

uint8_t Irq_Request(uint32_t irq,
                    irqHandler_t handler,
                    uint32_t irqflags,
                    void* devId)
{
    if (irq < MAX_IRQ_NUM)
    {
        irqRegs[irq].irq = irq;
        irqRegs[irq].handler = handler;
        irqRegs[irq].irqflags = irqflags;
        irqRegs[irq].devId = devId;
        return 0; // 0: success, 1: failed
    }
    else
    {
        // send error
        return 1; // 0: success, 1: failed
    }
}


void Irq_Init(uint64_t irqEnBit)
{
    // clear pending
    NVIC->ICPR[0] = (uint32_t)0xFFFFFFFFUL;
    NVIC->ICPR[1] = (uint32_t)0xFFFFFFFFUL;
    NVIC->ICPR[3] = (uint32_t)0xFFFFFFFFUL;
    // enable ext interrupts
    NVIC->ISER[0] = (uint32_t)irqEnBit;
    NVIC->ISER[1] = (uint32_t)(irqEnBit >> 32);
    NVIC->ISER[3] = (uint32_t)(irqEnBit);
    // enable interrupt
    __enable_irq();

}

void CrashCatcher_Entry_For_Irq_Rcvd(ExceptionStackFrame *stackFrame)
{
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);
    CrashCatcherExceptionRegisters crashCatcherExceptionRegisters = {0};
    crashCatcherExceptionRegisters.pc = stackFrame->PC;
    crashCatcherExceptionRegisters.lr = stackFrame->LR;
    crashCatcherExceptionRegisters.msp = (uint32_t)(stackFrame + sizeof(ExceptionStackFrame));

    CrashDump_StartDump((const CrashCatcherExceptionRegisters *)&crashCatcherExceptionRegisters, false, false);
}

void CrashCatcher_Entry(void* pExceptionRegisters)
{
    CrashDump_StartDump(pExceptionRegisters, true, false);
}

void Irq_EmptyHandler(void)
{
    //scan irq

    uint32_t ipsr = 0, irq = 0;
    ipsr = __get_IPSR();
    irq = (ipsr & ISR_NUMBER_MASK) - IRQ_OFFSET;

    // TODO: Decide on the handling.

    ClrPendingIrq(irq);
}

void Irq_FpsWakeup1Handler(uint32_t irq, void* devId)
{
    __asm volatile(
        "TST lr, #4 \n"    // Test EXC_RETURN bit 2
        "ITE EQ \n"        // If-Then-Else
        "MRSEQ r0, MSP \n" // Main Stack Pointer
        "MRSNE r0, PSP \n" // Process Stack Pointer
        "B CrashCatcher_Entry_For_Irq_Rcvd \n");
}

void Irq_FpsWakeup0Handler(uint32_t irq, void* devId)
{
    ClrPendingIrq(irq);

    #if defined (CPU2)
    if(gWakeUp0IrqCount > 0)
    {
        gWakeUp0IrqCount--;
        if(gWakeUp0IrqCount == 0)
        {
            DebugLogLvDbgInfo(cLogCPU2Common, cLogInfo, ("(CPU2) IPC Message Timeout, devID: [0x%x]\n", devId), "32");
            Explicit_CrashCatcher_Entry();
        }
    }
    #endif
}

#ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
void Irq_SysTickHandler(void)
{
    // VM: TODO uncomment this and add logic to identify if we are in FW update.
    // only in case of FW update call FpsDrainTimer
    //FpsTriggerTimer();
    #if defined (CPU2)
    {
        if(gDrainTimerValue != 0)
        {
            FpsDrainTimer();
        }
    }
    #endif
}
#endif
#if defined (CPU0)
void Irq_FpsCpu0MsgHandler(uint32_t irq, void* devId)
{
    // irq = 130
    uint32_t active, pendingDesc;
    if (irq != FP0_INT_NUM)
    {
        // send warning
        return;
    } //else do nothing
    active = GetIrqActive(irq);
    if (active == true)
    {
        pendingDesc = GetIpcPending(IPC_FP0);

        // turn on MsgFiber
        ClrPendingIrq(irq);

        if (pendingDesc & BIT(CPU1toCPU0_DESC))
        {
            IpcIntPendingClr(IPC_FP0, CPU1toCPU0_DESC);
            IpcIntMaskSet(IPC_FP0, CPU1toCPU0_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);

        }
        if (pendingDesc & BIT(CPU2toCPU0_DESC))
        {
            IpcIntPendingClr(IPC_FP0, CPU2toCPU0_DESC);
            IpcIntMaskSet(IPC_FP0, CPU2toCPU0_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);

        }

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        if (pendingDesc & BIT(UPDATE_TIMESTAMP_CPU2_TO_3CPU))
        {
            IpcIntPendingClr(IPC_FP0, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            IpcIntEnableClr(IPC_FP0, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            FpsUpdateTimestamp();
        }
        #endif
    }//else do nothing
     //read pending Descriptor

}
#elif defined (CPU1)
void Irq_FpsCpu1MsgHandler(uint32_t irq, void* devId)
{
    uint32_t active, pendingDesc;
    // irq = 131
    if (irq != FP1_INT_NUM)
    {
        // send warning
        return;
    } //else do nothing
    active = GetIrqActive(irq);

    if (active == true)
    {
        //read Descriptor
        pendingDesc = GetIpcPending(IPC_FP1);

        ClrPendingIrq(irq);

        if (pendingDesc & BIT(CPU0toCPU1_DESC))
        {
            IpcIntPendingClr(IPC_FP1, CPU0toCPU1_DESC);
            IpcIntMaskSet(IPC_FP1, CPU0toCPU1_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);

        }
        if (pendingDesc & BIT(CPU2toCPU1_DESC))
        {
            IpcIntPendingClr(IPC_FP1, CPU2toCPU1_DESC);
            IpcIntMaskSet(IPC_FP1, CPU2toCPU1_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);

        }

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        if (pendingDesc & BIT(UPDATE_TIMESTAMP_CPU2_TO_3CPU))
        {
            IpcIntPendingClr(IPC_FP1, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            IpcIntEnableClr(IPC_FP1, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            FpsUpdateTimestamp();
        }
        #endif
    }

}

#elif defined (CPU2)
void Irq_FpsCpu2MsgHandler(uint32_t irq, void* devId)
{
    // irq = 132
    if (irq != FP2_INT_NUM)
    {
        // send warning
        return;
    } //else do nothing
    uint32_t active, pendingDesc;
    active = GetIrqActive(irq);
    if (active == true)
    {
        pendingDesc = GetIpcPending(IPC_FP2);
        //read Descriptor

        //Turn off isr 132
        //VicIrqDisable(132);
        // turn on MsgFiber
        ClrPendingIrq(irq);

        if (pendingDesc & BIT(CP0toFP_REQ_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CP0toFP_REQ_DESC);
            IpcIntMaskSet(IPC_FP2, CP0toFP_REQ_DESC);
            gIrqFiber_need_resume |= BIT(CP2FPMSG_FIBER);

        }
        if (pendingDesc & BIT(CP1toFP_REQ_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CP1toFP_REQ_DESC);
            IpcIntMaskSet(IPC_FP2, CP1toFP_REQ_DESC);
            gIrqFiber_need_resume |= BIT(CP2FPMSG_FIBER);

        }

        if (pendingDesc & BIT(CP0toFP_RES_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CP0toFP_RES_DESC);
            IpcIntMaskSet(IPC_FP2, CP0toFP_RES_DESC);
            gIrqFiber_need_resume |= BIT(CP2FPMSG_FIBER);

        }
        if (pendingDesc & BIT(CP1toFP_RES_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CP1toFP_RES_DESC);
            IpcIntMaskSet(IPC_FP2, CP1toFP_RES_DESC);
            gIrqFiber_need_resume |= BIT(CP2FPMSG_FIBER);

        }
        if (pendingDesc & BIT(CPU0toCPU2_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CPU0toCPU2_DESC);
            IpcIntMaskSet(IPC_FP2, CPU0toCPU2_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);
            ;
        }
        if (pendingDesc & BIT(CPU1toCPU2_DESC))
        {
            IpcIntPendingClr(IPC_FP2, CPU1toCPU2_DESC);
            IpcIntMaskSet(IPC_FP2, CPU1toCPU2_DESC);
            gIrqFiber_need_resume |= BIT(FP2FPMSG_FIBER);

        }

        #ifdef SUPPORT_UPDATE_TIMESTAMP_IPC
        if (pendingDesc & BIT(UPDATE_TIMESTAMP_CPU2_TO_3CPU))
        {
            IpcIntPendingClr(IPC_FP2, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            IpcIntEnableClr(IPC_FP2, UPDATE_TIMESTAMP_CPU2_TO_3CPU);
            FpsUpdateTimestamp();
        }
        #endif

        if (pendingDesc & BIT(ResetCP2FP))
        {
            IpcIntPendingClr(IPC_FP2, ResetCP2FP);
            // IpcIntMaskSet(IPC_FP2, ResetCP2FP);
            gIrqFiber_need_resume |= BIT(RESET_FIBER);
        }
    }

}

void Irq_FpsCpu2CDMAIrqHandler(uint32_t irq, void* devId)
{
    uint32_t active;
    active = GetIrqActive(CDMA_INT_1_NUM);
    if (active)
    {
        VicIrqDisable(CDMA_INT_1_NUM);
        gIrqFiber_need_resume |= BIT(CDMA_FIBER);
        // FpsCpu2TriggerCDMAFatalErrorHandleFiber();
    }
}
#endif

void Disable_Tcon_Wakeup1()
{
    volatile Tcon_t *tconRegs = (volatile Tcon_t*)TCON_REG_ADDR;
    volatile WakeupCtrl_t* wakeupCtrl = (volatile WakeupCtrl_t*)&tconRegs->wakeupCtrl;
    wakeupCtrl->b.WAKEUP_ENABLE = wakeupCtrl->b.WAKEUP_ENABLE & 0b01;
}

