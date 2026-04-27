// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include "platform_config.h"
#include "platform_io_api.h"
#include "common/unused.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"
#include "trap/hsp_trap.h"
#include "trap/hsp_trap_asm.h"


// Ensure the manually computed trap context size is correct
_Static_assert (TRAP_CTX_SIZE == sizeof (struct hsp_trap_context),
	"Manually computed trap context size incorrect!");

// Ensure the manually computed context stack size is aligned properly
#if (TRAP_STACK_SIZE % RISCV_STACK_ALIGNMENT) != 0
#error Trap stack not aligned!
#endif	/* TRAP_STACK_SIZE */


// Importing from hsp_trap_asm.S
extern void hsp_trap_entry_init (uintptr_t mtvec, uintptr_t mtvt);
extern void hsp_trap_entry_point ();
extern uintptr_t hsp_trap_mscratch_read ();
extern void hsp_trap_mtvec_set_mask (uintptr_t mask);
extern void hsp_trap_mtvec_clr_mask (uintptr_t mask);


/**
 * Sifive E20 CLIC priority.
 */
#define E20_CLIC_PRIORITY_BITS		2								/**< E20 only uses 2 bits for priority */
#define E20_CLIC_PRIORITY_SHIFT		(8 - E20_CLIC_PRIORITY_BITS)	/**< Shift to get interrupt priority */

/**
 * MTVEC flags.
 */
#define RISCV_MTVEC_VEC_MODE		(1 << 0)						/**< VECTOR mode flag. */
#define RISCV_MTVEC_CLIC_MODE		(1 << 1)						/**< CLIC mode flag. */


/**
 * Creates a CLIC HART0 register to read and write to.
 */
#define RISCV_CLIC_HART0_REG(reg) ((volatile uint8_t*) RISCV_CLIC_HART0_ ## reg)


/**
 * An address to a stack designated to process interrupts with.  If 0, the trap handler will use
 * the stack of the trapped context.
 */
uintptr_t hsp_trap_isr_stack = 0;

/**
 * The registered callback to notify that the application just entered a trap.
 */
hsp_trap_reg_getter hsp_trap_first_trap_begin;

/**
 * The registered callback to notify that the application is ready to save it's context.
 */
hsp_trap_action hsp_trap_first_trap_save;

/**
 * The registered callback to notify that a nested IRQ just entered a trap.
 */
hsp_trap_reg_getter hsp_trap_nested_trap_begin;

/**
 * The registered callback to notify that the nested trap is ready to save it's context.
 */
hsp_trap_action hsp_trap_nested_trap_save;

/**
 * The registered callback to notify that the trap is going to return to the previous trap.
 */
hsp_trap_action hsp_trap_nested_trap_leave;

/**
 * The registered callback to notify that the trap is going to return to the application.
 */
hsp_trap_reg_getter hsp_trap_final_trap_leave;

/**
 * The trap vector to handle RISCV exceptions.
 */
static hsp_trap_routine exception_vector[HSP_TRAP_EXCEPTION_CODE_MAX];

/**
 * The trap vector to handle RISCV interrupts.
 */
static hsp_trap_routine interrupt_vector[HSP_TRAP_INTERRUPT_CODE_MAX];

/**
 * If true, enable nested interrupts from interrupt context. Defaults to true.
 */
static bool enable_nested_interrupts_in_interrupts;

/**
 * If true, enable interrupts during exception handling.
 * Defaults to true to allow interrupts during exceptions.
 * This is legacy behavior, some platforms may depend on the watchdog being
 * serviced during exception handling, for instance.
 */
static bool enable_nested_interrupts_in_exceptions;


/**
 * Gets a pointer to an element in a trap vector.
 *
 * @param vector The vector array
 * @param count The count of elements in the vector
 * @param code The event code to obtain the element pointer to
 *
 * @return NULL if invalid code, else a pointer to the element.
 */
