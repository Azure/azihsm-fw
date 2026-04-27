// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTERRUPT_H_
#define HSP_INTERRUPT_H_

#include <stdbool.h>
#include <stdint.h>
#include "trap/hsp_interrupt_handler.h"
#include "trap/irq_error.h"


/**
 * The IRQ levels defined for the HSP interrupt controller.
 */
enum hsp_interrupt_irq_level {
	HSP_INTERRUPT_IRQ_LEVEL_IRQ = 0,	/**< Base IRQ level */
	HSP_INTERRUPT_IRQ_LEVEL_FIQ = 1,	/**< Fast, higher priority IRQ level */
	HSP_INTERRUPT_IRQ_LEVEL_MAX,
};


/* Task API */

int hsp_interrupt_init (bool clear_outstanding);

int hsp_interrupt_register (unsigned intr_bit, const struct hsp_interrupt_handler *handler);
int hsp_interrupt_unregister (unsigned intr_bit);

/* Interrupt configuration */

bool hsp_interrupt_is_enabled (unsigned intr_bit, enum hsp_interrupt_irq_level irql);
bool hsp_interrupt_is_pending (unsigned intr_bit);
bool hsp_interrupt_is_pending_for_irql (unsigned intr_bit, enum hsp_interrupt_irq_level irql);
int hsp_interrupt_enable (unsigned intr_bit, enum hsp_interrupt_irq_level irql);
int hsp_interrupt_disable (unsigned intr_bit, enum hsp_interrupt_irq_level irql);

/* ISR API */

int hsp_interrupt_get_irql (uintptr_t param);


#endif	/* HSP_INTERRUPT_H_ */
