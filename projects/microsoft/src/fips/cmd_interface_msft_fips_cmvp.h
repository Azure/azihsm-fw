// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_CMVP_H_
#define CMD_INTERFACE_MSFT_FIPS_CMVP_H_

#include "cmd_interface/cmd_background.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "fips/cmvp_test_interface.h"
#include "fips/fips_commands.h"


/**
 * The list of supported messages types for the FIPS CMVP certification testing command handler.
 *
 * This is a list of cmd_interface_multi_handler_msg_type entries that can be used with a
 * cmd_interface_multi_handler instance.  This list can be used by itself when initializing the
 * array that will be passed to the cmd_interface_multi_handler instance or can be used along with
 * other cmd_interface_multi_handler_msg_type instances in the same array.
 *
 * @param intf_ptr Pointer to the CMVP command handler to register with the supported message types.
 * This must be a cmd_interface_msft_fips_cmvp instance.
 *
 * @return List of supported FIPS CMVP commands to register with a cmd_interface_multi_handler.
 */
#define	CMD_INTERFACE_MSFT_FIPS_CMVP_SUPPORTED_MSG_TYPES(intf_ptr)  \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_CMVP_TEST_CASE, &(intf_ptr)->base)


/**
 * Handler for MSFT FIPS command set requests involving CMVP certification test commands.
 */
struct cmd_interface_msft_fips_cmvp {
	struct cmd_interface base;					/**< Base command handler API instance. */
	const struct cmvp_test_interface *cmvp;		/**< The interface for executing CMVP tests. */
	const struct cmd_background *background;	/**< Background command handler. */
};


int cmd_interface_msft_fips_cmvp_init (struct cmd_interface_msft_fips_cmvp *intf,
	const struct cmvp_test_interface *cmvp, const struct cmd_background *background);
void cmd_interface_msft_fips_cmvp_release (const struct cmd_interface_msft_fips_cmvp *intf);


#endif	/* CMD_INTERFACE_MSFT_FIPS_CMVP_H_ */
