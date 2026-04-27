// Copyright (c) Microsoft Corporation. All rights reserved.

#include <memory.h>
#include "hsp_top.h"
#include "init_cmd.h"
#include "init_crypto.h"
#include "init_ephemeral_key.h"
#include "init_ipc.h"
#include "init_spdm.h"
#include "init_system.h"
#include "init_tdisp.h"
#include "periodic_task_freertos_static.h"
#include "rot_memory_map.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "ipc/cmd_interface_ipc_admin_static.h"
#include "ipc/cmd_interface_ipc_hsm_static.h"
#include "ipc/ipc_message_handler_static.h"
#include "pcisig/doe/doe_interface_static.h"
#include "splibs/hsprt/riscvcpu.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_interrupt_group_static.h"


/**
 * SOC address of IPC INTC registers
 */
#define IPC_INT_REGISTERS_BLOCK_ADDRESS				0xB0006000

/**
 * IPC queue length
 */
#define IPC_QUEUE_LENGTH							2


/**
 * Admin to HSP IPC channel state
 */
static struct ipc_channel_state ipc_admin_to_hsp_channel_state;


/**
 * Admin to HSP IPC channel
 */
SECTION (".sprtro.ipc_admin_to_hsp_channel")
static struct ipc_channel ipc_admin_to_hsp_channel;

/**
 * HSP to Admin IPC channel state
 */
static struct ipc_channel_state ipc_hsp_to_admin_channel_state;


/**
 * HSP to Admin IPC channel
 */
SECTION (".sprtro.ipc_hsp_to_admin_channel")
struct ipc_channel ipc_hsp_to_admin_channel;

/**
 * HSP to Admin IPC channel state used for Stop Interface handling
 */
static struct ipc_channel_state ipc_hsp_to_admin_stop_intf_channel_state;

/**
 * HSP to Admin IPC channel used for Stop Interface handling
 */
struct ipc_channel ipc_hsp_to_admin_stop_intf_channel;

/**
 * HSM to HSP IPC channel state
 */
static struct ipc_channel_state ipc_hsm_to_hsp_channel_state;


/**
 * HSM to HSP IPC channel
 */
SECTION (".sprtro.ipc_hsm_to_hsp_channel")
static struct ipc_channel ipc_hsm_to_hsp_channel;

/**
 * Interrupt handlers list for interrupt group
 */
static const struct hsp_interrupt_handler *const ipc_irq_list[] = {
	&ipc_admin_to_hsp_channel.base,
	&ipc_hsp_to_admin_channel.base,
	&ipc_hsp_to_admin_stop_intf_channel.base,
	&ipc_hsm_to_hsp_channel.base,
};

/**
 * Interrupt handlers group
 */
static const struct hsp_interrupt_group ipc_irq_group =
	hsp_interrupt_group_static_init (ipc_irq_list, ARRAY_SIZE (ipc_irq_list), false);

/**
 * DOE supported data objects
 */
static const struct doe_data_object_protocol data_object_protocol[] = {
	{DOE_VENDOR_ID_PCISIG, DOE_DATA_OBJECT_TYPE_DOE_DISCOVERY},
	{DOE_VENDOR_ID_PCISIG, DOE_DATA_OBJECT_TYPE_SPDM},
	{DOE_VENDOR_ID_PCISIG, DOE_DATA_OBJECT_TYPE_SECURED_SPDM},
};

/**
 * DOE interface
 */
static const struct doe_interface doe_interface_instance =
	doe_interface_static_init (&doe_spdm_handler.base, data_object_protocol,
	ARRAY_SIZE (data_object_protocol));

/**
 * Command interface for Admin->HSP
 */
static const struct cmd_interface_ipc_admin admin_to_hsp_cmd_interface =
	cmd_interface_ipc_admin_static_init (&dmb, &doe_interface_instance, &tdisp_event_policy.base);

/**
 * Message handler for Admin->HSP channel
 */
static const struct ipc_message_handler ipc_admin_to_hsp_message_handler =
	ipc_message_handler_static_init (&admin_to_hsp_cmd_interface.base, &ipc_admin_to_hsp_channel);

/**
 * Periodic task state
 */
static struct periodic_task_freertos_state admin_to_hsp_task_context;

/**
 * Periodic task handlers array
 */
static const struct periodic_task_handler *const admin_to_hsp_handlers[] = {
	&ipc_admin_to_hsp_message_handler.base,
};

/**
 * Task to listen for Admin->HSP IPC messages
 */
static const struct periodic_task_freertos admin_to_hsp_task =
	periodic_task_freertos_static_init (&admin_to_hsp_task_context, admin_to_hsp_handlers,
	ARRAY_SIZE (admin_to_hsp_handlers), IPC_ADMIN_TO_HSP_TASK_LOG_ID);

/**
 * Statically allocated task control block for the IPC handler task from the Admin core.
 */
static StaticTask_t admin_to_hsp_task_tcb;

/**
 * Statically allocated stack for the IPC handler task from the Admin core
 */
static StackType_t admin_to_hsp_task_stack[ADMIN_TO_HSP_TASK_STACK_WORDS];

/**
 * Statically allocated buffer to read the DER key from the flash
 */
static uint8_t key_buffer[RSA_2K_DER_KEY_MAX_SIZE];

/**
 * Command interface for HSM->HSP
 */
