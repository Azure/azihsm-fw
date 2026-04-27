// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "build_version.h"
#include "FreeRTOS.h"
#include "manticore_hsp_gpio.h"
#include "manticore_sticky_regs.h"
#include "platform_api.h"
#include "platform_io_api.h"
#include "queue.h"
#include "reset_counter_init.h"
#include "sp_boot.h"
#include "system_observer_stack_usage.h"
#include "task.h"
#include "task_priority.h"
#include "asn1/base64_core_static.h"
#include "asn1/dice/x509_extension_builder_dice_tcbinfo_static.h"
#include "asn1/dice/x509_extension_builder_dice_ueid_static.h"
#include "asn1/x509_cert_build_static.h"
#include "cmd_interface/cerberus_protocol_required_commands.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "crypto/ecc_ccs_static.h"
#include "crypto/ecc_ecc_hw_static.h"
#include "crypto/ecc_hw_pka_static.h"
#include "drivers/hsp_aeb_static.h"
#include "firmware/manticore_device_keys.h"
#include "freertos/hsp_freertos.h"
#include "host_fw/host_logging.h"
#include "init/init_attestation.h"
#include "init/init_cmd.h"
#include "init/init_crypto.h"
#include "init/init_firmware.h"
#include "init/init_flash.h"
#include "init/init_host.h"
#include "init/init_log.h"
#include "init/init_manifest.h"
#include "init/init_system.h"
#include "init/task_stack_size.h"
#include "logging/boot_logging.h"
#include "logging/code_path_integrity.h"
#include "logging/init_logging.h"
#include "logging/logging_memory_static.h"
#include "mmio/mmio_register_block_hsp_static.h"
#include "mpu/hsp_mpu_static.h"
#include "mpu/memory_protection_mpu_only_static.h"
#include "riot/riot_core_hsp_static.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "sprt/manticore_sprt.h"
#include "system/manticore_aeb.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_trap.h"

/**
 * Data populated by ROM that can be used with local static initialization.
 */
static struct manticore_rom_shared_sram *const rom_shared_static =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * HSP MPU page size
 */
#define HSP_MPU_PAGE_SIZE	4096

/**
 * HSP MPU MMIO registers block.
 */
static const struct mmio_register_block_hsp mpu_regs =
	mmio_register_block_hsp_static_init ((uint32_t*) HSP_ADDR_MAP_CREG_MPU_REGS_ADDRESS,
	sizeof (struct Creg_regs_mpu));

/**
 * HSP MPU memory map
 */
static const struct hsp_mpu_memory_map_entry hsp_mpu_memory_map[] = {
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_ROM_ADDRESS,	/* SP ROM */
			.length = HSP_ADDR_MAP_SP_ROM_SIZE,
		},
		.user_register_offset = offsetof (struct Creg_regs_mpu,	sprom_mpu_regs.SPROM_USER_ATTRIB),
		.privileged_register_offset = offsetof (struct Creg_regs_mpu,
			sprom_mpu_regs.SPROM_PRIVILEGE_ATTRIB),
	},
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS,	/* SP iTCM */
			.length = HSP_ADDR_MAP_SP_IRAM_SIZE,
		},
		.user_register_offset = offsetof (struct Creg_regs_mpu,	spiram_mpu_regs.SPIRAM_USER_ATTRIB),
		.privileged_register_offset = offsetof (struct Creg_regs_mpu,
			spiram_mpu_regs.SPIRAM_PRIVILEGE_ATTRIB),
	},
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_DRAM_ADDRESS,	/* SP dTCM */
			.length = HSP_ADDR_MAP_SP_DRAM_SIZE,
		},
		.user_register_offset = offsetof (struct Creg_regs_mpu,	spdram_mpu_regs.SPDRAM_USER_ATTRIB),
		.privileged_register_offset = offsetof (struct Creg_regs_mpu,
			spdram_mpu_regs.SPDRAM_PRIVILEGE_ATTRIB),
	},
};

/**
 * HSP MPU driver
 */
