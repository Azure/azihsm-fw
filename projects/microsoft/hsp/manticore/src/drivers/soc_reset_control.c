// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "drivers/soc_reset_control.h"


/**
 * Base address for the POR registers.
 */
#define	SOC_RESET_CONTROL_POR_REGISTERS				0xb0003000

/**
 * Address for the reset control register for SoC hardware.
 */
#define	SOC_RESET_CONTROL_RESET_CTRL_REGISTER		0xb0004000

/**
 * Reset control bits for executing soft reset of cores.
 */
enum {
	SOC_RESET_CONTROL_RESET_CTRL_CP0 = (1U << 19),	/**< Soft reset control for CP core 0. */
	SOC_RESET_CONTROL_RESET_CTRL_CP1 = (1U << 20),	/**< Soft reset control for CP core 1. */
	SOC_RESET_CONTROL_RESET_CTRL_FP0 = (1U << 22),	/**< Soft reset control for FP core 0. */
	SOC_RESET_CONTROL_RESET_CTRL_FP1 = (1U << 23),	/**< Soft reset control for FP core 1. */
	SOC_RESET_CONTROL_RESET_CTRL_FP2 = (1U << 24),	/**< Soft reset control for FP core 2. */
};

/**
 * Address for the run/stall control register for the FP hardware.
 */
#define	SOC_RESET_CONTROL_FP_RUNSTALL_REGISTER		0xb0004004

/**
 * Control bits for stalling the FP cores.
 */
enum {
	SOC_RESET_CONTROL_FP_RUNSTALL_FP0_WAIT = (1U << 0),	/**< Stall FP core 0. */
	SOC_RESET_CONTROL_FP_RUNSTALL_FP1_WAIT = (1U << 1),	/**< Stall FP core 1. */
	SOC_RESET_CONTROL_FP_RUNSTALL_FP2_WAIT = (1U << 2),	/**< Stall FP core 2. */
};

/**
 * Address for the run/stall control register for the CP processors.
 */
#define	SOC_RESET_CONTROL_CP_RUNSTALL_REGISTER		0xb0003008

/**
 * Control bits for stalling the CP cores.
 */
enum {
	SOC_RESET_CONTROL_CP_RUNSTALL_CP0_WAIT = (1U << 0),	/**< Stall CP core 0. */
	SOC_RESET_CONTROL_CP_RUNSTALL_CP1_WAIT = (1U << 1),	/**< Stall CP core 1. */
};

/**
 * Address for the SoC GPIO register where status of boot strapping pins is stored.
 */
#define	SOC_RESET_CONTROL_GPIO_LATCHED_REGISTER		0xb0007104

/**
 * Latched values of the boot strapping pins on the SoC.
 */
enum {
	SOC_RESET_CONTROL_GPIO_STRAP_STRAP0 = (1U << 0),			/**< Value of Strap 0 pin. */
	SOC_RESET_CONTROL_GPIO_STRAP_STRAP1 = (1U << 1),			/**< Value of Strap 1 pin. */
	SOC_RESET_CONTROL_GPIO_STRAP_A0_BYPASS = (1U << 2),			/**< Value of A0 Bypass pin. */
	SOC_RESET_CONTROL_GPIO_STRAP_FORCE_RECOVERY = (1U << 3),	/**< Value of Force Recovery pin. */
	SOC_RESET_CONTROL_GPIO_STRAP_FLASH_PRIORITY = (1U << 4),	/**< Value of Flash Priority pin. */
};

/**
 * Reset control for the CP and FP CPUs and latched strapping values.
 */
struct soc_reset_control_por_regs {
	uint8_t pad0[8];						/**< Unused. */
	volatile uint32_t cp_run_stall;			/**< Run/stall control register for CP. */
	uint8_t pad1[4084];						/**< Unused. */
	volatile uint32_t reset_control;		/**< SoC reset control. */
	volatile uint32_t fp_run_stall;			/**< Run/stall control register for FP. */
	uint8_t pad2[0x30fc];					/**< Unused. */
	volatile uint32_t gpio_latched_input;	/**< Latched input for strapping pins. */
};


