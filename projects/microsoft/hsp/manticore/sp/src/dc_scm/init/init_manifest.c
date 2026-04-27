// Copyright (c) Microsoft Corporation. All rights reserved.

#include "init_attestation.h"
#include "init_crypto.h"
#include "init_firmware.h"
#include "init_flash.h"
#include "init_host.h"
#include "init_manifest.h"
#include "init_system.h"
#include "manticore_pcr.h"
#include "platform_io_api.h"
#include "rot_memory_map.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "asn1/ecc_der_util.h"
#include "common/array_size.h"
#include "crypto/signature_verification_ecc_static.h"
#include "manifest/manifest_verification_static.h"
#include "manifest/pcd/pcd_flash_static.h"
#include "manifest/pfm/pfm_flash_static.h"
#include "manifest/pfm/pfm_observer_pending_reset_static.h"
#include "splibs/hsprt/riscvcpu.h"


/**
 * Default key to use for manifest verification.  Production manifest key (CP-500671-Key) signed
 * with the production manifest root key (Manticore_ManifestRoot_P384).
 */
#if ECC_MAX_KEY_LENGTH == ECC_KEY_LENGTH_521
const struct manifest_verification_key_ecc manifest_ecc_key MANIFEST_KEY_ATTRIBUTE = {
	.id = 2,
	.key = {
		0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86,
		0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b,
		0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04,
		0x7f, 0x01, 0x59, 0x16, 0x73, 0x7c, 0x59, 0xf4,
		0xf4, 0x33, 0x51, 0x01, 0xf0, 0xec, 0xa4, 0x4a,
		0x7d, 0xd4, 0x52, 0xc8, 0xba, 0x5d, 0x88, 0xd7,
		0x62, 0x7b, 0xc7, 0xfc, 0xf1, 0xde, 0xee, 0xfd,
		0xad, 0xe2, 0x25, 0xd4, 0x81, 0x27, 0x23, 0x09,
		0x81, 0x58, 0x8e, 0x5a, 0x4c, 0xf0, 0xcc, 0x22,
		0x76, 0xcd, 0xa7, 0xc0, 0x2a, 0x49, 0xda, 0x6b,
		0x9a, 0x5e, 0xec, 0x25, 0x1f, 0x29, 0x79, 0xca,
		0x47, 0x60, 0xdd, 0x13, 0x8b, 0xbe, 0x55, 0xca,
		0x14, 0x03, 0xd8, 0xf4, 0x2d, 0x64, 0xda, 0xc2,
		0xe4, 0xb6, 0xa1, 0xc7, 0x17, 0x4a, 0xeb, 0xad,
		0x00, 0x0c, 0x15, 0xca, 0xa5, 0x3c, 0xa8, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	.signature = {
		0x30, 0x66, 0x02, 0x31, 0x00, 0xf0, 0x84, 0xc0,
		0xb9, 0x49, 0xa9, 0x5d, 0x7a, 0x7e, 0x7b, 0xf5,
		0x29, 0x20, 0x72, 0xa9, 0xd5, 0xb7, 0x00, 0xfe,
		0x9f, 0x12, 0xba, 0xff, 0x00, 0x99, 0xbd, 0x30,
		0xbe, 0x21, 0x2c, 0xcd, 0x09, 0xc0, 0x97, 0xf2,
		0x6c, 0x18, 0x94, 0x2b, 0x5e, 0x6c, 0x56, 0xcb,
		0x3b, 0x21, 0x73, 0x82, 0x12, 0x02, 0x31, 0x00,
		0x8a, 0xa0, 0xfa, 0x1a, 0x1d, 0x47, 0x7b, 0xa5,
		0x66, 0xa1, 0x1a, 0xc5, 0xaf, 0xe7, 0xf0, 0x97,
		0x97, 0xbf, 0xd4, 0xd4, 0x69, 0xda, 0x10, 0x94,
		0x08, 0x22, 0x19, 0x1b, 0x9c, 0x32, 0x39, 0x9f,
		0x09, 0xf7, 0xc9, 0xfd, 0x21, 0xf0, 0x24, 0x58,
		0xce, 0xbe, 0x7d, 0x6f, 0x82, 0x09, 0x45, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00
	}
};
#else
const struct manifest_verification_key_ecc manifest_ecc_key MANIFEST_KEY_ATTRIBUTE = {
	.id = 2,
	.key = {
		0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86,
		0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b,
		0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04,
		0x7f, 0x01, 0x59, 0x16, 0x73, 0x7c, 0x59, 0xf4,
		0xf4, 0x33, 0x51, 0x01, 0xf0, 0xec, 0xa4, 0x4a,
		0x7d, 0xd4, 0x52, 0xc8, 0xba, 0x5d, 0x88, 0xd7,
		0x62, 0x7b, 0xc7, 0xfc, 0xf1, 0xde, 0xee, 0xfd,
		0xad, 0xe2, 0x25, 0xd4, 0x81, 0x27, 0x23, 0x09,
		0x81, 0x58, 0x8e, 0x5a, 0x4c, 0xf0, 0xcc, 0x22,
		0x76, 0xcd, 0xa7, 0xc0, 0x2a, 0x49, 0xda, 0x6b,
		0x9a, 0x5e, 0xec, 0x25, 0x1f, 0x29, 0x79, 0xca,
		0x47, 0x60, 0xdd, 0x13, 0x8b, 0xbe, 0x55, 0xca,
		0x14, 0x03, 0xd8, 0xf4, 0x2d, 0x64, 0xda, 0xc2,
		0xe4, 0xb6, 0xa1, 0xc7, 0x17, 0x4a, 0xeb, 0xad,
		0x00, 0x0c, 0x15, 0xca, 0xa5, 0x3c, 0xa8, 0x00
	},
	.signature = {
		0x30, 0x64, 0x02, 0x30, 0x17, 0xdf, 0x5b, 0xeb,
		0x87, 0x99, 0x84, 0xa2, 0x5d, 0x75, 0x1b, 0x5e,
		0x14, 0xd7, 0xf9, 0xc3, 0x81, 0xb4, 0xee, 0x2d,
		0x4d, 0xe9, 0xe1, 0x45, 0x66, 0x13, 0xdd, 0xfd,
		0x82, 0xb9, 0xdd, 0xa7, 0xa6, 0xef, 0xee, 0x88,
		0x15, 0xf6, 0xb3, 0x93, 0x98, 0x53, 0x5c, 0xd0,
		0xf3, 0xe5, 0x18, 0xb4, 0x02, 0x30, 0x64, 0xa9,
		0xcd, 0xf1, 0x69, 0xb6, 0xcd, 0xc2, 0x04, 0x08,
		0x2e, 0xc0, 0x86, 0xbb, 0x39, 0x8b, 0x8a, 0x8c,
		0x9f, 0x71, 0x38, 0x7c, 0x05, 0xf1, 0xf7, 0xcb,
		0x72, 0x80, 0x9a, 0x10, 0x3d, 0x1c, 0x0f, 0xc6,
		0x94, 0xf7, 0xfd, 0x8f, 0x03, 0x59, 0x30, 0xa2,
		0xe4, 0xea, 0x88, 0xba, 0xd4, 0x88, 0x00, 0x00
	}
};
#endif

/**
 * Wrapper for the manifest verification key.
 */
static const struct manifest_verification_key manifest_key = {
	.key_data = (const uint8_t*) &manifest_ecc_key,
	.key_data_length = sizeof (struct manifest_verification_key_ecc),
	.key = (const struct manifest_verification_key_header*) &manifest_ecc_key,
	.pub_key_length = sizeof (manifest_ecc_key.key),
	.signature = (const uint8_t*) &manifest_ecc_key.signature,
	.sig_length = sizeof (manifest_ecc_key.signature),
	.sig_hash = HASH_TYPE_SHA384
};

/**
 * Command task handler for manifests.
 */
static const struct event_task_handler *const manifest_cmd_task_handler[] = {
	&pcd_handler.base.base_event, &pfm_handler.base.base_event, &host_handler.base_event
};

/**
 * Variable context for the task context for manifest commands.
 */
static struct event_task_freertos_state manifest_cmd_task_context;

/**
 * Command task for executing manifest commands.
 */
const struct event_task_freertos manifest_cmd_task =
	event_task_freertos_static_init (&manifest_cmd_task_context, &system_mgr,
	manifest_cmd_task_handler, ARRAY_SIZE (manifest_cmd_task_handler));

/**
 * Statically allocated task control block for the manifest command handler task.
 */
static StaticTask_t manifest_cmd_task_tcb;

/**
 * Statically allocated stack for the manifest command handler task.
 */
static StackType_t manifest_cmd_task_stack[MANIFEST_CMD_TASK_STACK_WORDS];


/**
 * Variable context for the PCD ECC verification wrapper.
 */
static struct signature_verification_ecc_state pcd_ecc_verify_context;

/**
 * Wrapper for PCD ECC verification.
 */
static const struct signature_verification_ecc pcd_ecc_verify =
	signature_verification_ecc_static_init (&pcd_ecc_verify_context, &shared_ecc.base);

/**
 * Variable context for PCD signature verification.
 */
static struct manifest_verification_state pcd_verification_context;

/**
 * Signature verification for the PCD.
 *
 * TODO:  It would be good to have the ability to set the root key pointer as part of the constant
 * instance.  This may not always be available, but it is here by using
 * KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys, MANTICORE_FW_KEYS_MANIFEST_ROOT_KEY).
 */
static const struct manifest_verification pcd_verification =
	manifest_verification_static_init (&pcd_verification_context, &shared_hash.base,
	&pcd_ecc_verify.base, &manifest_key, &main_keystore.base, PCD_VERIFICATION_KEY);

/**
 * Buffer for storing the PCD signature in region 1.
 */
static uint8_t pcd_region1_signature[ECC_DER_P384_ECDSA_MAX_LENGTH];

/**
 * Buffer for storing the PCD platform ID in region 1.
 */
static uint8_t pcd_region1_platform_id[MANIFEST_MAX_STRING];

/**
 * Variable context for the PCD in the first region of PCD flash.
 */
static struct pcd_flash_state pcd_region1_context;

/**
 * The PCD stored in the first region allocated for PCDs.
 */
static const struct pcd_flash pcd_region1 = pcd_flash_static_init (&pcd_region1_context,
	&flash_internal.base, &shared_hash.base, pcd_region_addr[0], pcd_region1_signature,
	sizeof (pcd_region1_signature), pcd_region1_platform_id, sizeof (pcd_region1_platform_id));

/**
 * Buffer for storing the PCD signature in region 1.
 */
static uint8_t pcd_region2_signature[ECC_DER_P384_ECDSA_MAX_LENGTH];

/**
 * Buffer for storing the PCD platform ID in region 1.
 */
static uint8_t pcd_region2_platform_id[MANIFEST_MAX_STRING];

/**
 * Variable context for the PCD in the second region of PCD flash.
 */
static struct pcd_flash_state pcd_region2_context;

/**
 * The PCD stored in the second region allocated for PCDs.
 */
static const struct pcd_flash pcd_region2 = pcd_flash_static_init (&pcd_region2_context,
	&flash_internal.base, &shared_hash.base, pcd_region_addr[1], pcd_region2_signature,
	sizeof (pcd_region2_signature), pcd_region2_platform_id, sizeof (pcd_region2_platform_id));

/**
 * Variable context for PCD management.
 */
static struct pcd_manager_flash_state platform_config_context;

/**
 * Management for the PCD.
 */
const struct pcd_manager_flash platform_config =
	pcd_manager_flash_static_init (&platform_config, &platform_config_context, &pcd_region1,
	&pcd_region2, &system_state, &shared_hash.base, &pcd_verification.base_verify);

/**
 * Variable context for the PCD handler.
 */
static struct manifest_cmd_handler_state pcd_handler_context;

/**
 * Command handler for PCD operations.
 */
const struct manifest_cmd_handler_pcd pcd_handler =
	manifest_cmd_handler_pcd_static_init (&pcd_handler_context, &platform_config.base.base,
	&manifest_cmd_task.base);

/**
 * PCR management for the PCD.
 */
const struct pcd_observer_pcr pcr_pcd = pcd_observer_pcr_static_init (&shared_hash.base,
	&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PCD, PCR_MEASUREMENT_TYPE_CERBERUS_PCD_ID,
	PCR_MEASUREMENT_TYPE_CERBERUS_PCD_PLATFORM_ID);


/**
 * Flag indicating if the system was initialized with an active PCD.
 */
SECTION (".sprtro.has_active_pcd")
bool has_active_pcd;


/**
 * Initialize management of the platform configuration manifest.
 *
 * @return 0 if platform manifest management was successfully initialized or an error code.
 */
int initialize_pcd_management ()
{
	const struct key_manifest_public_key *root_key;
	const struct pcd *active_pcd;
	int status;

	status = pcd_flash_init_state (&pcd_region1);
	if (status != 0) {
		return status;
	}

	status = pcd_flash_init_state (&pcd_region2);
	if (status != 0) {
		return status;
	}

	status = signature_verification_ecc_init_state (&pcd_ecc_verify, NULL, 0);
	if (status != 0) {
		return status;
	}

	root_key = img_keys.base.get_manifest_key (&img_keys.base);

	status = manifest_verification_init_state (&pcd_verification, root_key->key.ecc_der_ref.der,
		root_key->key.ecc_der_ref.length);
	if (status != 0) {
		return status;
	}

	status = pcd_manager_flash_init_state (&platform_config);
	if (status != 0) {
		return status;
	}

	status = pcd_manager_add_observer (&platform_config.base,
		manifest_verification_get_pcd_observer (&pcd_verification));
	if (status != 0) {
		return status;
	}

	status = firmware_update_add_observer (&fw_updater, &pcd_verification.base_update);
	if (status != 0) {
		return status;
	}

	status = pcd_manager_add_observer (&platform_config.base, &pcr_pcd.base);
	if (status != 0) {
		return status;
	}

	active_pcd = platform_config.base.get_active_pcd (&platform_config.base);
	platform_printf ("PCD: active=0x%x" NEWLINE, active_pcd);
	has_active_pcd = (active_pcd != NULL);
	platform_config.base.free_pcd (&platform_config.base, active_pcd);

	status = manifest_cmd_handler_pcd_init_state (&pcd_handler);
	if (status != 0) {
		return status;
	}

	pcd_observer_pcr_record_measurement (&pcr_pcd, &platform_config.base);

	return 0;
}


/**
 * Variable context for the PFM ECC verification wrapper.
 */
static struct signature_verification_ecc_state pfm_ecc_verify_context;

/**
 * Wrapper for PFM ECC verification.
 */
static const struct signature_verification_ecc pfm_ecc_verify =
	signature_verification_ecc_static_init (&pfm_ecc_verify_context, &shared_ecc.base);

/**
 * Variable context for PFM signature verification.
 */
static struct manifest_verification_state pfm_verification_context;

/**
 * Signature verification for PFMs.
 *
 * TODO:  It would be good to have the ability to set the root key pointer as part of the constant
 * instance.  This may not always be available, but it is here by using
 * KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys, MANTICORE_FW_KEYS_MANIFEST_ROOT_KEY).
 */
static const struct manifest_verification pfm_verification =
	manifest_verification_static_init (&pfm_verification_context, &shared_hash.base,
	&pfm_ecc_verify.base, &manifest_key, &main_keystore.base, PFM_PORT1_VERIFICATION_KEY);

/**
 * Reset management for pending PFMs for hosts with reset notification.
 */
static const struct pfm_observer_pending_reset host_pending_reset =
	pfm_observer_pending_reset_static_init (&host_gpio.base);

/**
 * Buffer for storing the PFM signature in region 1.
 */
static uint8_t pfm_region1_signature[ECC_DER_P384_ECDSA_MAX_LENGTH];

/**
 * Buffer for storing the PFM platform ID in region 1.
 */
static uint8_t pfm_region1_platform_id[MANIFEST_MAX_STRING];

/**
 * Variable context for the PFM stored in the first PFM flash region.
 */
static struct pfm_flash_state pfm_region1_context;

/**
 * The host PFMs stored in the first region allocated for PFMs.
 */
static const struct pfm_flash pfm_region1 = pfm_flash_v2_static_init (&pfm_region1_context,
	&flash_internal.base, &shared_hash.base, host_pfm_region1_addr, pfm_region1_signature,
	sizeof (pfm_region1_signature), pfm_region1_platform_id, sizeof (pfm_region1_platform_id));

/**
 * Buffer for storing the PFM signature in region 2.
 */
static uint8_t pfm_region2_signature[ECC_DER_P384_ECDSA_MAX_LENGTH];

/**
 * Buffer for storing the PFM platform ID in region 2.
 */
static uint8_t pfm_region2_platform_id[MANIFEST_MAX_STRING];

/**
 * Variable context for the PFM stored in the second PFM flash region.
 */
static struct pfm_flash_state pfm_region2_context;

/**
 * The PFMs stored in the second region allocated for PFMs.
 */
static const struct pfm_flash pfm_region2 = pfm_flash_v2_static_init (&pfm_region2_context,
	&flash_internal.base, &shared_hash.base, host_pfm_region2_addr, pfm_region2_signature,
	sizeof (pfm_region2_signature), pfm_region2_platform_id, sizeof (pfm_region2_platform_id));

/**
 * Variable context for host firmware PFM management.
 */
static struct pfm_manager_flash_state host_fw_manifest_context;

/**
 * Management for PFMs describing host firmware.
 */
const struct pfm_manager_flash host_fw_manifest =
	pfm_manager_flash_static_init (&host_fw_manifest, &host_fw_manifest_context, &pfm_region1,
	&pfm_region2, &host_state, &shared_hash.base, &pfm_verification.base_verify, 1);

/**
 * Variable context for the PFM handlers.
 */
static struct manifest_cmd_handler_state pfm_handler_context;


/**
 * Command handler for PFM operations.
 *
 * Static initialization doesn't work here since it gets initialized either as a base type or a PFM
 * handler depending on the run-time activation setting.  This is a very clean approach and fits
 * with object oriented design, but it prevents the use of a constant instance.  A flag could be
 * added to the PFM handler to enable the run-time flows to support a constant instance, but is not
 * as clean of an implementation.  Complicating things is the list of handlers in the
 * manifest_cmd_task, meaning there can't even be both types as static instances that are selected
 * at run-time.  That would prevent the list of handlers from being constant.
 *
 * Due to these challenges, mark it as RO after initialization is done.
 */
SECTION (".sprtro.pfm_handler")
struct manifest_cmd_handler_pfm pfm_handler;

/**
 * PCR management for PFMs.
 */
const struct pfm_observer_pcr pcr_pfm = pfm_observer_pcr_static_init (&shared_hash.base,
	&pcr_storage, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_ID,
	PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_PLATFORM_ID);


/**
 * Initialize PFM management for host firmware.
 *
 * @param reset_notify Flag indicating the host processor should be notified of PFM events through
 * the reset control signal.
 * @param run_time_activation Flag indicating if the port supports run-time activation of PFMs.
 *
 * @return 0 if the PFM management was successfully initialized or an error code.
 */
int initialize_host_pfm_management (bool reset_notify, bool run_time_activation)
{
	const struct key_manifest_public_key *root_key;
	int status;

	status = pfm_flash_init_state (&pfm_region1);
	if (status != 0) {
		return status;
	}

	status = pfm_flash_init_state (&pfm_region2);
	if (status != 0) {
		return status;
	}

	status = signature_verification_ecc_init_state (&pfm_ecc_verify, NULL, 0);
	if (status != 0) {
		return status;
	}

	root_key = img_keys.base.get_manifest_key (&img_keys.base);

	status = manifest_verification_init_state (&pfm_verification, root_key->key.ecc_der_ref.der,
		root_key->key.ecc_der_ref.length);
	if (status != 0) {
		return status;
	}

	status = pfm_manager_flash_init_state (&host_fw_manifest);
	if (status != 0) {
		return status;
	}

	status = pfm_manager_add_observer (&host_fw_manifest.base,
		manifest_verification_get_pfm_observer (&pfm_verification));
	if (status != 0) {
		return status;
	}

	status = firmware_update_add_observer (&fw_updater, &pfm_verification.base_update);
	if (status != 0) {
		return status;
	}

	status = pfm_manager_add_observer (&host_fw_manifest.base, &pcr_pfm.base);
	if (status != 0) {
		return status;
	}

	if (reset_notify) {
		status = pfm_manager_add_observer (&host_fw_manifest.base, &host_pending_reset.base);
		if (status != 0) {
			return status;
		}
	}

	if (!run_time_activation) {
		status = manifest_cmd_handler_init (&pfm_handler.base, &pfm_handler_context,
			&host_fw_manifest.base.base, &manifest_cmd_task.base);
	}
	else {
		status = manifest_cmd_handler_pfm_init (&pfm_handler, &pfm_handler_context,
			&host_fw_manifest.base.base, &manifest_cmd_task.base, &host_manager.base, &host_state,
			&shared_hash.base, &shared_rsa.base, &host_filter.base);
	}
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the task for handling manifest commands.
 *
 * @return 0 if the task was initialized successfully or an error code.
 */
int initialize_manifest_command_task ()
{
	return event_task_freertos_init_state (&manifest_cmd_task);
}

/**
 * Start the task for handling manifest commands.
 *
 * @return 0 if the task was started successfully or an error code.
 */
int start_manifest_command_task ()
{
	int status;

	status = event_task_freertos_allocate_static (&manifest_cmd_task, &manifest_cmd_task_tcb,
		manifest_cmd_task_stack, MANIFEST_CMD_TASK_STACK_WORDS, "config_cmd",
		CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	event_task_freertos_start (&manifest_cmd_task);

	return 0;
}
