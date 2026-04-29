// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "security_manager_hsp_manticore.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "logging/manticore_logging.h"


int security_manager_hsp_manticore_load_security_policy (const struct security_manager *manager)
{
	const struct security_manager_hsp_manticore *manticore =
		(const struct security_manager_hsp_manticore*) manager;
	uint32_t counter_buffer[SECURITY_MANAGER_HSP_MAX_COUNTER_DWORDS];
	uint8_t *unlock_counter;
	bool is_unlocked;
	int status;

	if (manager == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	security_manager_hsp_get_current_unlock_counter (&manticore->base, counter_buffer,
		&unlock_counter, &is_unlocked);

	if (common_math_is_array_zero (unlock_counter, manticore->base.counter_length)) {
		/* A zero unlock counter is not considered to be a valid value.  It either means that this
		 * is the first time the device is booting in manufacturing or the fuse read returned
		 * incorrect results (perhaps due to some kind of glitching attack).  In either case, the
		 * action is to update the counter fuses to 1 and return an error, which will trigger a
		 * device reset.  If the fuses are not actually zero, attempting to program them to 1 won't
		 * have any effect. */
		unlock_counter[0] = 1;

		status = manticore->base.fuses->program_sw_fuses (manticore->base.fuses,
			manticore->base.counter_addr, (uint32_t*) unlock_counter,
			IN_DWORDS (manticore->base.counter_length));
		if (status != 0) {
			return status;
		}

		status = SECURITY_MANAGER_INVALID_COUNTER_VALUE;
	}
	else if (common_math_get_num_bits_set_in_array (unlock_counter,
		manticore->base.counter_length) == 1) {
		/* If there is only one bit set in the unlock counter, the device has never been locked.
		 * Load the manufacturing security policy. */
		status = manticore->base.policy->base.parse_unlock_policy (&manticore->base.policy->base,
			manticore->mfg_data, manticore->base.locked_length);
		if (status == 0) {
			/* Treat the manufacturing policy as a locked device since the firmware will be trusted.
			 * Reporting as unlocked would cause the DICE identity to be changed, which is not what
			 * we want during manufacturing. */
			manticore->base.state->is_unlocked = false;

			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_MFG_SECURITY_POLICY, 0, 0);
		}
	}
	else {
		/* New unlock policies can only be applied on SoC reset.  If the device is already unlocked,
		 * the existing policy can continue to be used across warm resets. */
		if (manticore->state->is_por || is_unlocked) {
			status = security_manager_hsp_determine_device_security_policy (&manticore->base,
				unlock_counter, is_unlocked);
		}
		else {
			status = security_manager_hsp_load_locked_security_policy (&manticore->base);
		}
	}

	return status;
}

/**
 * Finish initialization of the security manager.  This should be called after successful
 * initialization of the base instance.
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This will have the state for the base
 * instance already initialized.
 * @param mfg_policy The policy data that should be loaded by the security policy when the device is
 * running in manufacturing unlocked mode.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param is_por Flag to indicate if the current boot context is the result of a full SoC reset.
 */
static void security_manager_hsp_manticore_finalize_init (
	struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const uint8_t *mfg_policy, bool is_por)
{
	manager->base.base.load_security_policy = security_manager_hsp_manticore_load_security_policy;

	manager->state = state;
	manager->state->is_por = is_por;

	manager->mfg_data = mfg_policy;
}

/**
 * Initialize a manager for the security configuration of Manticore.
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param locked_policy The policy data that should be loaded by the security policy when the device
 * is locked.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param mfg_policy The policy data that should be loaded by the security policy when the device is
 * running in manufacturing unlocked mode.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param policy_length Length of the policy data.  Both lock and manufacturing policies must be the
 * same length.
 * @param aeb Driver to configure HSP AEBs.
 * @param fuses Interface to the HSP fuses.
 * @param aeb_addr Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect Handler for configuring hardware memory protections.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot KSU key slot that contains the DICE CDI.
 * @param devid_slot KSU key slot that contains the DICE Device ID.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param is_por Flag to indicate if the current boot context is the result of a full SoC reset.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_manticore_init (struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, const uint8_t *mfg_policy, size_t policy_length,
	const struct hsp_aeb *aeb, const struct fuse_controller_interface *fuses, uint16_t aeb_addr,
	uint16_t counter_addr, size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, bool is_por)
{
	int status;

	if (mfg_policy == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	status = security_manager_hsp_init (&manager->base, &state->hsp_state, policy, locked_policy,
		policy_length, aeb, fuses, aeb_addr, counter_addr, counter_length, mem_protect, hash, ccs,
		cdi_slot, devid_slot, hmac_slot, hmac_buffer, buffer_length, flash, unlock_id, NULL, 0);
	if (status != 0) {
		return status;
	}

	security_manager_hsp_manticore_finalize_init (manager, state, mfg_policy, is_por);

	return 0;
}

/**
 * Initialize a manager for the security configuration of Manticore.  Only APIs that are used to
 * lock or unlock the device are supported.  Applying the unlocked policy is not supported.  This
 * type of manager would typically be used by run-time firmware.
 *
 * These API calls are supported:
 * - lock_device
 * - unlock_device
 * - get_unlock_counter
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param fuses Interface to the HSP fuses.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param is_por Flag to indicate if the current boot context is the result of a full SoC reset.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_manticore_init_only_config_unlock (
	struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const struct fuse_controller_interface *fuses, uint16_t counter_addr, size_t counter_length,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t hmac_slot,
	uint8_t *hmac_buffer, size_t buffer_length, const struct flash_store *flash, int unlock_id,
	bool is_por)
{
	/* This flag and the additional state pointer are not needed in this context. */
	UNUSED (is_por);

	return security_manager_hsp_init_only_config_unlock (&manager->base, &state->hsp_state, policy,
		fuses, counter_addr, counter_length, hash, ccs, hmac_slot, hmac_buffer, buffer_length,
		flash, unlock_id, NULL, 0);
}