_Static_assert ((offsetof (struct soc_reset_control_por_regs, cp_run_stall) == 0x0008),
	"CP run/stall offset wrong.");
_Static_assert ((offsetof (struct soc_reset_control_por_regs, reset_control) == 0x1000),
	"Reset Control offset wrong.");
_Static_assert ((offsetof (struct soc_reset_control_por_regs, fp_run_stall) == 0x1004),
	"FP run/stall offset wrong.");
_Static_assert ((offsetof (struct soc_reset_control_por_regs, gpio_latched_input) == 0x4104),
	"Strapping pin latched input offset wrong.");


int soc_reset_control_map_por_registers (const struct soc_reset_control *reset_ctrl,
	void **por_regs)
{
	int status;

	if ((reset_ctrl == NULL) || (por_regs == NULL)) {
		return SOC_RESET_CONTROL_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&reset_ctrl->state->lock);

	status = reset_ctrl->dmb->map_soc_address (reset_ctrl->dmb, SOC_RESET_CONTROL_POR_REGISTERS,
		sizeof (struct soc_reset_control_por_regs), HSP_DMB_ACCESS_WRITE, por_regs);
	if (status != 0) {
		platform_mutex_unlock (&reset_ctrl->state->lock);
	}

	return status;
}

void soc_reset_control_unmap_por_registers (const struct soc_reset_control *reset_ctrl,
	void *por_regs)
{
	if (reset_ctrl == NULL) {
		return;
	}

	reset_ctrl->dmb->unmap_soc_address (reset_ctrl->dmb, por_regs);

	platform_mutex_unlock (&reset_ctrl->state->lock);
}

/**
 * Get the register mask to use for a CP CPU reset operation.
 *
 * @param core The CP core(s) targeted for the operation.
 *
 * @return The reset register mask for the core(s).
 */
static uint32_t soc_reset_control_get_cp_reset_mask (enum soc_reset_control_cp_core core)
{
	switch (core) {
		case SOC_RESET_CONTROL_CP_CORE_ADMIN:
			return SOC_RESET_CONTROL_RESET_CTRL_CP0;

		case SOC_RESET_CONTROL_CP_CORE_HSM:
			return SOC_RESET_CONTROL_RESET_CTRL_CP1;

		case SOC_RESET_CONTROL_CP_CORE_BOTH:
			return SOC_RESET_CONTROL_RESET_CTRL_CP0 | SOC_RESET_CONTROL_RESET_CTRL_CP1;

		default:
			return 0;
	}
}

void soc_reset_control_reset_cp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset, enum soc_reset_control_cp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return;
	}

	mask = soc_reset_control_get_cp_reset_mask (core);

	if (reset) {
		regs->reset_control |= mask;
	}
	else {
		regs->reset_control &= ~mask;
	}
}

bool soc_reset_control_is_cp_in_reset (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_cp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return false;
	}

	mask = soc_reset_control_get_cp_reset_mask (core);

	return !!(regs->reset_control & mask);
}

/**
 * Get the register mask to use for a CP CPU stall operation.
 *
 * @param core The CP core(s) targeted for the operation.
 *
 * @return The run/stall register mask for the core(s).
 */
static uint32_t soc_reset_control_get_cp_stall_mask (enum soc_reset_control_cp_core core)
{
	switch (core) {
		case SOC_RESET_CONTROL_CP_CORE_ADMIN:
			return SOC_RESET_CONTROL_CP_RUNSTALL_CP0_WAIT;

		case SOC_RESET_CONTROL_CP_CORE_HSM:
			return SOC_RESET_CONTROL_CP_RUNSTALL_CP1_WAIT;

		case SOC_RESET_CONTROL_CP_CORE_BOTH:
			return SOC_RESET_CONTROL_CP_RUNSTALL_CP0_WAIT | SOC_RESET_CONTROL_CP_RUNSTALL_CP1_WAIT;

		default:
			return 0;
	}
}

