// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "msft_protocol_logging.h"
#include "platform_api.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "msft_protocol/mctp_notifier_msft_const_list.h"

int mctp_notifier_msft_const_list_send_notification_request (
	const struct mctp_notifier_interface *notifier,	uint8_t *payload, size_t payload_len)
{
	const struct mctp_notifier_msft *base_ptr = TO_DERIVED_TYPE (notifier,
		const struct mctp_notifier_msft, base);
	const struct mctp_notifier_msft_const_list *notify = TO_DERIVED_TYPE (base_ptr,
		const struct mctp_notifier_msft_const_list, base);
	uint8_t i;
	int status;

	if (notify == NULL) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	// Prepare and send notification to all registered EIDs
	for (i = 0; i < notify->eid_list_len; i++) {
		status = mctp_notifier_msft_send_notification_request (notifier, payload, payload_len,
			notify->eid_list[i]);

		if (status != 0) {
			return status;
		}
	}

	return 0;
}

int mctp_notifier_msft_const_list_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	UNUSED (notifier);
	UNUSED (dest_eid);

	return MCTP_NOTIFIER_UNSUPPORTED_OPERATION;
}

int mctp_notifier_msft_const_list_force_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	UNUSED (notifier);
	UNUSED (dest_eid);

	return MCTP_NOTIFIER_UNSUPPORTED_OPERATION;
}

int mctp_notifier_msft_const_list_deregister_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	UNUSED (notifier);
	UNUSED (dest_eid);

	return MCTP_NOTIFIER_UNSUPPORTED_OPERATION;
}

/**
 * Initialize MCTP notifier MSFT const list instance for sending MCTP notification.
 *
 * @param notify MCTP notifier const list instance.
 * @param transport msg_transport instance.
 * @param eid_list Buffer containing list of EIDs to be notified
 * @param eid_list_len Max number of EIDs that can register for notification
 * @param msg_buffer Message buffer for sending notifications
 * @param msg_buffer_len Length of the transmit message buffer
 * @param response_timeout_ms Response timeout value in ms
 *
 * @return 0 if the notifier initialized successfully or an error code.
 */
int mctp_notifier_msft_const_list_init (struct mctp_notifier_msft_const_list *notify,
	const struct msg_transport *transport, const uint8_t *eid_list, uint8_t eid_list_len,
	uint8_t *msg_buffer, uint32_t msg_buffer_len, uint32_t resp_timeout_ms)
{
	// Validate input instance
	if ((notify == NULL) || (transport == NULL) || (eid_list == NULL) ||
		(eid_list_len == 0) || (msg_buffer == NULL) || (msg_buffer_len == 0) ||
		(resp_timeout_ms == 0)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	memset (notify, 0, sizeof (*notify));

	notify->base.transport = transport;
	notify->eid_list = eid_list;
	notify->eid_list_len = eid_list_len;
	notify->base.msg_buffer = msg_buffer;
	notify->base.msg_buffer_len = msg_buffer_len;
	notify->base.resp_timeout_ms = resp_timeout_ms;
	notify->base.skip_send_fail_log = false;

	notify->base.base.register_listener = mctp_notifier_msft_const_list_register_listener;
	notify->base.base.force_register_listener =
		mctp_notifier_msft_const_list_force_register_listener;
	notify->base.base.deregister_listener = mctp_notifier_msft_const_list_deregister_listener;
	notify->base.base.send_notification_request =
		mctp_notifier_msft_const_list_send_notification_request;

	return 0;
}

/**
 * Initialize MCTP notifier MSFT const list instance for sending MCTP notification, without logging send failures.
 *
 * There could be cases where the notifier is sending notification to an endpoint which does not
 * have support for the particular notification or the endpoint is down, in those cases the send_notification_request
 * will fail and log an error. This initializer can be used to create a notifier instance which can skip logging send
 * failure for the notification.
 *
 * @param notify MCTP notifier const list instance.
 * @param transport msg_transport instance.
 * @param eid_list Buffer containing list of EIDs to be notified
 * @param eid_list_len Max number of EIDs that can register for notification
 * @param msg_buffer Message buffer for sending notifications
 * @param msg_buffer_len Length of the transmit message buffer
 * @param response_timeout_ms Response timeout value in ms
 *
 * @return 0 if the notifier initialized successfully or an error code.
 */
int mctp_notifier_msft_const_list_init_no_fail_log (struct mctp_notifier_msft_const_list *notify,
	const struct msg_transport *transport, const uint8_t *eid_list, uint8_t eid_list_len,
	uint8_t *msg_buffer, uint32_t msg_buffer_len, uint32_t resp_timeout_ms)
{
	int status;

	status = mctp_notifier_msft_const_list_init (notify, transport, eid_list, eid_list_len,
		msg_buffer, msg_buffer_len, resp_timeout_ms);
	if (status != 0) {
		return status;
	}

	notify->base.skip_send_fail_log = true;

	return 0;
}

/**
 * Release the MCTP notifier const list instance.
 *
 * @param notify MCTP notifier const list instance.
 *
 */
void mctp_notifier_msft_const_list_release (const struct mctp_notifier_msft_const_list *notify)
{
	UNUSED (notify);
}
