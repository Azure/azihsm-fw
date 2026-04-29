// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IPC_CHANNEL_H_
#define IPC_CHANNEL_H_

#include <stdint.h>
#include <string.h>
#include "ipc_message.h"
#include "platform_api.h"
#include "drivers/hsp_dmb.h"
#include "marvell/RegIntcIpc.h"
#include "status/manticore_module_id.h"
#include "trap/hsp_interrupt_handler.h"


/**
 * Increment IPC Queue (pi/ci) Index and rollover it to zero if index reaches to queue length
 */
#define IPC_QUEUE_INDEX_INC(index, length)		(((index) + 1) % length)

/**
 * Calculate the total message queue size.
 */
#define IPC_MESSAGE_QUEUE_SIZE(queue_length)	(sizeof (struct ipc_message_queue) + \
	(sizeof(struct ipc_message) * (queue_length)))

/**
 * Interrupt Block to be used on the give CPU core
 */
enum ipc_channel_interrupt_block {
	IPC_INT_BLOCK_0 = 0,	/**< Interrupt Block 0 */
	IPC_INT_BLOCK_1 = 1,	/**< Interrupt Block 1 */
	IPC_INT_BLOCK_2 = 2,	/**< Interrupt Block 2 */
	IPC_INT_BLOCK_3 = 3,	/**< Interrupt Block 3 */
	IPC_INT_BLOCK_4 = 4,	/**< Interrupt Block 4 */
	IPC_INT_BLOCK_5 = 5,	/**< Interrupt Block 5 */
};

/**
 * Enum for IPC Descriptor Value.
 */
enum ipc_channel_descriptor {
	IPC_DESCRIPTOR_0 = 0,	/**< IPC descriptor 0 */
	IPC_DESCRIPTOR_1 = 1,	/**< IPC descriptor 1 */
	IPC_DESCRIPTOR_2 = 2,	/**< IPC descriptor 2 */
	IPC_DESCRIPTOR_3 = 3,	/**< IPC descriptor 3 */
	IPC_DESCRIPTOR_4 = 4,	/**< IPC descriptor 4 */
	IPC_DESCRIPTOR_5 = 5,	/**< IPC descriptor 5 */
	IPC_DESCRIPTOR_6 = 6,	/**< IPC descriptor 6 */
	IPC_DESCRIPTOR_7 = 7,	/**< IPC descriptor 7 */
	IPC_DESCRIPTOR_8 = 8,	/**< IPC descriptor 8 */
	IPC_DESCRIPTOR_9 = 9,	/**< IPC descriptor 9 */
	IPC_DESCRIPTOR_10 = 10,	/**< IPC descriptor 10 */
	IPC_DESCRIPTOR_11 = 11,	/**< IPC descriptor 11 */
	IPC_DESCRIPTOR_12 = 12,	/**< IPC descriptor 12 */
	IPC_DESCRIPTOR_13 = 13,	/**< IPC descriptor 13 */
	IPC_DESCRIPTOR_14 = 14,	/**< IPC descriptor 14 */
	IPC_DESCRIPTOR_15 = 15,	/**< IPC descriptor 15 */
	IPC_DESCRIPTOR_16 = 16,	/**< IPC descriptor 16 */
	IPC_DESCRIPTOR_17 = 17,	/**< IPC descriptor 17 */
	IPC_DESCRIPTOR_18 = 18,	/**< IPC descriptor 18 */
	IPC_DESCRIPTOR_19 = 19,	/**< IPC descriptor 19 */
	IPC_DESCRIPTOR_20 = 20,	/**< IPC descriptor 20 */
	IPC_DESCRIPTOR_21 = 21,	/**< IPC descriptor 21 */
	IPC_DESCRIPTOR_22 = 22,	/**< IPC descriptor 22 */
	IPC_DESCRIPTOR_23 = 23,	/**< IPC descriptor 23 */
	IPC_DESCRIPTOR_24 = 24,	/**< IPC descriptor 24 */
	IPC_DESCRIPTOR_25 = 25,	/**< IPC descriptor 25 */
	IPC_DESCRIPTOR_26 = 26,	/**< IPC descriptor 26 */
	IPC_DESCRIPTOR_27 = 27,	/**< IPC descriptor 27 */
	IPC_DESCRIPTOR_28 = 28,	/**< IPC descriptor 28 */
	IPC_DESCRIPTOR_29 = 29,	/**< IPC descriptor 29 */
	IPC_DESCRIPTOR_30 = 30,	/**< IPC descriptor 30 */
	IPC_DESCRIPTOR_31 = 31,	/**< IPC descriptor 31 */
};


