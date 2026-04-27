// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include "platform_io_api.h"
#include "common/unused.h"
#include "trap/hsp_trap.h"


/**
 * Default handler for all interrupts and exceptions.
 *
 * @param param The trap context.
 */
bool hsp_trap_default_trap_handler (uintptr_t param)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;

	platform_printf ("Unhandled trap!" NEWLINE);
	hsp_trap_print_trap (ctx);

	return false;	// Dispatcher ceases
}

/**
 * Handler for when an exception occurs. Dumps registers and returns to the default trap handler.
 *
 * @param param The value passed from the trap context.
 * @param except_type A string that describes the fault.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_notify_halt (uintptr_t param, const char *except_type)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;

	platform_printf (NEWLINE "EXCEPTION: %s" NEWLINE, except_type);

	hsp_trap_print_trap (ctx);
	hsp_trap_print_regs (ctx);

	// Fall back to the default trap handler
	return false;
}

/**
 * Handler to catch unaligned instruction exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_insn_alignment (uintptr_t param)
{
	return except_notify_halt (param, "instruction address misaligned");
}

/**
 * Handler to catch instruction access fault exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_insn_fault (uintptr_t param)
{
	return except_notify_halt (param, "instruction access fault");
}

/**
 * Handler to catch illegal instruction exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_insn_illegal (uintptr_t param)
{
	return except_notify_halt (param, "illegal instruction");
}

/**
 * Handler to catch unaligned load exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_load_alignment (uintptr_t param)
{
	return except_notify_halt (param, "load address misaligned");
}

/**
 * Handler to catch load access fault exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_load_fault (uintptr_t param)
{
	return except_notify_halt (param, "load access fault");
}

/**
 * Handler to catch unaligned store exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_store_alignment (uintptr_t param)
{
	return except_notify_halt (param, "store/AMO address misaligned");
}

/**
 * Handler to catch store access fault exceptions.
 *
 * @param param The value passed from the trap context.
 *
 * @return False to cause the default trap handler to fire (and CEASE).
 */
static bool except_store_fault (uintptr_t param)
{
	return except_notify_halt (param, "store/AMO access fault");
}

/**
 * Registers generic handlers for standard faults/exceptions.
 */
void traps_init_exception_catch ()
{
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_IAM, except_insn_alignment);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_IAF, except_insn_fault);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_II, except_insn_illegal);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_LAM, except_load_alignment);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_LAF, except_load_fault);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_SAMOAM, except_store_alignment);
	hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_SAMOAF, except_store_fault);
}
