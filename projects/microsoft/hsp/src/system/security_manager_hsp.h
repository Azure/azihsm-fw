// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SECURITY_MANAGER_HSP_H_
#define SECURITY_MANAGER_HSP_H_

#include <stdbool.h>
#include "crypto/hash.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_aeb.h"
#include "flash/flash_store.h"
#include "mpu/memory_protection.h"
#include "system/security_manager.h"
#include "system/security_policy_hsp.h"


/**
 * The maximum counter length supported on HSP.
 *
 * This is not a hard limit, just the most needed by any current implementation.  If more bytes are
 * needed, this value can be increased.  Note that increasing this value will increase stack usage
 * during lock and policy load flows.
 */
#define	SECURITY_MANAGER_HSP_MAX_COUNTER_LENGTH		32

/**
 * The maximum counter length expressed as a count of dwords.
 */
#define	SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS     \
	IN_DWORDS (SECURITY_MANAGER_HSP_MAX_COUNTER_LENGTH)


/**
 * Variable context for the HSP security manager.
 */
struct security_manager_hsp_state {
	bool is_unlocked;	/**< Flag indicating whether an unlock policy was loaded or not. */
};

/**
 * Manages unlock token storage and verification for HSP-based devices to determine the current
 * security policy of the device.  Once the current security policy is known, the appropriate
 * security configuration is applied.
 */
struct security_manager_hsp {
	struct security_manager base;					/**< Base security manager API. */
	struct security_manager_hsp_state *state;		/**< Variable context for the security manager. */
	const struct security_policy_hsp *policy;		/**< Security policy for the device. */
	const uint8_t *locked_data;						/**< Policy data for a locked device. */
	size_t locked_length;							/**< Length of the locked policy data. */
	const struct hsp_aeb *aeb;						/**< Driver to configure HSP AEBs. */
	const struct fuse_controller_interface *fuses;	/**< Driver for the HSP fuses. */
	const struct memory_protection *mem_protect;	/**< Handler for hardware memory protection. */
	const struct hash_engine *hash;					/**< Hash engine to use with unlock policy signing. */
	const struct ccs_ksu_interface *ccs;			/**< CCS driver for signing stored unlock policies. */
	const struct flash_store *flash;				/**< Storage for applied unlock policies. */
	uint16_t aeb_addr;								/**< Fuse address for AEBs. */
	uint16_t counter_addr;							/**< Fuse address for the unlock counter. */
	size_t counter_length;							/**< Length of the unlock counter. */
	uint8_t cdi_slot;								/**< KSU slot that contains the DICE CDI. */
	uint8_t devid_slot;								/**< KSU slot that contains the DICE device ID.*/
	uint8_t hmac_slot;								/**< KSU slot that contains the unlock HMAC key. */
	uint8_t *unlock_buffer;							/**< Shared memory buffer to use for unlock HMAC operations. */
	size_t unlock_length;							/**< Length of the buffer for unlock HMAC operations. */
	int unlock_id;									/**< Flash block ID for storing unlock policies. */
	uint32_t *unlock_nonce;							/**< Storage for a one-time unlock nonce. */
	size_t nonce_length;							/**< Length of the unlock nonce storage buffer. */
	bool support_policy_load;						/**< Supports loading an unlock policy. */
};


int security_manager_hsp_init (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, size_t policy_length, const struct hsp_aeb *aeb,
	const struct fuse_controller_interface *fuses, uint16_t aeb_addr, uint16_t counter_addr,
	size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, uint32_t *unlock_nonce, size_t nonce_length);
int security_manager_hsp_init_only_config_unlock (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const struct fuse_controller_interface *fuses, uint16_t counter_addr, size_t counter_length,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t hmac_slot,
	uint8_t *hmac_buffer, size_t buffer_length, const struct flash_store *flash, int unlock_id,
	uint32_t *unlock_nonce, size_t nonce_length);
int security_manager_hsp_init_only_apply_unlock (struct security_manager_hsp *manager,
	struct security_manager_hsp_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, size_t policy_length, const struct hsp_aeb *aeb,
	const struct fuse_controller_interface *fuses, uint16_t aeb_addr, uint16_t counter_addr,
	size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, uint32_t *unlock_nonce, size_t nonce_length);
int security_manager_hsp_init_state (const struct security_manager_hsp *manager);
void security_manager_hsp_release (const struct security_manager_hsp *manager);

int security_manager_hsp_derive_hmac_key (const struct security_manager_hsp *manager,
	uint8_t src_key_slot);
int security_manager_hsp_derive_unlocked_device_keys (const struct security_manager_hsp *hsp);

/* Internal functions for use by derived types. */
bool security_manager_hsp_is_counter_unlocked (const struct security_manager_hsp *hsp,
	const uint8_t *unlock_counter);

void security_manager_hsp_get_current_unlock_counter (const struct security_manager_hsp *hsp,
	uint32_t counter_buffer[SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS], uint8_t **unlock_counter,
	bool *is_unlocked);
int security_manager_hsp_determine_device_security_policy (const struct security_manager_hsp *hsp,
	uint8_t *unlock_counter, bool is_unlocked);
int security_manager_hsp_load_locked_security_policy (const struct security_manager_hsp *hsp);


#endif	/* SECURITY_MANAGER_HSP_H_ */
