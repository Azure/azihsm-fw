// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "ccs_ksu_interface.h"
#include "common/buffer_util.h"
#include "crypto/ecdsa.h"
#include "crypto/kat/ecc_kat_vectors.h"


#ifdef CCS_KSU_ENABLE_FIPS_CMVP_TESTING
/**
 * Global flag that will be used to trigger a PCT failure for a single ECDSA key pair generated
 * using CCS.  Triggering this failure is necessary to support FIPS CMVP testing for certification.
 */
bool ccs_ksu_interface_fail_ecdsa_pct;
#endif


/**
 * Run a pairwise consistency test (PCT) against an ECDSA key pair whose private key is managed by
 * CCS.  If the PCT fails, the private key will be erased.
 *
 * @param ccs The CCS containing the ECDSA private key.
 * @param ecc The ECC hardware to use for signature verification.
 * @param hash The hash engine to use for signature verification.
 * @param key_slot Slot number for the ECDSA private key.
 * @param key_attributes Attributes assigned to the ECDSA key.
 * @param public_key Optional output for the public key of the ECDSA key pair.
 *
 * @return 0 if the key pair was validated successfully or an error code.
 */
static int ccs_ksu_interface_validate_ecdsa_key_pair (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_slot,
	uint32_t key_attributes, union ccs_ksu_ecc_public_key *public_key)
{
	union ccs_ksu_ecc_public_key pub_key = {0};
	union ccs_ksu_ecc_signature signature = {0};
	struct ecc_point_public_key ecc_pub_key = {0};
	struct ecc_ecdsa_signature ecdsa = {0};
	enum hash_type hash_type;
	int status;

	if (public_key == NULL) {
		/* If public key output is not provided, use internal storage for the key. */
		public_key = &pub_key;
	}

	status = ccs->get_ecc_public_key (ccs, key_slot, &public_key->p384, NULL);
	if (status != 0) {
		goto exit;
	}

	if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		hash_type = HASH_TYPE_SHA384;
	}
	else {
		hash_type = HASH_TYPE_SHA256;
	}

	status = ccs->ecdsa_sign_message (ccs, key_slot, ECC_KAT_VECTORS_ECDSA_SIGNED,
		ECC_KAT_VECTORS_ECDSA_SIGNED_LEN, hash, hash_type, &signature.p384, NULL);
	if (status != 0) {
		goto exit;
	}

	/* TODO:  This duplicates some functionality that is present in hsp_fw_util, but that can't be
	 * used directly here since it doesn't support 256-bit keys.  Public key and signature
	 * conversion should be refactored to a common location (perhaps as a utility function of
	 * ecc_hw_pka) to convert from PKA/CCS format to ecc_hw format. */
	if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		memcpy (ecc_pub_key.x, public_key->p384.Parts.X.AsBytes, SP_MSG_384_SIZE);
		memcpy (ecc_pub_key.y, public_key->p384.Parts.Y.AsBytes, SP_MSG_384_SIZE);
		ecc_pub_key.key_length = SP_MSG_384_SIZE;

		memcpy (ecdsa.r, signature.p384.Parts.R.AsBytes, SP_MSG_384_SIZE);
		memcpy (ecdsa.s, signature.p384.Parts.S.AsBytes, SP_MSG_384_SIZE);
		ecdsa.length = SP_MSG_384_SIZE;
	}
	else {
		memcpy (ecc_pub_key.x, public_key->p256.Parts.X.AsBytes, SP_MSG_256_SIZE);
		memcpy (ecc_pub_key.y, public_key->p256.Parts.Y.AsBytes, SP_MSG_256_SIZE);
		ecc_pub_key.key_length = SP_MSG_256_SIZE;

		memcpy (ecdsa.r, signature.p256.Parts.R.AsBytes, SP_MSG_256_SIZE);
		memcpy (ecdsa.s, signature.p256.Parts.S.AsBytes, SP_MSG_256_SIZE);
		ecdsa.length = SP_MSG_256_SIZE;
	}

