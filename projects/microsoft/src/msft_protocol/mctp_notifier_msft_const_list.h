// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MCTP_NOTIFIER_MSFT_CONST_LIST_H_
#define MCTP_NOTIFIER_MSFT_CONST_LIST_H_

#include <stdbool.h>
#include <stdint.h>
#include "cmd_interface/msg_transport.h"
#include "mctp/mctp_notifier_interface.h"
#include "msft_protocol/mctp_notifier_msft.h"


/**
 * MCTP notifier MSFT const list object. This will handle listener EID
 * register/deregister/send_notification by using mctp transport interface.
 */
struct mctp_notifier_msft_const_list {
	struct mctp_notifier_msft base;	/**< MCTP notifier MSFT base object */
	const uint8_t *eid_list;		/**< List of maximum allowable registered EIDs */
	uint8_t eid_list_len;			/**< Length of the above eid_list */
};


int mctp_notifier_msft_const_list_init (struct mctp_notifier_msft_const_list *notify,
	const struct msg_transport *transport, const uint8_t *eid_list, uint8_t eid_list_len,
	uint8_t *msg_buffer, uint32_t msg_buffer_len, uint32_t resp_timeout_ms);
int mctp_notifier_msft_const_list_init_no_fail_log (struct mctp_notifier_msft_const_list *notify,
	const struct msg_transport *transport, const uint8_t *eid_list, uint8_t eid_list_len,
	uint8_t *msg_buffer, uint32_t msg_buffer_len, uint32_t resp_timeout_ms);
void mctp_notifier_msft_const_list_release (const struct mctp_notifier_msft_const_list *notify);


#endif	/* MCTP_NOTIFIER_MSFT_CONST_LIST_H_ */
