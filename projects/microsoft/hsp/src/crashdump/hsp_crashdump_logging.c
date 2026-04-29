// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_crashdump_logging.h"
#include "crashdump/hsp_crashdump_hw_err_handler_access_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_bus_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_chk_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_dmb_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_mem_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_mpu_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_rng_err.h"
#include "crashdump/hsp_crashdump_hw_err_handler_watchdog_timeout.h"
#include "crashdump/hsp_crashdump_hw_err_handler_wdt_err.h"
#include "logging/crash_dump_logging.h"
#include "logging/debug_log.h"
#include "logging/hsp_logging.h"
#include "system/system_logging.h"

/**
 * Take the trap context, format/collect it to HSP crashdump packet.
 *
 * @param[in] fault_code The fault code.
 * @param[in] crash_type The crash type could be either crash or hanging.
 * @param[in] hsp_trap_context The HSP trap context.
 * @param[in] fw_version The version of FW that crashed.
 * @param[in] fw_version_length The length of build version.
 * @param[out] pkt The crashdump packet.
 */
void hsp_crashdump_logging_collect_crashdump (
	uint32_t fault_code, uint8_t crash_type, struct hsp_trap_context *ctx,
	const uint8_t *fw_version, size_t fw_version_length,
	struct hsp_crashdump_packet_hsp_production_packet *pkt)
{
	/* TODO: we should generate some default crash dump information in this case. */
	if ((ctx == NULL) || (fw_version == NULL) || (pkt == NULL)) {
		return;
	}

	memset (pkt, 0, sizeof (struct hsp_crashdump_packet_hsp_production_packet));

	pkt->header.crashdump_version = HSP_CRASHDUMP_VERSION;
	pkt->header.core_type = HSP_CRASHDUMP_PACKET_CORE_TYPE_HSP_TYPE;
	pkt->header.crash_type = crash_type;
	pkt->header.fault_code = fault_code;
	pkt->header.payload_size = sizeof (struct hsp_crashdump_packet_hsp_production_payload);
	pkt->header.dump_type = CRASH_DUMP_DUMP_TYPE_RELEASE;
	memcpy (pkt->payload.fw_version, fw_version, fw_version_length);
	pkt->payload.riscv.mepc = ctx->mepc;
	pkt->payload.riscv.mstatus = ctx->mstatus;
	pkt->payload.riscv.mcause = ctx->mcause;
	pkt->payload.riscv.mtval = ctx->mtval;
	pkt->payload.riscv.seq_intr = ctx->seq_intr;
	pkt->payload.riscv.ra = ctx->regs.ra;
	pkt->payload.riscv.sp = ctx->regs.sp;
	pkt->payload.riscv.gp = ctx->regs.gp;
	pkt->payload.riscv.tp = ctx->regs.tp;
	pkt->payload.riscv.s0 = ctx->regs.s0;
	pkt->header.magic = HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED;
}

/**
 * Initialize a trap context for storing details about a software panic.  There is no exception
 * executed.  Instead details about the error are stored in the trap context.
 *
 * @param error_code Code identifying the fatal error condition that was encountered.
 * @param error_log Optional log entry containing additional details about the error condition.
 * This can be null if there are no additional details to log.
 * @param ctx The trap context that will be updated with the panic details.
 */
void hsp_crashdump_logging_create_panic_context (int error_code,
	const struct debug_log_entry_info *error_log, struct hsp_trap_context *ctx)
{
	if (ctx == NULL) {
		return;
	}

	ctx->mcause = error_code;

	if (error_log != NULL) {
		ctx->mstatus = 1;
		ctx->regs.ra = error_log->severity;
		ctx->regs.sp = error_log->component;
		ctx->regs.gp = error_log->msg_index;
		ctx->regs.tp = error_log->arg1;
		ctx->regs.s0 = error_log->arg2;
	}
	else {
		ctx->mstatus = 0;
	}
}

/**
 * Save crashdump header and FW version to debug log.
 *
 * @param[in] header The content of crashdump header.
 * @param[in] fw_version The content of FW version.
 */
