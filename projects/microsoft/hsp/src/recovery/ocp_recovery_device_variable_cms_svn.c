// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ocp_recovery_device_variable_cms_svn.h"
#include "common/buffer_util.h"
#include "common/unused.h"


int ocp_recovery_device_variable_cms_svn_get_size (
	const struct ocp_recovery_device_variable_cms *cms)
{
	if (cms == NULL) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	/* Not every implementation will use all 64 bits of the SVN, but to make this interface simpler,
	 * just report the maximum size to cover all scenarios. */
	return sizeof (uint64_t);
}

int ocp_recovery_device_variable_cms_svn_get_data (
	const struct ocp_recovery_device_variable_cms *cms, size_t offset, uint8_t *data, size_t length)
{
	const struct ocp_recovery_device_variable_cms_svn *cms_svn =
		(const struct ocp_recovery_device_variable_cms_svn*) cms;
	uint64_t svn;
	int status;

	if ((cms_svn == NULL) || (data == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	status = cms_svn->rot->get_svn (cms_svn->rot, &svn);
	if (status != 0) {
		return status;
	}

	return buffer_copy ((uint8_t*) &svn, sizeof (svn), &offset, &length, data);
}

/**
 * Initialize an SVN wrapper for a variable CMS.
 *
 * @param cms The CMS interface to initialize.
 * @param rot Interface to the RoT that contains the SVN to access.
 *
 * @return 0 if the CMS SVN wrapper was initialized successfully or an error code.
 */
int ocp_recovery_device_variable_cms_svn_init (struct ocp_recovery_device_variable_cms_svn *cms,
	const struct hw_rot *rot)
{
	if ((cms == NULL) || (rot == NULL)) {
		return OCP_RECOVERY_DEVICE_INVALID_ARGUMENT;
	}

	memset (cms, 0, sizeof (struct ocp_recovery_device_variable_cms_svn));

	cms->base.get_size = ocp_recovery_device_variable_cms_svn_get_size;
	cms->base.get_data = ocp_recovery_device_variable_cms_svn_get_data;

	cms->rot = rot;

	return 0;
}

/**
 * Release the resources used for a variable CMS SVN wrapper.
 *
 * @param cms The CMS interface to release.
 */
void ocp_recovery_device_variable_cms_svn_release (
	const struct ocp_recovery_device_variable_cms_svn *cms)
{
	UNUSED (cms);
}
