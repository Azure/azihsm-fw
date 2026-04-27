// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "msft_protocol_logging.h"
#include "common/type_cast.h"
#include "msft_protocol/mctp_notifier_msft_mutable_list.h"


/**
 * Find and store the destination EID in first empty slot of the EID list.
 *
 * @param eid_list EID list array.
 * @param eid_list_len EID list array length.
 * @param dest_eid Destination EID to find in the list.
 *
 * @return 0 if the dest_eid stored into EID list slot, else -1
 */
static int mctp_notifier_msft_mutable_list_find_and_store_dest_eid (uint8_t *eid_list,
	uint8_t eid_list_len, uint8_t dest_eid)
{
	uint8_t i;

	for (i = 0; i < eid_list_len; i++) {
		// Check if the destination EID is already registered or if the slot is empty
		if ((eid_list[i] == dest_eid) || (eid_list[i] == MCTP_BASE_PROTOCOL_BROADCAST_EID)) {
			eid_list[i] = dest_eid;

			return 0;
		}
	}

	return -1;
}

/**
 * Register an EID in to the EID List gracefully or forcefully.
 *
 * @param notifier notifier interface
 * @param dest_eid Destination EID to register in the list.
 * @param force enable force register.
 *
 * @return returns 0 if success, else an error code
 */
static int mctp_notifier_msft_mutable_list_register_eid (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid, bool force)
{
	const struct mctp_notifier_msft *base_ptr = TO_DERIVED_TYPE (notifier,
		const struct mctp_notifier_msft, base);
	const struct mctp_notifier_msft_mutable_list *notify = TO_DERIVED_TYPE (base_ptr,
		const struct mctp_notifier_msft_mutable_list, base);
	int status;

	platform_mutex_lock (&notify->state->lock);

	status = mctp_notifier_msft_mutable_list_find_and_store_dest_eid (notify->eid_list,
		notify->eid_list_len, dest_eid);

	if (status == -1) {
		if (force == true) {
			// Max number of EIDs already registered, so forcefully register this new EID
			notify->eid_list[0] = dest_eid;
			status = 0;
		}
		else {
			// Max number of EIDs already registered, do not register this new EID
			status = MCTP_NOTIFIER_MAX_REGISTERED;
		}
	}

	platform_mutex_unlock (&notify->state->lock);

	return status;
}

int mctp_notifier_msft_mutable_list_send_notification_request (
	const struct mctp_notifier_interface *notifier,	uint8_t *payload, size_t payload_len)
{
	const struct mctp_notifier_msft *base_ptr = TO_DERIVED_TYPE (notifier,
		const struct mctp_notifier_msft, base);
	const struct mctp_notifier_msft_mutable_list *notify = TO_DERIVED_TYPE (base_ptr,
		const struct mctp_notifier_msft_mutable_list, base);
	uint8_t dest_eid;
	uint8_t i;
	int status = 0;

	if (notify == NULL) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	/* Optimize this in the future should we find scenarios where holding the lock for the
	 * entire loop duration is negatively impacting system behavior */
	platform_mutex_lock (&notify->state->lock);
	// Prepare and send notification to all registered EIDs
	for (i = 0; i < notify->eid_list_len; i++) {
		dest_eid = notify->eid_list[i];

		if (dest_eid == MCTP_BASE_PROTOCOL_BROADCAST_EID) {
			continue;
		}

		status = mctp_notifier_msft_send_notification_request (notifier, payload, payload_len,
			dest_eid);

		if (status != 0) {
			break;
		}
	}

	platform_mutex_unlock (&notify->state->lock);

	return status;
}

int mctp_notifier_msft_mutable_list_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	if ((notifier == NULL) || (dest_eid == MCTP_BASE_PROTOCOL_BROADCAST_EID)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	return mctp_notifier_msft_mutable_list_register_eid (notifier, dest_eid, false);
}

int mctp_notifier_msft_mutable_list_force_register_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	if ((notifier == NULL) || (dest_eid == MCTP_BASE_PROTOCOL_BROADCAST_EID)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	return mctp_notifier_msft_mutable_list_register_eid (notifier, dest_eid, true);
}

int mctp_notifier_msft_mutable_list_deregister_listener (
	const struct mctp_notifier_interface *notifier,	uint8_t dest_eid)
{
	const struct mctp_notifier_msft *base_ptr = TO_DERIVED_TYPE (notifier,
		const struct mctp_notifier_msft, base);
	const struct mctp_notifier_msft_mutable_list *notify = TO_DERIVED_TYPE (base_ptr,
		const struct mctp_notifier_msft_mutable_list, base);
	uint8_t i;
	int status;

	if ((notify == NULL) || (dest_eid == MCTP_BASE_PROTOCOL_BROADCAST_EID)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&notify->state->lock);

	status = MCTP_NOTIFIER_NOT_REGISTERED;

	// Check if registered, then remove by overwriting this entity from next onward entity list
	for (i = 0; i < notify->eid_list_len; i++) {
		if (notify->eid_list[i] == dest_eid) {
			notify->eid_list[i] = MCTP_BASE_PROTOCOL_BROADCAST_EID;
			status = 0;
			goto exit;
		}
	}

exit:
	platform_mutex_unlock (&notify->state->lock);

	return status;
}

