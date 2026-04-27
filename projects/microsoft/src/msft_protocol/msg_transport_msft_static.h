// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSG_TRANSPORT_MSFT_STATIC_H_
#define MSG_TRANSPORT_MSFT_STATIC_H_

#include "msft_mctp_protocol.h"
#include "msg_transport_msft.h"
#include "cmd_interface/msg_transport_intermediate_static.h"


/* Internal functions declared to allow for static initialization. */
int msg_transport_msft_send_request_message (const struct msg_transport *transport,
	struct cmd_interface_msg *request, uint32_t timeout_ms, struct cmd_interface_msg *response);


/**
 * Initialize a static transport for Microsoft Vendor Defined Protocol (MVDP) messages. This only
 * handles MVDP message encapsulation.  The actual transmission protocol for the message would be a
 * different layer.
 *
 * This can be a constant instance.  There is no validation done on the arguments.
 *
 * Initialize a transport for Microsoft Vendor Defined Protocol (MVDP) messages.
 *
 * @param transport_ptr The transport interface for the message transmission protocol used to send
 * the messages.
 * @param protocol_ptr Protocol handler for MVDP messages.
 */
#define	msg_transport_msft_static_init(transport_ptr, protocol_ptr) { \
		.base = msg_transport_intermediate_static_init (msg_transport_msft_send_request_message, \
			transport_ptr, sizeof (struct msft_mctp_protocol_response_header)), \
		.protocol = protocol_ptr, \
	}


#endif	/* MSG_TRANSPORT_MSFT_STATIC_H_ */
