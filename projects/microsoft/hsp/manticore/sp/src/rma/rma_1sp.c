// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "FreeRTOS.h"
#include "hsp_top.h"
#include "manticore_rom.h"
#include "periodic_task_freertos_static.h"
#include "platform_config.h"
#include "platform_io_api.h"
#include "traps.h"
#include "asn1/base64_core_static.h"
#include "asn1/dice/x509_extension_builder_dice_tcbinfo_static.h"
#include "asn1/dice/x509_extension_builder_dice_ueid_static.h"
#include "asn1/dme/dme_structure_raw_ecc.h"
#include "asn1/dme/x509_extension_builder_dme_static.h"
#include "asn1/ecc_der_util.h"
#include "asn1/x509_cert_build_static.h"
#include "cmd_interface/cmd_channel_handler_static.h"
#include "cmd_interface/cmd_channel_i2c_dw_apb_multimaster_static.h"
#include "cmd_interface/cmd_device_hsp_static.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "cmd_interface/device_manager.h"
#include "common/array_size.h"
#include "common/msft_device_id.h"
#include "crypto/ecc_ccs_static.h"
#include "crypto/ecc_ecc_hw_static.h"
#include "crypto/ecc_hw_pka_static.h"
#include "crypto/hash_hs_sha_static.h"
#include "crypto/signature_verification_ecc_static.h"
#include "dc_scm/1sp/manticore_1sp.h"
#include "dc_scm/build_version.h"
#include "dc_scm/init/init_cmd.h"
#include "dc_scm/reset_counter_init.h"
#include "dc_scm/sp_boot.h"
#include "dc_scm/sprt/manticore_sprt.h"
#include "drivers/ccs_ksu_static.h"
#include "drivers/fuse_controller_sw0_rng_calibration_static.h"
#include "drivers/hs_sha_static.h"
#include "drivers/hsp_aes_static.h"
#include "drivers/hsp_dmb_static.h"
#include "drivers/hsp_fuses.h"
#include "drivers/hsp_rng_hw_static.h"
#include "drivers/i2c_dw_apb_multimaster_static.h"
#include "firmware/identity_renewal_static.h"
#include "firmware/manticore_device_keys.h"
#include "freertos/hsp_freertos.h"
#include "logging/code_path_integrity.h"
#include "manifest/pcd/pcd.h"
#include "mctp/cmd_interface_mctp_control.h"
#include "mctp/cmd_interface_protocol_mctp_msft_vdm_static.h"
#include "mctp/cmd_interface_protocol_mctp_static.h"
#include "mctp/mctp_interface_static.h"
#include "mctp/msft_mctp_base_protocol.h"
#include "msft_protocol/cmd_interface_msft_rot_static.h"
#include "msft_protocol/cmd_interface_protocol_msft_static.h"
#include "msft_protocol/msft_base_commands_static.h"
#include "msft_protocol/msft_mctp_protocol.h"
#include "msft_protocol/rot_commands.h"
#include "riot/dice_oid.h"
#include "riot/riot_core_hsp_static.h"
#include "riot/tcg_dice.h"
#include "rma/cmd_interface_rma_static.h"
#include "rma/device_rma_transition_hsp_retest_static.h"
#include "rma/rma_unlock_token_static.h"
#include "rma/secure_device_unlock_rma_static.h"
#include "rom/device_keys.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_trap.h"


/**
 * Public key to use for authenticating RMA unlock tokens.
 */
static const uint8_t RMA_AUTH_PUBLIC_KEY[] = {
	0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b,
	0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04, 0x02, 0x57, 0xf5, 0x66, 0x52, 0xb3, 0xa2, 0x1f,
	0x51, 0xef, 0x25, 0x25, 0xd2, 0xde, 0xf2, 0xe9, 0xa9, 0x4a, 0x68, 0x0c, 0xd2, 0xa7, 0xf7, 0x88,
	0x0d, 0xf7, 0x32, 0x43, 0x55, 0x51, 0x57, 0x28, 0x48, 0x3e, 0xed, 0x60, 0x93, 0x8f, 0x92, 0x0f,
	0xcd, 0x7d, 0x76, 0x8c, 0xd0, 0x8f, 0xd3, 0x3d, 0x1e, 0x88, 0x49, 0xef, 0x29, 0xc5, 0x27, 0x01,
	0x2c, 0x1b, 0x6b, 0xb0, 0x44, 0xea, 0x50, 0x02, 0xab, 0xf3, 0x57, 0x0e, 0xc3, 0x3c, 0xc0, 0xb8,
	0x14, 0x09, 0x63, 0xd2, 0xcf, 0xc4, 0x33, 0xef, 0x9e, 0x3f, 0x64, 0x7d, 0xd6, 0xd0, 0xcc, 0x8a,
	0x5c, 0x78, 0x1b, 0x9b, 0x04, 0xc2, 0x63, 0x52
};

