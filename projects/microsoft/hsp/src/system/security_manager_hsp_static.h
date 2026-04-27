// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SECURITY_MANAGER_HSP_STATIC_H_
#define SECURITY_MANAGER_HSP_STATIC_H_

#include "security_manager_hsp.h"


/* Internal functions declared to allow for static initialization. */
int security_manager_hsp_lock_device (const struct security_manager *manager);
int security_manager_hsp_unlock_device (const struct security_manager *manager,
	const uint8_t *policy, size_t length);
int security_manager_hsp_get_unlock_counter (const struct security_manager *manager,
	uint8_t *counter, size_t length);
int security_manager_hsp_has_unlock_policy (const struct security_manager *manager);
int security_manager_hsp_load_security_policy (const struct security_manager *manager);
int security_manager_hsp_apply_device_config (const struct security_manager *manager);

int security_manager_hsp_lock_device_unsupported (const struct security_manager *manager);
int security_manager_hsp_unlock_device_unsupported (const struct security_manager *manager,
	const uint8_t *policy, size_t length);
int security_manager_hsp_get_unlock_counter_unsupported (const struct security_manager *manager,
	uint8_t *counter, size_t length);
int security_manager_hsp_has_unlock_policy_unsupported (const struct security_manager *manager);
int security_manager_hsp_load_security_policy_unsupported (const struct security_manager *manager);
int security_manager_hsp_apply_device_config_unsupported (const struct security_manager *manager);

int security_manager_hsp_get_security_policy (const struct security_manager *manager,
	const struct security_policy **policy);


/**
 * Constant initializer for the security manager API.
 */
#define	SECURITY_MANAGER_HSP_API_INIT  { \
		.lock_device = security_manager_hsp_lock_device, \
		.unlock_device = security_manager_hsp_unlock_device, \
		.get_unlock_counter = security_manager_hsp_get_unlock_counter, \
		.has_unlock_policy = security_manager_hsp_has_unlock_policy, \
		.load_security_policy = security_manager_hsp_load_security_policy, \
		.apply_device_config = security_manager_hsp_apply_device_config, \
		.internal = { \
			.get_security_policy = security_manager_hsp_get_security_policy, \
		}, \
	}

/**
 * Constant initializer for the security manager API for instances that only support lock/unlock
 * flows.
 */
#define	SECURITY_MANAGER_HSP_ONLY_CONFIG_UNLOCK_API_INIT  { \
		.lock_device = security_manager_hsp_lock_device, \
		.unlock_device = security_manager_hsp_unlock_device, \
		.get_unlock_counter = security_manager_hsp_get_unlock_counter, \
		.has_unlock_policy = security_manager_hsp_has_unlock_policy_unsupported, \
		.load_security_policy = security_manager_hsp_load_security_policy_unsupported, \
		.apply_device_config = security_manager_hsp_apply_device_config_unsupported, \
		.internal = { \
			.get_security_policy = security_manager_hsp_get_security_policy, \
		}, \
	}

/**
 * Constant initializer for the security manager API for instances that only support loading unlock
 * policies.
 */
#define	SECURITY_MANAGER_HSP_ONLY_APPLY_UNLOCK_API_INIT  { \
		.lock_device = security_manager_hsp_lock_device_unsupported, \
		.unlock_device = security_manager_hsp_unlock_device_unsupported, \
		.get_unlock_counter = security_manager_hsp_get_unlock_counter_unsupported, \
		.has_unlock_policy = security_manager_hsp_has_unlock_policy, \
		.load_security_policy = security_manager_hsp_load_security_policy, \
		.apply_device_config = security_manager_hsp_apply_device_config, \
		.internal = { \
			.get_security_policy = security_manager_hsp_get_security_policy, \
		}, \
	}

