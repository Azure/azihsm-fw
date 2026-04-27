// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_PROTOCOL_COMMANDS_OMC_H_
#define OVERLAKE_PROTOCOL_COMMANDS_OMC_H_

#include <stdint.h>
#include "asn1/x509.h"
#include "attestation/attestation_responder.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/overlake_protocol.h"
#include "cmd_interface/overlake_protocol_commands_common.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/omc_flash_manager.h"
#include "host_fw/overlake_control.h"
#include "riot/riot_key_manager.h"


int overlake_protocol_host_fw_init_omc (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, bool prioritize_update, uint8_t max_channel_id,
	struct cmd_interface_msg *request);
int overlake_protocol_host_fw_update_omc (const struct host_fw_cmd_interface *host_fw_cmd[],
	uint8_t num_ports, struct cmd_interface_msg *request);
int overlake_protocol_get_host_fw_update_status_omc (
	const struct host_fw_cmd_interface *host_fw_cmd[], uint8_t num_ports,
	struct cmd_interface_msg *request);

int overlake_protocol_soc_reset_omc (struct omc_flash_manager *flash,
	struct overlake_control *soc_control, struct cmd_interface_msg *request);


#endif	/* OVERLAKE_PROTOCOL_COMMANDS_OMC_H_ */