static hsp_trap_routine* hsp_trap_vector_entry (hsp_trap_routine *vector, unsigned count,
	unsigned code)
{
	if (code >= count) {
		return NULL;
	}

	return vector + code;
}

/**
 * Assigns a handler to the desired trap event in the vector.
 *
 * @param vector The vector array
 * @param count The count of elements in the vector
 * @param code The event code to register the handler for
 * @param handler The handler callback
 *
 * @return 0 if successful, else an error code
 */
static int hsp_trap_vector_register (hsp_trap_routine *vector, unsigned count, unsigned code,
	hsp_trap_routine handler)
{
	hsp_trap_routine *entry;

	if (handler == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	entry = hsp_trap_vector_entry (vector, count, code);
	if (entry == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	if (*entry != hsp_trap_default_trap_handler) {
		// A registrar already exists or hsp_trap_init () was not called
		return IRQ_ALREADY_REGISTERED;
	}

	*entry = handler;

	return 0;
}

/**
 * Sets a trap event to the default handler.
 *
 * @param vector The vector array
 * @param count The count of elements in the vector
 * @param code The event code to unregister the handler for
 *
 * @return 0 if successful, else an error code
 */
static int hsp_trap_vector_unregister (hsp_trap_routine *vector, unsigned count, unsigned code)
{
	hsp_trap_routine *entry;

	entry = hsp_trap_vector_entry (vector, count, code);
	if (entry == NULL) {
		return IRQ_INVALID_ARGUMENT;
	}

	if (*entry == hsp_trap_default_trap_handler) {
		return IRQ_NOT_REGISTERED;
	}

	*entry = hsp_trap_default_trap_handler;

	return 0;
}

/**
 * Set nested interrupt behavior.
 * External users should called this function after calling hsp_trap_init as
 * hsp_trap_init sets the default behavior. The default behavior is to have
 * nested interrupts enabled in both interrupt and exception context.
 *
 * @param enable_in_interrupts Enable nested interrupts in interrupt context
 * @param enable_in_exceptions Enable nested interrupts in exception context
 */
void hsp_trap_set_nested_interrupt_behavior (bool enable_in_interrupts, bool enable_in_exceptions)
{
	enable_nested_interrupts_in_interrupts = enable_in_interrupts;
	enable_nested_interrupts_in_exceptions = enable_in_exceptions;
}

/**
 * Registers an action to a trap extension event.
 *
 * @param event The target event to register for.
 * @param default_act The default handler for the target event.
 * @param action The action to register.
 *
 * @return 0 if successful, else an error code
 */
static int hsp_trap_action_register (void **event, void *default_act, void *action)
{
	if ((event == NULL) || (default_act == NULL) || (action == NULL)) {
		return IRQ_INVALID_ARGUMENT;
	}

	if (*event != default_act) {
		// A registrar already exists or hsp_trap_init () was not called.
		return IRQ_ALREADY_REGISTERED;
	}

	*event = action;

	return 0;
}

/**
 * Sets the default action to a trap extension event.
 *
 * @param event The target event to register for.
 * @param default_act The default action for the target event.
 *
 * @return 0 if successful, else an error code
 */
static int hsp_trap_action_unregister (void **event, void *default_act)
{
	if ((event == NULL) || (default_act == NULL)) {
		return IRQ_INVALID_ARGUMENT;
	}

	if (*event == default_act) {
		return IRQ_NOT_REGISTERED;
	}

	*event = default_act;

	return 0;
}

/**
 * Default trap action that is just a NOP.
 *
 * @param param The first parameter passed by the trap handler
 */
static void hsp_trap_action_default (uintptr_t param)
{
	UNUSED (param);

	// Do nothing
}

/**
 * Default handler for entering a trapped context.  No extra context data is needed.
 *
 * @param param The first parameter passed by the trap handler
 *
 * @return 0 to prevent any stack allocation.
 */
static uintptr_t hsp_trap_trap_begin_default (uintptr_t param)
{
	UNUSED (param);

	return 0;
}

/**
 * Default handler to return to the application from handling interrupts.
 *
 * @param param The first parameter passed by the trap handler
 *
 * @return Returns the passed parameter
 */
static uintptr_t hsp_trap_final_trap_leave_default (uintptr_t param)
{
	// Continue with executing context
	return param;
}

/* Trap handler extensions */

/**
 * Processes a trap event and dispatches it to the proper handler. If the handler doesn't
 * handle the event, it falls back to the default handler.
 *
 * @param param The first parameter passed by the trap handler
 */
void hsp_trap_dispatch (uintptr_t param)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;
	size_t mcause = ctx->mcause;
	hsp_trap_routine handler;
	hsp_trap_routine *vector;
	unsigned count;
	bool interrupt;
	// Save interrupt state
	uintptr_t mstatus = hsp_trap_mstatus_read ();

	interrupt = hsp_trap_mcause_is_interrupt (mcause);
	mcause = hsp_trap_mcause_code (mcause);

	if (interrupt) {
		if (enable_nested_interrupts_in_interrupts) {
			hsp_trap_mstatus_mie_enable ();
		}
		vector = interrupt_vector;
		count = HSP_TRAP_INTERRUPT_CODE_MAX;
	}
	else {
		if (enable_nested_interrupts_in_exceptions) {
			hsp_trap_mstatus_mie_enable ();
		}
		vector = exception_vector;
		count = HSP_TRAP_EXCEPTION_CODE_MAX;
	}

	// Will never be NULL, should always at least be hsp_trap_default_trap_handler
	handler = *hsp_trap_vector_entry (vector, count, mcause);
	if (!handler (param)) {
		// Fallback to the default handler
		if ((handler == hsp_trap_default_trap_handler) ||
			!hsp_trap_default_trap_handler (param)) {
			// Unhandled event...
			// TODO: Maybe think about internal logging of this situation?
			CEASE;
		}
	}
	// Restore interrupt state to what it was on entry
	hsp_trap_mstatus_write (mstatus);
}

/**
 * Increments the context MEPC to the next instruction.
 *
 * @param param The parameter passed by the trap hander
 *
 * @return Returns the passed parameter
 */
uintptr_t hsp_trap_mepc_increment (uintptr_t param)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;
	size_t mepc = ctx->trap_mepc;
	uint16_t instruction = *(const uint16_t*) mepc;

	// Compressed, 16-bit instructions are when low 2 bits are not b'11

	mepc += 2;
	if ((instruction & 3) == 3) {
		// Normal 4 byte base instruction
		mepc += 2;
	}

	// SiFive E20 doesn't have other larger instruction extensions

	ctx->trap_mepc = mepc;

	return param;
}

