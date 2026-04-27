// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTRUSION_IRQ_HANDLER_STATIC_H_
#define HSP_INTRUSION_IRQ_HANDLER_STATIC_H_

#include "intrusion/hsp_intrusion_irq_handler.h"
#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
void hsp_intrusion_irq_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* hsp_intrusion_irq_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void hsp_intrusion_irq_handler_execute (const struct periodic_task_handler *handler);

bool hsp_intrusion_irq_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);


/**
 * Constant initializer for the task API.
 */
#define	HSP_INTRUSION_IRQ_HANDLER_API_INIT  { \
		.prepare = hsp_intrusion_irq_handler_prepare, \
		.get_next_execution = hsp_intrusion_irq_handler_get_next_execution, \
		.execute = hsp_intrusion_irq_handler_execute \
	}


/**
 * Initialize a static handler for HSP RTC interrupts.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the interrupt handler.
 * @param rtc_ptr The HSP RTC generating the interrupts.
 * @param manager_ptr The instance of intrusion manager.
 * @param intrusion_state_ptr The instance of intrusion state HSP.
 */
#define	hsp_intrusion_irq_handler_static_init(state_ptr, rtc_ptr, manager_ptr, \
		intrusion_state_ptr)	{ \
		.base = HSP_INTRUSION_IRQ_HANDLER_API_INIT, \
		.base_irq = \
			hsp_interrupt_handler_static_init (hsp_intrusion_irq_handler_handle_interrupt), \
		.state = state_ptr, \
		.rtc = rtc_ptr, \
		.manager = manager_ptr, \
		.intrusion_state = intrusion_state_ptr, \
	}


#endif	/* HSP_INTRUSION_IRQ_HANDLER_STATIC_H_ */
