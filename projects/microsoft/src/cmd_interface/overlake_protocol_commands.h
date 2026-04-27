// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_PROTOCOL_COMMANDS_H_
#define OVERLAKE_PROTOCOL_COMMANDS_H_

#include <stdint.h>
#include "asn1/x509.h"
#include "attestation/attestation_responder.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands_common.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/overlake_control.h"
#include "host_fw/overlake_flash_manager.h"
#include "riot/riot_key_manager.h"
#include "tpm/tpm.h"


int overlake_protocol_get_storage (struct tpm *tpm, bool mask_data_error,
	struct cmd_interface_msg *request);
int overlake_protocol_set_storage (struct tpm *tpm, struct cmd_interface_msg *request);

int overlake_protocol_read_data (const struct flash_store *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_clear_data (const struct flash_store *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_store_data (const struct flash_store *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_sign_data (const struct ecc_engine *ecc, const struct riot_key_manager *riot,
	const struct hash_engine *hash, struct cmd_interface_msg *request);

int overlake_protocol_host_fw_init (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, bool prioritize_update, uint8_t max_channel_id,
	struct cmd_interface_msg *request);
int overlake_protocol_host_fw_update (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, struct cmd_interface_msg *request);
int overlake_protocol_get_host_fw_update_status (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, struct cmd_interface_msg *request);

int overlake_protocol_tpm_clear (struct tpm *tpm, struct cmd_interface_msg *request);

int overlake_protocol_soc_reset (struct overlake_flash_manager *flash,
	struct overlake_flash_manager *flash_fpga, struct overlake_control *soc_control,
	struct cmd_interface_msg *request);
int overlake_protocol_nmi_trigger (struct overlake_control *soc_control,
	struct cmd_interface_msg *request);

int overlake_protocol_set_soc_boot (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_get_soc_boot (struct overlake_flash_manager *flash,
	enum overlake_board_type board_type, struct cmd_interface_msg *request);
int overlake_protocol_set_fpga_boot (struct overlake_flash_manager *flash,
	enum overlake_board_type board_type, struct cmd_interface_msg *request);
int overlake_protocol_get_soc_mac_address (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_get_soc_crmu_log (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_set_soc_debug_level (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_get_soc_debug_level (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);
int overlake_protocol_get_soc_fwversion (struct overlake_flash_manager *flash,
	struct cmd_interface_msg *request);

int overlake_protocol_get_public_key (struct attestation_responder *attestation,
	struct cmd_interface_msg *request, const struct x509_engine *x509);

int overlake_protocol_decrypt_payload (struct attestation_responder *attestation,
	struct cmd_interface_msg *request);


#endif	/* OVERLAKE_PROTOCOL_COMMANDS_H_ */
