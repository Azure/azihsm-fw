// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_STATIC_H_
#define OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_STATIC_H_

#include "ocp_recovery_device_variable_cms_svn.h"


/* Internal functions declared to allow for static initialization. */
int ocp_recovery_device_variable_cms_svn_get_size (
	const struct ocp_recovery_device_variable_cms *cms);
int ocp_recovery_device_variable_cms_svn_get_data (
	const struct ocp_recovery_device_variable_cms *cms, size_t offset, uint8_t *data,
	size_t length);


/**
 * Constant initializer for the variable CMS API.
 */
#define	OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_API_INIT  { \
		.get_size = ocp_recovery_device_variable_cms_svn_get_size, \
		.get_data = ocp_recovery_device_variable_cms_svn_get_data, \
	}


/**
 * Initialize a static instance of a variable CMS SVN wrapper.
 *
 * There is no validation done on the arguments.
 *
 * @param rot_ptr Interface to the RoT containing the SVN.  This can be a constant instance.
 */
#define	ocp_recovery_device_variable_cms_svn_static_init(rot_ptr)	{ \
		.base = OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_API_INIT, \
		.rot = rot_ptr, \
	}


#endif	/* OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_STATIC_H_ */
