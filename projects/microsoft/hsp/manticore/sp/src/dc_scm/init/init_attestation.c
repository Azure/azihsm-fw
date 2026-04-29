// Copyright (c) Microsoft Corporation. All rights reserved.

#include "init_attestation.h"
#include "init_crypto.h"
#include "init_flash.h"
#include "init_host.h"
#include "init_manifest.h"
#include "init_system.h"
#include "manticore_pcr.h"
#include "manticore_sticky_regs.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "attestation/pcr_store.h"
#include "attestation/pcr_tcg.h"
#include "common/array_size.h"
#include "common/buffer_util.h"
#include "logging/init_logging.h"
#include "splibs/hsprt/riscvcpu.h"


/**
 * Data populated by 1SP that can be used with local static initialization.
 */
static const struct manticore_1sp_shared_data *const sp1_shared_static =
	(struct manticore_1sp_shared_data*) SP1_SHARED_ADDRESS;

/**
 * Variable context for the flash for key and certificate storage.
 */
static struct flash_store_contiguous_blocks_state keystore_flash_context;

/**
 * Flash block storage for keys and certificates.
 */
const struct flash_store_contiguous_blocks keystore_flash =
	flash_store_contiguous_blocks_static_init_variable_storage_decreasing (
	FLASH_STORE_CONTIGUOUS_BLOCKS_WITH_HASH_API_INIT, &keystore_flash_context, MAIN_KEYSTORE_ADDR,
	MAIN_KEYSTORE_MAX_KEYS, &flash_internal.base, &shared_hash.base);

/**
 * Storage for keys and certificates.
 */
const struct keystore_flash main_keystore = keystore_flash_static_init (&keystore_flash.base);


/**
 * Container the DME public key to be exported by the device.
 */
SECTION (".sprtro.dme_key")
struct device_keys_dme_public_key dme_key;

/**
 * List of DME key data to provide through the Export CSR command.
 */
static const struct der_cert dme_key_export[] = {
	{
		.cert = (uint8_t*) &dme_key,
		.length = sizeof (dme_key),
	},
};

/**
 * Variable context for managing DICE identity keys and certificates.
 */
static struct riot_key_manager_state dice_key_context;

/**
 * Application manager for device identity keys and certificates.
 */
const struct riot_key_manager dice_key_manager = riot_key_manager_static_init (&dice_key_context,
	&main_keystore.base, &shared_x509.base, dme_key_export, ARRAY_SIZE (dme_key_export));

/**
 * Handler for renewal of the device identity.
 */
const struct identity_renewal identity = identity_renewal_static_init (&fuses.base);

/**
 * Platform and host PCR storage
 *
 * TODO:  Create a static initializer for this type.
 */
struct pcr_store pcr_storage;

/* PCR 0 - ROM measurements
 * Note:  This does not correspond to HW PCR0, which contains POR SP measurements. */

/**
 * Measurement data for the device security state.
 */
static const struct pcr_measured_data security_state_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.security_state.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.security_state.data)
		}
	}
};

/**
 * Measurement data for the owner public key used by ROM.
 */
static const struct pcr_measured_data owner_public_key_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.owner_public_key.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.owner_public_key.data)
		}
	}
};

/**
 * Measurement data for the SVN of the root key manifest.
 */
static const struct pcr_measured_data root_key_manifest_svn_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.key_manifest_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.key_manifest_svn.data)
		}
	}
};

/**
 * Measurement data for the current device tenancy counter.
 */
static const struct pcr_measured_data tenancy_counter_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.tenancy_counter.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.tenancy_counter.data)
		}
	}
};

/**
 * Measurement data for the public key used to verify 1SP firmware.
 */
static const struct pcr_measured_data public_key_1sp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.fw_public_key.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.fw_public_key.data)
		}
	}
};

/**
 * Measurement data for a secondary public key used to verify 1SP firmware.  This could either be an
 * additional firmware key or a tenancy grant key.
 */
