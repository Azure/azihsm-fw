// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_INTERFACE_H_
#define CCS_KSU_INTERFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "crypto/rng.h"
#include "splibs/inc/spcryptotypes.h"
#include "status/msft_module_id.h"


/**
 * Attribute flags that can be set for a key slot.  Additional details about these attributes can be
 * found in the CCS documentation.
 */
enum ccs_ksu_key_attributes {
	CCS_KSU_ATTR_IS_DEVICE_SECRET = (1U << 0),				/**< Key is unique and secret to the device. */
	CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED = (1U << 1),			/**< Key can be used for AES encryption. */
	CCS_KSU_ATTR_AES_DECRYPT_ALLOWED = (1U << 2),			/**< Key can be used for AES decryption. */
	CCS_KSU_ATTR_AES_128BIT_KEY_ALLOWED = (1U << 3),		/**< Part of the key can be used for 128/192 bit AES operations. */
	CCS_KSU_ATTR_DEPRECATED_AES_XTS_ONLY = (1U << 4),		/**< No longer a valid attribute. */
	CCS_KSU_ATTR_SEND_KEY_ALLOWED = (1U << 5),				/**< SendKey operations are allowed for the key. */
	CCS_KSU_ATTR_KEY_ALLOWED_AS_DEC_KEK = (1U << 6),		/**< Unwrapping operations are allowed for the key. */
	CCS_KSU_ATTR_DECRYPT_LEGACY_KEY_ALLOWED = (1U << 7),	/**< DecryptLegacyKey operations are allowed for the key. */
	CCS_KSU_ATTR_KEY_ALLOWED_AS_ENC_KEK = (1U << 8),		/**< Wrapping operations are allowed for the key. */
	CCS_KSU_ATTR_SAVE_KEY_ALLOWED = (1U << 9),				/**< SaveKey operations are allowed for the key. */
	CCS_KSU_ATTR_KDF_KEY_ALLOWED = (1U << 10),				/**< KDFKey operations are allowed for the key. */
	CCS_KSU_ATTR_KDF_PCR_ALLOWED = (1U << 11),				/**< KDFPCR operations are allowed for the key. */
	CCS_KSU_ATTR_ECC_SIGN_ALLOWED = (1U << 12),				/**< ECC private key can be used for signing operations. */
	CCS_KSU_ATTR_ECDH_ALLOWED = (1U << 13),					/**< ECC private key can be used for ECDH operations. */
	CCS_KSU_ATTR_MUST_APPEND_PCR = (1U << 14),				/**< ECC signing operations must include a PCR value. */
	CCS_KSU_ATTR_KEY_SIZE_384 = (1U << 15),					/**< Key slot contains a 384-bit key. */
	CCS_KSU_ATTR_IS_EPHEMERAL_KEY = (1U << 16),				/**< Key is a temporary key. */
	CCS_KSU_ATTR_DERIVE_FW_KEY_ALLOWED = (1U << 17),		/**< DeriveECCKey and HMAC are allowed for the key. */
};


/* TODO:  Once the AES driver doesn't need to know about the KSU structures, the key and PCR slot
 * definitions should move to ccs_ksu.h. */
#ifndef HSP_KSU_HW_0100
/**
 * The structure of each key slot.  While the key slots are not directly accessible, the memory
 * structure is necessary for determining key slot addresses.
 */
struct ksu_key_slot {
	uint32_t key[0xc];		/**< Key data in the slot. */
	uint8_t key_pad[0x10];	/**< Padding for address alignment. */
	uint32_t attributes;	/**< Key attributes for the slot. */
	uint8_t attr_pad[0x3c];	/**< Padding for address alignment. */
};

/**
 * The structure for each PCR in KSU memory.
 */
struct ksu_pcr_slot {
	volatile uint32_t pcr[0xc];	/**< PCR value. */
	uint8_t pcr_pad[0x10];		/**< Padding for address alignment. */
};

