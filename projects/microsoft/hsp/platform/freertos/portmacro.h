// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_FREERTOS_PORTMACRO_H_
#define HSP_FREERTOS_PORTMACRO_H_

#include <stddef.h>
#include <stdint.h>
#include "RISC-V/portmacro.h"
#include "trap/hsp_trap.h"

//
// Redefine port macro's for HSP implementation
//

extern void freertos_isr_update_yield (BaseType_t priority_woken);

/* NOTE: If implementing FreeRTOS on a RISCV core for SUPERVISOR or USER mode traps, ensure the
 * interrupt enable/disable macro's are redefined with the proper mstatus bits (s/uie). */

// Use optimized context switching method
#undef portEND_SWITCHING_ISR
#define portEND_SWITCHING_ISR(xSwitchRequired)		freertos_isr_update_yield (xSwitchRequired)

// We allow nested interrupts, allow for disabling interrupts within an ISR context
#undef portSET_INTERRUPT_MASK_FROM_ISR
#undef portCLEAR_INTERRUPT_MASK_FROM_ISR
#define portSET_INTERRUPT_MASK_FROM_ISR()							hsp_trap_mstatus_mie_disable()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedStatusValue)\
	hsp_trap_mstatus_write (uxSavedStatusValue)


#endif	/* HSP_FREERTOS_PORTMACRO_H_ */
