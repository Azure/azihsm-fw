// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_TRAP_H_
#define HSP_TRAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "trap/hsp_trap_context.h"
#include "trap/irq_error.h"


/**
 * Default level configured for MTI interrupts
 */
#define HSP_TRAP_CLIC_LEVEL_DEFAULT_MTI		1

/**
 * Default level configured for IRQ interrupts
 */
#define HSP_TRAP_CLIC_LEVEL_DEFAULT_IRQ		2

/**
 * Default level configured for FIQ interrupts
 */
#define HSP_TRAP_CLIC_LEVEL_DEFAULT_FIQ		3

/**
 * Count of priority levels that can be configured for CLIC interrupts
 */
#define HSP_TRAP_CLIC_PRIORITY_LEVEL_COUNT		4

/**
 * MSTATUS flags.
 */
#define RISCV_MSTATUS_UIE		(1 << 0)	/**< Global USER interrupts enabled*/
#define RISCV_MSTATUS_SIE		(1 << 1)	/**< Global SUPERVISOR interrupts enabled*/
#define RISCV_MSTATUS_MIE		(1 << 3)	/**< Global MACHINE interrupts enabled. */

#define RISCV_MSTATUS_INTERRUPT_MASK        \
		(RISCV_MSTATUS_UIE | RISCV_MSTATUS_SIE | RISCV_MSTATUS_MIE)

/**
 * MIE flags.
 */
#define RISCV_MIE_USIE		(1 << 0)	/**< USER software interrupts enabled. */
#define RISCV_MIE_SSIE		(1 << 1)	/**< SUPERVISOR software interrupts enabled. */
#define RISCV_MIE_MSIE		(1 << 3)	/**< MACHINE software interrupts enabled. */
#define RISCV_MIE_UTIE		(1 << 4)	/**< USER timer interrupts enabled. */
#define RISCV_MIE_STIE		(1 << 5)	/**< SUPERVISOR timer interrupts enabled. */
#define RISCV_MIE_MTIE		(1 << 7)	/**< MACHINE timer interrupts enabled. */
#define RISCV_MIE_UEIE		(1 << 8)	/**< USER external interrupts enabled. */
#define RISCV_MIE_SEIE		(1 << 9)	/**< SUPERVISOR external interrupts enabled. */
#define RISCV_MIE_MEIE		(1 << 11)	/**< MACHINE external interrupts enabled. */

/**
 * MIP flags.
 */
#define RISCV_MIP_MSIP		(1 << 3)	/**< MACHINE software interrupt pending. */
#define RISCV_MIP_MTIP		(1 << 7)	/**< MACHINE timer interrupt pending. */
#define RISCV_MIP_MEIP		(1 << 11)	/**< MACHINE external interrupt pending. */

/**
 * Bit mask for all the USER interrupts in MIE.
 */
#define RISCV_MIE_USER_INTERRUPT_MASK		(RISCV_MIE_USIE | RISCV_MIE_UTIE | RISCV_MIE_UEIE)

/**
 * Bit mask for the SUPERVISOR interrupts in MIE.
 */
#define RISCV_MIE_SUPERVISOR_INTERRUPT_MASK		(RISCV_MIE_SSIE | RISCV_MIE_STIE | RISCV_MIE_SEIE)

/**
 * Bit mask for the MACHINE interrupts in MIE.
 */
#define RISCV_MIE_MACHINE_INTERRUPT_MASK		(RISCV_MIE_MSIE | RISCV_MIE_MTIE | RISCV_MIE_MEIE)

/**
 * Bit mask for the all interrupts in MIE.
 */
#define RISCV_MIE_INTERRUPT_MASK        \
		(RISCV_MIE_USER_INTERRUPT_MASK | RISCV_MIE_SUPERVISOR_INTERRUPT_MASK | RISCV_MIE_MACHINE_INTERRUPT_MASK)

/**
 * RISCV stack alignment.
 */
#ifdef __riscv_32e
#define RISCV_STACK_ALIGNMENT		8
#else
#define RISCV_STACK_ALIGNMENT		16
#endif

/**
 * Low byte mask for the RISCV stack alignment.
 */
#define RISCV_STACK_ALIGN_MASK		(RISCV_STACK_ALIGNMENT - 1)

/**
 * Rounds a value down to be properly aligned to the stack.
 *
 * @param value The value to align for the stack.
 *
 * @return A properly aligned uintptr_t value.
 */
#define RISCV_STACK_ALIGN_DOWN(value)		((uintptr_t) (value) & ~RISCV_STACK_ALIGN_MASK)

/**
 * Rounds a value up to be properly aligned for the stack.
 *
 * @param value The value to align for the stack.
 *
 * @return A properly aligned uintptr_t value.
 */
#define RISCV_STACK_ALIGN_UP(\
	value)		   RISCV_STACK_ALIGN_DOWN ((uintptr_t) (value) + RISCV_STACK_ALIGN_MASK)

/**
 * Rounds up the size of a type to adhere to stack requirements.
 *
 * @param value The value to align for the stack.
 *
 * @return A properly aligned uintptr_t value.
 */
