// Copyright (c) Microsoft Corporation. All rights reserved.

#include "trap/hsp_trap_asm.h"

.global hsp_trap_entry_init
.global hsp_trap_entry_point
.global hsp_trap_save_regs
.global hsp_trap_restore_regs
.global hsp_trap_mscratch_read
.global hsp_trap_mstatus_read
.global hsp_trap_mstatus_set_mask
.global hsp_trap_mstatus_clr_mask
.global hsp_trap_mstatus_write
.global hsp_trap_mie_read
.global hsp_trap_mie_set_mask
.global hsp_trap_mie_clr_mask
.global hsp_trap_mie_write
.global hsp_trap_mip_read
.global hsp_trap_get_interrupt_mode
.global hsp_trap_mtvec_set_mask
.global hsp_trap_mtvec_clr_mask

// Importing from hsp_trap.c
.extern hsp_trap_isr_stack
.extern hsp_trap_first_trap_begin
.extern hsp_trap_first_trap_save
.extern hsp_trap_nested_trap_begin
.extern hsp_trap_nested_trap_save
.extern hsp_trap_dispatch
.extern hsp_trap_nested_trap_leave
.extern hsp_trap_final_trap_leave

/* Trap/context routines */

/**
 * The RISCV trap handler entry point. This uses mscratch to track chained interrupts and is
 * configurable to address multiple types of baremetal environments.
 *
 * Trap handler needs to be 64 byte aligned.
 */
#if !defined(__clang__)
.func
#endif
.align 6				// 2^6 = 64 byte aligned required by CLIC
.option rvc
hsp_trap_entry_point:
	#define TRAP_SREG_CTX			s0		/**< Trapped context */
	#define TRAP_SREG_SEQ			s1		/**< Interrupt counter */
	#define TRAP_SREG_SCRATCH		s2		/**< Saved mscratch value */
	#define TRAP_SREG_MEPC			s3		/**< Current mepc value */
	#define TRAP_SREG_MSTATUS		s4		/**< Current mstatus value */
	#define TRAP_SREG_MCAUSE		s5		/**< Current mcause value */
	#define TRAP_SREG_MTVAL			s6		/**< Current mtval value */
	#define TRAP_SREG_TMP1			s7		/**< Temporary reg that is saved cross calls */
	#define TRAP_SREG_TMP2			s8		/**< Temporary reg that is saved cross calls */
	//
	// Interrupts are disabled on entry by hardware
	//

	// Save context
	STORE_WORD		sp, -TRAP_STACK_SIZE+SP_OFFSET(sp)		// Save the top of the trapped stack
	addi			sp, sp, -TRAP_STACK_SIZE				// Allocate trap context stack space
	STORE_WORD		ra, RA_OFFSET(sp)						// Save trapped ra
	call			hsp_trap_save_regs						// Save rest of registers
	ori				TRAP_SREG_CTX, sp, 0					// Used as first argument to each API call

	//
	// Use TRAP_SREG_CTX from this point to read/write to the context
	//

	// Init interrupt state
	li				TRAP_SREG_SEQ, 0		// Init interrupt counter
	csrr			TRAP_SREG_MEPC, mepc
	csrr			TRAP_SREG_MSTATUS, mstatus
	csrr			TRAP_SREG_MCAUSE, mcause
	csrr			TRAP_SREG_MTVAL, mtval
	STORE_WORD		TRAP_SREG_MEPC, TRAP_MEPC_OFFSET(TRAP_SREG_CTX)
	STORE_WORD		TRAP_SREG_MSTATUS, TRAP_MSTATUS_OFFSET(TRAP_SREG_CTX)
	STORE_WORD		TRAP_SREG_MCAUSE, TRAP_MCAUSE_OFFSET(TRAP_SREG_CTX)

	//
	// Use mscratch to determine if we are coming from an application context
	//

	csrrw		TRAP_SREG_SCRATCH, mscratch, TRAP_SREG_CTX		// Save mscratch and write trap context to it
	bnez		TRAP_SREG_SCRATCH, enter_nested_trap			// Nested trap if mscratch is not 0

	//
	// First trap call, mscratch was 0, now TRAP_SREG_CTX
	//

	// Load the ISR stack if one is available, else continue using the trapped stack
	LOAD_WORD		TRAP_SREG_TMP1, hsp_trap_isr_stack
	beqz			TRAP_SREG_TMP1, enter_first_trap
	ori				sp, TRAP_SREG_TMP1, 0
	
