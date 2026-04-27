// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_H_
#define OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_H_

#include "firmware/hw_rot.h"
#include "recovery/ocp_recovery_device.h"


/**
 * A variable CMS interface that reports the current SVN value from a HW RoT instance.
 */
struct ocp_recovery_device_variable_cms_svn {
	struct ocp_recovery_device_variable_cms base;	/**< The base CMS interface. */
	const struct hw_rot *rot;						/**< Interface to the HW RoT state. */
};


int ocp_recovery_device_variable_cms_svn_init (struct ocp_recovery_device_variable_cms_svn *cms,
	const struct hw_rot *rot);
void ocp_recovery_device_variable_cms_svn_release (
	const struct ocp_recovery_device_variable_cms_svn *cms);


#endif	/* OCP_RECOVERY_DEVICE_VARIABLE_CMS_SVN_H_ */
