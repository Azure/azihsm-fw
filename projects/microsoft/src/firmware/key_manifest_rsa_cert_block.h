// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_RSA_CERT_BLOCK_H_
#define KEY_MANIFEST_RSA_CERT_BLOCK_H_

#include <stddef.h>
#include <stdint.h>
#include "asn1/x509.h"
#include "crypto/rsa.h"
#include "firmware/cert_device_hw.h"
#include "firmware/key_manifest.h"
#include "flash/flash.h"


/**
 * Signature of the certificate block header.
 */
#define	KEY_MANIFEST_RSA_CERT_BLOCK_SIGNATURE	('c' | ('e' << 8) | ('r' << 16) | ('t' << 24))

#pragma pack(push,1)
/**
 * Header information on the certificate block.
 */
struct key_manifest_rsa_cert_block_header {
	uint32_t signature;			/**< Marker for the certificate block: 'cert'. */
	uint16_t major_version;		/**< Major version number of the header. */
	uint16_t minor_version;		/**< Minor version number of the header. */
	uint32_t header_length;		/**< Length of the certificate block header. */
	uint32_t flags;				/**< Header flags.  Reserved. */
	uint32_t build_number;		/**< Anti-rollback identifier. */
	uint32_t image_length;		/**< Total length of the firmware image, including certificate block. */
	uint32_t cert_count;		/**< Number of certificates in the chain. */
	uint32_t cert_table_length;	/**< Total length of the certificate chain. */
};

/**
 * List of hashes for trusted root keys.
 */
struct key_manifest_rsa_cert_block_root_keys {
	uint8_t key_hash[4][SHA256_HASH_LENGTH];
};

#pragma pack(pop)

/**
 * Key manifest that uses an X.509 certificate chain to validate the application key.  The
 * certificates must be for RSA 2k, 3k, or 4k keys.
 */
struct key_manifest_rsa_cert_block {
	struct key_manifest base;	/**< Base key manifest instance. */

	/**
	 * Calculate the SHA-256 hash of the entire certificate block.
	 *
	 * @param manifest The manifest to hash.
	 * @param hash Hash engine to use for the calculation.
	 * @param hash_out Output buffer for the hash.  This must be at least SHA256_HASH_LENGTH bytes.
	 * @param hash_length Size of the hash output buffer.
	 *
	 * @return 0 if the hash was calculated successfully or an error code.
	 */
	int (*get_hash) (const struct key_manifest_rsa_cert_block *manifest,
		const struct hash_engine *hash, uint8_t *hash_out, size_t hash_length);

	/**
	 * Get the root key hash table for the certificate block.
	 *
	 * @param manifest The manifest to query.
	 * @param root_keys Output for the root key hash table.
	 *
	 * @return 0 if the root keys were retrieved successfully or an error code.
	 */
	int (*get_root_keys) (const struct key_manifest_rsa_cert_block *manifest,
		struct key_manifest_rsa_cert_block_root_keys *root_keys);

	struct cert_device_hw *hw;					/**< Hardware management of valid certificates. */
	const struct x509_engine *x509;				/**< X.509 engine to use for certificate validation. */
	const struct rsa_engine *rsa;				/**< RSA engine to use for key operations. */
	const struct flash *flash;					/**< Flash where the manifest is stored. */
	uintptr_t address;							/**< Base address of the manifest. */
	uint32_t table_length;						/**< Length of the certificate table. */
	struct key_manifest_public_key root_key;	/**< Public key used for manifest verification. */
	struct key_manifest_public_key image_key;	/**< Public key for application verification. */
	struct rsa_public_key root_key_data;		/**< Key data for the root public key. */
	struct rsa_public_key image_key_data;		/**< Key data for the application public key. */
	uint16_t revocation_id;						/**< ID for revoking the certificate block. */
};


int key_manifest_rsa_cert_block_init (struct key_manifest_rsa_cert_block *manifest,
	struct cert_device_hw *hw, const struct flash *flash, uint32_t base_addr,
	const struct x509_engine *x509, const struct rsa_engine *rsa);
int key_manifest_rsa_cert_block_init_from_memory (struct key_manifest_rsa_cert_block *manifest,
	struct cert_device_hw *hw, const uint8_t *cert, const struct x509_engine *x509,
	const struct rsa_engine *rsa);
void key_manifest_rsa_cert_block_release (struct key_manifest_rsa_cert_block *manifest);

int key_manifest_rsa_cert_block_get_length (const struct key_manifest_rsa_cert_block *manifest,
	size_t *length);


#endif	/* KEY_MANIFEST_RSA_CERT_BLOCK_H_ */
