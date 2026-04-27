// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_LOGGING_H_
#define HSP_CRASHDUMP_LOGGING_H_

#include <stdbool.h>
#include <stdint.h>
#include "hsp_crashdump_handler.h"
#include "hsp_crashdump_packet.h"
#include "logging/debug_log.h"
#include "trap/hsp_trap_context.h"


void hsp_crashdump_logging_collect_crashdump (
	uint32_t fault_code, uint8_t crash_type, struct hsp_trap_context *ctx,
	const uint8_t *fw_version, size_t fw_version_length,
	struct hsp_crashdump_packet_hsp_production_packet *pkt);
void hsp_crashdump_logging_create_panic_context (int error_code,
	const struct debug_log_entry_info *error_log, struct hsp_trap_context *ctx);

void hsp_crashdump_logging_save_crashdump_header_to_debug_log (
	const struct crash_dump_packet_header *header, const uint32_t *fw_version);
int hsp_crashdump_logging_save_crashdump_to_debug_log (
	struct hsp_crashdump_packet_hsp_production_packet *pkt);
void hsp_crashdump_logging_save_crashdump_to_persistent_ram (
	struct hsp_crashdump_packet_hsp_production_packet *pkt, uint32_t *persistent_ram,
	size_t persistent_ram_size);

int hsp_crashdump_logging_save_crashdump_from_persistent_ram_to_debug_log (
	uint32_t *persistent_ram);
void hsp_crashdump_logging_save_stack_overflow_from_persistent_ram_to_debug_log (
	uint32_t *persistent_ram);
void hsp_crashdump_logging_save_panic_from_persistent_ram_to_debug_log (uint32_t *persistent_ram);


#endif	/* HSP_CRASHDUMP_LOGGING_H_ */
