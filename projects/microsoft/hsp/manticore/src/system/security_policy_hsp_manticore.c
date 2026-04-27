// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "manticore_soc_rev.h"
#include "security_policy_hsp_manticore.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "drivers/hsp_aeb.h"
#include "system/manticore_aeb.h"


/**
 * Values for the type of security policy.
 */
enum {
	SECURITY_POLICY_HSP_MANTICORE_ONE_TIME_UNLOCK = 0,		/**< The policy represents a one-time unlock. */
	SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK = 1,	/**< The policy is persistent. */
};

/**
 * Values for the secure boot policy.
 */
enum {
	SECURITY_POLICY_HSP_MANTICORE_NO_SIGNATURE_VERIFICATION = 0x01,	/**< Flag to skip firmware signature verification. */
	SECURITY_POLICY_HSP_MANTICORE_NO_ANTI_ROLLBACK = 0x02,			/**< Flag to skip anti-rollback checks. */
};

/**
 * Values for AEB policy control.
 */
enum {
	SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_ENABLE = 0x01,	/**< Flag to apply AEB enable policy. */
	SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_DISABLE = 0x02,	/**< Flag to apply AEB disable policy. */
	SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_LOCK = 0x04,	/**< Flag to apply AEB lock policy. */
};

/**
 * Values for the memory fencing policy.
 */
enum {
	SECURITY_POLICY_HSP_MANTICORE_ENABLE_MEMORY_FENCING = 0,	/**< Apply memory fencing to the SoC. */
	SECURITY_POLICY_HSP_MANTICORE_DISABLE_MEMORY_FENCING = 1,	/**< Disable memory fencing for the SoC. */
};

/**
 * Values for SPRT feature flags.
 */
enum {
	SECURITY_POLICY_HSP_MANTICORE_ALLOW_FACTORY_DEFAULT = 0x01,	/**< Allow factory default without authorization. */
	SECURITY_POLICY_HSP_MANTICORE_ALLOW_MANIFEST_ERASE = 0x02,	/**< Allow erasing manifests without authorization. */
	SECURITY_POLICY_HSP_MANTICORE_ALLOW_INTRUSION_RESET = 0x04,	/**< Allow resetting intrusion state without authorization. */
	SECURITY_POLICY_HSP_MANTICORE_ALLOW_FIRMWARE_UPDATE = 0x08,	/**< Allow firmware updates without authorization. */
};

/**
 * Bitmask for the list of AEBs that will disable memory fencing when they are enabled.
 */
#define	SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEBS	( \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_RNG) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_DUAL_CP) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_NQM) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_BCP) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GDMA) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GSRAM) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_UPKA) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_AES) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_HSSHA) | \
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL) \
	)

/**
 * The AEB group that contains the memory fencing disable AEBs.
 */
#define	SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEB_GROUP \
	HSP_AEB_GET_GROUP (MANTICORE_AEB_DISABLE_FENCE_RNG)


/**
 * This is the security policy to apply to a locked device.  This represents a device with full
 * security features enabled.
 */
const struct security_policy_hsp_manticore_data locked_device_policy = {
	.policy_type = SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK,
	.secure_boot = 0x00,
	.apply_aeb = SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_ENABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_DISABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_LOCK,
	.fencing = SECURITY_POLICY_HSP_MANTICORE_ENABLE_MEMORY_FENCING,
	.sprt_features = 0x00,
	.aeb_policy[0] = 0x00000000,
	.aeb_policy[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_TDR_HSP_RESET) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_WRITE_ENABLE_FATAL_ERROR_MASK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW2_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW3_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD1_FUSES),
	.aeb_policy[2] = 0x00000000,
	.aeb_policy[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_WRITES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_UPKA),
	.aeb_lock[0] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_CORE_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_1P8_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_TEMP_TMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_CLK_CMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ALLOW_BLANK_CALIBRATION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_ACCESS_EMC_TDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ENABLE_CALIBRATION_WRITE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_SP_DEBUG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_EXT_SRAM_EXECUTION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_HWCHKPT_DISABLE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_SRAM_ECC) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_FMC_TDR),
	.aeb_lock[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_ROM_ADDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_RNG_ALLOW_DAS_MODE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SOCID_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_KEY_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR1_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_TOP_4K) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_EMC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD0_FUSES),
	.aeb_lock[2] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_SCAN_MBIST) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY0_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_EXTERNAL_AEMC_CLK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_RNG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_DUAL_CP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_NQM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_BCP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GDMA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GSRAM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_UPKA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SP_JTAG_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_CP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_FP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_DAP2AXI) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY1_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_BISR_CHAINS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_FMP_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_DFD_OBSERVABILITY_BUS),
	.aeb_lock[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_DAS_MODE)
};