#define RISCV_STACK_ALIGN_TYPE(type)		RISCV_STACK_ALIGN_UP (sizeof (type))

/**
 * Allocates space from a stack pointer.  This expects the stack to be properly aligned.
 *
 * @param stack The current stack pointer.
 * @param size The size of data to alloc.
 *
 * @return A pointer on the stack that will hold the target memory.
 */
#define RISCV_STACK_ALLOC_TYPE(stack,\
		type) ((void*) ((uintptr_t) (stack) - RISCV_STACK_ALIGN_TYPE (type)))


/**
 * RISCV exception codes
 */
enum hsp_trap_exception_code {
	HSP_TRAP_EXCEPTION_CODE_IAM	= 0,		/**< Instruction address misaligned */
	HSP_TRAP_EXCEPTION_CODE_IAF	= 1,		/**< Instruction access fault */
	HSP_TRAP_EXCEPTION_CODE_II = 2,			/**< Illegal instruction */
	HSP_TRAP_EXCEPTION_CODE_BREAK = 3,		/**< Breakpoint */
	HSP_TRAP_EXCEPTION_CODE_LAM	= 4,		/**< Load address misaligned */
	HSP_TRAP_EXCEPTION_CODE_LAF	= 5,		/**< Load access fault */
	HSP_TRAP_EXCEPTION_CODE_SAMOAM = 6,		/**< Store/AMO address misaligned */
	HSP_TRAP_EXCEPTION_CODE_SAMOAF = 7,		/**< Store/AMO access fault */
	HSP_TRAP_EXCEPTION_CODE_ECALL_U	= 8,	/**< Environment call from U-mode */
	HSP_TRAP_EXCEPTION_CODE_R9 = 9,			/**< Reserved - Environment call from S-mode */
	HSP_TRAP_EXCEPTION_CODE_R10	= 10,		/**< Reserved */
	HSP_TRAP_EXCEPTION_CODE_ECALL_M	= 11,	/**< Environment call from M-mode */
	HSP_TRAP_EXCEPTION_CODE_R12	= 12,		/**< Reserved - Instruction page fault */
	HSP_TRAP_EXCEPTION_CODE_R13	= 13,		/**< Reserved - Load page fault */
	HSP_TRAP_EXCEPTION_CODE_R14	= 14,		/**< Reserved */
	HSP_TRAP_EXCEPTION_CODE_R15	= 15,		/**< Reserved - Store/AMO page fault */
	HSP_TRAP_EXCEPTION_CODE_MAX,			/**< Max number of defined exceptions */
};

/**
 * RISCV interrupt codes
 */
enum hsp_trap_interrupt_code {
	HSP_TRAP_INTERRUPT_CODE_R0 = 0,		/**< Reserved - User software interrupt */
	HSP_TRAP_INTERRUPT_CODE_R1 = 1,		/**< Reserved - Supervisor software interrupt */
	HSP_TRAP_INTERRUPT_CODE_R2 = 2,		/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_MSI	= 3,	/**< Machine Software Interrupt */
	HSP_TRAP_INTERRUPT_CODE_R4 = 4,		/**< Reserved - User timer interrupt */
	HSP_TRAP_INTERRUPT_CODE_R5 = 5,		/**< Reserved - Supervisor timer interrupt */
	HSP_TRAP_INTERRUPT_CODE_R6 = 6,		/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_MTI	= 7,	/**< Machine Timer Interrupt */
	HSP_TRAP_INTERRUPT_CODE_R8 = 8,		/**< Reserved - User external interrupt */
	HSP_TRAP_INTERRUPT_CODE_R9 = 9,		/**< Reserved - Supervisor external interrupt */
	HSP_TRAP_INTERRUPT_CODE_R10	= 10,	/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_MEI	= 11,	/**< Machine External Interrupt */
	HSP_TRAP_INTERRUPT_CODE_CSIP = 12,	/**< CLIC Software Interrupt Pending */
	HSP_TRAP_INTERRUPT_CODE_R13	= 13,	/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_R14	= 14,	/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_R15	= 15,	/**< Reserved */
	HSP_TRAP_INTERRUPT_CODE_IRQ	= 16,	/**< CLIC Local Interrupt 0 */
	HSP_TRAP_INTERRUPT_CODE_FIQ	= 17,	/**< CLIC Local Interrupt 1 */
	HSP_TRAP_INTERRUPT_CODE_CLIC2 = 18,	/**< CLIC Local Interrupt 2 */
	HSP_TRAP_INTERRUPT_CODE_CLIC3 = 19,	/**< CLIC Local Interrupt 3 */
	HSP_TRAP_INTERRUPT_CODE_CLIC4 = 20,	/**< CLIC Local Interrupt 4 */
	HSP_TRAP_INTERRUPT_CODE_CLIC5 = 21,	/**< CLIC Local Interrupt 5 */
	HSP_TRAP_INTERRUPT_CODE_CLIC6 = 22,	/**< CLIC Local Interrupt 6 */
	HSP_TRAP_INTERRUPT_CODE_CLIC7 = 23,	/**< CLIC Local Interrupt 7 */
	HSP_TRAP_INTERRUPT_CODE_MAX,		/**< Max number of defined interrupts */
};


