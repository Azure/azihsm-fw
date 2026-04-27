// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_STATIC_H_
#define OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_STATIC_H_

#include "ocp_recovery_device_variable_cms_root_key_hash.h"


/* Internal functions declared to allow for static initialization. */
int ocp_recovery_device_variable_cms_root_key_hash_get_size (
	const struct ocp_recovery_device_variable_cms *cms);
int ocp_recovery_device_variable_cms_root_key_hash_get_data (
	const struct ocp_recovery_device_variable_cms *cms, size_t offset, uint8_t *data,
	size_t length);


/**
 * Constant initializer for the variable CMS API.
 */
#define	OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_API_INIT  { \
		.get_size = ocp_recovery_device_variable_cms_root_key_hash_get_size, \
		.get_data = ocp_recovery_device_variable_cms_root_key_hash_get_data, \
	}


/**
 * Initialize a static instance of a variable CMS root key wrapper.
 *
 * There is no validation done on the arguments.
 *
 * @param rot_ptr Interface to the RoT containing the root key information.  This can be a constant
 * instance.
 * @param hash_ptr Hash engine to use when retrieving the root key hash.  This can be a constant
 * instance.
 */
#define	ocp_recovery_device_variable_cms_root_key_hash_static_init(rot_ptr, hash_ptr)	{ \
		.base = OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_API_INIT, \
		.rot = rot_ptr, \
		.hash = hash_ptr, \
	}


#endif	/* OCP_RECOVERY_DEVICE_VARIABLE_CMS_ROOT_KEY_HASH_STATIC_H_ */