static const struct pcr_measured_data secondary_public_key_1sp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.secondary_public_key.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.secondary_public_key.data)
		}
	}
};

/**
 * Measurement data for the SVN of 1SP firmware image.
 */
static const struct pcr_measured_data svn_1sp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.fw_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.fw_svn.data)
		}
	}
};

/**
 * Measurement data for the build version number of the 1SP firmware image.
 */
static const struct pcr_measured_data build_version_1sp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.fw_version.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.fw_version.data)
		}
	}
};

/**
 * Measurement data for the 1SP firmware image.
 */
static const struct pcr_measured_data fw_image_1sp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.rom.fw_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.rom.fw_image.data)
		}
	}
};

/* PCR 1 - SPRT measurements
 * Note:  This does not correspond to HW PCR1, which also contains ROM and 1SP measurements. */

/**
 * Measurement data for the active device security policy.
 */
static const struct pcr_measured_data security_policy_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.security_policy.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.security_policy.data)
		}
	}
};

/**
 * Measurement data for the firmware key manifest being used.
 */
static const struct pcr_measured_data fw_key_manifest_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.fw_key_manifest.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.fw_key_manifest.data)
		}
	}
};

/**
 * Measurement data for the SVN of the firmware key manifest.
 */
static const struct pcr_measured_data fw_key_manifest_svn_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.fw_key_manifest_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.fw_key_manifest_svn.data)
		}
	}
};

/**
 * Measurement data for the public key used for SPRT verification.
 */
static const struct pcr_measured_data public_key_sprt_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.fw_pkg_public_key.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.fw_pkg_public_key.data)
		}
	}
};

/**
 * Measurement data for the SVN of the SPRT image.
 */
static const struct pcr_measured_data svn_sprt_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.fw_pkg_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.fw_pkg_svn.data)
		}
	}
};

/**
 * Measurement data for the build version of the SPRT image.
 */
static const struct pcr_measured_data build_version_sprt_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.fw_pkg_version.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.fw_pkg_version.data)
		}
	}
};

/**
 * Measurement data for the SPRT image.
 */
static const struct pcr_measured_data fw_image_sprt_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.sprt_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.sprt_image.data)
		}
	}
};

/**
 * Measurement data for the AEB state at the beginning of SPRT execution.
 */
static const struct pcr_measured_data aeb_state_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.aeb_state.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.aeb_state.data)
		}
	}
};

/**
 * Measurement data for the AEB locked state at the beginning of SPRT execution.
 */
static const struct pcr_measured_data aeb_locked_state_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.sp.aeb_locked.data,
			.length = sizeof (sp1_shared_static->pcr_log.sp.aeb_locked.data)
		}
	}
};

/* PCR 2 - SoC measurements
 * Note:  This does not correspond to HW PCR2, which contains the POR SoC measurements. */

/**
 * Measurement data for the firmware key manifest used to load SoC firmware.
 */
static const struct pcr_measured_data soc_fw_key_manifest_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fw_key_manifest.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fw_key_manifest.data)
		}
	}
};

/**
 * Measurement data for the SVN of the firmware key manifest used to load SoC firmware.
 */
static const struct pcr_measured_data soc_fw_key_manifest_svn_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fw_key_manifest_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fw_key_manifest_svn.data)
		}
	}
};

/**
 * Measurement data for the public key used for SoC firmware verification.
 */
static const struct pcr_measured_data public_key_soc_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fw_pkg_public_key.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fw_pkg_public_key.data)
		}
	}
};

/**
 * Measurement data for the SVN of the SoC firmware images.
 */
static const struct pcr_measured_data svn_soc_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fw_pkg_svn.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fw_pkg_svn.data)
		}
	}
};

/**
 * Measurement data for the build version of the SoC firmware images.
 */
static const struct pcr_measured_data build_version_soc_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fw_pkg_version.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fw_pkg_version.data)
		}
	}
};

/**
 * Measurement data for the CP image.
 */
static const struct pcr_measured_data fw_image_cp_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.cp_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.cp_image.data)
		}
	}
};