/* CPU/interrupt config API */

/**
 * Initializes the CPU trap environment.  This only initializes a MACHINE mode trap environment and
 * disables lower priority interrupts.
 *
 * @param clic_mode True to configure trap environment for CLIC mode, else false for CLINT.
 * @param vector Optional vector address provided by the application to handle vectored interrupts.
 * Setting vector to 0 configures the trap handler to non-vectored mode. Application vector entries
 * can jump/point to hsp_trap_entry_point if they don't want to handle that specific interrupt in a
 * special way.
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_init (bool clic_mode, uintptr_t vector)
{
	uintptr_t entry_point = (uintptr_t) hsp_trap_entry_point;
	bool vectored = (vector != 0);
	int status;
	size_t i;

	hsp_trap_set_nested_interrupt_behavior (true, true);

	if (!clic_mode) {
		// CLINT mode
		if (vectored) {
			// CLINT vector requries 128 byte alignment
			if ((vector % 128) != 0) {
				return IRQ_VECTOR_NOT_ALIGNED;
			}

			// CLINT vectored assigns the vector to mtvec
			entry_point = vector;
			vector = 0;
		}	// CLINT direct requires 4 byte alignment
		else if ((entry_point % 4) != 0) {
			return IRQ_ENTRY_NOT_ALIGNED;
		}
	}
	else {
		// CLIC requires 64 byte alignment

		if ((entry_point % 64) != 0) {
			return IRQ_ENTRY_NOT_ALIGNED;
		}

		if ((vector % 64) != 0) {
			return IRQ_VECTOR_NOT_ALIGNED;
		}
	}

	// Ensure no interrupts are enabled before configuring.
	hsp_trap_init_interrupt_ctl_regs ();

	// SiFive E20 ClicIntCfg.NlBits is 2, disable selective vectoring.
	*RISCV_CLIC_HART0_REG (CFG) = E20_CLIC_PRIORITY_BITS << 1;

	status = hsp_trap_first_trap_begin_unregister ();
	if (status != 0) {
		return status;
	}

	status = hsp_trap_first_trap_save_unregister ();
	if (status != 0) {
		return status;
	}

	status = hsp_trap_nested_trap_begin_unregister ();
	if (status != 0) {
		return status;
	}

	status = hsp_trap_nested_trap_save_unregister ();
	if (status != 0) {
		return status;
	}

	status = hsp_trap_nested_trap_leave_unregister ();
	if (status != 0) {
		return status;
	}

	status = hsp_trap_final_trap_leave_unregister ();
	if (status != 0) {
		return status;
	}

	for (i = 0; i < HSP_TRAP_EXCEPTION_CODE_MAX; ++i) {
		status = hsp_trap_exception_unregister (i);
		if (status != 0) {
			return status;
		}
	}

	for (i = 0; i < HSP_TRAP_INTERRUPT_CODE_MAX; ++i) {
		status = hsp_trap_interrupt_unregister (i);
		if (status != 0) {
			return status;
		}

		status = hsp_trap_clic_interrupt_disable (i);
		if (status != 0) {
			return status;
		}
	}

	// Initializes as CLINT
	hsp_trap_entry_init (entry_point, vector);

	if (clic_mode) {
		hsp_trap_mtvec_set_mask (RISCV_MTVEC_CLIC_MODE);
	}

	if (vectored) {
		hsp_trap_mtvec_set_mask (RISCV_MTVEC_VEC_MODE);
	}

	return 0;
}

/**
 * Gets a flag inidicating if a CLIC interrupt is enabled.
 *
 * @param intr_code The target RISCV interrupt.
 *
 * @return true if the interrupt is enabled, else false.
 */
