// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cerberus_protocol_diagnostic_commands.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/cerberus_protocol_optional_commands.h"
#include "cmd_interface/cerberus_protocol_required_commands.h"
#include "cmd_interface/cmd_interface_overlake.h"
#include "cmd_interface/cmd_interface_system_overlake.h"
#include "cmd_interface/cmd_logging.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands.h"
#include "common/unused.h"
#include "host_fw/overlake_board_id.h"
#include "host_fw/overlake_host_id.h"


static int cmd_interface_system_overlake_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_system_overlake *interface =
		(const struct cmd_interface_system_overlake*) intf;
	uint8_t command_id;
	uint8_t command_set;
	int status;

	status = cmd_interface_process_cerberus_protocol_message (intf, request, &command_id,
		&command_set, true, true);
	if (status != 0) {
		return status;
	}

	switch (command_id) {
		case OVERLAKE_PROTOCOL_SOC_RESET:
			status = overlake_protocol_soc_reset (interface->flash, interface->flash_1,
				interface->soc_control, request);
			break;

		case OVERLAKE_PROTOCOL_TRIGGER_NMI:
			status = overlake_protocol_nmi_trigger (interface->soc_control, request);
			break;

		case OVERLAKE_PROTOCOL_SET_BOOT_DEVICE:
			status = overlake_protocol_set_soc_boot (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_GET_BOOT_DEVICE:
			status = overlake_protocol_get_soc_boot (interface->flash, interface->board_type,
				request);
			break;

		case OVERLAKE_PROTOCOL_SET_FPGA_BOOT_MODE:
			status = overlake_protocol_set_fpga_boot (interface->flash_1, interface->board_type,
				request);
			break;

		case OVERLAKE_PROTOCOL_GET_FPGA_BOOT_MODE:
			status = overlake_protocol_get_soc_boot (interface->flash_1, interface->board_type,
				request);
			break;

		case OVERLAKE_PROTOCOL_SOC_INIT_FW_UPDATE: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};
			bool prioritize_update =
				(request->channel_id == CMD_INTERFACE_OVERLAKE_CHANNEL_I2C_BMC) ? true : false;

			status = overlake_protocol_host_fw_init (host_fw_cmd, OVERLAKE_HOST_NUM_PORTS,
				prioritize_update, CMD_INTERFACE_OVERLAKE_CHANNEL_SPI_SOC, request);
			break;
		}

		case OVERLAKE_PROTOCOL_SOC_UPDATE_FW: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};

			status = overlake_protocol_host_fw_update (host_fw_cmd, OVERLAKE_HOST_NUM_PORTS,
				request);
			break;
		}

		case OVERLAKE_PROTOCOL_GET_SOC_UPDATE_STATUS: {
			const struct host_fw_cmd_interface *host_fw_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->boot, interface->nitro, interface->fpga
			};

			status = overlake_protocol_get_host_fw_update_status (host_fw_cmd,
				OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case OVERLAKE_PROTOCOL_GET_MAC_ADDRESS:
			status = overlake_protocol_get_soc_mac_address (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_GET_DEBUG_LOG:
			status = overlake_protocol_get_soc_crmu_log (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_SET_DEBUG_VERBOSITY:
			status = overlake_protocol_set_soc_debug_level (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_GET_DEBUG_VERBOSITY:
			status = overlake_protocol_get_soc_debug_level (interface->flash, request);
			break;

		case OVERLAKE_PROTOCOL_GET_SOC_FWVERSION:
			status = overlake_protocol_get_soc_fwversion (interface->flash, request);
			break;

		case CERBERUS_PROTOCOL_GET_FW_VERSION:
			status = cerberus_protocol_get_fw_version (interface->fw_version, request);
			break;

		case CERBERUS_PROTOCOL_GET_DIGEST:
			status = cerberus_protocol_get_certificate_digest (interface->attestation,
				interface->base.session, request);
			break;

		case CERBERUS_PROTOCOL_GET_CERTIFICATE:
			status = cerberus_protocol_get_certificate (interface->attestation, request);
			break;

		case CERBERUS_PROTOCOL_ATTESTATION_CHALLENGE:
			status = cerberus_protocol_get_challenge_response (interface->attestation,
				interface->base.session, request);
			break;

		case CERBERUS_PROTOCOL_GET_LOG_INFO:
			status = cerberus_protocol_get_log_info (interface->pcr_store, request);
			break;

		case CERBERUS_PROTOCOL_READ_LOG:
			status = cerberus_protocol_log_read (interface->pcr_store, interface->hash, request);
			break;

		case CERBERUS_PROTOCOL_CLEAR_LOG:
			status = cerberus_protocol_log_clear (interface->background, request);
			break;

		case CERBERUS_PROTOCOL_GET_PFM_ID: {
			const struct pfm_manager *const pfm_mgr[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_manager_0, interface->pfm_manager_1, interface->pfm_manager_2
			};

			status = cerberus_protocol_get_pfm_id (pfm_mgr, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_PFM_SUPPORTED_FW: {
			const struct pfm_manager *const pfm_mgr[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_manager_0, interface->pfm_manager_1, interface->pfm_manager_2
			};

			status = cerberus_protocol_get_pfm_fw (pfm_mgr, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_INIT_PFM_UPDATE: {
			const struct manifest_cmd_interface *const pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update_init (pfm_cmd, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_PFM_UPDATE: {
			const struct manifest_cmd_interface *const pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update (pfm_cmd, OVERLAKE_HOST_NUM_PORTS, request);
			break;
		}

		case CERBERUS_PROTOCOL_COMPLETE_PFM_UPDATE: {
			const struct manifest_cmd_interface *const pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};

			status = cerberus_protocol_pfm_update_complete (pfm_cmd, OVERLAKE_HOST_NUM_PORTS,
				request);
			break;
		}

		case CERBERUS_PROTOCOL_INIT_FW_UPDATE:
			status = cerberus_protocol_fw_update_init (interface->control, request);
			break;

		case CERBERUS_PROTOCOL_FW_UPDATE:
			status = cerberus_protocol_fw_update (interface->control, request);
			break;

		case CERBERUS_PROTOCOL_COMPLETE_FW_UPDATE:
			status = cerberus_protocol_fw_update_start (interface->control, request);
			break;

		case CERBERUS_PROTOCOL_GET_UPDATE_STATUS: {
			const struct manifest_cmd_interface *const pfm_cmd[OVERLAKE_HOST_NUM_PORTS] = {
				interface->pfm_0, interface->pfm_1, interface->pfm_2
			};
			const struct host_cmd_interface *const host[OVERLAKE_HOST_NUM_PORTS] = {
				interface->host_0, interface->host_1, interface->host_2
			};

			status = cerberus_protocol_get_update_status (interface->control,
				OVERLAKE_HOST_NUM_PORTS, pfm_cmd, NULL, NULL, host, NULL, NULL,
				interface->background, request);
			break;
		}

		case CERBERUS_PROTOCOL_GET_EXT_UPDATE_STATUS:
			status = cerberus_protocol_get_extended_update_status (interface->control, NULL, NULL,
				NULL, NULL, request);
			break;

		case CERBERUS_PROTOCOL_GET_DEVICE_CAPABILITIES:
			status = cerberus_protocol_get_device_capabilities (interface->device_manager, request);
			break;

		case CERBERUS_PROTOCOL_RESET_COUNTER:
			status = cerberus_protocol_reset_counter (interface->cmd_device, request);
			break;

		case CERBERUS_PROTOCOL_UNSEAL_MESSAGE:
			status = cerberus_protocol_unseal_message (interface->background, request);
			break;

		case CERBERUS_PROTOCOL_UNSEAL_MESSAGE_RESULT:
			status = cerberus_protocol_unseal_message_result (interface->background, request);
			break;

		case CERBERUS_PROTOCOL_EXPORT_CSR:
			status = cerberus_protocol_export_csr (interface->riot, request);
			break;

		case CERBERUS_PROTOCOL_IMPORT_CA_SIGNED_CERT:
			status = cerberus_protocol_import_ca_signed_cert (interface->riot,
				interface->background, request);
			break;

		case CERBERUS_PROTOCOL_GET_SIGNED_CERT_STATE:
			status = cerberus_protocol_get_signed_cert_state (interface->background, request);
			break;

		case CERBERUS_PROTOCOL_RESET_CONFIG:
			status = cerberus_protocol_reset_config (interface->auth, interface->background,
				request);
			break;

		case CERBERUS_PROTOCOL_GET_HOST_STATE:
			status = cerberus_protocol_get_host_reset_status (interface->host_0_ctrl,
				interface->host_1_ctrl, request);
			break;

		case CERBERUS_PROTOCOL_GET_DEVICE_INFO:
			status = cerberus_protocol_get_device_info (interface->cmd_device, request);
			break;

		case CERBERUS_PROTOCOL_GET_DEVICE_ID:
			status = cerberus_protocol_get_device_id (&interface->device_id, request);
			break;

		case CERBERUS_PROTOCOL_GET_ATTESTATION_DATA:
			status = cerberus_protocol_get_attestation_data (interface->pcr_store, request);
			break;

		case CERBERUS_PROTOCOL_EXCHANGE_KEYS:
			status = cerberus_protocol_key_exchange (interface->base.session, request,
				request->is_encrypted);
			break;

		case CERBERUS_PROTOCOL_SESSION_SYNC:
			status = cerberus_protocol_session_sync (interface->base.session, request,
				request->is_encrypted);
			break;

		case CERBERUS_PROTOCOL_GET_PCD_ID:
			return cerberus_protocol_get_pcd_id (interface->pcd_mgr, request);

		case CERBERUS_PROTOCOL_DIAG_HEAP_USAGE:
			return cerberus_protocol_heap_stats (interface->cmd_device, request);

#ifdef CMD_ENABLE_STACK_STATS
		case CERBERUS_PROTOCOL_DIAG_STACK_USAGE:
			return cerberus_protocol_stack_stats (interface->cmd_device, request);
#endif

		default:
			return CMD_HANDLER_UNKNOWN_REQUEST;
	}

	if (status == 0) {
		status = cmd_interface_prepare_response (&interface->base, request);
	}

	return status;
}

/**
 * Initialize Overlake system command interface instance
 *
 * @param intf The Overlake system command interface instance to initialize
 * @param control The FW update control instance to use
 * @param pfm_0 Command interface to PFM for port 0
 * @param pfm_1 Command interface to PFM for port 1
 * @param pfm_2 Command interface to PFM for port 2
 * @param pfm_manager_0 PFM manager for port 0
 * @param pfm_manager_1 PFM manager for port 1
 * @param pfm_manager_2 PFM manager for port 2
 * @param attestation Slave attestation manager
 * @param device_manager Device manager
 * @param store PCR storage
 * @param hash Hash engine to to use for PCR operations
 * @param background Context for executing long-running operations in the background.
 * @param host_0 Host interface for port 0
 * @param host_1 Host interface for port 1
 * @param host_2 Host interface for port 2
 * @param fw_version The FW version strings
 * @param riot RIoT keys manager
 * @param auth Handler for authorizing protected commands
 * @param host_0_ctrl The host control instance for port 0
 * @param host_1_ctrl The host control instance for port 1
 * @param host_2_ctrl The host control instance for port 2
 * @param soc_control The SoC control instance to use
 * @param flash_mgr_0 The flash manager for the Overlake SoC
 * @param flash_mgr_1 The flash manager for the Overlake FPGA
 * @param soc_boot Command interface for Overlake SoC boot firmware operations
 * @param soc_nitro Command interface for Overlake SoC nitro firmware operations
 * @param fpga Command interface for Overlake C5 fpga firmware operations
 * @param cmd_device Device command handler instance
 * @param vendor_id Device vendor ID
 * @param device_id Device ID
 * @param subsystem_vid Subsystem vendor ID
 * @param subsystem_id Subsystem ID
 * @param session Session manager for channel encryption
 * @param pcd_manager PCD manager instance
 * @param board_type The Overlake board type
 *
 * @return Initialization status, 0 if success or an error code.
 */
int cmd_interface_system_overlake_init (struct cmd_interface_system_overlake *intf,
	const struct firmware_update_control *control, const struct manifest_cmd_interface *pfm_0,
	const struct manifest_cmd_interface *pfm_1, const struct manifest_cmd_interface *pfm_2,
	const struct pfm_manager *pfm_manager_0, const struct pfm_manager *pfm_manager_1,
	const struct pfm_manager *pfm_manager_2, struct attestation_responder *attestation,
	struct device_manager *device_manager, struct pcr_store *store, const struct hash_engine *hash,
	const struct cmd_background *background, const struct host_cmd_interface *host_0,
	const struct host_cmd_interface *host_1, const struct host_cmd_interface *host_2,
	const struct cmd_interface_fw_version *fw_version, const struct riot_key_manager *riot,
	const struct cmd_authorization *auth, const struct host_control *host_0_ctrl,
	const struct host_control *host_1_ctrl, const struct host_control *host_2_ctrl,
	struct overlake_control *soc_control, struct overlake_flash_manager *flash_mgr_0,
	struct overlake_flash_manager *flash_mgr_1, const struct host_fw_cmd_interface *soc_boot,
	const struct host_fw_cmd_interface *soc_nitro, const struct host_fw_cmd_interface *fpga,
	const struct cmd_device *cmd_device, uint16_t vendor_id, uint16_t device_id,
	uint16_t subsystem_vid, uint16_t subsystem_id, struct session_manager *session,
	const struct pcd_manager *pcd_manager, enum overlake_board_type board_type)
{
	if ((intf == NULL) || (control == NULL) || (store == NULL) || (background == NULL) ||
		(riot == NULL) || (auth == NULL) || (attestation == NULL) || (hash == NULL) ||
		(device_manager == NULL) || (fw_version == NULL) || (cmd_device == NULL) ||
		(soc_control == NULL) || (flash_mgr_0 == NULL) || (soc_boot == NULL) ||
		(soc_nitro == NULL) || (fpga == NULL)) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	if ((board_type == OVERLAKE_GLACIER_PEAK) && (flash_mgr_1 == NULL)) {
		return CMD_HANDLER_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_system_overlake));

	intf->control = control;
	intf->pfm_0 = pfm_0;
	intf->pfm_1 = pfm_1;
	intf->pfm_2 = pfm_2;
	intf->pfm_manager_0 = pfm_manager_0;
	intf->pfm_manager_1 = pfm_manager_1;
	intf->pfm_manager_2 = pfm_manager_2;
	intf->host_0 = host_0;
	intf->host_1 = host_1;
	intf->host_2 = host_2;
	intf->pcr_store = store;
	intf->riot = riot;
	intf->background = background;
	intf->auth = auth;
	intf->attestation = attestation;
	intf->hash = hash;
	intf->host_0_ctrl = host_0_ctrl;
	intf->host_1_ctrl = host_1_ctrl;
	intf->host_2_ctrl = host_2_ctrl;
	intf->device_manager = device_manager;
	intf->fw_version = fw_version;
	intf->cmd_device = cmd_device;
	intf->soc_control = soc_control;
	intf->flash = flash_mgr_0;
	intf->flash_1 = flash_mgr_1;
	intf->boot = soc_boot;
	intf->nitro = soc_nitro;
	intf->fpga = fpga;
	intf->pcd_mgr = pcd_manager;
	intf->board_type = board_type;

	intf->device_id.vendor_id = vendor_id;
	intf->device_id.device_id = device_id;
	intf->device_id.subsystem_vid = subsystem_vid;
	intf->device_id.subsystem_id = subsystem_id;

	intf->base.process_request = cmd_interface_system_overlake_process_request;

	intf->base.session = session;

	return 0;
}

/**
 * Deinitialize System command interface instance
 *
 * @param intf The System command interface instance to deinitialize
 */
void cmd_interface_system_overlake_deinit (const struct cmd_interface_system_overlake *intf)
{
	UNUSED (intf);
}
