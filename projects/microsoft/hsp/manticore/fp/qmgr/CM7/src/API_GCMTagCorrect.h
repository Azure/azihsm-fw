// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

#ifndef GCM_TAG_CORRECT_H_
#define GCM_TAG_CORRECT_H_

#include <stdint.h>
#include <stdbool.h>
#include "GCM_Entry.h"
#include "Debug/logging/ring_buffer_state.h"

/**
 * Structure for the gcm tag ring buffer.
 */
struct queue_producer {
	void *mem_region_start;	/*< Start of the region to mapped */
	size_t mem_region_size;	/*< Size of the region in bytes */
};

/**
 * Initialize the queue producer.
 *
 * @param qprod The queue producer to initialize.
 * @param mem_region_start Start of the memory region to map.
 * @param mem_region_size Size of the memory region to map.
 * @param ceindex Cmd Entry index of gcm entries.
 *
 * @return 0 on success.
 */
int queue_init (struct queue_producer *qprod, void *mem_region_start,
	size_t mem_region_size);

/**
 * Sends a gcm entry to the ring buffer.
 *
 * @param qprod The buffer to send entries.
 * @param ceindex Cmd Entry index of gcm entries.
 * @param dflBuffPhysicalAddr Pointer address to GCM entry.
 *
 * @return 0 on success.
 */
int queue_producer_send (const struct queue_producer *qprod, uint16_t ceindex,bool tagInvalid, uint8_t pfn, uint32_t dflBuffPhysicalAddr);

/**
 * Fetch an entry from ring buffer.
 *
 * @param qprod The buffer to get entries from.
 *
 * @return ceindex on success.
 */
GcmResponseEntry_t queue_consumer_get (const struct queue_producer *qprod, uint32_t* pIsEmpty);

/**
 * Fetch an entry from IV queue ring buffer.
 *
 * @param qprod The buffer to get entries from.
 * @param pStatus Pointer to queue empty status variable.
 *
 * @return IV on success.
 */
IVEntry_t queue_consumer_get_iv (const struct queue_producer *qprod, uint32_t* pStatus);

/**
 * API for IV queue Initialization
 */
void API_GcmIvQueueOneTimeInitByFp1();

/**
 * API to get IV queue entries
 * 
 * @param pStatus To check if queue is empty. Set to RING_BUFFER_EMPTY if queue is empty.
 */
IVEntry_t API_GetIVEntry(uint32_t* pStatus);

/**
 * API for GCM Tag correction request and response queues initialization
 */
void API_GcmReqResQueuesOneTimeInitByFp2();

/**
 * API to send a gcm entry to the ring buffer via the queue_producer module
 *
 * @param ceindex the SQE index used to get cmd for tag correction
 * @param dflBuffPhysicalAddr SQE address
 *
 */
void API_AddGCMTagCorrect(uint16_t ceindex, bool tagInvalid, uint8_t pfn, uint32_t dflBuffPhysicalAddr);

/**
 * API to get a gcm entry from the ring buffer
 *
 * @param none 
 *
 */
GcmResponseEntry_t API_GetGCMTagCorrect(uint32_t* pIsEmpty);

uint8_t check_if_queue_empty(const struct queue_producer *qcons);
uint8_t API_GcmReqQueueFull();

#endif	/* GCM_TAG_CORRECT_H_ */