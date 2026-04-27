// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_H_
#define CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_H_

#include "cmd_interface/cmd_background.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "fips/fips_commands.h"
#include "fips/fips_self_test_interface.h"


/**
 * The list of supported messages types for the FIPS on-demand self-test command handler.
 *
 * This is a list of cmd_interface_multi_handler_msg_type entries that can be used with a
 * cmd_interface_multi_handler instance.  This list can be used by itself when initializing the
 * array that will be passed to the cmd_interface_multi_handler instance or can be used along with
 * other cmd_interface_multi_handler_msg_type instances in the same array.
 *
 * @param intf_ptr Pointer to the on-demand self-test handler to register with the supported message
 * types.  This must be a cmd_interface_msft_fips_on_demand_self_test instance.
 *
 * @return List of supported FIPS on-demand self-test commands to register with a
 * cmd_interface_multi_handler.
 */
#define	CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_SUPPORTED_MSG_TYPES(intf_ptr)  \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_ON_DEMAND_SELF_TEST, \
		&(intf_ptr)->base), \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_ON_DEMAND_SELF_TEST_RESULT, \
		&(intf_ptr)->base)


/**
 * Handler for MSFT FIPS command set requests to execute on-demand self-tests.
 */
struct cmd_interface_msft_fips_on_demand_self_test {
	struct cmd_interface base;							/**< Base command handler API instance. */
	const struct fips_self_test_interface *self_test;	/**< The interface for executing FIPS self-tests. */
};


int cmd_interface_msft_fips_on_demand_self_test_init (
	struct cmd_interface_msft_fips_on_demand_self_test *intf,
	const struct fips_self_test_interface *self_test);
void cmd_interface_msft_fips_on_demand_self_test_release (
	const struct cmd_interface_msft_fips_on_demand_self_test *intf);


#endif	/* CMD_INTERFACE_MSFT_FIPS_ON_DEMAND_SELF_TEST_H_ */