bool hsp_trap_clic_interrupt_is_enabled (unsigned intr_code)
{
	if (intr_code >= HSP_TRAP_INTERRUPT_CODE_MAX) {
		return false;
	}

	return !!(RISCV_CLIC_HART0_REG (INT_IE)[intr_code] & 1);
}

/**
 * Gets a flag inidicating if a CLIC interrupt is pending.
 *
 * @param intr_code The target RISCV interrupt.
 *
 * @return true if an interrupt is pending, else false.
 */
bool hsp_trap_clic_interrupt_is_pending (unsigned intr_code)
{
	if (intr_code >= HSP_TRAP_INTERRUPT_CODE_MAX) {
		return false;
	}

	return !!(RISCV_CLIC_HART0_REG (INT_IP)[intr_code] & 1);
}

/**
 * Enable an interrupt in CLIC mode
 *
 * @param intr_code The target RISCV interrupt to enable
 * @param level The priority
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_clic_interrupt_enable (unsigned intr_code, unsigned priority)
{
	if ((intr_code >= HSP_TRAP_INTERRUPT_CODE_MAX) ||
		(priority >= HSP_TRAP_CLIC_PRIORITY_LEVEL_COUNT)) {
		return IRQ_INVALID_ARGUMENT;
	}

	RISCV_CLIC_HART0_REG (INT_CFG)[intr_code] = (priority << E20_CLIC_PRIORITY_SHIFT);

	RISCV_CLIC_HART0_REG (INT_IE)[intr_code] = 1;

	return 0;
}

/**
 * Disable an interrupt in CLIC mode
 *
 * @param intr_code The target RISCV interrupt to disable
 *
 * @return true if successful, else false
 */
