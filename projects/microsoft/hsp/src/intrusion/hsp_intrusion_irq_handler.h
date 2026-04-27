// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTRUSION_IRQ_HANDLER_H_
#define HSP_INTRUSION_IRQ_HANDLER_H_

#include <stdbool.h>
#include "platform_api.h"
#include "intrusion/intrusion_manager.h"
#include "intrusion/intrusion_state_hsp.h"
#include "system/periodic_task.h"
#include "system/real_time_clock_hsp.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Variable context for the HSP RTC interrupt handler.
 */
struct hsp_intrusion_irq_handler_state {
	platform_semaphore rtc_event;	/**< Signal that an interrupt event occurred. */
	platform_timer timer;			/**< Timer used for intrusion time-delay callback. */
};

/**
 * Interrupt handler for the HSP RTC.
 */
struct hsp_intrusion_irq_handler {
	struct periodic_task_handler base;					/**< Task handler for processing interrupt events. */
	struct hsp_interrupt_handler base_irq;				/**< IRQ handler for processing hardware events. */
	struct hsp_intrusion_irq_handler_state *state;		/**< Variable context for the IRQ handler. */
	const struct real_time_clock_hsp *rtc;				/**< The RTC generating the interrupts. */
	struct intrusion_manager *manager;					/**< Instance of intrusion manager. */
	const struct intrusion_state_hsp *intrusion_state;	/**< Instance of intrusion state HSP. */
};


void hsp_intrusion_irq_handler_handle_intrusion (const struct hsp_intrusion_irq_handler *handler);
int hsp_intrusion_irq_handler_init (struct hsp_intrusion_irq_handler *handler,
	struct hsp_intrusion_irq_handler_state *state, const struct real_time_clock_hsp *rtc,
	bool clear_irq, struct intrusion_manager *manager,
	const struct intrusion_state_hsp *intrusion_state);
int hsp_intrusion_irq_handler_init_state (const struct hsp_intrusion_irq_handler *handler,
	bool clear_irq);
void hsp_intrusion_irq_handler_release (const struct hsp_intrusion_irq_handler *handler);


#endif	/* HSP_INTRUSION_IRQ_HANDLER_H_ */
