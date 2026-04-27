// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_RSA2K_H_
#define KEY_MANIFEST_RSA2K_H_

#include <stdbool.h>
#include "crypto/hash.h"
#include "crypto/rsa.h"
#include "firmware/cert_device_hw.h"
#include "firmware/key_manifest.h"
#include "flash/flash.h"


/**
 * The length of the RSA keys contained in the manifest.
 */
#define	MANIFEST_RSA_KEY_SIZE		RSA_KEY_LENGTH_2K

#pragma pack(push,1)
/**
 * The information stored for each public key in the manifest.
 */
struct cert_public_key {
	uint8_t modulus[MANIFEST_RSA_KEY_SIZE];	/**< The modulus portion of the public key. */
	uint32_t exponent;						/**< The exponent portion of the public key. */
};

/**
 * The structure of the manifest that holds root keys for the device.
 */
struct key_certificate {
	uint8_t version;							/**< Manifest format identifier. */
	uint8_t cert_id;							/**< The ID for the manifest. */
	uint16_t marker;							/**< Constant manifest marker. */
	struct cert_public_key pfm_key;				/**< The public key used to sign PFMs. */
	struct cert_public_key app_key;				/**< The public key used to sign the application images. */
	struct cert_public_key root_key;			/**< The public key used to sign this manifest. */
	uint8_t signature[MANIFEST_RSA_KEY_SIZE];	/**< The signature for the manifest. */
};

#pragma pack(pop)

/**
 * The expected version number of the manifest.
 */
#define	KEY_MANIFEST_VERSION		3

/**
 * The value of the manifest marker.
 */
#define	KEY_MANIFEST_MARKER			0x4d43


/**
 * A basic root manifest with raw key data that uses RSA 2k keys.
 */
struct key_manifest_rsa2k {
	struct key_manifest base;					/**< The base key manifest. */
	const struct key_certificate *cert;			/**< The certificate containing the manifest data. */
	struct cert_device_hw *hw;					/**< The certificate hardware API to use. */
	const struct rsa_engine *rsa;				/**< RSA engine to use for certificate verification. */
	struct key_manifest_public_key root_key;	/**< The certificate root key. */
	struct key_manifest_public_key app_key;		/**< The certificate application key. */
	struct key_manifest_public_key pfm_key;		/**< The certificate PFM key. */
	struct rsa_public_key root_key_data;		/**< Key data for the root public key. */
	struct rsa_public_key app_key_data;			/**< Key data for the application public key. */
	struct rsa_public_key pfm_key_data;			/**< Key data for the PFM public key. */
	bool cert_alloc;							/**< Flag when the certificate is dynamically allocated. */
};


int key_manifest_rsa2k_init (struct key_manifest_rsa2k *manifest, struct cert_device_hw *hw,
	const struct flash *flash, uint32_t base_addr, const struct rsa_engine *rsa);
int key_manifest_rsa2k_init_from_memory (struct key_manifest_rsa2k *manifest,
	struct cert_device_hw *hw, const struct key_certificate *cert, const struct rsa_engine *rsa);
void key_manifest_rsa2k_release (struct key_manifest_rsa2k *manifest);

/* Internal functions for use by derived types. */
int key_manifest_rsa2k_check_cert_id (struct cert_device_hw *hw, uint32_t cert_id);
int key_manifest_rsa2k_check_cert_revocation (struct cert_device_hw *hw, uint32_t cert_id);


#endif	/* KEY_MANIFEST_RSA2K_H_ */
