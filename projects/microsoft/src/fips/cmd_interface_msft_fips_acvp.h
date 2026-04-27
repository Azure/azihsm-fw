// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_FIPS_ACVP_H_
#define CMD_INTERFACE_MSFT_FIPS_ACVP_H_

#include "acvp/acvp_proto_interface.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "fips/fips_commands.h"
#include "msft_protocol/cmd_interface_msft.h"


/**
 * The list of supported messages types for the ACVP command handler.
 *
 * This is a list of cmd_interface_multi_handler_msg_type entries that can be used with a
 * cmd_interface_multi_handler instance.  This list can be used by itself when initializing the
 * array that will be passed to the cmd_interface_multi_handler instance or can be used along with
 * other cmd_interface_multi_handler_msg_type instances in the same array.
 *
 * @param intf_ptr Pointer to the ACVP command handler to register with the supported message types.
 * This must be a cmd_interface_msft_fips_acvp instance.
 *
 * @return List of supported FIPS ACVP commands to register with a cmd_interface_multi_handler.
 */
#define	CMD_INTERFACE_MSFT_FIPS_ACVP_SUPPORTED_MSG_TYPES(intf_ptr)  \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_INIT_ACVP_TEST, &(intf_ptr)->base), \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_ACVP_TEST, &(intf_ptr)->base), \
	cmd_interface_multi_handler_msg_type_static_init (FIPS_CMD_GET_ACVP_TEST_RESULTS, \
		&(intf_ptr)->base)


/**
 * Handler for MSFT FIPS command set requests involving ACVP commands.
 */
struct cmd_interface_msft_fips_acvp {
	struct cmd_interface base;						/**< Base command handler API instance. */
	const struct acvp_proto_interface *acvp_proto;	/**< The interface to ACVP Proto. */
};


int cmd_interface_msft_fips_acvp_init (struct cmd_interface_msft_fips_acvp *intf,
	const struct acvp_proto_interface *acvp_proto);
void cmd_interface_msft_fips_acvp_release (const struct cmd_interface_msft_fips_acvp *intf);


#endif	/* CMD_INTERFACE_MSFT_FIPS_ACVP_H_ */