/**
 * Length of the RMA authorization public key.
 */
const uint32_t RMA_AUTH_PUBLIC_KEY_LEN = sizeof (RMA_AUTH_PUBLIC_KEY);

/**
 * Regions of SoC SRAM that should be cleared as part of the RETEST transition.
 */
static const struct soc_sram_block soc_sram[] = {
	{
		.start = 0x60000000,	/* CP ITCM */
		.length = (512 * 1024)
	},
	{
		.start = 0x60200000,	/* CP0 DTCM */
		.length = (256 * 1024)
	},
	{
		.start = 0x60600000,	/* CP1 DTCM */
		.length = (256 * 1024)
	},
	{
		.start = 0xa2000000,	/* FP0 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3000000,	/* FP0 DTCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa2200000,	/* FP1 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3200000,	/* FP1 DTCM */
		.length = (64 * 1024)
	},
	{
		.start = 0xa2400000,	/* FP2 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3400000,	/* FP2 DTCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3020000,	/* FP0-FP1 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3030000,	/* FP0-FP2 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3220000,	/* FP1-FP2 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3e00000,	/* PSRAM */
		.length = (32 * 1024)
	},
	{
		.start = 0x61000000,	/* GSRAM */
		.length = (2 * 1024 * 1024)
	}
};

/**
 * Data populated by ROM that can be used with local static initialization.
 */
static struct manticore_rom_shared_sram *const rom_shared_static =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * Misc SW registers that can be used with local static initialization.
 */