/**
 * Internal initializer for an HSP security configuration manager.  This should not be called
 * directly and should only be used by other static initializers.
 *
 * There is no validation done on the arguments.
 *
 * @param api The security manager API to use for the instance.
 * @param state_ptr Variable context for the security manager.
 * @param policy_ptr The security policy handler for the device.
 * @param locked_policy_ptr The policy data that should loaded by the security policy when the
 * device is locked.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param policy_length_arg Length of the locked policy data.
 * @param aeb_ptr Driver to configure HSP AEBs.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param aeb_addr_arg Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr_arg Fuse address for the unlock anti-replay counter.  This address must be
 * 32-bit aligned.
 * @param counter_length_arg Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect_ptr Handler for configuring hardware memory protections.
 * @param hash_ptr Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs_ptr CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot_arg KSU key slot that contains the DICE CDI.
 * @param devid_slot_arg KSU key slot that contains the DICE Device ID.
 * @param hmac_slot_arg KSU key slot that contains the HMAC key used to sign unlock policies stored
 * on flash.
 * @param hmac_buffer_ptr A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length_arg Length of the unlock policy HMAC buffer.  This must be large enough to
 * hold the entire authorized unlock data.
 * @param flash_ptr Flash storage used for device unlock policies.
 * @param unlock_id_arg Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce_ptr Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length_arg Length of the buffer for the one-time unlock nonce.  This must be at
 * least large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends
 * on the length of the HMAC key when the one-time unlock policy has been received.
 * @param support_policy_load_arg Flag indicating if policy loading is supported by the instance.
 */
#define	security_manager_hsp_static_init_internal(api, state_ptr, policy_ptr, locked_policy_ptr, \
	policy_length_arg, aeb_ptr, fuses_ptr, aeb_addr_arg, counter_addr_arg, counter_length_arg, \
	mem_protect_ptr, hash_ptr, ccs_ptr, cdi_slot_arg, devid_slot_arg, hmac_slot_arg, \
	hmac_buffer_ptr, buffer_length_arg, flash_ptr, unlock_id_arg, unlock_nonce_ptr, \
	nonce_length_arg, support_policy_load_arg)	{ \
		.base = api, \
		.state = state_ptr, \
		.policy = policy_ptr, \
		.locked_data = locked_policy_ptr, \
		.locked_length = policy_length_arg, \
		.aeb = aeb_ptr, \
		.fuses = fuses_ptr, \
		.mem_protect = mem_protect_ptr, \
		.hash = hash_ptr, \
		.ccs = ccs_ptr, \
		.flash = flash_ptr, \
		.aeb_addr = aeb_addr_arg, \
		.counter_addr = counter_addr_arg, \
		.counter_length = counter_length_arg, \
		.cdi_slot = cdi_slot_arg, \
		.devid_slot = devid_slot_arg, \
		.hmac_slot = hmac_slot_arg, \
		.unlock_buffer = hmac_buffer_ptr, \
		.unlock_length = buffer_length_arg, \
		.unlock_id = unlock_id_arg, \
		.unlock_nonce = unlock_nonce_ptr, \
		.nonce_length = nonce_length_arg, \
		.support_policy_load = support_policy_load_arg, \
	}


/**
 * Initialize a static instance of a manager for the security configuration of HSP-based devices.
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the security manager.
 * @param policy_ptr The security policy handler for the device.
 * @param locked_policy_ptr The policy data that should loaded by the security policy when the
 * device is locked.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param policy_length_arg Length of the locked policy data.
 * @param aeb_ptr Driver to configure HSP AEBs.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param aeb_addr_arg Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr_arg Fuse address for the unlock anti-replay counter.  This address must be
 * 32-bit aligned.
 * @param counter_length_arg Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect_ptr Handler for configuring hardware memory protections.
 * @param hash_ptr Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs_ptr CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot_arg KSU key slot that contains the DICE CDI.
 * @param devid_slot_arg KSU key slot that contains the DICE Device ID.
 * @param hmac_slot_arg KSU key slot that contains the HMAC key used to sign unlock policies stored
 * on flash.
 * @param hmac_buffer_ptr A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length_arg Length of the unlock policy HMAC buffer.  This must be large enough to
 * hold the entire authorized unlock data.
 * @param flash_ptr Flash storage used for device unlock policies.
 * @param unlock_id_arg Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce_ptr Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length_arg Length of the buffer for the one-time unlock nonce.  This must be at
 * least large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends
 * on the length of the HMAC key when the one-time unlock policy has been received.
 */
