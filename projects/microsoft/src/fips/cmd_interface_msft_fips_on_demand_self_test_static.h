// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_STATIC_H_
#define CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_STATIC_H_

#include "cmd_interface_msft_fips_on_demand_self_test.h"
#include "msft_protocol/cmd_interface_msft_base_static.h"


/* Internal functions declared to allow for static initialization. */
int cmd_interface_msft_fips_on_demand_self_test_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request);


/**
 * Constant initializer for the command interface API.
 */
#define	CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_API_INIT { \
		.process_request = cmd_interface_msft_fips_on_demand_self_test_process_request, \
	}

/**
 * Initializes a static instance of a MSFT FIPS command set handler for on-demand self-test
 * commands.
 *
 * There is no validation done on the arguments.
 *
 * @param self_test_ptr The FIPS self-test handler for executing on-demand self-tests.
 */
#define	cmd_interface_msft_fips_on_demand_self_test_static_init(self_test_ptr) { \
		.base = CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_API_INIT, \
		.self_test = self_test_ptr, \
	}


#endif	/* CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_STATIC_H_ */