enter_first_trap:

	// Load first trap function pointers
	lui			TRAP_SREG_TMP1, %hi(hsp_trap_first_trap_begin)
	lui			TRAP_SREG_TMP2, %hi(hsp_trap_first_trap_save)
	addi		TRAP_SREG_TMP1, TRAP_SREG_TMP1, %lo(hsp_trap_first_trap_begin)
	addi		TRAP_SREG_TMP2, TRAP_SREG_TMP2, %lo(hsp_trap_first_trap_save)
	j			save_context

enter_nested_trap:
	//
	// Was already executing in an interrupted context
	//

	// Restore first trap context and load nested trap function pointers
	csrw		mscratch, TRAP_SREG_SCRATCH
	lui			TRAP_SREG_TMP1, %hi(hsp_trap_nested_trap_begin)
	lui			TRAP_SREG_TMP2, %hi(hsp_trap_nested_trap_save)
	addi		TRAP_SREG_TMP1, TRAP_SREG_TMP1, %lo(hsp_trap_nested_trap_begin)
	addi		TRAP_SREG_TMP2, TRAP_SREG_TMP2, %lo(hsp_trap_nested_trap_save)

save_context:

	// Call the trap entry and context save functions
	LOAD_WORD		ra, 0(TRAP_SREG_TMP1)
	ori				a0, TRAP_SREG_CTX, 0
	jalr			ra, 0(ra)
	beqz			a0, service_interrupts		// Skip calling saver if nothing needed
	sub				sp, sp, a0
	LOAD_WORD		ra, 0(TRAP_SREG_TMP2)
	ori				a0, TRAP_SREG_CTX, 0
	jalr			ra, 0(ra)

service_interrupts:
	//
	// Begin servicing interrupts.
	//

	// Load the high address of the dispatch routine
	lui		TRAP_SREG_TMP1, %hi(hsp_trap_dispatch)

service_loop:

	// Do not enable interrupts, leave it to hsp_trap_dispatch

	// Save the current trap information
	STORE_WORD		TRAP_SREG_MEPC, MEPC_OFFSET(TRAP_SREG_CTX)
	STORE_WORD		TRAP_SREG_MSTATUS, MSTATUS_OFFSET(TRAP_SREG_CTX)
	STORE_WORD		TRAP_SREG_MCAUSE, MCAUSE_OFFSET(TRAP_SREG_CTX)
	STORE_WORD		TRAP_SREG_MTVAL, MTVAL_OFFSET(TRAP_SREG_CTX)

	// Dispatch the interrupt
	STORE_WORD		TRAP_SREG_SEQ, SEQ_INTR_OFFSET(TRAP_SREG_CTX)		// Update interrupt iteration
	ori				a0, TRAP_SREG_CTX, 0
	jalr			ra, %lo(hsp_trap_dispatch)(TRAP_SREG_TMP1)			// Dispatch the interrupt
	csrrci			TRAP_SREG_TMP2, MNXTI, MSTATUS_MIE					// Read mnxti and disable interrupts
		//
		// mnxti returns 0 when in CLINT mode or if no more interrupts are available
		// Also updates mcause, mepc, and mtval
		//
	beqz			TRAP_SREG_TMP2, trap_exit		// Exit the loop if there is no new interrupt

	//
	// More interrupts to handle (not exceptions)
	//

	// Update interrupt state
	addi			TRAP_SREG_SEQ, TRAP_SREG_SEQ, 1							// Increment interrupt count
	csrr			TRAP_SREG_MEPC, mepc
	lui				TRAP_SREG_TMP2, 0x80000									// Only interrupts are serviced (not exceptions), but interrupt flag does not get set
	csrr			TRAP_SREG_MCAUSE, mcause
#if __riscv_xlen > 32
	slli			TRAP_SREG_TMP2, TRAP_SREG_TMP2, __riscv_xlen-20			// Shift in the interrupt flag
#endif
	csrr			TRAP_SREG_MTVAL, mtval
	or				TRAP_SREG_MCAUSE, TRAP_SREG_MCAUSE, TRAP_SREG_TMP2		// Mask in the interrupt flag for the dispatcher
	j				service_loop

