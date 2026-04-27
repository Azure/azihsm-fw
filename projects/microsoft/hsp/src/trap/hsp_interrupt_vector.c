// Copyright (c) Microsoft Corporation. All rights reserved.

#include "trap/hsp_interrupt_vector.h"
#include "trap/irq_error.h"


/**
 * Gets the registered handler and calls it if valid
 *
 * @param entry The handler vector entry
 * @param param The parameter passed by the trap handler
 *
 * @return true if interrupt handled, else false
 */
bool hsp_interrupt_vector_call_entry (const struct hsp_interrupt_handler *const *entry,
	uintptr_t param)
{
	const struct hsp_interrupt_handler *handler;

	handler = *entry;
	if (handler == NULL) {
		return false;
	}

	return handler->handle_interrupt (handler, param);
}

/**
 * Registers an interrupt handler in a handler vector
 *
 * @param vector Array of handlers
 * @param count Count of the handler array
 * @param index The index of the array to register a handler for
 * @param handler A handler for the interrupt
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_vector_register (const struct hsp_interrupt_handler **vector, size_t count,
	size_t index, const struct hsp_interrupt_handler *handler)
{
	if ((handler == NULL) || (index >= count)) {
		return IRQ_INVALID_ARGUMENT;
	}

	vector += index;
	if ((*vector) != NULL) {
		return IRQ_ALREADY_REGISTERED;
	}

	*vector = handler;

	return 0;
}

/**
 * Unregisters an interrupt handler in a handler vector
 *
 * @param vector Array of handlers
 * @param count Count of the handler array
 * @param index The array index to unregister the handler
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_vector_unregister (const struct hsp_interrupt_handler **vector, size_t count,
	size_t index)
{
	if (index >= count) {
		return IRQ_INVALID_ARGUMENT;
	}

	vector += index;
	if (*vector == NULL) {
		return IRQ_NOT_REGISTERED;
	}

	*vector = NULL;

	return 0;
}
