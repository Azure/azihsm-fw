// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "ccs_ksu_fips.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "crypto/ecdsa.h"


/**
 * Indicate if a private key is an ECC private key.
 *
 * @param attr Key attributes for the private key.
 *
 * @return true if the key is configured for ECC usage.
 */
#define	CCS_KSU_FIPS_IS_ECC_KEY(attr)   \
	((attr) & (CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_ECDH_ALLOWED))

/**
 * Context data for running CCS KDFs when deriving ECC private keys.  This is the SHA-384 hash of
 * the string "FipsEccKdf".
 */
static const SP_MSG_384 CCS_KSU_FIPS_ECC_KDF_CONTEXT = {
	.AsBytes = {
		0x31, 0x98, 0xe9, 0xf2, 0x90, 0xd8, 0x78, 0x72,
		0x1b, 0x3d, 0x01, 0x82, 0x56, 0xd1, 0x8c, 0xa5,
		0x97, 0x61, 0xdd, 0x7b, 0xf0, 0xed, 0x90, 0xf2,
		0xb4, 0x91, 0x71, 0x3f, 0x1b, 0xd2, 0xa2, 0x84,
		0x70, 0xfb, 0x42, 0x92, 0x8f, 0xdb, 0xef, 0xd6,
		0x68, 0x8c, 0x65, 0xfd, 0xb7, 0xa3, 0x89, 0x2d
	}
};


/**
 * Structure used to certify a P-384 public key.
 */
struct ccs_ksu_fips_public_key_certify_384 {
	SP_ECDSA_P384_PUBLIC public_key;	/**< The raw public key being certified. */
	uint32_t attributes;				/**< Attributes of the associated private key. */
	SP_MSG_384 pcr;						/**< PCR value used with the certification. */
	SP_MSG_384 context;					/**< Calling context provided for signing. */
};

/**
 * Structure used to certify a P-256 public key.
 */
struct ccs_ksu_fips_public_key_certify_256 {
	SP_ECDSA_P256_PUBLIC public_key;	/**< The raw public key being certified. */
	uint32_t attributes;				/**< Attributes of the associated private key. */
	SP_MSG_256 pcr;						/**< PCR value used with the certification. */
	SP_MSG_256 context;					/**< Calling context provided for signing. */
};

/**
 * Structure used to certify a P-256 public key with a P-384 signing key.
 */
struct ccs_ksu_fips_public_key_certify_256_with_384 {
	SP_ECDSA_P256_PUBLIC public_key;	/**< The raw public key being certified. */
	uint32_t attributes;				/**< Attributes of the associated private key. */
	SP_MSG_384 pcr;						/**< PCR value used with the certification. */
	SP_MSG_384 context;					/**< Calling context provided for signing. */
};


/**
 * Find the firmware KSU slot that contains the private key for the specified hardware KSU slot.
 *
 * @param fips The FIPS CCS driver that contains the KSU to search.
 * @param key_slot The hardware KSU key slot for the required key.
 * @param first_free Optional output indicating the first firmware KSU slot that is available for
 * use when the requested key is not currently in the firmware KSU.  If there are no available
 * firmware KSU slots, this will be equal to the firmware KSU slot count.  This value is only valid
 * when the requested key is not in the firmware KSU.
 *
 * @return The firmware KSU slot that contains the requested private key.  If the private key is not
 * present in the firmware KSU, this will be equal to the firmware KSU slot count.
 */
static size_t ccs_ksu_fips_get_fw_ksu_slot (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	size_t *first_free)
{
	size_t i;
	size_t unused_slot = fips->ksu->slot_count;

	for (i = 0; i < fips->ksu->slot_count; i++) {
		if (fips->ksu->slots[i].hw_slot == key_slot) {
			break;
		}

		/* Keep track of the first unused slot in the firmware KSU. */
		if ((unused_slot == fips->ksu->slot_count) &&
			(fips->ksu->slots[i].hw_slot == CCS_KSU_FIPS_KSU_SLOT_UNUSED)) {
			unused_slot = i;
		}
	}

	if (first_free) {
		*first_free = unused_slot;
	}

	return i;
}

/**
 * Get an available firmware KSU slot to use for a private key.  If the hardware KSU slot already
 * has a corresponding private key in the firmware KSU, this slot is returned.  If the hardware KSU
 * slot is not already in the firmware KSU, an unused slot is returned, if one is available.
 *
 * This doesn't actually mark the firmware KSU slot as being used.  This is the caller's
 * responsibility after populating it with a private key.
 *
 * @param fips The FIPS CCS driver that contains the KSU to query.
 * @param key_slot The hardware KSU key slot that will have a private key in the firmware KSU.
 * @param fw_slot Output for the firmware KSU key slot to use for the private key.
 *
 * @return 0 if an available firmware KSU slot was found or CCS_KSU_NO_KEY_SLOT_AVAILABLE if there
 * is no firmware KSU slot available for the private key.
 */
