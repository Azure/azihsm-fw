// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "hsp_top.h"
#include "common/unused.h"
#include "drivers/hsp_dmb.h"


/**
 * The number of address bits supported per HSP address segment.
 */
#define	HSP_DMB_HSP_ADDR_BITS				27

/**
 *  The number of SoC address bits stored in the lower segment register.
 */
#define	HSP_DMB_HSP_LOWER_MAP_BITS			13

/**
 * The number of SoC address bits stored in the upper segment register.
 */
#define	HSP_DMB_HSP_UPPER_MAP_BITS			24

/**
 * The minimum number of bits for a segment window.
 */
#define	HSP_DMB_WINDOW_MIN_BITS				17

/**
 * Minimum size of any mapped segment.
 */
#define	HSP_DMB_MIN_SEGMENT_SIZE			(1U << HSP_DMB_WINDOW_MIN_BITS)

/**
 * Maximum size of any mapped segment.
 */
#define	HSP_DMB_MAX_SEGMENT_SIZE			(1U << HSP_DMB_HSP_ADDR_BITS)

/**
 * The minimum HSP address that can be mapped.
 */
#define	HSP_DMB_MIN_MAPPED_ADDR				(HSP_DMB_MAX_SEGMENT_SIZE * 18)

/**
 * Mask to get the address offset within the segment.
 */
#define	HSP_DMB_SEGMENT_OFFSET_MASK			(HSP_DMB_MAX_SEGMENT_SIZE - 1)

/**
 * Get the segment index based on the HSP address.
 */
#define	HSP_DMB_SEGMENT_INDEX(x) \
	(((((uintptr_t) x) & ~HSP_DMB_SEGMENT_OFFSET_MASK) >> HSP_DMB_HSP_ADDR_BITS) - 18)

/**
 * Mask for the lower map bits.
 */
#define	HSP_DMB_HSP_LOWER_MAP_MASK			((1U << HSP_DMB_HSP_LOWER_MAP_BITS) - 1)

/**
 * Mask for the upper map bits.
 */
#define	HSP_DMB_HSP_UPPER_MAP_MASK			((1U << HSP_DMB_HSP_UPPER_MAP_BITS) - 1)

/**
 * Read access for the segment.
 */
#define	HSP_DMB_ATTRIBUTE_READ				0xaaaaaaaa

/**
 * Write access for the segment.
 */
#define	HSP_DMB_ATTRIBUTE_WRITE				0x55555555

/**
 * Read write access for the segment.
 */
#define	HSP_DMB_ATTRIBUTE_READ_WRITE		(HSP_DMB_ATTRIBUTE_READ | HSP_DMB_ATTRIBUTE_WRITE)

/**
 * AxUser bit 0. Platform specific functionality.
 */
#define HSP_DMB_ATTRIBUTE_AXUSER0_BIT_MASK	(1U << 24);

/**
 * Secure Bit. Platform specific functionality.
 */
#define HSP_DMB_ATTRIBUTE_SECURE_BIT_MASK	(1U << 28);

/**
 * Configuration value for different sized windows.
 */
enum {
	HSP_DMB_WINDOW_SIZE_128K = 0,	/**< A 128kB window, which is the minimum. */
	HSP_DMB_WINDOW_SIZE_256K = 1,	/**< A 256kB window. */
	HSP_DMB_WINDOW_SIZE_512K = 2,	/**< A 512kB window. */
	HSP_DMB_WINDOW_SIZE_1M = 3,		/**< A 1MB window. */
	HSP_DMB_WINDOW_SIZE_2M = 4,		/**< A 2MB window. */
	HSP_DMB_WINDOW_SIZE_4M = 5,		/**< A 4MB window. */
	HSP_DMB_WINDOW_SIZE_8M = 6,		/**< An 8MB window. */
	HSP_DMB_WINDOW_SIZE_16M = 7,	/**< A 16MB window. */
	HSP_DMB_WINDOW_SIZE_32M = 8,	/**< A 32MB window. */
	HSP_DMB_WINDOW_SIZE_64M = 9,	/**< A 64MB window. */
	HSP_DMB_WINDOW_SIZE_128M = 10,	/**< A 128MB window, which is the maximum. */
};