void hsp_crashdump_logging_save_crashdump_header_to_debug_log (
	const struct crash_dump_packet_header *header, const uint32_t *fw_version)
{
	/* Save crashdump header to debug log. */
	debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_CRASH_DUMP,
		CRASH_DUMP_LOGGING_HEADER, header->fault_code,
		header->core_type << 24 | header->dump_type << 16 | header->crash_type << 8);

	debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HSP,
		HSP_LOGGING_CRASHDUMP_FW_VERSION, fw_version[0], fw_version[1]);
}

/**
 * Save the HSP crash dumppacket into the debug log.
 *
 * @param[in] pkt The crashdump packet.
 *
 * @return 0 if crashdump save succeeded or an error code.
 */
int hsp_crashdump_logging_save_crashdump_to_debug_log (
	struct hsp_crashdump_packet_hsp_production_packet *pkt)
{
	size_t length;
	size_t dev_length;
	uint32_t *dev_data;

	if (pkt == NULL) {
		return HSP_CRASHDUMP_HANDLER_INVALID_ARGUMENT;
	}

	/* Save crashdump header to debug log. */
	hsp_crashdump_logging_save_crashdump_header_to_debug_log (&pkt->header,
		pkt->payload.fw_version);

	/* Save register set defined on the payload format to debug log. */
	crash_dump_riscv_log (&pkt->payload.riscv);

