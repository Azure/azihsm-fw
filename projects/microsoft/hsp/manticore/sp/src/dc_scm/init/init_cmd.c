// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include "hsp_top.h"
#include "init_attestation.h"
#include "init_cmd.h"
#include "init_crypto.h"
#include "init_firmware.h"
#include "init_host.h"
#include "init_i2c.h"
#include "init_intrusion.h"
#include "init_log.h"
#include "init_manifest.h"
#include "init_spdm.h"
#include "init_system.h"
#include "manticore_pcr.h"
#include "manticore_sticky_regs.h"
#include "rot_memory_map.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "attestation/attestation_logging.h"
#include "cmd_interface/cmd_background_handler_static.h"
#include "cmd_interface/cmd_channel_handler_static.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "cmd_interface/cmd_interface_system.h"
#include "common/array_size.h"
#include "common/auth_token_static.h"
#include "common/msft_device_id.h"
#include "common/unused.h"
#include "crypto/signature_verification_ecc_static.h"
#include "fips/cmd_interface_msft_fips_on_demand_self_test_static.h"
#include "firmware/manticore_fw_keys.h"
#include "logging/init_logging.h"
#include "mctp/cmd_interface_mctp_control.h"
#include "mctp/cmd_interface_protocol_mctp_msft_vdm_static.h"
#include "mctp/cmd_interface_protocol_mctp_static.h"
#include "mctp/mctp_interface_static.h"
#include "mctp/msg_transport_mctp_message_static.h"
#include "msft_protocol/cmd_interface_msft_base_static.h"
#include "msft_protocol/cmd_interface_msft_manticore_static.h"
#include "msft_protocol/cmd_interface_msft_rot_static.h"
#include "msft_protocol/cmd_interface_protocol_msft_cmd_set_static.h"
#include "msft_protocol/cmd_interface_protocol_msft_static.h"
#include "msft_protocol/manticore_commands.h"
#include "msft_protocol/msft_base_commands_static.h"
#include "msft_protocol/msft_mctp_protocol.h"
#include "msft_protocol/rot_commands.h"
#include "msft_protocol/temperature_sensor_cluster_static.h"
#include "spdm/cmd_interface_spdm.h"
#include "splibs/hsprt/riscvcpu.h"
#include "sprt/manticore_sprt.h"
#include "system/device_unlock_token_static.h"
#include "system/real_time_clock_hsp_static.h"
#include "system/secure_device_unlock_policy_static.h"
#include "trap/hsp_interrupt.h"

#ifdef MANTICORE_ENABLE_ACVP
#include "init_acvp.h"
#include "fips/cmd_interface_msft_fips_acvp_static.h"
#endif

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#include "sp_boot.h"
#include "fips/cmd_interface_msft_fips_cmvp_static.h"
#include "fips/cmvp_test_manticore_static.h"
#endif


/**
 * Data populated by 1SP that can be used with local static initialization.
 */
static const struct manticore_1sp_shared_data *const sp1_shared_static =
	(struct manticore_1sp_shared_data*) SP1_SHARED_ADDRESS;

/**
 * The system attestation responder instance
 *
 * TODO:  Create a static initializer for this type.
 */
struct attestation_responder system_attestation_responder;

/**
 * Variable context for the background command handler.
 */
static struct cmd_background_handler_state background_handler_context;

/**
 * Handler for processing commands in the background.
 */
static const struct cmd_background_handler background_handler =
	cmd_background_handler_static_init (&background_handler_context, &system_attestation_responder,
	&shared_hash.base, &dice_key_manager, &cmd_background_task.base);

/**
 * List of handlers for the background command task.
 */
static const struct event_task_handler *const background_handlers[] = {
	&background_handler.base_event
};

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
 *
 * TODO:  Create a static initializer for this type.  In the meantime, mark as RO after init.
 */
SECTION (".sprtro.cerberus_handler")
static struct cmd_interface_system cerberus_handler;

/**
 * Supported protocol versions for the Base MSFT command set.
 */
static const uint16_t msft_base_version[] = {MSFT_BASE_PROTOCOL_VERSION};

/**
 * Supported protocol versions for the RoT MSFT command set.
 */
static const uint16_t msft_rot_version[] = {ROT_PROTOCOL_VERSION};

/**
 * Supported protocol versions for the Manticore MSFT command set.
 */
static const uint16_t msft_manticore_version[] = {MANTICORE_PROTOCOL_VERSION};

/**
 * Supported protocol versions for the FIPS MSFT command set.
 */
static const uint16_t msft_fips_version[] = {FIPS_PROTOCOL_VERSION};

