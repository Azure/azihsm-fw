// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include "hsp_top.h"
#include "reset_counter_init.h"
#include "task_priority.h"
#include "cmd_interface/cmd_background_handler_static.h"
#include "cmd_interface/cmd_channel_handler_static.h"
#include "cmd_interface/cmd_channel_i2c_dw_apb_multimaster_static.h"
#include "cmd_interface/cmd_device_hsp_freertos_static.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "common/array_size.h"
#include "common/msft_device_id.h"
#include "drivers/i2c_dw_apb_multimaster_static.h"
#include "init/init_attestation.h"
#include "init/init_cmd.h"
#include "init/init_crypto.h"
#include "init/init_firmware.h"
#include "init/init_i2c.h"
#include "init/init_log.h"
#include "init/init_manifest.h"
#include "init/init_system.h"
#include "init/task_log_id.h"
#include "init/task_stack_size.h"
#include "mctp/cmd_interface_mctp_control.h"
#include "mctp/cmd_interface_protocol_mctp_msft_vdm_static.h"
#include "mctp/cmd_interface_protocol_mctp_static.h"
#include "recovery/cmd_interface_recovery.h"
#include "recovery/cmd_interface_recovery_static.h"
#include "trap/hsp_interrupt.h"


/**
 * Variable context for the background command handler.
 */
static struct cmd_background_handler_state background_handler_context;

/**
 * The system attestation responder instance
 *
 * TODO:  Create a static initializer for this type.
 */
struct attestation_responder system_attestation_responder;

/**
 * Handler for processing commands in the background.
 */
static const struct cmd_background_handler background_handler =
	cmd_background_handler_static_init (&background_handler_context, &system_attestation_responder,
	&shared_hash.base, &dice_key_manager, &cmd_background_task.base);

/**
 * List of handlers for the background command task.
 */
static const struct event_task_handler *background_handlers[] = {&background_handler.base_event};

/**
 * Variable context for the background command processing task.
 */
static struct event_task_freertos_state cmd_background_context;

/**
 * Task for processing commands in the background.
 */
const struct event_task_freertos cmd_background_task =
	event_task_freertos_static_init (&cmd_background_context, &system_mgr, background_handlers,
	ARRAY_SIZE (background_handlers));

/**
 * Statically allocated task control block for the background command handler task.
 */
static StaticTask_t cmd_background_task_tcb;

/**
 * Statically allocated stack for the background command handler task.
 */
static StackType_t cmd_background_task_stack[CMD_BACKGROUND_TASK_STACK_WORDS];

/**
 * The handler for Cerberus protocol messages.
 */
static const struct cmd_interface_recovery cerberus_handler =
	cmd_interface_recovery_static_init (&device_manager, &fw_handler.base.base_ctrl,
	&firmware_version, CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_DEVICE_ID_MANTICORE,
	CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_SUBSYSTEM_DEVICE_ID_DC_SCM, &system_attestation_responder,
	&dice_key_manager, &background_handler.base_cmd, &device_cmd.base.base);

/**
 * The handler for MCTP control messages.
 */
static struct cmd_interface_mctp_control mctp_control_handler;

/**
 * Protocol handler for MSFT MCTP VDM messages.
 */
static const struct cmd_interface_protocol_mctp_msft_vdm msft_vdm_protocol =
	cmd_interface_protocol_mctp_msft_vdm_static_init (&device_manager);

/**
 * List of message types supported by the MSFT MCTP VDM handler.
 */
static const struct cmd_interface_multi_handler_msg_type msft_vdm_message_types[] = {
	cmd_interface_multi_handler_msg_type_static_init (0, &cerberus_handler.base),
};

/**
 * Handler for received MSFT MCTP VDM request messages.
 */
static const struct cmd_interface_multi_handler msft_vdm_handler =
	cmd_interface_multi_handler_static_init (&msft_vdm_protocol.base, msft_vdm_message_types,
	ARRAY_SIZE (msft_vdm_message_types));

/**
 * Protocol handler for MCTP messages.
 */
static const struct cmd_interface_protocol_mctp mctp_protocol =
	cmd_interface_protocol_mctp_static_init;

/**
 * List of MCTP message types that are supported.
 */