/**
 * This is the security policy to apply to a device during manufacturing.  This represents a device
 * with most security features enabled, but SPRT is permitted to bypass authentication challenges
 * for authorized operations (e.g. chassis intrusion reset).
 */
const struct security_policy_hsp_manticore_data mfg_device_policy = {
	.policy_type = SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK,
	.secure_boot = 0x00,
	.apply_aeb = SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_ENABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_DISABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_LOCK,
	.fencing = SECURITY_POLICY_HSP_MANTICORE_ENABLE_MEMORY_FENCING,
	.sprt_features = SECURITY_POLICY_HSP_MANTICORE_ALLOW_FACTORY_DEFAULT |
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_MANIFEST_ERASE |
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_INTRUSION_RESET,
	.aeb_policy[0] = 0x00000000,
	.aeb_policy[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_TDR_HSP_RESET) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_WRITE_ENABLE_FATAL_ERROR_MASK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW2_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW3_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD1_FUSES),
	.aeb_policy[2] = 0x00000000,
	.aeb_policy[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_WRITES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_UPKA),
	.aeb_lock[0] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_CORE_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_1P8_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_TEMP_TMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_CLK_CMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ALLOW_BLANK_CALIBRATION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_ACCESS_EMC_TDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ENABLE_CALIBRATION_WRITE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_SP_DEBUG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_EXT_SRAM_EXECUTION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_HWCHKPT_DISABLE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_SRAM_ECC) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_FMC_TDR),
	.aeb_lock[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_ROM_ADDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_RNG_ALLOW_DAS_MODE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SOCID_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_KEY_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR1_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_TOP_4K) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_EMC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD0_FUSES),
	.aeb_lock[2] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_SCAN_MBIST) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY0_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_EXTERNAL_AEMC_CLK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_RNG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_DUAL_CP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_NQM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_BCP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GDMA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GSRAM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_UPKA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SP_JTAG_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_CP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_FP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_DAP2AXI) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY1_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_BISR_CHAINS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_FMP_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_DFD_OBSERVABILITY_BUS),
	.aeb_lock[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_DAS_MODE)
};

/**
 * This is the security policy to use with the recovery image.  This represents a device
 * with most security features enabled, but SPRT is permitted to allow firmware updates without
 * requiring any authorization.
 */
const struct security_policy_hsp_manticore_data recovery_device_policy = {
	.policy_type = SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK,
	.secure_boot = 0x00,
	.apply_aeb = SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_ENABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_DISABLE |
		SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_LOCK,
	.fencing = SECURITY_POLICY_HSP_MANTICORE_ENABLE_MEMORY_FENCING,
	.sprt_features = SECURITY_POLICY_HSP_MANTICORE_ALLOW_FIRMWARE_UPDATE,
	.aeb_policy[0] = 0x00000000,
	.aeb_policy[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_TDR_HSP_RESET) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_WRITE_ENABLE_FATAL_ERROR_MASK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW2_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW3_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD1_FUSES),
	.aeb_policy[2] = 0x00000000,
	.aeb_policy[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_WRITES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_UPKA),
	.aeb_lock[0] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_CORE_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_1P8_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_TEMP_TMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_CLK_CMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ALLOW_BLANK_CALIBRATION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_ACCESS_EMC_TDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_ENABLE_CALIBRATION_WRITE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_SP_DEBUG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ALLOW_EXT_SRAM_EXECUTION) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_HWCHKPT_DISABLE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_SRAM_ECC) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_JTAG_ENABLE_FMC_TDR),
	.aeb_lock[1] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_ROM_ADDR) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_RNG_ALLOW_DAS_MODE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SOCID_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_KEY_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_ECC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR1_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ROM_ALLOW_ACCESS_TOP_4K) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_EMC_FUSES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD0_FUSES),
	.aeb_lock[2] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_SCAN_MBIST) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY0_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_EXTERNAL_AEMC_CLK) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_RNG) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_DUAL_CP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_NQM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_BCP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GDMA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_GSRAM) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_UPKA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_AES) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_HSSHA) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SP_JTAG_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_CP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_FP_JTAG_DAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_DAP2AXI) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_PCIE_PHY1_SIF) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_BISR_CHAINS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_JTAG_FMP_ETAP) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_DFD_OBSERVABILITY_BUS),
	.aeb_lock[3] = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_ENABLE_SOC_RNG_DAS_MODE)
};


