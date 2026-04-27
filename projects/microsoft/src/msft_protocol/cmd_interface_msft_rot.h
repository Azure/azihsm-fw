// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMD_INTERFACE_MSFT_ROT_H_
#define CMD_INTERFACE_MSFT_ROT_H_

#include "cmd_interface/cmd_background.h"
#include "intrusion/intrusion_state.h"
#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/cmd_interface_msft.h"
#include "system/real_time_clock.h"
#include "system/secure_device_unlock.h"


/**
 * Handler for MSFT RoT command set requests.
 */
struct cmd_interface_msft_rot {
	struct cmd_interface base;						/**< Base command handler API instance. */
	const struct secure_device_unlock *unlock;		/**< Handler for device unlock requests. */
	const struct real_time_clock *rtc;				/**< Handler for time requests. */
	const struct cmd_background *reboot;			/**< Handler for device warm resets. */
	const struct intrusion_state *intrusion;		/**< Handler for intrusion state. */
	const struct mctp_notifier_interface *notifier;	/**< Handler for intrusion event notifications. */
	const struct cmd_interface *cmd_log;			/**< Handler for RoT log interface */
};


int cmd_interface_msft_rot_init (struct cmd_interface_msft_rot *intf,
	const struct secure_device_unlock *unlock, const struct real_time_clock *rtc,
	const struct cmd_background *reboot, const struct intrusion_state *intrusion,
	const struct mctp_notifier_interface *notifier, const struct cmd_interface *cmd_log);
void cmd_interface_msft_rot_release (const struct cmd_interface_msft_rot *intf);


#endif	/* CMD_INTERFACE_MSFT_ROT_H_ */