/**
 * Measurement data for the FP0 image.
 */
static const struct pcr_measured_data fw_image_fp0_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fp0_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fp0_image.data)
		}
	}
};

/**
 * Measurement data for the FP1 image.
 */
static const struct pcr_measured_data fw_image_fp1_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fp1_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fp1_image.data)
		}
	}
};

/**
 * Measurement data for the FP2 image.
 */
static const struct pcr_measured_data fw_image_fp2_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.fp2_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.fp2_image.data)
		}
	}
};

/**
 * Measurement data for the PCIe PHY image.
 */
static const struct pcr_measured_data fw_image_phy_measured_data = {
	.type = PCR_DATA_TYPE_MEMORY,
	.data = {
		.memory = {
			.buffer = (uint8_t*) &sp1_shared_static->pcr_log.soc.phy_image.data,
			.length = sizeof (sp1_shared_static->pcr_log.soc.phy_image.data)
		}
	}
};

/* PCR 3 - Cerberus measurements
 * Note:  This does not correspond to HW PCR3, which contains the SoC firmware measurements. */

/* Disable PFM and PCD flows to save code space when ACVP testing is enabled. */
#ifndef MANTICORE_ENABLE_ACVP
/**
 * Measurement data for the active port 1 PFM.
 */
static const struct pcr_measured_data pfm_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pfm_manager_get_pfm_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pfm_manager_hash_pfm_measured_data,
			.context = (void*) &host_fw_manifest.base
		}
	}
};


static int get_pfm_valid_measured_data (volatile uint32_t *context, size_t offset, uint8_t *buffer,
	size_t length, uint32_t *total_len);
static int hash_pfm_valid_measured_data (volatile uint32_t *context,
	const struct hash_engine *hash);

/**
 * Measurement data indicating the verification result using the PFM for port 1.
 */
static const struct pcr_measured_data pfm_valid_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		/* Need to use a callback to ensure only dword access to the sticky register. */
		.callback = {
			.get_data = (pcr_data_get_measured_data) get_pfm_valid_measured_data,
			.hash_data = (pcr_data_hash_measured_data) hash_pfm_valid_measured_data,
			.context = MANTICORE_STICKY_REG (MANTICORE_PFM_VALID_PORT1)
		}
	}
};

/**
 * Measurement data for the ID of the active port 1 PFM.
 */
static const struct pcr_measured_data pfm_id_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pfm_manager_get_id_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pfm_manager_hash_id_measured_data,
			.context = (void*) &host_fw_manifest.base
		}
	}
};

/**
 * Measurement data for the platform ID of the active port 1 PFM.
 */
static const struct pcr_measured_data pfm_platform_id_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pfm_manager_get_platform_id_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pfm_manager_hash_platform_id_measured_data,
			.context = (void*) &host_fw_manifest.base
		}
	}
};

/**
 * Measurement data for the active PCD.
 */
static const struct pcr_measured_data pcd_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pcd_manager_get_pcd_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pcd_manager_hash_pcd_measured_data,
			.context = (void*) &platform_config.base
		}
	}
};

/**
 * Measurement data for the ID of the active PCD.
 */
static const struct pcr_measured_data pcd_id_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pcd_manager_get_id_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pcd_manager_hash_id_measured_data,
			.context = (void*) &platform_config.base
		}
	}
};

/**
 * Measurement data for the platform ID of the active PCD.
 */
static const struct pcr_measured_data pcd_platform_id_measured_data = {
	.type = PCR_DATA_TYPE_CALLBACK,
	.data = {
		.callback = {
			.get_data = (pcr_data_get_measured_data) pcd_manager_get_platform_id_measured_data,
			.hash_data = (pcr_data_hash_measured_data) pcd_manager_hash_platform_id_measured_data,
			.context = (void*) &platform_config.base
		}
	}
};
#endif	/* MANTICORE_ENABLE_ACVP */


