// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_BASE_H_
#define CMD_INTERFACE_MSFT_BASE_H_

#include "cmd_interface/device_manager.h"
#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/msft_base_commands.h"
#include "msft_protocol/temperature_sensor_cluster.h"


/**
 * Handler for MSFT Base command set requests.
 */
struct cmd_interface_msft_base {
	struct cmd_interface base;								/**< Base command handler API instance. */
	const struct msft_base_supported_command_set *cmd_sets;	/**< List of supported command sets. */
	size_t set_count;										/**< Number of supported command sets. */
	struct device_manager *device_mgr;						/**< Manager for known devices. */
	const struct temperature_sensor_cluster *temp_sensors;	/**< Manager for device temperature sensors. */
	const struct mctp_notifier_interface *hb_notifier;		/**< Handler for heartbeat event notifications. */
};


int cmd_interface_msft_base_init (struct cmd_interface_msft_base *intf,
	const struct msft_base_supported_command_set *cmd_sets, size_t set_count,
	struct device_manager *device_mgr, const struct temperature_sensor_cluster *cluster,
	const struct mctp_notifier_interface *hb_notifier);

void cmd_interface_msft_base_release (const struct cmd_interface_msft_base *intf);


#endif	/* CMD_INTERFACE_MSFT_BASE_H_ */