trap_exit:
	//
	// No more interrupts. Interrupts disabled.
	//

	// Restore the stack and test if last interrupt in chain
	ori			a0, TRAP_SREG_CTX, 0
	beqz		TRAP_SREG_SCRATCH, leave_final_trap

	//
	// Continue the trap chain
	//

	LOAD_WORD		TRAP_SREG_TMP1, hsp_trap_nested_trap_leave
	jalr			ra, 0(TRAP_SREG_TMP1)
	j				restore_context

leave_final_trap:
	//
	// Completed all interrupts, return to application
	//

	LOAD_WORD		TRAP_SREG_TMP1, hsp_trap_final_trap_leave
	jalr			ra, 0(TRAP_SREG_TMP1)
		// hsp_trap_final_trap_leave may return a new context
	ori				sp, a0, 0
	csrw			mscratch, x0				// Clear mscratch

restore_context:
	//
	// Restore an execution context, using sp
	//

	// Restore registers and exit the trap
	LOAD_WORD		TRAP_SREG_MEPC, TRAP_MEPC_OFFSET(sp)
	LOAD_WORD		TRAP_SREG_MSTATUS, TRAP_MSTATUS_OFFSET(sp)
	LOAD_WORD		TRAP_SREG_MCAUSE, TRAP_MCAUSE_OFFSET(sp)
	andi			TRAP_SREG_MSTATUS, TRAP_SREG_MSTATUS, ~MSTATUS_MIE		// Ensure interrupts disabled while restoring
	csrw			mepc, TRAP_SREG_MEPC
	csrw			mcause, TRAP_SREG_MCAUSE
	csrw			mstatus, TRAP_SREG_MSTATUS
	call			hsp_trap_restore_regs
	LOAD_WORD		ra, RA_OFFSET(sp)
	LOAD_WORD		sp, SP_OFFSET(sp)
	mret

#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Uses the current address in sp to save all the general purpose registers except for ra and sp.
 * It is up to the caller to save sp and ra.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_save_regs:
	STORE_WORD		gp, GP_OFFSET(sp)
	STORE_WORD		tp, TP_OFFSET(sp)
	STORE_WORD		t0, T0_OFFSET(sp)
	STORE_WORD		t1, T1_OFFSET(sp)
	STORE_WORD		t2, T2_OFFSET(sp)
	STORE_WORD		s0, S0_OFFSET(sp)
	STORE_WORD		s1, S1_OFFSET(sp)

	STORE_WORD		a0, A0_OFFSET(sp)
	STORE_WORD		a1, A1_OFFSET(sp)
	STORE_WORD		a2, A2_OFFSET(sp)
	STORE_WORD		a3, A3_OFFSET(sp)
	STORE_WORD		a4, A4_OFFSET(sp)
	STORE_WORD		a5, A5_OFFSET(sp)
	STORE_WORD		a6, A6_OFFSET(sp)
	STORE_WORD		a7, A7_OFFSET(sp)
	STORE_WORD		s2, S2_OFFSET(sp)
	STORE_WORD		s3, S3_OFFSET(sp)

	STORE_WORD		s4, S4_OFFSET(sp)
	STORE_WORD		s5, S5_OFFSET(sp)
	STORE_WORD		s6, S6_OFFSET(sp)
	STORE_WORD		s7, S7_OFFSET(sp)
	STORE_WORD		s8, S8_OFFSET(sp)
	STORE_WORD		s9, S9_OFFSET(sp)
	STORE_WORD		s10, S10_OFFSET(sp)
	STORE_WORD		s11, S11_OFFSET(sp)

	STORE_WORD		t3, T3_OFFSET(sp)
	STORE_WORD		t4, T4_OFFSET(sp)
	STORE_WORD		t5, T5_OFFSET(sp)
	STORE_WORD		t6, T6_OFFSET(sp)
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Uses the current address in sp to restore all the general purpose registers except for ra and
 * sp. It is up to the caller to restore ra and sp.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_restore_regs:
	LOAD_WORD		gp, GP_OFFSET(sp)
	LOAD_WORD		tp, TP_OFFSET(sp)
	LOAD_WORD		t0, T0_OFFSET(sp)
	LOAD_WORD		t1, T1_OFFSET(sp)
	LOAD_WORD		t2, T2_OFFSET(sp)
	LOAD_WORD		s0, S0_OFFSET(sp)
	LOAD_WORD		s1, S1_OFFSET(sp)

	LOAD_WORD		a0, A0_OFFSET(sp)
	LOAD_WORD		a1, A1_OFFSET(sp)
	LOAD_WORD		a2, A2_OFFSET(sp)
	LOAD_WORD		a3, A3_OFFSET(sp)
	LOAD_WORD		a4, A4_OFFSET(sp)
	LOAD_WORD		a5, A5_OFFSET(sp)
	LOAD_WORD		a6, A6_OFFSET(sp)
	LOAD_WORD		a7, A7_OFFSET(sp)
	LOAD_WORD		s2, S2_OFFSET(sp)
	LOAD_WORD		s3, S3_OFFSET(sp)

	LOAD_WORD		s4, S4_OFFSET(sp)
	LOAD_WORD		s5, S5_OFFSET(sp)
	LOAD_WORD		s6, S6_OFFSET(sp)
	LOAD_WORD		s7, S7_OFFSET(sp)
	LOAD_WORD		s8, S8_OFFSET(sp)
	LOAD_WORD	 	s9, S9_OFFSET(sp)
	LOAD_WORD		s10, S10_OFFSET(sp)
	LOAD_WORD		s11, S11_OFFSET(sp)

	LOAD_WORD		t3, T3_OFFSET(sp)
	LOAD_WORD		t4, T4_OFFSET(sp)
	LOAD_WORD		t5, T5_OFFSET(sp)
	LOAD_WORD		t6, T6_OFFSET(sp)
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/* Control register R/W */