/**
 * Initialize the structure for exporting the DME public key.  This must be called before any crypto
 * drivers are initialized to ensure the data is copied out of shared SRAM before being used for
 * command buffers.
 */
int initialize_dme_key_export ()
{
	int status;

	memcpy (dme_key.key.AsBytes, rom_shared->firmware.dme_key.AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE);
	memcpy (dme_key.signature.AsBytes, rom_shared->firmware.dme_signature.AsBytes,
		SP_ECDSA_P384_SIGNATURE_SIZE);

	status = identity.get_dme_renewal (&identity, &dme_key.renew_counter);
	if (status != 0) {
		return status;
	}

	status = fuses.base.read_registered_socid (&fuses.base, (uint8_t*) dme_key.socid,
		sizeof (dme_key.socid));
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return 0;
}

/**
 * Get the device keys created by 1SP.
 *
 * @param dice_keys Key structure to initialize for DICE identity key management.
 */
static void get_dice_keys (struct riot_keys *dice_keys)
{
	dice_keys->devid_cert = sp1_shared->devid_cert_fips;
	dice_keys->devid_cert_length = sp1_shared->devid_cert_fips_length;
	if (sp1_shared->devid_cert_fips_length < 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_RIOT_KEY_TOO_BIG, INIT_RIOT_KEY_DEVICE_ID, 0);
	}

	dice_keys->devid_csr = sp1_shared->devid_csr_fips;
	dice_keys->devid_csr_length = sp1_shared->devid_csr_fips_length;
	if (sp1_shared->devid_csr_fips_length < 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_RIOT_KEY_TOO_BIG, INIT_RIOT_KEY_DEVICE_ID_CSR, 0);
	}

	dice_keys->alias_key = sp1_shared->alias_key;
	dice_keys->alias_key_length = sp1_shared->alias_key_length;
	if (sp1_shared->alias_key_length < 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_RIOT_KEY_TOO_BIG, INIT_RIOT_KEY_ALIAS_KEY, 0);
	}

	dice_keys->alias_cert = sp1_shared->alias_cert;
	dice_keys->alias_cert_length = sp1_shared->alias_cert_length;
	if (sp1_shared->alias_cert_length < 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_RIOT_KEY_TOO_BIG, INIT_RIOT_KEY_ALIAS_CERT, 0);
	}
}

/**
 * Initialize the manager for DICE identity certificates and keys.
 *
 * @return 0 if the identity manager was successfully initialized or an error code.
 */
int initialize_dice_key_manager ()
{
	struct riot_keys dice_keys;
	int status;

	/* TODO:  Enable certificate redundancy. */

	get_dice_keys (&dice_keys);

	status = flash_store_contiguous_blocks_init_state (&keystore_flash, 0);
	if (status != 0) {
		return status;
	}

	return riot_key_manager_init_state_static_keys (&dice_key_manager, &dice_keys);
}

/**
 * Initialize the Manticore measurement management.
 *
 * @return 0 if measurements was successfully initialized or an error code.
 */
