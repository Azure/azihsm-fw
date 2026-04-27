// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_top.h"
#include "platform_config.h"
#include "common/unused.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_interrupt_vector.h"
#include "trap/hsp_trap.h"


/* TODO: add functionaly to error on registration of reserved/unused IRQ/FIQ bits. */

/**
 * Number of interrupts in HSP_INTSTS
 */
#define HSP_INTERRUPT_IRQ_COUNT		32

/**
 * Declares a variable declaration for an HSP interrupt register
 *
 * @param reg The specific register name
 */
#define HSP_CREG_INT_REG(reg)       \
	((volatile uint32_t*) HSP_ADDR_MAP_CREG_INT_REGS_CREG_INT_GROUP_HSP_ ## reg ## _ADDRESS)


/**
 * Global HSP interrupt vector for IRQ's.
 */
static const struct hsp_interrupt_handler *irq_vector[HSP_INTERRUPT_IRQ_COUNT];


/**
 * Clears an interrupt from the HSP_INTSTS register.
 *
 * @param intr_mask A 1-hot bitmask for a specific signal that is written to HSP_INTSTS to clear.
 */
static void hsp_interrupt_intr_clr (uint32_t intr_mask)
{
	*HSP_CREG_INT_REG (INTSTS) = intr_mask;
}

/**
 * Gets the desired interrupt enable register.
 *
 * @param irql The HSP IRQ level to get the INTEN register for.
 *
 * @return NULL if an invalid IRQ level, else a pointer to the INTEN register.
 */
static volatile uint32_t* hsp_interrupt_inten_reg (enum hsp_interrupt_irq_level irql)
{
	switch (irql) {
		case HSP_INTERRUPT_IRQ_LEVEL_IRQ:
			return HSP_CREG_INT_REG (IRQINTEN);

		case HSP_INTERRUPT_IRQ_LEVEL_FIQ:
			return HSP_CREG_INT_REG (FIQINTEN);

		default:
			return NULL;
	}
}

/**
 * Creates a bitmask for a valid HSP_INTSTS interrupt bit.
 *
 * @param intr_bit The interrupt bit index.
 *
 * @return 0 if bit index is invalid, else a non-zero bitmask.
 */
static uint32_t hsp_interrupt_intr_mask (unsigned intr_bit)
{
	if (intr_bit >= HSP_INTERRUPT_IRQ_COUNT) {
		return 0;
	}

	return (uint32_t) (1u << intr_bit);
}

/**
 * Reads the HSP_INTSTS register and masks the desired bits.
 *
 * @param mask The bit mask to apply to the register.
 *
 * @return The masked value of HSP_INTSTS.
 */
static uint32_t hsp_interrupt_read_intsts_masked (uint32_t mask)
{
	return *HSP_CREG_INT_REG (INTSTS) & mask;
}

/**
 * Reads the HSP_INTSTS register and masks the bits according to the INTEN register.
 *
 * @param inten_reg The INTEN register.
 *
 * @return The masked value of HSP_INTSTS.
 */
static uint32_t hsp_interrupt_read_intsts_for_irql (volatile uint32_t *inten_reg)
{
	return hsp_interrupt_read_intsts_masked (*inten_reg);
}

/**
 * Gets a bitmask for the target HSP interrupt and the interrupt enable register for the IRQL.
 *
 * @param inten_reg Pointer to store the interrupt enable register.
 * @param intr_bit The interrupt bit index.
 * @param irql The interrupt request level.
 *
 * @return 0 if bit index or IRQL are invalid, else a non-zero bitmask.
 */
static uint32_t hsp_interrupt_get_inten_info (volatile uint32_t **inten_reg, unsigned intr_bit,
	enum hsp_interrupt_irq_level irql)
{
	volatile uint32_t *inten_val = NULL;
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_intr_mask (intr_bit);
	if (intr_mask != 0) {
		inten_val = hsp_interrupt_inten_reg (irql);
	}

	if (inten_val == NULL) {
		intr_mask = 0;
	}

	*inten_reg = inten_val;

	return intr_mask;
}

/**
 * HSP peripherals trigger interrupts to a single IRQ and/or FIQ RISCV line.  When the interrupt is
 * raised and calls this ISR, it will query the HSP interrupt group status register and notify the
 * appropriate handlers.
 *
 * @param param The parameter passed by the trap hander.
 * @param irql The HSP IRQ level that triggered this trap.
 *
 * @return true if all HSP interrupts were handled, else false
 */
static bool hsp_interrupt_query_and_dispatch (uintptr_t param, enum hsp_interrupt_irq_level irql)
{
	unsigned unhandled = 0;
	uint32_t handled = 0;
	uint32_t intr_mask = 1;
	uint32_t intsts;
	volatile uint32_t *inten_reg;
	const struct hsp_interrupt_handler **entry;

	inten_reg = hsp_interrupt_inten_reg (irql);
	if (inten_reg == NULL) {
		return false;
	}

	entry = irq_vector;

	// Process the interrupts for the current level
	intsts = hsp_interrupt_read_intsts_for_irql (inten_reg);
	while (intsts != 0) {
		if (intsts & 1) {
			if (hsp_interrupt_vector_call_entry (entry, param)) {
				handled |= intr_mask;
			}
			else {
				++unhandled;
			}
		}

		intsts >>= 1;
		intr_mask <<= 1;
		++entry;
	}

	*HSP_CREG_INT_REG (INTSTS) = handled;

	return (unhandled == 0);
}

/* Trap handlers */

/**
 * Trap handler to handle IRQ interrupts.
 *
 * @param param The parameter passed by the trap handler.
 *
 * @return true if all HSP IRQ interrupts were handled, else false
 */
static bool hsp_interrupt_irq_trap_handler (uintptr_t param)
{
	return hsp_interrupt_query_and_dispatch (param, HSP_INTERRUPT_IRQ_LEVEL_IRQ);
}

/**
 * Trap handler to handle FIQ interrupts.
 *
 * @param param The parameter passed by the trap handler.
 *
 * @return true if all HSP FIQ interrupts were handled, else false
 */
static bool hsp_interrupt_fiq_trap_handler (uintptr_t param)
{
	return hsp_interrupt_query_and_dispatch (param, HSP_INTERRUPT_IRQ_LEVEL_FIQ);
}

/* Task API */

/**
 * Initializes the HSP interrupt vector state. hsp_trap_init () must have been called with CLIC
 * mode prior to initialization.
 *
 * @param clear_outstanding Flag to clear any outstanding interrupts from previous boot stages.
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_init (bool clear_outstanding)
{
	int status;

	// TODO: Ensure SiFive's CLIC implementation is level triggered.

	if (!hsp_trap_is_clic_mode ()) {
		// HSP interrupts are hard-wired to CLIC mode interrupts.
		return IRQ_MODE_UNSUPPORTED;
	}

	// Disable global interrupts
	hsp_trap_init_interrupt_ctl_regs ();

	status = hsp_trap_interrupt_register (HSP_TRAP_INTERRUPT_CODE_IRQ,
		hsp_interrupt_irq_trap_handler);
	if (status != 0) {
		return status;
	}

	status = hsp_trap_interrupt_register (HSP_TRAP_INTERRUPT_CODE_FIQ,
		hsp_interrupt_fiq_trap_handler);
	if (status != 0) {
		goto unreg_irq;
	}

	status = hsp_trap_clic_interrupt_enable (HSP_TRAP_INTERRUPT_CODE_IRQ,
		HSP_TRAP_CLIC_LEVEL_DEFAULT_IRQ);
	if (status != 0) {
		goto unreg_fiq;
	}

	status = hsp_trap_clic_interrupt_enable (HSP_TRAP_INTERRUPT_CODE_FIQ,
		HSP_TRAP_CLIC_LEVEL_DEFAULT_FIQ);
	if (status != 0) {
		goto dis_irq;
	}

	if (clear_outstanding) {
		hsp_interrupt_intr_clr (*HSP_CREG_INT_REG (INTSTS));
	}

	return 0;

dis_irq:
	hsp_trap_clic_interrupt_disable (HSP_TRAP_INTERRUPT_CODE_IRQ);

unreg_fiq:
	hsp_trap_interrupt_unregister (HSP_TRAP_INTERRUPT_CODE_FIQ);

unreg_irq:
	hsp_trap_interrupt_unregister (HSP_TRAP_INTERRUPT_CODE_IRQ);

	return status;
}

/**
 * Registers a handler for a specific HSP interrupt
 *
 * @param intr_bit The bit index of HSP_INTSTS to register a handler for
 * @param handler A handler for the interrupt
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_register (unsigned intr_bit, const struct hsp_interrupt_handler *handler)
{
	return hsp_interrupt_vector_register (irq_vector, HSP_INTERRUPT_IRQ_COUNT, intr_bit, handler);
}

/**
 * Unregisters a handler for a specific HSP interrupt
 *
 * @param intr_bit The bit index of HSP_INTSTS to unregister a handler
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_unregister (unsigned intr_bit)
{
	return hsp_interrupt_vector_unregister (irq_vector, HSP_INTERRUPT_IRQ_COUNT, intr_bit);
}

/* CPU Config API */

/**
 * Get a flag that indicates if an interrupt is enabled for a given IRQL.
 *
 * @param intr_bit The bit index of HSP_INTSTS.
 * @param irql The HSP IRQ level.
 *
 * @return true if the interrupt is enabled, else false.
 */
bool hsp_interrupt_is_enabled (unsigned intr_bit, enum hsp_interrupt_irq_level irql)
{
	volatile uint32_t *inten_reg;
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_get_inten_info (&inten_reg, intr_bit, irql);
	if (intr_mask == 0) {
		return false;
	}

	return !!(*inten_reg & intr_mask);
}

/**
 * Gets a flag that indicates if a specific HSP interrupt is pending.
 *
 * @param intr_bit The bit index of HSP_INTSTS.
 *
 * @return true if the interrupt is pending, else false.
 */
bool hsp_interrupt_is_pending (unsigned intr_bit)
{
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_intr_mask (intr_bit);
	if (intr_mask == 0) {
		return false;
	}

	return !!hsp_interrupt_read_intsts_masked (intr_mask);
}

/**
 * Gets a flag that indicates if a specific HSP interrupt is pending for an IRQ level.
 *
 * @param intr_bit The bit index of HSP_INTSTS.
 * @param irql The HSP IRQ level.
 *
 * @return true if the interrupt is pending, else false.
 */
bool hsp_interrupt_is_pending_for_irql (unsigned intr_bit, enum hsp_interrupt_irq_level irql)
{
	volatile uint32_t *inten_reg;
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_get_inten_info (&inten_reg, intr_bit, irql);
	if (intr_mask == 0) {
		return false;
	}

	return !!(hsp_interrupt_read_intsts_for_irql (inten_reg) & intr_mask);
}

/**
 * Enables a specific HSP interrupt.
 *
 * @param intr_bit The bit index of HSP_INTSTS to enable.
 * @param irql The HSP IRQ level to enable.
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_enable (unsigned intr_bit, enum hsp_interrupt_irq_level irql)
{
	volatile uint32_t *inten_reg;
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_get_inten_info (&inten_reg, intr_bit, irql);
	if (intr_mask == 0) {
		return IRQ_INVALID_ARGUMENT;
	}

	*inten_reg |= intr_mask;

	return 0;
}

/**
 * Disables a specific HSP interrupt.
 *
 * @param intr_bit The bit index of HSP_INTSTS to disable.
 * @param irql The HSP IRQ level to disable.
 *
 * @return 0 if successful, else an error code
 */
int hsp_interrupt_disable (unsigned intr_bit, enum hsp_interrupt_irq_level irql)
{
	volatile uint32_t *inten_reg;
	uint32_t intr_mask;

	intr_mask = hsp_interrupt_get_inten_info (&inten_reg, intr_bit, irql);
	if (intr_mask == 0) {
		return IRQ_INVALID_ARGUMENT;
	}

	*inten_reg &= ~intr_mask;

	return 0;
}

/* ISR API */

/**
 * Gets the IRQ level for a trap context.
 *
 * @param param The parameter passed by the trap handler.
 *
 * @return -1 if not an HSP IRQ, else the IRQ level
 */
int hsp_interrupt_get_irql (uintptr_t param)
{
	struct hsp_trap_context *ctx = (struct hsp_trap_context*) param;

	switch (hsp_trap_mcause_code (ctx->mcause)) {
		case HSP_TRAP_INTERRUPT_CODE_IRQ:
			return HSP_INTERRUPT_IRQ_LEVEL_IRQ;

		case HSP_TRAP_INTERRUPT_CODE_FIQ:
			return HSP_INTERRUPT_IRQ_LEVEL_FIQ;

		default:
			return -1;
	}
}
