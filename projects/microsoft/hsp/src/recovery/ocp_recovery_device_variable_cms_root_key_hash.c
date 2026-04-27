// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ocp_recovery_device_variable_cms_root_key_hash.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "splibs/inc/spcryptotypes.h"


int ocp_recovery_device_variable_cms_root_key_hash_get_size (
	const struct ocp_recovery_device_variable_cms *cms)
{
	if (cms == NULL) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	/* Make this simple for now and always get the SHA384 hash of the root key. */
	return SHA384_HASH_LENGTH;
}

int ocp_recovery_device_variable_cms_root_key_hash_get_data (
	const struct ocp_recovery_device_variable_cms *cms, size_t offset, uint8_t *data, size_t length)
{
	const struct ocp_recovery_device_variable_cms_root_key_hash *root_key =
		(const struct ocp_recovery_device_variable_cms_root_key_hash*) cms;
	SP_MSG_384 digest;
	int status;

	if ((root_key == NULL) || (data == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (digest.AsBytes, 0, SP_MSG_384_SIZE);

	status = root_key->rot->get_root_key_hash (root_key->rot, root_key->hash, HASH_TYPE_SHA384,
		digest.AsBytes, SP_MSG_384_SIZE);
	if ((status != 0) && (status != HW_ROT_UNSUPPORTED) && (status != HW_ROT_NO_ROOT_KEY)) {
		return status;
	}

	return buffer_copy (digest.AsBytes, SP_MSG_384_SIZE, &offset, &length, data);
}

/**
 * Initialize a root key wrapper for a variable CMS.
 *
 * @param cms The CMS interface to initialize.
 * @param rot Interface to the RoT that contains the root key information to access.
 * @param hash The hash engine to use with the RoT to retrieve the root key hash.
 *
 * @return 0 if the CMS root key wrapper was initialized successfully or an error code.
 */
int ocp_recovery_device_variable_cms_root_key_hash_init (
	struct ocp_recovery_device_variable_cms_root_key_hash *cms, const struct hw_rot *rot,
	const struct hash_engine *hash)
{
	if ((cms == NULL) || (rot == NULL) || (hash == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (cms, 0, sizeof (struct ocp_recovery_device_variable_cms_root_key_hash));

	cms->base.get_size = ocp_recovery_device_variable_cms_root_key_hash_get_size;
	cms->base.get_data = ocp_recovery_device_variable_cms_root_key_hash_get_data;

	cms->rot = rot;
	cms->hash = hash;

	return 0;
}

/**
 * Release the resources used for a variable CMS root key wrapper.
 *
 * @param cms The CMS interface to release.
 */
void ocp_recovery_device_variable_cms_root_key_hash_release (
	const struct ocp_recovery_device_variable_cms_root_key_hash *cms)
{
	UNUSED (cms);
}