static struct Creg_regs_misc_creg_sw_regs *const sw_regs_static =
	(struct Creg_regs_misc_creg_sw_regs*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_ADDRESS;

/**
 * Stack guard to check for overflows.
 */
extern uint32_t __stack_chk_guard;

/**
 * Cache for the SOCID that does not require word access.
 */
static uint32_t ueid[IN_DWORDS (HSP_FUSES_LENGTH (SOCID))];

/**
 * Version string for the RMA firmware image.
 */
static char version_1sp[CERBERUS_PROTOCOL_FW_VERSION_LEN];


/**
 * No code path integrity enabled for this image.
 */
CODE_PATH_INTEGRITY_NONE;


/**
 * Generate the 1SP version string based on the build version in the ROM log.
 */
static void initialize_1sp_version_str ()
{
	/* 1SP is secure if there is an owner key.  Tenant boots are considered secure. */
	bool secure_boot = has_owner_key ();
	const uint32_t *socid = (const uint32_t*) HSP_ADDR_MAP_GFC_SOCID_ADDRESS;

	build_version_to_string (rom_shared->firmware.pcr_log[1].fw_version.data.build_version,
		secure_boot, false, version_1sp, sizeof (version_1sp));

	/* Cache the SOCID in a byte-addressable buffer to allow general purpose use of the data by
	 * firmware. */
	memcpy (ueid, socid, sizeof (ueid));
}


/**
 * Context for the fuse controller driver.
 */
static struct fuse_controller_state fuse_context;

/**
 * Driver for the HSP fuse controller.
 */
static const struct fuse_controller fuses =
	fuse_controller_sw0_rng_calibration_static_init (&fuse_context,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS);

/**
 * Context for the RNG driver.
 */
static struct hsp_rng_hw_state rng_context;

/**
 * Driver for the HSP random number generator.
 */
static const struct hsp_rng_hw rng_hw = hsp_rng_hw_static_init (&rng_context,
	(struct Rng_regs*) HSP_ADDR_MAP_RNG_ADDRESS, &fuses.base, MANTICORE_ROM_MIN_RNG_CLOCK_DIVIDER,
	MANTICORE_ROM_MAX_RNG_CLOCK_DIVIDER);

/**
 * Variable context for the HS-SHA driver.
 */
static struct hs_sha_state hash_hw_context;

/**
 * Driver for the HS-SHA.
 */
static const struct hs_sha hash_hw = hs_sha_static_init_polling (&hash_hw_context,
	(struct Sha_regs*) HSP_ADDR_MAP_SHA_ADDRESS, &rom_shared_static->internal.hs_sha.cmd,
	rom_shared_static->internal.hs_sha.data, MANTICORE_ROM_HS_SHA_BUFFER_SIZE);

/**
 * Variable context for the hash API.
 */
static struct hash_engine_hs_sha_state hash_context;

/**
 * Hash engine wrapper for the HS-SHA driver.
 */
static const struct hash_engine_hs_sha hash = hash_hs_sha_static_init (&hash_context, &hash_hw);

/**
 * Variable context for the PKA driver.
 */
static struct ecc_hw_pka_state pka_context;

/**
 * Driver for the PKA.
 */
static const struct ecc_hw_pka pka = ecc_hw_pka_static_init_polling (&pka_context,
	(struct Pka_regs*) HSP_ADDR_MAP_PKA_ADDRESS, &rng_hw, &rom_shared_static->internal.pka);

/**
 * ECC engine wrapper or the PKA driver.
 */
static const struct ecc_engine_ecc_hw ecc = ecc_ecc_hw_static_init (&pka.base, NULL);

/**
 * Variable context for the AES driver.
 */
static struct hsp_aes_state aes_context;

/**
 * Driver for the HW AES engine.
 */
static const struct hsp_aes aes = hsp_aes_static_init_polling (&aes_context,
	(struct Aes_regs*) HSP_ADDR_MAP_AES_ADDRESS, &rom_shared_static->internal.aes.cmd,
	rom_shared_static->internal.aes.data, MANTICORE_ROM_HS_SHA_BUFFER_SIZE,
	(struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS, CCS_KSU_STATIC_NUM_KEYS);

/**
 * Variable context for the CCS driver.
 */
static struct ccs_ksu_state ccs_context;

/**
 * Driver for the CCS and KSU.
 */
static const struct ccs_ksu ccs = ccs_ksu_static_init_polling (&ccs_context,
	(struct Ccs_regs*) HSP_ADDR_MAP_CCS_ADDRESS, &hash_hw, &aes, &pka, &rng_hw,
	&rom_shared_static->internal.ccs.cmd, (struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS,
	CCS_KSU_STATIC_NUM_KEYS, (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS,
	CCS_KSU_STATIC_NUM_PCRS);


/**
 * Initialize the HW random number generator.
 *
 * @return 0 if the RNG was successfully initialized or an error code.
 */
static int initialize_rng ()
{
	int status;

	status = fuse_controller_init_state (&fuses);
	if (status != 0) {
		return status;
	}

	/* No need to calibrate since that would have already been done by ROM. */
	status = hsp_rng_hw_init_state (&rng_hw, false);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the HW crypto engines.
 *
 * @return 0 if the crypto engines were successfully initialized or an error code.
 */
static int initialize_crypto ()
{
	int status;

	status = ccs_ksu_init_state (&ccs);
	if (status != 0) {
		return status;
	}

	status = hs_sha_init_state (&hash_hw);
	if (status != 0) {
		return status;
	}

	status = hash_hs_sha_init_state (&hash);
	if (status != 0) {
		return status;
	}

	status = ecc_hw_pka_init_state (&pka);
	if (status != 0) {
		return status;
	}

	status = hsp_aes_init_state (&aes);
	if (status != 0) {
		return status;
	}

	return 0;
}


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
	&hash.base, MAX_FIPS_DEVID_CERT_LENGTH);

/**
 * Buffer to use for building DICE certificate extensions.  A single buffer can be used for all
 * extensions.
 */
static uint8_t ext_buffer[512];

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
static const struct tcg_dice_tcbinfo layer0_tcb = {
	.version = version_1sp,
	.layer = 0,
	.svn = layer0_svn,
	.svn_length = sizeof (layer0_svn),
	.fwid_list = layer0_fwid_list,
	.fwid_count = ARRAY_SIZE (layer0_fwid_list)
};

/**
 * Handler for the layer 0 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer0_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer0_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Information about the TCB for DICE layer 1.
 */
static struct tcg_dice_tcbinfo layer1_tcb;

/**
 * Handler for the layer 1 TcbInfo extension.  This is unused, but needed to satisfy the DICE
 * handler.
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
 * Current value of the DME renewal counter.
 */
static uint32_t dme_renewal_counter;

/**
 * Information about DME for this device and boot context.
 */
static struct dme_structure_raw_ecc dme;

/**
 * Handler for the DME extension for the layer 0 certificate.
 */
static const struct x509_extension_builder_dme layer0_dme_ext =
	x509_extension_builder_dme_static_init_with_buffer (&dme.base, ext_buffer, sizeof (ext_buffer));

/**
 * List of extensions to add to the DICE layer 0 certificate and CSR.
 */
static const struct x509_extension_builder *layer0_ext[] = {
	&layer0_tcb_ext.base, &layer0_ueid_ext.base, &layer0_dme_ext.base
};

/**
 * List of extensions to add to the DICE layer 1 certificate.
 */
static const struct x509_extension_builder *layer1_ext[] = {&layer1_tcb_ext.base};

/**
 * Variable context for DICE layer 0 processing.
 */
static struct riot_core_hsp_state dice_context;

/**
 * DICE layer 0 handler.
 */
static const struct riot_core_hsp dice = riot_core_hsp_static_init (&dice_context, &ccs.base,
	&base64.base, &x509.base, DEVICE_KEYS_DICE_CDI, MANTICORE_DEVICE_KEYS_NON_FIPS_DEVICE_ID_KEY,
	MANTICORE_DEVICE_KEYS_SP_ALIAS_KEY, layer0_ext, ARRAY_SIZE (layer0_ext), 0, layer1_ext,
	ARRAY_SIZE (layer1_ext));

/**
 * Handler for renewal of the device identity.
 */
static const struct identity_renewal identity = identity_renewal_static_init (&fuses.base);

/**
 * Buffer to hold the DICE CSR for the RMA firmware.
 */
static uint8_t dice_csr[MAX_FIPS_DEVID_CSR_LENGTH];

/**
 * Length of the DICE CSR.
 */
static size_t dice_csr_length;


/**
 * Create the DICE CSR for the device
 *
 * @return 0 if the DICE CSR was created successfully or an error code.
 */
static int create_dice_csr ()
{
	struct ksu_pcr_slot *pcr = (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS;
	uint8_t *der;
	size_t length;
	int status;

	/* Configure the TCB structure for layer 0. */
	buffer_reverse_copy (layer0_svn, (uint8_t*) &rom_shared->firmware.pcr_log[1].fw_svn.data.svn,
		sizeof (layer0_svn));
	memcpy (layer0_fwid, (uint32_t*) pcr[1].pcr, SHA384_HASH_LENGTH);

	/* Configure the DME structure for layer 0. */
	status = dme_structure_raw_ecc_init_sha384 (&dme,
		(uint8_t*) &rom_shared->firmware.dme_structure,
		sizeof (rom_shared->firmware.dme_structure.signed_data),
		rom_shared->firmware.dme_key.Parts.X.AsBytes, rom_shared->firmware.dme_key.Parts.Y.AsBytes,
		ECC_KEY_LENGTH_384, rom_shared->firmware.dme_structure.signature.Parts.R.AsBytes,
		rom_shared->firmware.dme_structure.signature.Parts.S.AsBytes, HASH_TYPE_SHA384);
	if (status != 0) {
		return status;
	}

	status = identity.get_dme_renewal (&identity, &dme_renewal_counter);
	if (status != 0) {
		return status;
	}

	dme.base.device_oid = DICE_OID_MANTICORE;
	dme.base.dev_oid_length = DICE_OID_MANTICORE_LENGTH;
	dme.base.renewal_counter = (uint8_t*) &dme_renewal_counter;
	dme.base.counter_length = sizeof (dme_renewal_counter);

	status = riot_core_hsp_init_state (&dice);
	if (status != 0) {
		return status;
	}

	/* CDI is not relevant here. */
	status = dice.base.generate_device_id (&dice.base, NULL, 0);
	if (status != 0) {
		goto exit;
	}

	status = dice.base.get_device_id_csr (&dice.base, DICE_OID_MANTICORE, DICE_OID_MANTICORE_LENGTH,
		&der, &length);
	if (status != 0) {
		goto exit;
	}

	if (length > sizeof (dice_csr)) {
		/* The generated CSR is too large for the buffer.  This should never happen. */
		status = -1;
		goto exit;
	}

	memcpy (dice_csr, der, length);
	dice_csr_length = length;
	platform_free (der);

exit:
	/* Only need the CSR.  Wipe the DICE keys. */
	riot_core_hsp_release (&dice);

	return status;
}


/**
 * Variable context for the unlock authorization token.
 */
static struct signature_verification_ecc_state unlock_auth_verify_context;

/**
 * Verification handler for the unlock authorization token.
 */
static const struct signature_verification_ecc unlock_auth_verify =
	signature_verification_ecc_static_init (&unlock_auth_verify_context, &ecc.base);

/**
 * The interface for device operations.
 */
const struct cmd_device_hsp device_cmd = cmd_device_hsp_static_init (&reset_count,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS, sw_regs_static);

/**
 * Authenticator for the authorized RMA unlock token.
 */
static const struct rma_unlock_token rma_token = rma_unlock_token_static_init (RMA_AUTH_PUBLIC_KEY,
	RMA_AUTH_PUBLIC_KEY_LEN, &unlock_auth_verify.base, &hash.base, HASH_TYPE_SHA384,
	&device_cmd.base, DICE_OID_MANTICORE, MANTICORE_OID_LENGTH,
	rom_shared_static->firmware.dme_structure.signed_data.device_id_hash.AsBytes,
	SHA384_HASH_LENGTH);

/**
 * DMB segment descriptors for the system.
 */
static struct hsp_dmb_segment dmb_segments[HSP_DMB_SEGMENTS];

/**
 * Variable context for the DMB driver.
 */
static struct hsp_dmb_state dmb_context;

/**
 * Driver for the HSP DMB.
 */
static const struct hsp_dmb dmb = hsp_dmb_static_init (&dmb_context, dmb_segments, HSP_DMB_SEGMENTS,
	HSP_DMB_BASE_MAPPING_ADDRESS, (struct Dmb_reg*) HSP_ADDR_MAP_DMB_ADDRESS);

/**
 * Handler to transition the device to RETEST state.
 */
static const struct device_rma_transition_hsp_retest rma_retest =
	device_rma_transition_hsp_retest_static_init_erase_sram (&fuses.base, &ccs.base, &dmb, soc_sram,
	ARRAY_SIZE (soc_sram));

/**
 * Handler for executing RMA requests.
 */
static struct secure_device_unlock_rma rma_unlock;


/**
 * Initialize the handler for executing the RETEST transition.
 *
 * @return 0 if the RMA handler was initialized successfully or an error code.
 */
static int initialize_rma_handler ()
{
	int status;

	status = signature_verification_ecc_init_state (&unlock_auth_verify, NULL, 0);
	if (status != 0) {
		return status;
	}

	status = hsp_dmb_init_state (&dmb);
	if (status != 0) {
		return status;
	}

	return secure_device_unlock_rma_init (&rma_unlock, &rma_token, &rma_retest.base, dice_csr,
		dice_csr_length);
}


/**
 * Manager for details about known devices.
 *
 * TODO:  Create a static initializer for this type.
 */
static struct device_manager device_manager;

/**
 * Buffers to receive packets over system I2C channel.
 */
static struct cmd_packet system_rx_buffers[2];

/**
 * Forward declaration for system_i2c.
 */
static const struct i2c_dw_apb_multimaster i2c_hw;

/**
 * Variable context for the I2C command channel.
 */
static struct cmd_channel_i2c_dw_apb_multimaster_state system_i2c_context;

/**
 * The I2C interface to the BMC.
 */
static const struct cmd_channel_i2c_dw_apb_multimaster system_i2c =
	cmd_channel_i2c_dw_apb_multimaster_static_init (&system_i2c_context, &i2c_hw, 0, 0);

/**
 * Variable context for the I2C slave driver.
 */
static struct i2c_dw_apb_multimaster_state i2c_hw_context;

/**
 * I2C slave driver for managing the hardware interface.
 */
static const struct i2c_dw_apb_multimaster i2c_hw =
	i2c_dw_apb_multimaster_static_init (&i2c_hw_context,
	(struct Creg_regs_DW_apb_i2c_APB_Slave*) HSP_ADDR_MAP_CREG_I2C0_ADDRESS,
	&system_i2c.i2c_handler);

/**
 * The command handler for Cerberus requests and error message generation.
 */
static const struct cmd_interface_rma cerberus_handler =
	cmd_interface_rma_static_init (&device_manager, CERBERUS_PROTOCOL_MSFT_PCI_VID,
	MSFT_DEVICE_ID_MANTICORE, CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_SUBSYSTEM_DEVICE_ID_DC_SCM);

/**
 * Handler for the RoT MSFT command set.
 */
static const struct cmd_interface_msft_rot msft_rot_handler =
	cmd_interface_msft_rot_static_init (&rma_unlock.base, NULL, NULL, NULL, NULL, NULL);

/**
 * Protocol handler for the MCTP vendor defined protocol.
 */
static const struct cmd_interface_protocol_msft msft_protocol =
	cmd_interface_protocol_msft_static_init;

/**
 * List of MSFT handlers for the supported command sets.
 */
static const struct cmd_interface_multi_handler_msg_type msft_command_sets[] = {
	cmd_interface_multi_handler_msg_type_static_init (MSFT_MCTP_PROTOCOL_COMMAND_SET_ROT,
		&msft_rot_handler.base),
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
 * TODO:  Create a static initializer for this type.
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
static const struct mctp_interface mctp_transport_1sp =
	mctp_interface_static_init (&mctp_transport_context, &mctp_handler, &device_manager,
	&system_i2c.base.base);

/**
 * Handler for received system commands.
 */
static const struct cmd_channel_handler system_cmd_handler =
	cmd_channel_handler_static_init (&system_i2c.base.base, &mctp_transport_1sp, NULL);

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
	ARRAY_SIZE (system_cmd_handlers), 0);


/**
 * Initialize the command handler to receive RMA requests.
 *
 * @return 0 if the command handler was initialized successfully or an error code.
 */
static int initialize_cmd_handler ()
{
	uint8_t i_device = 2;
	int status;
	struct pcd_rot_info rot_info = {
		.is_pa_rot = false,
		.port_count = 1,
		.components_count = 0,
		.i2c_slave_addr = DEFAULT_I2C_SLAVE_ADDR,
		.eid = MCTP_BASE_PROTOCOL_MANTICORE_AC_ROT_EID,
		.bridge_i2c_addr = DEFAULT_BMC_SLAVE_ADDRESS,
		.bridge_eid = MCTP_BASE_PROTOCOL_BMC_EID,
	};

	status = device_manager_init (&device_manager, 4, 0, 0, DEVICE_MANAGER_AC_ROT_MODE,
		DEVICE_MANAGER_SLAVE_BUS_ROLE, rot_info.attestation_fail_retry,
		rot_info.attestation_success_retry, rot_info.discovery_fail_retry,
		rot_info.mctp_ctrl_timeout, rot_info.mctp_bridge_additional_timeout,
		rot_info.attestation_rsp_not_ready_max_duration,
		rot_info.attestation_rsp_not_ready_max_retry);
	if (status != 0) {
		return status;
	}

	// Update entry for Manticore
	status = device_manager_update_not_attestable_device_entry (&device_manager,
		DEVICE_MANAGER_SELF_DEVICE_NUM, rot_info.eid, rot_info.i2c_slave_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		return status;
	}

	// Update entry for BMC
	status = device_manager_update_not_attestable_device_entry (&device_manager,
		DEVICE_MANAGER_MCTP_BRIDGE_DEVICE_NUM, rot_info.bridge_eid, rot_info.bridge_i2c_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		return status;
	}

	// Update entry for in-band utility
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		MCTP_BASE_PROTOCOL_IB_EXT_MGMT, rot_info.bridge_i2c_addr, DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		return status;
	}

	++i_device;

	// Update entry for out-of-band utility
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		MCTP_BASE_PROTOCOL_OOB_EXT_MGMT, rot_info.bridge_i2c_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_multimaster_init_hw (&i2c_hw, rot_info.i2c_slave_addr,
		I2C_DW_APB_SPEED_FAST, HSP_CLOCK_FREQUENCY_HZ);
	if (status != 0) {
		return status;
	}

	status = i2c_dw_apb_multimaster_init_state (&i2c_hw);
	if (status != 0) {
		return status;
	}

	status = cmd_channel_i2c_dw_apb_multimaster_init_state (&system_i2c, system_rx_buffers,
		ARRAY_SIZE (system_rx_buffers));
	if (status != 0) {
		return status;
	}

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_INTSTS_I2C_INTSTS_LSB,
		&i2c_hw.i2c_base.isr_handler);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_mctp_control_init (&mctp_control_handler, &device_manager,
		CERBERUS_PROTOCOL_MSFT_PCI_VID, CERBERUS_PROTOCOL_PROTOCOL_VERSION);
	if (status != 0) {
		return status;
	}

	status = mctp_interface_init_state (&mctp_transport_1sp);
	if (status != 0) {
		return status;
	}

	return periodic_task_freertos_init_state (&system_cmd_task);
}

/**
 * Start the command handler task for executing the RMA flows.
 *
 * @return 0 if the RMA handler task successfully started or an error code.
 */
int start_rma_handler ()
{
	int status;

	/* TODO:  Confirm stack requirements on this platform. */
	status = periodic_task_freertos_allocate (&system_cmd_task, 6 * 256, "MCTP_LOOP",
		CERBERUS_PRIORITY_HIGH);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&system_cmd_task);

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_I2C_INTEN_LSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	i2c_dw_apb_enable_slave_mode (&i2c_hw.i2c_base);

	return 0;
}


/**
 * Entry point for Manticore RMA 1SP.
 */
void main ()
{
	uint32_t random_val = 0;
	int status;

	hsp_trap_init (true, 0);
	traps_init_exception_catch ();
	determine_hsp_clock_frequency ();
	HspUartInitializeEx (HSP_CLOCK_FREQUENCY_HZ, 115200);

	initialize_1sp_version_str ();

	platform_printf (NEWLINE);
	platform_printf ("Manticore Boot: %s" NEWLINE,
		(MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) ? "Internal" : "External");
	platform_printf ("RMA: %s" NEWLINE, version_1sp);
	platform_printf ("SOCID: %x.%x.%x.%x" NEWLINE, ueid[0], ueid[1], ueid[2], ueid[3]);
	platform_printf ("Boot Source: 0x%x" NEWLINE, MANTICORE_BOOT_SOURCE);
	platform_printf ("Boot Order: 0x%x" NEWLINE, MANTICORE_BOOT_ORDER);
	platform_printf ("Scratch Reg 0: 0x%x" NEWLINE, MANTICORE_HSP_SCRATCH0_REG);
	platform_printf ("Sticky Reg 1: 0x%x" NEWLINE, MANTICORE_HSP_STICKY1_REG);
	platform_printf ("Scratch Reg 1: 0x%x" NEWLINE, MANTICORE_HSP_SCRATCH1_REG);

	status = initialize_rng ();
	if (status != 0) {
		platform_printf ("RNG FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* Apply a random value to the stack guard.  It can't be done in the context of a function
	 * call */
	hsp_rng_hw_get_random_word (&rng_hw, &random_val);
	__stack_chk_guard = random_val;

	status = initialize_crypto ();
	if (status != 0) {
		platform_printf ("Crypto Init FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = create_dice_csr ();
	if (status != 0) {
		platform_printf ("DICE Init FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = initialize_rma_handler ();
	if (status != 0) {
		platform_printf ("RMA Init FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = initialize_cmd_handler ();
	if (status != 0) {
		platform_printf ("Command Init FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* Initialize and enable interrupts. */
	status = hsp_interrupt_init (true);
	if (status != 0) {
		platform_printf ("IRQ INIT FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = hsp_freertos_init ();
	if (status != 0) {
		platform_printf ("FreeRTOS INIT FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = start_rma_handler ();
	if (status != 0) {
		platform_printf ("Task Start FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	vTaskStartScheduler ();
	platform_printf ("Returned from FreeRTOS scheduler!?" NEWLINE);

error:
	/* Error during initialization.  Just wait for a reset. */
	CEASE;
}
