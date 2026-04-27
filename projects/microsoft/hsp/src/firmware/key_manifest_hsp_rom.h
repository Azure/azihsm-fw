// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef KEY_MANIFEST_HSP_ROM_H_
#define KEY_MANIFEST_HSP_ROM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "crypto/ecc_hw.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest.h"
#include "flash/flash.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Magic number identifying a key manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_MARKER		0x4d414e54

/**
 * Magic number identifying a tenancy transfer manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_TENANCY_MANIFEST_MARKER	0x746d616e

/**
 * Magic number identifying an ownership transfer manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_OWNERSHIP_MANIFEST_MARKER	0x6f776e74

/**
 * Magic number identifying a null manifest.
 */
#define	KEY_MANIFEST_HSP_ROM_NULL_MANIFEST_MARKER		0x4e554c4c

/**
 * Supported types of key manifests.
 */
enum {
	KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_OWNER = 0,	/**< An owner key manifest. */
	KEY_MANIFEST_HSP_ROM_KEY_MANIFEST_TENANT = 1,	/**< A tenant key manifest. */
};

/**
 * Manifest structure for a Tenancy Grant Manifest.  This manifest provides a temporary signing key
 * to use for firmware authentication.
 */
struct key_manifest_hsp_rom_grant {
	SP_ECDSA_P384_SIGNATURE signature;		/**< Signature of the signed manifest header. */
	struct {
		uint8_t type;						/**< Identifier for the type of tenancy manifest. */
		uint8_t reserved[3];				/**< Unused. */
		uint32_t length;					/**< Length of the manifest data. */
		SP_MSG_384 digest;					/**< SHA2-384 digest of the manifest data. */
	} header_signed;						/**< Signed manifest header. */
	struct {
		SP_MSG_384 grant_token;				/**< Device-unique token granting tenancy. */
		SP_ECDSA_P384_PUBLIC tenant_key;	/**< ECC-384 public key to use for image validation. */
	} data;									/**< Manifest contents. */
};

/**
 * Manifest structure for an Ownership Transfer Manifest.  This manifest provides a new, permanent
 * owner key for the device.
 */
struct key_manifest_hsp_rom_ownership {
	SP_ECDSA_P384_PUBLIC owner_key;			/**< ECC-384 public key of the current device owner. */
	SP_ECDSA_P384_SIGNATURE signature;		/**< Signature of the signed manifest header. */
	struct {
		uint32_t length;					/**< Length of the manifest data. */
		SP_MSG_384 digest;					/**< SHA2-384 digest of the manifest data. */
	} header_signed;						/**< Signed manifest header. */
	struct {
		SP_ECDSA_P384_PUBLIC new_owner_key;	/**< ECC-384 public key for the new device owner. */
	} data;									/**< Manifest contents. */
};

/**
 * The Transfer Manifest section of the HSP image manifests.  This manifest provides details about
 * any ownership or tenancy transfers to perform.
 */
struct key_manifest_hsp_rom_transfer {
	uint32_t marker;									/**< Marker identifying the type of transfer manifest. */
	union {
		struct key_manifest_hsp_rom_grant grant;		/**< A tenancy grant manifest. */
		struct key_manifest_hsp_rom_ownership owner;	/**< A ownership transfer manifest. */
	};
};

/**
 * The Key Manifest section of the HSP image manifests.  This is the root manifest for image
 * verification.
 */
struct key_manifest_hsp_rom_keys {
	uint32_t marker;						/**< Marker identifying a key manifest. */
	SP_ECDSA_P384_PUBLIC owner_key;			/**< ECC-384 public key of the current device owner. */
	SP_ECDSA_P384_SIGNATURE signature;		/**< Signature of the signed manifest header. */
	struct {
		uint8_t type;						/**< Identifier for the type of key manifest. */
		uint8_t valid_keys;					/**< Number of valid keys contained in the manifest. */
		uint16_t reserved;					/**< Unused. */
		uint32_t svn;						/**< Anti-rollback value for the manifest. */
		uint32_t length;					/**< Length of the manifest data. */
		SP_MSG_384 digest;					/**< SHA2-384 digest of the manifest data. */
	} header_signed;						/**< Signed manifest header. */
	struct {
		SP_ECDSA_P384_PUBLIC signing_key;	/**< ECC-384 public key to use for further validation. */
		SP_ECDSA_P384_PUBLIC secondary_key;	/**< Second ECC-384 public key for image validation. */
	} data;									/**< Manifest contents. */
};

/**
 * Complete manifest structure to use for for firmware image validation and revocation management.
 */
struct key_manifest_hsp_rom_manifest {
	struct key_manifest_hsp_rom_transfer transfer;	/**< The transfer manifest for the image. */
	struct key_manifest_hsp_rom_keys keys;			/**< The key manifest for the image. */
};


/**
 * Variable context for a key manifest parsed by HSP ROM.
 */
