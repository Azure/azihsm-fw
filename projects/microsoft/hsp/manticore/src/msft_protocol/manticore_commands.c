#include <limits.h>
#include <string.h>
#include "manticore_commands.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "msft_protocol/cmd_interface_msft.h"
#include "msft_protocol/msft_base_commands.h"


/**
 * Process a set drain time request and create a response with the result.
 *
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int manticore_set_drain_time (struct cmd_interface_msg *request,
	const struct graceful_shutdown_control *shutdown_ctrl)
{
	const struct manticore_set_drain_time_request *req;
	struct manticore_set_drain_time_response *resp;
	uint32_t drain_time_ms = 0;
	int status;

	if ((request == NULL) || (shutdown_ctrl == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct manticore_set_drain_time_request*) request->payload;

	if (req->header.protocol_version != MANTICORE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct manticore_set_drain_time_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	drain_time_ms = buffer_unaligned_read32 (&req->drain_time_ms);

	status = shutdown_ctrl->set_allowed_drain_time (shutdown_ctrl, drain_time_ms);
	if (status != 0) {
		return status;
	}

	resp = (struct manticore_set_drain_time_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Process a get drain time request and create a response with the result.
 *
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int manticore_get_drain_time (struct cmd_interface_msg *request,
	const struct graceful_shutdown_control *shutdown_ctrl)
{
	const struct manticore_get_drain_time_request *req;
	struct manticore_get_drain_time_response *resp;
	int status;
	uint32_t drain_time_ms;

	if ((request == NULL) || (shutdown_ctrl == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (const struct manticore_get_drain_time_request*) request->payload;
	if (req->header.protocol_version != MANTICORE_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length !=
		sizeof (struct manticore_get_drain_time_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = shutdown_ctrl->get_allowed_drain_time (shutdown_ctrl, &drain_time_ms);
	if (status != 0) {
		return status;
	}

	resp = (struct manticore_get_drain_time_response*) request->payload;
	resp->drain_time_ms = drain_time_ms;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}
