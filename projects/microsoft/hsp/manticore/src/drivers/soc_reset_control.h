// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_RESET_CONTROL_H_
#define SOC_RESET_CONTROL_H_

// #include <stdint.h>
// #include <stddef.h>
#include <stdbool.h>
#include "platform_api.h"
#include "drivers/hsp_dmb.h"
#include "status/manticore_module_id.h"


/**
 * Options for CP CPU core control.
 */
enum soc_reset_control_cp_core {
	SOC_RESET_CONTROL_CP_CORE_NONE = 0,		/**< Target none of the CP CPU cores. */
	SOC_RESET_CONTROL_CP_CORE_ADMIN = 1,	/**< Target only the Admin core, which is CP core 0. */
	SOC_RESET_CONTROL_CP_CORE_HSM = 2,		/**< Target only the HSM core, which is CP core 1. */
	SOC_RESET_CONTROL_CP_CORE_BOTH = 3,		/**< Target both CP CPU cores. */
};

/**
 * Options for FP CPU core control.
 */
enum soc_reset_control_fp_core {
	SOC_RESET_CONTROL_FP_CORE_NONE = 0,	/**< Target none of the FP CPU cores. */
	SOC_RESET_CONTROL_FP_CORE_0 = 1,	/**< Target only FP core 0. */
	SOC_RESET_CONTROL_FP_CORE_1 = 2,	/**< Target only FP core 1. */
	SOC_RESET_CONTROL_FP_CORE_2 = 4,	/**< Target only FP core 2. */
	SOC_RESET_CONTROL_FP_CORE_ALL = 7,	/**< Target all FP CPU cores. */
};


/**
 * Variable context for the SoC reset control driver.
 */
struct soc_reset_control_state {
	platform_mutex lock;	/**< Synchronization for hardware register accesses. */
};

/**
 * Interface to control reset for SoC hardware blocks.
 */
struct soc_reset_control {
	/**
	 * Map the SoC POR register block into the HSP address space.  This will need to be called
	 * before using any of the other APIs.
	 *
	 * Attempts to map the registers again will be blocked until the previous mapping has been
	 * unmapped.
	 *
	 * @param reset_ctrl The reset control driver that will access the registers.
	 * @param por_regs Output for the mapped register address.
	 *
	 * @return 0 if the registers were mapped successfully or an error code.
	 */
	int (*map_por_registers) (const struct soc_reset_control *reset_ctrl, void **por_regs);

	/**
	 * Remove the SoC POR register block address mapping from HSP address space.  This must be
	 * called when finished accessing the POR registers.
	 *
	 * @param reset_ctrl The reset control driver accessing the registers.
	 * @param por_regs The mapped register address to unmap.
	 */
	void (*unmap_por_registers) (const struct soc_reset_control *reset_ctrl, void *por_regs);

	/**
	 * Update the reset control bit for all CP and FP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param reset true to assert reset or false to deassert.
	 */
	void (*reset_all_cores) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		bool reset);

	/**
	 * Update the run/stall control bit for all CP and FP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param stall true to stall the CPUs or false to allow them to run.
	 */
	void (*stall_all_cores) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		bool stall);

	/**
	 * Update the reset control bit for CP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param reset true to assert reset or false to deassert.
	 * @param core The CPU core(s) to update.
	 */
	void (*reset_cp_core) (const struct soc_reset_control *reset_ctrl, void *por_regs, bool reset,
		enum soc_reset_control_cp_core core);

	/**
	 * Determine if CP CPU cores are currently in reset.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param core The CPU core(s) to check.
	 *
	 * @return true if the CPU is in reset or false if not.  If multiple CPU cores are selected,
	 * this will be true if any of the CPUs are in reset, even if not all of them are.
	 */
	bool (*is_cp_in_reset) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		enum soc_reset_control_cp_core core);

	/**
	 * Update the run/stall control bit for CP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param stall true to stall the CPUs or false to allow them to run.
	 * @param core The CPU core(s) to update.
	 */
	void (*stall_cp_core) (const struct soc_reset_control *reset_ctrl, void *por_regs, bool stall,
		enum soc_reset_control_cp_core core);

	/**
	 * Determine if CP CPU cores are currently stalled.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param core The CPU core(s) to check.
	 *
	 * @return true if the CPU is stalled or false if not.  If multiple CPU cores are selected, this
	 * will be true if any of the CPUs are stalled, even if not all of them are.
	 */
	bool (*is_cp_stalled) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		enum soc_reset_control_cp_core core);

	/**
	 * Update the reset control bit for FP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param reset true to assert reset or false to deassert.
	 * @param core The CPU core(s) to update.
	 */
	void (*reset_fp_core) (const struct soc_reset_control *reset_ctrl, void *por_regs, bool reset,
		enum soc_reset_control_fp_core core);

	/**
	 * Determine if FP CPU cores are currently in reset.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param core The CPU core(s) to check.
	 *
	 * @return true if the CPU is in reset or false if not.  If multiple CPU cores are selected,
	 * this will be true if any of the CPUs are in reset, even if not all of them are.
	 */
	bool (*is_fp_in_reset) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		enum soc_reset_control_fp_core core);

	/**
	 * Update the run/stall control bit for FP CPU cores.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param stall true to stall the CPUs or false to allow them to run.
	 * @param core The CPU core(s) to update.
	 */
	void (*stall_fp_core) (const struct soc_reset_control *reset_ctrl, void *por_regs, bool stall,
		enum soc_reset_control_fp_core core);

	/**
	 * Determine if FP CPU cores are currently stalled.
	 *
	 * @param reset_ctrl Reset control driver for SoC hardware.
	 * @param por_regs Access to the SoC POR register block, mapped with
	 * {@link soc_reset_control.map_por_registers}.
	 * @param core The CPU core(s) to check.
	 *
	 * @return true if the CPU is stalled or false if not.  If multiple CPU cores are selected, this
	 * will be true if any of the CPUs are stalled, even if not all of them are.
	 */
	bool (*is_fp_stalled) (const struct soc_reset_control *reset_ctrl, void *por_regs,
		enum soc_reset_control_fp_core core);

	struct soc_reset_control_state *state;	/**< Variable context for the reset control driver. */
	const struct hsp_dmb *dmb;				/**< DMB driver for accessing SoC registers. */
};


int soc_reset_control_init (struct soc_reset_control *reset_ctrl,
	struct soc_reset_control_state *state, const struct hsp_dmb *dmb);
int soc_reset_control_init_state (const struct soc_reset_control *reset_ctrl);
void soc_reset_control_release (const struct soc_reset_control *reset_ctrl);


#define	SOC_RESET_CONTROL_ERROR(code)			ROT_ERROR (MANTICORE_MODULE_SOC_RESET_CONTROL, code)

/**
 * Error codes that can be generated by the SoC hardware reset controller.
 */
enum {
	SOC_RESET_CONTROL_INVALID_ARGUMENT = SOC_RESET_CONTROL_ERROR (0x00),	/**< Input parameter is null or not valid. */
	SOC_RESET_CONTROL_NO_MEMORY = SOC_RESET_CONTROL_ERROR (0x01),			/**< Memory allocation failed. */
	SOC_RESET_CONTROL_MAP_REGS_FAILED = SOC_RESET_CONTROL_ERROR (0x02),		/**< Failed to map SoC POR registers. */
};


#endif	/* SOC_RESET_CONTROL_H_ */
