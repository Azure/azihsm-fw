# ++
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.
#
#Module Name:
#
#	AllFeatures.cmake
#
# Abstract:
#
#	Defines the full set of compiler definitions necessary to enable all optional features of the
#	OneFleet HSP code.  This is mostly useful for unit testing scenarios where everything needs to
#	be enabled.
#
# --

set(
	ONEFLEET_HSP_ALL_FEATURES
		CCS_KSU_ENABLE_FIPS_CMVP_TESTING
		CCS_KSU_ENABLE_SEND_KEY
	)