static const struct cmd_interface_ipc_hsm hsm_to_hsp_cmd_interface =
	cmd_interface_ipc_hsm_static_init (&dmb, &rsa_ephemeral_key_manager, key_buffer,
	RSA_2K_DER_KEY_MAX_SIZE, &system_attestation_responder, &shared_hash.base);

/**
 * Message handler for HSM->HSP channel
 */
static const struct ipc_message_handler ipc_hsm_to_hsp_message_handler =
	ipc_message_handler_static_init (&hsm_to_hsp_cmd_interface.base, &ipc_hsm_to_hsp_channel);

/**
 * Periodic task state for HSM->HSM IPC messages
 */
static struct periodic_task_freertos_state hsm_to_hsp_task_context;

/**
 * Periodic task handlers array for HSM->HSP IPC messages
 */
static const struct periodic_task_handler *const hsm_to_hsp_handlers[] = {
	&ipc_hsm_to_hsp_message_handler.base,
};

/**
 * Task to listen for HSM->HSP IPC messages
 */
static const struct periodic_task_freertos hsm_to_hsp_task =
	periodic_task_freertos_static_init (&hsm_to_hsp_task_context, hsm_to_hsp_handlers,
	ARRAY_SIZE (hsm_to_hsp_handlers), IPC_HSM_TO_HSP_TASK_LOG_ID);


/**
 * Statically allocated task control block for the IPC handler task from the HSM core.
 */
static StaticTask_t hsm_to_hsp_task_tcb;

/**
 * Statically allocated stack for the IPC handler task from the HSM core
 */
static StackType_t hsm_to_hsp_task_stack[HSM_TO_HSP_TASK_STACK_WORDS];


/**
 * Initialize IPC infrastructure for communicating with CP cores.
 *
 * @return 0 if successful, otherwise error code
 */
int initialize_ipc ()
{
	IntcIpc_t *ipc_intc_regs = 0;
	int status;

	/* This mapping will remain active until a device reset to ensure IPC drivers can always access
	 * IPC interrupt registers. */
	status = dmb.map_soc_address (&dmb, IPC_INT_REGISTERS_BLOCK_ADDRESS, sizeof (*ipc_intc_regs),
		HSP_DMB_ACCESS_WRITE, (void**) &ipc_intc_regs);
	if (status != 0) {
		return status;
	}

	/* Requester channel for communicating to Admin core. */
	status = ipc_channel_init (&ipc_hsp_to_admin_channel, &ipc_hsp_to_admin_channel_state,
		IPC_HSP_TO_ADMIN_REQUEST_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_22,
		IPC_HSP_TO_ADMIN_RESPONSE_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_23, &dmb,
		IPC_INT_BLOCK_5, ipc_intc_regs);
	if (status != 0) {
		return status;
	}

	/* Requester channel for communicating to Admin core for Stop Interface requests. */
	status = ipc_channel_init (&ipc_hsp_to_admin_stop_intf_channel,
		&ipc_hsp_to_admin_stop_intf_channel_state,
		IPC_HSP_TO_ADMIN_STOP_INTF_REQUEST_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_8,
		IPC_HSP_TO_ADMIN_STOP_INTF_RESPONSE_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_9,
		&dmb, IPC_INT_BLOCK_5, ipc_intc_regs);
	if (status != 0) {
		return status;
	}

	/* Responder channel for handing Admin requests. */
	status = ipc_channel_init (&ipc_admin_to_hsp_channel, &ipc_admin_to_hsp_channel_state,
		IPC_ADMIN_TO_HSP_RESPONSE_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_21,
		IPC_ADMIN_TO_HSP_REQUEST_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_20, &dmb,
		IPC_INT_BLOCK_5, ipc_intc_regs);
	if (status != 0) {
		return status;
	}

	/* Responder channel for handling HSM requests. */
	return ipc_channel_init (&ipc_hsm_to_hsp_channel, &ipc_hsm_to_hsp_channel_state,
		IPC_HSM_TO_HSP_RESPONSE_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_25,
		IPC_HSM_TO_HSP_REQUEST_QUEUE_SOC_ADDRESS, IPC_QUEUE_LENGTH, IPC_DESCRIPTOR_24, &dmb,
		IPC_INT_BLOCK_5, ipc_intc_regs);
}

/**
 * Start the task handlers for receiving IPC requests from CP cores.
 *
 * @return 0 if the task handlers have been started or an error code.
 */
int start_ipc_handlers ()
{
	int status;

	/* Task for handling Admin to HSP requests. */
	status = periodic_task_freertos_init_state (&admin_to_hsp_task);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&admin_to_hsp_task, &admin_to_hsp_task_tcb,
		admin_to_hsp_task_stack, ADMIN_TO_HSP_TASK_STACK_WORDS, "IPC Admin to HSP",
		CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&admin_to_hsp_task);

	/* Task for handling HSM to HSP requests. */
	status = periodic_task_freertos_init_state (&hsm_to_hsp_task);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&hsm_to_hsp_task, &hsm_to_hsp_task_tcb,
		hsm_to_hsp_task_stack, HSM_TO_HSP_TASK_STACK_WORDS, "IPC HSM to HSP",
		CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&hsm_to_hsp_task);

	/* Enable IPC interrupts in HSP. */
	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_SYS_INT_INTEN_LSB,
		&ipc_irq_group.base);
	if (status != 0) {
		return status;
	}

	return hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_SYS_INT_INTEN_LSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
}
