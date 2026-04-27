// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "device_keys.h"
#include "rom_logging.h"
#include "logging/code_path_integrity.h"


/* Allow each implementation to provide their own context for KDF calls, which allows better control
 * over where in memory these pieces are placed and what values are actually used.  Rather than
 * creating a structure that takes pointers to the buffers, we just access global memory.  This is a
 * deviation from the general architecture, but serves to make ROM a bit leaner.
 *
 * The first set of KDF inputs are ROM secrets that should be placed in the appropriate section of
 * the ROM address space.  They are used to transform the loaded keys before other KDFs with
 * possibly non-secret contexts are executed. */

/**
 * 384-bit context to use as the secret context for the initial KDF on KSU slot 0.
 */
extern const SP_MSG_384 DEVICE_KEYS_FUSE_KEY_0_KDF;

/**
 * 384-bit context to use as the secret context for the initial KDF on KSU slot 1.
 */
extern const SP_MSG_384 DEVICE_KEYS_FUSE_KEY_1_KDF;

/**
 * 384-bit context to use as the secret context for the initial KDF on KSU slot 2.
 */
extern const SP_MSG_384 DEVICE_KEYS_FUSE_KEY_2_KDF;

/**
 * 384-bit context to use as the secret context for the initial KDF on KSU slot 3.
 */
extern const SP_MSG_384 DEVICE_KEYS_FUSE_KEY_3_KDF;

/**
 * SHA384 hash to use as the context for the DICE UDS KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_DICE_UDS_KDF;

/**
 * SHA384 hash to use as the context for the DME UDS KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_DME_UDS_KDF;

/**
 * SHA384 hash to use as the context for the Hash Stick Key KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_HASH_STICK_KEY_KDF;

/**
 * SHA384 hash to use as the context for the Device Key KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_DEVICE_KEY_KDF;

/**
 * SHA384 hash to use as the context for the Tenancy Seed KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_TENANCY_KEY_KDF;

/**
 * SHA384 hash to use as the context for the Firmware AES Encryption Key KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_FIRMWARE_AES_KEY_KDF;

/**
 * SHA384 hash to use as the context for the Global Key KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_GLOBAL_KEY_KDF;

/**
 * SHA384 hash to use as the context for the Global Seed Unwrapping Key KDF.
 */
extern const SP_MSG_384 DEVICE_KEYS_GLOBAL_UNWRAP_KEY_KDF;

/**
 * SHA384 hash to use as the context for the FuseSync Key KDF
 */
extern const SP_MSG_384 DEVICE_KEYS_FUSE_SYNC_KEY_KDF;


/**
 * Initialize the KSU key slots for ROM use.  This will clear out all key slots, except for the
 * first 4, since those are the ones loaded from fuses by HW.
 *
 * In the Production or Secure state, the fuse-backed key slots will be morphed with ROM secrets
 * before they can be used generally.
 *
 * @param state The current security state of the device.
 * @param ccs CCS and KSU driver managing the ROM keys.
 * @param ksu_slots The number of KSU key slots present in the device.
 *
 * @return 0 if the key slots were successfully initialized or an error code.
 */
