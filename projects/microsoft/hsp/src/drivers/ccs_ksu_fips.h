// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_FIPS_H_
#define CCS_KSU_FIPS_H_

#include "ccs_ksu_interface.h"
#include "platform_api.h"
#include "crypto/ecc_hw.h"


/**
 * Key slot for a single private key stored in the firmware KSU.  Just like with CCS hardware, the
 * public key is not stored.
 */
struct ccs_ksu_fips_key {
	union {
		SP_MSG_256 key_256;	/**< A 256-bit private key. */
		SP_MSG_384 key_384;	/**< A 384-bit private key. */
	};

	uint32_t attributes;	/**< Key slot attributes.  This uses the same bit definition as HW. */
	int hw_slot;			/**< CCS HW slot that corresponds to this private key. */
};

/**
 * KSU implementation for keys residing in firmware memory.
 *
 * This KSU does not need to match the number of slots available in the hardware KSU, since there is
 * a dynamic mapping between firmware and hardware KSU slot.  This provides flexibility to optimize
 * memory usage since not all keys will need to be in the firmware KSU.
 *
 * This does not expand key storage capabilities.  The overall number keys managed by both KSUs is
 * still limited by the number of slots supported by hardware.
 */
struct ccs_ksu_fips_ksu {
	struct ccs_ksu_fips_key *slots;	/**< List of key slots in the firmware KSU. */
	size_t slot_count;				/**< The number of key slots in the KSU. */
};

/**
 * Marker for a firmware KSU slot that is not being used for a key.
 */
#define	CCS_KSU_FIPS_KSU_SLOT_UNUSED		-1


/**
 * Variable context for the FIPS compliant CCS implementation.
 */
struct ccs_ksu_fips_state {
	platform_mutex lock;	/**< Lock for synchronization. */
};

/**
 * A CCS implementation that complies with FIPS requirements for crypto usage.
 *
 * CCS hardware cannot be used in a FIPS compliant way for ECC operations, particularly when paired
 * with PKA.  Instead, these keys and operations are managed through firmware, while the remaining
 * CCS commands that do meet the FIPS requirements will continue to be done in hardware.  This
 * results in a few this to note about CCS behavior in this context.
 *
 * 1. ECC private keys do not have the same guarantees regarding protections from firmware access as
 *    the keys that are stored in the hardware KSU.  Since they exist in firmware memory, firmware
 *    can access them directly at any time.
 * 2. The MustAppendPCR attribute is not enforced for ECC signing operations and no PCR is ever
 *    appended.  This attribute still must be set for the CertifyECCPublicKey command.
 * 3. Whenever SetKey is used to store a known ECC private in the KSU, that key will not persist to
 *    other CCS instances unless they share the same firmware KSU memory.  When using the same keys
 *    through multiple stages of boot, this means that this key storage would need to be in a shared
 *    location that doesn't get reinitialized.
 * 4. ECC private keys acquired through the DeriveECCKey command will persist in hardware as they
 *    typically would.
 * 5. If the firmware KSU is shared between CCS instances within the same execution context, that
 *    execution context must ensure proper synchronization between CCS accesses.
 * 6. SHACK1 CCS hardware implementations are not supported.
 * 7. Depending on the size of the firmware KSU, it's possible to run out of space for new keys,
 *    even if the specified key slot is valid for the hardware implementation.  If this situation is
 *    encountered, a larger firmware KSU is required.
 */
struct ccs_ksu_fips {
	struct ccs_ksu_interface base;			/**< Base CCS driver interface. */
	struct ccs_ksu_fips_state *state;		/**< Variable context for the CCS driver. */
	const struct ccs_ksu_fips_ksu *ksu;		/**< Firmware KSU to use with this CCS instance. */
	const struct ccs_ksu_interface *ccs_hw;	/**< Hardware CCS implementation. */
	const struct ecc_hw *ecc;				/**< Hardware accelerator to use for ECC operations. */
	const struct hash_engine *hash;			/**< Hash engine used for key certification. */
};


int ccs_ksu_fips_init (struct ccs_ksu_fips *ccs, struct ccs_ksu_fips_state *state,
	const struct ccs_ksu_fips_ksu *ksu, const struct ccs_ksu_interface *ccs_hw,
	const struct ecc_hw *ecc, const struct hash_engine *hash, bool ksu_init);
int ccs_ksu_fips_init_state (const struct ccs_ksu_fips *ccs, bool ksu_init);
void ccs_ksu_fips_release (const struct ccs_ksu_fips *ccs);


#endif	/* CCS_KSU_FIPS_H_ */