/**
 * Initialize a manager for the security configuration of Manticore.  Only APIs that are used to
 * load and apply an unlock policy are supported.  Locking or unlocking the device is not supported.
 * This type of manager would typically be used at boot-time (1SP).
 *
 * These API calls are supported:
 * - has_unlock_policy
 * - load_security_policy
 * - apply_device_config
 *
 * @param manager The security manager to initialize.
 * @param state Variable context for the security manager.  This must be uninitialized.
 * @param policy The security policy handler for the device.
 * @param locked_policy The policy data that should be loaded by the security policy when the device
 * is locked.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param mfg_policy The policy data that should be loaded by the security policy when the device is
 * running in manufacturing unlocked mode.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param policy_length Length of the policy data.  Both lock and manufacturing policies must be the
 * same length.
 * @param aeb Driver to configure HSP AEBs.
 * @param fuses Interface to the HSP fuses.
 * @param aeb_addr Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr Fuse address for the unlock anti-replay counter.  This address must be 32-bit
 * aligned.
 * @param counter_length Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect Handler for configuring hardware memory protections.
 * @param hash Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot KSU key slot that contains the DICE CDI.
 * @param devid_slot KSU key slot that contains the DICE Device ID.
 * @param hmac_slot KSU key slot that contains the HMAC key used to sign unlock policies stored on
 * flash.
 * @param hmac_buffer A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length Length of the unlock policy HMAC buffer.  This must be large enough to hold
 * the entire authorized unlock data.
 * @param flash Flash storage used for device unlock policies.
 * @param unlock_id Block ID in flash storage that will be used for storing unlock policies.
 * @param is_por Flag to indicate if the current boot context is the result of a full SoC reset.
 *
 * @return 0 if the manager was initialized successfully or an error code.
 */
int security_manager_hsp_manticore_init_only_apply_unlock (
	struct security_manager_hsp_manticore *manager,
	struct security_manager_hsp_manticore_state *state, const struct security_policy_hsp *policy,
	const uint8_t *locked_policy, const uint8_t *mfg_policy, size_t policy_length,
	const struct hsp_aeb *aeb, const struct fuse_controller_interface *fuses, uint16_t aeb_addr,
	uint16_t counter_addr, size_t counter_length, const struct memory_protection *mem_protect,
	const struct hash_engine *hash, const struct ccs_ksu_interface *ccs, uint8_t cdi_slot,
	uint8_t devid_slot, uint8_t hmac_slot, uint8_t *hmac_buffer, size_t buffer_length,
	const struct flash_store *flash, int unlock_id, bool is_por)
{
	int status;

	if (mfg_policy == NULL) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	status = security_manager_hsp_init_only_apply_unlock (&manager->base, &state->hsp_state, policy,
		locked_policy, policy_length, aeb, fuses, aeb_addr, counter_addr, counter_length,
		mem_protect, hash, ccs, cdi_slot, devid_slot, hmac_slot, hmac_buffer, buffer_length, flash,
		unlock_id, NULL, 0);
	if (status != 0) {
		return status;
	}

	security_manager_hsp_manticore_finalize_init (manager, state, mfg_policy, is_por);

	return 0;
}

/**
 * Initialize only the variable state for a Manticore security manager.  The rest of the manager
 * structure is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param manager The security manager that contains the state to initialize.
 * @param is_por Flag indicating if the current boot is due to a full SoC reset.
 *
 * @return 0 if the state was initialized successfully or an error code.
 */
int security_manager_hsp_manticore_init_state (const struct security_manager_hsp_manticore *manager,
	bool is_por)
{
	int status;

	if ((manager == NULL) || (manager->base.support_policy_load && (manager->mfg_data == NULL))) {
		return SECURITY_MANAGER_INVALID_ARGUMENT;
	}

	status = security_manager_hsp_init_state (&manager->base);
	if (status != 0) {
		return status;
	}

	manager->state->is_por = is_por;

	return 0;
}

/**
 * Release the resources used by a Manticore security manager.
 *
 * @param manager The security manager to release.
 */
void security_manager_hsp_manticore_release (const struct security_manager_hsp_manticore *manager)
{
	UNUSED (manager);
}