static int ccs_ksu_fips_alloc_fw_ksu_slot (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	size_t *fw_slot)
{
	size_t first_free;

	*fw_slot = ccs_ksu_fips_get_fw_ksu_slot (fips, key_slot, &first_free);

	if (*fw_slot == fips->ksu->slot_count) {
		if (first_free == fips->ksu->slot_count) {
			return CCS_KSU_NO_KEY_SLOT_AVAILABLE;
		}

		/* The HW key slot is unmapped.  Use the first free firmware slot. */
		*fw_slot = first_free;
	}

	return 0;
}

/**
 * Erase a private key present in a firmware KSU slot and mark the slot as unused.  The firmware KSU
 * slot to erase is specified explicitly.  If the specified KSU slot is not valid for the firmware
 * KSU, nothing is done.
 *
 * @param fips The FIPS CCS driver that contains the KSU to update.
 * @param fw_slot The firmware KSU key slot for the key to erase.
 */
static void ccs_ksu_fips_delete_private_key (const struct ccs_ksu_fips *fips, size_t fw_slot)
{
	if (fw_slot < fips->ksu->slot_count) {
		buffer_zeroize (&fips->ksu->slots[fw_slot], sizeof (fips->ksu->slots[0]));

		fips->ksu->slots[fw_slot].hw_slot = CCS_KSU_FIPS_KSU_SLOT_UNUSED;
	}
}

/**
 * Erase a private key present in a firmware KSU slot and mark the slot as unused.  The firmware KSU
 * slot to erase is determined using a specified hardware KSU slot number.  If the specified key
 * slot is not present in the firmware KSU, nothing is done.
 *
 * @param fips The FIPS CCS driver that contains the KSU to update.
 * @param key_slot The hardware KSU key slot for the key to erase.
 */
static void ccs_ksu_fips_free_fw_ksu_slot (const struct ccs_ksu_fips *fips, uint8_t key_slot)
{
	size_t fw_slot;

	fw_slot = ccs_ksu_fips_get_fw_ksu_slot (fips, key_slot, NULL);

	ccs_ksu_fips_delete_private_key (fips, fw_slot);
}

/**
 * Derive an ECC private key into a firmware KSU key slot from a key seed in the hardware KSU.
 *
 * @param fips The FIPS CCS driver that contains the KSU to update with the new private key.
 * @param key_slot The hardware KSU key slot containing the seed for private key derivation.
 * @param fw_slot The firmware KSU key slot that will contain the ECC private key.
 * @param key_attributes KSU attributes that will be assigned to the new private key.
 *
 * @return 0 if the private key was derived successfully or an error code.
 */
static int ccs_ksu_fips_new_private_key (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	size_t fw_slot, uint32_t key_attributes)
{
	int status;

	status = fips->ccs_hw->derive_fw_ecc_key (fips->ccs_hw, key_slot,
		&fips->ksu->slots[fw_slot].key_384, key_attributes);
	if (status != 0) {
		return status;
	}

	fips->ksu->slots[fw_slot].hw_slot = key_slot;
	fips->ksu->slots[fw_slot].attributes = key_attributes;

	return 0;
}

/**
 * Get the private key in the firmware KSU for the specified hardware KSU key slot.  If the private
 * key does not currently exist in the firmware KSU, it will be derived from the seed in hardware
 * and populated in an available firmware KSU slot.
 *
 * @param fips The FIPS CCS driver that contains the KSU for the private key.
 * @param key_slot The hardware KSU key slot for the private key.
 * @param fw_slot Output for the firmware KSU key slot that contains the private key.
 *
 * @return 0 if the private key is present in the specified firmware KSU slot or an error code.
 */
static int ccs_ksu_fips_get_private_key (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	size_t *fw_slot)
{
	int status;

	status = ccs_ksu_fips_alloc_fw_ksu_slot (fips, key_slot, fw_slot);
	if (status != 0) {
		return status;
	}

	/* If no mapping, derive the private key into the empty FW KSU slot.  The derived key attributes
	 * will be the attributes from the HW KSU after masking DeriveECCKey.  There is no need to check
	 * the attributes at this stage since the HW will reject derive requests for invalid keys and
	 * required attributes for the ECC private key are unknown in this context. */
	if (fips->ksu->slots[*fw_slot].hw_slot == CCS_KSU_FIPS_KSU_SLOT_UNUSED) {
		uint32_t key_attributes;

		status = fips->ccs_hw->get_key_attributes (fips->ccs_hw, key_slot, &key_attributes);
		if (status != 0) {
			return status;
		}

		key_attributes &= ~CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED;

		/* Private key derivation is an expensive process since it involves switching the RNG into
		 * FW mode, so keep the derived key active in the FW KSU so it doesn't need to be derived
		 * again later. */
		status = ccs_ksu_fips_new_private_key (fips, key_slot, *fw_slot, key_attributes);
	}

	return status;
}

/**
 * Get a private key that can be used for ECDSA signing in the firmware KSU for the specified
 * hardware KSU key slot.  If the private key does not currently exist in the firmware KSU, it will
 * be derived from the seed in hardware and populated in an available firmware KSU slot.
 *
 * The key attributes will be inspected to ensure compatible usage.
 *
 * @param fips The FIPS CCS driver that contains the KSU for the ECDSA private key.
 * @param key_slot The hardware KSU key slot for the ECDSA private key.
 * @param attr_error The error code to return if the key attributes are not correct for ECDSA
 * signing operations.
 * @param fw_slot Output for the firmware KSU key slot that contains the ECDSA private key.
 *
 * @return 0 if the ECDSA private key is present in the specified firmware KSU slot or an error
 * code.
 */