/**
 * Define IPC Message Queue configuration parameters
 */
struct __attribute__((aligned (0x4))) ipc_message_queue {
	uint32_t consumer_index;			/**< Queue consumer index */
	uint32_t producer_index;			/**< Queue Producer index */
	struct ipc_message msg_queue[0];	/**< Zero sized IPC Message Queue, accessible upto queue_length */
};

/**
 * Define IPC Message Channel state parameters
 */
struct ipc_channel_state {
	/**
	 * Any threads want to send message on the specific channel, First It will acquire
	 * Mutex and send message while other threads wait for it until it is released.
	 */
	platform_mutex lock;

	/**
	 * It will be used for signaling purpose. When any message received on the sepecific
	 * receive descriptor than it will signal that specific thread that message is received and
	 * further process can be perform.
	 */
	platform_semaphore rx_wait;

	uint16_t tag;	/**< IPC message tag identifier */
};


/**
 * IPC channel interface to send and receive message from one core to another core.
 * This interface is implemented for each IPC channel. A channel can be used to communicate
 * with a specific core.
 */
struct ipc_channel {
	/**
	 * Enable the IPC channel to start receiving messages
	 * Note that caller should reserve the IPC channel before calling this function.
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 *
	 *  @return 0 if completed successfully or an error code.
	 */
	int (*enable) (const struct ipc_channel *ipc_channel);

	/**
	 * Disable the IPC channel to stop receiving messages
	 * Note that caller should reserve the IPC channel before calling this function.
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 *
	 *  @return 0 if completed successfully or an error code.
	 */
	int (*disable) (const struct ipc_channel *ipc_channel);

	/**
	 * Sends an IPC message using transmit queue without waiting for a response
	 * Note that caller should reserve the IPC channel before calling this function.
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 * @param message The message data to be transmitted over the channel
	 *
	 * @return 0 if completed successfully or an error code.
	 */
	int (*send) (const struct ipc_channel *ipc_channel, struct ipc_message *message);

	/**
	 * Receives an IPC message using receive queue blocking until a message is available
	 * or timeout occurs
	 * Note that caller should reserve the IPC channel before calling this function.
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 * @param message A pointer to copy the received ipc_message
	 * @param timeout Timeout in milliseconds before receive returns. 0 - for infinite timeout
	 *
	 * @return 0 if completed successfully or an error code.
	 */
	int (*receive) (const struct ipc_channel *ipc_channel, struct ipc_message *message,
		uint32_t timeout_ms);

	/**
	 * Send an IPC message and wait to receive the corresponding response message.
	 * Before sending IPC message request it will validate that the IPC channel is enable or not.
	 * It will validate opcode and tag of the send and receive message are same. if it mis-matched
	 * than try to receive the valid message, until the response is available or a timeout occurs.
	 * The whole process is protected by an internal reserve/free mechanism
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 * @param message The message to send over the channel. Upon successful return,
	 * this will be updated to contain the received message
	 * @param timeout Timeout in milliseconds before receive returns. 0 - for infinite timeout
	 *
	 * @return 0 if completed successfully or an error code.
	 */
	int (*send_and_receive) (const struct ipc_channel *ipc_channel, struct ipc_message *message,
		uint32_t timeout_ms);

	/**
	 * Reserve this IPC channel before sending a new IPC message. Sleeps until the channel is
	 * reserved.
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 *
	 * @return 0 if completed successfully or an error code.
	 */
	int (*reserve) (const struct ipc_channel *ipc_channel);

	/**
	 * Free this IPC channel after sending a new IPC message to allow a waiting task to reserve
	 * this channel
	 *
	 * @param ipc_channel A pointer to this IPC channel object
	 *
	 * @return 0 if completed successfully or an error code.
	 */
	int (*free) (const struct ipc_channel *ipc_channel);