int initialize_manticore_measurements ()
{
	const struct pcr_config pcr_config[] = {
		{
			.num_measurements = MANTICORE_PCR_ROM_MEASUREMENTS,
			.measurement_algo = HASH_TYPE_SHA384
		},
		{
			.num_measurements = MANTICORE_PCR_SPRT_MEASUREMENTS,
			.measurement_algo = HASH_TYPE_SHA384
		},
		{
			.num_measurements = MANTICORE_PCR_SOC_MEASUREMENTS,
			.measurement_algo = HASH_TYPE_SHA384
		},
		{
			.num_measurements = MANTICORE_PCR_CERBERUS_MEASUREMENTS,
			.measurement_algo = HASH_TYPE_SHA384
		}
	};
	int status;

	status = pcr_store_init (&pcr_storage, pcr_config, ARRAY_SIZE (pcr_config));
	if (status != 0) {
		return status;
	}

	/* ROM and 1SP measurements */
	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECURITY_STATE,
		BOOT_MEASUREMENTS_EVENT_SECURITY_STATE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECURITY_STATE,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECURITY_STATE,
		&security_state_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECURITY_STATE,
		sp1_shared->pcr_log.rom.security_state.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_SECURITY_STATE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_OWNER_PUBLIC_KEY,
		BOOT_MEASUREMENTS_EVENT_OWNER_PUBLIC_KEY);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_OWNER_PUBLIC_KEY,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_OWNER_PUBLIC_KEY,
		&owner_public_key_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_OWNER_PUBLIC_KEY,
		sp1_shared->pcr_log.rom.owner_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_OWNER_PUBLIC_KEY,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_KEY_MANIFEST_SVN,
		BOOT_MEASUREMENTS_EVENT_KEY_MANIFEST_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_KEY_MANIFEST_SVN,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_KEY_MANIFEST_SVN,
		&root_key_manifest_svn_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_KEY_MANIFEST_SVN,
		sp1_shared->pcr_log.rom.key_manifest_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_KEY_MANIFEST_SVN,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_TENANCY_COUNTER,
		BOOT_MEASUREMENTS_EVENT_TENANCY_COUNTER);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_TENANCY_COUNTER,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_TENANCY_COUNTER,
		&tenancy_counter_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_TENANCY_COUNTER,
		sp1_shared->pcr_log.rom.tenancy_counter.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_TENANCY_COUNTER, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_PUBLIC_KEY,
		BOOT_MEASUREMENTS_EVENT_FW_PUBLIC_KEY);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_PUBLIC_KEY,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_PUBLIC_KEY,
		&public_key_1sp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_PUBLIC_KEY,
		sp1_shared->pcr_log.rom.fw_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_FW_PUBLIC_KEY, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECONDARY_PUBLIC_KEY,
		sp1_shared->pcr_log.rom.secondary_public_key.data.event.event_id);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECONDARY_PUBLIC_KEY,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_SECONDARY_PUBLIC_KEY,
		&secondary_public_key_1sp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage,
		PCR_MEASUREMENT_TYPE_ROM_SECONDARY_PUBLIC_KEY,
		sp1_shared->pcr_log.rom.secondary_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_SECONDARY_PUBLIC_KEY,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_SVN,
		BOOT_MEASUREMENTS_EVENT_FW_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_SVN,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_SVN,
		&svn_1sp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_SVN,
		sp1_shared->pcr_log.rom.fw_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_FW_SVN, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_BUILD_VERSION,
		BOOT_MEASUREMENTS_EVENT_BUILD_VERSION);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_BUILD_VERSION,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_BUILD_VERSION,
		&build_version_1sp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_BUILD_VERSION,
		sp1_shared->pcr_log.rom.fw_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_FW_BUILD_VERSION,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_IMAGE,
		BOOT_MEASUREMENTS_EVENT_FW_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_IMAGE,
		&fw_image_1sp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_ROM_FW_IMAGE,
		sp1_shared->pcr_log.rom.fw_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_ROM_FW_IMAGE, status);
	}

	/* SPRT measurements */
	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_SECURITY_POLICY,
		MANTICORE_MEASUREMENTS_EVENT_SECURITY_POLICY);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_SECURITY_POLICY,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_SECURITY_POLICY,
		&security_policy_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_SECURITY_POLICY,
		sp1_shared->pcr_log.sp.security_policy.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_SECURITY_POLICY,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST,
		MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST,
		&fw_key_manifest_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST,
		sp1_shared->pcr_log.sp.fw_key_manifest.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST_SVN,
		MANTICORE_MEASUREMENTS_EVENT_FW_KEY_MANIFEST_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST_SVN,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST_SVN,
		&fw_key_manifest_svn_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage,
		PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST_SVN,
		sp1_shared->pcr_log.sp.fw_key_manifest_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_KEY_MANIFEST_SVN,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_PUBLIC_KEY,
		MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_PUBLIC_KEY);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_PUBLIC_KEY,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_PUBLIC_KEY,
		&public_key_sprt_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_PUBLIC_KEY,
		sp1_shared->pcr_log.sp.fw_pkg_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_PUBLIC_KEY, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_SVN,
		MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_SVN,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_SVN,
		&svn_sprt_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_SVN,
		sp1_shared->pcr_log.sp.fw_pkg_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_SVN, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_BUILD_VERSION,
		MANTICORE_MEASUREMENTS_EVENT_FW_PACKAGE_BUILD_VERSION);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_BUILD_VERSION,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_BUILD_VERSION,
		&build_version_sprt_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage,
		PCR_MEASUREMENT_TYPE_SPRT_FW_BUILD_VERSION,
		sp1_shared->pcr_log.sp.fw_pkg_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_BUILD_VERSION,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_SPRT_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_IMAGE,
		&fw_image_sprt_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_FW_IMAGE,
		sp1_shared->pcr_log.sp.sprt_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_FW_IMAGE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_STATE,
		MANTICORE_MEASUREMENTS_EVENT_AEB_STATE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_STATE,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_STATE,
		&aeb_state_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_STATE,
		sp1_shared->pcr_log.sp.aeb_state.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_AEB_STATE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_LOCKED_STATE,
		MANTICORE_MEASUREMENTS_EVENT_AEB_LOCKED);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_LOCKED_STATE,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SPRT_AEB_LOCKED_STATE,
		&aeb_locked_state_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage,
		PCR_MEASUREMENT_TYPE_SPRT_AEB_LOCKED_STATE,
		sp1_shared->pcr_log.sp.aeb_locked.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SPRT_AEB_LOCKED_STATE,
			status);
	}

	/* SoC measurements */
	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST,
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST,
		&soc_fw_key_manifest_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST,
		sp1_shared->pcr_log.soc.fw_key_manifest.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST,	status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST_SVN,
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_KEY_MANIFEST_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST_SVN,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST_SVN,
		&soc_fw_key_manifest_svn_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage,
		PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST_SVN,
		sp1_shared->pcr_log.soc.fw_key_manifest_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FW_KEY_MANIFEST_SVN,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_PUBLIC_KEY,
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_PUBLIC_KEY);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_PUBLIC_KEY,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_PUBLIC_KEY,
		&public_key_soc_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_PUBLIC_KEY,
		sp1_shared->pcr_log.soc.fw_pkg_public_key.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FW_PUBLIC_KEY, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_SVN,
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_SVN);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_SVN,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_SVN,
		&svn_soc_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_SVN,
		sp1_shared->pcr_log.soc.fw_pkg_svn.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FW_SVN, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_BUILD_VERSION,
		MANTICORE_MEASUREMENTS_EVENT_CP_FW_PACKAGE_BUILD_VERSION);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_BUILD_VERSION,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_BUILD_VERSION,
		&build_version_soc_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FW_BUILD_VERSION,
		sp1_shared->pcr_log.soc.fw_pkg_version.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FW_BUILD_VERSION,
			status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_CP_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_CP_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_CP_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_CP_IMAGE,
		&fw_image_cp_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_CP_IMAGE,
		sp1_shared->pcr_log.soc.cp_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_CP_IMAGE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP0_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_FP0_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP0_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP0_IMAGE,
		&fw_image_fp0_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP0_IMAGE,
		sp1_shared->pcr_log.soc.fp0_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FP0_IMAGE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP1_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_FP1_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP1_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP1_IMAGE,
		&fw_image_fp1_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP1_IMAGE,
		sp1_shared->pcr_log.soc.fp1_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FP1_IMAGE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP2_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_FP2_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP2_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP2_IMAGE,
		&fw_image_fp2_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_FP2_IMAGE,
		sp1_shared->pcr_log.soc.fp2_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_FP2_IMAGE, status);
	}

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_PCIE_PHY_IMAGE,
		MANTICORE_MEASUREMENTS_EVENT_PCIE_PHY_IMAGE);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_PCIE_PHY_IMAGE,
		PCR_DMTF_VALUE_TYPE_FIRMWARE, false);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_PCIE_PHY_IMAGE,
		&fw_image_phy_measured_data);

	status = pcr_store_const_update_digest (&pcr_storage, PCR_MEASUREMENT_TYPE_SOC_PCIE_PHY_IMAGE,
		sp1_shared->pcr_log.soc.phy_image.digest.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_SOC_PCIE_PHY_IMAGE, status);
	}