static int ccs_ksu_fips_get_ecdsa_private_key (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	int attr_error, size_t *fw_slot)
{
	int status;

	status = ccs_ksu_fips_get_private_key (fips, key_slot, fw_slot);
	if (status != 0) {
		return status;
	}

	/* NOTE:  MustAppendPCR is ignored as part of this check and subsequent processing.  Requiring
	 * a PCR to be appended is challenging to manage from a FIPS compliance perspective for both
	 * ECDSA signature generation and pairwise consistency tests.  Additionally, there is no current
	 * use-case that requires a PCR to be automatically included as part of the signature.  This
	 * deviates from CCS hardware behavior, but allows this layer to work with any key that can be
	 * used for ECC signing. */

	/* The key must allow ECC signing and must not require a PCR to be appended. */
	if (!(fips->ksu->slots[*fw_slot].attributes & CCS_KSU_ATTR_ECC_SIGN_ALLOWED)) {
		return attr_error;
	}

	return 0;
}

/**
 * Get an ECC private key in the firmware KSU for the specified hardware KSU key slot.  If the
 * private key does not currently exist in the firmware KSU, it will be derived from the seed in
 * hardware and populated in an available firmware KSU slot.
 *
 * The key attributes will be inspected to ensure compatible usage.
 *
 * @param fips The FIPS CCS driver that contains the KSU for the ECC private key.
 * @param key_slot The hardware KSU key slot for the ECC private key.
 * @param attr_error The error code to return if the key attributes are not correct for an ECC key.
 * @param fw_slot Output for the firmware KSU key slot that contains the ECC private key.
 * @param key_attributes Optional output for the attributes on the private key.
 *
 * @return 0 if the ECDSA private key is present in the specified firmware KSU slot or an error
 * code.
 */
static int ccs_ksu_fips_get_ecc_private_key (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	int attr_error, size_t *fw_slot, uint32_t *key_attributes)
{
	int status;

	status = ccs_ksu_fips_get_private_key (fips, key_slot, fw_slot);
	if (status != 0) {
		return status;
	}

	/* If FW KSU slot does not have ECC attributes (ECCSign or ECDH), fail the operation. */
	if (!CCS_KSU_FIPS_IS_ECC_KEY (fips->ksu->slots[*fw_slot].attributes)) {
		return attr_error;
	}

	if (key_attributes) {
		*key_attributes = fips->ksu->slots[*fw_slot].attributes;
	}

	return 0;
}

/**
 * Get the public key for an ECC private key stored in the KSU.  If the private key is not present
 * in firmware memory, an available firmware KSU slot will be populated with the private key.
 *
 * @param fips The FIPS CCS instance containing the private key.
 * @param key_slot HW KSU slot identifier for the ECC private key.
 * @param public_key Output for the public key data.  This can hold either a P-384 or P-256 public
 * key, depending on the attributes on the private key.
 * @param key_attributes Optional output for the attributes on the private key.
 *
 * @return 0 if the public key was generated successfully or an error code.
 */
static int ccs_ksu_fips_generate_public_key (const struct ccs_ksu_fips *fips, uint8_t key_slot,
	SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes)
{
	struct ecc_point_public_key pub_key = {0};
	SP_ECDSA_P256_PUBLIC *key_p256 = (SP_ECDSA_P256_PUBLIC*) public_key;
	size_t fw_slot;
	size_t key_length;
	int status;

	status = ccs_ksu_fips_get_ecc_private_key (fips, key_slot, CCS_KSU_ECC_PUBLIC_FAILED, &fw_slot,
		key_attributes);
	if (status != 0) {
		return status;
	}

	if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
	}
	else {
		key_length = SP_MSG_256_SIZE;
	}

	/* Call PKA to get the public key for the private key from FW KSU. */
	status = fips->ecc->get_ecc_public_key (fips->ecc, fips->ksu->slots[fw_slot].key_384.AsBytes,
		key_length, &pub_key);
	if (status != 0) {
		goto exit;
	}

	if (pub_key.key_length == ECC_KEY_LENGTH_384) {
		memcpy (public_key->Parts.X.AsBytes, pub_key.x, SP_MSG_384_SIZE);
		memcpy (public_key->Parts.Y.AsBytes, pub_key.y, SP_MSG_384_SIZE);
	}
	else {
		memcpy (key_p256->Parts.X.AsBytes, pub_key.x, SP_MSG_256_SIZE);
		memcpy (key_p256->Parts.Y.AsBytes, pub_key.y, SP_MSG_256_SIZE);
	}

exit:
	buffer_zeroize (&pub_key, sizeof (pub_key));

	return status;
}

/**
 * Copy ECDSA signature data into a packed, raw format.
 *
 * @param ecdsa The ECDSA signature to copy.
 * @param signature Output for the raw signature data.
 */