struct key_manifest_hsp_rom_state {
	struct key_manifest_hsp_rom_manifest manifest;	/**< The key manifest data. */
	struct ecc_point_public_key root_key;			/**< Key data for the root public key. */
	struct ecc_point_public_key fw_key;				/**< Key data for the image public key. */
	struct ecc_point_public_key secondary_key;		/**< Key data for the secondary image public key. */
	struct ecc_point_public_key tenancy_grant_key;	/**< Key data for key tenancy grant public key in tenant manifests. */
	uint8_t transfer;								/**< State tracking for transfers. */
};

/**
 * Handler for the key manifest parsed by HSP ROM to validate a 1SP image.
 */
struct key_manifest_hsp_rom {
	struct key_manifest base;	/**< Base key manifest instance. */

	/**
	 * Initialize only the variable state of an HSP ROM key manifest instance from data stored on flash.
	 * The rest of the manifest structure is assumed to have already been initialized.
	 *
	 * This would generally be used with a statically initialized manifest instance, but it can also be
	 * used to reinitialize an instance that was previously valid and released with
	 * key_manifest_hsp_rom_release.
	 *
	 * @param rom The manifest instance containing the state to initialize.
	 * @param flash Flash device where the key manifest is stored.
	 * @param base_addr Starting address for ROM manifests on flash.
	 *
	 * @return 0 if the manifest state was successfully initialized or an error code.
	 */
	int (*init_state) (const struct key_manifest_hsp_rom *rom, const struct flash *flash,
		uint32_t base_addr);

	/**
	 * Initialize only the variable state of an HSP ROM key manifest instance from data stored in
	 * memory.  The rest of the manifest structure is assumed to have already been initialized.
	 *
	 * This would generally be used with a statically initialized manifest instance, but it can also be
	 * used to reinitialize an instance that was previously valid and released with
	 * key_manifest_hsp_rom_release.
	 *
	 * @param rom The manifest instance containing the state to initialize.
	 * @param base_addr Starting address for ROM manifests in memory.
	 *
	 * @return 0 if the manifest state was successfully initialized or an error code.
	 */
	int (*init_state_from_memory) (const struct key_manifest_hsp_rom *rom,
		const uint8_t *base_addr);

	/**
	 * Get the total size of the key manifest section of the image.
	 *
	 * @param rom The key manifest to query.
	 *
	 * @return The total size of all manifests prepended to the image.  If the manifest is null, 0
	 * will be returned
	 */
	size_t (*get_total_size) (const struct key_manifest_hsp_rom *rom);

	/**
	 * Get the SVN value of the key manifest.
	 *
	 * @param rom The key manifest to query.
	 *
	 * @return The SVN value from the key manifest.  If the manifest is null, 0 will be returned.
	 */
	uint32_t (*get_svn) (const struct key_manifest_hsp_rom *rom);

	/**
	 * Get the secondary public key used for image validation from an owner manifest. Will return
	 * null if this manifest does not have a second key.
	 *
	 * A key returned by this function doesn't necessarily mean it is trusted.  This can only be
	 * guaranteed after successful verification of the manifest, which is a separate operation.
	 *
	 * @param rom The manifest to get the key from.
	 *
	 * @return The secondary public key or null if there is no secondary key.  The memory for this
	 * key is managed by the manifest instance.
	 */
	const struct key_manifest_public_key* (*get_secondary_key) (
		const struct key_manifest_hsp_rom *rom);

	/**
	 * Get the tenancy grant key in the tenant key manifest.  Will return null if this is not a tenant
	 * key manifest.
	 *
	 * A key returned by this function doesn't necessarily mean it is trusted. This can only be
	 * guaranteed after successful verification of the manifest, which is a separate operation.
	 *
	 * @param rom The manifest to get the key from.
	 *
	 * @return The grant manifest public key or null if this is not a grant manifest. The memory
	 * for this key is managed by the manifest instance.
	 */
	const struct key_manifest_public_key* (*get_tenancy_grant_key) (
		const struct key_manifest_hsp_rom *rom);

	/**
	 * Indicate if the key manifest describes a valid ownership transfer to a new root key.
	 *
	 * Until the manifest has been verified, this will always be false.
	 *
	 * @param rom The key manifest to query.
	 *
	 * @return true if there is a valid ownership transfer or false if not.
	 */
	bool (*is_ownership_transfer) (const struct key_manifest_hsp_rom *rom);

	/**
	 * If the manifest indicates an ownership transfer, update the hardware-backed root key to use
	 * the root key from the owner manifest.  Only if the transfer specifies a valid transfer for a
	 * key different than one currently in hardware will any update actually take place.
	 *
	 * Until the manifest has been verified, no root key update is possible.
	 *
	 * @param rom The key manifest to use for updating RoT state.
	 * @param hash A hash engine to use for generating the root key hash.
	 *
	 * @return 0 if the hardware RoT state is consistent with the manifest information or an error
	 * code.  A successful return does not mean hardware was updated.  It could equally mean that
	 * hardware needed no update.
	 */
	int (*update_root_key) (const struct key_manifest_hsp_rom *rom, const struct hash_engine *hash);