#else
/**
 * Shack1 version of the key slot structure.
 */
struct ksu_key_slot {
	uint32_t key[0x8];		/**< Key data in the slot. */
	uint32_t attributes;	/**< Key attributes for the slot. */
	uint8_t attr_pad[0x1c];	/**< Padding for address alignment. */
};

/**
 * Shack1 version of the PCR structure.
 */
struct ksu_pcr_slot {
	volatile uint32_t pcr[0x8];	/**< PCR value. */
	uint8_t pcr_pad[0x20];		/**< Padding for address alignment. */
};

#endif

/**
 * Wrapper for dealing with both 256 and 384 bit ECC public keys.
 */
union ccs_ksu_ecc_public_key {
	SP_ECDSA_P256_PUBLIC p256;	/**< 256-bit ECC public key. */
	SP_ECDSA_P384_PUBLIC p384;	/**< 384-bit ECC public key. */
};

/**
 * Wrapper for dealing with both 256 and 384 bit ECDSA signatures.
 */
union ccs_ksu_ecc_signature {
	SP_ECDSA_P256_SIGNATURE p256;	/**< 256-bit ECDSA signature. */
	SP_ECDSA_P384_SIGNATURE p384;	/**< 384-bit ECDSA signature. */
};

/**
 * Driver interface for executing Complex Command Sequencer (CCS) actions with the Key Storage Unit
 * (KSU).
 */
struct ccs_ksu_interface {
	/**
	 * Indicate if a specified key slot number is valid for the KSU.
	 *
	 * @param ccs The CCS to query.
	 * @param key_slot Slot number to check.
	 *
	 * @return 0 if the key slot is valid or an error code.  CCS_KSU_UNSUPPORTED_KEY_SLOT will be
	 * returned if the key slot is not available in the KSU.
	 */
	int (*is_key_slot_valid) (const struct ccs_ksu_interface *ccs, uint8_t key_slot);

	/**
	 * Get the attributes currently assigned to a key in the KSU.
	 *
	 * @param ccs The CCS to query.
	 * @param key_slot Slot number for the key to query.
	 * @param key_attributes Output for the attributes assigned to the key.
	 *
	 * @return 0 if the attributes were retrieved successfully or an error code.
	 */
	int (*get_key_attributes) (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
		uint32_t *key_attributes);

	/**
	 * Load raw key data into a KSU key slot.
	 *
	 * @param ccs The CCS to use to load the key.
	 * @param key Buffer containing the key data. Only the first 256 bits will be used for 256-bit
	 * keys.
	 * @param key_slot Slot number the key should be loaded into.
	 * @param key_attributes Attributes to assign to the key slot.  This is a bitmask of attributes
	 * defined by {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully loaded or an error code.
	 */
	int (*set_key) (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key, uint8_t key_slot,
		uint32_t key_attributes);

#ifdef CCS_KSU_ENABLE_SEND_KEY
	/**
	 * Send a key stored in a key slot to some other memory location.
	 *
	 * @param ccs The CCS to use for sending.
	 * @param key_slot Slot number where the stored key should be retrieved from.
	 * @param dest_addr Destination address in hw where the key will be stored.
	 *
	 * @return 0 if the key was successfully sent or an error code.
	 */
	int (*send_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot, uint32_t dest_addr);
#endif