/**
 * List of MSFT command sets supported by the device.
 */
static const struct msft_base_supported_command_set msft_cmd_sets[] = {
	msft_base_supported_command_set_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_BASE,
		msft_base_version, ARRAY_SIZE (msft_base_version)),
	msft_base_supported_command_set_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT,
		msft_rot_version, ARRAY_SIZE (msft_rot_version)),
	msft_base_supported_command_set_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_MANTICORE,
		msft_manticore_version, ARRAY_SIZE (msft_manticore_version)),
	msft_base_supported_command_set_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_FIPS,
		msft_fips_version, ARRAY_SIZE (msft_fips_version)),
};

/**
 * Variable context for the on-die temperature sensor.
 */
struct temperature_sensor_tsen_state tsen_context;

/**
 * On-die temperature sensor with the ability to read several different regions of the chip.
 */
const struct temperature_sensor_tsen tsen =
	temperature_sensor_tsen_static_init (&tsen_context, &dmb);

/**
 * List of temperature sensors available from the device.
 *
 * Currently only the middle die is reported, but this list can be expanded if there is ever a need
 * to read the other temperature sensors.
 */
static const struct temperature_sensor *const temp_sensors[] = {&tsen.middle_die.base};

/**
 * Manager for all temperature sensors available from the device.
 */
static const struct temperature_sensor_cluster temp_manager =
	temperature_sensor_cluster_static_init (temp_sensors, ARRAY_SIZE (temp_sensors));

/**
 * Handler for the Base MSFT command set.
 */
static const struct cmd_interface_msft_base msft_base_handler =
	cmd_interface_msft_base_static_init (msft_cmd_sets, ARRAY_SIZE (msft_cmd_sets), &device_manager,
	&temp_manager, NULL);

/**
 * Variable context for the unlock authorization token.
 */
static struct signature_verification_ecc_state unlock_auth_verify_context;

/**
 * Verification handler for the unlock authorization token.
 */
static const struct signature_verification_ecc unlock_auth_verify =
	signature_verification_ecc_static_init (&unlock_auth_verify_context, &shared_ecc.base);

/**
 * Variable context for the unlock authorization token.
 */
static struct auth_token_state unlock_auth_context;

/**
 * Buffer to use for managing the device unlock token.
 */
static uint8_t unlock_token_buffer[MANTICORE_MAX_UNLOCK_TOKEN_LENGTH];

/**
 * Authorization token for device unlock operations.  Unlock tokens are valid for 24 hours.
 */
static const struct auth_token unlock_auth = auth_token_static_init (&unlock_auth_context,
	&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
	KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared_static->fw_keys,
	MANTICORE_FW_KEYS_DEBUG_UNLOCK_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
	&unlock_auth_verify.base, MANTICORE_UNLOCK_TOKEN_DATA_LENGTH, DEVICE_UNLOCK_TOKEN_NONCE_LENGTH,
	ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384, MANTICORE_AUTH_TOKEN_EXPIRATION,
	unlock_token_buffer, sizeof (unlock_token_buffer));

/**
 * Manager for the active device unlock token.
 */
static const struct device_unlock_token unlock_token =
	device_unlock_token_static_init (&unlock_auth, &graceful_shutdown.base_device,
	DICE_OID_MANTICORE, MANTICORE_OID_LENGTH, MANTICORE_1SP_UNLOCK_COUNTER_LENGTH,
	HASH_TYPE_SHA384);

/**
 * Handler for device unlock operations.
 */
static const struct secure_device_unlock_policy device_unlock =
	secure_device_unlock_policy_static_init (&unlock_token, &security_mgr.base.base);

/**
 * Handler for the RoT MSFT command set.
 */
#ifdef CMD_SUPPORT_PLATFORM_RESET
static const struct cmd_interface_msft_rot msft_rot_handler =
	cmd_interface_msft_rot_static_init (&device_unlock.base, &system_rtc.base,
	&background_handler.base_cmd, &intrusion_state.base, &mctp_notifier_msft.base.base, NULL);
#else
static const struct cmd_interface_msft_rot msft_rot_handler =
	cmd_interface_msft_rot_static_init (&device_unlock.base, &system_rtc.base, NULL,
	&intrusion_state.base, &mctp_notifier_msft.base.base, NULL);
#endif

/**
 * Handler for the Manticore MSFT command set.
 */
static const struct cmd_interface_msft_manticore msft_manticore_handler =
	cmd_interface_msft_manticore_static_init (&graceful_shutdown.base_ctrl);

#ifdef MANTICORE_ENABLE_ACVP
/**
 * Handler for ACVP commands in the Manticore FIPS command set.
 */