void soc_reset_control_stall_cp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall, enum soc_reset_control_cp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return;
	}

	mask = soc_reset_control_get_cp_stall_mask (core);

	if (stall) {
		regs->cp_run_stall |= mask;
	}
	else {
		regs->cp_run_stall &= ~mask;
	}
}

bool soc_reset_control_is_cp_stalled (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_cp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return false;
	}

	mask = soc_reset_control_get_cp_stall_mask (core);

	return !!(regs->cp_run_stall & mask);
}

/**
 * Get the register mask to use for a FP CPU reset operation.
 *
 * @param core The FP core(s) targeted for the operation.
 *
 * @return The reset register mask for the core(s).
 */
static uint32_t soc_reset_control_get_fp_reset_mask (enum soc_reset_control_fp_core core)
{
	switch (core) {
		case SOC_RESET_CONTROL_FP_CORE_0:
			return SOC_RESET_CONTROL_RESET_CTRL_FP0;

		case SOC_RESET_CONTROL_FP_CORE_1:
			return SOC_RESET_CONTROL_RESET_CTRL_FP1;

		case SOC_RESET_CONTROL_FP_CORE_2:
			return SOC_RESET_CONTROL_RESET_CTRL_FP2;

		case SOC_RESET_CONTROL_FP_CORE_ALL:
			return SOC_RESET_CONTROL_RESET_CTRL_FP0 | SOC_RESET_CONTROL_RESET_CTRL_FP1 |
				   SOC_RESET_CONTROL_RESET_CTRL_FP2;

		default:
			return 0;
	}
}

void soc_reset_control_reset_fp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset, enum soc_reset_control_fp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return;
	}

	mask = soc_reset_control_get_fp_reset_mask (core);

	if (reset) {
		regs->reset_control |= mask;
	}
	else {
		regs->reset_control &= ~mask;
	}
}

bool soc_reset_control_is_fp_in_reset (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_fp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return false;
	}

	mask = soc_reset_control_get_fp_reset_mask (core);

	return !!(regs->reset_control & mask);
}

/**
 * Get the register mask to use for a FP CPU stall operation.
 *
 * @param core The FP core(s) targeted for the operation.
 *
 * @return The run/stall register mask for the core(s).
 */
static uint32_t soc_reset_control_get_fp_stall_mask (enum soc_reset_control_fp_core core)
{
	switch (core) {
		case SOC_RESET_CONTROL_FP_CORE_0:
			return SOC_RESET_CONTROL_FP_RUNSTALL_FP0_WAIT;

		case SOC_RESET_CONTROL_FP_CORE_1:
			return SOC_RESET_CONTROL_FP_RUNSTALL_FP1_WAIT;

		case SOC_RESET_CONTROL_FP_CORE_2:
			return SOC_RESET_CONTROL_FP_RUNSTALL_FP2_WAIT;

		case SOC_RESET_CONTROL_FP_CORE_ALL:
			return SOC_RESET_CONTROL_FP_RUNSTALL_FP0_WAIT | SOC_RESET_CONTROL_FP_RUNSTALL_FP1_WAIT |
				   SOC_RESET_CONTROL_FP_RUNSTALL_FP2_WAIT;

		default:
			return 0;
	}
}

void soc_reset_control_stall_fp_core (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall, enum soc_reset_control_fp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return;
	}

	mask = soc_reset_control_get_fp_stall_mask (core);

	if (stall) {
		regs->fp_run_stall |= mask;
	}
	else {
		regs->fp_run_stall &= ~mask;
	}
}

bool soc_reset_control_is_fp_stalled (const struct soc_reset_control *reset_ctrl, void *por_regs,
	enum soc_reset_control_fp_core core)
{
	struct soc_reset_control_por_regs *regs = por_regs;
	uint32_t mask;