	/**
	 * Indicate if the key manifest describes a valid tenancy transfer, representing a change in the
	 * tenancy state.  This can be either a tenancy grant or revocation.
	 *
	 * Until the manifest has been verified, this will always be false.
	 *
	 * @param rom The key manifest to query.
	 *
	 * @return true if there is a valid tenancy transfer or false if not.
	 */
	bool (*is_tenancy_transfer) (const struct key_manifest_hsp_rom *rom);

	/**
	 * Indicate if the key manifest describes a valid tenancy grant.  This is not an indication of
	 * a tenancy transfer.  It is an indication that the current manifest structure is a tenancy
	 * grant, even if it is for an active tenancy.
	 *
	 * When there is no hardware-backed root key in the device, tenancy transfers are not possible.
	 * However, even in this case images, valid tenant manifests will still be indicated by this
	 * call.
	 *
	 * Until the manifest has been verified, this will always be false.
	 *
	 * @param rom The key manifest to query.
	 *
	 * @return true if there is a valid tenancy grant or false if not.
	 */
	bool (*is_tenancy_grant) (const struct key_manifest_hsp_rom *rom);

	/**
	 * Update the hardware tenancy counter to reflect the tenancy state represented by the manifest
	 * structure.  If the manifest contains an owner manifest, an active tenancy will be revoked.
	 * If the manifest contains a tenant manifest, an active tenancy will be granted.
	 *
	 * Until the manifest has been verified, no tenancy counter update is possible.
	 *
	 * @param rom The key manifest to use for updating RoT state.
	 *
	 * @return 0 if the hardware RoT state is consistent with the manifest information or an error
	 * code.  A successful return does not mean hardware was updated.  It could equally mean that
	 * hardware needed no update.
	 */
	int (*update_tenancy_counter) (const struct key_manifest_hsp_rom *rom);

	struct key_manifest_hsp_rom_state *state;			/**< Variable context for the key manifest. */
	const struct hw_rot *rot;							/**< Interface to the RoT state. */
	const struct ecc_hw *pka;							/**< Interface to the HW PKA engine. */
	struct key_manifest_public_key root_key;			/**< The key used for manifest validation. */
	struct key_manifest_public_key fw_key;				/**< The key to use for image validation. */
	struct key_manifest_public_key secondary_key;		/**< A secondary key for image validation. */
	struct key_manifest_public_key tenancy_grant_key;	/**< The tenancy grant key from the tenant key manifest. */

	/**
	 * Internal function to authenticate the ownership transfer manifest signature against its
	 * root key and to compare the new ownership key against the root key in the owner manifest.
	 *
	 * @param rom The ROM manifest to check for ownership transfer.
	 * @param hash A hash engine to use for generating the root key hash.
	 * @param key_buffer_out Output to the current owner key buffer for verification against what was
	 * previously stored.
	 * @param key_buffer_out_length Length of the output key buffer for verification.
	 *
	 * @return 0 if the signature was authenticated and the new ownership key matches the root key in
	 * the owner manifest. An error code if not.
	 */
	int (*authenticate_ownership_transfer_root_key_buffer) (const struct key_manifest_hsp_rom *rom,
		const struct hash_engine *hash, uint8_t **key_buffer_out, size_t *key_buffer_out_length);
};


int key_manifest_hsp_rom_init (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const struct flash *flash,
	uint32_t base_addr, const struct ecc_hw *pka);
int key_manifest_hsp_rom_init_from_memory (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const uint8_t *base_addr,
	const struct ecc_hw *pka);

int key_manifest_hsp_rom_init_api (struct key_manifest_hsp_rom *manifest,
	struct key_manifest_hsp_rom_state *state, const struct hw_rot *rot, const struct ecc_hw *pka);

void key_manifest_hsp_rom_release (const struct key_manifest_hsp_rom *manifest);

int key_manifest_hsp_rom_get_size_on_flash (const struct flash *flash, uint32_t base_addr);

/* Internal functions for use by derived types. */
int key_manifest_hsp_rom_check_manifest_signature (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, const uint8_t *signed_data, size_t signed_length,
	const SP_ECDSA_P384_SIGNATURE *signature, const SP_ECDSA_P384_PUBLIC *key,
	const uint8_t *hashed_data, size_t hashed_length, const uint8_t *expected);

int key_manifest_hsp_rom_check_manifest_svn (const struct key_manifest_hsp_rom *rom,
	uint64_t *rot_svn);

int key_manifest_hsp_rom_verify_transfer_manifest_no_root_key (
	const struct key_manifest_hsp_rom *rom, const struct hash_engine *hash, int status,
	uint32_t expected_owership_transfer_marker);

int key_manifest_hsp_rom_verify_transfer_manifest_with_root_key (
	const struct key_manifest_hsp_rom *rom, const struct hash_engine *hash,
	const uint8_t *key_buffer, size_t key_buffer_length,
	uint32_t expected_owership_transfer_marker);

int hsp_key_manifest_hsp_rom_execute_ownership_transfer (const struct key_manifest_hsp_rom *rom,
	const struct hash_engine *hash, const uint8_t *key_buffer, size_t key_buffer_length);


#endif	/* KEY_MANIFEST_HSP_ROM_H_ */
