// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "msft_mctp_protocol.h"
#include "msg_transport_msft.h"
#include "common/unused.h"


int msg_transport_msft_send_request_message (const struct msg_transport *transport,
	struct cmd_interface_msg *request, uint32_t timeout_ms, struct cmd_interface_msg *response)
{
	const struct msg_transport_msft *msft = (const struct msg_transport_msft*) transport;
	uint32_t message_type;
	int status;

	if ((msft == NULL) || (request == NULL) || (response == NULL)) {
		return MSG_TRANSPORT_INVALID_ARGUMENT;
	}

	status = cmd_interface_protocol_msft_add_header (msft->protocol, request);
	if (status != 0) {
		return status;
	}

	status = msft->base.next->send_request_message (msft->base.next, request, timeout_ms, response);
	if (status != 0) {
		return status;
	}

	status = msft->protocol->base.parse_message (&msft->protocol->base, response, &message_type);
	if (status != 0) {
		return status;
	}

	/* MVDP request and response messages have different header lengths.  The protocol handler
	 * parsing only guarantees enough data for the request header, which is smaller.  Since this is
	 * a response message, check to see if there is enough payload data. */
	if (response->payload_length < sizeof (struct msft_mctp_protocol_response_header)) {
		return MSG_TRANSPORT_RESPONSE_TOO_SHORT;
	}

	return 0;
}

/**
 * Initialize a transport for Microsoft Vendor Defined Protocol (MVDP) messages.  This only handles
 * MVDP message encapsulation.  The actual transmission protocol for the message would be a
 * different layer.
 *
 * @param msft The MVDP message transport to initialize.
 * @param transport The transport interface for the message transmission protocol used to send the
 * messages.
 * @param protocol Protocol handler for MVDP messages.
 *
 * @return 0 if the message transport was initialized successfully or an error code.
 */
int msg_transport_msft_init (struct msg_transport_msft *msft, const struct msg_transport *transport,
	const struct cmd_interface_protocol_msft *protocol)
{
	int status;

	if ((msft == NULL) || (protocol == NULL)) {
		return MSG_TRANSPORT_INVALID_ARGUMENT;
	}

	memset (msft, 0, sizeof (*msft));

	status = msg_transport_intermediate_init (&msft->base, transport,
		sizeof (struct msft_mctp_protocol_response_header));
	if (status == 0) {
		msft->base.base.send_request_message = msg_transport_msft_send_request_message;

		msft->protocol = protocol;
	}

	return status;
}

/**
 * Release the resources used by a MVDP message transport.
 *
 * @param msft The MVDP message transport to release.
 */
void msg_transport_msft_release (const struct msg_transport_msft *msft)
{
	msg_transport_intermediate_release (&msft->base);
}