static const struct cmd_interface_multi_handler_msg_type mctp_message_types[] = {
	cmd_interface_multi_handler_msg_type_static_init (MCTP_BASE_PROTOCOL_MSG_TYPE_CONTROL_MSG,
		&mctp_control_handler.base),
	cmd_interface_multi_handler_msg_type_static_init (MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF,
		&msft_vdm_handler.base),
};

/**
 * Variable context for the MCTP transport layer.
 */
static struct mctp_interface_state mctp_transport_context;

/**
 * Handler for received MCTP request messages.
 */
static const struct cmd_interface_multi_handler mctp_handler =
	cmd_interface_multi_handler_static_init (&mctp_protocol.base, mctp_message_types,
	ARRAY_SIZE (mctp_message_types));

/**
 * The MCTP transport layer handler for the I2C command channel.
 */
const struct mctp_interface mctp_transport = mctp_interface_static_init (&mctp_transport_context,
	&mctp_handler, &device_manager, &fips_i2c.base_channel);

/**
 * Handler for received system commands.
 */
static const struct cmd_channel_handler system_cmd_handler =
	cmd_channel_handler_static_init (&fips_i2c.base_channel, &mctp_transport, NULL);

/**
 * List of handlers for the system command processing task.
 */
static const struct periodic_task_handler *system_cmd_handlers[1] = {&system_cmd_handler.base};

/**
 * Variable context for the system command processing task.
 */
static struct periodic_task_freertos_state system_cmd_task_context;

/**
 * The system command interface processing task.
 */
const struct periodic_task_freertos system_cmd_task =
	periodic_task_freertos_static_init (&system_cmd_task_context, system_cmd_handlers,
	ARRAY_SIZE (system_cmd_handlers), SYSTEM_CMD_TASK_LOG_ID);

/**
 * Statically allocated FreeRTOS task control block for the I2C command handler task.
 */
static StaticTask_t system_cmd_task_tcb;

/**
 * Statically allocated stack for the I2C command handler task.
 */
static StackType_t system_cmd_task_stack[SYSTEM_CMD_TASK_STACK_WORDS];


/**
 * Initialize the I2C channel for recovery receiving commands .
 *
 * @return 0 if the command channel was successfully initialized or an error code.
 */
int initialize_cmd_interface_recovery ()
{
	int status;

	status = initialize_i2c_interface (NULL);
	if (status != 0) {
		goto done;
	}

	status = attestation_responder_init_no_aux (&system_attestation_responder, &dice_key_manager,
		&shared_hash.base, &shared_ecc.base, &shared_rng.base, &pcr_storage,
		CERBERUS_PROTOCOL_PROTOCOL_VERSION,	CERBERUS_PROTOCOL_PROTOCOL_VERSION);
	if (status != 0) {
		goto done;
	}

	status = cmd_background_handler_init_state (&background_handler);
	if (status != 0) {
		goto done;
	}

	status = event_task_freertos_init_state (&cmd_background_task);
	if (status != 0) {
		goto done;
	}

	status = cmd_interface_mctp_control_init (&mctp_control_handler, &device_manager,
		CERBERUS_PROTOCOL_MSFT_PCI_VID, CERBERUS_PROTOCOL_PROTOCOL_VERSION);
	if (status != 0) {
		goto done;
	}

	status = mctp_interface_init_state (&mctp_transport);
	if (status != 0) {
		goto done;
	}

	status = periodic_task_freertos_init_state (&system_cmd_task);

done:

	return status;
}

/**
 * Start the command interface tasks.
 *
 * @return 0 if all command processing tasks were successfully started or an error code.
 */
int start_cmd_interface ()
{
	int status;

	status = event_task_freertos_allocate_static (&cmd_background_task, &cmd_background_task_tcb,
		cmd_background_task_stack, CMD_BACKGROUND_TASK_STACK_WORDS, "CmdBgnd",
		CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&system_cmd_task, &system_cmd_task_tcb,
		system_cmd_task_stack, SYSTEM_CMD_TASK_STACK_WORDS, "MCTP_LOOP", CERBERUS_PRIORITY_HIGH);
	if (status != 0) {
		return status;
	}

	event_task_freertos_start (&cmd_background_task);
	periodic_task_freertos_start (&system_cmd_task);

	status = start_i2c_interface ();
	if (status != 0) {
		return status;
	}

	return 0;
}
