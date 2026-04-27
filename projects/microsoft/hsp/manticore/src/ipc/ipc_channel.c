// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "platform_api.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "ipc/ipc_channel.h"


/**
 * The function sends a message using an IPC channel and updates the producer index.
 *
 * @param ipc_channel A pointer to ipc channel instance
 * @param message pointer to Ipc message data to be sent.
 *
 * @return  0 if completed successfully or an error code.
 */
static int ipc_channel_send_message (const struct ipc_channel *ipc_channel,
	struct ipc_message *message)
{
	struct ipc_message_queue *tx_queue;
	uint32_t new_producer_index;
	uint32_t *int_reg_address;
	int status = 0;

	status = ipc_channel->dmb->map_soc_address (ipc_channel->dmb,
		(uint64_t) ipc_channel->tx_queue_base_addr,
		IPC_MESSAGE_QUEUE_SIZE (ipc_channel->tx_queue_length), HSP_DMB_ACCESS_WRITE,
		(void**) &tx_queue);
	if (status != 0) {
		return status;
	}

	do {
		/* Update the new producer index */
		new_producer_index = IPC_QUEUE_INDEX_INC (tx_queue->producer_index,
			ipc_channel->tx_queue_length);

		if (new_producer_index == tx_queue->consumer_index) {
			status = IPC_CHANNEL_TX_QUEUE_FULL;
			break;
		}

		/* Copy the message to the message queue */
		tx_queue->msg_queue[tx_queue->producer_index] = *message;
		tx_queue->producer_index = new_producer_index;

		/* Write the message using the descriptor */
		int_reg_address = (&ipc_channel->register_block->desc00DescReg00 +
			ipc_channel->tx_descriptor_id);

		*int_reg_address = new_producer_index;
		status = 0;
	} while (0);

	ipc_channel->dmb->unmap_soc_address (ipc_channel->dmb, tx_queue);

	return status;
}

/**
 * The function waits for message until timeout on IPC Channel. It will read message and copy in the
 * `message` buffer.
 *
 * @param ipc_channel A pointer to ipc channel instance
 * @param message pointer to Ipc message data to be sent.
 * @param timeout Timeout is in milliseconds and for waiting forever supply the value Zero in
 * timeout.
 *
 * @return  0 if completed successfully or an error code.
 */
static int ipc_channel_receive_message (const struct ipc_channel *ipc_channel,
	struct ipc_message *message, uint32_t timeout_ms)
{
	struct ipc_message_queue *rx_queue;
	uint32_t new_consumer_index;
	int status = 0;

	/* Wait until the request is received or timeout occurs. */
	status = platform_semaphore_wait (&ipc_channel->state->rx_wait, timeout_ms);
	if (status == 1) {
		return IPC_CHANNEL_RESP_TIMEOUT;
	}

	status = ipc_channel->dmb->map_soc_address (ipc_channel->dmb,
		(uint64_t) ipc_channel->rx_queue_base_addr,
		IPC_MESSAGE_QUEUE_SIZE (ipc_channel->rx_queue_length), HSP_DMB_ACCESS_WRITE,
		(void**) &rx_queue);
	if (status != 0) {
		return status;
	}

	do {
		if (rx_queue->consumer_index == rx_queue->producer_index) {
			status = IPC_CHANNEL_RX_QUEUE_EMPTY;
			break;
		}

		/* Read the message from the message queue */
		*message = rx_queue->msg_queue[rx_queue->consumer_index];

		/* Update the consumer index */
		new_consumer_index = IPC_QUEUE_INDEX_INC (rx_queue->consumer_index,
			ipc_channel->rx_queue_length);
		rx_queue->consumer_index = new_consumer_index;

		status = 0;
	} while (0);

	ipc_channel->dmb->unmap_soc_address (ipc_channel->dmb, rx_queue);

	return status;
}

/**
 * The function waits for message until timeout on IPC Channel. It will read message, and validate
 * the received message's tag, response bit and expected opcode value.
 *
 * @param ipc_channel A pointer to ipc channel instance
 * @param message pointer to Ipc message data to be sent.
 * @param timeout Timeout is in milliseconds and for waiting forever supply the value Zero in
 * timeout.
 *
 * @return  0 if completed successfully or an error code.
 */
static int ipc_channel_receive_and_validate_message (const struct ipc_channel *ipc_channel,
	struct ipc_message *message, uint32_t timeout_ms)
{
	struct ipc_message rx_msg;
	int status = 0;

	status = ipc_channel_receive_message (ipc_channel, &rx_msg, timeout_ms);
	if (status != 0) {
		return status;
	}

	/* Validate the Response bit of received message */
	if (IPC_MESSAGE_RESPONSE_BIT != rx_msg.header.response) {
		return IPC_CHANNEL_RX_INVALID_RESP_BIT;
	}

	/* Match tag value of send and received message */
	if (message->header.tag != rx_msg.header.tag) {
		return IPC_CHANNEL_RX_TAG_MISMATCHED;
	}

