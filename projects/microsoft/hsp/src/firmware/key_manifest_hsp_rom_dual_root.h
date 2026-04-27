// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_HSP_ROM_DUAL_ROOT_H_
#define KEY_MANIFEST_HSP_ROM_DUAL_ROOT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "key_manifest_hsp_rom.h"
#include "crypto/ecc_hw.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest.h"
#include "flash/flash.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Magic number identifying a dual root owner key manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_DUAL_ROOT_KEY_MANIFEST_MARKER				0x524b4d32


/**
 * Magic number identifying a dual root ownership transfer manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_DUAL_ROOT_OWNERSHIP_MANIFEST_MARKER		0x6f746d32


/**
 * Manifest structure for an Ownership Transfer Manifest.  This manifest provides a new, permanent
 * owner key for the device.
 */
struct key_manifest_hsp_rom_dual_root_ownership {
	SP_ECDSA_P384_PUBLIC authenticity_key;			/**< ECC-384 public key of the current authenticity owner. */
	SP_ECDSA_P384_PUBLIC authority_key;				/**< ECC-384 public key of the current authority owner. */
	SP_ECDSA_P384_SIGNATURE authenticity_signature;	/**< Authenticity signature of the signed manifest header. */
	SP_ECDSA_P384_SIGNATURE authority_signature;	/**< Authority signature of the signed manifest header. */
	struct {
		uint32_t length;							/**< Length of the manifest data. */
		SP_MSG_384 digest;							/**< SHA2-384 digest of the manifest data. */
	} header_signed;								/**< Signed manifest header. */
	struct {
		SP_ECDSA_P384_PUBLIC new_authenticity_key;	/**< ECC-384 public key for the new authenticity owner. */
		SP_ECDSA_P384_PUBLIC new_authority_key;		/**< ECC-384 public key for the new authority owner. */
	} data;											/**< Manifest contents. */
};

/**
 * The Transfer Manifest section of the HSP image manifests.  This manifest provides details about
 * any ownership or tenancy transfers to perform.
 */
struct key_manifest_hsp_rom_dual_root_transfer {
	uint32_t marker;											/**< Marker identifying the type of transfer manifest. */
	union {
		struct key_manifest_hsp_rom_grant grant;				/**< A tenancy grant manifest. */
		struct key_manifest_hsp_rom_dual_root_ownership owner;	/**< A ownership transfer manifest. */
	};
};

/**
 * The Key Manifest section of the HSP image manifests.  This is the root manifest for image
 * verification.
 */
struct key_manifest_hsp_rom_dual_root_keys {
	uint32_t marker;									/**< Marker identifying a key manifest. */
	SP_ECDSA_P384_PUBLIC authenticity_key;				/**< ECC-384 public key of the current authenticity owner. */
	SP_ECDSA_P384_PUBLIC authority_key;					/**< ECC-384 public key of the current authority owner. */
	SP_ECDSA_P384_SIGNATURE authenticity_signature;		/**< Authenticity signature of the signed manifest header. */
	SP_ECDSA_P384_SIGNATURE authority_signature;		/**< Authority signature of the signed manifest header. */
	struct {
		uint8_t type;									/**< Identifier for the type of key manifest. */
		uint8_t valid_keys;								/**< Number of valid keys contained in the manifest. */
		uint16_t reserved;								/**< Unused. */
		uint32_t svn;									/**< Anti-rollback value for the manifest. */
		uint32_t length;								/**< Length of the manifest data. */
		SP_MSG_384 digest;								/**< SHA2-384 digest of the manifest data. */
	} header_signed;									/**< Signed manifest header. */
	struct {
		SP_ECDSA_P384_PUBLIC image_authenticity_key;	/**< ECC-384 public key to use for authenticity validation. */
		SP_ECDSA_P384_PUBLIC image_authority_key;		/**< ECC-384 public key to use for authority validation. */
	} data;												/**< Manifest contents. */
};

/**
 * Complete manifest structure to use for for firmware image validation and revocation management.
 */
struct key_manifest_hsp_rom_dual_root_manifest {
	struct key_manifest_hsp_rom_dual_root_transfer transfer;	/**< The transfer manifest for the image. */
	struct key_manifest_hsp_rom_dual_root_keys keys;			/**< The key manifest for the image. */
};

/**
 * Variable context for a key manifest parsed by HSP ROM.
 */
struct key_manifest_hsp_rom_dual_root_state {
	struct key_manifest_hsp_rom_state base;						/**< Base instance of manifest data. */
	struct key_manifest_hsp_rom_dual_root_manifest manifest;	/**< Dual root manifest data. */
	struct ecc_point_public_key authority_key;					/**< Key data for the authority public key. */
	bool authority_key_populated;								/**< Flag to keep track of whether the authority key is populated. */
};

/**
 * Handler for the key manifest parsed by HSP ROM to validate a 1SP image on devices
 * that support optional Dual Root keys. If no Authority Key is provided, the mandatory
 * Authenticity key acts as both Authority and Authenticity.
 */
struct key_manifest_hsp_rom_dual_root {
	/**
	 * Base key manifest ROM instance. Includes the RoT state, HW PKA engine, and Authenticity
	 * Root Key (referred to simply as root key). Also includes Image Authenticity Key (fw_key)
	 * and optional Image Authority Key (secondary_key) for image validation.
	 * */
	struct key_manifest_hsp_rom base;

	/**
	 * Get the root authority public key used to verify the manifest. This key is optional and is
	 * not guaranteed to be present. If there is no valid root authority key in the manifest, it
	 * will be null.
	 *
	 * A key returned by this function doesn't necessarily mean it is trusted. This can only be
	 * guaranteed after successful verification of the manifest, which is a separate operation.
	 *
	 * @param rom The manifest to get the key from.
	 *
	 * @return The root authority public key or null if there is no root authority key. The memory
	 * for this key is managed by the manifest instance.
	 */
	const struct key_manifest_public_key* (*get_root_authority_key) (
		const struct key_manifest_hsp_rom_dual_root *rom);

	struct key_manifest_hsp_rom_dual_root_state *state;	/**< Variable context for the key manifest. */
	struct key_manifest_public_key authority_key;		/**< The optional key used for manifest authority validation. */
};


int key_manifest_hsp_rom_dual_root_init (struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const struct flash *flash, uint32_t base_addr, const struct ecc_hw *pka);
int key_manifest_hsp_rom_dual_root_init_from_memory (
	struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const uint8_t *base_addr, const struct ecc_hw *pka);

int key_manifest_hsp_rom_dual_root_init_api (struct key_manifest_hsp_rom_dual_root *manifest,
	struct key_manifest_hsp_rom_dual_root_state *state, const struct hw_rot *rot,
	const struct ecc_hw *pka);

void key_manifest_hsp_rom_dual_root_release (const struct key_manifest_hsp_rom_dual_root *manifest);

int key_manifest_hsp_rom_dual_root_get_size_on_flash (const struct flash *flash,
	uint32_t base_addr);


#endif	/* KEY_MANIFEST_HSP_ROM_DUAL_ROOT_H_ */
