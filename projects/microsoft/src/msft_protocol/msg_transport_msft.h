// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSG_TRANSPORT_MSFT_H_
#define MSG_TRANSPORT_MSFT_H_

#include "cmd_interface/msg_transport_intermediate.h"
#include "msft_protocol/cmd_interface_protocol_msft.h"


/**
 * Defines a transport for sending Microsoft Vendor Defined Protocol (MVDP) messages.
 *
 * This layer handles the header bytes specific to all MVDP messages.  It does not populate or
 * manage specific command set or command code fields.  This data is expected to be part of the
 * message payload sent to this transport layer.
 */
struct msg_transport_msft {
	struct msg_transport_intermediate base;				/**< Base transport API. */
	const struct cmd_interface_protocol_msft *protocol;	/**< Protocol handler for MVDP messages. */
};


int msg_transport_msft_init (struct msg_transport_msft *msft, const struct msg_transport *transport,
	const struct cmd_interface_protocol_msft *protocol);
void msg_transport_msft_release (const struct msg_transport_msft *msft);


#endif	/* MSG_TRANSPORT_MSFT_H_ */