	/* Save HW error opaque data or debug/development data to debug log. */
	length = sizeof (struct crash_dump_packet_header) + pkt->header.payload_size;
	if (length > sizeof (struct hsp_crashdump_packet_hsp_production_packet)) {
		dev_length = length - sizeof (struct hsp_crashdump_packet_hsp_production_packet);
		dev_data = (uint32_t*) ((uint8_t*) pkt +
			sizeof (struct hsp_crashdump_packet_hsp_production_packet));

		switch (pkt->header.fault_code) {
			case HSP_CRASHDUMP_PACKET_FAULT_CODE_ACCESS_ERR:
				hsp_crashdump_hw_err_handler_access_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_BUS_ERR:
				hsp_crashdump_hw_err_handler_bus_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_CHK_ERR:
				hsp_crashdump_hw_err_handler_chk_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_DMB_ERR:
				hsp_crashdump_hw_err_handler_dmb_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_MEM_ERR:
				hsp_crashdump_hw_err_handler_mem_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_MPU_ERR:
				hsp_crashdump_hw_err_handler_mpu_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_RNG_ERR:
				hsp_crashdump_hw_err_handler_rng_err_save_opaque_data (dev_data, dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_WATCHDOG_TIMEOUT:
				hsp_crashdump_hw_err_handler_watchdog_timeout_save_opaque_data (dev_data,
					dev_length);
				break;

			case HSP_CRASHDUMP_PACKET_FAULT_CODE_WDT_ERR:
				hsp_crashdump_hw_err_handler_wdt_err_save_opaque_data (dev_data, dev_length);
				break;

			default:
				crash_dump_logging_save_opaque_data (dev_data, dev_length);
				break;
		}
	}

	/* Flush debug logs. */
	debug_log_flush ();

	return 0;
}

/**
 * Save the HSP crash dumppacket into the persistent RAM.
 *
 * @param[in] pkt The crash dump packet.
 * @param[in] persistent_ram The persistent RAM address.
 * @param[in] persistent_ram_size The persistent RAM size.
 */
void hsp_crashdump_logging_save_crashdump_to_persistent_ram (
	struct hsp_crashdump_packet_hsp_production_packet *pkt, uint32_t *persistent_ram,
	size_t persistent_ram_size)
{
	if ((pkt == NULL) || (persistent_ram == NULL)) {
		return;
	}

	size_t length = sizeof (struct crash_dump_packet_header) +
		pkt->header.payload_size;

	if (length > persistent_ram_size) {
		length = persistent_ram_size;
		if (length >= sizeof (struct crash_dump_packet_header)) {
			pkt->header.payload_size = length - sizeof (struct crash_dump_packet_header);
		}
		else {
			pkt->header.payload_size = 0;
		}
	}

	memcpy (persistent_ram, pkt, length);
}

/**
 * Save a HSP crashdump from persistent RAM if it exists, to debug log.
 *
 * @param[in] persistent_ram The persistent RAM, such as sticky registers, used to store crashdump.
 *
 * @return 0 if crashdump save succeeded or an error code.
 */
int hsp_crashdump_logging_save_crashdump_from_persistent_ram_to_debug_log (
	uint32_t *persistent_ram)
{
	int status = 0;

	if (persistent_ram == NULL) {
		return HSP_CRASHDUMP_HANDLER_INVALID_ARGUMENT;
	}

	/* SPRT booted from HSP crash. The crashdump was stored on persistent RAM. */
	if (*persistent_ram == HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED) {
		status =
			hsp_crashdump_logging_save_crashdump_to_debug_log (
			(struct hsp_crashdump_packet_hsp_production_packet*) persistent_ram);
		/* Set magic # to dirty on sticky register to make the crashdump invalid. */
		*persistent_ram = HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_DIRTY;
	}

	return status;
}

/**
 * Save details for a crash due to stack overflow from persistent RAM into the debug log.
 *
 * If there is no crash dump present or the crash dump is not for a stack overflow, this function
 * does nothing.
 *
 * @param persistent_ram The persistent RAM, such as sticky registers, where the crash dump is
 * stored.
 */
void hsp_crashdump_logging_save_stack_overflow_from_persistent_ram_to_debug_log (
	uint32_t *persistent_ram)
{
	struct hsp_crashdump_packet_hsp_production_packet *pkt =
		(struct hsp_crashdump_packet_hsp_production_packet*) persistent_ram;

	/* Only handle valid stack overflow crashes. */
	if ((pkt != NULL) && (pkt->header.magic == HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED) &&
		(pkt->header.fault_code == HSP_CRASHDUMP_PACKET_FAULT_CODE_STACK_OVER_FLOW)) {
		hsp_crashdump_logging_save_crashdump_header_to_debug_log (&pkt->header,
			pkt->payload.fw_version);

		crash_dump_logging_save_stack_overflow ((void*) (uintptr_t) pkt->payload.riscv.sp);

		debug_log_flush ();

		/* Invalidate the crash dump data. */
		*persistent_ram = HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_DIRTY;
	}
}

/**
 * Save details for a crash due to a firmware panic from persistent RAM into the debug log.
 *
 * If there is no crash dump present or the crash dump is not for a panic, this function does
 * nothing.
 *
 * @param persistent_ram The persistent RAM, such as sticky registers, where the crash dump is
 * stored.
 */
void hsp_crashdump_logging_save_panic_from_persistent_ram_to_debug_log (uint32_t *persistent_ram)
{
	struct hsp_crashdump_packet_hsp_production_packet *pkt =
		(struct hsp_crashdump_packet_hsp_production_packet*) persistent_ram;

	if ((pkt != NULL) && (pkt->header.magic == HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED) &&
		(pkt->header.fault_code == HSP_CRASHDUMP_PACKET_FAULT_CODE_PANIC) &&
		(pkt->header.crash_type == CRASH_DUMP_CRASH_TYPE_NORMAL)) {
		hsp_crashdump_logging_save_crashdump_header_to_debug_log (&pkt->header,
			pkt->payload.fw_version);

		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_SYSTEM,
			SYSTEM_LOGGING_FATAL_ERROR, pkt->payload.riscv.mcause, 0);

		if (pkt->payload.riscv.mstatus != 0) {
			debug_log_create_entry (pkt->payload.riscv.ra, pkt->payload.riscv.sp,
				pkt->payload.riscv.gp, pkt->payload.riscv.tp, pkt->payload.riscv.s0);
		}

		debug_log_flush ();

		/* Invalidate the crash dump data. */
		*persistent_ram = HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_DIRTY;
	}
}
