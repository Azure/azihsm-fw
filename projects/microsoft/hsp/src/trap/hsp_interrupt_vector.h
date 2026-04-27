// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_INTERRUPT_VECTOR_H_
#define HSP_INTERRUPT_VECTOR_H_

#include <stddef.h>
#include "trap/hsp_interrupt_handler.h"


// TODO: Unit tests...

bool hsp_interrupt_vector_call_entry (const struct hsp_interrupt_handler *const *entry,
	uintptr_t param);

int hsp_interrupt_vector_register (const struct hsp_interrupt_handler **vector, size_t count,
	size_t index, const struct hsp_interrupt_handler *handler);

int hsp_interrupt_vector_unregister (const struct hsp_interrupt_handler **vector, size_t count,
	size_t index);


#endif	/* HSP_INTERRUPT_VECTOR_H_ */
