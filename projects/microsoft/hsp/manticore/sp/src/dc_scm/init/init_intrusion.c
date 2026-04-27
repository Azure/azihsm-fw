// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include "init_attestation.h"
#include "init_cmd.h"
#include "init_crypto.h"
#include "init_intrusion.h"
#include "init_log.h"
#include "init_system.h"
#include "manticore_pcr.h"
#include "periodic_task_freertos_static.h"
#include "platform.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "dc_scm/manticore_pcr.h"
#include "intrusion/hsp_intrusion_irq_handler_static.h"
#include "intrusion/intrusion_logging.h"
#include "intrusion/intrusion_manager_msft.h"
#include "logging/init_logging.h"
#include "system/periodic_task.h"
#include "system/real_time_clock_hsp.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_interrupt_group_static.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Maximum statically registered external entities for mctp notification.
 */
#define MCTP_NOTIFIER_MAX_REGISTERED_EIDS_COUNT	1

/**
 * MCTP Notifier response timeout value in ms.
 */
#define MCTP_NOTIFIER_RESP_TIMEOUT				100


/**
 * Variable context for the RTC HW interrupt handler.
 */
static struct hsp_intrusion_irq_handler_state rtc_irq_context;

/**
 * Intrusion manager for handling intrusion events.
 * TODO: need to make static initialization in the near future.
 */
struct intrusion_manager_msft intrusion_manager;

/**
 * Keep track of whether the chassis has been opened.
 */
const struct intrusion_state_hsp intrusion_state = intrusion_state_hsp_static_init (&system_rtc);

/**
 * Handler for RTC HW interrupts.
 */
static const struct hsp_intrusion_irq_handler rtc_hsp_irq =
	hsp_intrusion_irq_handler_static_init (&rtc_irq_context, &system_rtc, &intrusion_manager.base,
	&intrusion_state);

/**
 * List of handlers for the RTC interrupt handling tasks.
 */
static const struct periodic_task_handler *const rtc_irq_handlers[] = {&rtc_hsp_irq.base};

/**
 * Variable context for the RTC interrupt handling tasks.
 */
static struct periodic_task_freertos_state rtc_irq_task_context;

/**
 * Task context for handling RTC interrupts.
 */
static const struct periodic_task_freertos rtc_irq_task =
	periodic_task_freertos_static_init (&rtc_irq_task_context, rtc_irq_handlers,
	ARRAY_SIZE (rtc_irq_handlers), RTC_IRQ_TASK_LOG_ID);

/**
 * Statically allocated task control block for RTC intrusion IRQ handler task.
 */
static StaticTask_t rtc_irq_task_tcb;

/**
 * Statically allocated stack for the TTC intrusion IRQ handler task.
 */
static StackType_t rtc_irq_task_stack[RTC_IRQ_TASK_STACK_WORDS];

/**
 * The MCTP notifier maximum supported EID list for notification registration.
 * This list contains pre-registered EIDs as its initialized parameter itself,
 * which can't be unregistered/removed from MCTP notifier registered EID list.
 */
static const uint8_t notifier_eid_list[MCTP_NOTIFIER_MAX_REGISTERED_EIDS_COUNT] = {
	MCTP_BASE_PROTOCOL_NULL_EID
};

/**
 * The MCTP notifier message buffer for sending notification request to any external entities.
 */
uint8_t mctp_msg_buffer[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN];

/**
 * The MSFT MCTP notifier instance used for sending notification request to any external endpoints.
 */
const struct mctp_notifier_msft_const_list mctp_notifier_msft =
	mctp_notifier_msft_const_list_static_init (&mctp_transport.base, notifier_eid_list,
	MCTP_NOTIFIER_MAX_REGISTERED_EIDS_COUNT, mctp_msg_buffer, MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN,
	MCTP_NOTIFIER_RESP_TIMEOUT);

/**
 * Authorized execution context for resetting intrusion state.
 */
const struct authorized_execution_reset_intrusion reset_intrusion_execution =
	authorized_execution_reset_intrusion_static_init (&intrusion_manager.base);


/**
 * Initialize intrusion components.
 *
 * @return 0 if the initialization of intrusion components succeeded or an error code.
 */
int initialize_intrusion (void)
{
	int status;

	/* TODO: we probably need to think about when/if we should be clearing interrupts on init. */
	status = hsp_intrusion_irq_handler_init_state (&rtc_hsp_irq, (reset_source == RESET_POR));
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_init_state (&rtc_irq_task);
	if (status != 0) {
		return status;
	}

	status = intrusion_manager_msft_init (&intrusion_manager, &intrusion_state.base,
		&shared_hash.base, &pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_CHASSIS_INTRUSION,
		&mctp_notifier_msft.base.base);

	return status;
}

/**
 * Register and enable interrupt for intrusion.
 *
 * @return 0 if the interrupt register and enable succeeded or an error code.
 */
static int init_intrusion_enable_interrupt (void)
{
	int status;

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_RTC_INTEN_MSB,
		&rtc_hsp_irq.base_irq);
	if (status != 0) {
		return status;
	}

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_RTC_INTEN_MSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	return status;
}

/**
 * Initialize the intrusion IRQ task state and then start the task.
 *
 * @return 0 if the intrusion IRQ task state initialization and start succeeded
 * or an error code.
 */
static int init_intrusion_start_task (void)
{
	int status;

	status = periodic_task_freertos_allocate_static (&rtc_irq_task, &rtc_irq_task_tcb,
		rtc_irq_task_stack, RTC_IRQ_TASK_STACK_WORDS, "RTC_IRQ", CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&rtc_irq_task);

	return 0;
}

/**
 * This function enables intrusion interrupt, initialize intrusion data structures,
 * starts the IRQ handling task, and checks intrusion state at power on.
 *
 * @return 0 if operations were enabled successfully or an error code.
 */
int start_intrusion (void)
{
	int status;

	status = init_intrusion_enable_interrupt ();
	if (status != 0) {
		return status;
	}

	status = init_intrusion_start_task ();

	return status;
}