#ifndef MANTICORE_ENABLE_ACVP
	/* Cerberus measurements */
	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1,
		PCR_TCG_EVENT_TYPE_PORT_1_PFM_DATA);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1,
		&pfm_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_VALID,
		PCR_TCG_EVENT_TYPE_PORT_1_INITIALIZATION_STATUS);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_VALID,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_VALID,
		&pfm_valid_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_ID,
		PCR_TCG_EVENT_TYPE_PORT_1_PFM_ID);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_ID,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_ID,
		&pfm_id_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_PLATFORM_ID,
		PCR_TCG_EVENT_TYPE_PORT_1_PFM_PLATFORM_ID);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_PLATFORM_ID,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_PLATFORM_ID,
		&pfm_platform_id_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD,
		PCR_TCG_EVENT_TYPE_PCD_DATA);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD,
		&pcd_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_ID,
		PCR_TCG_EVENT_TYPE_PCD_ID);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_ID,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_ID,
		&pcd_id_measured_data);

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_PLATFORM_ID,
		PCR_TCG_EVENT_TYPE_PCD_PLATFORM_ID);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_PLATFORM_ID,
		PCR_DMTF_VALUE_TYPE_FW_CONFIG, true);
	pcr_store_set_measurement_data (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_PLATFORM_ID,
		&pcd_platform_id_measured_data);
