// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

#include "API_GCMTagCorrect.h"
#include "GCM_Entry.h"
#include "M7MemMap.h"

int queue_init (struct queue_producer *queue, void *mem_region_start,
	size_t mem_region_size)
{
	if ((queue == NULL) || (mem_region_start == NULL) || (mem_region_size == 0)) {
		return -1;
	}

	queue->mem_region_start = mem_region_start;
	queue->mem_region_size = mem_region_size;

	struct ring_buffer_state *rb_gcm =
		(struct ring_buffer_state*) (queue->mem_region_start);

	// (Re)initialize the ring buffer state on PoR or recover it if corrupted after reset.
	if (!ring_buffer_is_valid (rb_gcm) || (rb_gcm->buffer_size != queue->mem_region_size)) {
		return ring_buffer_init (rb_gcm, queue->mem_region_size);
	}

	return 0;
}

int queue_producer_send(const struct queue_producer *qprod, uint16_t ceindex, bool tagInvalid, uint8_t pfn, uint32_t dflBuffPhysicalAddr)
{
	struct ring_buffer_state *rb_req =
		(struct ring_buffer_state*) (qprod->mem_region_start);
	struct GcmRequestEntry_t *array1 = (struct GcmRequestEntry_t*) (rb_req + 1);

	struct GcmRequestEntry_t params = {
		.ceIndex = ceindex,
		.tagInvalid = tagInvalid,
		.pfn = pfn,
		.sqeAddr = dflBuffPhysicalAddr,
	};

	if (ring_buffer_get_overflows(rb_req) != 0){
		ring_buffer_reset_overflows(rb_req);
	}

	return ring_buffer_push_tail (rb_req, array1, params);
}

GcmResponseEntry_t queue_consumer_get(const struct queue_producer *qcons, uint32_t* pIsEmpty)
{
	struct ring_buffer_state *rb_res =
		(struct ring_buffer_state*) (qcons->mem_region_start);
	struct GcmResponseEntry_t *array2 = (struct GcmResponseEntry_t*) (rb_res + 1); 
	struct GcmResponseEntry_t value;
	struct GcmResponseEntry_t resp;
	

	*pIsEmpty = ring_buffer_peek_head (rb_res, array2, resp);
	if (*pIsEmpty == 0){
		resp = array2[(rb_res)->head];
		ring_buffer_pop_head(rb_res, array2, array2[(rb_res)->head]);
	}
	else{
		resp = (struct GcmResponseEntry_t){0};
	}

	return resp;
}

IVEntry_t queue_consumer_get_iv(const struct queue_producer *qcons, uint32_t* pStatus)
{
	struct ring_buffer_state *rb_iv =
		(struct ring_buffer_state*) (qcons->mem_region_start);
	struct IVEntry_t *array3 = (struct IVEntry_t*) (rb_iv + 1); 
	struct IVEntry_t resp;
	
    *pStatus = ring_buffer_peek_head (rb_iv, array3, resp);
	if (*pStatus == 0){
		resp = array3[(rb_iv)->head];
		array3[(rb_iv)->head] = (struct IVEntry_t){0}; //zeroize IV once consumed
		ring_buffer_pop_head(rb_iv, array3, array3[(rb_iv)->head]);
	}
	else{
		resp = (struct IVEntry_t){0};
	}
	
	return resp;
}

uint8_t check_if_queue_empty(const struct queue_producer *qcons)
{
	struct ring_buffer_state *rb_req =
		(struct ring_buffer_state*) (qcons->mem_region_start);

	return ring_buffer_is_full(rb_req);
}

struct queue_producer queue_handle_req, queue_handle_res, queue_handle_iv;

void API_GcmIvQueueOneTimeInitByFp1()
{
	queue_init(&queue_handle_iv, (void *)M7_FPS_CPU12_GCM_IV_QUEUE_ADDR, GCM_IV_QUEUE_ENTRIES);
}

IVEntry_t API_GetIVEntry(uint32_t* pStatus)
{
	IVEntry_t iv_entry = queue_consumer_get_iv(&queue_handle_iv, pStatus);
	return iv_entry;
}

void API_GcmReqResQueuesOneTimeInitByFp2()
{
    queue_init(&queue_handle_req, (void *)M7_FPS_CPU20_GCM_REQUEST_QUEUE, GCM_REQ_RESP_QUEUE_ENTRIES);
	queue_init(&queue_handle_res, (void *)M7_FPS_CPU20_GCM_RESPONSE_QUEUE, GCM_REQ_RESP_QUEUE_ENTRIES);
}

void API_AddGCMTagCorrect(uint16_t ceindex, bool tagInvalid, uint8_t pfn, uint32_t dflBuffPhysicalAddr)
{
	queue_producer_send(&queue_handle_req,ceindex,tagInvalid,pfn, dflBuffPhysicalAddr);
}

GcmResponseEntry_t API_GetGCMTagCorrect(uint32_t* pIsEmpty)
{
	GcmResponseEntry_t entry_res = queue_consumer_get(&queue_handle_res, pIsEmpty);
	return entry_res;
}

uint8_t API_GcmReqQueueFull()
{
	return check_if_queue_empty(&queue_handle_req);

}