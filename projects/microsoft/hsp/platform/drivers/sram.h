// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SRAM_H_
#define SRAM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common/sram_util.h"


bool sram_is_shared_address (const void *addr);
bool sram_is_buffer_in_shared_sram (const void *addr, size_t length);

bool sram_is_instruction_ram_address (const void *addr);
bool sram_is_buffer_in_instruction_ram (const void *addr, size_t length);

bool sram_is_data_ram_address (const void *addr);
bool sram_is_buffer_in_data_ram (const void *addr, size_t length);


#endif	/* SRAM_H_ */