#ifdef CCS_KSU_ENABLE_FIPS_CMVP_TESTING
	/* Provide the ability to inject errors into the ECDSA PCT to validate negative test scenarios
	 * for FIPS CMVP certification. */
	if (ccs_ksu_interface_fail_ecdsa_pct) {
		if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
			memcpy (&ecc_pub_key, &ECC_KAT_VECTORS_P384_ECC_PUBLIC, sizeof (ecc_pub_key));
		}
		else {
			memcpy (&ecc_pub_key, &ECC_KAT_VECTORS_P256_ECC_PUBLIC, sizeof (ecc_pub_key));
		}

		ccs_ksu_interface_fail_ecdsa_pct = false;
	}
#endif

	status = ecdsa_ecc_hw_verify_message (ecc, hash, hash_type, ECC_KAT_VECTORS_ECDSA_SIGNED,
		ECC_KAT_VECTORS_ECDSA_SIGNED_LEN, &ecc_pub_key, &ecdsa);
	if (status == ECC_HW_ECDSA_BAD_SIGNATURE) {
		status = CCS_KSU_PCT_FAILURE;
	}

exit:
	buffer_zeroize (&signature, sizeof (signature));
	buffer_zeroize (&ecc_pub_key, sizeof (ecc_pub_key));
	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	if (status != 0) {
		/* Clear the private key if there is an error with the PCT.  This is a best-effort step, so
		 * ignore any errors here. */
		memset (&pub_key.p384.Parts.X, 0, SP_MSG_384_SIZE);
		ccs->set_key (ccs, &pub_key.p384.Parts.X, key_slot, 0);

		buffer_zeroize (public_key, sizeof (*public_key));
	}
	else if (public_key == &pub_key) {
		buffer_zeroize (&pub_key, sizeof (pub_key));
	}

	return status;
}

/**
 * Generate a random ECDSA private key and put it in a key slot.  After key generation, the key pair
 * will be tested for consistency with a sign/verify sequence.  If the key attributes do not allow
 * for ECC signing, key generation will fail.
 *
 * @param ccs The CCS to use for key generation.
 * @param ecc The ECC hardware to use for signature verification.
 * @param key_slot Slot number for the generated ECDSA private key.
 * @param key_attributes Attributes to assign to the generated ECDSA key.  This is a bitmask of
 * attributes defined by {@link enum ccs_ksu_key_attributes}.
 * @param public_key Optional output for the public key of the ECDSA key pair.  The public key will
 * always be generated as part of the operation but will be discarded if this output is not
 * specified.
 *
 * @return 0 if the key was successfully generated or an error code.
 */
