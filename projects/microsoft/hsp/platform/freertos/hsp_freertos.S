// Copyright (c) Microsoft Corporation. All rights reserved.

#include "trap/hsp_trap_asm.h"

.global hsp_freertos_start_first_task

// Importing from hsp_trap_asm.S
.extern hsp_trap_restore_regs

/**
 * Begins executing the first FreeRTOS task.
 *
 * @param a0 A pointer to the context to load and execute.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_freertos_start_first_task:
	// Initialize the current context
	LOAD_WORD		sp, 0(a0)
	call			hsp_trap_restore_regs
	LOAD_WORD		ra, TRAP_MEPC_OFFSET(sp)		// Trapped MEPC used as the entry point
	LOAD_WORD		sp, SP_OFFSET(sp)

	//
	// First syscall will enable interrupts and adjust privilege
	//
	ecall

	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/
