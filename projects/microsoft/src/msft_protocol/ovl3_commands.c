#include <limits.h>
#include <string.h>
#include "ovl3_commands.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft.h"
#include "msft_protocol/msft_base_commands.h"


/**
 * Get the status of the SOC flash erase operation.
 *
 * @param handler OMC background handler to utilize.
 * @param request SOC Erase status request.
 *
 * @return 0 on success, 1 on in-progress, or a negative error code on failure.
 */
int ovl3_soc_flash_erase_omc (
	struct omc_background *handler, struct cmd_interface_msg *request)
{
	const struct ovl3_soc_flash_erase_request *req;
	struct ovl3_soc_flash_erase_response *resp;
	int status;

	if ((handler == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct ovl3_soc_flash_erase_request*) request->payload;

	if (req->header.protocol_version != OVL3_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct ovl3_soc_flash_erase_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = handler->soc_flash_erase (handler);
	if (status != 0) {
		return status;
	}

	resp = (struct ovl3_soc_flash_erase_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}


/**
 * Get the status of the SOC flash erase operation.
 *
 * @param handler OMC background handler to utilize.
 * @param request SOC Erase status request.
 *
 * @return 0 on success, 1 on in-progress, or a negative error code on failure.
 */
int ovl3_get_soc_flash_erase_status_omc (
	struct omc_background *handler, struct cmd_interface_msg *request)
{
	const struct ovl3_get_soc_flash_erase_status_request *req;
	struct ovl3_get_soc_flash_erase_status_response *resp;

	if ((request == NULL) || (handler == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct ovl3_get_soc_flash_erase_status_request*) request->payload;
	if (req->header.protocol_version != OVL3_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length !=
		sizeof (struct ovl3_get_soc_flash_erase_status_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct ovl3_get_soc_flash_erase_status_response*) request->payload;
	resp->erase_status = handler->get_soc_flash_erase_status (handler);

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}


/**
 * Erase the SOC flash image partitions (active and backup).
 *
 * @param handler OMC background handler to utilize.
 * @param request SOC image partitions erase request.
 *
 * @return 0 on success, 1 on in-progress, or a negative error code on failure.
 */
int ovl3_soc_image_partitions_erase_omc (
	struct omc_background *handler, struct cmd_interface_msg *request)
{
	const struct ovl3_soc_image_partitions_erase_request *req;
	struct ovl3_soc_image_partitions_erase_response *resp;
	int status;

	if ((handler == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct ovl3_soc_image_partitions_erase_request*) request->payload;

	if (req->header.protocol_version != OVL3_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct ovl3_soc_image_partitions_erase_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = handler->soc_image_partitions_erase (handler);
	if (status != 0) {
		return status;
	}

	resp = (struct ovl3_soc_image_partitions_erase_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}


/**
 * Get the status of the SOC image partitions erase operation.
 *
 * @param handler OMC background handler to utilize.
 * @param request SOC image partitions erase status request.
 *
 * @return 0 on success, 1 on in-progress, or a negative error code on failure.
 */
int ovl3_get_soc_image_partitions_erase_status_omc (
	struct omc_background *handler, struct cmd_interface_msg *request)
{
	const struct ovl3_get_soc_image_partitions_erase_status_request *req;
	struct ovl3_get_soc_image_partitions_erase_status_response *resp;

	if ((request == NULL) || (handler == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct ovl3_get_soc_image_partitions_erase_status_request*) request->payload;
	if (req->header.protocol_version != OVL3_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length !=
		sizeof (struct ovl3_get_soc_image_partitions_erase_status_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct ovl3_get_soc_image_partitions_erase_status_response*) request->payload;
	resp->erase_status = handler->get_soc_image_partitions_erase_status (handler);

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}