int hsp_trap_clic_interrupt_disable (unsigned intr_code)
{
	if (intr_code >= HSP_TRAP_INTERRUPT_CODE_MAX) {
		return IRQ_INVALID_ARGUMENT;
	}

	RISCV_CLIC_HART0_REG (INT_IE)[intr_code] = 0;

	return 0;
}

/**
 * Gets a flag to indicate whether trap environment is configured for CLIC or CLINT.
 *
 * @return true if in CLIC mode, else false for CLINT.
 */
bool hsp_trap_is_clic_mode ()
{
	return (hsp_trap_get_interrupt_mode () >= RISCV_MTVEC_CLIC_MODE);
}

/**
 * Gets a flag to indicate whether trap environment is configured for VECTOR or DIRECT mode.
 *
 * @return true if in VECTOR mode, else false for DIRECT.
 */
bool hsp_trap_is_vector_mode ()
{
	// Should always be in direct mode as this API doesn't currently support vectored.

	return !!(hsp_trap_get_interrupt_mode () & RISCV_MTVEC_VEC_MODE);
}

/**
 * Gets a flag indicating if global MACHINE interrupts are enabled.
 *
 * @return true if enabled, else false.
 */
bool hsp_trap_mstatus_mie_is_enabled ()
{
	return !!(hsp_trap_mstatus_read () & RISCV_MSTATUS_MIE);
}

/**
 * Enables global interrupts in MACHINE mode.
 *
 * @return The value of mstatus before the mask was applied.
 */
uintptr_t hsp_trap_mstatus_mie_enable ()
{
	return hsp_trap_mstatus_set_mask (RISCV_MSTATUS_MIE);
}

/**
 * Disables global interrupts in MACHINE mode.
 *
 * @return The value of mstatus before the mask was applied.
 */
uintptr_t hsp_trap_mstatus_mie_disable ()
{
	return hsp_trap_mstatus_clr_mask (RISCV_MSTATUS_MIE);
}

/**
 * Enables software interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_msie_enable ()
{
	return hsp_trap_mie_set_mask (RISCV_MIE_MSIE);
}

/**
 * Disables software interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_msie_disable ()
{
	return hsp_trap_mie_clr_mask (RISCV_MIE_MSIE);
}

/**
 * Enables timer interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_mtie_enable ()
{
	return hsp_trap_mie_set_mask (RISCV_MIE_MTIE);
}

/**
 * Disables timer interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_mtie_disable ()
{
	return hsp_trap_mie_clr_mask (RISCV_MIE_MTIE);
}

/**
 * Enables external interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_meie_enable ()
{
	return hsp_trap_mie_set_mask (RISCV_MIE_MEIE);
}

/**
 * Disables external interrupts for CLINT mode.
 *
 * @return The value of mie before the mask was applied.
 */
uintptr_t hsp_trap_mie_meie_disable ()
{
	return hsp_trap_mie_clr_mask (RISCV_MIE_MEIE);
}

/**
 * Gets a flag indicating if any MACHINE software interrupts are pending.
 *
 * @return true if there is an interrupt pending, else false.
 */
bool hsp_trap_mip_get_msip ()
{
	return !!(hsp_trap_mip_read () & RISCV_MIP_MSIP);
}

/**
 * Gets a flag indicating if any MACHINE timer interrupts are pending.
 *
 * @return true if there is an interrupt pending, else false.
 */
bool hsp_trap_mip_get_mtip ()
{
	return !!(hsp_trap_mip_read () & RISCV_MIP_MTIP);
}

