// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "hsp_freertos.h"
#include "task.h"
#include "common/unused.h"
#include "trap/hsp_trap.h"


#if portBYTE_ALIGNMENT != RISCV_STACK_ALIGNMENT
#error Stack alignment out of sync!
#endif


// Importing from hsp_freertos.S
extern __attribute__((__noreturn__)) void hsp_freertos_start_first_task (StackType_t **xCurrentTCB);
extern void hsp_freertos_init_stack (size_t stack);

// FreeRTOS timer variables
extern volatile uint64_t *pullNextTime;
extern volatile uint64_t *pullMachineTimerCompareRegister;
extern const size_t uxTimerIncrementsForOneTick;

// FreeRTOS task variables
extern const StackType_t xISRStackTop;
extern size_t xTaskReturnAddress;
extern portDONT_DISCARD PRIVILEGED_DATA void *volatile pxCurrentTCB;
extern size_t xCriticalNesting;


/**
 * The stack size required for the FreeRTOS context.
 */
#define HSP_FREERTOS_CONTEXT_STACK_SIZE     \
		RISCV_STACK_ALIGN_TYPE (struct hsp_freertos_trap_context)


/**
 * FreeRTOS trap context data.
 */
struct hsp_freertos_trap_context {
	uintptr_t critical_nesting;	/**< Internal critical nesting variable. */
};


/**
 * The RISCV mode specific method to read the timer interrupt pending flag.
 */
static bool (*is_mti_pending) () = NULL;

/**
 * Global flag to indicate whether or not an ISR needs to switch contexts before returning to the
 * application.
 */
static bool prioritized_task_ready = false;


/* Trap handlers */

/**
 * Obtains the FreeRTOS trap context structure for the trap state.  Note that this should only be
 * called from a non-nested (first/final) trap.  We do not allocate this for nested IRQ's.
 *
 * @param param The first argument passed by the trap handler.
 *
 * @return A pointer to the FreeRTOS trap context.
 */
static struct hsp_freertos_trap_context* hsp_freertos_get_context (uintptr_t param)
{
	return (struct hsp_freertos_trap_context*) hsp_trap_get_context (param,
		HSP_FREERTOS_CONTEXT_STACK_SIZE);
}

/**
 * Initializes a FreeRTOS task context to execute in MACHINE mode, sets the lowest interruptable
 * level, and enables interrupts.
 *
 * @param ctx The trap context.
 */
static void hsp_freertos_init_privilege (struct hsp_trap_context *ctx)
{
	// Set trap return privilege to MACHINE, MPIE=1, and MIE=0
	ctx->trap_mstatus = 0x1880;

	/* This is the default reset value of MCAUSE.  This has previous interrupt privilege set to 0
	 * to ensure all interrupts get triggered. */
	ctx->trap_mcause = 0x30000000;
}

/**
 * Gets the MTIME interrupt pending bit when in CLIC mode.
 *
 * @return true if an interrupt is pending, else false.
 */
static bool hsp_freertos_clic_mti_pending ()
{
	return hsp_trap_clic_interrupt_is_pending (HSP_TRAP_INTERRUPT_CODE_MTI);
}

/**
 * Saves the trapped task execution context to the current FreeRTOS task context.
 *
 * @param param The first argument passed by the trap handler.
 *
 * @return The stack size required to allocate the FreeRTOS trap context.
 */
static uintptr_t hsp_freertos_first_trap_begin (uintptr_t param)
{
	UNUSED (param);

	// Interrupts disabled at this point. Safe to access globals.

	// Initialize the context switch flag
	prioritized_task_ready = false;

	// Allocate space to save freertos context
	return HSP_FREERTOS_CONTEXT_STACK_SIZE;
}

/**
 * Saves FreeRTOS context data to the trapped context.
 *
 * @param param The first argument passed by the trap handler.
 */
static void hsp_freertos_first_trap_save (uintptr_t param)
{
	struct hsp_freertos_trap_context *os_ctx = hsp_freertos_get_context (param);

	// Interrupts are disabled at this point. Safe to access globals.

	// xCriticalNesting is only used by tasks, not ISR
	os_ctx->critical_nesting = xCriticalNesting;

	// Save execution context
	*((StackType_t**) pxCurrentTCB) = (StackType_t*) param;
}

/**
 * This initial first trap handler is meant to adjust the privilege of the first FreeRTOS task. In
 * hsp_freertos_start_first_task(), the very first syscall is made, which will call this handler,
 * adjust the privilege, switch to the normal handler, and begin normal execution.
 *
 * @param param The first argument passed by the trap handler.
 */
