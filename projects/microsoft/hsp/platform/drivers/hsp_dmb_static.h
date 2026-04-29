// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_DMB_STATIC_H_
#define HSP_DMB_STATIC_H_

#include "drivers/hsp_dmb.h"


/* Internal functions declared to allow for static initialization. */
int hsp_dmb_map_soc_address (const struct hsp_dmb *dmb, uint64_t soc_address, size_t length,
	uint8_t access_flags, void **hsp_address);
void hsp_dmb_unmap_soc_address (const struct hsp_dmb *dmb, void *hsp_address);


/**
 * Initialize a static DMB driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the DMB driver.
 * @param segments_ptr A list of DMB segment descriptors for the platform.  These do not need any
 * type of initialization.
 * @param total_segments The number of DMB segments supported by the platform.  This must match the
 * number of entries in the list of segment descriptors.  Segments must represent contiguous HSP
 * addresses.
 * @param base_map_addr The first HSP address that can be used by the DMB for mapping to external
 * addresses.
 * @param regs_ptr Register interface for the DMB hardware.
 */
#define	hsp_dmb_static_init(state_ptr, segments_ptr, total_segments, base_map_addr, regs_ptr)	{ \
		.map_soc_address = hsp_dmb_map_soc_address, \
		.unmap_soc_address = hsp_dmb_unmap_soc_address, \
		.state = state_ptr, \
		.segments = segments_ptr, \
		.segment_count = total_segments, \
		.hsp_dmb_base = base_map_addr, \
		.regs = regs_ptr \
	}


#endif	/* HSP_DMB_STATIC_H_ */
