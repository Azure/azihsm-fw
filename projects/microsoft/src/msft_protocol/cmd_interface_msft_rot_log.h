// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_ROT_LOG_H_
#define CMD_INTERFACE_MSFT_ROT_LOG_H_

#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/cmd_interface_msft.h"


/**
 * Handler for MSFT RoT log command set requests.
 */
struct cmd_interface_msft_rot_log {
	struct cmd_interface base;				/**< Base command handler API instance. */
	const struct rot_log_interface *log;	/**< Handler for RoT log command */
	const struct hash_engine *hash;			/**< The hashing engine for hash of log data. */
};


int cmd_interface_msft_rot_log_init (struct cmd_interface_msft_rot_log *intf,
	const struct rot_log_interface *log, const struct hash_engine *hash);
void cmd_interface_msft_rot_log_release (const struct cmd_interface_msft_rot_log *intf);


#endif	/* CMD_INTERFACE_MSFT_ROT_LOG_H_ */