#define	security_manager_hsp_static_init(state_ptr, policy_ptr, locked_policy_ptr, \
	policy_length_arg, aeb_ptr, fuses_ptr, aeb_addr_arg, counter_addr_arg, counter_length_arg, \
	mem_protect_ptr, hash_ptr, ccs_ptr, cdi_slot_arg, devid_slot_arg, hmac_slot_arg, \
	hmac_buffer_ptr, buffer_length_arg, flash_ptr, unlock_id_arg, unlock_nonce_ptr, \
	nonce_length_arg)   \
		security_manager_hsp_static_init_internal (SECURITY_MANAGER_HSP_API_INIT, state_ptr, \
			policy_ptr, locked_policy_ptr, policy_length_arg, aeb_ptr, fuses_ptr, aeb_addr_arg, \
			counter_addr_arg, counter_length_arg, mem_protect_ptr, hash_ptr, ccs_ptr, \
			cdi_slot_arg, devid_slot_arg, hmac_slot_arg, hmac_buffer_ptr, buffer_length_arg, \
			flash_ptr, unlock_id_arg, unlock_nonce_ptr, nonce_length_arg, true)

/**
 * Initialize a static instance of a manager for the security configuration of HSP-based devices.
 * Only APIs that are used to lock or unlock the device are supported.  Applying the unlocked policy
 * is not supported. This type of manager would typically be used by run-time firmware.
 *
 * These API calls are supported:
 * - lock_device
 * - unlock_device
 * - get_unlock_counter
 *
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the security manager.
 * @param policy_ptr The security policy handler for the device.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param counter_addr_arg Fuse address for the unlock anti-replay counter.  This address must be
 * 32-bit aligned.
 * @param counter_length_arg Length of the unlock counter.  This must be 32-bit aligned.
 * @param hash_ptr Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs_ptr CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param hmac_slot_arg KSU key slot that contains the HMAC key used to sign unlock policies stored
 * on flash.
 * @param hmac_buffer_ptr A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length_arg Length of the unlock policy HMAC buffer.  This must be large enough to
 * hold the entire authorized unlock data.
 * @param flash_ptr Flash storage used for device unlock policies.
 * @param unlock_id_arg Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce_ptr Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length_arg Length of the buffer for the one-time unlock nonce.  This must be at
 * least large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends
 * on the length of the HMAC key when the one-time unlock policy has been received.
 */
#define	security_manager_hsp_static_init_only_config_unlock(state_ptr, policy_ptr, fuses_ptr, \
	counter_addr_arg, counter_length_arg, hash_ptr, ccs_ptr, hmac_slot_arg, hmac_buffer_ptr, \
	buffer_length_arg, flash_ptr, unlock_id_arg, unlock_nonce_ptr, nonce_length_arg)    \
		security_manager_hsp_static_init_internal ( \
			SECURITY_MANAGER_HSP_ONLY_CONFIG_UNLOCK_API_INIT, state_ptr, policy_ptr, NULL, 0, \
			NULL, fuses_ptr, 0, counter_addr_arg, counter_length_arg, NULL, hash_ptr, ccs_ptr, 0, \
			0, hmac_slot_arg, hmac_buffer_ptr, buffer_length_arg, flash_ptr, unlock_id_arg, \
			unlock_nonce_ptr, nonce_length_arg, false)