	/**
	 * Generate a random key and put it in a key slot.  This should not be used to generate ECC
	 * keys.
	 *
	 * @param ccs The CCS to use for key generation.
	 * @param key_slot Slot number for the generated key.
	 * @param key_attributes Attributes to assign to the generated key.  This is a bitmask of
	 * attributes defined by {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*generate_random_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
		uint32_t key_attributes);

	/**
	 * Derive a deterministic key from a source key.  The key derivation uses NIST 800-108.  This
	 * is not DRBG based, so should not be used to generate ECC keys.
	 *
	 * @param ccs The CCS to use to generate the new key.
	 * @param key_in Input key to the KDF.
	 * @param context A value to use as the Context of the NIST KDF.  Only 256 bits of this context
	 * will be used for 256-bit keys.  Key length is determined by the input attributes.  The Label
	 * is hard-coded within the CCS.
	 * @param key_slot Destination slot for the new key.
	 * @param key_attributes Attributes to apply to the generated key.  These attributes will be
	 * appended to the context value during the KDF.  This is a bitmask of attributes defined by
	 * {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*derive_key) (const struct ccs_ksu_interface *ccs, uint8_t key_in,
		const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes);

	/**
	 * Derive a deterministic key from a source key using a current PCR value as the KDF context.
	 * The key derivation uses NIST 800-108.  This is not DRBG based, so should not be used to
	 * generate ECC keys.
	 *
	 * @param ccs The CCS to use to generate the new key.
	 * @param key_in Input key to the KDF.
	 * @param pcr PCR number to use as the KDF context.
	 * @param key_slot Destination slot for the new key.
	 * @param key_attributes Attributes to apply to the generated key.  These attributes will be
	 * appended to the PCR value during the KDF.  This is a bitmask of attributes defined by
	 * {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*derive_key_using_pcr) (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t pcr,
		uint8_t key_slot, uint32_t key_attributes);

	/**
	 * Generate a random ECC key and put it in a key slot.
	 *
	 * @param ccs The CCS to use for key generation.
	 * @param key_slot Slot number for the generated ECC key.
	 * @param key_attributes Attributes to assign to the generated ECC key.  This is a bitmask of
	 * attributes defined by {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*generate_random_ecc_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
		uint32_t key_attributes);

	/**
	 * Derive a deterministic ECC key from a source key.  This key derivation uses a deterministic
	 * seed to a DRBG.
	 *
	 * @param ccs The CCS to use to generate the new key.
	 * @param key_in Input key to use as a seed for the DRBG.
	 * @param key_slot Destination slot for the new key.
	 * @param key_attributes Attributes to apply to the generated key.  These attributes will also
	 * be part of the DRBG seed.  This is a bitmask of attributes defined by
	 * {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*derive_ecc_key) (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t key_slot,
		uint32_t key_attributes);

	/**
	 * Derive a deterministic ECC key from a source key and provide that key to firmware.
	 *
	 * @param ccs The CCS to use to generate the new key.
	 * @param key_in Input key to use as a seed for the DRBG.
	 * @param key Output buffer for the derived key.  For a 256-bit key, only the first 256 bits
	 * will contain valid data and the remaining data should be ignored.  Key length is determined
	 * by the input attributes.
	 * @param key_attributes Attributes to apply to the generated key.  These attributes will also
	 * be part of the DRBG seed.  This is a bitmask of attributes defined by
	 * {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return 0 if the key was successfully generated or an error code.
	 */
	int (*derive_fw_ecc_key) (const struct ccs_ksu_interface *ccs, uint8_t key_in, SP_MSG_384 *key,
		uint32_t key_attributes);

	/**
	 * Export an existing ECC private key to firmware.  The private key will still be present within
	 * CCS.
	 *
	 * @param ccs The CCS that contains the ECC private key.
	 * @param key_slot Key slot for the ECC private key.
	 * @param key Output buffer for the ECC private key.  For 256-bit ECC keys, only first 256 bits
	 * contain valid data, with the remaining bytes being zeros.
	 * @param key_attributes Output buffer for the attributes of the private key corresponding to
	 * the public key.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the public key by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the private key was successfully retrieved or an error code.  If it's not
	 * possible for the CCS to export the private key, CCS_KSU_PRIVATE_KEY_PROTECTED will be
	 * returned.
	 */
	int (*export_fw_ecc_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
		SP_MSG_384 *key, uint32_t *key_attributes);