int ccs_ksu_interface_generate_random_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_slot,
	uint32_t key_attributes, union ccs_ksu_ecc_public_key *public_key)
{
	int status;

	if ((ccs == NULL) || (ecc == NULL) || (hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->generate_random_ecc_key (ccs, key_slot, key_attributes);
	if (status != 0) {
		return status;
	}

	return ccs_ksu_interface_validate_ecdsa_key_pair (ccs, ecc, hash, key_slot, key_attributes,
		public_key);
}

/**
 * Derive a deterministic ECDSA private key from a source key.  This key derivation uses a
 * deterministic seed to a DRBG.  After key generation, the key pair will be tested for consistency
 * with a sign/verify sequence.  If the key attributes do not allow for ECC signing, key generation
 * will fail.
 *
 * @param ccs The CCS to use to generate the new key.
 * @param ecc The ECC hardware to use for signature verification.
 * @param key_in Input key to use as a seed for the DRBG.
 * @param key_slot Destination slot for the new ECDSA private key.
 * @param key_attributes Attributes to apply to the generated ECDSA key.  These attributes will also
 * be part of the DRBG seed.  This is a bitmask of attributes defined by
 * {@link enum ccs_ksu_key_attributes}.
 * @param public_key Optional output for the public key of the ECDSA key pair.  The public key will
 * always be generated as part of the operation but will be discarded if this output is not
 * specified.
 *
 * @return 0 if the key was successfully generated or an error code.
 */
int ccs_ksu_interface_derive_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_in, uint8_t key_slot,
	uint32_t key_attributes, union ccs_ksu_ecc_public_key *public_key)
{
	int status;

	if ((ccs == NULL) || (ecc == NULL) || (hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs->derive_ecc_key (ccs, key_in, key_slot, key_attributes);
	if (status != 0) {
		return status;
	}

	return ccs_ksu_interface_validate_ecdsa_key_pair (ccs, ecc, hash, key_slot, key_attributes,
		public_key);
}

/**
 * Derive a deterministic ECDSA key from a source key and provide that key to firmware.  After key
 * generation, the key pair will be tested for consistency with a sign/verify sequence.  If the key
 * attributes do not allow for ECC signing, key generation will fail.
 *
 * @param ccs The CCS to use to generate the new key.
 * @param ecc The ECC hardware to use for signature verification.
 * @param key_in Input key to use as a seed for the DRBG.
 * @param key Output buffer for the derived ECDSA private key.  For a 256-bit key, only the first
 * 256 bits will contain valid data and the remaining data should be ignored.  Key length is
 * determined by the input attributes.
 * @param key_attributes Attributes to apply to the generated ECDSA key.  These attributes will also
 * be part of the DRBG seed.  This is a bitmask of attributes defined by
 * {@link enum ccs_ksu_key_attributes}.
 * @param public_key Optional output for the public key of the ECDSA key pair.  The public key will
 * always be generated as part of the operation but will be discarded if this output is not
 * specified.
 *
 * @return 0 if the key was successfully generated or an error code.
 */
int ccs_ksu_interface_derive_fw_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_in, SP_MSG_384 *key,
	uint32_t key_attributes, struct ecc_point_public_key *public_key)
{
	struct ecc_point_public_key pub_key = {0};
	size_t key_length;
	enum hash_type hash_type;
	int status;

	if ((ccs == NULL) || (ecc == NULL) || (hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (public_key == NULL) {
		/* If public key output is not provided, use internal storage for the key. */
		public_key = &pub_key;
	}

	if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = ECC_KEY_LENGTH_384;
		hash_type = HASH_TYPE_SHA384;
	}
	else {
		key_length = ECC_KEY_LENGTH_256;
		hash_type = HASH_TYPE_SHA256;
	}

	status = ccs->derive_fw_ecc_key (ccs, key_in, key, key_attributes);
	if (status != 0) {
		return status;
	}

	status = ecc->get_ecc_public_key (ecc, key->AsBytes, key_length, public_key);
	if (status != 0) {
		goto exit;
	}

	status = ecdsa_ecc_hw_pairwise_consistency_test (ecc, hash, hash_type, key->AsBytes, key_length,
		public_key);

exit:
	if (status != 0) {
		/* Clear the private key if there is an error with the PCT. */
		buffer_zeroize (key, SP_MSG_384_SIZE);
		buffer_zeroize (public_key, sizeof (*public_key));
	}
	else if (public_key == &pub_key) {
		buffer_zeroize (&pub_key, sizeof (pub_key));
	}

	return status;
}

/**
 * Zeroize all key slots available in the KSU.  This will be achieved by setting a key of zeros with
 * no attributes to each key slot.
 *
 * @param ccs The CCS to use for KSU zeroization.
 *
 * @return 0 if all key slots were zeroized or an error code.  On error, the KSU will be left in an
 * indeterminate state since some key slots may have been zeroized while others were not.
 */
int ccs_ksu_interface_zeroize_ksu (const struct ccs_ksu_interface *ccs)
{
	SP_MSG_384 zero = {0};
	int key_slot = 0;
	int status;

	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	do {
		status = ccs->is_key_slot_valid (ccs, key_slot);
		if (status != 0) {
			if (status == CCS_KSU_UNSUPPORTED_KEY_SLOT) {
				status = 0;
			}
			break;
		}

		status = ccs->set_key (ccs, &zero, key_slot++, 0);
	} while ((status == 0) && (key_slot < 256));

	return status;
}
