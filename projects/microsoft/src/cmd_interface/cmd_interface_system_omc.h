// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_SYSTEM_OMC_H_
#define CMD_INTERFACE_SYSTEM_OMC_H_


#include <stdbool.h>
#include <stdint.h>
#include "attestation/attestation_responder.h"
#include "attestation/pcr_store.h"
#include "cmd_interface/cmd_authorization.h"
#include "cmd_interface/cmd_background.h"
#include "cmd_interface/cmd_device.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/device_manager.h"
#include "cmd_interface/session_manager.h"
#include "crypto/hash.h"
#include "firmware/firmware_update_control.h"
#include "host_fw/host_cmd_interface.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/omc_flash_manager.h"
#include "host_fw/overlake_control.h"
#include "manifest/manifest_cmd_interface.h"
#include "manifest/pfm/pfm_manager.h"
#include "riot/riot_key_manager.h"

/**
 * Command channel IDs for Overlake.
 */
enum {
	CMD_INTERFACE_OMC_CHANNEL_I2C_BMC = 0,	/**< I2C Channel ID for BMC. */
};

/**
 * System command interface for processing requests received from system, including
 * Overlake-specific requests.
 */
struct cmd_interface_system_omc {
	struct cmd_interface base;							/**< Base command interface */
	const struct firmware_update_control *control;		/**< FW update control instance */
	const struct manifest_cmd_interface *pfm_0;			/**< PFM update command interface instance for port 0 */
	const struct cmd_background *background;			/**< Context for completing background commands */
	const struct pfm_manager *pfm_manager_0;			/**< PFM manager instance for port 0 */
	const struct host_cmd_interface *host_0;			/**< Host interface for port 0 */
	struct pcr_store *pcr_store;						/**< PCR storage */
	const struct riot_key_manager *riot;				/**< RIoT key manager */
	struct attestation_responder *attestation;			/**< Attestation responder instance */
	const struct hash_engine *hash;						/**< The hashing engine for PCR operations. */
	const struct cmd_interface_fw_version *fw_version;	/**< FW version numbers */
	const struct host_control *host_0_ctrl;				/**< Host hardware control for port 0. */
	struct device_manager *device_manager;				/**< Device manager instance */
	const struct cmd_device *cmd_device;				/**< Device command handler instance */
	struct cmd_interface_device_id device_id;			/**< Device ID information */
	struct overlake_control *soc_control;				/**< The interface for hardware control of the Overlake SoC. */
	struct omc_flash_manager *flash;					/**< The flash manager for the Overlake SoC. */
	const struct host_fw_cmd_interface *boot;			/**< Command handler for the SoC boot firmware. */
	enum overlake_board_type board_type;				/**< Overlake Board type */
};


int cmd_interface_system_omc_init (struct cmd_interface_system_omc *intf,
	const struct firmware_update_control *control, const struct manifest_cmd_interface *pfm_0,
	const struct pfm_manager *pfm_manager_0, struct attestation_responder *attestation,
	struct device_manager *device_manager, struct pcr_store *store, const struct hash_engine *hash,
	const struct cmd_background *background, const struct host_cmd_interface *host_0,
	const struct cmd_interface_fw_version *fw_version, const struct riot_key_manager *riot,
	const struct host_control *host_0_ctrl, struct overlake_control *soc_control,
	struct omc_flash_manager *flash_mgr_0, const struct host_fw_cmd_interface *soc_boot,
	const struct cmd_device *cmd_device, uint16_t vendor_id, uint16_t device_id,
	uint16_t subsystem_vid,	uint16_t subsystem_id, struct session_manager *session,
	enum overlake_board_type board_type);
void cmd_interface_system_omc_deinit (const struct cmd_interface_system_omc *intf);


#endif	/* CMD_INTERFACE_SYSTEM_OMC_H_ */