	/**
	 * Get the ECC public key for a private key.
	 *
	 * @param ccs The CCS that contains the ECC private key.
	 * @param key_slot Key slot for the ECC private key,
	 * @param public_key Output buffer for the ECC public key.  For 256-bit ECC keys, only the
	 * first 512 bits contain valid data, with the remaining bytes being zeros.  The returned key
	 * should be processed using the SP_ECDSA_P256_PUBLIC structure.
	 * @param key_attributes Output buffer for the attributes of the private key corresponding to
	 * the public key.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the public key by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the public key was successfully retrieved or an error code.
	 */
	int (*get_ecc_public_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
		SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes);

	/**
	 * Endorse an ECC key by signing the public key with another ECC key.  The signature will be
	 * over the concatenation of (public key || key slot attributes || PCR value || input data).
	 *
	 * @param ccs The CCS that contains the ECC keys.
	 * @param signing_key Key slot for the ECC key to use for signing.
	 * @param pcr Index for a PCR to include in the signing operation.
	 * @param key_slot Key slot for the ECC key that will be signed.
	 * @param sign_data Input data that will be added to the signature.
	 * @param public_key Output buffer for the signed public key.  For 256-bit ECC keys, only the
	 * first 512 bits contain valid data, with the remaining bytes being zeros.  The returned key
	 * should be processed using the SP_ECDSA_P256_PUBLIC structure.
	 * @param signature Output buffer for the signature generated for the ECC public key.  For
	 * 256-bit ECC keys, only the first 512 bits contain valid data, with the remaining bytes being
	 * zeros.  The returned signature should be processed using the SP_ECDSA_P256_SIGNATURE
	 * structure.
	 * @param public_key_attributes Output buffer for the attributes of the private key
	 * corresponding to the public key.  This can be null if the attributes are not required.  The
	 * attributes can be used to determine the length of the public key by inspecting the
	 * CCS_KSU_ATTR_KEY_SIZE_384 bit.
	 * @param signing_key_attributes Output buffer for the attributes of the private key using to
	 * certify the public key.  This can be null if the attributes are not required.  The
	 * attributes can be used to determine the length of the signature by inspecting the
	 * CCS_KSU_ATTR_KEY_SIZE_384 bit.
	 *
	 * @return 0 if the public key was successfully signed or an error code.
	 */
	int (*certify_ecc_public_key) (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
		uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data,
		SP_ECDSA_P384_PUBLIC *public_key, SP_ECDSA_P384_SIGNATURE *signature,
		uint32_t *public_key_attributes, uint32_t *signing_key_attributes);

	/**
	 * Initiates an ECDH key exchange operation.
	 *
	 * @param ccs The CCS instance to use for the operation.
	 * @param key_in The key slot containing the private key.
	 * @param key_out The key slot where the derived shared key will be stored.
	 * @param partner_public_key_and_hash The public key and optional hash of the other party.
	 * @param input_len Length of the input data, which should be the size of the public key plus the size of the hash
	 * @param key_attributes Attributes for the key exchange operation.
	 *
	 * @return 0 if the key exchange was successful, or an error code otherwise.
	 */
	int (*ecdh_key_exchange) (const struct ccs_ksu_interface *ccs, uint8_t key_in, uint8_t key_out,
		const uint8_t *partner_public_key_and_hash, size_t input_len, uint32_t key_attributes);

	/**
	 * Generates an ECDSA signature for a specified input digest.
	 *
	 * This call does not comply with FIPS requirements for ECDSA signature generation.
	 *
	 * @param ccs The CCS that will generate the signature.
	 * @param signing_key Key slot for the ECC key to use for signing.
	 * @param digest The digest that will be wrapped in a signature.  When using a 256-bit key, only
	 * the first 256 bits of data are used.  Key length will be determined using the attributes on
	 * key slot.
	 * @param rng An optional random number generator to use for generating the random 'r' value in
	 * the signature.  If this is null, CCS will use the hardware RNG to generate this value.
	 * @param signature Output buffer for the generated signature.  For 256-bit keys, only the first
	 * 512 bits are accessed, which is equivalent to using a SP_ECDSA_P256_SIGNATURE structure.
	 * @param key_attributes Output buffer for the attributes of the private key used to generate
	 * the signature.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the signature by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the ECDSA signature was successfully generated or an error code.
	 */
	int (*ecc_sign) (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
		const SP_MSG_384 *digest, const struct rng_engine *rng, SP_ECDSA_P384_SIGNATURE *signature,
		uint32_t *key_attributes);

