// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SECURITY_MANAGER_HSP_MANTICORE_H_
#define SECURITY_MANAGER_HSP_MANTICORE_H_

#include "system/security_manager_hsp.h"


/**
 * Variable context for the HSP security manager.
 */
struct security_manager_hsp_manticore_state {
	struct security_manager_hsp_state hsp_state;	/**< Variable context for the base instance. */
	bool is_por;									/**< Flag indicating whether the current boot context is a POR. */
};

/**
 * Applies Manticore specific handling for security policy management for HSP.
 */
struct security_manager_hsp_manticore {
	struct security_manager_hsp base;					/**< Base security manager API. */
	struct security_manager_hsp_manticore_state *state;	/**< Variable context for the security manager. */
	const uint8_t *mfg_data;							/**< Policy data for a manufacturing unlocked device. */
};


int security_manager_hsp_manticore_init (struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, const uint8_t *mfg_policy, size_t policy_length,
	const struct hsp_aeb *aeb, const struct fuse_controller_interface *fuses, uint16_t aeb_addr,
	uint16_t counter_addr, size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, bool is_por);
int security_manager_hsp_manticore_init_only_config_unlock (
	struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const struct fuse_controller_interface *fuses, uint16_t counter_addr, size_t counter_length,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t hmac_slot,
	uint8_t *hmac_buffer, size_t buffer_length, const struct flash_store *flash, int unlock_id,
	bool is_por);
int security_manager_hsp_manticore_init_only_apply_unlock (
	struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, const uint8_t *mfg_policy, size_t policy_length,
	const struct hsp_aeb *aeb, const struct fuse_controller_interface *fuses, uint16_t aeb_addr,
	uint16_t counter_addr, size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, bool is_por);
int security_manager_hsp_manticore_init_state (
	const struct security_manager_hsp_manticore *manager, bool is_por);
void security_manager_hsp_manticore_release (const struct security_manager_hsp_manticore *manager);


#endif	/* SECURITY_MANAGER_HSP_MANTICORE_H_ */