static void hsp_freertos_first_trap_save_init (uintptr_t param)
{
	// Interrupts disabled at this point. Safe to access globals.

	// Ensure the first task is in the correct privilege
	hsp_freertos_init_privilege ((struct hsp_trap_context*) param);

	hsp_freertos_first_trap_save (param);

	// We don't need this extra processing after the first call
	hsp_trap_first_trap_save_unregister ();
	hsp_trap_first_trap_save_register (hsp_freertos_first_trap_save);
}

/**
 * Switches task contexts if needed and returns the application context that the trap handler will
 * restore and begin/resume executing.
 *
 * @param param The first argument passed by the trap handler.
 *
 * @return Returns the current FreeRTOS task to return execution to.
 */
static uintptr_t hsp_freertos_final_trap_leave (uintptr_t param)
{
	struct hsp_freertos_trap_context *os_ctx;

	// Interrupts are disabled at this point. Safe to access globals.

	// Trap handler wants to return to application.  Handle context switch if required.
	if (prioritized_task_ready) {
		vTaskSwitchContext ();
		prioritized_task_ready = false;
	}

	/* NOTE: vTaskSwitchContext could have been called outside of this optimized implementation.
	 * Always read and get the current context. */
	param = (uintptr_t) *(uintptr_t*) pxCurrentTCB;

	os_ctx = hsp_freertos_get_context (param);

	xCriticalNesting = os_ctx->critical_nesting;

	return param;
}

/**
 * ECALL (syscall) is used to yield a task and switch context to another task.
 *
 * @param param The first argument passed by the trap handler.
 *
 * @return Returns true to signal that the syscall was handled.
 */
static bool hsp_freertos_syscall_handler (uintptr_t param)
{
	// See note in freertos_isr_update_yield
	prioritized_task_ready = true;

	return hsp_trap_syscall_continue (param);
}

/**
 * Handles the MTIME trap to update the FreeRTOS scheduler state.
 *
 * @param param The first argument passed by the trap handler.
 *
 * @return Returns true to signal that the interrupt was handled.
 */
static bool hsp_freertos_mtime_trap (uintptr_t param)
{
	bool (*mti_ip) () = is_mti_pending;
	uint64_t next_time = *pullNextTime;
	uintptr_t mstatus;
	BaseType_t priority_woken;

	UNUSED (param);

	do {
		*pullMachineTimerCompareRegister = next_time;

		mstatus = hsp_trap_mstatus_mie_disable ();
		priority_woken = xTaskIncrementTick ();
		hsp_trap_mstatus_write (mstatus);

		/* NOTE: OSS uxTimerIncrementsForOneTick is calculated off the constant configCPU_CLOCK_HZ
		 * definition.  Does not support a CPU that can have a configurable clock rate. */
		next_time += uxTimerIncrementsForOneTick;
		freertos_isr_update_yield (priority_woken);
		*pullNextTime = next_time;
	} while (mti_ip ());

	return true;
}

/* FreeRTOS handlers */

/**
 * Loads the first task context and begins executing it directly.
 */
__attribute__((__noreturn__)) void xPortStartFirstTask ()
{
	xCriticalNesting = 0;

	hsp_freertos_start_first_task ((StackType_t**) pxCurrentTCB);
}

/**
 * Initialize the stack context for FreeRTOS tasks. First task will be executed directly
 * (xPortStartFirstTask) while future tasks will be swapped in during interrupt context switches.
 *
 * @param pxTopOfStack The top of the stack allocated for the task.
 * @param pxCode Task routine entry point to jump to when starting.
 * @param pvParameters Parameters that get passed to the task routine.
 *
 * @return A pointer to the interrupt context that would be used to restore and begin execution
 * from an interrupt context.
 */
StackType_t* pxPortInitialiseStack (StackType_t *pxTopOfStack, TaskFunction_t pxCode,
	void *pvParameters)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) RISCV_STACK_ALLOC_TYPE (pxTopOfStack,
		struct hsp_trap_context);
	struct hsp_freertos_trap_context *os_ctx =
		(struct hsp_freertos_trap_context*) RISCV_STACK_ALLOC_TYPE (ctx,
		struct hsp_freertos_trap_context);

	register uintptr_t gp __asm__ ("gp");

	memset (ctx, 0, sizeof (*ctx));
	memset (os_ctx, 0, sizeof (*os_ctx));

	// Default return address (not the entry point)
	ctx->regs.ra = (uintptr_t) xTaskReturnAddress;

	// Stack
	ctx->regs.sp = (uintptr_t) pxTopOfStack;

	// Global pointer (same everywhere, be sure to initialize from current gp)
	ctx->regs.gp = (uintptr_t) gp;

	// Task argument
	ctx->regs.a0 = (uintptr_t) pvParameters;

	// MEPC used as the entry return address
	ctx->trap_mepc = (uintptr_t) pxCode;

	hsp_freertos_init_privilege (ctx);

	// Init the FreeRTOS trap context
	os_ctx->critical_nesting = 0;

	return (StackType_t*) ctx;
}