	/**
	 * Generates an ECDSA signature for a specified input message.
	 *
	 * This call complies with FIPS requirements for ECDSA signature generation.
	 *
	 * @param ccs The CCS that will generate the signature.
	 * @param signing_key Key slot for the ECC key to use for signing.
	 * @param message The complete message that will be signed.
	 * @param length Length of the message to sign.
	 * @param hash Hash engine that will be used to calculate the message digest.
	 * @param hash_algo Hash algorithm that should be used for the message digest.
	 * @param signature Output buffer for the generated signature.  For 256-bit keys, only the first
	 * 512 bits are accessed, which is equivalent to using a SP_ECDSA_P256_SIGNATURE structure.
	 * @param key_attributes Output buffer for the attributes of the private key used to generate
	 * the signature.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the signature by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the ECDSA signature was successfully generated or an error code.
	 */
	int (*ecdsa_sign_message) (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
		const uint8_t *message, size_t length, const struct hash_engine *hash,
		enum hash_type hash_algo, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);

	/**
	 * Generates an ECDSA signature for an active hash context.
	 *
	 * The hash context will remain active after signature generation, allowing additional updates
	 * to be made.  HSP HS-SHA supports this behavior, but other hash implementations may not.  Most
	 * scenarios should use ccs_ksu_interface.sign_hash_and_finish instead.
	 *
	 * This call complies with FIPS requirements for ECDSA signature generation.
	 *
	 * @param ccs The CCS that will generate the signature.
	 * @param signing_key Key slot for the ECC key to use for signing.
	 * @param hash Hash engine that contains the digest to sign.
	 * @param signature Output buffer for the generated signature.  For 256-bit keys, only the first
	 * 512 bits are accessed, which is equivalent to using a SP_ECDSA_P256_SIGNATURE structure.
	 * @param key_attributes Output buffer for the attributes of the private key used to generate
	 * the signature.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the signature by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the ECDSA signature was successfully generated or an error code.
	 */
	int (*ecdsa_sign_hash) (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
		const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature,
		uint32_t *key_attributes);

	/**
	 * Generates an ECDSA signature for an active hash context.
	 *
	 * The hash context will be finished as part of signature generation.  No additional updates can
	 * be made to the hash context, regardless of whether the signature generation was successful or
	 * not.
	 *
	 * This call complies with FIPS requirements for ECDSA signature generation.
	 *
	 * @param ccs The CCS that will generate the signature.
	 * @param signing_key Key slot for the ECC key to use for signing.
	 * @param hash Hash engine that contains the digest to sign.
	 * @param signature Output buffer for the generated signature.  For 256-bit keys, only the first
	 * 512 bits are accessed, which is equivalent to using a SP_ECDSA_P256_SIGNATURE structure.
	 * @param key_attributes Output buffer for the attributes of the private key used to generate
	 * the signature.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of the signature by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the ECDSA signature was successfully generated or an error code.
	 */
	int (*ecdsa_sign_hash_and_finish) (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
		const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature,
		uint32_t *key_attributes);