/**
 * Simple action callback for various trap API calls.
 *
 * @param param A register sized parameter, context dependent.
 */
typedef void (*hsp_trap_action) (uintptr_t param);

/**
 * Callback for various trap API calls that is meant to return a value.
 *
 * @param param A register sized parameter, context dependent.
 *
 * @return A register sized value, context dependent.
 */
typedef uintptr_t (*hsp_trap_reg_getter) (uintptr_t param);

/**
 * Trap routine callback to handle an event.
 *
 * @param param A register sized parameter, context dependent.
 *
 * @return True if event handled, else false.
 */
typedef bool (*hsp_trap_routine) (uintptr_t param);


/* App implemented default handler routines */

extern bool hsp_trap_default_trap_handler (uintptr_t param0);

/* CPU/interrupt config API */

int hsp_trap_init (bool clic_mode, uintptr_t vector);

bool hsp_trap_clic_interrupt_is_enabled (unsigned intr_code);
bool hsp_trap_clic_interrupt_is_pending (unsigned intr_code);
int hsp_trap_clic_interrupt_enable (unsigned intr_code, unsigned priority);
int hsp_trap_clic_interrupt_disable (unsigned intr_code);

unsigned hsp_trap_get_interrupt_mode ();
bool hsp_trap_is_clic_mode ();
bool hsp_trap_is_vector_mode ();

uintptr_t hsp_trap_mstatus_read ();
bool hsp_trap_mstatus_mie_is_enabled ();
uintptr_t hsp_trap_mstatus_mie_enable ();
uintptr_t hsp_trap_mstatus_mie_disable ();
uintptr_t hsp_trap_mstatus_set_mask (uintptr_t mask);
uintptr_t hsp_trap_mstatus_clr_mask (uintptr_t mask);
uintptr_t hsp_trap_mstatus_write (uintptr_t mstatus);

uintptr_t hsp_trap_mie_read ();
uintptr_t hsp_trap_mie_msie_enable ();
uintptr_t hsp_trap_mie_msie_disable ();
uintptr_t hsp_trap_mie_mtie_enable ();
uintptr_t hsp_trap_mie_mtie_disable ();
uintptr_t hsp_trap_mie_meie_enable ();
uintptr_t hsp_trap_mie_meie_disable ();
uintptr_t hsp_trap_mie_set_mask (uintptr_t mask);
uintptr_t hsp_trap_mie_clr_mask (uintptr_t mask);
uintptr_t hsp_trap_mie_write (uintptr_t mie);

uintptr_t hsp_trap_mip_read ();
bool hsp_trap_mip_get_msip ();
bool hsp_trap_mip_get_mtip ();
bool hsp_trap_mip_get_meip ();

void hsp_trap_init_interrupt_ctl_regs ();

/* Event handling API */

int hsp_trap_interrupt_register (unsigned intr_code, hsp_trap_routine handler);
int hsp_trap_interrupt_unregister (unsigned intr_code);

int hsp_trap_exception_register (unsigned except_code, hsp_trap_routine handler);
int hsp_trap_exception_unregister (unsigned except_code);

void hsp_trap_set_nested_interrupt_behavior (bool enable_in_interrupts, bool enable_in_exceptions);

/* Trap processing API */

int hsp_trap_set_isr_stack (uintptr_t stack_addr);

int hsp_trap_first_trap_begin_register (hsp_trap_reg_getter action);
int hsp_trap_first_trap_begin_unregister ();

int hsp_trap_first_trap_save_register (hsp_trap_action saver);
int hsp_trap_first_trap_save_unregister ();

int hsp_trap_nested_trap_begin_register (hsp_trap_reg_getter action);
int hsp_trap_nested_trap_begin_unregister ();

int hsp_trap_nested_trap_save_register (hsp_trap_action saver);
int hsp_trap_nested_trap_save_unregister ();

int hsp_trap_nested_trap_leave_register (hsp_trap_action action);
int hsp_trap_nested_trap_leave_unregister ();

int hsp_trap_final_trap_leave_register (hsp_trap_reg_getter getter);
int hsp_trap_final_trap_leave_unregister ();

/* Trap handler API */

void* hsp_trap_get_context (uintptr_t param, size_t stack_size);
bool hsp_trap_in_trap ();
bool hsp_trap_is_nested (uintptr_t param);
bool hsp_trap_mcause_is_interrupt (uintptr_t mcause);
unsigned hsp_trap_mcause_code (uintptr_t mcause);
bool hsp_trap_exception_continue (uintptr_t param);
bool hsp_trap_syscall_continue (uintptr_t param);
void hsp_trap_print_entry (struct hsp_trap_context *ctx);
void hsp_trap_print_trap (struct hsp_trap_context *ctx);
void hsp_trap_print_regs (struct hsp_trap_context *ctx);


#endif	/* HSP_TRAP_H_ */
