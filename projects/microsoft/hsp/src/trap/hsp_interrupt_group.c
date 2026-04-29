// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/unused.h"
#include "trap/hsp_interrupt_group_static.h"
#include "trap/hsp_interrupt_vector.h"
#include "trap/irq_error.h"


bool hsp_interrupt_group_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	const struct hsp_interrupt_group *group = (const struct hsp_interrupt_group*) handler;
	size_t count = group->count;
	const struct hsp_interrupt_handler *const *entry = group->vector;
	bool handled = false;

	while (count > 0) {
		handled |= hsp_interrupt_vector_call_entry (entry, param);
		if (handled && group->first_handled) {
			break;
		}

		--count;
		++entry;
	}

	return handled;
}

/**
 * Initializes an interrupt handler group
 *
 * @param group The handler group instance
 * @param vector The handler vector array
 * @param count The size of the vector
 * @param first_handled Flag to indicate if handler should break on first handled in the set, or to
 * notify all.
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_group_init (struct hsp_interrupt_group *group,
	const struct hsp_interrupt_handler *const *vector, size_t count, bool first_handled)
{
	if (group == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	if ((count > 0) && (vector == NULL)) {
		return IRQ_INVALID_ARGUMENT;
	}

	group->count = count;
	group->vector = vector;
	group->first_handled = first_handled;

	return 0;
}

/**
 * Releases any resources held by an interrupt group instance.
 *
 * @param group The handler group instance
 */
void hsp_interrupt_group_release (struct hsp_interrupt_group *group)
{
	UNUSED (group);
}

/* TODO:  See details in the header file. */
#if 0
/**
 * Registers an interrupt handler in a handler group
 *
 * @param group The handler group instance
 * @param index The vector index to register the handler
 * @param handler The interrupt handler instance
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_group_register (const struct hsp_interrupt_group *group, size_t index,
	const struct hsp_interrupt_handler *handler)
{
	if (group == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	return hsp_interrupt_vector_register (group->vector, group->count, index, handler);
}

/**
 * Unregisters an interrupt handler in a handler group
 *
 * @param group The handler group instance
 * @param index The vector index to unregister the handler
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_group_unregister (const struct hsp_interrupt_group *group, size_t index)
{
	if (group == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	return hsp_interrupt_vector_unregister (group->vector, group->count, index);
}
#endif
