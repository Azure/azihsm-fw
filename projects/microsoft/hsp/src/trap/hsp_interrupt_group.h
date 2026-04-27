// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTERRUPT_GROUP_H_
#define HSP_INTERRUPT_GROUP_H_

#include <stddef.h>
#include "trap/hsp_interrupt_handler.h"


// TODO: Unit tests...

/**
 * A group of interrupt handlers to process an interrupt signal.
 */
struct hsp_interrupt_group {
	struct hsp_interrupt_handler base;					/**< Base handler instance */
	size_t count;										/**< Size of the vector */
	const struct hsp_interrupt_handler *const *vector;	/**< An array of handlers */
	bool first_handled;									/**< Flag to break on first handled or to notify all */
};


int hsp_interrupt_group_init (struct hsp_interrupt_group *group,
	const struct hsp_interrupt_handler *const *vector, size_t count, bool first_handled);
void hsp_interrupt_group_release (struct hsp_interrupt_group *group);

/* TODO:  These functions are not compatible with const list of handlers.  There should generally
 * not be a need for them, so remove support until a scheme can be devised to support both const and
 * mutable groups. */
#if 0
int hsp_interrupt_group_register (const struct hsp_interrupt_group *group, size_t index,
	const struct hsp_interrupt_handler *handler);

int hsp_interrupt_group_unregister (const struct hsp_interrupt_group *group, size_t index);
#endif


#endif	/* HSP_INTERRUPT_GROUP_H_ */
