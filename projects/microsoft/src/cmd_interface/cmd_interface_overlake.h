// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_OVERLAKE_H_
#define CMD_INTERFACE_OVERLAKE_H_

#include <stdint.h>
#include "asn1/x509.h"
#include "attestation/attestation.h"
#include "attestation/pcr_store.h"
#include "cmd_interface/cmd_background.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/device_manager.h"
#include "cmd_interface/overlake_background.h"
#include "cmd_interface/session_manager.h"
#include "crypto/ecc.h"
#include "host_fw/host_fw_cmd_interface.h"
#include "host_fw/overlake_board_id.h"
#include "manifest/manifest_cmd_interface.h"
#include "manifest/pfm/pfm_manager.h"
#include "riot/riot_key_manager.h"
#include "tpm/tpm.h"

/**
 * Command channel IDs for Overlake.
 */
enum {
	CMD_INTERFACE_OVERLAKE_CHANNEL_I2C_BMC = 0,		/**< I2C Channel ID for BMC. */
	CMD_INTERFACE_OVERLAKE_CHANNEL_SPI_SOC,			/**< SPI Channel ID for SOC. */
	CMD_INTERFACE_OVERLAKE_CHANNEL_RPM_SOC,			/**< RPM Channel ID for SOC. */
	CMD_INTERFACE_OVERLAKE_CHANNEL_OPTEE_SOC,		/**< OPTEE Channel ID for SOC. */
	CMD_INTERFACE_OVERLAKE_CHANNEL_IPI_SEC,			/**< IPI Channel ID for SEC. */
	CMD_INTERFACE_OVERLAKE_CHANNEL_IPI_VENDOR_LOG,	/**< IPI Channel ID for Vendor Log. */
};

/**
 * Command interface for processing received requests from Overlake.
 */
struct cmd_interface_overlake {
	struct cmd_interface base;					/**< Base command interface */
	struct tpm *tpm;							/**< TPM instance tied to interface */
	const struct host_fw_cmd_interface *boot;	/**< Boot FW command handler. */
	const struct host_fw_cmd_interface *nitro;	/**< Nitro FW command handler. */
	const struct host_fw_cmd_interface *fpga;	/**< Command handler for C5 fpga FW. */
	const struct manifest_cmd_interface *pfm_0;	/**< PFM update command interface instance for port 0 */
	const struct manifest_cmd_interface *pfm_1;	/**< PFM update command interface instance for port 1 */
	const struct manifest_cmd_interface *pfm_2;	/**< PFM update command interface instance for port 2 */
	const struct pfm_manager *pfm_manager_0;	/**< PFM manager instance for port 0 */
	const struct pfm_manager *pfm_manager_1;	/**< PFM manager instance for port 1 */
	const struct pfm_manager *pfm_manager_2;	/**< PFM manager instance for port 2 */
	struct attestation_responder *attestation;	/**< Attestation responder instance */
	const struct cmd_background *background;	/**< Context for completing background commands */
	struct overlake_background *overlake_bgnd;	/**< Overlake context for background commands */
	const struct x509_engine *x509;				/**< X.509 engine for certificate parsing. */
	struct pcr_store *pcr;						/**< Manager for system PCRs. */
	struct device_manager *device_manager;		/**< Device manager instance */
	const struct hash_engine *hash;				/**< Hash engine for data signing. */
	const struct ecc_engine *ecc;				/**< ECC engine for data signing. */
	const struct riot_key_manager *riot;		/**< Data signing key. */
	const struct flash_store *flash;			/**< The flash block storage to store generic data. */
	enum overlake_board_type board_type;		/**< Overlake Board type */
};


int cmd_interface_overlake_init (struct cmd_interface_overlake *intf,
	struct device_manager *device_manager, struct tpm *tpm,
	const struct host_fw_cmd_interface *boot_fw, const struct host_fw_cmd_interface *nitro_fw,
	const struct host_fw_cmd_interface *fpga_fw, const struct manifest_cmd_interface *pfm_0,
	const struct manifest_cmd_interface *pfm_1, const struct manifest_cmd_interface *pfm_2,
	const struct pfm_manager *pfm_manager_0, const struct pfm_manager *pfm_manager_1,
	const struct pfm_manager *pfm_manager_2, struct attestation_responder *attestation,
	const struct cmd_background *background, struct overlake_background *overlake_bgnd,
	const struct x509_engine *x509, struct pcr_store *pcr, struct session_manager *session,
	const struct hash_engine *hash, const struct ecc_engine *ecc,
	const struct riot_key_manager *riot, const struct flash_store *flash,
	enum overlake_board_type board_type);
void cmd_interface_overlake_deinit (struct cmd_interface_overlake *intf);


#endif	/* CMD_INTERFACE_OVERLAKE_H_ */
