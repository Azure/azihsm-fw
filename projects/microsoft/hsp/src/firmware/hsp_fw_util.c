// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_fw_util.h"
#include "common/buffer_util.h"
#include "crypto/kat/ecc_kat_vectors.h"


/**
 * Convert an ECC public key present within a firmware image metadata into a structure that can be
 * used for verification.
 *
 * @param pub_in Input format from the firmware image.
 * @param pub_out Output format for use in firmware.
 */
void hsp_fw_load_public_key (const SP_ECDSA_P384_PUBLIC *pub_in,
	struct ecc_point_public_key *pub_out)
{
	if (pub_in && pub_out) {
		memcpy (pub_out->x, pub_in->Parts.X.AsBytes, SP_MSG_384_SIZE);
		memcpy (pub_out->y, pub_in->Parts.Y.AsBytes, SP_MSG_384_SIZE);
		pub_out->key_length = SP_MSG_384_SIZE;
	}
}

/**
 * Convert an ECDSA signature present within a firmware image metadata into a structure that can be
 * used for verification.
 *
 * @param sig_in Input format from the firmware image.
 * @param sig_out Output format for use in firmware.
 */
void hsp_fw_load_signature (const SP_ECDSA_P384_SIGNATURE *sig_in,
	struct ecc_ecdsa_signature *sig_out)
{
	if (sig_in && sig_out) {
		memcpy (sig_out->r, sig_in->Parts.R.AsBytes, SP_MSG_384_SIZE);
		memcpy (sig_out->s, sig_in->Parts.S.AsBytes, SP_MSG_384_SIZE);
		sig_out->length = SP_MSG_384_SIZE;
	}
}

/**
 * Verify integrity of a signed image through signature and, optionally, hash validation.
 *
 * @param pka The PKA engine to use to verify the image signature.
 * @param hash A hash engine to use for signature verification.
 * @param signed_data The signed header for the image.
 * @param signed_length Length of the signed header.
 * @param signature Signature to verify the signed header.
 * @param key Public key to use for signature verification.
 * @param hashed_data The image data to verify.  Set this to null to only verify the signed header.
 * @param hashed_length Length of the image data.  This will be 0 when there is no data to verify.
 * @param expected The expected digest for the data.
 * @param error_code The error code to generate if the image data does not match the expected hash.
 *
 * @return 0 if the image is valid or an error code.
 */
int hsp_fw_verify_signed_image (const struct ecc_hw *pka, const struct hash_engine *hash,
	const uint8_t *signed_data, size_t signed_length, const SP_ECDSA_P384_SIGNATURE *signature,
	const struct ecc_point_public_key *key, const uint8_t *hashed_data, size_t hashed_length,
	const uint8_t *expected, int error_code)
{
	uint8_t digest[SHA384_HASH_LENGTH];
	struct ecc_ecdsa_signature tmp_sig;
	int status;

	if ((pka == NULL) || (hash == NULL) || (signed_data == NULL) || (signature == NULL) ||
		(key == NULL)) {
		return HSP_FW_UTIL_INVALID_ARGUMENT;
	}

	status = hash->calculate_sha384 (hash, signed_data, signed_length, digest, SHA384_HASH_LENGTH);
	if (status != 0) {
		return status;
	}

	hsp_fw_load_signature (signature, &tmp_sig);
	status = pka->ecdsa_verify (pka, key, &tmp_sig, digest, SHA384_HASH_LENGTH);
	if (status != 0) {
		return status;
	}

	if (hashed_data != NULL) {
		status = hash->calculate_sha384 (hash, hashed_data, hashed_length, digest,
			SHA384_HASH_LENGTH);
		if (status != 0) {
			return status;
		}

		status = buffer_compare (digest, expected, SHA384_HASH_LENGTH);
		if (status != 0) {
			status = error_code;
		}
	}

	return status;
}

/**
 * Run a known-answer test (KAT) against the ECDSA verification handler used to verify signed
 * headers in firmware and manifest images.
 *
 * This test uses a P-384 public key with a SHA2-384 signature.
 *
 * @param pka The PKA engine that to use during the self-test.  This must be the same instance that
 * will be used to verify header signatures.
 * @param hash The hash engine to use during the self-test.  This must be the same instance that
 * will be used during signature verification.
 *
 * @return 0 if the self test passed or an error code.
 */
int hsp_fw_run_self_test_verify_signed_image (const struct ecc_hw *pka,
	const struct hash_engine *hash)
{
	/* This is the same signature as ECC_KAT_VECTORS_P384_SHA384_ECDSA_SIGNATURE, just converted to
	 * a SP_ECDSA_P384_SIGNATURE structure. */
	const SP_ECDSA_P384_SIGNATURE signature = {
		.AsBytes = {
			0x32, 0x13, 0xe2, 0x75, 0x9d, 0xa9, 0xe4, 0x0a,
			0x3d, 0x4f, 0x99, 0xa3, 0xe6, 0x1c, 0xad, 0x34,
			0xdd, 0xb6, 0xa5, 0xaf, 0x1b, 0x3a, 0x53, 0xa6,
			0xbf, 0x69, 0xe3, 0xf8, 0x2a, 0x40, 0x67, 0x8e,
			0x32, 0xf4, 0xc7, 0xdc, 0x2b, 0x74, 0xe3, 0x91,
			0x7e, 0x6d, 0x38, 0x9c, 0xfe, 0x76, 0x89, 0xb3,
			0x0d, 0x37, 0xf0, 0x01, 0xb9, 0x99, 0x99, 0x9d,
			0x25, 0xe5, 0xa1, 0xec, 0x16, 0x15, 0xa5, 0xf4,
			0x20, 0x5c, 0xde, 0xc3, 0x01, 0x8a, 0xc1, 0x58,
			0x99, 0x2d, 0x93, 0x88, 0x01, 0xda, 0x29, 0x76,
			0x69, 0x26, 0xdc, 0x4d, 0x07, 0xd3, 0x7f, 0x9f,
			0x4d, 0x04, 0xf4, 0x39, 0xdb, 0x91, 0xdb, 0x87
		}
	};
	int status;

	status = hsp_fw_verify_signed_image (pka, hash, ECC_KAT_VECTORS_ECDSA_SIGNED,
		ECC_KAT_VECTORS_ECDSA_SIGNED_LEN, &signature, &ECC_KAT_VECTORS_P384_ECC_PUBLIC, NULL, 0,
		NULL, -1);
	if (status == ECC_HW_ECDSA_BAD_SIGNATURE) {
		status = ECC_HW_SELF_TEST_FAILED;
	}

	return status;
}