int security_policy_hsp_manticore_is_persistent (const struct security_policy *policy)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if (manticore == NULL) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	return (manticore->data->policy_type == SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK);
}

int security_policy_hsp_manticore_enforce_firmware_signing (const struct security_policy *policy)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if (manticore == NULL) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	return !(manticore->data->secure_boot &
		SECURITY_POLICY_HSP_MANTICORE_NO_SIGNATURE_VERIFICATION);
}

int security_policy_hsp_manticore_enforce_anti_rollback (const struct security_policy *policy)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if (manticore == NULL) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	return !(manticore->data->secure_boot & SECURITY_POLICY_HSP_MANTICORE_NO_ANTI_ROLLBACK);
}

int security_policy_hsp_manticore_check_unlock_persistence (const struct security_policy *policy,
	const uint8_t *unlock, size_t length)
{
	const struct security_policy_hsp_manticore_data *data =
		(const struct security_policy_hsp_manticore_data*) unlock;

	if ((policy == NULL) || (data == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	/* Make sure there is enough data to be a valid policy. */
	if (length < sizeof (struct security_policy_hsp_manticore_data)) {
		return SECURITY_POLICY_BAD_DATA;
	}

	return (data->policy_type == SECURITY_POLICY_HSP_MANTICORE_PERSISTENT_UNLOCK);
}

int security_policy_hsp_manticore_parse_unlock_policy (const struct security_policy *policy,
	const uint8_t *unlock, size_t length)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if ((manticore == NULL) || (unlock == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	if (manticore->is_const) {
		return SECURITY_POLICY_IMMUTABLE;
	}

	/* If there is not enough policy data, reject it.  Anything else represents valid data. */
	if (length < sizeof (struct security_policy_hsp_manticore_data)) {
		return SECURITY_POLICY_BAD_DATA;
	}

	memcpy (manticore->data, unlock, sizeof (struct security_policy_hsp_manticore_data));

	return 0;
}

int security_policy_hsp_manticore_get_enabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if ((manticore == NULL) || (aeb == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	if (word_count < ARRAY_SIZE (manticore->data->aeb_policy)) {
		return SECURITY_POLICY_SMALL_BUFFER;
	}

	if (manticore->data->apply_aeb & SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_ENABLE) {
		memcpy (aeb, manticore->data->aeb_policy, sizeof (manticore->data->aeb_policy));
	}
	else {
		memset (aeb, 0, sizeof (manticore->data->aeb_policy));
	}

	if (manticore->data->fencing == SECURITY_POLICY_HSP_MANTICORE_DISABLE_MEMORY_FENCING) {
		/* When memory fencing is disabled, always enable the AEBs that disable the SoC memory
		 * fences. */
		aeb[SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEB_GROUP] |=
			SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEBS;
	}

	if (MANTICORE_IS_A0 (manticore->socid[0])) {
		/* For A0 devices, AEB65 needs to remain enabled for internal flash access. */
		aeb[HSP_AEB_GET_GROUP (65)] |= HSP_AEB_GET_BIT_MASK (65);
	}

	return 0;
}

/**
 * Mask bits for AEBs that should not be disabled based on the security policy or device revision.
 *
 * @param manticore The security policy generating the AEB list.
 * @param aeb List of AEB words that contain the AEBs to disable.
 */
static void security_policy_hsp_manticore_mask_disabled_aebs (
	const struct security_policy_hsp_manticore *manticore, uint32_t *aeb)
{
	if (manticore->data->fencing == SECURITY_POLICY_HSP_MANTICORE_DISABLE_MEMORY_FENCING) {
		/* When memory fencing is disabled, never disable the AEBs that disable the SoC memory
		 * fences. */
		aeb[SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEB_GROUP] &=
			~SECURITY_POLICY_HSP_MANTICORE_DISABLE_FENCING_AEBS;
	}

	if (MANTICORE_IS_A0 (manticore->socid[0])) {
		/* For A0 devices, AEB65 needs to remain enabled for internal flash access. */
		aeb[HSP_AEB_GET_GROUP (65)] &= ~HSP_AEB_GET_BIT_MASK (65);
	}
}

int security_policy_hsp_manticore_get_disabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;
	size_t i;

	if ((manticore == NULL) || (aeb == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	if (word_count < ARRAY_SIZE (manticore->data->aeb_policy)) {
		return SECURITY_POLICY_SMALL_BUFFER;
	}

	for (i = 0; i < ARRAY_SIZE (manticore->data->aeb_policy); i++) {
		if (manticore->data->apply_aeb & SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_DISABLE) {
			aeb[i] = manticore->data->aeb_policy[i] ^ 0xffffffff;
		}
		else {
			aeb[i] = 0;
		}
	}

	security_policy_hsp_manticore_mask_disabled_aebs (manticore, aeb);

	return 0;
}

int security_policy_hsp_manticore_get_locked_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb, size_t word_count)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if ((manticore == NULL) || (aeb == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	if (word_count < ARRAY_SIZE (manticore->data->aeb_lock)) {
		return SECURITY_POLICY_SMALL_BUFFER;
	}

	if (manticore->data->apply_aeb & SECURITY_POLICY_HSP_MANTICORE_APPLY_AEB_LOCK) {
		memcpy (aeb, manticore->data->aeb_lock, sizeof (manticore->data->aeb_lock));

		/* Locked AEBs are disabled, so bits need to be masked. */
		security_policy_hsp_manticore_mask_disabled_aebs (manticore, aeb);
	}
	else {
		memset (aeb, 0, sizeof (manticore->data->aeb_lock));
	}

	/* Always block access to the AEB fuses, regardless of loaded security policy. */
	aeb[1] |= HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES);

	return 0;
}

int security_policy_hsp_manticore_get_fuse_disabled_aebs (const struct security_policy_hsp *policy,
	uint32_t *aeb)
{
	if ((policy == NULL) || (aeb == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	/* The list of AEBs to permanently disable via fuses is constant and cannot be overridden by any
	 * unlock policy or other device configuration. */
	*aeb = HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_CORE_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_VDD_1P8_VMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_TEMP_TMON_ERRORS) |
		HSP_AEB_GET_BIT_MASK (MANTICORE_AEB_EMC_BLOCK_CLK_CMON_ERRORS);

	return 0;
}

int security_policy_hsp_manticore_enforce_memory_fencing (const struct security_policy_hsp *policy)
{
	const struct security_policy_hsp_manticore *manticore =
		(const struct security_policy_hsp_manticore*) policy;

	if (manticore == NULL) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	return (manticore->data->fencing == SECURITY_POLICY_HSP_MANTICORE_ENABLE_MEMORY_FENCING);
}

/**
 * Initialize a security policy handler for Manticore.  The security policy is mutable and can be
 * replaced with a new policy at run-time.
 *
 * @param policy The policy handler to initialize.
 * @param data Buffer for the current security policy.  This may be pre-loaded with policy data, but
 * is not required.  The security policy can be updated by parsing an unlock policy.
 * @param socid The device SOCID.  This is used to differentiate between A0 and B0 devices.
 *
 * @return 0 if the policy handler was initialized successfully or an error code.
 */
int security_policy_hsp_manticore_init (struct security_policy_hsp_manticore *policy,
	struct security_policy_hsp_manticore_data *data, const uint32_t *socid)
{
	if ((policy == NULL) || (data == NULL) || (socid == NULL)) {
		return SECURITY_POLICY_INVALID_ARGUMENT;
	}

	memset (policy, 0, sizeof (struct security_policy_hsp_manticore));

	policy->base.base.is_persistent = security_policy_hsp_manticore_is_persistent;
	policy->base.base.enforce_firmware_signing =
		security_policy_hsp_manticore_enforce_firmware_signing;
	policy->base.base.enforce_anti_rollback = security_policy_hsp_manticore_enforce_anti_rollback;
	policy->base.base.check_unlock_persistence =
		security_policy_hsp_manticore_check_unlock_persistence;
	policy->base.base.parse_unlock_policy = security_policy_hsp_manticore_parse_unlock_policy;

	policy->base.get_enabled_aebs = security_policy_hsp_manticore_get_enabled_aebs;
	policy->base.get_disabled_aebs = security_policy_hsp_manticore_get_disabled_aebs;
	policy->base.get_locked_aebs = security_policy_hsp_manticore_get_locked_aebs;
	policy->base.get_fuse_disabled_aebs = security_policy_hsp_manticore_get_fuse_disabled_aebs;
	policy->base.enforce_memory_fencing = security_policy_hsp_manticore_enforce_memory_fencing;

	policy->data = data;
	policy->socid = socid;

	return 0;
}

/**
 * Initialize a security policy handler for Manticore.  The security policy will remain constant and
 * cannot be changed.
 *
 * @param policy The policy handler to initialize.
 * @param data Buffer for the current security policy.  This must contain valid policy data.  The
 * contents cannot be updated at run-time.
 * @param socid The device SOCID.  This is used to differentiate between A0 and B0 devices.
 *
 * @return 0 if the policy handler was initialized successfully or an error code.
 */
int security_policy_hsp_manticore_init_constant_policy (
	struct security_policy_hsp_manticore *policy,
	const struct security_policy_hsp_manticore_data *data, const uint32_t *socid)
{
	int status;

	status = security_policy_hsp_manticore_init (policy,
		(struct security_policy_hsp_manticore_data*) data, socid);
	if (status == 0) {
		policy->is_const = true;
	}

	return status;
}

/**
 * Release the resources used by a Manticore security policy handler.
 *
 * @param policy The policy handler to release.
 */
void security_policy_hsp_manticore_release (const struct security_policy_hsp_manticore *policy)
{
	UNUSED (policy);
}

/**
 * Check if a specific SPRT feature is unlocked in the security policy.
 *
 * @param policy The security policy to query.
 * @param feature The SPRT feature to to check for.
 *
 * @return true if the feature is enabled or false otherwise.
 */
static bool security_policy_hsp_manticore_check_sprt_feature (
	const struct security_policy_hsp_manticore *policy, uint8_t feature)
{
	if (policy == NULL) {
		return false;
	}

	return !!(policy->data->sprt_features & feature);
}

/**
 * Determine if factory default commands received by the device should be allowed to execute without
 * authorization.
 *
 * @param policy The security policy to query.
 *
 * @return true if unauthenticated factory default commands should be allowed.
 */
bool security_policy_hsp_manticore_allow_no_auth_factory_default (
	const struct security_policy_hsp_manticore *policy)
{
	return security_policy_hsp_manticore_check_sprt_feature (policy,
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_FACTORY_DEFAULT);
}

/**
 * Determine if manifest erase commands received by the device should be allowed to execute without
 * authorization.
 *
 * @param policy The security policy to query.
 *
 * @return true if unauthenticated manifest erase commands should be allowed.
 */
bool security_policy_hsp_manticore_allow_no_auth_manifest_erase (
	const struct security_policy_hsp_manticore *policy)
{
	return security_policy_hsp_manticore_check_sprt_feature (policy,
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_MANIFEST_ERASE);
}

/**
 * Determine if intrusion reset commands received by the device should be allowed to execute without
 * authorization.
 *
 * @param policy The security policy to query.
 *
 * @return true if unauthenticated intrusion reset commands should be allowed.
 */
bool security_policy_hsp_manticore_allow_no_auth_intrusion_reset (
	const struct security_policy_hsp_manticore *policy)
{
	return security_policy_hsp_manticore_check_sprt_feature (policy,
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_INTRUSION_RESET);
}

/**
 * Determine if firmware updates should be allowed to execute without authorization.
 *
 * @param policy The security policy to query.
 *
 * @return true if unauthenticated firmware updates should be allowed.
 */
bool security_policy_hsp_manticore_allow_no_auth_firmware_update (
	const struct security_policy_hsp_manticore *policy)
{
	return security_policy_hsp_manticore_check_sprt_feature (policy,
		SECURITY_POLICY_HSP_MANTICORE_ALLOW_FIRMWARE_UPDATE);
}