/**
 * Initialize a static instance of a manager for the security configuration of HSP-based devices.
 * Only APIs that are used to load and apply an unlock policy are supported.  Locking or unlocking
 * the device is not supported.  This type of manager would typically be used at boot-time (1SP).
 *
 * These API calls are supported:
 * - has_unlock_policy
 * - load_security_policy
 * - apply_device_config
 *
 * This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the security manager.
 * @param policy_ptr The security policy handler for the device.
 * @param locked_policy_ptr The policy data that should loaded by the security policy when the
 * device is locked.  This must be in a format compatible with a call to
 * security_policy.parse_unlock_policy.
 * @param policy_length_arg Length of the locked policy data.
 * @param aeb_ptr Driver to configure HSP AEBs.
 * @param fuses_ptr Interface to the HSP fuses.
 * @param aeb_addr_arg Fuse address for the AEB fuses.  It's assumed there are 4 fuse words to
 * accommodate 32 fuse-backed AEBs.  This address must be 32-bit aligned.
 * @param counter_addr_arg Fuse address for the unlock anti-replay counter.  This address must be
 * 32-bit aligned.
 * @param counter_length_arg Length of the unlock counter.  This must be 32-bit aligned.
 * @param mem_protect_ptr Handler for configuring hardware memory protections.
 * @param hash_ptr Hash engine used during HMAC generation and verification of unlock policies.
 * @param ccs_ptr CCS and KSU driver used to manage unlock authorization and DICE keys.
 * @param cdi_slot_arg KSU key slot that contains the DICE CDI.
 * @param devid_slot_arg KSU key slot that contains the DICE Device ID.
 * @param hmac_slot_arg KSU key slot that contains the HMAC key used to sign unlock policies stored
 * on flash.
 * @param hmac_buffer_ptr A buffer to use for HMAC operations against unlock policy data.  This is
 * optional.  If null, the buffer will be dynamically allocated when needed.
 * @param buffer_length_arg Length of the unlock policy HMAC buffer.  This must be large enough to
 * hold the entire authorized unlock data.
 * @param flash_ptr Flash storage used for device unlock policies.
 * @param unlock_id_arg Block ID in flash storage that will be used for storing unlock policies.
 * @param unlock_nonce_ptr Buffer that contains the nonce for one-time unlock policies.  This memory
 * location must not be altered during device resets and must be 32-bit aligned.  This can be null
 * if one-time unlock policies are not supported.
 * @param nonce_length_arg Length of the buffer for the one-time unlock nonce.  This must be at
 * least large enough to hold an HMAC of the HSP unlock token nonce.  The length of the HMAC depends
 * on the length of the HMAC key when the one-time unlock policy has been received.
 */
#define	security_manager_hsp_static_init_only_apply_unlock(state_ptr, policy_ptr, \
	locked_policy_ptr, policy_length_arg, aeb_ptr, fuses_ptr, aeb_addr_arg, counter_addr_arg, \
	counter_length_arg, mem_protect_ptr, hash_ptr, ccs_ptr, cdi_slot_arg, devid_slot_arg, \
	hmac_slot_arg, hmac_buffer_ptr, buffer_length_arg, flash_ptr, unlock_id_arg, unlock_nonce_ptr, \
	nonce_length_arg)   \
		security_manager_hsp_static_init_internal ( \
			SECURITY_MANAGER_HSP_ONLY_APPLY_UNLOCK_API_INIT, state_ptr, policy_ptr, \
			locked_policy_ptr, policy_length_arg, aeb_ptr, fuses_ptr, aeb_addr_arg, \
			counter_addr_arg, counter_length_arg, mem_protect_ptr, hash_ptr, ccs_ptr, \
			cdi_slot_arg, devid_slot_arg, hmac_slot_arg, hmac_buffer_ptr, buffer_length_arg, \
			flash_ptr, unlock_id_arg, unlock_nonce_ptr, nonce_length_arg, true)


#endif	/* SECURITY_MANAGER_HSP_STATIC_H_ */