/**
 * Initialize only the variable state for an MCTP notifier.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param notify The MCTP notifier MSFT mutable list instance that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int mctp_notifier_msft_mutable_list_init_state (
	const struct mctp_notifier_msft_mutable_list *notify)
{
	// Validate input instance
	if ((notify == NULL) || (notify->state == NULL) || (notify->base.transport == NULL) ||
		(notify->eid_list == NULL) || (notify->eid_list_len == 0) ||
		(notify->base.msg_buffer == NULL) ||
		(notify->base.msg_buffer_len == 0)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	memset (notify->state, 0, sizeof (struct mctp_notifier_msft_mutable_list_state));

	// Initialize eid_list with broadcast EID
	memset (notify->eid_list, MCTP_BASE_PROTOCOL_BROADCAST_EID, notify->eid_list_len);

	// Init Mutex for the MCTP notifier
	return platform_mutex_init (&notify->state->lock);
}

/**
 * Initialize MCTP notifier MSFT mutable list instance for sending MCTP notification.
 *
 * @param notify MCTP notifier mutable list instance.
 * @param transport msg_transport instance.
 * @param state mctp_notifier_state instance.
 * @param eid_list Buffer containing list of EIDs to be notified
 * @param eid_list_len Max number of EIDs that can register for notification
 * @param msg_buffer Message buffer for sending notifications
 * @param msg_buffer_len Length of the transmit message buffer
 * @param response_timeout_ms Response timeout value in ms
 *
 * @return 0 if the notifier initialized successfully or an error code.
 */
int mctp_notifier_msft_mutable_list_init (struct mctp_notifier_msft_mutable_list *notify,
	struct mctp_notifier_msft_mutable_list_state *state, const struct msg_transport *transport,
	uint8_t *eid_list, uint8_t eid_list_len, uint8_t *msg_buffer, uint32_t msg_buffer_len,
	uint32_t resp_timeout_ms)
{
	// Validate input instance
	if ((notify == NULL) || (state == NULL) || (transport == NULL) || (eid_list == NULL) ||
		(eid_list_len == 0) || (msg_buffer == NULL) || (msg_buffer_len == 0) ||
		(resp_timeout_ms == 0)) {
		return MCTP_NOTIFIER_INVALID_ARGUMENT;
	}

	memset (notify, 0, sizeof (*notify));

	notify->base.transport = transport;
	notify->state = state;
	notify->eid_list = eid_list;
	notify->eid_list_len = eid_list_len;
	notify->base.msg_buffer = msg_buffer;
	notify->base.msg_buffer_len = msg_buffer_len;
	notify->base.resp_timeout_ms = resp_timeout_ms;

	notify->base.base.register_listener = mctp_notifier_msft_mutable_list_register_listener;
	notify->base.base.force_register_listener =
		mctp_notifier_msft_mutable_list_force_register_listener;
	notify->base.base.deregister_listener = mctp_notifier_msft_mutable_list_deregister_listener;
	notify->base.base.send_notification_request =
		mctp_notifier_msft_mutable_list_send_notification_request;

	return mctp_notifier_msft_mutable_list_init_state (notify);
}

/**
 * Initialize MCTP notifier MSFT mutable list instance for sending MCTP notification, without logging send failures.
 *
 * There could be cases where the notifier is sending notification to an endpoint which does not
 * have support for the particular notification or the endpoint is down, in those cases the send_notification_request
 * will fail and log an error. This initializer can be used to create a notifier instance which can skip logging send
 * failure for the notification.
 *
 * @param notify MCTP notifier mutable list instance.
 * @param transport msg_transport instance.
 * @param state mctp_notifier_state instance.
 * @param eid_list Buffer containing list of EIDs to be notified
 * @param eid_list_len Max number of EIDs that can register for notification
 * @param msg_buffer Message buffer for sending notifications
 * @param msg_buffer_len Length of the transmit message buffer
 * @param response_timeout_ms Response timeout value in ms
 *
 * @return 0 if the notifier initialized successfully or an error code.
 */
int mctp_notifier_msft_mutable_list_init_no_fail_log (
	struct mctp_notifier_msft_mutable_list *notify,
	struct mctp_notifier_msft_mutable_list_state *state, const struct msg_transport *transport,
	uint8_t *eid_list, uint8_t eid_list_len, uint8_t *msg_buffer, uint32_t msg_buffer_len,
	uint32_t resp_timeout_ms)
{
	int status;

	status = mctp_notifier_msft_mutable_list_init (notify, state, transport, eid_list, eid_list_len,
		msg_buffer, msg_buffer_len, resp_timeout_ms);
	if (status != 0) {
		return status;
	}

	notify->base.skip_send_fail_log = true;

	return 0;
}

/**
 * Release the MCTP notifier mutable list instance.
 * Release is not thread-safe and can be called only after there are no operations in flight.
 *
 * @param notify MCTP notifier mutable list instance.
 *
 */
void mctp_notifier_msft_mutable_list_release (const struct mctp_notifier_msft_mutable_list *notify)
{
	if ((notify == NULL) || (notify->state == NULL)) {
		return;
	}

	// Release Mutex for the MCTP notifier
	platform_mutex_free (&notify->state->lock);
}
