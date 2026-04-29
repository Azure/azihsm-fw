// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once
#include "SysTypes.h"
/*
 * IRQ callback registration
 */
#include "vicommon.h"
#include "assert.h"
#include "cm7ikmcu.h"
#include "platform.h"
#include "LoggingDebug.h"
#define MAX_FP_IABR_NUMBER 0x8UL
#define MAX_IRQ_NUM 0x100UL
#define IRQ_OFFSET 0x10UL
#define ISR_NUMBER_MASK 0xFFUL
#define MAX_WAKEUP0_HSM_IRQ_TIMEOUT_COUNT 0x5
typedef void (*irqHandler_t)(uint32_t, void*);
//typedef int (*irq_check_t)(int, void*);
extern struct irqRegistration_t
{
    uint32_t irq;
    irqHandler_t handler;
    uint32_t irqflags;
    void* devId;
} irqRegistration_t;
extern struct irqRegistration_t irqRegs[MAX_IRQ_NUM];

//uint32_t irq_cnt=0;
uint8_t Irq_Request(uint32_t irq, irqHandler_t handler, uint32_t irqflags, void* devId);
void Irq_Init(uint64_t irqEnBit);
/*
 * IRQ handlers
 */


#define EXT_IRQ_HANDLER(id)                                    \
    void __attribute__((weak)) ext_irq ## id ## _handler(void) \
    {                                                          \
        if (irqRegs[(id)].handler != NULL)                     \
        {                                                      \
            irqRegs[(id)].handler((id), irqRegs[(id)].devId);  \
        }                                                      \
    }

#if defined (CPU0)
extern void FpsCpu0TriggerFiber(void);
void Irq_FpsCpu0MsgHandler(uint32_t irq, void* devId);
#elif defined (CPU1)
extern void FpsCpu1TriggerFiber(void);
void Irq_FpsCpu1MsgHandler(uint32_t irq, void* devId);
#elif defined (CPU2)
extern void FpsCpu2TriggerFiber(uint8_t CP2FPResume);
extern void FpsCpu2TriggerCDMAFatalErrorHandleFiber(void);
void Irq_FpsCpu2MsgHandler(uint32_t irq, void* devId);
void Irq_FpsCpu2CDMAIrqHandler(uint32_t irq, void* devId);
extern void FpsDrainTimer();
#endif
void Irq_FpsWakeup0Handler(uint32_t irq, void* devId);
void Irq_FpsWakeup1Handler(uint32_t irq, void* devId);
#ifdef SUPPORT_UPDATE_TIMESTAMP
extern void FpsUpdateTimestamp(void);
extern void FpsTriggerTimer(void);
#endif
void Irq_EmptyHandler(void);
void Disable_Tcon_Wakeup1(void);

EXT_IRQ_HANDLER(59)    // CDMA fatal error irq
EXT_IRQ_HANDLER(88)    // internal TCON Wakeup 0 irq
EXT_IRQ_HANDLER(89)    // internal TCON Wakeup 1 irq
EXT_IRQ_HANDLER(128)   // internal cp0 error irq
EXT_IRQ_HANDLER(129)   // internal cp1 error irq
EXT_IRQ_HANDLER(130)   // internal fp0 error irq
EXT_IRQ_HANDLER(131)   // internal fp1 error irq
EXT_IRQ_HANDLER(132)   // internal fp2 error irq
EXT_IRQ_HANDLER(133)   // internal hsp error irq
