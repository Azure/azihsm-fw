// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SECURITY_POLICY_HSP_MANTICORE_H_
#define SECURITY_POLICY_HSP_MANTICORE_H_

#include "system/security_policy_hsp.h"


/**
 * Manticore security policy data.  This matches the structure of the unlock policy.
 */
struct security_policy_hsp_manticore_data {
	uint8_t policy_type;	/**< Identifier indicating persistence of the policy. */
	uint8_t secure_boot;	/**< The secure boot policy for firmware loaded by the device. */
	uint8_t apply_aeb;		/**< The policy for applying AEB configuration. */
	uint8_t fencing;		/**< The policy for enforcing memory fencing. */
	uint8_t sprt_features;	/**< Control flags for run-time firmware features. */
	uint8_t reserved[3];	/**< Reserved. */
	uint32_t aeb_policy[4];	/**< The AEB configuration for the policy. */
	uint32_t aeb_lock[4];	/**< The AEB lock settings for the policy. */
};

/**
 * Handles security policy reporting and parsing for Manticore.
 */
struct security_policy_hsp_manticore {
	struct security_policy_hsp base;					/**< Base security policy API. */
	struct security_policy_hsp_manticore_data *data;	/**< Buffer for the current security policy. */
	const uint32_t *socid;								/**< The SOCID for the device. */
	bool is_const;										/**< Flag indicating whether the policy can be updated. */
};


extern const struct security_policy_hsp_manticore_data locked_device_policy;
extern const struct security_policy_hsp_manticore_data mfg_device_policy;
extern const struct security_policy_hsp_manticore_data recovery_device_policy;


int security_policy_hsp_manticore_init (struct security_policy_hsp_manticore *policy,
	struct security_policy_hsp_manticore_data *data, const uint32_t *socid);
int security_policy_hsp_manticore_init_constant_policy (
	struct security_policy_hsp_manticore *policy,
	const struct security_policy_hsp_manticore_data *data, const uint32_t *socid);
void security_policy_hsp_manticore_release (const struct security_policy_hsp_manticore *policy);

bool security_policy_hsp_manticore_allow_no_auth_factory_default (
	const struct security_policy_hsp_manticore *policy);
bool security_policy_hsp_manticore_allow_no_auth_manifest_erase (
	const struct security_policy_hsp_manticore *policy);
bool security_policy_hsp_manticore_allow_no_auth_intrusion_reset (
	const struct security_policy_hsp_manticore *policy);
bool security_policy_hsp_manticore_allow_no_auth_firmware_update (
	const struct security_policy_hsp_manticore *policy);


#endif	/* SECURITY_POLICY_HSP_MANTICORE_H_ */
