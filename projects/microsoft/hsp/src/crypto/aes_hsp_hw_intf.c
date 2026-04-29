// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "aes_hsp_hw_intf.h"
#include "common/buffer_util.h"



/**
 * Set an AES key to use for future encryption or decryption options.
 *
 * No validation is done on any of the arguments.  The key length and validity must be checked by
 * the calling implementation.
 *
 * @param ccs The CCS that manages the AES key.
 * @param key The AES key.
 * @param length Length of the AES key.
 * @param key_slot ID for the KSU key slot where the key will be stored.
 * @param extra_attributes Additional attributes that will be applied to the AES.  Every key will
 * always be given AESEncryptAllowed and AESDecryptAllowed attributes.
 *
 * @return 0 if the key was set successfully or an error code.
 */
int aes_hsp_hw_intf_set_key (const struct ccs_ksu_interface *ccs, const uint8_t *key, size_t length,
	uint8_t key_slot, uint32_t extra_attributes)
{
	SP_MSG_384 tmp_key = {0};
	int status;

	memcpy (tmp_key.AsBytes, key, length);

	status = ccs->set_key (ccs, &tmp_key, key_slot,
		CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED | CCS_KSU_ATTR_AES_DECRYPT_ALLOWED | extra_attributes);

	/* Clear the temporary copy of the key from FW memory. */
	buffer_zeroize (tmp_key.AsBytes, SP_MSG_384_SIZE);

	return status;
}

/**
 * Clear an AES key from the KSU.  Future encryption or decryption operations will need to set a new
 * key.
 *
 * No validation is done on the arguments.
 *
 * @param ccs The CCS that manages the AES key.
 * @param key_slot ID for the KSU key slot for the key to clear.
 *
 * @return 0 if the key was cleared successfully or an error code.
 */
int aes_hsp_hw_intf_clear_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot)
{
	SP_MSG_384 zero = {0};

	return ccs->set_key (ccs, &zero, key_slot, 0);
}

/**
 * Encrypt data using a specified AES mode and key.
 *
 * No validation is done on the arguments.
 *
 * @param aes The HSP AES driver to use for encryption.
 * @param ccs The CSS that contains the AES encryption key.
 * @param mode AES cipher mode to use for encryption.
 * @param key_slot KSU key slot that contains the encryption key.
 * @param extra_attributes Additional key attributes that are required for the operation.  Every key
 * will need to have AESEncryptAllowed.
 * @param no_key_error Error code to return when there is no encryption key available in the KSU.
 * @param iv Initial vector for the encryption operation.  This will be ignored for ECB operations
 * and can be null in that case.
 * @param plaintext Buffer containing the data to encrypt.
 * @param length Length of the data.
 * @param ciphertext Output buffer for the encrypted data.  It can be the same as the plaintext
 * buffer.
 * @param out_length Length of the output buffer.  It must be at least the same size as the
 * plaintext data.
 * @param out_iv Optional output buffer for the IV that would be used to continue the
 * encryption operation with additional data.  This allows encryption to be executed without
 * having all the data available in memory at the same time.  This will always be ignored for
 * ECB operations.
 *
 * @return 0 if encryption was successful or an error code.
 */
int aes_hsp_hw_intf_encrypt (const struct hsp_aes *aes, const struct ccs_ksu_interface *ccs,
	enum hsp_aes_mode mode, uint8_t key_slot, uint32_t extra_attributes, int no_key_error,
	const uint8_t *plaintext, size_t length, const SP_MSG_128 *iv, uint8_t *ciphertext,
	size_t out_length, SP_MSG_128 *out_iv)
{
	const uint32_t required_attr = CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED | extra_attributes;
	uint32_t key_attributes;
	int status;

	status = ccs->get_key_attributes (ccs, key_slot, &key_attributes);
	if (status != 0) {
		return status;
	}

	if ((key_attributes & required_attr) != required_attr) {
		/* The key slot cannot support the requested encryption. */
		return no_key_error;
	}

	return aes->encrypt (aes, mode, key_slot, iv, plaintext, length, ciphertext, out_length,
		out_iv);
}

/**
 * Decrypt data using a specified AES mode and key.
 *
 * No validation is done on the arguments.
 *
 * @param aes The HSP AES driver to use for decryption.
 * @param ccs The CSS that contains the AES decryption key.
 * @param mode AES cipher mode to use for decryption.
 * @param key_slot KSU key slot that contains the decryption key.
 * @param extra_attributes Additional key attributes that are required for the operation.  Every key
 * will need to have AESDecryptAllowed.
 * @param no_key_error Error code to return when there is no decryption key available in the KSU.
 * @param iv Initial vector for the decryption operation.  This will be ignored for ECB operations
 * and can be null in that case.
 * @param ciphertext Buffer containing the data to decrypt.
 * @param length Length of the data.
 * @param plaintext Output buffer for the decrypted data.  It can be the same as the ciphertext
 * buffer.
 * @param out_length Length of the output buffer.  It must be at least the same size as the
 * ciphertext data.
 * @param out_iv Optional output buffer for the IV that would be used to continue the
 * decryption operation with additional data.  This allows decryption to be executed without
 * having all the data available in memory at the same time.  This will always be ignored for
 * ECB operations.
 *
 * @return 0 if decryption was successful or an error code.
 */
int aes_hsp_hw_intf_decrypt (const struct hsp_aes *aes, const struct ccs_ksu_interface *ccs,
	enum hsp_aes_mode mode, uint8_t key_slot, uint32_t extra_attributes, int no_key_error,
	const uint8_t *ciphertext, size_t length, const SP_MSG_128 *iv, uint8_t *plaintext,
	size_t out_length, SP_MSG_128 *out_iv)
{
	const uint32_t required_attr = CCS_KSU_ATTR_AES_DECRYPT_ALLOWED | extra_attributes;
	uint32_t key_attributes;
	int status;

	status = ccs->get_key_attributes (ccs, key_slot, &key_attributes);
	if (status != 0) {
		return status;
	}

	if ((key_attributes & required_attr) != required_attr) {
		/* The key slot cannot support the requested decryption. */
		return no_key_error;
	}

	return aes->decrypt (aes, mode, key_slot, iv, ciphertext, length, plaintext, out_length,
		out_iv);
}
