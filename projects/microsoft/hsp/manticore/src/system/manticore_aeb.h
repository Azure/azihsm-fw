// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_AEB_H_
#define MANTICORE_AEB_H_


/**
 * Defines the AEB values that are valid for Manticore.  Any AEB number not listed does not map to
 * any functionality in the device.
 */
enum {
	/**
	 * EMC: Block VDD_CORE VMON error from propagating to error collector
	 */
	MANTICORE_AEB_EMC_BLOCK_VDD_CORE_VMON_ERRORS = 17,
	/**
	 * EMC: Block VDD_1p8 VMON error from propagating to error collector
	 */
	MANTICORE_AEB_EMC_BLOCK_VDD_1P8_VMON_ERRORS = 18,
	/**
	 * EMC: Block TEMP TMON error from propagating to error collector
	 */
	MANTICORE_AEB_EMC_BLOCK_TEMP_TMON_ERRORS = 19,
	/**
	 * EMC: Block CLK CMON error from propagating to error collector
	 */
	MANTICORE_AEB_EMC_BLOCK_CLK_CMON_ERRORS = 20,
	/**
	 * EMC: Allow Zero (Blank) Calibration fuses
	 */
	MANTICORE_AEB_EMC_ALLOW_BLANK_CALIBRATION = 21,
	/**
	 * JTAG: Enable access to EMC control TDR
	 */
	MANTICORE_AEB_JTAG_ENABLE_ACCESS_EMC_TDR = 22,
	/**
	 * EMC: Enable writing to EMC calibration registers
	 */
	MANTICORE_AEB_EMC_ENABLE_CALIBRATION_WRITE = 23,
	/**
	 * Enable EMC analog debug bus
	 */
	MANTICORE_AEB_ENABLE_EMC_ANALOG_BUS = 24,
	/**
	 * JTAG: SW Debug for HSP's SP cores
	 */
	MANTICORE_AEB_JTAG_ENABLE_SP_DEBUG = 25,
	/**
	 * Intercepts all executable AXI transactions (instruction fetch) from SP to HSP bus (and the
	 * rest of SOCs)
	 */
	MANTICORE_AEB_ALLOW_EXT_SRAM_EXECUTION = 27,
	/**
	 * HWCHKPT: Turn-off HSP HW checkpointing (hashing) feature locks
	 */
	MANTICORE_AEB_HWCHKPT_DISABLE = 29,
	/**
	 * Disable ECC checks on RAMs, including TCM
	 */
	MANTICORE_AEB_DISABLE_SRAM_ECC = 30,
	/**
	 * JTAG: Enable Fuse Macro TDR (routed to FMC)
	 */
	MANTICORE_AEB_JTAG_ENABLE_FMC_TDR = 31,
	/**
	 * Allow HSP to be Reset via the tdr_hsp_reset register
	 */
	MANTICORE_AEB_ALLOW_TDR_HSP_RESET = 32,
	/**
	 * Disable MPU protection
	 */
	MANTICORE_AEB_DISABLE_MPU_PROTECTION = 33,
	/**
	 * HSP and FMC DFT IJTAG enable (Affect MBIST, scan, scan dump)
	 */
	MANTICORE_AEB_ENABLE_DFT_IJTAG = 34,
	/**
	 * MEM: Allow EDC error injection
	 */
	MANTICORE_AEB_MEM_ALLOW_EDC_ERROR_INJECTION = 35,
	/**
	 * HSP Fatal Error Mask Register (Sticky/Non-Sticky) Write Enable
	 */
	MANTICORE_AEB_WRITE_ENABLE_FATAL_ERROR_MASK = 36,
	/**
	 * ROM: Allow access to  SP ROM address space
	 */
	MANTICORE_AEB_ROM_ALLOW_ACCESS_ROM_ADDR = 37,
	/**
	 * RNG: Allow RNG DAS mode for raw TRBG collection
	 */
	MANTICORE_AEB_RNG_ALLOW_DAS_MODE = 39,
	/**
	 * FCTRL: Program and read enable for AEB  Fuse (fuses are loaded at boot irrespective of this
	 * AEB)
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES = 40,
	/**
	 * FCTRL: Program and read enable for the N_SOC_ID (fuses are loaded at boot irrespective of
	 * this AEB)
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SOCID_FUSES = 41,
	/**
	 * FCTRL: Program enable for the entire key region
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_KEY_FUSES = 42,
	/**
	 * FCTRL: Program, read and load enable for the Software Fuses Region 0
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_FUSES = 43,
	/**
	 * FCTRL: Program, read and load enable for the Software Fuses Region 1
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_FUSES = 44,
	/**
	 * FCTRL: Program, read and load enable for SW0_ECC slot
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW0_ECC_FUSES = 45,
	/**
	 * FCTRL: Sense enable for the Key fuses
	 */
	MANTICORE_AEB_FCTRL_ENABLE_SENSE_KEY_FUSES = 46,
	/**
	 * FCTRL: Obs enable
	 */
	MANTICORE_AEB_FCTRL_ENABLE_OBS = 47,
	/**
	 * allow SP to run
	 */
	MANTICORE_AEB_ALLOW_SP_RUN = 48,
	/**
	 * Allows SP to access the SoC-level JTAG interface via register reads/writes into CREG.
	 */
	MANTICORE_AEB_ALLOW_SOC_JTAG_ACCESS = 49,
	/**
	 * FCTRL: Program, read and load enable for SW1_ECC slot
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW1_ECC_FUSES = 50,
	/**
	 * FCTRL: Program, read and load enable for SW2_ECC slot
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW2_ECC_FUSES = 51,
	/**
	 * FCTRL: Program, read and load enable for SW3_ECC slot
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW3_ECC_FUSES = 52,
	/**
	 * CCS: SP reinit PCR1
	 */
	MANTICORE_AEB_CCS_ALLOW_PCR1_REINIT = 53,
	/**
	 * CCS: non-SP reinit PCR3
	 */
	MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT = 54,
	/**
	 * ROM: Allow access to top 4k page of SP ROM that holds secrets
	 */
	MANTICORE_AEB_ROM_ALLOW_ACCESS_TOP_4K = 55,
	/**
	 * FCTRL_Program enable for the EMC fuses
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_EMC_FUSES = 56,
	/**
	 * FCTRL: Program, read and load enable for SW4_ECC slot
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_SW4_ECC_FUSES = 57,
	/**
	 * FCTRL: Program enable for RSVD0 regions
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD0_FUSES = 58,
	/**
	 * FCTRL: Program enable for RSVD1 regions
	 */
	MANTICORE_AEB_FCTRL_ENABLE_ACCESS_RSVD1_FUSES = 59,
	/**
	 * Enable SoC scan/mbist/scan dump
	 */
	MANTICORE_AEB_ENABLE_SOC_SCAN_MBIST = 64,
	/**
	 * Enable PCIe PHY (COMPHY0) SIF
	 */
	MANTICORE_AEB_ENABLE_PCIE_PHY0_SIF = 65,
	/**
	 * Enable external AEMC clock source rather than internal PLL
	 */
	MANTICORE_AEB_ENABLE_EXTERNAL_AEMC_CLK = 67,
	/**
	 * Disable FENCE: RNG
	 */
	MANTICORE_AEB_DISABLE_FENCE_RNG = 70,
	/**
	 * Disable FENCE:APB Bridge
	 */
	MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE = 71,
	/**
	 * Disable FENCE:DUAL_CP
	 */
	MANTICORE_AEB_DISABLE_FENCE_DUAL_CP = 72,
	/**
	 * Disable FENCE:NQM
	 */
	MANTICORE_AEB_DISABLE_FENCE_NQM = 73,
	/**
	 * Disable FENCE:BCP
	 */
	MANTICORE_AEB_DISABLE_FENCE_BCP = 74,
	/**
	 * Disable FENCE:GDMA
	 */
	MANTICORE_AEB_DISABLE_FENCE_GDMA = 75,
	/**
	 * Disable FENCE:GSRAM
	 */
	MANTICORE_AEB_DISABLE_FENCE_GSRAM = 76,
	/**
	 * Disable FENCE:UPKA Mini Fabric
	 */
	MANTICORE_AEB_DISABLE_FENCE_UPKA = 77,
	/**
	 * Disable FENCE:AES
	 */
	MANTICORE_AEB_DISABLE_FENCE_AES = 78,
	/**
	 * Disable FENCE:HSSHA
	 */
	MANTICORE_AEB_DISABLE_FENCE_HSSHA = 79,
	/**
	 * Disable FENCE:PCIE_CTRL 32-bit addr
	 */
	MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL = 80,
	/**
	 * Enable HSP SP eTAP
	 */
	MANTICORE_AEB_ENABLE_SP_JTAG_ETAP = 83,
	/**
	 * Enable ARM CP DAP (2 M7's in CP Cluster)
	 */
	MANTICORE_AEB_ENABLE_CP_JTAG_DAP = 85,
	/**
	 * Enable ARM FP DAP (3 M7's in NQM)
	 */
	MANTICORE_AEB_ENABLE_FP_JTAG_DAP = 86,
	/**
	 * Enable JTAG - DAP2AXI Access Port
	 */
	MANTICORE_AEB_ENABLE_JTAG_DAP2AXI = 87,
	/**
	 * Enable PCIe PHY (COMPHY1) SIF
	 */
	MANTICORE_AEB_ENABLE_PCIE_PHY1_SIF = 88,
	/**
	 * Enable JTAG access to BISR chains
	 */
	MANTICORE_AEB_ENABLE_JTAG_BISR_CHAINS = 89,
	/**
	 * Enable JTAG eTAP SYS FMC
	 */
	MANTICORE_AEB_ENABLE_JTAG_FMP_ETAP = 90,
	/**
	 * Enable for DFD Observability Bus
	 */
	MANTICORE_AEB_ENABLE_DFD_OBSERVABILITY_BUS = 93,
	/**
	 * Enable AES operations
	 */
	MANTICORE_AEB_ENABLE_SOC_AES = 96,
	/**
	 * Enable HSSHA operations
	 */
	MANTICORE_AEB_ENABLE_SOC_HSSHA = 97,
	/**
	 * Enable RNG DAS mode
	 */
	MANTICORE_AEB_ENABLE_SOC_RNG_DAS_MODE = 98,
	/**
	 * Enable RNG writes
	 */
	MANTICORE_AEB_ENABLE_SOC_RNG_WRITES = 99,
	/**
	 * Enable UPKA operations (all engines tied to single bit)
	 */
	MANTICORE_AEB_ENABLE_SOC_UPKA = 100,
	/**
	 * A0_BYPASS Status Output
	 */
	MANTICORE_AEB_A0_BYPASS_STATUS = 127,
};


#endif	/* MANTICORE_AEB_H_ */