	if ((reset_ctrl == NULL) || (regs == NULL)) {
		return false;
	}

	mask = soc_reset_control_get_fp_stall_mask (core);

	return !!(regs->fp_run_stall & mask);
}

void soc_reset_control_reset_all_cores (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool reset)
{
	soc_reset_control_reset_cp_core (reset_ctrl, por_regs, reset, SOC_RESET_CONTROL_CP_CORE_BOTH);
	soc_reset_control_reset_fp_core (reset_ctrl, por_regs, reset, SOC_RESET_CONTROL_FP_CORE_ALL);
}

void soc_reset_control_stall_all_cores (const struct soc_reset_control *reset_ctrl, void *por_regs,
	bool stall)
{
	soc_reset_control_stall_cp_core (reset_ctrl, por_regs, stall, SOC_RESET_CONTROL_CP_CORE_BOTH);
	soc_reset_control_stall_fp_core (reset_ctrl, por_regs, stall, SOC_RESET_CONTROL_FP_CORE_ALL);
}

/**
 * Initialize a driver for controlling reset of Manticore SoC hardware components.
 *
 * @param control The driver to initialize.
 * @param state Variable context for the driver.  This must be uninitialized.
 * @param dmb THe HSP DMB driver for mapping SoC addresses.
 *
 * @return 0 if the driver was initialized successfully or an error code.
 */
int soc_reset_control_init (struct soc_reset_control *reset_ctrl,
	struct soc_reset_control_state *state, const struct hsp_dmb *dmb)
{
	if (reset_ctrl == NULL) {
		return SOC_RESET_CONTROL_INVALID_ARGUMENT;
	}

	memset (reset_ctrl, 0, sizeof (struct soc_reset_control));

	reset_ctrl->map_por_registers = soc_reset_control_map_por_registers;
	reset_ctrl->unmap_por_registers = soc_reset_control_unmap_por_registers;
	reset_ctrl->reset_all_cores = soc_reset_control_reset_all_cores;
	reset_ctrl->stall_all_cores = soc_reset_control_stall_all_cores;
	reset_ctrl->reset_cp_core = soc_reset_control_reset_cp_core;
	reset_ctrl->is_cp_in_reset = soc_reset_control_is_cp_in_reset;
	reset_ctrl->stall_cp_core = soc_reset_control_stall_cp_core;
	reset_ctrl->is_cp_stalled = soc_reset_control_is_cp_stalled;
	reset_ctrl->reset_fp_core = soc_reset_control_reset_fp_core;
	reset_ctrl->is_fp_in_reset = soc_reset_control_is_fp_in_reset;
	reset_ctrl->stall_fp_core = soc_reset_control_stall_fp_core;
	reset_ctrl->is_fp_stalled = soc_reset_control_is_fp_stalled;

	reset_ctrl->state = state;
	reset_ctrl->dmb = dmb;

	return soc_reset_control_init_state (reset_ctrl);
}

/**
 * Initialize only the variable state for a SoC reset control driver.  The rest of the driver is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param reset_ctrl The driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int soc_reset_control_init_state (const struct soc_reset_control *reset_ctrl)
{
	if ((reset_ctrl == NULL) || (reset_ctrl->state == NULL) || (reset_ctrl->dmb == NULL)) {
		return SOC_RESET_CONTROL_INVALID_ARGUMENT;
	}

	memset (reset_ctrl->state, 0, sizeof (struct soc_reset_control_state));

	return platform_mutex_init (&reset_ctrl->state->lock);
}

/**
 * Release the resources used by a SoC hardware reset control driver.
 *
 * @param control The driver to release.
 */
void soc_reset_control_release (const struct soc_reset_control *reset_ctrl)
{
	if (reset_ctrl) {
		platform_mutex_free (&reset_ctrl->state->lock);
	}
}
