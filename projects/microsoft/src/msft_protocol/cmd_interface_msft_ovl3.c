#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_msft_ovl3.h"
#include "ovl3_commands.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft_base.h"


int cmd_interface_msft_ovl3_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_msft_ovl3 *msft =
		(const struct cmd_interface_msft_ovl3*) intf;
	const struct msft_mctp_protocol_header *header;
	int status;

	if ((msft == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	/* There is no need to check the header length or validate contents in this handler.  That would
	 * have been already done by an earlier layer of the stack before passing the message here. */
	header = (const struct msft_mctp_protocol_header*) request->payload;

	switch (header->command) {
		case MSFT_PROTOCOL_OVL3_SOC_FLASH_ERASE:
			status = ovl3_soc_flash_erase_omc (msft->omc_bgnd, request);
			break;

		case MSFT_PROTOCOL_OVL3_SOC_FLASH_ERASE_STATUS:
			status = ovl3_get_soc_flash_erase_status_omc (msft->omc_bgnd, request);
			break;

		case MSFT_PROTOCOL_OVL3_SOC_IMAGE_PARTITIONS_ERASE:
			status = ovl3_soc_image_partitions_erase_omc (msft->omc_bgnd, request);
			break;

		case MSFT_PROTOCOL_OVL3_SOC_IMAGE_PARTITIONS_ERASE_STATUS:
			status = ovl3_get_soc_image_partitions_erase_status_omc (msft->omc_bgnd, request);
			break;

		default:
			status = CMD_HANDLER_MSFT_UNKNOWN_COMMAND;
	}

	return status;
}

/**
 * Initialize a command handler for MSFT OVL3 command set requests.
 *
 * @param intf The command handler to initialize.
 * @param shutdown_ctrl The handler to shutdown gracefully.
 * @return 0 if the handler was initialized successfully or an error code.
 */
int cmd_interface_msft_ovl3_init (struct cmd_interface_msft_ovl3 *intf,
	struct omc_background *background)
{
	if ((intf == NULL) || (background == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	memset (intf, 0, sizeof (struct cmd_interface_msft_ovl3));

	intf->base.process_request = cmd_interface_msft_ovl3_process_request;

	intf->omc_bgnd = background;

	return 0;
}

/**
 * Release the resources used by a MSFT OVL3 command set handler.
 *
 * @param intf The command handler to release.
 */
void cmd_interface_msft_ovl3_release (const struct cmd_interface_msft_ovl3 *intf)
{
	UNUSED (intf);
}