/* API */

/**
 * Initializes environment for FreeRTOS. hsp_trap_init() must have been called prior to calling
 * this.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_freertos_init ()
{
	int status;

	if (hsp_trap_is_vector_mode ()) {
		/* Not supported since chained interrupts require the final interrupt to handle pending
		 * context switches. */
		return IRQ_MODE_UNSUPPORTED;
	}

	// Ensure no interrupts are enabled before configuring
	hsp_trap_init_interrupt_ctl_regs ();

	status = hsp_trap_first_trap_begin_register (hsp_freertos_first_trap_begin);
	if (status != 0) {
		return status;
	}

	status = hsp_trap_first_trap_save_register (hsp_freertos_first_trap_save_init);
	if (status != 0) {
		goto unreg_first_trap_begin;
	}

	/* NOTE: If FreeRTOS ISR debugging is needed, it may be helpful to register for nested_trap
	 * actions to intercept and log nested trap events. */

	status = hsp_trap_final_trap_leave_register (hsp_freertos_final_trap_leave);
	if (status != 0) {
		goto unreg_first_trap_save;
	}

	status = hsp_trap_interrupt_register (HSP_TRAP_INTERRUPT_CODE_MTI, hsp_freertos_mtime_trap);
	if (status != 0) {
		goto unreg_final_trap_leave;
	}

	status = hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_ECALL_M,
		hsp_freertos_syscall_handler);
	if (status != 0) {
		goto unreg_mti;
	}

	status = hsp_trap_exception_register (HSP_TRAP_EXCEPTION_CODE_ECALL_U,
		hsp_freertos_syscall_handler);
	if (status != 0) {
		goto unreg_mcall;
	}

	status = hsp_trap_set_isr_stack ((uintptr_t) xISRStackTop);
	if (status != 0) {
		goto unreg_ucall;
	}

	// Enable mode specific interrupts
	if (hsp_trap_is_clic_mode ()) {
		is_mti_pending = hsp_freertos_clic_mti_pending;

		status = hsp_trap_clic_interrupt_enable (HSP_TRAP_INTERRUPT_CODE_MTI,
			HSP_TRAP_CLIC_LEVEL_DEFAULT_MTI);
		if (status != 0) {
			goto clr_isr_stack;
		}
	}
	else {
		is_mti_pending = hsp_trap_mip_get_mtip;

		// MIE only applies to CLINT mode.
		hsp_trap_mie_set_mask (RISCV_MIE_MSIE | RISCV_MIE_MTIE);
	}

	return 0;

clr_isr_stack:
	hsp_trap_set_isr_stack (0);

unreg_ucall:
	hsp_trap_interrupt_unregister (HSP_TRAP_EXCEPTION_CODE_ECALL_U);

unreg_mcall:
	hsp_trap_interrupt_unregister (HSP_TRAP_EXCEPTION_CODE_ECALL_M);

unreg_mti:
	hsp_trap_interrupt_unregister (HSP_TRAP_INTERRUPT_CODE_MTI);

unreg_final_trap_leave:
	hsp_trap_final_trap_leave_unregister ();

unreg_first_trap_save:
	hsp_trap_first_trap_save_unregister ();

unreg_first_trap_begin:
	hsp_trap_first_trap_begin_unregister ();

	return status;
}

/**
 * Updates the global FreeRTOS ISR yield flag.
 *
 * This is the HSP implementation for the required FreeRTOS platform API to use FromISR functions.
 *
 * @param priority_woken The value returned from any of the FreeRTOS ISR API's.
 */
void freertos_isr_update_yield (BaseType_t priority_woken)
{
	if (priority_woken == pdTRUE) {
		/* A higher priority task than the one in pxCurrentTCB is ready to run.
		 *
		 * NOTE: ENTER_CRITICAL not necessary. Higher priority ISR's will only ever write true to
		 * the flag.  Final ISR will handle the switch and clear the flag in a CRITICAL context. */
		prioritized_task_ready = true;
	}
}