/**
 * Gets a flag indicating if any MACHINE external interrupts are pending.
 *
 * @return true if there is an interrupt pending, else false.
 */
bool hsp_trap_mip_get_meip ()
{
	return !!(hsp_trap_mip_read () & RISCV_MIP_MTIP);
}

/**
 * Disables all interrupt registers and clears interrupt/exception delegation registers.
 */
void hsp_trap_init_interrupt_ctl_regs ()
{
	hsp_trap_mstatus_clr_mask (RISCV_MSTATUS_INTERRUPT_MASK);
	hsp_trap_mie_clr_mask (RISCV_MIE_INTERRUPT_MASK);

	/* SiFive E20 only supports MACHINE mode interrupts/traps.  Thus, it does not implement
	 * SUPERVISOR or USER mode related registers (m/sedeleg, s/uie, etc).  Implement for other
	 * platforms when needed. */
}

/* Trap processing API */

/**
 * Sets the dedicated stack for the trap handler.
 *
 * @param stack_addr The address of the stack.  If 0, the trap handler will use the stack of the
 * trapped context.  Otherwise, a previous stack must not have been set and must be reset to 0.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_set_isr_stack (uintptr_t stack_addr)
{
	if (stack_addr != 0) {
		if ((stack_addr & RISCV_STACK_ALIGN_MASK) != 0) {
			return IRQ_STACK_NOT_ALIGNED;
		}

		if (hsp_trap_isr_stack != 0) {
			return IRQ_STACK_ALREADY_SET;
		}
	}

	hsp_trap_isr_stack = stack_addr;

	return 0;
}

/**
 * Registers the callback for entering the first trap.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_first_trap_begin_register (hsp_trap_reg_getter action)
{
	return hsp_trap_action_register ((void**) &hsp_trap_first_trap_begin,
		hsp_trap_trap_begin_default, action);
}

/**
 * Sets the default trap entered handler for the first trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_first_trap_begin_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_first_trap_begin,
		hsp_trap_trap_begin_default);
}

/**
 * Registers the callback for saving extra context data for the first trap.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_first_trap_save_register (hsp_trap_action saver)
{
	return hsp_trap_action_register ((void**) &hsp_trap_first_trap_save, hsp_trap_action_default,
		saver);
}

/**
 * Sets the default trap saving handler for the first trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_first_trap_save_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_first_trap_save, hsp_trap_action_default);
}

/**
 * Registers the callback for entering a nested trap.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_nested_trap_begin_register (hsp_trap_reg_getter action)
{
	return hsp_trap_action_register ((void**) &hsp_trap_nested_trap_begin,
		hsp_trap_trap_begin_default, action);
}

/**
 * Sets the default trap entered handler for a nested trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_nested_trap_begin_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_nested_trap_begin,
		hsp_trap_trap_begin_default);
}

/**
 * Registers the callback for saving extra context data for a nested trap.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_nested_trap_save_register (hsp_trap_action saver)
{
	return hsp_trap_action_register ((void**) &hsp_trap_nested_trap_save, hsp_trap_action_default,
		saver);
}

/**
 * Sets the default trap saving handler for a nested trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_nested_trap_save_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_nested_trap_save,
		hsp_trap_action_default);
}

/**
 * Registers the callback for leaving a nested trap.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_nested_trap_leave_register (hsp_trap_action action)
{
	return hsp_trap_action_register ((void**) &hsp_trap_nested_trap_leave, hsp_trap_action_default,
		action);
}

/**
 * Sets the default trap handler for leaving a nested trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_nested_trap_leave_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_nested_trap_leave,
		hsp_trap_action_default);
}

/**
 * Registers the callback for leaving the final trap and returning to the application.
 *
 * @param saver The callback to handle the event.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_final_trap_leave_register (hsp_trap_reg_getter getter)
{
	return hsp_trap_action_register ((void**) &hsp_trap_final_trap_leave,
		hsp_trap_final_trap_leave_default, getter);
}

/**
 * Sets the default trap handler for leaving the final trap.
 *
 * @return 0 if successful, else an error code.
 */