	struct hsp_interrupt_handler base;	/**< hsp_interrupt_handler object. */
	uint32_t tx_queue_base_addr;		/**< IPC message channel's transmit queue base address */
	size_t tx_queue_length;				/**< IPC message channel's transmit queue length */
	int tx_descriptor_id;				/**< Transmit Descriptor ID ranging from 0 to 31 */
	uint32_t rx_queue_base_addr;		/**< IPC message channel's receive queue base address */
	size_t rx_queue_length;				/**< IPC message channel's receive queue length */
	int rx_descriptor_id;				/**< Receive Descriptor ID ranging from 0 to 31 */
	const struct hsp_dmb *dmb;			/**< HSP DMB object used to map and unmap memory regions */
	uint32_t int_block_id;				/**< Access the interrupt control registers in IPC hardware register block */
	IntcIpc_t *register_block;			/**< IPC hardware register block to control the IPC hardware */
	struct ipc_channel_state *state;	/**< Maintain the IPC channel state */
};


int ipc_channel_init (struct ipc_channel *ipc_channel, struct ipc_channel_state *ipc_channel_state,
	uint32_t tx_queue_base_addr, uint32_t tx_queue_length, int tx_descriptor_id,
	uint32_t rx_queue_base_addr, uint32_t rx_queue_length, int rx_descriptor_id,
	const struct hsp_dmb *dmb, uint32_t int_block_id, IntcIpc_t *ipc_interrupt_registers);
void ipc_channel_release (const struct ipc_channel *ipc_channel);


/**
 * IPC Error code.
 */
#define	IPC_ERROR(code)			ROT_ERROR (MANTICORE_MODULE_IPC_CHANNEL, code)

/**
 * Error codes that can be generated by the IPC Module.
 */
enum {
	IPC_CHANNEL_INVALID_ARGUMENT = IPC_ERROR (0x00),			/**< Input parameter is null or not valid. */
	IPC_CHANNEL_NO_MEMORY = IPC_ERROR (0x01),					/**< Memory allocation failure. */
	IPC_CHANNEL_TX_QUEUE_FULL = IPC_ERROR (0x02),				/**< Queue is full once attempt to send a message. */
	IPC_CHANNEL_RX_QUEUE_EMPTY = IPC_ERROR (0x03),				/**< Queue is empty once attempt to receive a message. */
	IPC_CHANNEL_RESP_TIMEOUT = IPC_ERROR (0x04),				/**< Wait for response timeout. */
	IPC_CHANNEL_RX_INTERRUPT_DISABLED = IPC_ERROR (0x05),		/**< IPC RX interrupt is disable. */
	IPC_CHANNEL_SEND_FAILED = IPC_ERROR (0x06),					/**< Failed to send message on the ipc channel */
	IPC_CHANNEL_RECEIVED_FAILED = IPC_ERROR (0x07),				/**< Failed to receive message on the ipc channel */
	IPC_CHANNEL_SEND_AND_RECEIVED_FAILED = IPC_ERROR (0x08),	/**< Failed to send and receive message on ipc channel */
	IPC_CHANNEL_ENABLE_FAILED = IPC_ERROR (0x09),				/**< Failed to enable the ipc channel */
	IPC_CHANNEL_DISABLE_FAILED = IPC_ERROR (0x0A),				/**< Failed to disable the ipc channel*/
	IPC_CHANNEL_RESERVE_FAILED = IPC_ERROR (0x0B),				/**< Failed to reserve/take mutex for channel */
	IPC_CHANNEL_FREE_FAILED = IPC_ERROR (0x0C),					/**< Failed to free mutex for ipc channel */
	IPC_CHANNEL_RX_INVALID_RESP_BIT = IPC_ERROR (0x0D),			/**< IPC received message with invalid response bit */
	IPC_CHANNEL_RX_TAG_MISMATCHED = IPC_ERROR (0x0E),			/**< IPC send and received message tag mismatched */
	IPC_CHANNEL_RX_OPCODE_MISMATCHED = IPC_ERROR (0x0F),		/**< IPC send and received message opcode mismatched */
	IPC_CHANNEL_PLATFORM_TIME_ERROR = IPC_ERROR (0x10),			/**< Get time API failed */
};


#endif	/* IPC_CHANNEL_H_ */