	/* Validate the Message Opcode */
	if (message->header.opcode != rx_msg.header.opcode) {
		return IPC_CHANNEL_RX_OPCODE_MISMATCHED;
	}

	/* Copy the read rx_msg to message */
	*message = rx_msg;

	return status;
}

int ipc_channel_send (const struct ipc_channel *ipc_channel, struct ipc_message *message)
{
	if ((ipc_channel == NULL) || (message == NULL)) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	return ipc_channel_send_message (ipc_channel, message);
}

int ipc_channel_receive (const struct ipc_channel *ipc_channel,	struct ipc_message *message,
	uint32_t timeout_ms)
{
	if ((ipc_channel == NULL) || (message == NULL)) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	return ipc_channel_receive_message (ipc_channel, message, timeout_ms);
}

int ipc_channel_send_and_receive (const struct ipc_channel *ipc_channel,
	struct ipc_message *message, uint32_t timeout_ms)
{
	struct ipc_message_queue *msg_queue;
	uint32_t *interrupt_pend_clr_reg = 0;
	uint32_t timediff_ms = 0;
	platform_clock timeout;
	int status = 0;

	if ((ipc_channel == NULL) || (message == NULL)) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	/* Acquire the IPC Channel to avoid race condition */
	ipc_channel->reserve (ipc_channel);

	/* Rx Queue data is not consumed than flush queue before sending the new request to other core */
	status = ipc_channel->dmb->map_soc_address (ipc_channel->dmb,
		(uint64_t) ipc_channel->rx_queue_base_addr,
		IPC_MESSAGE_QUEUE_SIZE (ipc_channel->rx_queue_length), HSP_DMB_ACCESS_WRITE,
		(void**) &msg_queue);
	if (status != 0) {
		ipc_channel->free (ipc_channel);

		return status;
	}

	/* flush the RX queue if it is not consumed */
	if (msg_queue->consumer_index != msg_queue->producer_index) {
		msg_queue->consumer_index = msg_queue->producer_index;
	}

	ipc_channel->dmb->unmap_soc_address (ipc_channel->dmb, msg_queue);

	/* Clear Pending Interrupt */
	interrupt_pend_clr_reg = &ipc_channel->register_block->intPendClr0 + ipc_channel->int_block_id;
	*interrupt_pend_clr_reg = (1 << ipc_channel->rx_descriptor_id);

	/* Enable the IPC Channel */
	ipc_channel->enable (ipc_channel);

	/* Update tag value for the transaction */
	ipc_channel->state->tag = IPC_MESSAGE_TAG_INC (ipc_channel->state->tag);

	/* Update tag value and Request bit in message header */
	message->header.tag = ipc_channel->state->tag;
	message->header.response = IPC_MESSAGE_REQUEST_BIT;

	/* Send Message Over IPC Channel */
	status = ipc_channel_send_message (ipc_channel, message);
	if (status != 0) {
		goto err_disable_channel;
	}

	timediff_ms = timeout_ms;
	if (timeout_ms != 0) {
		/* Initialize the timeout */
		status = platform_init_timeout (timeout_ms, &timeout);
		if (status != 0) {
			goto err_disable_channel;
		}
	}

	do {
		if (timeout_ms != 0) {
			status = platform_get_timeout_remaining (&timeout, &timediff_ms);
			if (status != 0) {
				break;
			}

			if (timediff_ms == 0) {
				status = IPC_CHANNEL_RESP_TIMEOUT;
				break;
			}
		}

		/* Read IPC Message and validate parameters */
		status = ipc_channel_receive_and_validate_message (ipc_channel, message, timediff_ms);
	} while (status != 0);

err_disable_channel:
	/* Disable the IPC Channel */
	ipc_channel->disable (ipc_channel);

	/* Free the IPC Channel */
	ipc_channel->free (ipc_channel);

	return status;
}

int ipc_channel_enable (const struct ipc_channel *ipc_channel)
{
	uint32_t int_value = 0;
	uint32_t *int_reg_address = 0;

	if (ipc_channel == NULL) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	/* Set the interrupt enable bit for the rx descriptor */
	int_value = (0x01 << (ipc_channel->rx_descriptor_id));
	int_reg_address = &ipc_channel->register_block->intEnabSet0 + ipc_channel->int_block_id;
	*int_reg_address |= int_value;

	return 0;
}

int ipc_channel_disable (const struct ipc_channel *ipc_channel)
{
	uint32_t int_value = 0;
	uint32_t *int_reg_address = 0;

	if (ipc_channel == NULL) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	/* Clear the interrupt clear bit for the rx descriptor */
	int_value = (0x01 << (ipc_channel->rx_descriptor_id));
	int_reg_address = &ipc_channel->register_block->intEnabClr0 + ipc_channel->int_block_id;
	*int_reg_address = int_value;

	return 0;
}

int ipc_channel_reserve (const struct ipc_channel *ipc_channel)
{
	if (ipc_channel == NULL) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	return platform_mutex_lock (&ipc_channel->state->lock);
}

int ipc_channel_free (const struct ipc_channel *ipc_channel)
{
	if (ipc_channel == NULL) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	return platform_mutex_unlock (&ipc_channel->state->lock);
}

