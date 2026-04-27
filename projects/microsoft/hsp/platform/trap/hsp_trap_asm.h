// Copyright c Microsoft Corporation. All rights reserved.

#if __riscv_xlen == 64
#define WORD_SIZE		8
#define STORE_WORD		sd
#define LOAD_WORD		ld
#elif __riscv_xlen == 32
#define WORD_SIZE		4
#define STORE_WORD		sw
#define LOAD_WORD		lw
#else
#error Assembler did not define __riscv_xlen
#endif

#define WORD_MSB		__riscv_xlen-1

#define RA_OFFSET		0*WORD_SIZE
#define SP_OFFSET		1*WORD_SIZE
#define GP_OFFSET		2*WORD_SIZE
#define TP_OFFSET		3*WORD_SIZE
#define T0_OFFSET		4*WORD_SIZE
#define T1_OFFSET		5*WORD_SIZE
#define T2_OFFSET		6*WORD_SIZE
#define S0_OFFSET		7*WORD_SIZE
#define S1_OFFSET		8*WORD_SIZE
#define A0_OFFSET		9*WORD_SIZE
#define A1_OFFSET		10*WORD_SIZE
#define A2_OFFSET		11*WORD_SIZE
#define A3_OFFSET		12*WORD_SIZE
#define A4_OFFSET		13*WORD_SIZE
#define A5_OFFSET		14*WORD_SIZE
#define A6_OFFSET		15*WORD_SIZE
#define A7_OFFSET		16*WORD_SIZE
#define S2_OFFSET		17*WORD_SIZE
#define S3_OFFSET		18*WORD_SIZE
#define S4_OFFSET		19*WORD_SIZE
#define S5_OFFSET		20*WORD_SIZE
#define S6_OFFSET		21*WORD_SIZE
#define S7_OFFSET		22*WORD_SIZE
#define S8_OFFSET		23*WORD_SIZE
#define S9_OFFSET		24*WORD_SIZE
#define S10_OFFSET		25*WORD_SIZE
#define S11_OFFSET		26*WORD_SIZE
#define T3_OFFSET		27*WORD_SIZE
#define T4_OFFSET		28*WORD_SIZE
#define T5_OFFSET		29*WORD_SIZE
#define T6_OFFSET		30*WORD_SIZE

#define REGS_CTX_SIZE		31*WORD_SIZE

#define TRAP_MEPC_OFFSET		31*WORD_SIZE
#define TRAP_MSTATUS_OFFSET		32*WORD_SIZE
#define TRAP_MCAUSE_OFFSET		33*WORD_SIZE
#define MEPC_OFFSET				34*WORD_SIZE
#define MSTATUS_OFFSET			35*WORD_SIZE
#define MCAUSE_OFFSET			36*WORD_SIZE
#define MTVAL_OFFSET			37*WORD_SIZE
#define SEQ_INTR_OFFSET			38*WORD_SIZE

#define TRAP_CTX_SIZE		39*WORD_SIZE

/* Ensure stack is properly aligned
 * RISCV_32E: 8 byte alignment, else 16 bytes
 * RISCV_32E 64 bit configuration naturally aligned (WORD_SIZE == ALIGNMENT). */

#if !defined (__riscv_32e) || (__riscv_xlen == 32)
#define TRAP_STACK_SIZE		40*WORD_SIZE
#endif

#ifndef TRAP_STACK_SIZE
#define TRAP_STACK_SIZE		TRAP_CTX_SIZE
#endif

// Some compilers don't have these defined
#define MTVT		0x307
#define MNXTI		0x345

#define MSTATUS_MIE		(1 << 3)
