// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_mailbox_protocol.h"
#include "common/unused.h"
#include "logging/hsp_logging.h"

/* Forward declaration */
int hsp_mailbox_protocol_flush (const struct hsp_mailbox_protocol_interface *protocol);

int hsp_mailbox_protocol_send (const struct hsp_mailbox_protocol_interface *protocol,
	const struct hsp_mailbox_msg *message)
{
	int status;
	bool is_valid;
	bool is_err;
	uint32_t fifo_count;
	size_t idx;
	const struct hsp_mailbox_protocol *mb_prot = (const struct hsp_mailbox_protocol*) protocol;

	/* Validate input parameters */
	if ((mb_prot == NULL) || (message == NULL)) {
		return HSP_MAILBOX_PROTOCOL_INVALID_ARGUMENT;
	}

	/* Ensure the size specified in the message is valid */
	if ((message->msg_sz == 0) || (message->msg_sz > HSP_MAILBOX_MAX_SIZE))	{
		return HSP_MAILBOX_PROTOCOL_SEND_INVALID_SIZE;
	}

	/* Get the send mailbox status info*/
	status = mb_prot->mailbox->send_get_status (mb_prot->mailbox, &is_valid, &is_err, &fifo_count);
	if (status != 0) {
		return status;
	}

	/* Ensure err bit is not set before sending */
	if (is_err)	{
		return HSP_MAILBOX_PROTOCOL_SEND_ERR_BIT_SET;
	}

	/* Ensure fifo_valid bit is not set before sending */
	if (is_valid) {
		return HSP_MAILBOX_PROTOCOL_SEND_FIFO_VALID_SET;
	}

	/* Ensure fifo is empty before sending */
	if (fifo_count > 0)	{
		return HSP_MAILBOX_PROTOCOL_SEND_FIFO_NOT_EMPTY;
	}

	/* Send each word in the message */
	for (idx = 0; idx < message->msg_sz; idx++) {
		status = mb_prot->mailbox->send_fifo_push (mb_prot->mailbox, message->msg[idx]);
		if (status != 0) {
			return status;
		}
	}

	/* Set the fifo_valid bit */
	status = mb_prot->mailbox->send_set_valid (mb_prot->mailbox);
	if (status != 0) {
		return status;
	}

	return 0;
}

int hsp_mailbox_protocol_request_flush (const struct hsp_mailbox_protocol_interface *protocol)
{
	int status;
	const struct hsp_mailbox_protocol *mb_prot = (const struct hsp_mailbox_protocol*) protocol;

	/* Validate input parameters */
	if (mb_prot == NULL) {
		return HSP_MAILBOX_PROTOCOL_INVALID_ARGUMENT;
	}

	/* Set the err bit */
	status = mb_prot->mailbox->send_set_err (mb_prot->mailbox);
	if (status != 0) {
		return status;
	}

	return 0;
}

int hsp_mailbox_protocol_receive (const struct hsp_mailbox_protocol_interface *protocol,
	struct hsp_mailbox_msg *message)
{
	int status;
	bool is_valid;
	bool is_err;
	uint32_t fifo_count;
	uint32_t idx;
	const struct hsp_mailbox_protocol *mb_prot = (const struct hsp_mailbox_protocol*) protocol;
	int flush_status;

	/* Validate input parameters */
	if ((mb_prot == NULL) || (message == NULL)) {
		return HSP_MAILBOX_PROTOCOL_INVALID_ARGUMENT;
	}

	/* Clear the receive size in the message */
	message->msg_sz = 0;

	/* Get the recv mailbox status info*/
	status = mb_prot->mailbox->recv_get_status (mb_prot->mailbox, &is_valid, &is_err, &fifo_count);
	if (status != 0) {
		return status;
	}

	/* Check for err bit */
	if (is_err)	{
		status = HSP_MAILBOX_PROTOCOL_RECV_ERR_BIT_SET;
		goto flush;
	}

	/* Check for fifo_valid */
	if (!is_valid) {
		return HSP_MAILBOX_PROTOCOL_RECV_FIFO_NOT_VALID;
	}

	/* Validate the fifo count */
	if (fifo_count == 0) {
		status = HSP_MAILBOX_PROTOCOL_RECV_FIFO_NO_DATA;
		goto flush;
	}
	if (fifo_count > HSP_MAILBOX_MAX_SIZE) {
		status = HSP_MAILBOX_PROTOCOL_RECV_FIFO_OVERFLOW;
		goto flush;
	}

	/* Receive each word in the message */
	for (idx = 0; idx < fifo_count; idx++) {
		status = mb_prot->mailbox->recv_fifo_pop (mb_prot->mailbox, &message->msg[idx]);
		if (status != 0) {
			return status;
		}
	}

	/* Set the message size */
	message->msg_sz = fifo_count;

	/* Clear the fifo_valid bit */
	status = mb_prot->mailbox->recv_clear_valid (mb_prot->mailbox);
	if (status != 0) {
		return status;
	}

	return 0;

flush:
	flush_status = hsp_mailbox_protocol_flush (protocol);
	if (flush_status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_MBOX_PROT_READ_FAILED_FLUSH, flush_status, 0);
	}

	return status;
}

int hsp_mailbox_protocol_flush (const struct hsp_mailbox_protocol_interface *protocol)
{
	int status;
	uint32_t fifo_count;
	uint32_t fifo_data;
	uint32_t idx;
	const struct hsp_mailbox_protocol *mb_prot = (const struct hsp_mailbox_protocol*) protocol;

	/* Validate input parameters */
	if (mb_prot == NULL) {
		return HSP_MAILBOX_PROTOCOL_INVALID_ARGUMENT;
	}

	/* Read the fifo count */
	status = mb_prot->mailbox->recv_get_status (mb_prot->mailbox, NULL, NULL, &fifo_count);
	if (status != 0) {
		return status;
	}

	/* Drain the fifo */
	for (idx = 0; idx < fifo_count; idx++) {
		status = mb_prot->mailbox->recv_fifo_pop (mb_prot->mailbox, &fifo_data);
		if (status != 0) {
			return status;
		}
	}

	/* Clear the fifo valid bit */
	status = mb_prot->mailbox->recv_clear_valid (mb_prot->mailbox);
	if (status != 0) {
		return status;
	}

	/* Clear the err bit */
	status = mb_prot->mailbox->recv_clear_err (mb_prot->mailbox);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the HSP mailbox protocol.
 *
 * @param protocol The protocol instance to initialize.
 * @param mailbox The mailbox instance to use.
 *
 * @return 0 if the HSP mailbox protocol was initialized or an error code.
 */
int hsp_mailbox_protocol_init (struct hsp_mailbox_protocol *protocol,
	const struct hsp_mailbox_interface *mailbox)
{
	if ((protocol == NULL) || (mailbox == NULL)) {
		return HSP_MAILBOX_PROTOCOL_INVALID_ARGUMENT;
	}

	memset (protocol, 0, sizeof (*protocol));

	protocol->base.send = hsp_mailbox_protocol_send;
	protocol->base.request_flush = hsp_mailbox_protocol_request_flush;
	protocol->base.receive = hsp_mailbox_protocol_receive;
	protocol->base.flush = hsp_mailbox_protocol_flush;

	protocol->mailbox = mailbox;

	return 0;
}

/**
 * Release the resources used by a HSP mailbox protocol instance.
 *
 * @param protocol The mailbox protocol instance to release.
 */
void hsp_mailbox_protocol_release (const struct hsp_mailbox_protocol *protocol)
{
	UNUSED (protocol);
}