/**
 * Reads the mscratch register.
 *
 * @return The current mscratch value.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mscratch_read:
	csrr		a0, mscratch
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Reads the mstatus register.
 *
 * @return The current mstatus value.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mstatus_read:
	csrr		a0, mstatus
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Bitwise OR a mask into mstatus.
 *
 * @param a0 The bitmask to apply
 *
 * @return The value of mstatus before applying the mask.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mstatus_set_mask:
	csrrs		a0, mstatus, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Clears a mask from mstatus (Bitwise NAND)
 *
 * @param a0 The bitmask to apply to mstatus.
 *
 * @return The value of mstatus before applying the mask. 
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mstatus_clr_mask:
	csrrc		a0, mstatus, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Writes a value to mstatus.
 *
 * @param a0 The value to write to mstatus.
 *
 * @return The previous value of mstatus.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mstatus_write:
	csrrw		a0, mstatus, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Reads the mie register.
 *
 * @return The current mie value.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mie_read:
	csrr		a0, mie
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Bitwise OR a mask into mie.
 *
 * @param a0 The bitmask to apply to mie.
 *
 * @return The value of mie before applying the mask.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mie_set_mask:
	csrrs		a0, mie, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Clears a mask from mie (Bitwise NAND).
 *
 * @param a0 The bitmask to apply to mie.
 *
 * @return The value of mie before applying the mask.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mie_clr_mask:
	csrrc		a0, mie, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Writes a value to mie.
 *
 * @param a0 The value to write to mie.
 *
 * @return The previous value of mie.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mie_write:
	csrrw		a0, mie, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Reads the mip register.
 *
 * @return The current mip value.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mip_read:
	csrr		a0, mip
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/* Trap environment configuration */

/**
 * Gets the current trap mode the system is configured to.
 *
 * @return Returns the trap mode.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_get_interrupt_mode:
	csrr		a0, mtvec
	andi		a0, a0, 3
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Bitwise OR a mask into the mtvec register.
 *
 * @param a0 The bitmask to apply to mtvec.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mtvec_set_mask:
	csrs		mtvec, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Clears a mask from the mtvec register (bitwise NAND).
 *
 * @param a0 The mask to apply to mtvec.
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_mtvec_clr_mask:
	csrc	  mtvec, a0
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/

/**
 * Initializes the HSP trap environment.
 *
 * @param a0 The value to write to mtvec
 * @param a1 The value to write to mtvt
 */
#if !defined(__clang__)
.func
#endif
.option rvc
hsp_trap_entry_init:
	csrw		mscratch, x0					// Clear mscratch
	csrw		mtvec, a0						// Set vector to entry as CLINT mode
	csrw		MTVT, a1						// Set mtvt to provided interrupt vector
	ret
#if !defined(__clang__)
	.endfunc
#endif
/*-----------------------------------------------------------------------------------------------*/