/* It handles IPC interrupts for receive message of the IPC channel.  It will clear the interrupt
 * flag the for that IPC receive descriptor.  It will post the semaphore to IPC channel waiting on
 * the receive IPC message. */
bool ipc_channel_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	bool status = false;
	uint32_t *int_pend_reg = 0;
	uint32_t *int_pend_clr_reg = 0;
	uint32_t ipc_pend_value = 0;
	const struct ipc_channel *ipc_channel = TO_DERIVED_TYPE (handler, const struct ipc_channel,
		base);

	UNUSED (param);

	ipc_pend_value = (1 << ipc_channel->rx_descriptor_id);
	int_pend_reg = &ipc_channel->register_block->intPendSet0 + ipc_channel->int_block_id;

	/* Validated the interrupt */
	if (*int_pend_reg & ipc_pend_value) {
		/* Clear Pending interrupt */
		int_pend_clr_reg = &ipc_channel->register_block->intPendClr0 + ipc_channel->int_block_id;
		*int_pend_clr_reg = ipc_pend_value;

		platform_semaphore_post_from_isr (&ipc_channel->state->rx_wait);
		status = true;
	}

	return status;
}

/**
 * Initialize an IPC channel Object
 *
 * @param ipc_channel A pointer to ipc channel instance
 * @param ipc_channel_state A pointer to the IPC channel state
 * @param tx_queue_base_addr IPC message channel's transmit queue base address
 * @param tx_queue_length IPC message channel's transmit queue length
 * @param tx_descriptor_id Transmit Descriptor ID ranging from 0 to 31
 * @param rx_queue_base_addr IPC message channel's receive queue base address
 * @param rx_queue_length IPC message channel's receive queue length
 * @param rx_descriptor_id Receive Descriptor ID ranging from 0 to 31
 * @param dmb A pointer to HSP DMB object used to map and unmap memory regions
 * @param int_block_id Access the interrupt control registers in IPC hardware register block
 * @param ipc_int_registers A pointer to IPC hardware register block to control the IPC hardware.
 *
 * @return 0 if completed successfully or an error code.
 */
int ipc_channel_init (struct ipc_channel *ipc_channel, struct ipc_channel_state *ipc_channel_state,
	uint32_t tx_queue_base_addr, uint32_t tx_queue_length, int tx_descriptor_id,
	uint32_t rx_queue_base_addr, uint32_t rx_queue_length, int rx_descriptor_id,
	const struct hsp_dmb *dmb, uint32_t int_block_id, IntcIpc_t *ipc_interrupt_registers)
{
	int status = 0;

	if ((ipc_channel == NULL) || (ipc_channel_state == NULL) || (dmb == NULL) ||
		(ipc_interrupt_registers == NULL)) {
		return IPC_CHANNEL_INVALID_ARGUMENT;
	}

	memset (ipc_channel, 0, sizeof (*ipc_channel));

	/* Assign object to IPC Channel Object */
	ipc_channel->dmb = dmb;
	ipc_channel->state = ipc_channel_state;
	ipc_channel->register_block = ipc_interrupt_registers;

	/* IPC channel config parameter assignment */
	ipc_channel->tx_queue_base_addr = tx_queue_base_addr;
	ipc_channel->tx_queue_length = tx_queue_length;
	ipc_channel->tx_descriptor_id = tx_descriptor_id;

	ipc_channel->rx_queue_base_addr = rx_queue_base_addr;
	ipc_channel->rx_queue_length = rx_queue_length;
	ipc_channel->rx_descriptor_id = rx_descriptor_id;

	ipc_channel->int_block_id = int_block_id;

	/* Tag Initialization */
	ipc_channel->state->tag = 0;

	/* IPC channel API mapping */
	ipc_channel->enable = ipc_channel_enable;
	ipc_channel->disable = ipc_channel_disable;
	ipc_channel->send = ipc_channel_send;
	ipc_channel->receive = ipc_channel_receive;
	ipc_channel->send_and_receive = ipc_channel_send_and_receive;
	ipc_channel->reserve = ipc_channel_reserve;
	ipc_channel->free = ipc_channel_free;

	/* IPC Channel map irq handler */
	ipc_channel->base.handle_interrupt = ipc_channel_handle_interrupt;

	/* Init Mutex &  Semaphore for the IPC Channel */
	status = platform_mutex_init (&ipc_channel->state->lock);
	if (status != 0) {
		return status;
	}

	status = platform_semaphore_init (&ipc_channel->state->rx_wait);
	if (status != 0) {
		platform_mutex_free (&ipc_channel->state->lock);

		return status;
	}

	return 0;
}

/**
 * Release the IPC channel
 *
 * @param ipc_channel A pointer to ipc channel instance
 *
 */
void ipc_channel_release (const struct ipc_channel *ipc_channel)
{
	if ((ipc_channel == NULL) || (ipc_channel->state == NULL)) {
		return;
	}

	platform_mutex_free (&ipc_channel->state->lock);
	platform_semaphore_free (&ipc_channel->state->rx_wait);
}