#endif	/* MANTICORE_ENABLE_ACVP */

	pcr_store_set_tcg_event_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_CHASSIS_INTRUSION,
		PCR_TCG_EVENT_TYPE_PLATFORM_CHASSIS_INTRUSION);
	pcr_store_set_dmtf_value_type (&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_CHASSIS_INTRUSION,
		PCR_DMTF_VALUE_TYPE_HW_CONFIG, true);
	/* Measurement data will be configured by the intrusion manager. */

	return 0;
}

#ifndef MANTICORE_ENABLE_ACVP
/**
 * Get the measurement data for the PFM valid measurement data, ensuring only dword accesses to the
 * data.
 *
 * @param context The sticky register containing the measured state.
 * @param offset Offset to start reading in the register.
 * @param buffer Output buffer for the data.
 * @param length The maximum amount of data to retrieve.
 * @param total_len Output for the total length of the measured data, regardless of how much data
 * is returned.
 *
 * @return The number of bytes written to the buffer or an error code.
 */
static int get_pfm_valid_measured_data (volatile uint32_t *context, size_t offset, uint8_t *buffer,
	size_t length, uint32_t *total_len)
{
	uint32_t copy;

	if ((context == NULL) || (buffer == NULL) || (total_len == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	copy = *context;
	*total_len = sizeof (copy);

	return buffer_copy ((uint8_t*) &copy, sizeof (copy), &offset, &length, buffer);
}

/**
 * Update a hash context with the PFM valid measurement data, ensuring only dword accesses to the
 * data.
 *
 * @param context The sticky register containing the measured state.
 * @param hash Hash context that will be updated with the data.
 *
 * @return 0 if the hash was updated successfully or an error code.
 */
static int hash_pfm_valid_measured_data (volatile uint32_t *context, const struct hash_engine *hash)
{
	uint32_t copy;

	if ((context == NULL) || (hash == NULL)) {
		return MANIFEST_MANAGER_INVALID_ARGUMENT;
	}

	copy = *context;

	return hash->update (hash, (uint8_t*) &copy, sizeof (copy));
}
#endif