static const struct cmd_interface_msft_fips_acvp msft_fips_acvp_handler =
	cmd_interface_msft_fips_acvp_static_init (&acvp.base);
#endif

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
/**
 * Handler for CMVP tests.
 */
static const struct cmvp_test_manticore cmvp = cmvp_test_manticore_static_init (&dmb, &cmvp_test,
	CMVP_TEST_CASE_ADDRESS);

/**
 * Handler for CMVP commands in the Manticore FIPS command set.
 */
static const struct cmd_interface_msft_fips_cmvp msft_fips_cmvp_handler =
	cmd_interface_msft_fips_cmvp_static_init (&cmvp.base, &background_handler.base_cmd);
#endif

/**
 * Manager for execution of FIPS self-tests at run-time.
 */
const struct fips_self_test_manticore self_test =
	fips_self_test_manticore_static_init (&shared_rng.base, &background_handler.base_cmd,
	MANTICORE_STICKY_REG (MANTICORE_ON_DEMAND_SELF_TEST));

/**
 * Handler for on-demand self-test commands in the Manticore FIPS command set.
 */
static const struct cmd_interface_msft_fips_on_demand_self_test msft_fips_self_test_handler =
	cmd_interface_msft_fips_on_demand_self_test_static_init (&self_test.base);

/**
 * Protocol handler for parsing command codes within a MCTP vendor defined protocol command set.
 */
static const struct cmd_interface_protocol_msft_cmd_set msft_protocol_cmd_set =
	cmd_interface_protocol_msft_cmd_set_static_init;

/**
 * List of FIPS handlers for supported commands.
 */
static const struct cmd_interface_multi_handler_msg_type fips_command_set[] = {
	CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_SUPPORTED_MSG_TYPES (&msft_fips_self_test_handler),

#ifdef MANTICORE_ENABLE_ACVP
	CMD_INTERFACE_MSFT_FIPS_ACVP_SUPPORTED_MSG_TYPES (&msft_fips_acvp_handler),
#endif

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	CMD_INTERFACE_MSFT_FIPS_CMVP_SUPPORTED_MSG_TYPES (&msft_fips_cmvp_handler),
#endif
};

/**
 * Handler for MSFT vender defined FIPS request messages.
 */
static const struct cmd_interface_multi_handler msft_fips_handler =
	cmd_interface_multi_handler_static_init (&msft_protocol_cmd_set.base, fips_command_set,
	ARRAY_SIZE (fips_command_set));

/**
 * Protocol handler for the MCTP vendor defined protocol.
 */
static const struct cmd_interface_protocol_msft msft_protocol =
	cmd_interface_protocol_msft_static_init;

/**
 * List of MSFT handlers for the supported command sets.
 */
static const struct cmd_interface_multi_handler_msg_type msft_command_sets[] = {
	cmd_interface_multi_handler_msg_type_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_BASE,
		&msft_base_handler.base),
	cmd_interface_multi_handler_msg_type_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT,
		&msft_rot_handler.base),
	cmd_interface_multi_handler_msg_type_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_MANTICORE,
		&msft_manticore_handler.base),
	cmd_interface_multi_handler_msg_type_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_FIPS,
		&msft_fips_handler.base),
};

/**
 * Handler for MSFT vendor defined request messages.
 */
static const struct cmd_interface_multi_handler msft_handler =
	cmd_interface_multi_handler_static_init (&msft_protocol.base, msft_command_sets,
	ARRAY_SIZE (msft_command_sets));


/**
 * The handler for MCTP control messages.
 *
 * TODO:  Create a static initializer for this type.  In the meantime, mark it as RO after init.
 */
SECTION (".sprtro.mctp_control_handler")
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
	cmd_interface_multi_handler_msg_type_static_init (1, &msft_handler.base),
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
	cmd_interface_multi_handler_msg_type_static_init (MCTP_BASE_PROTOCOL_MSG_TYPE_SPDM,
		&mctp_spdm_handler.base),
	cmd_interface_multi_handler_msg_type_static_init (MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF,
		&msft_vdm_handler.base),
};

/**
 * Handler for received MCTP request messages.
 */
static const struct cmd_interface_multi_handler mctp_handler =
	cmd_interface_multi_handler_static_init (&mctp_protocol.base, mctp_message_types,
	ARRAY_SIZE (mctp_message_types));

/**
 * Variable context for the MCTP transport layer.
 */
static struct mctp_interface_state mctp_transport_context;

/**
 * The MCTP transport layer handler for the I2C command channel.
 */
const struct mctp_interface mctp_transport = mctp_interface_static_init (&mctp_transport_context,
	&mctp_handler, &device_manager, &fips_i2c.base_channel);