	/**
	 * Wrap a key in memory using a KEK from the KSU.
	 *
	 * @param ccs The CCS to use for key wrapping.
	 * @param kek_slot Slot number for the encryption key to use for wrapping.
	 * @param key Buffer containing the key to wrap.  When using a 256-bit key, only
	 * the first 256 bits of data are used.  Key length will be determined using the attributes on
	 * key slot.
	 * @param wrapped_key Output buffer for the wrapped key.  This must be located within HSP shared
	 * SRAM.  When using a 256-bit key, only the first 384 bits of data are filled.
	 * @param key_attributes Attributes to assign to the wrapped key.  This is a bitmask of
	 * attributes defined by {@link enum ccs_ksu_key_attributes}.
	 *
	 * @return Length of the wrapped key or an error code.  Use ROT_IS_ERROR to check the return
	 * value.
	 */
	int (*wrap_key_buffer) (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
		const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes);

	/**
	 * Unwrap an encrypted key blob into a key slot.
	 *
	 * @param ccs The CCS to use for unwrapping.
	 * @param kek_slot Slot number for the encryption key to use for unwrapping.
	 * @param wrapped_key Encrypted key that will be unwrapped.  When using a 256-bit key, only the
	 * first 384 bits of data are used.
	 * @param key_slot Slot number where the unwrapped key should be stored.
	 *
	 * @return 0 if the key was successfully unwrapped or an error code.
	 */
	int (*unwrap_key) (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
		const SP_MSG_512 *wrapped_key, uint8_t key_slot);

	/**
	 * Write a key slot to fuses.  Only key slots backed by fuses can be stored and only if the
	 * fuses have not already been programmed.
	 *
	 * @param ccs The CCS with the key to store.
	 * @param key_slot Slot number of the key that should be stored in fuses.
	 *
	 * @return 0 if the key was successfully stored or an error code.
	 */
	int (*burn_key) (const struct ccs_ksu_interface *ccs, uint8_t key_slot);

	/**
	 * Reset a PCR value to its initial state.
	 *
	 * @param ccs The CCS for the PCR to reset.
	 * @param pcr Index of the PCR to reset.
	 *
	 * @return 0 if the PCR was reset successfully or an error code.
	 */
	int (*reset_pcr) (const struct ccs_ksu_interface *ccs, uint8_t pcr);

	/**
	 * Extend a PCR value with a new measurement.
	 *
	 * @param ccs The CSS for the PCR to extend.
	 * @param pcr Index of the PCR to extend.
	 * @param digest The digest that should be used to extend the PCR value. Only the first 256 bits
	 * will be used for 256-bit PCR slots.
	 *
	 * @return 0 if the PCR was extended successfully or an error code.
	 */
	int (*extend_pcr) (const struct ccs_ksu_interface *ccs, uint8_t pcr, const SP_MSG_384 *digest);

	/**
	 * Get the current value of a PCR.
	 *
	 * @param ccs THe CCS for the PCR to query.
	 * @param pcr Index of the PCR to report.
	 * @param value Output for the current PCR value.  Only the first 256 bits will be used for
	 * 256-bit PCR slots.
	 *
	 * @return 0 if the PCR was retrieved successfully or an error code.
	 */
	int (*get_pcr_value) (const struct ccs_ksu_interface *ccs, uint8_t pcr, SP_MSG_384 *value);

	/**
	 * Calculate the HMAC for a input buffer using a key from the KSU.  If the key used to generate
	 * the HMAC is a 256-bit key, the result will be an HMAC-SHA256.  If the key is 384 bits, the
	 * result will be an HMAC-SHA384.
	 *
	 * @param ccs The CCS containing the HMAC key.
	 * @param key_slot Slot number for the HMAC key.
	 * @param data Buffer containing the data to HMAC.  This buffer must be contained within HSP
	 * shared SRAM or be small enough to fit in the memory used by the CCS command buffer.
	 * @param length Length of the data buffer.
	 * @param hmac Output for the calculated HMAC.  For a SHA256 HMAC, only the first 256 bits will
	 * be valid.
	 * @param key_attributes Output buffer for the attributes of the private key used to generate
	 * the HMAC.  This can be null if the attributes are not required.  The attributes can be
	 * used to determine the length of output HMAC data by inspecting the CCS_KSU_ATTR_KEY_SIZE_384
	 * bit.
	 *
	 * @return 0 if the HMAC was calculated successfully or an error code.
	 */
	int (*hmac) (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
		size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes);
};