static const struct hsp_mpu mpu = hsp_mpu_static_init (&mpu_regs.base, HSP_MPU_PAGE_SIZE,
	hsp_mpu_memory_map, ARRAY_SIZE (hsp_mpu_memory_map));


/**
 * Generate the 1SP version string based on the build version in the ROM log.
 */
static void initialize_1sp_version_str ()
{
	bool secure_boot = has_owner_key ();
	struct manticore_1sp_shared_data *sp1_shared_local =
		(struct manticore_1sp_shared_data*) sp1_shared;

	if (secure_boot) {
		MANTICORE_1SP_SET_SECURE_BOOT (sp1_shared_local);
	}
	else {
		MANTICORE_1SP_SET_UNSECURE_BOOT (sp1_shared_local);
	}

	memset (sp1_shared_local, 0, sizeof (*sp1_shared_local));

	/* Copying the build version into fw_descriptor which will be used in debug log */
	memcpy (sp1_shared_local->fw_descriptor.build_ver,
		rom_shared->firmware.pcr_log[1].fw_version.data.build_version,
		sizeof (rom_shared->firmware.pcr_log[1].fw_version.data.build_version));

	build_version_to_string (rom_shared->firmware.pcr_log[1].fw_version.data.build_version,
		secure_boot, false, sp1_shared_local->version_1sp, sizeof (sp1_shared_local->version_1sp));
}

/**
 * Cache for the SOCID that does not require word access.
 */
static uint32_t ueid[IN_DWORDS (HSP_FUSES_LENGTH (SOCID))];

/**
 * Variable context for the AEB driver.
 */
static struct hsp_aeb_state aeb_context;

/**
 * Interface for configuring AEBs.
 */
static const struct hsp_aeb aeb = hsp_aeb_static_init (&aeb_context,
	(struct Creg_regs_aeb_regs*) HSP_ADDR_MAP_CREG_AEB_INTERFACE_ADDRESS);


/**
 * Buffer in shared SRAM to use for unlock policy verification.  The only crypto operation used
 * during unlock verification is CCS HMAC, so all the shared crypto memory, except the CCS command
 * buffer, is available for use.
 */
#define	MANTICORE_UNLOCK_HMAC_BUFFER			rom_shared_static->internal.ccs.data

/**
 * Length of the unlock policy HMAC buffer.
 */
#define	MANTICORE_UNLOCK_HMAC_BUFFER_LENGTH		MANTICORE_ROM_CCS_HMAC_BUFFER_SIZE

/**
 * Security policy handler for the device.
 */

/* TODO: Fix SP Recovery device policy
 * Current code is using sp1_shared which does not exist. We need to limit to data available
 * after ROM initialization. */
const struct security_policy_hsp_manticore sec_policy =
	security_policy_hsp_manticore_static_init_constant_policy (
	(struct security_policy_hsp_manticore_data*) &recovery_device_policy, ueid);

/**
 * Handler for configuring memory protections for 1SP execution and SoC fences.
 */
static const struct memory_protection_mpu_only mem_protect =
	memory_protection_mpu_only_static_init (&mpu.base, NULL, 0);

/**
 * Variable context for the flash that contains the unlock policy.
 */
static struct flash_store_contiguous_blocks_state unlock_flash_context;

/**
 * Variable context for the hash API.
 */
static struct hash_engine_hs_sha_state hash_context;

/**
 * Hash engine wrapper for the HS-SHA driver.
 */
static const struct hash_engine_hs_sha hash = hash_hs_sha_static_init (&hash_context, &hash_hw);

/**
 * Flash block storage for the unlock policy.
 */
static const struct flash_store_contiguous_blocks unlock_flash =
	flash_store_contiguous_blocks_static_init_variable_storage_decreasing (
	FLASH_STORE_CONTIGUOUS_BLOCKS_WITH_HASH_API_INIT, &unlock_flash_context, MAIN_KEYSTORE_ADDR,
	MAIN_KEYSTORE_MAX_KEYS, &flash_internal.base, &hash.base);

/**
 * Variable context for the device security manager.
 */
static struct security_manager_hsp_state sec_manager_context;

/**
 * Security manager for applying the appropriate device security configuration.
 */