/**
 * Transport for sending MCTP control message requests.
 */
static const struct msg_transport_mctp_message mctp_control_req =
	msg_transport_mctp_message_static_init (&mctp_transport.base, &mctp_protocol,
	MCTP_BASE_PROTOCOL_MSG_TYPE_CONTROL_MSG);

/**
 * Handler for received system commands.
 */
static const struct cmd_channel_handler system_cmd_handler =
	cmd_channel_handler_static_init_notify_null_eid (&fips_i2c.base_channel, &mctp_transport,
	&mctp_control_req.base.base);

/**
 * List of handlers for the system command processing task.
 */
static const struct periodic_task_handler *const system_cmd_handlers[1] = {
	&system_cmd_handler.base
};

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
 * Initialize handling for device unlock requests.
 *
 * @return 0 if the unlock handler was initialized successfully or an error code.
 */
static int initialize_device_unlock ()
{
	int status;

	status = signature_verification_ecc_init_state (&unlock_auth_verify, NULL, 0);
	if (status != 0) {
		return status;
	}

	status = auth_token_init_state (&unlock_auth);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the I2C channel for receiving commands.
 *
 * @return 0 if the command channel was successfully initialized or an error code.
 */
int initialize_cmd_interface ()
{
	int status;

#ifndef MANTICORE_ENABLE_ACVP
	const struct pcd *active_pcd = platform_config.base.get_active_pcd (&platform_config.base);
#else
	const struct pcd *active_pcd = NULL;
#endif

	status = initialize_i2c_interface (active_pcd);
	if (status != 0) {
		goto done;
	}

	status = attestation_responder_init_no_aux (&system_attestation_responder, &dice_key_manager,
		&shared_hash.base, &shared_ecc.base, &shared_rng.base, &pcr_storage,
		CERBERUS_PROTOCOL_PROTOCOL_VERSION, CERBERUS_PROTOCOL_PROTOCOL_VERSION);
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

#ifndef MANTICORE_ENABLE_ACVP
	status = cmd_interface_system_init (&cerberus_handler, &impactful_handler.base_ctrl, NULL,
		&pfm_handler.base.base_cmd, NULL, &pcd_handler.base.base_cmd, NULL, &host_fw_manifest.base,
		NULL, &platform_config.base, &system_attestation_responder, NULL, &device_manager,
		&pcr_storage, &shared_hash.base, &background_handler.base_cmd, NULL, &host_handler.base_cmd,
		&firmware_version, &dice_key_manager, &cmd_auth, NULL, &host_gpio.base, NULL, NULL, NULL,
		NULL, &graceful_shutdown.base_device, CERBERUS_PROTOCOL_MSFT_PCI_VID,
		MSFT_DEVICE_ID_MANTICORE, CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_SUBSYSTEM_DEVICE_ID_DC_SCM,
		NULL);
#else
	/* When ACVP testing is enabled, the host interface, host PFM management, and PCD management are
	 * unavailable. */
	status = cmd_interface_system_init (&cerberus_handler, &impactful_handler.base_ctrl, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, &system_attestation_responder, NULL, &device_manager,
		&pcr_storage, &shared_hash.base, &background_handler.base_cmd, NULL, NULL,
		&firmware_version, &dice_key_manager, &cmd_auth, NULL, NULL, NULL, NULL, NULL, NULL,
		&graceful_shutdown.base_device, CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_DEVICE_ID_MANTICORE,
		CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_SUBSYSTEM_DEVICE_ID_DC_SCM, NULL);
#endif
	if (status != 0) {
		goto done;
	}

	status = cmd_interface_mctp_control_init (&mctp_control_handler, &device_manager,
		CERBERUS_PROTOCOL_MSFT_PCI_VID, CERBERUS_PROTOCOL_PROTOCOL_VERSION);
	if (status != 0) {
		goto done;
	}

	status = temperature_sensor_tsen_init_state (&tsen);
	if (status != 0) {
		goto done;
	}

	status = initialize_device_unlock ();
	if (status != 0) {
		goto done;
	}

	status = initialize_mctp_spdm_responder ();
	if (status != 0) {
		goto done;
	}

	status = mctp_interface_init_state (&mctp_transport);
	if (status != 0) {
		goto done;
	}

	status = periodic_task_freertos_init_state (&system_cmd_task);

done:
#ifndef MANTICORE_ENABLE_ACVP
	platform_config.base.free_pcd (&platform_config.base, active_pcd);
#endif

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

	status = start_i2c_interface ();
	if (status != 0) {
		return status;
	}

	return 0;
}