int hsp_trap_final_trap_leave_unregister ()
{
	return hsp_trap_action_unregister ((void**) &hsp_trap_final_trap_leave,
		hsp_trap_final_trap_leave_default);
}

/* Event handling API */

/**
 * Registers a trap routine handler for a RISCV interrupt
 *
 * @param intr_code The target RISCV interrupt to trap
 * @param handler The routine handler to register
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_interrupt_register (unsigned intr_code, hsp_trap_routine handler)
{
	return hsp_trap_vector_register (interrupt_vector, HSP_TRAP_INTERRUPT_CODE_MAX, intr_code,
		handler);
}

/**
 * Replaces the currently registered trap routine to the default handler
 *
 * @param intr_code The target RISCV interrupt
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_interrupt_unregister (unsigned intr_code)
{
	return hsp_trap_vector_unregister (interrupt_vector, HSP_TRAP_INTERRUPT_CODE_MAX, intr_code);
}

/**
 * Registers a trap routine handler for a RISCV exception
 *
 * @param except_code The target RISCV exception to trap
 * @param handler The routine handler to register
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_exception_register (unsigned except_code, hsp_trap_routine handler)
{
	return hsp_trap_vector_register (exception_vector, HSP_TRAP_EXCEPTION_CODE_MAX, except_code,
		handler);
}

/**
 * Replaces the currently registered exception routine to the default handler
 *
 * @param except_code The target RISCV exception
 *
 * @return 0 if successful, else an error code
 */
int hsp_trap_exception_unregister (unsigned except_code)
{
	return hsp_trap_vector_unregister (exception_vector, HSP_TRAP_EXCEPTION_CODE_MAX, except_code);
}

/* Trap handler API */

/**
 * Gets a pointer to the extra data allocated for when the trap handler requested data to be
 * allocated.
 *
 * @param param The first parameter passed by the trap handler.
 * @param stack_size The required stack size of the requested data.  This is expected to be the
 * same value returned from the first_trap_begin handler.
 *
 * @return A pointer to the trap handler context data.
 */
void* hsp_trap_get_context (uintptr_t param, size_t stack_size)
{
	return (void*) (param - stack_size);
}

/**
 * Returns true if currently in a trap, false otherwise.
 *
 * This is useful to implement runtime behavior of calls which have no API to
 * pass in the calling context (such as printf).
 *
 * @return true if in a trap, else false
 */
bool hsp_trap_in_trap ()
{
	return hsp_trap_mscratch_read () != 0;
}

/**
 * Gets a flag that indicates if the trap context is nested.
 *
 * @param param The first parameter passed by the trap handler.
 *
 * @return true if a nested interrupt, else false
 */
bool hsp_trap_is_nested (uintptr_t param)
{
	uintptr_t mscratch = hsp_trap_mscratch_read ();

	if (mscratch == 0) {
		// Not in a trap or called from one of the trap initialization actions
		return false;
	}

	// mscratch points to the first trapped stack
	return (mscratch != param);
}

/**
 * Gets the interrupt flag of the mcause value
 *
 * @param mcause The value returned from the mcause register
 *
 * @return true if an interrupt, else false for an exception
 */
bool hsp_trap_mcause_is_interrupt (uintptr_t mcause)
{
	return !!(mcause >> (__riscv_xlen - 1));
}

/**
 * Gets the interrupt code of the mcause value
 *
 * @param mcause The value returned from the mcause register
 *
 * @return The interrupt code from the mcause value.
 */
unsigned hsp_trap_mcause_code (uintptr_t mcause)
{
	return (unsigned) (mcause & 0x3FF);
}