/**
 * Find the next DMB segment that is available for use.
 *
 * @param dmb The DMB driver to query.
 * @param segment_index Output for the index in the segment list of the free segment descriptor.
 *
 * @return The next free segment descriptor or null if there are no available segments.
 */
struct hsp_dmb_segment* hsp_dmb_find_next_free_segment (const struct hsp_dmb *dmb,
	uint8_t *segment_index)
{
	size_t i;

	for (i = 0; i < dmb->segment_count; i++) {
		if (!dmb->segments[i].in_use) {
			break;
		}
	}

	if (i == dmb->segment_count) {
		return NULL;
	}

	*segment_index = i;

	return &dmb->segments[i];
}

/**
 * Determine the necessary window size value for a specified length.
 *
 * @param length The length to check.
 *
 * @return The window size setting.
 */
uint8_t hsp_dmb_determine_window_size (size_t length)
{
	if (length <= (128 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_128K;
	}
	else if (length <= (256 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_256K;
	}
	else if (length <= (512 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_512K;
	}
	else if (length <= (1 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_1M;
	}
	else if (length <= (2 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_2M;
	}
	else if (length <= (4 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_4M;
	}
	else if (length <= (8 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_8M;
	}
	else if (length <= (16 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_16M;
	}
	else if (length <= (32 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_32M;
	}
	else if (length <= (64 * 1024 * 1024)) {
		return HSP_DMB_WINDOW_SIZE_64M;
	}
	else {
		return HSP_DMB_WINDOW_SIZE_128M;
	}
}

int hsp_dmb_map_soc_address (const struct hsp_dmb *dmb, uint64_t soc_address, size_t length,
	uint8_t access_flags, void **hsp_address)
{
	struct hsp_dmb_segment *segment;
	uint8_t segment_index;
	uint32_t segment_mask;
	uint32_t segment_address;
	uint32_t lower_soc_address;
	uint32_t upper_soc_address;
	uint32_t access;
	uint8_t window;
	uint32_t offset;

	if ((dmb == NULL) || (length == 0) || (hsp_address == NULL)) {
		return HSP_DMB_INVALID_ARGUMENT;
	}

	if ((access_flags & HSP_DMB_ACCESS_NO_PRIVILEGED) &&
		!(access_flags & (HSP_DMB_ACCESS_USER | HSP_DMB_ACCESS_CRYPTO))) {
		return HSP_DMB_INVALID_PERMISSIONS;
	}

	if (length > HSP_DMB_MAX_SEGMENT_SIZE) {
		return HSP_DMB_MAP_TOO_LARGE;
	}

	if (length < HSP_DMB_MIN_SEGMENT_SIZE) {
		/* This needs to be updated to support overflow checks. */
		length = HSP_DMB_MIN_SEGMENT_SIZE;
	}

	/* TODO:  Just increase the window size to account for any offset within the segment.  DMB does
	 * provide a 10-bit segment offset setting that could potentially be used to leave the window
	 * smaller while still providing the necessary access.  This should be explored further.
	 *
	 * Even when utilizing the segment offset, segment overflow still needs to be checked. */
	offset = soc_address & HSP_DMB_SEGMENT_OFFSET_MASK;
	length += offset;
	if (length > HSP_DMB_MAX_SEGMENT_SIZE) {
		/* TODO:  Rather than failing due to segment overflow, another approach would be to allocate
		 * two segments to this mapping.  They would be continuous addresses from the HSP point of
		 * view, it would just require some additional tracking here in the driver. */
		return HSP_DMB_SEGMENT_OVERFLOW;
	}

	platform_mutex_lock (&dmb->state->lock);

	/* TODO:  DMB usage could be optimized by reusing overlapping mappings with the same permissions
	 * while keeping track of references to the mapping.  If DMB gets heavily used by multiple
	 * different components and/or tasks, this kind of optimization could be necessary.  For current
	 * usages, just using a free segment each time is probably sufficient. */
	segment = hsp_dmb_find_next_free_segment (dmb, &segment_index);
	if (segment == NULL) {
		platform_mutex_unlock (&dmb->state->lock);

		return HSP_DMB_NOT_AVAILABLE;
	}

	/* Determine the DMB configuration to apply. */
	segment_address = dmb->hsp_dmb_base + (HSP_DMB_MAX_SEGMENT_SIZE * segment_index);
	segment_index += (dmb->hsp_dmb_base - HSP_DMB_MIN_MAPPED_ADDR) / HSP_DMB_MAX_SEGMENT_SIZE;
	segment_mask = (1U << (18 + segment_index));

	lower_soc_address = (soc_address >> HSP_DMB_HSP_ADDR_BITS) & HSP_DMB_HSP_LOWER_MAP_MASK;
	upper_soc_address = (soc_address >> (HSP_DMB_HSP_ADDR_BITS + HSP_DMB_HSP_LOWER_MAP_BITS)) &
		HSP_DMB_HSP_UPPER_MAP_MASK;

	/* Set the AxUser 0 bit if requested. */
	if (access_flags & HSP_DMB_ACCESS_AXUSER0) {
		upper_soc_address |= HSP_DMB_ATTRIBUTE_AXUSER0_BIT_MASK;
	}

	/* Set the secure bit if requested. */
	if (access_flags & HSP_DMB_ACCESS_SECURE) {
		lower_soc_address |= HSP_DMB_ATTRIBUTE_SECURE_BIT_MASK;
	}

	window = hsp_dmb_determine_window_size (length);

	access = (access_flags & HSP_DMB_ACCESS_WRITE) ?
			HSP_DMB_ATTRIBUTE_READ_WRITE : HSP_DMB_ATTRIBUTE_READ;

	/* Write the configuration to the DMB segment registers. */
	(&dmb->regs->seg_reg18)[segment_index] = lower_soc_address |
		DMB_REG_SEG_REG18_WINDOW_N_SET (window);
	(&dmb->regs->seg_upper_addr_reg18)[segment_index] = upper_soc_address;
	(&dmb->regs->seg18_attr_high)[segment_index * 2] = access;
	(&dmb->regs->seg18_attr_low)[segment_index * 2] = access;

	if (!(access_flags & HSP_DMB_ACCESS_NO_PRIVILEGED)) {
		dmb->regs->privilege_permission |= segment_mask;
	}
	else {
		dmb->regs->privilege_permission &= ~segment_mask;
	}

	if (access_flags & HSP_DMB_ACCESS_USER) {
		dmb->regs->user_permission |= segment_mask;
	}
	else {
		dmb->regs->user_permission &= ~segment_mask;
	}

	if (access_flags & HSP_DMB_ACCESS_CRYPTO) {
		dmb->regs->crypto_permission |= segment_mask;
	}
	else {
		dmb->regs->crypto_permission &= ~segment_mask;
	}

	/* If the destination address is offset from the segment start, the mapped address will be, too.
	 *
	 * TODO:  It is unclear if this will get affected by the segment offset setting, but this may
	 * need to get adjusted along with leveraging that configuration. */
	*hsp_address = (void*) (segment_address + offset);
	segment->in_use = true;

	platform_mutex_unlock (&dmb->state->lock);

	return 0;
}

void hsp_dmb_unmap_soc_address (const struct hsp_dmb *dmb, void *hsp_address)
{
	uint8_t segment_index;
	uint8_t list_index;
	uint32_t segment_mask;

	if ((dmb == NULL) || (hsp_address == NULL)) {
		return;
	}

	segment_index = HSP_DMB_SEGMENT_INDEX (hsp_address);
	segment_mask = ~(1U << (18 + segment_index));
	list_index =
		segment_index - ((dmb->hsp_dmb_base - HSP_DMB_MIN_MAPPED_ADDR) / HSP_DMB_MAX_SEGMENT_SIZE);

	/* If the address is not in our valid range, just exit. */
	if (((uintptr_t) hsp_address < dmb->hsp_dmb_base) || (list_index >= dmb->segment_count)) {
		return;
	}

	platform_mutex_lock (&dmb->state->lock);

	/* Clear the DMB register settings for the segment. */
	(&dmb->regs->seg_reg18)[segment_index] = 0;
	(&dmb->regs->seg_upper_addr_reg18)[segment_index] = 0;
	(&dmb->regs->seg18_attr_high)[segment_index * 2] = 0;
	(&dmb->regs->seg18_attr_low)[segment_index * 2] = 0;
	dmb->regs->privilege_permission &= segment_mask;
	dmb->regs->user_permission &= segment_mask;
	dmb->regs->crypto_permission &= segment_mask;

	/* Mark the segment as free for the driver. */
	dmb->segments[list_index].in_use = false;

	platform_mutex_unlock (&dmb->state->lock);
}

/**
 * Initialize a driver for the HSP DMB to support access to external addresses.
 *
 * @param dmb The DMB driver to initialize.
 * @param state Variable context for the DMB driver.  This must be uninitialized.
 * @param segments A list of DMB segment descriptors for the platform.  These do not need any type
 * of initialization.
 * @param total_segments The number of DMB segments supported by the platform.  This must match the
 * number of entries in the list of segment descriptors.  Segments must represent contiguous HSP
 * addresses.
 * @param base_map_addr The first HSP address that can be used by the DMB for mapping to external
 * addresses.
 * @param regs Register interface for the DMB hardware.
 *
 * @return 0 if the DMB driver was successfully initialized or an error code.
 */
int hsp_dmb_init (struct hsp_dmb *dmb, struct hsp_dmb_state *state,
	struct hsp_dmb_segment *segments, size_t total_segments, uint32_t base_map_addr,
	struct Dmb_reg *regs)
{
	if ((dmb == NULL) || (state == NULL) || (segments == NULL) || (total_segments == 0) ||
		(regs == NULL)) {
		return HSP_DMB_INVALID_ARGUMENT;
	}

	memset (dmb, 0, sizeof (struct hsp_dmb));

	dmb->map_soc_address = hsp_dmb_map_soc_address;
	dmb->unmap_soc_address = hsp_dmb_unmap_soc_address;

	dmb->state = state;
	dmb->segments = segments;
	dmb->segment_count = total_segments;
	dmb->hsp_dmb_base = base_map_addr;
	dmb->regs = regs;

	return hsp_dmb_init_state (dmb);
}

/**
 * Initialize only the variable state for a DMB driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param dmb The DMB driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_dmb_init_state (const struct hsp_dmb *dmb)
{
	if ((dmb == NULL) || (dmb->state == NULL) || (dmb->segments == NULL) ||
		(dmb->segment_count == 0) || (dmb->regs == NULL)) {
		return HSP_DMB_INVALID_ARGUMENT;
	}

	if (dmb->hsp_dmb_base < HSP_DMB_MIN_MAPPED_ADDR) {
		return HSP_DMB_BAD_BASE_ADDR;
	}

	/* The base address needs to be aligned to the beginning of a segment. */
	if ((dmb->hsp_dmb_base & HSP_DMB_SEGMENT_OFFSET_MASK) != 0) {
		return HSP_DMB_BAD_BASE_ADDR;
	}

	memset (dmb->state, 0, sizeof (struct hsp_dmb_state));
	memset (dmb->segments, 0, sizeof (struct hsp_dmb_segment) * dmb->segment_count);

	return platform_mutex_init (&dmb->state->lock);
}

/**
 * Release the resources used by a DMB driver.
 *
 * @param dmb The DMB driver to release.
 */
void hsp_dmb_release (const struct hsp_dmb *dmb)
{
	if (dmb) {
		platform_mutex_free (&dmb->state->lock);
	}
}