int device_keys_initialize_rom_keys (enum hsp_security_state state,
	const struct ccs_ksu_interface *ccs, int ksu_slots)
{
	SP_MSG_384 clear = {0};
	int i;
	int status;

	if (ccs == NULL) {
		return DEVICE_KEYS_INVALID_ARGUMENT;
	}

	switch (state) {
		case HSP_SECURITY_STATE_TEST:
			i = 0;	// Clear all the key slots.
			break;

		case HSP_SECURITY_STATE_PRODUCTION:
		case HSP_SECURITY_STATE_SECURE:
			status = ccs->derive_key (ccs, DEVICE_KEYS_DICE_SEED, &DEVICE_KEYS_FUSE_KEY_0_KDF,
				DEVICE_KEYS_DICE_SEED,
				CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
				CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				return status;
			}

			status = ccs->derive_key (ccs, DEVICE_KEYS_DME_SEED, &DEVICE_KEYS_FUSE_KEY_1_KDF,
				DEVICE_KEYS_DME_SEED, CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
				CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				return status;
			}

			status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_FUSE_KEY_2_KDF,
				DEVICE_KEYS_DEVICE_SEED,
				CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
				CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				return status;
			}

			status = ccs->derive_key (ccs, DEVICE_KEYS_GLOBAL_SEED, &DEVICE_KEYS_FUSE_KEY_3_KDF,
				DEVICE_KEYS_GLOBAL_SEED, CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				return status;
			}

			i = 4;	// Skip clearing the key slots backed by fuses.
			break;

		default:
			/* Nothing needs to be done here.  HSP is not operating in a secure way with OBS and
			 * keys enabled, and FW won't even be loaded.  There are no keys to clear. */
			return 0;
	}

	/* Clear the appropriate key slots. */
	for (; i < ksu_slots; i++) {
		status = ccs->set_key (ccs, &clear, i, 0);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Derive the key necessary for fuse syncing from KSU slot2.
 *
 * @param ccs CCS and KSU driver to use for key derivations.
 *
 * @return 0 if the device keys were successfully generated or an error code.
 */
int device_keys_initialize_rom_keys_extended (enum hsp_security_state state,
	const struct ccs_ksu_interface *ccs)
{
	int status;

	if (ccs == NULL) {
		return DEVICE_KEYS_INVALID_ARGUMENT;
	}

	switch (state) {
		case HSP_SECURITY_STATE_PRODUCTION:
		case HSP_SECURITY_STATE_SECURE:
			status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_FUSE_SYNC_KEY_KDF,
				DEVICE_KEYS_FUSE_SYNC_KEY, CCS_KSU_ATTR_IS_DEVICE_SECRET |
				CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
			break;

		default:
			status = 0;
	}

	return status;
}

/**
 * Derive the keys necessary for operation in Production state.  This will only generate the
 * unwrapping key needed during DME provisioning.
 *
 * @param ccs CCS and KSU driver to use for key derivations and storage.
 *
 * @return 0 if the device keys were successfully generated or an error code.
 */
static int device_keys_derive_production_keys (const struct ccs_ksu_interface *ccs)
{
	SP_MSG_384 clear = {0};
	int i;
	int status;

	status = ccs->derive_key (ccs, DEVICE_KEYS_GLOBAL_SEED, &DEVICE_KEYS_GLOBAL_UNWRAP_KEY_KDF,
		DEVICE_KEYS_GLOBAL_UNWRAP_KEY,
		CCS_KSU_ATTR_KEY_ALLOWED_AS_DEC_KEK | CCS_KSU_ATTR_KEY_ALLOWED_AS_ENC_KEK);
	if (status != 0) {
		return status;
	}

	for (i = 0; i < 3; i++) {
		status = ccs->set_key (ccs, &clear, i, 0);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Derive the keys necessary for operation in Secure state.  This will generate the following keys
 * and seeds:
 * - DICE UDS
 * - DME Key
 * - Base Hash Stick Key
 * - Tenancy Transfer Seed
 * - Firmware AES Encryption Key
 * - Global DME Signing Key (generated and used, but erased before returning)
 *
 * @param ccs CCS and KSU driver to use for key derivations and storage.
 * @param ecc ECC hardware driver to use for verifying derived ECC keys.
 * @param hash Hash engine to use for verifying derived ECC keys.
 * @param identity Handler for device identity renewal.
 * @param socid Buffer containing the device SOCID.  This is used as part of DME signing.
 * @param socid_words Length of the SOCID in 32-bit words.
 * @param dme_pcr PCR to use for DME signing.  This PCR will be reinitialized prior to signing the
 * DME public key.
 * @param dme Output buffer for the signed DME public key.
 * @param dme_signer Optional output buffer for the public key used to sign the DME key.  If this is
 * null, the public key for the DME signer will not be generated and will not be retrievable at a
 * later time since the key is erased.
 *
 * @return 0 if the device keys were successfully generated or an error code.
 */
static int device_keys_derive_secure_keys (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash,
	const struct identity_renewal *identity, const uint32_t *socid, size_t socid_words,
	uint8_t dme_pcr, struct device_keys_dme_public_key *dme, SP_ECDSA_P384_PUBLIC *dme_signer)
{
	SP_MSG_384 context = {0};
	int status;
	size_t i;

	if ((ecc == NULL) || (hash == NULL) || (identity == NULL) || (socid == NULL) || (dme == NULL) ||
		(socid_words != DEVICE_KEYS_SOCID_WORDS)) {
		return DEVICE_KEYS_INVALID_ARGUMENT;
	}

	/* Derive the UDS that will be used to generate the DICE CDI.  Create the UDS by running two
	 * KDFs.  First with the implementation-defined context.  Second with the device revocation
	 * state. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DICE_SEED, &DEVICE_KEYS_DICE_UDS_KDF,
		DEVICE_KEYS_DICE_UDS,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = identity->get_dice_renewal (identity, &context.AsUINT32s[0]);
	if (status != 0) {
		return status;
	}

	status = ccs->derive_key (ccs, DEVICE_KEYS_DICE_UDS, &context, DEVICE_KEYS_DICE_UDS,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_PCR_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	/* Derive the DME key that will be used to endorse the DICE identity.  Create DME UDS by running
	 * two KDFs.  First with the implementation-defined context.  Second with the device revocation
	 * state.  From the UDS, derive the DME ECC private key.  Once we have the DME key, clear out
	 * the UDS. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DME_SEED, &DEVICE_KEYS_DME_UDS_KDF,
		DEVICE_KEYS_DME_UDS,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = identity->get_dme_renewal (identity, &dme->renew_counter);
	if (status != 0) {
		return status;
	}

	context.AsUINT32s[0] = dme->renew_counter;
	status = ccs->derive_key (ccs, DEVICE_KEYS_DME_UDS, &context, DEVICE_KEYS_DME_UDS,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_interface_derive_ecdsa_key (ccs, ecc, hash, DEVICE_KEYS_DME_UDS,
		DEVICE_KEYS_DME_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384,
		NULL);
	if (status != 0) {
		return status;
	}

	context.AsUINT32s[0] = 0;
	status = ccs->set_key (ccs, &context, DEVICE_KEYS_DME_UDS, 0);
	if (status != 0) {
		return status;
	}

	/* Derive the Hash Stick key.  This seed will be run through additional KDFs later depending on
	 * the device state to generate the final key. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_HASH_STICK_KEY_KDF,
		DEVICE_KEYS_HASH_STICK_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	/* Derive the constant Device key.  This key is independent of firmware or security
	 * configuration. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_DEVICE_KEY_KDF,
		DEVICE_KEYS_DEVICE_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	/* Derive the Tenancy key to support Tenancy Transfer workflows. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_TENANCY_KEY_KDF,
		DEVICE_KEYS_TENANCY_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED |
		CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	/* Derive the AES encryption key that will be used to load and store encrypted firmware images
	 * in flash. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_DEVICE_SEED, &DEVICE_KEYS_FIRMWARE_AES_KEY_KDF,
		DEVICE_KEYS_FW_IMAGE_AES_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED |
		CCS_KSU_ATTR_AES_DECRYPT_ALLOWED);
	if (status != 0) {
		return status;
	}

	/* Derive the DME signing key use to prove authenticity of DME public keys.  The DME signing key
	 * is derived from a global key.  This key is also used to generate a global key bound to the
	 * current device owner. */
	status = ccs->derive_key (ccs, DEVICE_KEYS_GLOBAL_SEED, &DEVICE_KEYS_GLOBAL_KEY_KDF,
		DEVICE_KEYS_GLOBAL_KEY, CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_interface_derive_ecdsa_key (ccs, ecc, hash, DEVICE_KEYS_GLOBAL_KEY,
		DEVICE_KEYS_DME_SIGNING_KEY,
		CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_MUST_APPEND_PCR | CCS_KSU_ATTR_KEY_SIZE_384,
		(union ccs_ksu_ecc_public_key*) dme_signer);
	if (status != 0) {
		return status;
	}

	/* Get the DME public key and sign it with the DME signing key.  In addition to the DME public
	 * key, the signature will include the SOCID and DME revocation information.  The DME public
	 * key endorsement must include a PCR value, and we use a clean PCR to show that now firmware
	 * has been executed when this signature was generated.  Once the DME public key is signed, the
	 * signing key is destroyed. */
	memset (&context, 0, sizeof (context));

	/* The SOCID memory location is typically only word-accessible, so loop through and copy words
	 * instead of assuming a memcpy call will provide this optimization. */
	for (i = 0; i < socid_words; i++) {
		context.AsUINT32s[i] = socid[i];
		dme->socid[i] = socid[i];
	}
	context.AsUINT32s[socid_words] = dme->renew_counter;

	status = ccs->reset_pcr (ccs, dme_pcr);
	if (status != 0) {
		return status;
	}

	status = ccs->certify_ecc_public_key (ccs, DEVICE_KEYS_DME_SIGNING_KEY, dme_pcr,
		DEVICE_KEYS_DME_KEY, &context, &dme->key, &dme->signature, NULL, NULL);
	if (status != 0) {
		return status;
	}

	memset (&context, 0, sizeof (context));

	return ccs->set_key (ccs, &context, DEVICE_KEYS_DME_SIGNING_KEY, 0);
}

/**
 * Derive all keys for a device that do not depend on firmware state.  Different keys will be
 * generated based on the current security state.  If the security state is not Production or
 * Secure, no keys will be generated.
 *
 * In general, the keys that could be generated include:
 * - DICE UDS
 * - DME key
 * - Device-unique encryption keys and seeds
 * - Global DME signing key
 * - Global unwrapping key
 *
 * As part of DME derivation, the DME public key will be generated and signed.
 *
 * @param state The current security state of the device.
 * @param ccs CCS and KSU driver to use for key derivations and storage.
 * @param ecc ECC hardware driver to use for verifying derived ECC keys.
 * @param hash Hash engine to use for verifying derived ECC keys.
 * @param identity Handler for device identity renewal.
 * @param socid Buffer containing the device SOCID.  This is used as part of DME signing.
 * @param socid_words Length of the SOCID in 32-bit words.
 * @param dme_pcr PCR to use for DME signing.  This PCR will be reinitialized prior to signing the
 * DME public key.
 * @param dme Output buffer for the signed DME public key.
 * @param dme_signer Optional output buffer for the public key used to sign the DME key.  If this is
 * null, the public key for the DME signer will not be generated and will not be retrievable at a
 * later time since the key is erased.
 *
 * @return 0 if the device keys were successfully generated or an error code.  The case where no
 * keys are generated due to the security state of the device is considered a successful case.
 */
int device_keys_derive_keys (enum hsp_security_state state, const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash,
	const struct identity_renewal *identity, const uint32_t *socid, size_t socid_words,
	uint8_t dme_pcr, struct device_keys_dme_public_key *dme, SP_ECDSA_P384_PUBLIC *dme_signer)
{
	int status;

	if (ccs == NULL) {
		return DEVICE_KEYS_INVALID_ARGUMENT;
	}

	switch (state) {
		case HSP_SECURITY_STATE_PRODUCTION:
			status = device_keys_derive_production_keys (ccs);
			if (status != 0) {
				return status;
			}

		/* fall through */ /* no break */

		case HSP_SECURITY_STATE_TEST:
			/* The PCR will not be used for DME endorsement, but it still needs to be cleared to
			 * ensure a proper measurement is generated. */
			return ccs->reset_pcr (ccs, dme_pcr);

		case HSP_SECURITY_STATE_SECURE:
			return device_keys_derive_secure_keys (ccs, ecc, hash, identity, socid, socid_words,
				dme_pcr, dme, dme_signer);

		default:
			return 0;
	}
}

/**
 * Derive device identity keys and other keys based on the loaded firmware when in the Secure state.
 *
 * @param ccs CCS and KSU driver to use for key derivation and storage.
 * @param ecc ECC hardware driver to use for verifying derived ECC keys.
 * @param rot Interface to the HW RoT state.
 * @param hash Hash engine to use for populating the DME structure for DICE endorsement.
 * @param dice_pcr PCR to use for DICE device ID generation.
 * @param measurements A list of SHA2-384 digests that will be used to generate the measurement
 * reported in the DME structure endorsing the device ID public key.
 * @param measurement_count The number of digests to include in the DME measurement.
 * @param device_id Output for the DICE device ID public key.
 * @param dme Output for the DME structure endorsing the DICE device ID public key.
 *
 * @return 0 if the device keys were successfully generated or an error code.
 */
static int device_keys_secure_firmware_keys (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hw_rot *rot, const struct hash_engine *hash,
	uint8_t dice_pcr, const SP_MSG_384 *const *measurements, size_t measurement_count,
	SP_ECDSA_P384_PUBLIC *device_id, struct device_keys_dice_endorsement *dme)
{
	SP_MSG_384 context;
	uint64_t svn;
	uint64_t svn_msb = 1;
	int svn_bits;
	uint8_t i;
	int status;
	uint8_t fail_id = ROM_LOGGING_FAIL_DICE;

	code_path_integrity_secure_message_trace (ROM_LOGGING_TRACE_DICE);

	/* Derive the DICE Device ID key.  This is done in two steps.  First, is to KDF the UDS using
	 * the current PCR state.  Second, is to use the CDI to generate an ECC key pair. */
	status = ccs->derive_key_using_pcr (ccs, DEVICE_KEYS_DICE_UDS, dice_pcr, DEVICE_KEYS_DICE_CDI,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		goto exit;
	}

	status = ccs_ksu_interface_derive_ecdsa_key (ccs, ecc, hash, DEVICE_KEYS_DICE_CDI,
		DEVICE_KEYS_DEVICE_ID_KEY,
		CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384,
		(union ccs_ksu_ecc_public_key*) device_id);
	if (status != 0) {
		goto exit;
	}

	/* Construct the DME structure for the DICE Device ID and sign it with the DME key.  This
	 * structure endorses the DICE key as one that can be trusted. */
	fail_id = ROM_LOGGING_FAIL_DME;
	code_path_integrity_secure_message_no_trace (ROM_LOGGING_FAIL_DME);

	status = hash->calculate_sha384 (hash, device_id->AsBytes, SP_ECDSA_P384_PUBLIC_KEY_SIZE,
		dme->signed_data.device_id_hash.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto exit;
	}

	status = hash->start_sha384 (hash);
	if (status != 0) {
		goto exit;
	}

	for (i = 0; i < measurement_count; i++) {
		status = hash->update (hash, measurements[i]->AsBytes, SP_MSG_384_SIZE);
		if (status != 0) {
			goto hash_error;
		}
	}

	status = hash->finish (hash, dme->signed_data.measurement.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		goto hash_error;
	}

	status = ccs->ecdsa_sign_message (ccs, DEVICE_KEYS_DME_KEY, (uint8_t*) dme,
		sizeof (dme->signed_data), hash, HASH_TYPE_SHA384, &dme->signature, NULL);
	if (status != 0) {
		goto exit;
	}

	/* We are done with the DME Key, so clear it. */
	memset (context.AsBytes, 0, SP_MSG_384_SIZE);
	status = ccs->set_key (ccs, &context, DEVICE_KEYS_DME_KEY, 0);
	if (status != 0) {
		goto exit;
	}

	fail_id = ROM_LOGGING_FAIL_DEVICE_KEYS;
	code_path_integrity_secure_message_no_trace (ROM_LOGGING_FAIL_DEVICE_KEYS);

	/* Determine if the device currently has a device owner key that was used to securely load the
	 * firmware.  If it does not, wipe all device-unique keys.  It is fine to leave the DICE keys
	 * since those are derived based on the FW state along with the HW state.  Those keys will
	 * change once a root key is programmed, the other keys will not. */
	status = rot->get_root_key_hash (rot, hash, HASH_TYPE_SHA384, context.AsBytes, SP_MSG_384_SIZE);
	if (status == 0) {
		code_path_integrity_message_trace (ROM_LOGGING_TRACE_DERIVE_FW_KEYS);

		/* Derive a Global Key for the current owner. */
		status = ccs->derive_key (ccs, DEVICE_KEYS_GLOBAL_KEY, &context,
			DEVICE_KEYS_OWNER_GLOBAL_KEY, CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
		if (status != 0) {
			goto exit;
		}

		/* Derive the Hash Stick Key that is valid for the current security configuration. */
		svn_bits = rot->get_svn_bits (rot);
		if (svn_bits > 0) {
			svn_msb = 1ULL << (svn_bits - 1);
		}

		status = rot->get_svn (rot, &svn);
		if (status != 0) {
			goto exit;
		}

		i = 0;
		memset (context.AsBytes, 0, SP_MSG_384_SIZE);
		while (!(svn_msb & svn) && (i++ < svn_bits)) {
			status = ccs->derive_key (ccs, DEVICE_KEYS_HASH_STICK_KEY, &context,
				DEVICE_KEYS_HASH_STICK_KEY,
				CCS_KSU_ATTR_IS_DEVICE_SECRET | CCS_KSU_ATTR_KDF_KEY_ALLOWED |
				CCS_KSU_ATTR_KEY_SIZE_384);
			if (status != 0) {
				goto exit;
			}

			svn <<= 1;
		}
	}
	else if (status == HW_ROT_NO_ROOT_KEY) {
		code_path_integrity_message_trace (ROM_LOGGING_TRACE_CLEAR_FW_KEYS);

		memset (context.AsBytes, 0, SP_MSG_384_SIZE);

		/* Clear the block of device-unique keys and the global key. */
		for (i = DEVICE_KEYS_FW_IMAGE_AES_KEY; i <= DEVICE_KEYS_TENANCY_KEY; i++) {
			status = ccs->set_key (ccs, &context, i, 0);
			if (status != 0) {
				goto exit;
			}
		}
	}

exit:
	/* Require a successful operation for checkpoints to pass.  Depending on where the process
	 * failed, sensitive keys that FW should not have access to could remain. */
	code_path_integrity_secure_message_no_trace (status);

	if (status != 0) {
		rom_logging_error (fail_id, status);
	}

	return status;

hash_error:
	hash->cancel (hash);
	rom_logging_error (fail_id, status);

	return status;
}

/**
 * Derive all keys for a device that depend on firmware and security configuration.  Different keys
 * will be generated based on the current security state.  If the security state is not Secure, all
 * keys will be cleared.
 *
 * In Secure state, the keys that could be generated include:
 * - DICE Device Identity (based on PCR measurement)
 * - Device-unique Hash Stick Key (based on SVN fuses)
 * - Global Owner Key (based on HW root key)
 *
 * The DME key will always be cleared by this call.  If there currently no root key in the device,
 * no Global Owner Key will be generated, and that key slot will also be cleared.
 *
 * @param state The current security state of the device.
 * @param ccs CCS and KSU driver to use for key derivation and storage.
 * @param ecc ECC hardware driver to use for verifying derived ECC keys.
 * @param rot Interface to the HW RoT state.
 * @param hash Hash engine to use for populating the DME structure for DICE endorsement.
 * @param dice_pcr PCR to use for DICE device ID generation.
 * @param measurements A list of SHA2-384 digests that will be used to generate the measurement
 * reported in the DME structure endorsing the device ID public key.  To avoid duplicate copies of
 * measurements, this is a list of pointers to the actual digest value.
 * @param measurement_count The number of digests to include in the DME measurement.
 * @param device_id Output for the DICE device ID public key.
 * @param dme Output for the DME structure endorsing the DICE device ID public key.
 *
 * @return 0 if the device keys were successfully generated or an error code.  The case where no
 * keys are generated due to the security state of the device is considered a successful case.
 */
int device_keys_dice (enum hsp_security_state state, const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hw_rot *rot, const struct hash_engine *hash,
	uint8_t dice_pcr, const SP_MSG_384 *const *measurements, size_t measurement_count,
	SP_ECDSA_P384_PUBLIC *device_id, struct device_keys_dice_endorsement *dme)
{
	SP_MSG_384 clear = {0};
	int status;

	if ((ccs == NULL) || (ecc == NULL) || (rot == NULL) || (hash == NULL) ||
		(measurements == NULL) || (measurement_count == 0) || (device_id == NULL) ||
		(dme == NULL)) {
		return DEVICE_KEYS_INVALID_ARGUMENT;
	}

	switch (state) {
		case HSP_SECURITY_STATE_SECURE:
			status = device_keys_secure_firmware_keys (ccs, ecc, rot, hash, dice_pcr, measurements,
				measurement_count, device_id, dme);
			break;

		case HSP_SECURITY_STATE_PRODUCTION:
			/* By this point, the only key that is left is the Global Unwrapping Key.  All other key
			 * slots have been cleared in previous steps. */
			status = ccs->set_key (ccs, &clear, DEVICE_KEYS_GLOBAL_UNWRAP_KEY, 0);
			break;

		default:
			/* There is nothing necessary here.  No keys will be generated and all key slots have
			 * already been cleared during initialization. */
			status = 0;
			break;
	}

	return status;
}
