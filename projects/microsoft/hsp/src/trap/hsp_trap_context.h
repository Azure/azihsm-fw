// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_TRAP_CONTEXT_H_
#define HSP_TRAP_CONTEXT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/**
 * Holds all the general purpose RISCV registers, except for x0, which is hardwired to 0.
 */
struct hsp_trap_regs {
	uintptr_t ra;	/**< x1, return address */
	uintptr_t sp;	/**< x2, stack pointer */
	uintptr_t gp;	/**< gp, global pointer */
	uintptr_t tp;	/**< x4, thread pointer */
	uintptr_t t0;	/**< x5, temporary */
	uintptr_t t1;	/**< x6, temporary */
	uintptr_t t2;	/**< x7, temporary */
	uintptr_t s0;	/**< x8/fp - saved register / frame pointer */
	uintptr_t s1;	/**< x9, saved register */
	uintptr_t a0;	/**< x10, function argument / return value */
	uintptr_t a1;	/**< x11, function argument / return value */
	uintptr_t a2;	/**< x12, function argument */
	uintptr_t a3;	/**< x13, function argument */
	uintptr_t a4;	/**< x14, function argument */
	uintptr_t a5;	/**< x15, function argument */
	uintptr_t a6;	/**< x16, function argument */
	uintptr_t a7;	/**< x17, function argument */
	uintptr_t s2;	/**< x18, saved register */
	uintptr_t s3;	/**< x19, saved register */
	uintptr_t s4;	/**< x20, saved register */
	uintptr_t s5;	/**< x21, saved register */
	uintptr_t s6;	/**< x22, saved register */
	uintptr_t s7;	/**< x23, saved register */
	uintptr_t s8;	/**< x24, saved register */
	uintptr_t s9;	/**< x25, saved register */
	uintptr_t s10;	/**< x26, saved register */
	uintptr_t s11;	/**< x27, saved register */
	uintptr_t t3;	/**< x28, temporary */
	uintptr_t t4;	/**< x29, temporary */
	uintptr_t t5;	/**< x30, temporary */
	uintptr_t t6;	/**< x31, temporary */
};

/**
 * Contains the entire context state needed for the trap handler.
 */
struct hsp_trap_context {
	struct hsp_trap_regs regs;	/**< Standard RISCV register context */
	uintptr_t trap_mepc;		/**< Original mepc value upon trap entry */
	uintptr_t trap_mstatus;		/**< Original mstatus value upon trap entry */
	uintptr_t trap_mcause;		/**< Original mcause value upon trap entry */
	uintptr_t mepc;				/**< Current machine exception return pc */
	uintptr_t mstatus;			/**< Machine status */
	uintptr_t mcause;			/**< Trap cause */
	uintptr_t mtval;			/**< Additional trap info */
	uintptr_t seq_intr;			/**< Number of sequential interrupts processed */
};


#endif	/* HSP_TRAP_CONTEXT_H_ */
