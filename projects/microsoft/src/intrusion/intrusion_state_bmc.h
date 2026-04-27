// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INTRUSION_STATE_BMC_H_
#define INTRUSION_STATE_BMC_H_

#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "cmd_interface/cmd_channel.h"
#include "cmd_interface/device_manager.h"
#include "common/observable.h"
#include "crypto/hash.h"
#include "crypto/rng.h"
#include "flash/flash_store.h"
#include "intrusion/intrusion_state.h"
#include "intrusion/intrusion_state_observer.h"
#include "mctp/mctp_base_protocol.h"
#include "mctp/mctp_interface.h"
#include "msft_protocol/bmc_commands.h"
#include "msft_protocol/msg_transport_msft.h"

/**
 * The minimum RNG data length for intrusion state.
 */
#define CERBERUS_INTRUSION_STATE_BMC_MIN_LENGTH_BYTES		(32u)

/**
 * The maximum RNG data length for intrusion state.
 */
#define CERBERUS_INTRUSION_STATE_BMC_MAX_LENGTH_BYTES		(128u)

/**
 * The period to wait before sending intrusion data challenge request.
 */
#define CHASSIS_INTRUSION_CHALLENGE_DELAY_MS				(60 * 1000)

/**
 * The period to wait before sending intrusion data challenge request retries.
 */
#define CHASSIS_INTRUSION_CHALLENGE_RETRY_DELAY_MS			(20 * 1000)

/**
 * Maximum payload size for an intrusion message.
 */
#define	INTRUSION_STATE_BMC_MAX_MESSAGE_LENGTH              \
	bmc_chassis_intrusion_store_data_length (CERBERUS_INTRUSION_STATE_BMC_MAX_LENGTH_BYTES)

/**
 * The maximum sized MCTP packet during an intrusion state transaction.
 */
#define CHASSIS_INTRUSION_STATE_MAX_PACKET_LENGTH 			(MCTP_BASE_PROTOCOL_PACKET_OVERHEAD + \
	CERBERUS_PROTOCOL_MIN_MSG_LEN + INTRUSION_STATE_BMC_MAX_MESSAGE_LENGTH)


/**
 * Chassis intrusion request transaction state
 */
enum intrusion_state_bmc_request_state {
	INTRUSION_STATE_BMC_REQUEST_WAITING = 0,	/**< Start of a transaction or no response received yet */
	INTRUSION_STATE_BMC_REQUEST_SUCCESSFULL,	/**< Successful response received */
	INTRUSION_STATE_BMC_REQUEST_RSP_FAIL,		/**< Failed response received */
};

/**
 * Platform-specific interface for intrusion state detection. For BMCs, this feature stores
 * random data into flash and then sends the data to the BMC. When intrusion state is checked,
 * the BMC sends a hash of the stored data to the device, which then compares the hash to the
 * hash of the data it stored in its own flash.
 */
struct intrusion_state_bmc {
	struct intrusion_state base;									/**< Implementation of core intrusion state API. */
	platform_semaphore run_check;									/**< The semaphore for asynchronous intrusion state checking. */
	uint8_t msg_buffer[CHASSIS_INTRUSION_STATE_MAX_PACKET_LENGTH];	/**< Buffer to be used for request generation and response processing. */
	struct observable observable;									/**< Observer manager for the intrusion state. */
	const struct flash_store *store;								/**< Flash store where data is saved. */
	const struct rng_engine *rng;									/**< RNG engine used to create data. */
	const struct hash_engine *hash;									/**< Hash engine used for comparison with BMC data. */
	const struct cmd_channel *channel;								/**< Channel for communicating with BMC. */
	struct device_manager *device_mgr;								/**< Device manager for getting response timeout. */
	const struct msg_transport *msft_transport;						/**< MSFT message transport for sending requests. */
	enum intrusion_state_bmc_request_state request_status;			/**< Response processing status. */
	enum hash_type hash_type_bmc;									/**< Hash type used by the BMC when responding to queries. */
	size_t rng_len;													/**< Length of the RNG number that will be stored. */
	uint8_t source_eid;												/**< EID of the source of the packet. */
	uint8_t dest_eid;												/**< EID of the packet destination. */
	uint8_t source_addr;											/**< I2C address of the source. */
	uint8_t dest_addr;												/**< I2C address of the destination. */
	int id;															/**< Block ID of the stored data in memory. */
	uint32_t challenge_retry_delay;									/**< Delay for challenge request retries, in ms. */
};


int intrusion_state_bmc_init (struct intrusion_state_bmc *intrusion, int id, size_t rng_length,
	const struct flash_store *store, const struct rng_engine *rng, const struct hash_engine *hash,
	enum hash_type hash_type_bmc, const struct cmd_channel *channel, uint8_t source_eid,
	uint8_t dest_eid, uint8_t source_addr, uint8_t dest_addr, uint32_t challenge_retry_delay,
	const struct msg_transport *msft_transport, struct device_manager *device_mgr);
void intrusion_state_bmc_release (struct intrusion_state_bmc *intrusion);

int intrusion_state_bmc_add_observer (struct intrusion_state_bmc *intrusion,
	struct intrusion_state_observer *observer);
int intrusion_state_bmc_remove_observer (struct intrusion_state_bmc *intrusion,
	struct intrusion_state_observer *observer);

int intrusion_state_bmc_background_check (struct intrusion_state_bmc *intrusion);


#endif	/* INTRUSION_STATE_BMC_H_ */