void ccs_ksu_fips_copy_raw_signature (const struct ecc_ecdsa_signature *ecdsa,
	SP_ECDSA_P384_SIGNATURE *signature)
{
	SP_ECDSA_P256_SIGNATURE *sig_p256 = (SP_ECDSA_P256_SIGNATURE*) signature;

	if (ecdsa->length == ECC_KEY_LENGTH_384) {
		memcpy (signature->Parts.R.AsBytes, ecdsa->r, SP_MSG_384_SIZE);
		memcpy (signature->Parts.S.AsBytes, ecdsa->s, SP_MSG_384_SIZE);
	}
	else {
		memcpy (sig_p256->Parts.R.AsBytes, ecdsa->r, SP_MSG_256_SIZE);
		memcpy (sig_p256->Parts.S.AsBytes, ecdsa->s, SP_MSG_256_SIZE);
	}
}

int ccs_ksu_fips_is_key_slot_valid (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->is_key_slot_valid (fips->ccs_hw, key_slot);
}

int ccs_ksu_fips_get_key_attributes (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	size_t fw_slot;
	int status;

	if ((fips == NULL) || (key_attributes == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	fw_slot = ccs_ksu_fips_get_fw_ksu_slot (fips, key_slot, NULL);

	if (fw_slot == fips->ksu->slot_count) {
		/* The key is not in the FW KSU, so query the hardware for attributes. */
		status = fips->ccs_hw->get_key_attributes (fips->ccs_hw, key_slot, key_attributes);
	}
	else {
		*key_attributes = fips->ksu->slots[fw_slot].attributes;
		status = 0;
	}

	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_set_key (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key,
	uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	size_t fw_slot;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (CCS_KSU_FIPS_IS_ECC_KEY (key_attributes)) {
		SP_MSG_384 zero = {{0}};

		/* Known ECC cannot be managed in both HW and FW like other keys can.  In this case, the ECC
		 * key will only exist in firmware KSU memory.  Any key that exists in the hardware KSU will
		 * get erased.  If firmware memory is cleared, the ECC key will be lost. */

		status = ccs_ksu_fips_alloc_fw_ksu_slot (fips, key_slot, &fw_slot);
		if (status != 0) {
			goto exit;
		}

		/* Clear any existing key data in both the hardware and firmware KSU before copying the new
		 * key to the firmware KSU. */
		status = fips->ccs_hw->set_key (fips->ccs_hw, &zero, key_slot, 0);
		if (status != 0) {
			goto exit;
		}

		ccs_ksu_fips_delete_private_key (fips, fw_slot);

		/* Set the ECC key. */
		if (key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
			memcpy (fips->ksu->slots[fw_slot].key_384.AsBytes, key->AsBytes, SP_MSG_384_SIZE);
		}
		else {
			memcpy (fips->ksu->slots[fw_slot].key_256.AsBytes, key->AsBytes, SP_MSG_256_SIZE);
		}

		fips->ksu->slots[fw_slot].hw_slot = key_slot;
		fips->ksu->slots[fw_slot].attributes = key_attributes;
	}
	else {
		status = fips->ccs_hw->set_key (fips->ccs_hw, key, key_slot, key_attributes);

		if (status == 0) {
			/* Free any firmware KSU slot used for the key that was just changed in hardware. */
			ccs_ksu_fips_free_fw_ksu_slot (fips, key_slot);
		}
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

#ifdef CCS_KSU_ENABLE_SEND_KEY
int ccs_ksu_fips_send_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t dest_addr)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = fips->ccs_hw->send_key (fips->ccs_hw, key_slot, dest_addr);

	platform_mutex_unlock (&fips->state->lock);

	return status;
}
#endif

int ccs_ksu_fips_generate_random_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = fips->ccs_hw->generate_random_key (fips->ccs_hw, key_slot, key_attributes);

	if (status == 0) {
		/* Free any firmware KSU slot used for the key that was just changed in hardware. */
		ccs_ksu_fips_free_fw_ksu_slot (fips, key_slot);
	}

	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_derive_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (ccs_ksu_fips_get_fw_ksu_slot (fips, key_in, NULL) != fips->ksu->slot_count) {
		/* Regardless of key attributes, deriving new keys from a key in the firmware KSU is not
		 * supported.  It's possible to do this derivation in firmware and use set_key to store it
		 * in hardware, but this is not an expected use-case for private keys in the firmware KSU
		 * and adds complexity. */
		status = CCS_KSU_DERIVE_KEY_FAILED;
		goto exit;
	}

	status = fips->ccs_hw->derive_key (fips->ccs_hw, key_in, context, key_slot, key_attributes);

	if (status == 0) {
		/* Free any firmware KSU slot used for the key that was just changed in hardware. */
		ccs_ksu_fips_free_fw_ksu_slot (fips, key_slot);
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_derive_key_using_pcr (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t pcr, uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (ccs_ksu_fips_get_fw_ksu_slot (fips, key_in, NULL) != fips->ksu->slot_count) {
		/* Regardless of key attributes, deriving new keys from a key in the firmware KSU is not
		 * supported.  It's possible to do this derivation in firmware and use set_key to store it
		 * in hardware, but this is not an expected use-case for private keys in the firmware KSU
		 * and adds complexity. */
		status = CCS_KSU_DERIVE_KEY_PCR_FAILED;
		goto exit;
	}

	status = fips->ccs_hw->derive_key_using_pcr (fips->ccs_hw, key_in, pcr, key_slot,
		key_attributes);

	if (status == 0) {
		/* Free any firmware KSU slot used for the key that was just changed in hardware. */
		ccs_ksu_fips_free_fw_ksu_slot (fips, key_slot);
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_generate_random_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	size_t fw_slot;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_alloc_fw_ksu_slot (fips, key_slot, &fw_slot);
	if (status != 0) {
		goto exit;
	}

	/* Generate a random seed that can be used to derive an ECC key into FW memory. */
	status = fips->ccs_hw->generate_random_key (fips->ccs_hw, key_slot,
		CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | key_attributes);
	if (status != 0) {
		goto exit;
	}

	/* The HW key has been changed, so erase the private key in the firmware KSU.  This will get
	 * repopulated on successful derivation of the new private key. */
	ccs_ksu_fips_delete_private_key (fips, fw_slot);

	status = ccs_ksu_fips_new_private_key (fips, key_slot, fw_slot, key_attributes);

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_derive_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	size_t fw_slot;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_alloc_fw_ksu_slot (fips, key_slot, &fw_slot);
	if (status != 0) {
		goto exit;
	}

	/* Derive a new ECC seed into the destination slot using the source key.  Derive a new private
	 * key into the firmware KSU from this seed. */
	status = fips->ccs_hw->derive_key (fips->ccs_hw, key_in, &CCS_KSU_FIPS_ECC_KDF_CONTEXT,
		key_slot, CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED | key_attributes);
	if (status != 0) {
		goto exit;
	}

	/* The HW key has been changed, so erase the private key in the firmware KSU.  This will get
	 * repopulated on successful derivation of the new private key. */
	ccs_ksu_fips_delete_private_key (fips, fw_slot);

	status = ccs_ksu_fips_new_private_key (fips, key_slot, fw_slot, key_attributes);

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_derive_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	SP_MSG_384 *key, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (ccs_ksu_fips_get_fw_ksu_slot (fips, key_in, NULL) != fips->ksu->slot_count) {
		/* Regardless of key attributes, deriving a key from a private key in the firmware KSU is
		 * not supported.  The expectation is that the firmware KSU will only contain ECC keys, so
		 * attempting to derive additional ECC keys from this is not an expected use-case. */
		status = CCS_KSU_DERIVE_FW_ECC_FAILED;
		goto exit;
	}

	/* There is nothing special to do here since the destination is outside the KSU.  Just forward
	 * the request to the CCS HW. */
	status = fips->ccs_hw->derive_fw_ecc_key (fips->ccs_hw, key_in, key, key_attributes);

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_export_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_MSG_384 *key, uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	size_t fw_slot;
	int status;

	if ((fips == NULL) || (key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_ecc_private_key (fips, key_slot, CCS_KSU_EXPORT_FW_ECC_FAILED,
		&fw_slot, key_attributes);
	if (status == 0) {
		/* Always copy the full data.  256-bit keys will have the last bytes set to zero. */
		memcpy (key->AsBytes, fips->ksu->slots[fw_slot].key_384.AsBytes, SP_MSG_384_SIZE);
	}

	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_get_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if ((fips == NULL) || (public_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_generate_public_key (fips, key_slot, public_key, key_attributes);

	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_certify_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data, SP_ECDSA_P384_PUBLIC *public_key,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *public_key_attributes,
	uint32_t *signing_key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	union {
		struct ccs_ksu_fips_public_key_certify_384 p384;
		struct ccs_ksu_fips_public_key_certify_256 p256;
		struct ccs_ksu_fips_public_key_certify_256_with_384 p256_384;
	} certify;
	uint32_t key_attributes;
	SP_MSG_384 pcr_value;
	enum hash_type hash_algo;
	size_t key_length;
	size_t pub_length;
	size_t certify_length;
	struct ecc_ecdsa_signature ecdsa;
	size_t fw_slot;
	int status;

	if ((fips == NULL) || (sign_data == NULL) || (public_key == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_private_key (fips, signing_key, &fw_slot);
	if (status != 0) {
		goto exit;
	}

	/* The signer key must be enabled for ECC signing and must require a PCR to be appended. */
	key_attributes = fips->ksu->slots[fw_slot].attributes &
		(CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_MUST_APPEND_PCR);

	if (key_attributes != (CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_MUST_APPEND_PCR)) {
		status = CCS_KSU_CERTIFY_ECC_FAILED;
		goto exit;
	}

	status = ccs_ksu_fips_generate_public_key (fips, key_slot, &certify.p384.public_key,
		&key_attributes);
	if (status != 0) {
		goto exit;
	}

	status = fips->ccs_hw->get_pcr_value (fips->ccs_hw, pcr, &pcr_value);
	if (status != 0) {
		goto exit;
	}

	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) &&
		(fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
		/* Both keys are 384-bit keys. */
		certify.p384.attributes = key_attributes;
		memcpy (certify.p384.pcr.AsBytes, pcr_value.AsBytes, SP_MSG_384_SIZE);
		memcpy (certify.p384.context.AsBytes, sign_data->AsBytes, SP_MSG_384_SIZE);

		hash_algo = HASH_TYPE_SHA384;
		key_length = SP_MSG_384_SIZE;
		pub_length = SP_ECDSA_P384_PUBLIC_KEY_SIZE;
		certify_length = sizeof (certify.p384);
	}
	else if (!(key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) &&
		!(fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384)) {
		/* Both keys are 256-bit keys. */
		certify.p256.attributes = key_attributes;
		memcpy (certify.p256.pcr.AsBytes, pcr_value.AsBytes, SP_MSG_256_SIZE);
		memcpy (certify.p256.context.AsBytes, sign_data->AsBytes, SP_MSG_256_SIZE);

		hash_algo = HASH_TYPE_SHA256;
		key_length = SP_MSG_256_SIZE;
		pub_length = SP_ECDSA_P256_PUBLIC_KEY_SIZE;
		certify_length = sizeof (certify.p256);
	}
	else if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		/* The signer is 384 bits and the key to certify is 256 bits. */
		certify.p256_384.attributes = key_attributes;
		memcpy (certify.p256_384.pcr.AsBytes, pcr_value.AsBytes, SP_MSG_384_SIZE);
		memcpy (certify.p256_384.context.AsBytes, sign_data->AsBytes, SP_MSG_384_SIZE);

		hash_algo = HASH_TYPE_SHA384;
		key_length = SP_MSG_384_SIZE;
		pub_length = SP_ECDSA_P256_PUBLIC_KEY_SIZE;
		certify_length = sizeof (certify.p256_384);
	}
	else {
		/* The signer is 256 bits and the key to certify is 384 bits.  CCS does not allow a 256-bit
		 * key to certify a 384-bit key. */
		status = CCS_KSU_CERTIFY_ECC_FAILED;
		goto exit;
	}

	status = ecdsa_ecc_hw_sign_message (fips->ecc, fips->hash, hash_algo, NULL,
		fips->ksu->slots[fw_slot].key_384.AsBytes, key_length, (uint8_t*) &certify.p384,
		certify_length, &ecdsa);
	if (status != 0) {
		goto exit;
	}

	memcpy (public_key->AsBytes, certify.p384.public_key.AsBytes, pub_length);
	ccs_ksu_fips_copy_raw_signature (&ecdsa, signature);

	if (public_key_attributes) {
		*public_key_attributes = key_attributes;
	}
	if (signing_key_attributes) {
		*signing_key_attributes = fips->ksu->slots[fw_slot].attributes;
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	buffer_zeroize (&certify, sizeof (certify));
	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	return status;
}

int ccs_ksu_fips_ecdh_key_exchange (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_out, const uint8_t *partner_public_key_and_hash, size_t input_len,
	uint32_t key_attributes)
{
	UNUSED (ccs);
	UNUSED (key_in);
	UNUSED (key_out);
	UNUSED (partner_public_key_and_hash);
	UNUSED (input_len);
	UNUSED (key_attributes);

	/* TODO: To implement the flow if it becomes necessary.*/
	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_fips_ecc_sign (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const SP_MSG_384 *digest, const struct rng_engine *rng, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	struct ecc_ecdsa_signature ecdsa;
	size_t fw_slot;
	size_t key_length;
	int status;

	if ((fips == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_ecdsa_private_key (fips, signing_key, CCS_KSU_ECC_SIGN_FAILED,
		&fw_slot);
	if (status != 0) {
		goto exit;
	}

	if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
	}
	else {
		key_length = SP_MSG_256_SIZE;
	}

	status = fips->ecc->ecdsa_sign (fips->ecc, fips->ksu->slots[fw_slot].key_384.AsBytes,
		key_length, digest->AsBytes, key_length, rng, &ecdsa);
	if (status != 0) {
		goto exit;
	}

	ccs_ksu_fips_copy_raw_signature (&ecdsa, signature);

	if (key_attributes) {
		*key_attributes = fips->ksu->slots[fw_slot].attributes;
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	return status;
}

int ccs_ksu_fips_ecdsa_sign_message (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const uint8_t *message, size_t length, const struct hash_engine *hash, enum hash_type hash_algo,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	struct ecc_ecdsa_signature ecdsa;
	size_t fw_slot;
	size_t key_length;
	int status;

	if ((fips == NULL) || (hash == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_ecdsa_private_key (fips, signing_key, CCS_KSU_ECDSA_SIGN_MSG_FAILED,
		&fw_slot);
	if (status != 0) {
		goto exit;
	}

	if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
	}
	else {
		key_length = SP_MSG_256_SIZE;
	}

	status = ecdsa_ecc_hw_sign_message (fips->ecc, hash, hash_algo, NULL,
		fips->ksu->slots[fw_slot].key_384.AsBytes, key_length, message, length, &ecdsa);
	if (status != 0) {
		goto exit;
	}

	ccs_ksu_fips_copy_raw_signature (&ecdsa, signature);

	if (key_attributes) {
		*key_attributes = fips->ksu->slots[fw_slot].attributes;
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	return status;
}

int ccs_ksu_fips_ecdsa_sign_hash (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	struct ecc_ecdsa_signature ecdsa;
	size_t fw_slot;
	size_t key_length;
	int status;

	if ((fips == NULL) || (hash == NULL) || (signature == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_ecdsa_private_key (fips, signing_key, CCS_KSU_ECDSA_SIGN_HASH_FAILED,
		&fw_slot);
	if (status != 0) {
		goto exit;
	}

	if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
	}
	else {
		key_length = SP_MSG_256_SIZE;
	}

	status = ecdsa_ecc_hw_sign_hash (fips->ecc, hash, NULL,
		fips->ksu->slots[fw_slot].key_384.AsBytes, key_length, &ecdsa);
	if (status != 0) {
		goto exit;
	}

	ccs_ksu_fips_copy_raw_signature (&ecdsa, signature);

	if (key_attributes) {
		*key_attributes = fips->ksu->slots[fw_slot].attributes;
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	return status;
}

int ccs_ksu_fips_ecdsa_sign_hash_and_finish (const struct ccs_ksu_interface *ccs,
	uint8_t signing_key, const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	struct ecc_ecdsa_signature ecdsa;
	size_t fw_slot;
	size_t key_length;
	int status;

	if (hash == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((fips == NULL) || (signature == NULL)) {
		/* Always cancel the active hash, if possible. */
		hash->cancel (hash);

		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	status = ccs_ksu_fips_get_ecdsa_private_key (fips, signing_key, CCS_KSU_ECDSA_SIGN_HASH_FAILED,
		&fw_slot);
	if (status != 0) {
		/* Cancel the active hash on any error. */
		hash->cancel (hash);
		goto exit;
	}

	if (fips->ksu->slots[fw_slot].attributes & CCS_KSU_ATTR_KEY_SIZE_384) {
		key_length = SP_MSG_384_SIZE;
	}
	else {
		key_length = SP_MSG_256_SIZE;
	}

	status = ecdsa_ecc_hw_sign_hash_and_finish (fips->ecc, hash, NULL,
		fips->ksu->slots[fw_slot].key_384.AsBytes, key_length, &ecdsa);
	if (status != 0) {
		goto exit;
	}

	ccs_ksu_fips_copy_raw_signature (&ecdsa, signature);

	if (key_attributes) {
		*key_attributes = fips->ksu->slots[fw_slot].attributes;
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	buffer_zeroize (&ecdsa, sizeof (ecdsa));

	return status;
}

int ccs_ksu_fips_wrap_key_buffer (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->wrap_key_buffer (fips->ccs_hw, kek_slot, key, wrapped_key, key_attributes);
}

int ccs_ksu_fips_unwrap_key (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_512 *wrapped_key, uint8_t key_slot)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (ccs_ksu_fips_get_fw_ksu_slot (fips, kek_slot, NULL) != fips->ksu->slot_count) {
		/* It is not possible to unwrap a key using a KEK in the FW KSU. */
		status = CCS_KSU_UNWRAP_FAILED;
		goto exit;
	}

	status = fips->ccs_hw->unwrap_key (fips->ccs_hw, kek_slot, wrapped_key, key_slot);

	if (status == 0) {
		/* Free any firmware KSU slot used for the key that was just changed in hardware. */
		ccs_ksu_fips_free_fw_ksu_slot (fips, key_slot);
	}

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

int ccs_ksu_fips_burn_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->burn_key (fips->ccs_hw, key_slot);
}

int ccs_ksu_fips_reset_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->reset_pcr (fips->ccs_hw, pcr);
}

int ccs_ksu_fips_extend_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr,
	const SP_MSG_384 *digest)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->extend_pcr (fips->ccs_hw, pcr, digest);
}

int ccs_ksu_fips_get_pcr_value (const struct ccs_ksu_interface *ccs, uint8_t pcr, SP_MSG_384 *value)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	return fips->ccs_hw->get_pcr_value (fips->ccs_hw, pcr, value);
}

int ccs_ksu_fips_hmac (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
	size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes)
{
	const struct ccs_ksu_fips *fips = (const struct ccs_ksu_fips*) ccs;
	int status;

	if (fips == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&fips->state->lock);

	if (ccs_ksu_fips_get_fw_ksu_slot (fips, key_slot, NULL) != fips->ksu->slot_count) {
		/* Regardless of key attributes, generating an HMAC from a key in the FW KSU is not
		 * supported.  In theory, this could be handled since the key is available, but this is not
		 * an expected use-case and adds complexity to this processing. */
		status = CCS_KSU_HMAC_FAILED;
		goto exit;
	}

	status = fips->ccs_hw->hmac (fips->ccs_hw, key_slot, data, length, hmac, key_attributes);

exit:
	platform_mutex_unlock (&fips->state->lock);

	return status;
}

/**
 * Initialize a FIPS compliant implementation for CCS operations.
 *
 * @param ccs The CCS instance to initialize.
 * @param state Variable context for the CCS instance.  This must be uninitialized.
 * @param ksu The firmware KSU memory available for storing private keys.
 * @param ccs_hw CCS hardware implementation that will be leveraged for non-ECC operations.
 * @param ecc The ECC hardware accelerator to use for ECC operations.
 * @param hash A hash engine to use for certifying public keys.
 * @param ksu_init Flag to indicate if the firmware KSU memory should be initialized or if the
 * contents should be left unmodified.
 *
 * @return 0 if the CCS was initialized successfully or an error code.
 */
int ccs_ksu_fips_init (struct ccs_ksu_fips *ccs, struct ccs_ksu_fips_state *state,
	const struct ccs_ksu_fips_ksu *ksu, const struct ccs_ksu_interface *ccs_hw,
	const struct ecc_hw *ecc, const struct hash_engine *hash, bool ksu_init)
{
	if (ccs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	memset (ccs, 0, sizeof (*ccs));

	ccs->base.is_key_slot_valid = ccs_ksu_fips_is_key_slot_valid;
	ccs->base.get_key_attributes = ccs_ksu_fips_get_key_attributes;
	ccs->base.set_key = ccs_ksu_fips_set_key;
#ifdef CCS_KSU_ENABLE_SEND_KEY
	ccs->base.send_key = ccs_ksu_fips_send_key;
#endif
	ccs->base.generate_random_key = ccs_ksu_fips_generate_random_key;
	ccs->base.derive_key = ccs_ksu_fips_derive_key;
	ccs->base.derive_key_using_pcr = ccs_ksu_fips_derive_key_using_pcr;
	ccs->base.generate_random_ecc_key = ccs_ksu_fips_generate_random_ecc_key;
	ccs->base.derive_ecc_key = ccs_ksu_fips_derive_ecc_key;
	ccs->base.derive_fw_ecc_key = ccs_ksu_fips_derive_fw_ecc_key;
	ccs->base.export_fw_ecc_key = ccs_ksu_fips_export_fw_ecc_key;
	ccs->base.get_ecc_public_key = ccs_ksu_fips_get_ecc_public_key;
	ccs->base.certify_ecc_public_key = ccs_ksu_fips_certify_ecc_public_key;
	ccs->base.ecdh_key_exchange = ccs_ksu_fips_ecdh_key_exchange;
	ccs->base.ecc_sign = ccs_ksu_fips_ecc_sign;
	ccs->base.ecdsa_sign_message = ccs_ksu_fips_ecdsa_sign_message;
	ccs->base.ecdsa_sign_hash = ccs_ksu_fips_ecdsa_sign_hash;
	ccs->base.ecdsa_sign_hash_and_finish = ccs_ksu_fips_ecdsa_sign_hash_and_finish;
	ccs->base.wrap_key_buffer = ccs_ksu_fips_wrap_key_buffer;
	ccs->base.unwrap_key = ccs_ksu_fips_unwrap_key;
	ccs->base.burn_key = ccs_ksu_fips_burn_key;
	ccs->base.reset_pcr = ccs_ksu_fips_reset_pcr;
	ccs->base.extend_pcr = ccs_ksu_fips_extend_pcr;
	ccs->base.get_pcr_value = ccs_ksu_fips_get_pcr_value;
	ccs->base.hmac = ccs_ksu_fips_hmac;

	ccs->state = state;
	ccs->ksu = ksu;
	ccs->ccs_hw = ccs_hw;
	ccs->ecc = ecc;
	ccs->hash = hash;

	return ccs_ksu_fips_init_state (ccs, ksu_init);
}

/**
 * Initialize only the variable context of a FIPS compliant CCS implementation.  The rest of the
 * instance is assumed to have already been initialized.
 *
 * This will typically be used with static initialzed instances.
 *
 * @param ccs The CCS instance containing the state to initialize.
 * @param ksu_init Flag to indicate if the firmware KSU memory should be initialized or if the
 * contents should be left unmodified.
 *
 * @return 0 if the CCS state was initialized successfully or an error code.
 */
int ccs_ksu_fips_init_state (const struct ccs_ksu_fips *ccs, bool ksu_init)
{
	size_t i;

	if ((ccs == NULL) || (ccs->state == NULL) || (ccs->ksu == NULL) || (ccs->ccs_hw == NULL) ||
		(ccs->ecc == NULL) || (ccs->hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((ccs->ksu->slots == NULL) || (ccs->ksu->slot_count == 0)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	memset (ccs->state, 0, sizeof (*ccs->state));

	if (ksu_init) {
		/* Mark each KSU slot as unused. */
		for (i = 0; i < ccs->ksu->slot_count; i++) {
			ccs->ksu->slots[i].hw_slot = CCS_KSU_FIPS_KSU_SLOT_UNUSED;
		}
	}

	return platform_mutex_init (&ccs->state->lock);
}

/**
 * Release the resources used by a FIPS compliant CCS instance.  Firmware KSU contents will not be
 * modified.
 *
 * @param ccs The CCS instance to release.
 */
void ccs_ksu_fips_release (const struct ccs_ksu_fips *ccs)
{
	if (ccs) {
		platform_mutex_free (&ccs->state->lock);
	}
}