static const struct security_manager_hsp sec_manager =
	security_manager_hsp_static_init_only_apply_unlock (&sec_manager_context, &sec_policy.base,
	(const uint8_t*) &locked_device_policy, sizeof (locked_device_policy), &aeb, &fuses.base,
	HSP_FUSES_ADDRESS (AEB), HSP_FUSES_ADDRESS (RSVD1), MANTICORE_1SP_UNLOCK_COUNTER_LENGTH,
	&mem_protect.base, &shared_hash.base, &ccs.base, DEVICE_KEYS_DICE_CDI,
	MANTICORE_DEVICE_KEYS_NON_FIPS_DEVICE_ID_KEY, MANTICORE_DEVICE_KEYS_UNLOCK_HMAC_KEY,
	MANTICORE_UNLOCK_HMAC_BUFFER, MANTICORE_UNLOCK_HMAC_BUFFER_LENGTH, &unlock_flash.base,
	DEVICE_UNLOCK_POLICY, (uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_4_ADDRESS,
	DEVICE_UNLOCK_TOKEN_NONCE_LENGTH);

/**
 * ECC engine wrapper or the PKA driver.
 */
static const struct ecc_engine_ecc_hw ecc = ecc_ecc_hw_static_init (&pka.base, NULL);

/**
 * The ECC API to use for DICE keys.
 */
static const struct ecc_engine_ccs ecc_dice = ecc_ccs_static_init (&ccs.base, &ecc.base);

/**
 * Base64 encoder to use for DICE certificates.
 */
static const struct base64_engine_core base64 = base64_core_static_init;

/**
 * X.509 builder to use for DICE certificates.
 */
static const struct x509_engine_cert_build x509 = x509_cert_build_static_init (&ecc_dice.base,
	&shared_hash.base, MAX_FIPS_DEVID_CERT_LENGTH + 32);

/**
 * Buffer to use for building DICE certificate extensions.  A single buffer can be used for all
 * extensions.
 */
static uint8_t ext_buffer[768];

/**
 * Storage for the big endian representation of the layer 0 SVN value.
 */
static uint8_t layer0_svn[sizeof (uint32_t)];

/**
 * Cache for the value of PCR1 that does not require word access, for use in the layer 0 DICE
 * certificate.
 */
static uint8_t layer0_fwid[SHA384_HASH_LENGTH];

/**
 * List of FWIDs for DICE layer 0.
 */
static const struct tcg_dice_fwid layer0_fwid_list[] = {
	{
		.digest = layer0_fwid,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * Information about the TCB for DICE layer 0.
 */
static struct tcg_dice_tcbinfo layer0_tcb;

/**
 * Handler for the layer 0 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer0_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer0_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Buffer for the layer 1 FWID value.  This is only needed to keep the APIs happy since there is no
 * layer 1 for this image.
 */
static const uint8_t layer1_fwid[SHA384_HASH_LENGTH] = {0};

/**
 * Storage for the big endian representation of the layer 1 SVN value.
 */
static const uint8_t layer1_svn[1] = {0};

/**
 * List of FWIDs for DICE layer 1.
 */
static const struct tcg_dice_fwid layer1_fwid_list[] = {
	{
		.digest = layer1_fwid,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * Information about the TCB for DICE layer 1.
 */
static struct tcg_dice_tcbinfo layer1_tcb;

/**
 * Handler for the layer 1 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer1_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer1_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Handler for the Ueid extension for the layer 0 certificate.
 */
static const struct x509_extension_builder_dice_ueid layer0_ueid_ext =
	x509_extension_builder_dice_ueid_static_init_with_buffer ((uint8_t*) ueid, sizeof (ueid),
	ext_buffer, sizeof (ext_buffer));


/**
 * List of extensions to add to the DICE layer 0 certificate and CSR.
 */
static const struct x509_extension_builder *const layer0_ext[] = {
	&layer0_tcb_ext.base, &layer0_ueid_ext.base
};

/**
 * List of extensions to add to the DICE layer 1 certificate.
 */
static const struct x509_extension_builder *const layer1_ext[] = {&layer1_tcb_ext.base};

/**
 * Variable context for DICE layer 0 processing.
 */
static struct riot_core_hsp_state dice_context;

/**
 * DICE layer 0 handler.
 */
static const struct riot_core_hsp dice = riot_core_hsp_static_init (&dice_context, &ccs.base,
	&base64.base, &x509.base, DEVICE_KEYS_DICE_CDI, DEVICE_KEYS_DEVICE_ID_KEY,
	MANTICORE_DEVICE_KEYS_SP_ALIAS_KEY, layer0_ext, ARRAY_SIZE (layer0_ext), 0, layer1_ext,
	ARRAY_SIZE (layer1_ext));


/**
 * Initialize the device security policy for boot and run-time configuration.
 *
 * @return 0 if the security policy was successfully initialized.
 */
static int initialize_security_policy ()
{
	int status;

	status = flash_store_contiguous_blocks_init_state (&unlock_flash, 0);
	if (status != 0) {
		return status;
	}

	status = security_manager_hsp_init_state (&sec_manager);
	if (status != 0) {
		return status;
	}

	return status;
}

/**
 * Initialize the handler for generating DICE identity keys and certificates.
 *
 * @return 0 if the DICE handler was initialized successfully or an error code.
 */
static int initialize_dice ()
{
	struct ksu_pcr_slot *pcr = (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS;
	int status;

	/* Configure the TCB structure for layer 0. */
	buffer_reverse_copy (layer0_svn, (uint8_t*) &rom_shared->firmware.pcr_log[1].fw_svn.data.svn,
		sizeof (layer0_svn));
	memcpy (layer0_fwid, (uint32_t*) pcr[1].pcr, SHA384_HASH_LENGTH);

	layer0_tcb.version = sp1_shared->version_1sp;
	layer0_tcb.layer = 0;
	layer0_tcb.svn = layer0_svn;
	layer0_tcb.svn_length = sizeof (layer0_svn);
	layer0_tcb.fwid_list = layer0_fwid_list;
	layer0_tcb.fwid_count = ARRAY_SIZE (layer0_fwid_list);

	status = riot_core_hsp_init_state (&dice);
	if (status != 0) {
		return status;
	}

	return status;
}

/**
 * Copy a single DICE key/cert to the shared memory location.
 *
 * @param dest The destination for the DICE key.
 * @param dest_length The maximum length that can be copied.
 * @param length Output for the length of the copied data.  This will be -1 if the key doesn't fit
 * in the destination memory.
 * @param src The DICE key to copy.
 * @param src_length The DICE key length.
 */
static void copy_dice_key (uint8_t *dest, size_t dest_length, int *length, const uint8_t *src,
	size_t src_length)
{
	if (src_length <= dest_length) {
		memcpy (dest, src, src_length);
		*length = src_length;
	}
	else {
		*length = -1;
	}
}

/**
 * Generate the DICE identity keys and certificates for the SPRT firmware.
 *
 * @note This is not a real identity key since there is no SPRT firmware in recovery mode.
 * It's just created to satisfy the certificate management requirements.
 * The key is still device-specific, since it uses the CDI,
 * but it's not dependent on firmware or any other device state
 *
 * @return 0 if the SPRT DICE keys were generated successfully or an error code.
 */
static int generate_sprt_dice_keys ()
{
	uint8_t *der;
	size_t length;
	int status;
	struct manticore_1sp_shared_data *sp1_shared_local =
		(struct manticore_1sp_shared_data*) sp1_shared;

	/* CDI is not relevant here. */
	status = dice.base.generate_device_id (&dice.base, NULL, 0);
	if (status != 0) {
		return status;
	}

	layer1_tcb.version = sp1_shared_local->version_1sp;
	layer1_tcb.layer = 1;
	layer1_tcb.svn = layer1_svn;
	layer1_tcb.svn_length = sizeof (layer1_svn);
	layer1_tcb.fwid_list = layer1_fwid_list;
	layer1_tcb.fwid_count = ARRAY_SIZE (layer1_fwid_list);

	status = dice.base.generate_alias_key (&dice.base, layer1_fwid, SHA384_HASH_LENGTH);
	if (status != 0) {
		return status;
	}

	/* Export DICE keys and certs for SPRT. */
	status = dice.base.get_device_id_cert (&dice.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared_local->devid_cert_fips, sizeof (sp1_shared_local->devid_cert_fips),
		&sp1_shared_local->devid_cert_fips_length, der, length);
	platform_free (der);

	status = dice.base.get_device_id_csr (&dice.base, DICE_OID_MANTICORE, DICE_OID_MANTICORE_LENGTH,
		&der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared_local->devid_csr_fips, sizeof (sp1_shared_local->devid_csr_fips),
		&sp1_shared_local->devid_csr_fips_length, der, length);
	platform_free (der);

	status = dice.base.get_alias_key (&dice.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared_local->alias_key, sizeof (sp1_shared_local->alias_key),
		&sp1_shared_local->alias_key_length, der, length);
	platform_free (der);

	status = dice.base.get_alias_key_cert (&dice.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared_local->alias_cert, sizeof (sp1_shared_local->alias_cert),
		&sp1_shared_local->alias_cert_length, der, length);
	platform_free (der);

	return 0;
}

/**
 * Task that will run system initialization.
 *
 * @param unused Unused.
 */
static void manticore_init (void *unused)
{
	int status;
	int error_msg = -1;
	int msg_arg = 0;

	UNUSED (unused);

	/* AEB init*/
	status = hsp_aeb_init_state (&aeb);
	if (status != 0) {
		platform_printf ("AEB Init FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* This doesn't impact any execution prior to this point and needs to be done after the AEB
	 * driver has been initialized. */
	handle_silicon_errata (&aeb, ueid[0], NULL, NULL);

	status = initialize_and_increment_reset_counter ();
	if (status != 0) {
		platform_printf ("Reset Ctr FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* Initialize core system components. */
	status = initialize_manticore_flash ();
	if (status != 0) {
		goto error;
	}

	initialize_debug_log ();

	status = initialize_security_policy ();
	if (status != 0) {
		error_msg = BOOT_LOGGING_INIT_SECURITY_POLICY;
		goto error;
	}

	/* Allow access to the AEB fuses so AEMC AEB fuses can be checked/updated. */
	status = aeb.enable_aeb (&aeb, MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES);
	if (status != 0) {
		platform_printf ("Enable aeb fuse access FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = sec_manager.base.apply_device_config (&sec_manager.base);
	if (status != 0) {
		platform_printf ("Apply security config FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_APPLY_SECURITY_CONFIG;
		goto error;
	}

	status = initialize_crypto_hardware ();
	if (status != 0) {
		error_msg = INIT_LOGGING_HW_CRYPTO;
		goto error;
	}

	status = initialize_system_crypto ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_CRYPTO;
		goto error;
	}

	status = initialize_dice ();
	if (status != 0) {
		platform_printf ("initialize_dice Failed." NEWLINE NEWLINE);
		error_msg = INIT_LOGGING_RIOT_MANAGER;
		goto error;
	}

	status = generate_sprt_dice_keys ();
	if (status != 0) {
		platform_printf ("generate_sprt_dice_keys Failed." NEWLINE NEWLINE);
		error_msg = INIT_LOGGING_RIOT_MANAGER;
		goto error;
	}

	status = initialize_dice_key_manager ();
	if (status != 0) {
		platform_printf ("initialize_dice_key_manager Failed." NEWLINE NEWLINE);
		goto error;
	}

	status = initialize_system_management ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_STATE;
		goto error;
	}

#ifdef DEBUG_STACK_USAGE
	status = enable_stack_usage_monitoring ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_STATE;
		goto error;
	}
#endif

	status = initialize_firmware_updater (false);
	if (status != 0) {
		error_msg = INIT_LOGGING_FW_UPDATER;
		goto error;
	}

	status = initialize_cmd_interface_recovery ();
	if (status != 0) {
		error_msg = INIT_LOGGING_COMMAND_HANDLER;
		goto error;
	}

	/* Everything has been initialized.  Start running system tasks. */

	/* TODO:  Confirm stack requirements on this platform. */
	status = allocate_firmware_update_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_FW_UPDATE_TASK;
		goto error;
	}

	event_task_freertos_start (&manticore_update);

	/* This is the recovery image, so ignore the contents of flash.  This could be used in scenarios
	 * where neither image is good, an the updater needs to not block the update because of that. */
	firmware_update_set_recovery_good (&fw_updater, true);

	status = start_cmd_interface ();
	if (status != 0) {
		error_msg = INIT_LOGGING_COMMAND_HANDLER_START;
		goto error;
	}

	platform_printf ("System Initialized." NEWLINE NEWLINE);

	/* At this point, core initialization has completed and the hosts are running.  Set the SPRT
	 * flag to detect the state correctly on the next reset and not interrupt host operation.  The
	 * init flag is not set before all authentication is done to ensure proper handling of
	 * unexpected reset events during flash validation. */
	system_init_done ();

#ifdef DEBUG_STACK_USAGE
	system_observer_stack_usage_print_all_tasks_usage ();
#endif

	/* Full initialization has completed successfully. */
	boot_error_clear_counter (sw_regs);

	vTaskDelete (NULL);

error:
	if (error_msg >= 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT, error_msg,
			status, msg_arg);

		platform_printf ("System initialization failed: msg=%d, status=0x%x, arg=%d" NEWLINE,
			error_msg, status, msg_arg);
	}
	else {
		platform_printf ("System initialization failed: status=0x%x" NEWLINE, status);
	}
	debug_log_flush ();

	platform_printf (NEWLINE);

	/* Error during initialization.  Just wait for a reset. */
	CEASE;
}

/**
 * Stack guard to check for overflows.
 */
extern uint32_t __stack_chk_guard;


/**
 * No code path integrity enabled for this image.
 */
CODE_PATH_INTEGRITY_NONE;


/**
 * Entry point for Manticore SPRT.
 */
int main ()
{
	int status;
	const char *reset;
	uint32_t random_val;
	const uint32_t *socid = (const uint32_t*) HSP_ADDR_MAP_GFC_SOCID_ADDRESS;

	hardware_init (&reset);

	/* Initialize the 1SP version string. */
	initialize_1sp_version_str ();

	/* Cache the SOCID in a byte-addressable buffer to allow general purpose use of the data by
	 * firmware. */
	memcpy (ueid, socid, sizeof (ueid));

	platform_printf (NEWLINE);
	platform_printf ("Manticore Recovery: %s " NEWLINE, sp1_shared->version_1sp);
	platform_printf ("System Clock: %d Hz" NEWLINE, HSP_CLOCK_FREQUENCY_HZ);
	platform_printf ("Reset Cause: %s" NEWLINE, reset);
	platform_printf (NEWLINE);

	status = initialize_rng ();
	if (status != 0) {
		platform_printf ("RNG FAILED: 0x%x" NEWLINE, status);
		goto errors;
	}

	/* Apply a random value to the stack guard.  It can't be done in the context of a function
	 * call */
	hsp_rng_hw_get_random_word (&rng_hw, &random_val);
	__stack_chk_guard = random_val;

	/* Initialize and enable interrupts. */
	status = hsp_interrupt_init (true);
	if (status != 0) {
		platform_printf ("IRQ INIT FAILED: 0x%x" NEWLINE, status);
		goto errors;
	}

	status = hsp_freertos_init ();
	if (status != 0) {
		platform_printf ("FreeRTOS INIT FAILED: 0x%x" NEWLINE, status);
		goto errors;
	}

	/* This can be a low priority task since nothing else is running during initialization.  Once
	 * init has completed, it will run additional background operations that need to be low
	 * priority. */
	status = xTaskCreate (manticore_init, "Init", 7 * 256, NULL, CERBERUS_PRIORITY_BACKGROUND,
		NULL);
	if (status == pdPASS) {
		vTaskStartScheduler ();
		platform_printf ("Returned from FreeRTOS scheduler!?" NEWLINE);
	}
	else {
		platform_printf ("Failed to create init task (0x%x)!" NEWLINE, status);
	}

errors:
	/* Error during initialization.  Just wait for a reset. */
	CEASE;
}
