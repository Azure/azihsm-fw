// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SECURITY_POLICY_HSP_MANTICORE_STATIC_H_
#define SECURITY_POLICY_HSP_MANTICORE_STATIC_H_

#include "system/security_policy_hsp_manticore.h"


/* Internal functions declared to allow for static initialization. */
int security_policy_hsp_manticore_is_persistent (const struct security_policy *policy);
int security_policy_hsp_manticore_enforce_firmware_signing (const struct security_policy *policy);
int security_policy_hsp_manticore_enforce_anti_rollback (const struct security_policy *policy);
int security_policy_hsp_manticore_check_unlock_persistence (const struct security_policy *policy,
	const uint8_t *unlock, size_t length);
int security_policy_hsp_manticore_parse_unlock_policy (const struct security_policy *policy,
	const uint8_t *unlock, size_t length);

int security_policy_hsp_manticore_get_enabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count);
int security_policy_hsp_manticore_get_disabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count);
int security_policy_hsp_manticore_get_locked_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count);
int security_policy_hsp_manticore_get_fuse_disabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb);
int security_policy_hsp_manticore_enforce_memory_fencing (const struct security_policy_hsp *policy);


/**
 * Constant initializer for the security policy base API.
 */
#define	SECURITY_POLICY_HSP_MANTICORE_API_INIT  { \
		.base = { \
			.is_persistent = security_policy_hsp_manticore_is_persistent, \
			.enforce_firmware_signing = security_policy_hsp_manticore_enforce_firmware_signing, \
			.enforce_anti_rollback = security_policy_hsp_manticore_enforce_anti_rollback, \
			.check_unlock_persistence = security_policy_hsp_manticore_check_unlock_persistence, \
			.parse_unlock_policy = security_policy_hsp_manticore_parse_unlock_policy, \
		}, \
		.get_enabled_aebs = security_policy_hsp_manticore_get_enabled_aebs, \
		.get_disabled_aebs = security_policy_hsp_manticore_get_disabled_aebs, \
		.get_locked_aebs = security_policy_hsp_manticore_get_locked_aebs, \
		.get_fuse_disabled_aebs = security_policy_hsp_manticore_get_fuse_disabled_aebs, \
		.enforce_memory_fencing = security_policy_hsp_manticore_enforce_memory_fencing, \
	}


/**
 * Initialize a static instance of a security policy for Manticore.  The security policy is mutable
 * and can be replaced with a new policy at run-time.
 *
 * There is no validation done on the arguments.
 *
 * @param data_ptr Buffer for the current security policy.  This may be pre-loaded with policy data,
 * but is not required.  The security policy can be updated by parsing an unlock policy.
 * @param socid_ptr The device SOCID.  This is used to differentiate between A0 and B0 devices.
 */
#define	security_policy_hsp_manticore_static_init(data_ptr, socid_ptr)	{ \
		.base = SECURITY_POLICY_HSP_MANTICORE_API_INIT, \
		.data = data_ptr, \
		.socid = socid_ptr, \
		.is_const = false, \
	}

/**
 * Initialize a static instance of a security policy for Manticore.  The security policy will remain
 * constant and cannot be changed.
 *
 * There is no validation done on the arguments.
 *
 * @param data_ptr Buffer for the current security policy.  This must contain valid policy data.
 * The contents cannot be updated at run-time.
 * @param socid_ptr The device SOCID.  This is used to differentiate between A0 and B0 devices.
 */
#define	security_policy_hsp_manticore_static_init_constant_policy(data_ptr, socid_ptr)	{ \
		.base = SECURITY_POLICY_HSP_MANTICORE_API_INIT, \
		.data = data_ptr, \
		.socid = socid_ptr, \
		.is_const = true, \
	}


#endif	/* SECURITY_POLICY_HSP_MANTICORE_STATIC_H_ */