#ifdef CCS_KSU_ENABLE_FIPS_CMVP_TESTING
/* Flag for PCT fault-injection during key generation for FIPS CMVP certification tests. */
extern bool ccs_ksu_interface_fail_ecdsa_pct;
#endif

int ccs_ksu_interface_generate_random_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_slot,
	uint32_t key_attributes, union ccs_ksu_ecc_public_key *public_key);
int ccs_ksu_interface_derive_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_in, uint8_t key_slot,
	uint32_t key_attributes, union ccs_ksu_ecc_public_key *public_key);
int ccs_ksu_interface_derive_fw_ecdsa_key (const struct ccs_ksu_interface *ccs,
	const struct ecc_hw *ecc, const struct hash_engine *hash, uint8_t key_in, SP_MSG_384 *key,
	uint32_t key_attributes, struct ecc_point_public_key *public_key);

int ccs_ksu_interface_zeroize_ksu (const struct ccs_ksu_interface *ccs);


#define	CCS_KSU_ERROR(code)		ROT_ERROR (MSFT_MODULE_CCS_KSU, code)

/**
 * Error codes that can be generated by the CCS.
 */
enum {
	CCS_KSU_INVALID_ARGUMENT = CCS_KSU_ERROR (0x00),			/**< Input parameter is null or not valid. */
	CCS_KSU_NO_MEMORY = CCS_KSU_ERROR (0x01),					/**< Memory allocation failed. */
	CCS_KSU_SET_KEY_FAILED = CCS_KSU_ERROR (0x02),				/**< Failed to assign a key to a KSU slot. */
	CCS_KSU_RANDOM_KEY_FAILED = CCS_KSU_ERROR (0x03),			/**< Failed to generate a random key. */
	CCS_KSU_DERIVE_KEY_FAILED = CCS_KSU_ERROR (0x04),			/**< Failed to derive a new key. */
	CCS_KSU_DERIVE_KEY_PCR_FAILED = CCS_KSU_ERROR (0x05),		/**< Failed to derive a new key using a PCR value. */
	CCS_KSU_RANDOM_ECC_FAILED = CCS_KSU_ERROR (0x06),			/**< Failed to generate a random ECC key. */
	CCS_KSU_DERIVE_ECC_FAILED = CCS_KSU_ERROR (0x07),			/**< Failed to derive a new ECC key. */
	CCS_KSU_DERIVE_FW_ECC_FAILED = CCS_KSU_ERROR (0x08),		/**< Failed to derive a firmware ECC key. */
	CCS_KSU_ECC_PUBLIC_FAILED = CCS_KSU_ERROR (0x09),			/**< Failed to get an ECC public key. */
	CCS_KSU_CERTIFY_ECC_FAILED = CCS_KSU_ERROR (0x0a),			/**< Failed to sign an ECC public key. */
	CCS_KSU_ECC_SIGN_FAILED = CCS_KSU_ERROR (0x0b),				/**< Failed to generate an ECC signature. */
	CCS_KSU_WRAP_BUFFER_FAILED = CCS_KSU_ERROR (0x0c),			/**< Failed to wrap a key buffer in memory. */
	CCS_KSU_UNWRAP_FAILED = CCS_KSU_ERROR (0x0d),				/**< Failed to unwrap a encrypted key. */
	CCS_KSU_BURN_KEY_FAILED = CCS_KSU_ERROR (0x0e),				/**< Failed to burn a key to fuses. */
	CCS_KSU_RESET_PCR_FAILED = CCS_KSU_ERROR (0x0f),			/**< Failed to reset a PCR value. */
	CCS_KSU_EXTEND_PCR_FAILED = CCS_KSU_ERROR (0x10),			/**< Failed to extend a PCR value. */
	CCS_KSU_HMAC_FAILED = CCS_KSU_ERROR (0x11),					/**< Failed to calculate an HMAC. */
	CCS_KSU_UNSUPPORTED_KEY_SLOT = CCS_KSU_ERROR (0x12),		/**< The key slot is not available. */
	CCS_KSU_UNSUPPORTED_PCR_SLOT = CCS_KSU_ERROR (0x13),		/**< The PCR slot is not available. */
	CCS_KSU_INVALID_ADDRESS = CCS_KSU_ERROR (0x14),				/**< A buffer is not in HSP shared SRAM. */
	CCS_KSU_CMD_NOT_STARTED = CCS_KSU_ERROR (0x15),				/**< The CCS did not accept a submitted command. */
	CCS_KSU_ADDRESS_NOT_ALIGNED = CCS_KSU_ERROR (0x16),			/**< A buffer address is not word aligned. */
	CCS_KSU_HMAC_SELF_TEST_FAILED = CCS_KSU_ERROR (0x17),		/**< An HMAC self-test of the CCS HW failed. */
	CCS_KSU_GET_ATTR_FAILED = CCS_KSU_ERROR (0x18),				/**< Failed to get attributes for a key. */
	CCS_KSU_UNSUPPORTED_CMD = CCS_KSU_ERROR (0x19),				/**< Command is unsupported. */
	CCS_KSU_UNSUPPORTED_KEY_ATTR = CCS_KSU_ERROR (0x1a),		/**< Key attribute unsupported. */
	CCS_KSU_KDF_SELF_TEST_FAILED = CCS_KSU_ERROR (0x1b),		/**< A KDF self-test of the CCS HW failed. */
	CCS_KSU_ECDSA_SIGN_MSG_FAILED = CCS_KSU_ERROR (0x1c),		/**< Failed to generate an ECDSA signature for a message. */
	CCS_KSU_ECDSA_SIGN_HASH_FAILED = CCS_KSU_ERROR (0x1d),		/**< Failed to generate an ECDSA signature for a hash context. */
	CCS_KSU_NO_KEY_SLOT_AVAILABLE = CCS_KSU_ERROR (0x1e),		/**< There is no key slot available to use for the operation. */
	CCS_KSU_SRC_KEY_TOO_SMALL = CCS_KSU_ERROR (0x1f),			/**< The source key is no long enough to derive the desired key. */
	CCS_KSU_GET_PCR_FAILED = CCS_KSU_ERROR (0x20),				/**< Failed to read a PCR value. */
	CCS_KSU_SIGN_RNG_UNSUPPORTED = CCS_KSU_ERROR (0x21),		/**< The CCS does not support ECC signing with an external RNG. */
	CCS_KSU_EXPORT_FW_ECC_FAILED = CCS_KSU_ERROR (0x22),		/**< Failed to export an ECC private key. */
	CCS_KSU_PRIVATE_KEY_PROTECTED = CCS_KSU_ERROR (0x23),		/**< The CCS does not support exporting a private key. */
	CCS_KSU_PCT_FAILURE = CCS_KSU_ERROR (0x24),					/**< Failed the pairwise consistency test. */
	CCS_KSU_SEND_KEY_FAILED = CCS_KSU_ERROR (0x25),				/**< Failed to send a KSU key. */
	CCS_KSU_ECDH_KEY_EXCHANGE_FAILED = CCS_KSU_ERROR (0x26),	/**< Failed to perform ECDH key exchange. */
	CCS_KSU_CMD_EXE_TIMEOUT = CCS_KSU_ERROR (0x27),				/**< CCS command execution timeout. */

	/* Error codes >0x80 are reserved for reporting bits from the HW status register. */
};

/**
 * An error has occurred with the hardware block.  The error code represents the status register
 * output.
 */
#define	CCS_KSU_HW_ERROR(reg)	CCS_KSU_ERROR (0x80 | ((reg & 0x38) >> 3))


#endif	/* CCS_KSU_INTERFACE_H_ */