/**
 * Sets the context to continue execution from the instruction after the one that caused the
 * exception to trap.
 *
 * @param param The parmeter passed by the trap handler.
 *
 * @return true to be returned from the trap handler to indicate the interrupt has been handled.
 */
bool hsp_trap_exception_continue (uintptr_t param)
{
	hsp_trap_mepc_increment (param);

	return true;
}

/**
 * Sets the context to continue execution from the instruction after the ECALL.
 *
 * @param param The parmeter passed by the trap handler.
 *
 * @return true to be returned from the trap handler to indicate the interrupt has been handled.
 */
bool hsp_trap_syscall_continue (uintptr_t param)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;

	// ECALL does not compress, no need to do the instruction size calculation.
	ctx->trap_mepc += RISCV_ECALL_SIZE;

	return true;
}

/**
 * Prints the initial trap registers of the context.
 *
 * @param ctx The trap context from the trap handler.
 */
void hsp_trap_print_entry (struct hsp_trap_context *ctx)
{
	platform_printf ("trapped mstatus=0x%x mepc=0x%x mcause=0x%x" NEWLINE, ctx->trap_mstatus,
		ctx->trap_mepc, ctx->trap_mcause);
}

/**
 * Prints the current trap registers of the context.
 *
 * @param ctx The trap context from the trap handler.
 */
void hsp_trap_print_trap (struct hsp_trap_context *ctx)
{
	platform_printf ("mstatus=0x%x mepc=0x%x" NEWLINE, ctx->mstatus, ctx->mepc);
	platform_printf ("mcause=0x%x mtval=0x%x" NEWLINE, ctx->mcause, ctx->mtval);
	platform_printf ("seq_intr=0x%x nested=%d" NEWLINE, ctx->seq_intr,
		(int) hsp_trap_is_nested ((uintptr_t) ctx));
}

/**
 * Prints general purpose registers of the context.
 *
 * @param ctx The trap context from the trap handler.
 */
void hsp_trap_print_regs (struct hsp_trap_context *ctx)
{
	platform_printf ("ra/x1=0x%x sp/x2=0x%x gp/x3=0x%x tp/x4=0x%x" NEWLINE, ctx->regs.ra,
		ctx->regs.sp, ctx->regs.gp, ctx->regs.tp);
	platform_printf ("t0/x5=0x%x t1/x6=0x%x t2/x7=0x%x s0/x8/fp=0x%x" NEWLINE, ctx->regs.t0,
		ctx->regs.t1, ctx->regs.t2, ctx->regs.s0);
	platform_printf ("s1/x9=0x%x a0/x10=0x%x a1/x11=0x%x a2/x12=0x%x" NEWLINE, ctx->regs.s1,
		ctx->regs.a0, ctx->regs.a1, ctx->regs.a2);
	platform_printf ("a3/x13=0x%x a4/x14=0x%x a5/x15=0x%x a6/x16=0x%x" NEWLINE, ctx->regs.a3,
		ctx->regs.a4, ctx->regs.a5, ctx->regs.a6);
	platform_printf ("a7/x17=0x%x s2/x18=0x%x s3/x19=0x%x s4/x20=0x%x" NEWLINE, ctx->regs.a7,
		ctx->regs.s2, ctx->regs.s3, ctx->regs.s4);
	platform_printf ("s5/x21=0x%x s6/x22=0x%x s7/x23=0x%x s8/x24=0x%x" NEWLINE, ctx->regs.s5,
		ctx->regs.s6, ctx->regs.s7, ctx->regs.s8);
	platform_printf ("s9/x25=0x%x s10/x26=0x%x s11/x27=0x%x t3/x28=0x%x" NEWLINE, ctx->regs.s9,
		ctx->regs.s10, ctx->regs.s11, ctx->regs.t3);
	platform_printf ("t4/x29=0x%x t5/x30=0x%x t6/x31=0x%x" NEWLINE, ctx->regs.t4, ctx->regs.t5,
		ctx->regs.t6);
}
