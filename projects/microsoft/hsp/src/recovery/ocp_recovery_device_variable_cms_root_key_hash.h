// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_H_
#define OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_H_

#include "crypto/hash.h"
#include "firmware/hw_rot.h"
#include "recovery/ocp_recovery_device.h"


/**
 * A variable CMS interface that reports the current root key hash from a HW RoT instance.
 */
struct ocp_recovery_device_variable_cms_root_key_hash {
	struct ocp_recovery_device_variable_cms base;	/**< The base CMS interface. */
	const struct hw_rot *rot;						/**< Interface to the HW RoT state. */
	const struct hash_engine *hash;					/**< Hash engine for getting the root key hash. */
};


int ocp_recovery_device_variable_cms_root_key_hash_init (
	struct ocp_recovery_device_variable_cms_root_key_hash *cms, const struct hw_rot *rot,
	const struct hash_engine *hash);
void ocp_recovery_device_variable_cms_root_key_hash_release (
	const struct ocp_recovery_device_variable_cms_root_key_hash *cms);


#endif	/* OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_H_ */
