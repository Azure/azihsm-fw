// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "build_version.h"
#include "chkpt_1sp.h"
#include "hsp_top.h"
#include "manticore_1sp.h"
#include "manticore_hsp_gpio.h"
#include "manticore_rom.h"
#include "manticore_sticky_regs.h"
#include "platform_config.h"
#include "platform_io_api.h"
#include "reset_counter_init.h"
#include "rot_memory_map.h"
#include "soc_shared.h"
#include "sp_boot.h"
#include "traps.h"
#include "asn1/base64_core_static.h"
#include "asn1/dice/tcg_dice_oid.h"
#include "asn1/dice/x509_extension_builder_dice_tcbinfo_static.h"
#include "asn1/dice/x509_extension_builder_dice_ueid_static.h"
#include "asn1/dme/dme_structure_raw_ecc.h"
#include "asn1/dme/x509_extension_builder_dme_static.h"
#include "asn1/ecc_der_util.h"
#include "asn1/spdm/spdm_oid.h"
#include "asn1/x509_cert_build_static.h"
#include "asn1/x509_extension_builder_eku_static.h"
#include "common/array_size.h"
#include "common/buffer_util.h"
#include "common/sram_util.h"
#include "common/unused.h"
#include "crypto/ecc_ccs_static.h"
#include "crypto/ecc_ecc_hw_static.h"
#include "crypto/ecc_hw_pka_static.h"
#include "crypto/ecdsa.h"
#include "crypto/hash_hs_sha_static.h"
#include "crypto/kat/ecdsa_kat.h"
#include "crypto/rng_hsp_static.h"
#include "drivers/ccs_ksu_fips_static.h"
#include "drivers/ccs_ksu_static.h"
#include "drivers/checkpoint_static.h"
#include "drivers/fuse_controller_manticore_fips_static.h"
#include "drivers/hs_sha_static.h"
#include "drivers/hsp_aeb_static.h"
#include "drivers/hsp_aes_static.h"
#include "drivers/hsp_dmb_static.h"
#include "drivers/hsp_fuses.h"
#include "drivers/hsp_gpio_static.h"
#include "drivers/hsp_rng_hw_static.h"
#include "drivers/kat/ccs_ksu_kat.h"
#include "drivers/kat/hsp_rng_hw_kat.h"
#include "drivers/spi_dwc_ssi_static.h"
#include "firmware/firmware_loader_hsp_dmb_static.h"
#include "firmware/firmware_loader_hsp_memory_static.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/hsp_fw_util.h"
#include "firmware/hw_rot_firmware_key_manifest_static.h"
#include "firmware/identity_renewal_static.h"
#include "firmware/manticore_bootloader_static.h"
#include "firmware/manticore_device_keys.h"
#include "flash/flash_master_dwc_ssi_static.h"
#include "flash/flash_store_contiguous_blocks_static.h"
#include "flash/spi_flash_static.h"
#include "init/init_error.h"
#include "init/pcie_phy.h"
#include "logging/boot_logging.h"
#include "logging/code_path_integrity.h"
#include "logging/logging_flash_static.h"
#include "logging/logging_memory_static.h"
#include "logging/manticore_logging.h"
#include "marvell/RegTcon.h"
#include "mmio/mmio_register_block_hsp_static.h"
#include "mmio/mmio_register_block_soc_static.h"
#include "mpu/fence_manticore_static.h"
#include "mpu/hsp_mpu_static.h"
#include "mpu/memory_protection_manticore_1sp_static.h"
#include "riot/dice_oid.h"
#include "riot/riot_core_hsp_fips_static.h"
#include "riot/tcg_dice.h"
#include "rom/device_keys.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "system/device_unlock_token.h"
#include "system/hsp_watchdog_static.h"
#include "system/manticore_aeb.h"
#include "system/real_time_clock_hsp_static.h"
#include "system/security_manager_hsp_manticore_static.h"
#include "system/security_policy_hsp_manticore_static.h"
#include "trap/hsp_trap.h"

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#include "crypto/kat/ecc_kat_vectors.h"
#include "drivers/fuse_controller_manticore_fips_cmvp_static.h"
#include "drivers/kat/ccs_ksu_kat_vectors.h"
#include "drivers/kat/hsp_rng_hw_kat_vectors.h"
#endif


/* EMC configuration to apply to the device. */
#define	MANTICORE_ENABLE_EMC_VMON		1
#define	MANTICORE_ENABLE_EMC_TMON		1
#define	MANTICORE_ENABLE_EMC_CMON		0


/**
 * Base address for the POR registers.
 */
#define	MANTICORE_SOC_POR_REGISTERS				0xb0003000

/**
 * Address for the reset control register for SoC hardware.
 */
#define	MANTICORE_SOC_RESET_CTRL_REGISTER		0xb0004000

/**
 * Base address for the TCON registers.
 */
#define	MANTICORE_SOC_TCON_REGISTERS			0xb0005000

/**
 * Bit number for tcon_regs wakeupCtrl.b.WAKEUP_ENABLE.
 */
#define	WAKEUP_CTR1_WAKEUP_ENABLE_BIT		0

/**
 * Bit number for tcon_regs wakeupCtrl.b.WKINTR_LEVEL_EN.
 */
#define	WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT		8

/**
 * Bit count for tcon_regs wakeupCtrl.b.WAKEUP_ENABLE.
 */
#define	WAKEUP_CTR1_WAKEUP_ENABLE_BIT_COUNT	2

/**
 * Bit count for tcon_regs wakeupCtrl.b.WKINTR_LEVEL_EN.
 */
#define	WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT_COUNT	2

/**
 * Delay to allow TCON wakeup ISR to be executed in all cores.
 */
#define TCON_WAKEUP_ISR_DELAY_IN_MS 10


/**
 * Reset control bits for executing soft reset of cores.
 */
enum {
	MANTICORE_SOC_RESET_CTRL_CP0 = (1U << 19),	/**< Soft reset control for CP core 0. */
	MANTICORE_SOC_RESET_CTRL_CP1 = (1U << 20),	/**< Soft reset control for CP core 1. */
	MANTICORE_SOC_RESET_CTRL_FP0 = (1U << 22),	/**< Soft reset control for FP core 0. */
	MANTICORE_SOC_RESET_CTRL_FP1 = (1U << 23),	/**< Soft reset control for FP core 1. */
	MANTICORE_SOC_RESET_CTRL_FP2 = (1U << 24),	/**< Soft reset control for FP core 2. */
};

/**
 * Address for the run/stall control register for the FP hardwares.
 */
#define	MANTICORE_SOC_FP_RUNSTALL_REGISTER		0xb0004004

/**
 * Control bits for stalling the CP cores.
 */
enum {
	MANTICORE_SOC_FP_RUNSTALL_FP0_WAIT = (1U << 0),	/**< Stall FP core 0. */
	MANTICORE_SOC_FP_RUNSTALL_FP1_WAIT = (1U << 1),	/**< Stall FP core 1. */
	MANTICORE_SOC_FP_RUNSTALL_FP2_WAIT = (1U << 2),	/**< Stall FP core 2. */
};

/**
 * Address for the run/stall control register for the CP processors.
 */
#define	MANTICORE_SOC_CP_RUNSTALL_REGISTER		0xb0003008

/**
 * Control bits for stalling the CP cores.
 */
enum {
	MANTICORE_SOC_CP_RUNSTALL_CP0_WAIT = (1U << 0),	/**< Stall CP core 0. */
	MANTICORE_SOC_CP_RUNSTALL_CP1_WAIT = (1U << 1),	/**< Stall CP core 1. */
};

/**
 * Address for the SoC GPIO register where status of boot strapping pins is stored.
 */
#define	MANTICORE_SOC_GPIO_LATCHED_REGISTER		0xb0007104

/**
 * Latched values of the boot strapping pins on the SoC.
 */
enum {
	MANTICORE_SOC_GPIO_STRAP_STRAP0 = (1U << 0),			/**< Value of Strap 0 pin. */
	MANTICORE_SOC_GPIO_STRAP_STRAP1 = (1U << 1),			/**< Value of Strap 1 pin. */
	MANTICORE_SOC_GPIO_STRAP_A0_BYPASS = (1U << 2),			/**< Value of A0 Bypass pin. */
	MANTICORE_SOC_GPIO_STRAP_FORCE_RECOVERY = (1U << 3),	/**< Value of Force Recovery pin. */
	MANTICORE_SOC_GPIO_STRAP_FLASH_PRIORITY = (1U << 4),	/**< Value of Flash Priority pin. */
};

/**
 * Reset control for the CP and FP CPUs and latched strapping values.
 */
struct por_regs {
	uint8_t pad0[8];						/**< Unused. */
	volatile uint32_t cp_run_stall;			/**< Run/stall control register for CP. */
	uint8_t pad1[4084];						/**< Unused. */
	volatile uint32_t reset_control;		/**< SoC reset control. */
	volatile uint32_t fp_run_stall;			/**< Run/stall control register for FP. */
	uint8_t pad2[0x30fc];					/**< Unused. */
	volatile uint32_t gpio_latched_input;	/**< Latched input for strapping pins. */
};


_Static_assert ((offsetof (struct por_regs, cp_run_stall) == 0x0008), "CP run/stall offset wrong.");
_Static_assert ((offsetof (struct por_regs, reset_control) == 0x1000),
	"Reset Control offset wrong.");
_Static_assert ((offsetof (struct por_regs, fp_run_stall) == 0x1004), "FP run/stall offset wrong.");
_Static_assert ((offsetof (struct por_regs, gpio_latched_input) == 0x4104),
	"Strapping pin latched input offset wrong.");


/**
 * Base address for registers to enable ECC in CP memories.
 */
#define	MANTICORE_SOC_CP_ECC_REGISTERS			0xb0200000

/**
 * Control bit for enabling TCM ECC on each of the CP cores.
 */
enum {
	MANTICORE_SOC_CP_CTRL_TCM_ECC_EN = (1U << 20),	/**< Enable ECC for CP TCM. */
};

/**
 * Memory control for CP CPUs.
 */
struct cp_ecc_control {
	uint8_t pad0[0x100];			/**< Unused. */
	volatile uint32_t cp0_control;	/**< Control register for CP core 0. */
	uint8_t pad1[0xfc];				/**< Unused. */
	volatile uint32_t cp1_control;	/**< Control register for CP core 1. */
};


_Static_assert ((offsetof (struct cp_ecc_control, cp0_control) == 0x0100),
	"CP0 Control offset wrong.");
_Static_assert ((offsetof (struct cp_ecc_control, cp1_control) == 0x0200),
	"CP1 Control offset wrong.");


/**
 * Base address for registers to enable ECC in FP memories.
 */
#define	MANTICORE_SOC_FP_ECC_REGISTERS			0xa1e00000

/**
 * Control bit for clearing ECC and parity errors.
 */
enum {
	MANTICORE_SOC_FP_CONTROL_ERROR_CLEAR = (1U << 2),	/**< Clear all ECC or parity errors. */
};

/**
 * Control bits for enabling TCM ECC on each of the FP cores.
 */
enum {
	MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_GENERATE = (1U << 28) | (1U << 30),	/**< Enable ECC generation. */
	MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE = (1U << 29) | (1U << 31),		/**< Enable ECC checking. */
};

enum {
	MANTICORE_SOC_FP_MEM_CONTROL_INIT_PATTERN_ONES = (1U < 16),			/**< PSRAM init all ones.  Clear for all zeros. */
	MANTICORE_SOC_FP_MEM_CONTROL_ECC_PARTIAL_WRITE_RMW_EN = (1U << 17),	/**< Enable read/modify/write for partial writes. */
};

/**
 * Control bit for enabling ECC on the SRAM shared with all FP cores.
 */
enum {
	MANTICORE_SOC_FP_PSRAM_MEM_CONTROL_ECC_EN = (1U << 16) | (1U << 17),	/**< Enable ECC for PSRAM0. */
	MANTICORE_SOC_FP_PSRAM_MEM_CONTROL_INIT_EN = (1U << 24),				/**< Run memory initialization. */
};

/**
 * Value to apply to enable parity checking for the data fabric.
 */
#define	MANTICORE_SOC_FP_FABRIC_PARITY_CHECK_EN		0x17070701

/**
 * Memory control for FP CPUs.
 */
struct fp_ecc_control {
	struct {
		uint8_t pad0[0x70];						/**< Unused. */
		volatile uint32_t fps_control;			/**< FPS control, including error clear. */
		uint8_t pad1[0x500 - 0x70 - 4];			/**< Unused. */
	} bank0;									/**< Bank 0 of global FP registers. */
	uint8_t pad0[6912];							/**< Unused. */
	struct {
		uint8_t pad0[0x60];						/**< Unused. */
		volatile uint32_t cpu_mem_control;		/**< iTCM/dTCM memory control for FP cores. */
		uint8_t pad1[0x100 - 0x60 - 4];			/**< Unused. */
	} fp_cpu[3];								/**< Memory control for FP cores. */
	uint8_t pad1[7424];							/**< Unused. */
	struct {
		uint8_t pad0[0x70];						/**< Unused. */
		volatile uint32_t psram_mem_control;	/**< PSRAM memory control. */
		uint8_t pad1[12];						/**< Unused. */
		volatile uint32_t fps_mem_control;		/**< FPS memory control. */
		uint8_t pad2[12];						/**< Unused. */
		volatile uint32_t fabric_error_control;	/**< Error control for the data fabric. */
	} bank1;									/**< Bank 1 of global FP registers. */
};


_Static_assert ((offsetof (struct fp_ecc_control, bank0.fps_control) == 0x0070),
	"FPS Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, fp_cpu[0].cpu_mem_control) == 0x2060),
	"FP0 Mem Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, fp_cpu[1].cpu_mem_control) == 0x2160),
	"FP1 Mem Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, fp_cpu[2].cpu_mem_control) == 0x2260),
	"FP2 Mem Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, bank1.psram_mem_control) == 0x4070),
	"PSRAM Mem Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, bank1.fps_mem_control) == 0x4080),
	"FPS Mem Control offset wrong.");
_Static_assert ((offsetof (struct fp_ecc_control, bank1.fabric_error_control) == 0x4090),
	"Fabric Error Control offset wrong.");

/**
 * Base address for registers to enable ECC in GSRAM.
 */
#define	MANTICORE_SOC_GSRAM_ECC_REGISTERS		0xb000c040

/**
 * Control bits to enable ECC for GSRAM.
 */
enum {
	MANTICORE_SOC_GSRAM_ECC_ENABLE = 0xff,	/**< Enable ECC on all GSRAM blocks. */
};

/**
 * Options for running the GSRAM memory initialization sequence.
 */
enum {
	MANTICORE_SOC_GSRAM_MEM_INIT_START = (1U << 16),			/**< Start the memory init sequence. */
	MANTICORE_SOC_GSRAM_MEM_INIT_START_FIRST_BLOCK = (0 << 8),	/**< Start memory init with the first block. */
	MANTICORE_SOC_GSRAM_MEM_INIT_ALL_BLOCKS = (0x7 << 0),		/**< Run memory init on all GSRAM blocks. */
	MANTICORE_SOC_GSRAM_MEM_ECC_ERR_STATUS = (1U << 1),			/**< Enable double bit error detection. */
};

/**
 * The GSRAM ERRDLOG0 register mask.
 */
#define MANTICORE_SOC_GSRAM_REG_ERRDLOG0_MASK			0x001fffe0


/**
 * Register interface for the GSRAM.
 */
struct gsram_ecc_control {
	volatile uint32_t ecc_enable_state;	/**< Read-only enable state for GSRAM ECC. */
	volatile uint32_t ecc_enable_set;	/**< Set the GSRAM ECC enable bits. */
	volatile uint32_t ecc_enable_clear;	/**< Clear the GSRAM ECC enable bits. */
	volatile uint32_t mem_init;			/**< Block memory initialization control. */
	volatile uint32_t mem_int_stt;		/**< Block memory init and error status. */
	volatile uint8_t rsvd_0[76];		/**< Unused. */
	volatile uint32_t errd_log0;		/**< Error log register 0. */
};


_Static_assert ((offsetof (struct gsram_ecc_control, errd_log0) == (0x00A0 - 0x40)),
	"GSRAM ECC ERRDLOG0 register offset wrong.");

/**
 * Data populated by ROM that can be used with local static initialization.
 */
static struct manticore_rom_shared_sram *const rom_shared_static =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * Address in shared SRAM that marks the start of crypto command buffers.
 */
#define	MANTICORE_CRYPTO_SHARED_SRAM_START	(uint32_t*) &rom_shared_static->internal

/**
 * Address in shared SRAM that marks the end of the crypto command buffers.
 */
#define	MANTICORE_CRYPTO_SHARED_SRAM_END    \
	(uint32_t*) (((uint32_t) &rom_shared_static->internal) + sizeof (rom_shared_static->internal))

/**
 * Register mapping for the entire CREG address space.
 */
static const struct mmio_register_block_hsp creg_regs =
	mmio_register_block_hsp_static_init ((uint32_t*) HSP_ADDR_MAP_CREG_ADDRESS,
	sizeof (struct Creg_regs));


/**
 * Data to be initialized by 1SP that is left available in protected memory for SPRT.
 */
SECTION (".sp1_shared_data")
static struct manticore_1sp_shared_data sp1_shared;

/**
 * Data in GSRAM that will be initialized by 1SP for consumption by CP.
 */
static struct cp_shared_data *cp_shared;

/**
 * HSP MPU page size
 */
#define HSP_MPU_PAGE_SIZE	4096

/**
 * HSP MPU memory map
 */
static const struct hsp_mpu_memory_map_entry hsp_mpu_memory_map[] = {
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_ROM_ADDRESS,	/* SP ROM */
			.length = HSP_ADDR_MAP_SP_ROM_SIZE,
		},
		.user_register_offset =
			CREG_OFFSET (HSP_ADDR_MAP_CREG_MPU_REGS_SPROM_MPU_REGS_SPROM_USER_ATTRIB_0_ADDRESS),
		.privileged_register_offset =
			CREG_OFFSET (
			HSP_ADDR_MAP_CREG_MPU_REGS_SPROM_MPU_REGS_SPROM_PRIVILEGE_ATTRIB_0_ADDRESS),
	},
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS,	/* SP iTCM */
			.length = HSP_ADDR_MAP_SP_IRAM_SIZE,
		},
		.user_register_offset =
			CREG_OFFSET (HSP_ADDR_MAP_CREG_MPU_REGS_SPIRAM_MPU_REGS_SPIRAM_USER_ATTRIB_0_ADDRESS),
		.privileged_register_offset =
			CREG_OFFSET (
			HSP_ADDR_MAP_CREG_MPU_REGS_SPIRAM_MPU_REGS_SPIRAM_PRIVILEGE_ATTRIB_0_ADDRESS),
	},
	{
		.memory_region = {
			.start = (const void*) HSP_ADDR_MAP_SP_DRAM_ADDRESS,	/* SP dTCM */
			.length = HSP_ADDR_MAP_SP_DRAM_SIZE,
		},
		.user_register_offset =
			CREG_OFFSET (HSP_ADDR_MAP_CREG_MPU_REGS_SPDRAM_MPU_REGS_SPDRAM_USER_ATTRIB_0_ADDRESS),
		.privileged_register_offset =
			CREG_OFFSET (
			HSP_ADDR_MAP_CREG_MPU_REGS_SPDRAM_MPU_REGS_SPDRAM_PRIVILEGE_ATTRIB_0_ADDRESS),
	},
};

/**
 * HSP MPU driver
 */
static const struct hsp_mpu mpu = hsp_mpu_static_init (&creg_regs.base, HSP_MPU_PAGE_SIZE,
	hsp_mpu_memory_map, ARRAY_SIZE (hsp_mpu_memory_map));

/**
 * CREG timer driver to enable hardware watchdog functionality.
 */
static const struct hsp_watchdog watchdog = hsp_watchdog_static_init (&creg_regs.base,
	offsetof (struct Creg_regs, timer0_regs), offsetof (struct Creg_regs, fatal_err_regs));

/**
 * Stack guard to check for overflows.
 */
extern uint32_t __stack_chk_guard;

/**
 * Linker output that marks the beginning of executable code.
 */
extern uint32_t _exe_start;

/**
 * Linker output that marks the beginning of read-only data.
 */
extern uint32_t _ro_start;

/**
 * Linker output that marks the start of SRAM that is used for 1SP data and stack.
 */
extern uint32_t _start_data;

/**
 * Linker output that marks the end of 1SP memory.
 */
extern uint32_t _memory_end;


/**
 * The end address of SRAM that is used for 1SP data and stack.
 */
#define	MANTICORE_1SP_DATA_END  \
	(uint32_t*) ((HSP_ADDR_MAP_SP_DRAM_ADDRESS + HSP_ADDR_MAP_SP_DRAM_SIZE) - \
		TOTAL_SHARED_DATA_MEMORY)

/**
 * Cache for the SOCID that does not require word access.
 */
static uint32_t ueid[IN_DWORDS (HSP_FUSES_LENGTH (SOCID))];

/**
 * Variable context for the AEB driver.
 */
static struct hsp_aeb_state aeb_context;

/**
 * Interface for configuring AEBs.
 */
static const struct hsp_aeb aeb = hsp_aeb_static_init (&aeb_context,
	(struct Creg_regs_aeb_regs*) HSP_ADDR_MAP_CREG_AEB_INTERFACE_ADDRESS);

/**
 * DMB segment descriptors for the system.
 */
static struct hsp_dmb_segment dmb_segments[HSP_DMB_SEGMENTS];

/**
 * Variable context for the DMB driver.
 */
static struct hsp_dmb_state dmb_context;

/**
 * Driver for the HSP DMB.
 */
const struct hsp_dmb dmb = hsp_dmb_static_init (&dmb_context, dmb_segments, HSP_DMB_SEGMENTS,
	HSP_DMB_BASE_MAPPING_ADDRESS, (struct Dmb_reg*) HSP_ADDR_MAP_DMB_ADDRESS);

/**
 * Driver for HSP GPIOs.
 */
static const struct hsp_gpio gpio =
	hsp_gpio_static_init_no_irq_support (
	(struct Creg_regs_gpc_regs*) HSP_ADDR_MAP_CREG_GPIO_REGS_ADDRESS, MANTICORE_HSP_GPIO_COUNT);

/**
 * Flag indicating if the boot context is secure.
 */
static bool secure_boot;


/**
 * Ensures the detection logic for EMC trips is enabled and configures the failure to be a sticky
 * fatal error.
 *
 * This does not disable any of the AEBs that block EMC trips, as those will be permanently disabled
 * through AEB fuses.
 *
 * Ultimately, the VMON is expected to be enabled by default through the EMC calibration, so this
 * would only need to enable TMON (and perhaps CMON).  Although, the VMOM write locks would still be
 * needed.
 *
 * @param socid The SOCID for the device.  This must be cached into byte accessible memory.
 */
static void enable_emc_errors (const uint32_t *socid)
{
	struct Creg_regs_emc_control *emc_ctrl =
		(struct Creg_regs_emc_control*) HSP_ADDR_MAP_CREG_ECL_EMC_ADDRESS;
	struct Creg_regs_emc_regs *emc_error =
		(struct Creg_regs_emc_regs*) HSP_ADDR_MAP_CREG_EMC_EN_ADDRESS;
	uint32_t power_down = 0;
	uint32_t status_mask = 0;

#ifdef MANTICORE_ENABLE_A0_SUPPORT
	if (MANTICORE_IS_A0 (socid[0])) {
		/* Do nothing on A0 devices since they are not calibrated. */
		platform_printf ("A0 device, no EMC" NEWLINE);

		return;
	}
#endif

	/* Enable EMC sticky fatal errors to halt HSP and reset SoC cores on EMC trips. */
	emc_error->EMC_STICKY_ERR_EN |= CREG_REGS_EMC_REGS_EMC_STICKY_ERR_EN_ERR_EMC_FIELD_MASK;

#if MANTICORE_ENABLE_EMC_VMON || MANTICORE_ENABLE_EMC_TMON || MANTICORE_ENABLE_EMC_CMON
	/* At least one EMC monitor is enabled. */
	power_down |= CREG_REGS_EMC_CONTROL_POWER_DOWN_REG_COMMON_MODULE_FIELD_MASK;
#endif

#if MANTICORE_ENABLE_EMC_VMON
	/* Enable errors based on VMON trips. */
	power_down |= CREG_REGS_EMC_CONTROL_POWER_DOWN_REG_VMON_HSP_CORE_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_POWER_DOWN_REG_VMON_AEMC_CORE_FIELD_MASK;
	status_mask |= CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_VMON_HSP_CORE_MAX_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_VMON_HSP_CORE_MIN_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_VMON_AEMC_CORE_MAX_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_VMON_AEMC_CORE_MIN_OK_MASK_FIELD_MASK;
#endif

#if MANTICORE_ENABLE_EMC_TMON
	/* Enable errors based on TMON trips. */
	power_down |= CREG_REGS_EMC_CONTROL_POWER_DOWN_REG_TMON_FIELD_MASK;
	status_mask |= CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_TMON_MAX_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_TMON_MIN_OK_MASK_FIELD_MASK;
#endif

#if MANTICORE_ENABLE_EMC_CMON
	/* Enable errors based on CMON trips. */
	power_down |= CREG_REGS_EMC_CONTROL_POWER_DOWN_REG_CMON_CPLCLK_FIELD_MASK;
	status_mask |=
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_GLITCH_CHECK_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_LOTIME_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_HITIME_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_FALLING_PERIOD_MAX_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_FALLING_PERIOD_MIN_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_RISING_PERIOD_MAX_OK_MASK_FIELD_MASK |
		CREG_REGS_EMC_CONTROL_STATUS_MASK_REG_CMON_CPLCLK_RISING_PERIOD_MIN_OK_MASK_FIELD_MASK;
#endif

	/* Power the EMC monitors and lock them so they can't be disabled. */
	emc_ctrl->power_down_reg &= ~power_down;
	emc_ctrl->power_down_wlock_reg |= power_down;

	/* Unmask EMC errors and lock them so they can't be masked again. */
	emc_ctrl->status_mask_reg &= ~status_mask;
	emc_ctrl->status_mask_wlock_reg |= status_mask;

	/* Lock all EMC calibration so that it can't be changed at run-time. */
	emc_ctrl->cal_reg_wlock1 = CREG_REGS_EMC_CONTROL_CAL_REG_WLOCK1_WRITE_MASK;
	emc_ctrl->cal_reg_wlock2 = CREG_REGS_EMC_CONTROL_CAL_REG_WLOCK2_WRITE_MASK;
}

/**
 * Generate the 1SP version string based on the build version in the ROM log.
 */
static void initialize_1sp_version_str ()
{
	/* 1SP is secure if there is an owner key.  Tenant boots are considered secure. */
	secure_boot = has_owner_key ();
	if (secure_boot) {
		MANTICORE_1SP_SET_SECURE_BOOT (&sp1_shared);
	}
	else {
		MANTICORE_1SP_SET_UNSECURE_BOOT (&sp1_shared);
	}

	/* 1SP is FIPS certified if it's dual-signed. */
	sp1_shared.fips_certifed_1sp = has_secondary_signing_key ();

	build_version_to_string (rom_shared->firmware.pcr_log[1].fw_version.data.build_version,
		secure_boot, sp1_shared.fips_certifed_1sp, sp1_shared.version_1sp,
		sizeof (sp1_shared.version_1sp));
}

/**
 * Context for the fuse controller driver.
 */
static struct fuse_controller_state fuse_context;

/**
 * Driver for the HSP fuse controller.
 */
#ifndef MANTICORE_ENABLE_FIPS_CMVP_TESTING
static const struct fuse_controller fuses =
	fuse_controller_manticore_fips_static_init (&fuse_context,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS);
#else
static const struct fuse_controller fuses =
	fuse_controller_manticore_fips_cmvp_static_init (&fuse_context,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS);
#endif

/**
 * Context for the RNG driver.
 */
static struct hsp_rng_hw_state rng_context;

/**
 * Driver for the HSP random number generator.
 */
static const struct hsp_rng_hw rng_hw = hsp_rng_hw_static_init (&rng_context,
	(struct Rng_regs*) HSP_ADDR_MAP_RNG_ADDRESS, &fuses.base, MANTICORE_ROM_MIN_RNG_CLOCK_DIVIDER,
	MANTICORE_ROM_MAX_RNG_CLOCK_DIVIDER);

/**
 * API wrapper for accessing the HSP RNG.
 */
static const struct rng_engine_hsp rng = rng_hsp_static_init (&rng_hw);

/**
 * Variable context for the HS-SHA driver.
 */
static struct hs_sha_state hash_hw_context;

/**
 * Driver for the HS-SHA.
 */
static const struct hs_sha hash_hw = hs_sha_static_init_polling (&hash_hw_context,
	(struct Sha_regs*) HSP_ADDR_MAP_SHA_ADDRESS, &rom_shared_static->internal.hs_sha.cmd,
	rom_shared_static->internal.hs_sha.data, MANTICORE_ROM_HS_SHA_BUFFER_SIZE);

/**
 * Variable context for the hash API.
 */
static struct hash_engine_hs_sha_state hash_context;

/**
 * Hash engine wrapper for the HS-SHA driver.
 */
const struct hash_engine_hs_sha hash = hash_hs_sha_static_init (&hash_context, &hash_hw);

/**
 * Variable context for the PKA driver.
 */
static struct ecc_hw_pka_state pka_context;

/**
 * Driver for the PKA.
 */
const struct ecc_hw_pka pka = ecc_hw_pka_static_init_polling (&pka_context,
	(struct Pka_regs*) HSP_ADDR_MAP_PKA_ADDRESS, &rng_hw, &rom_shared_static->internal.pka);

/**
 * ECC engine wrapper or the PKA driver.
 */
static const struct ecc_engine_ecc_hw ecc = ecc_ecc_hw_static_init (&pka.base, NULL);

/**
 * Variable context for the AES driver.
 */
static struct hsp_aes_state aes_context;

/**
 * Driver for the HW AES engine.
 */
static const struct hsp_aes aes = hsp_aes_static_init_polling (&aes_context,
	(struct Aes_regs*) HSP_ADDR_MAP_AES_ADDRESS, &rom_shared_static->internal.aes.cmd,
	rom_shared_static->internal.aes.data, MANTICORE_ROM_HS_SHA_BUFFER_SIZE,
	(struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS, CCS_KSU_STATIC_NUM_KEYS);

/**
 * Variable context for the CCS driver.
 */
static struct ccs_ksu_state ccs_context;

/**
 * Driver for the hardware CCS and KSU.
 */
static const struct ccs_ksu ccs = ccs_ksu_static_init_polling (&ccs_context,
	(struct Ccs_regs*) HSP_ADDR_MAP_CCS_ADDRESS, &hash_hw, &aes, &pka, &rng_hw,
	&rom_shared_static->internal.ccs.cmd, (struct ksu_key_slot*) HSP_ADDR_MAP_KSB_KEYS_ADDRESS,
	CCS_KSU_STATIC_NUM_KEYS, (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS,
	CCS_KSU_STATIC_NUM_PCRS);

/**
 * The maximum number of ECC keys that can be managed by the FIPS compliant CCS.
 */
#define	MANTICORE_FIPS_FW_KSU_SLOTS		3

/**
 * Key slots for the firmware KSU used by the FIPS compliant CCS.
 */
struct ccs_ksu_fips_key ccs_fips_keys[MANTICORE_FIPS_FW_KSU_SLOTS];

/**
 * KSU for the FIPS compliant CCS.
 */
static const struct ccs_ksu_fips_ksu ccs_fips_ksu =
	ccs_ksu_fips_ksu_static_init (ccs_fips_keys, ARRAY_SIZE (ccs_fips_keys));

/**
 * Variable context for the FIPS compliant CCS driver.
 */
static struct ccs_ksu_fips_state ccs_fips_context;

/**
 * Driver for the FIPS compliant CCS and KSU.
 */
static const struct ccs_ksu_fips ccs_fips = ccs_ksu_fips_static_init (&ccs_fips_context,
	&ccs_fips_ksu, &ccs.base, &pka.base, &hash.base);

/**
 * KDF context to use for converting FIPS unapproved keys to approved ones.  This is the SHA384 hash
 * of "FIPS".
 */
static const SP_MSG_384 MANTICORE_1SP_FIPS_KDF_CONTEXT = {
	.AsBytes = {
		0x8f, 0xb7, 0xd8, 0xe5, 0x45, 0x00, 0xfa, 0x16,
		0x5a, 0xc0, 0x89, 0x59, 0x94, 0xb0, 0xff, 0xf7,
		0x0d, 0x30, 0xf3, 0x9c, 0xe4, 0x91, 0x3c, 0x32,
		0x2b, 0xc9, 0x0a, 0xc5, 0x67, 0x10, 0xeb, 0x07,
		0x16, 0x68, 0x9f, 0x09, 0x4b, 0x24, 0xec, 0x64,
		0x71, 0x49, 0x9e, 0x40, 0xd5, 0x6a, 0x96, 0xc3
	}
};

/**
 * KDF key to use for converting FIPS unapproved keys to approved ones.  Since the source keys are
 * unapproved, a different key needs to be used in the KDF.  In this KDF, while the key is fixed,
 * the context comes from the existing key and is device-specific.
 */
static const SP_MSG_384 MANTICORE_1SP_FIPS_KDF_KEY = {
	.AsBytes = {
		0x0a, 0x4f, 0xb2, 0xf9, 0xaf, 0xd5, 0xda, 0xf1,
		0xd3, 0xde, 0x21, 0x3a, 0x53, 0x6d, 0x78, 0x96,
		0x3e, 0xc4, 0xc5, 0xb5, 0x57, 0x61, 0xdd, 0xa5,
		0xda, 0x1b, 0xdb, 0xfd, 0x3c, 0x9e, 0x8c, 0x09,
		0xd3, 0x0d, 0x72, 0xb1, 0x60, 0x9c, 0x5e, 0x56,
		0xce, 0xc0, 0x01, 0x7d, 0x38, 0xdf, 0x64, 0x8e
	}
};

/**
 * KDF context to use for deriving the Firmware HMAC Key.  This is the SHA384 hash of
 * "FirmwareHMACKey".
 */
static const SP_MSG_384 MANTICORE_FIRMWARE_HMAC_KDF_CONTEXT = {
	.AsBytes = {
		0xb4, 0x6d, 0xea, 0x03, 0x92, 0x2e, 0x5f, 0x9e,
		0xf9, 0x06, 0xdd, 0x82, 0x86, 0x35, 0xfb, 0x27,
		0x14, 0x01, 0x19, 0xb6, 0x49, 0xdf, 0x45, 0xcc,
		0x85, 0x28, 0xbf, 0xc1, 0x1c, 0x72, 0xd2, 0xea,
		0xb9, 0x90, 0x8a, 0xd8, 0x70, 0xe3, 0x55, 0x25,
		0xf5, 0x4b, 0xcf, 0x43, 0xb5, 0x8d, 0x0e, 0x40
	}
};

/**
 * KDF context to use for deriving the SPRT Device Key.  This is the SHA384 hash of "SPRTDeviceKey".
 */
static const SP_MSG_384 MANTICORE_SPRT_DEVICE_KEY_KDF_CONTEXT = {
	.AsBytes = {
		0xb6, 0xfb, 0x2e, 0x1e, 0xc2, 0xea, 0xe6, 0x41,
		0xc3, 0x78, 0x20, 0xe3, 0x0a, 0x0d, 0x1d, 0xb9,
		0x49, 0xe4, 0xd1, 0xec, 0xaa, 0x63, 0xc8, 0x3d,
		0xa4, 0xb8, 0xe8, 0x50, 0x56, 0xb0, 0x8c, 0xfa,
		0xc2, 0xfe, 0x60, 0xa2, 0x96, 0xc2, 0xaf, 0x72,
		0x85, 0xcc, 0x77, 0x28, 0x13, 0x37, 0x92, 0x3a
	}
};

/**
 * KDF context to use for destroying production keys on an unlocked device.  This is the SHA384 hash
 * of "Unlocked".
 */
static const SP_MSG_384 MANTICORE_UNLOCKED_DEVICE_KDF_CONTEXT = {
	.AsBytes = {
		0x35, 0x0f, 0xcd, 0x06, 0x66, 0x63, 0x32, 0x49,
		0xb4, 0x49, 0xe1, 0x9b, 0x1f, 0x10, 0x33, 0x0d,
		0xb5, 0x82, 0x5f, 0xcc, 0x35, 0x22, 0x86, 0x0e,
		0x20, 0xeb, 0x8f, 0x94, 0xfe, 0x99, 0x39, 0xe5,
		0xe7, 0x42, 0x88, 0x65, 0xd4, 0x4e, 0xcf, 0x83,
		0xbf, 0x28, 0xbf, 0x92, 0x97, 0xdd, 0xd1, 0x4a
	}
};

/**
 * KDF context to use for deriving production keys for FIPS non-certified firmware.  This is the
 * SHA384 hash of "NonFIPS".
 */
static const SP_MSG_384 MANTICORE_NON_FIPS_DEVICE_KDF_CONTEXT = {
	.AsBytes = {
		0x02, 0x73, 0xf0, 0x28, 0xbb, 0x04, 0x72, 0x10,
		0x8c, 0x47, 0x0b, 0xbc, 0x0c, 0x1f, 0x56, 0x26,
		0x7a, 0x66, 0xcb, 0x1f, 0x07, 0x84, 0xca, 0x4f,
		0x1c, 0x76, 0x1c, 0x99, 0x21, 0x56, 0xb9, 0x29,
		0xe0, 0x7e, 0x12, 0x9e, 0xcb, 0x78, 0x78, 0x4c,
		0x0d, 0x72, 0x39, 0x08, 0xb4, 0x94, 0x60, 0x2c
	}
};

/**
 * KDF context to use for deriving a different BKS Key to use in FIPS approved mode.  This is the
 * SHA384 hash of "FIPSBKS".
 */
static const SP_MSG_384 MANTICORE_FIPS_BKS_KDF_CONTEXT = {
	.AsBytes = {
		0xa9, 0xd9, 0x8a, 0x40, 0x9f, 0xd7, 0x8c, 0x0b,
		0xc9, 0xc2, 0xe7, 0x15, 0x77, 0x20, 0x8a, 0xce,
		0x71, 0x58, 0x88, 0x17, 0x2f, 0x40, 0xa8, 0xf6,
		0xbf, 0x9c, 0x41, 0x1b, 0x15, 0x5f, 0x00, 0x7d,
		0x5a, 0xd7, 0x40, 0xce, 0xcf, 0x15, 0x59, 0x23,
		0x63, 0xcb, 0xbb, 0x9c, 0x6d, 0x43, 0x0c, 0xea
	}
};

/**
 * KDF context to use for deriving the BKS1 Key.  This is the SHA384 hash of "BKS1Key".
 */
static const SP_MSG_384 MANTICORE_BKS1_KDF_CONTEXT = {
	.AsBytes = {
		0xb8, 0x88, 0xbf, 0x7b, 0x45, 0x50, 0x6e, 0xab,
		0x0e, 0xfb, 0xa2, 0x2d, 0xa3, 0xc7, 0x49, 0xc7,
		0x3b, 0x7e, 0x56, 0xbf, 0xba, 0x6d, 0x07, 0xcb,
		0x10, 0x60, 0x4c, 0x84, 0xa3, 0x69, 0x7b, 0xc0,
		0xc6, 0x55, 0x41, 0x7f, 0xa0, 0x3d, 0x5f, 0x05,
		0x8c, 0xc8, 0x1e, 0x15, 0x17, 0x26, 0xa8, 0x9b
	}
};

/**
 * KDF context to use for deriving the BKS2 Key.  This is the SHA384 hash of "BKS2Key".
 */
static const SP_MSG_384 MANTICORE_BKS2_KDF_CONTEXT = {
	.AsBytes = {
		0xed, 0x7d, 0x21, 0xa0, 0x17, 0x0c, 0x3c, 0xbd,
		0x3c, 0x70, 0x2d, 0xaf, 0xa8, 0x8e, 0x46, 0x33,
		0x65, 0x66, 0xf1, 0xc9, 0xca, 0xa0, 0x79, 0x5c,
		0x2f, 0x1f, 0xf4, 0xbc, 0xb2, 0x79, 0x36, 0x0f,
		0x6b, 0x94, 0x96, 0x15, 0x46, 0x31, 0x99, 0x7a,
		0x93, 0x42, 0xdb, 0xc0, 0x3c, 0x56, 0x75, 0xac
	}
};

/**
 * KDF context to use for deriving the SPRT Owner Global Key.  This is the SHA384 hash of
 * "SPRTOwnerGlobalKey".
 */
static const SP_MSG_384 MANTICORE_SPRT_OWNER_GLOBAL_KEY_KDF_CONTEXT = {
	.AsBytes = {
		0x1e, 0xe1, 0x07, 0x82, 0x41, 0x4f, 0xb6, 0x52,
		0xf7, 0xa9, 0x49, 0xb8, 0x33, 0x95, 0xa2, 0x9a,
		0x44, 0xe4, 0x8e, 0x5b, 0x88, 0x57, 0x01, 0xb0,
		0x6b, 0x78, 0x5d, 0x4b, 0xfa, 0x8b, 0x8c, 0x1e,
		0xea, 0x98, 0xc2, 0xd9, 0xb7, 0xcd, 0xef, 0xf3,
		0x5d, 0x98, 0xc3, 0xb8, 0xd1, 0x39, 0x17, 0xf0
	}
};

/**
 * KDF context to use for deriving FMC CDI seed for HSM use.  This is the SHA384 hash of
 * "HSMFMCCDI".
 */
static const SP_MSG_384 MANTICORE_HSM_FMC_CDI_KDF_CONTEXT = {
	.AsBytes = {
		0xdf, 0x9e, 0x7f, 0xee, 0xed, 0x41, 0xe8, 0xc4,
		0x83, 0x14, 0x12, 0xb4, 0x29, 0x36, 0x46, 0xc0,
		0x6f, 0x25, 0x38, 0xbb, 0x7e, 0xc5, 0x0f, 0x7f,
		0xe0, 0xf3, 0xa1, 0xcb, 0xf1, 0xf0, 0xe3, 0x1c,
		0x1b, 0x21, 0x8a, 0xa6, 0x61, 0x29, 0x81, 0x5c,
		0xcf, 0x6f, 0xfa, 0xba, 0x61, 0x64, 0x9e, 0x68
	}
};

/**
 * KDF contexts to use for deriving 1SP keys.
 */
static const struct manticore_device_keys_1sp_kdf kdf_context_1sp_keys = {
	.firmware_hmac = &MANTICORE_FIRMWARE_HMAC_KDF_CONTEXT,
	.sprt_device_key = &MANTICORE_SPRT_DEVICE_KEY_KDF_CONTEXT,
	.fips_unlock = &MANTICORE_1SP_FIPS_KDF_CONTEXT,
	.non_fips_unlock = &MANTICORE_NON_FIPS_DEVICE_KDF_CONTEXT,
};


/**
 * Initialize the HW random number generator.
 *
 * @return 0 if the RNG was successfully initialized or an error code.
 */
static int initialize_rng ()
{
	int status;

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	status = get_cmvp_test_case (CMVP_TEST_CASE_BOOT_STAGE_1SP);
	if (status != 0) {
		return status;
	}
#endif

	status = fuse_controller_init_state (&fuses);
	if (status != 0) {
		return status;
	}

	/* Recalibrate the RNG to use the FIPS approved settings. */
	status = hsp_rng_hw_init_state (&rng_hw, true);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize the HW crypto engines.
 *
 * @return 0 if the crypto engines were successfully initialized or an error code.
 */
static int initialize_crypto ()
{
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CRYPTO_INIT_START);

	status = hs_sha_init_state (&hash_hw);
	if (status != 0) {
		return status;
	}

	status = hash_hs_sha_init_state (&hash);
	if (status != 0) {
		return status;
	}

	status = ecc_hw_pka_init_state (&pka);
	if (status != 0) {
		return status;
	}

	status = hsp_aes_init_state (&aes);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_init_state (&ccs);
	if (status != 0) {
		return status;
	}

	status = ccs_ksu_fips_init_state (&ccs_fips, true);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CRYPTO_INIT_END ^ status);

	return 0;
}

/**
 * Execute self-tests on the crypto components.
 *
 * @return 0 if the self-tests have been executed successfully or an error code.
 */
static int run_crypto_self_tests ()
{
	int status;

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;
	uint8_t *corrupt;

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		case CMVP_TEST_CASE_ALGORITHM_DRBG_INSTANTIATE_HW:
			corrupt = (uint8_t*) &RNG_HSP_HW_KAT_INSTANTIATE_INPUT.AsBytes[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_DRBG_RESEED_HW:
			corrupt = (uint8_t*) &RNG_HSP_HW_KAT_RESEED_INPUT.AsBytes[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_KBKDF_HW:
			corrupt = (uint8_t*) &CCS_KSU_KAT_VECTORS_HMAC_SHA384_KEY.AsBytes[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_PKA:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_ECC_PRIVATE[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_PKA:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_SHA384_ECDSA_SIGNATURE.r[16];
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_SW:
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_P384_SHA384_ECDSA_SIGNATURE_DER[16];
			break;

		default:
			corrupt = NULL;
			break;
	}

	/* ECDSA verification using PKA can't be corrupted here because it would cause the the sign
	 * self-test to fail first.  The corruption needs to be delayed until after the sign self-test
	 * has been executed. */
	if ((corrupt != NULL) && (algo != CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_PKA)) {
		*corrupt ^= 0xff;
		corrupt = NULL;
	}

	/* Configure the firmware to trigger a PCT failure, if requested. */
	trigger_cmvp_pct_failure ();
#endif

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_START);

	/* DRBG self-test needs to be re-run since the ROM execution does not test Reseed. */
	status = hsp_rng_hw_kat_run_self_test (&rng_hw);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_DRBG ^ status);

	/* KDF relies on HMAC working correctly, but HW HMAC has a valid self-test in ROM so doesn't
	 * need to be re-run. */
	status = ccs_ksu_kat_run_self_test_kdf384 (&ccs_fips.base, MANTICORE_DEVICE_KEYS_SELF_TEST);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_KDF ^ status);

	/* 1SP uses 2 different ECDSA implementations.  Each needs to be self-tested.  The only ECDSA
	 * signing operations that happen in 1SP are for DICE certificates and PHY firmware, which both
	 * use the PKA ECDSA implementation.  As such, that is the only implementation that needs a
	 * self-test for signing. */
	status = ecdsa_kat_run_self_test_ecc_hw_sign_p384_sha384 (&pka.base, &hash.base);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_ECDSA_PKA_SIGN ^ status);

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	/* Corrupt the PKA verification test, if necessary. */
	if (corrupt != NULL) {
		*corrupt ^= 0xff;
	}
#endif

	status = ecdsa_kat_run_self_test_ecc_hw_verify_p384_sha384 (&pka.base, &hash.base);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_ECDSA_PKA_VERIFY ^ status);

	status = ecdsa_kat_run_self_test_verify_p384_sha384 (&ecc.base, &hash.base);
	if (status != 0) {
		return status;
	}

	/* AES-CBC decryption that could be used during firmware loading does not need a self-test here
	 * since a valid self test for this mode is already run in ROM. */

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_ECDSA_ECC_VERIFY ^ status);

	return 0;
}

/**
 * Frequency to use when communicating with the SPI flash devices.
 */
#ifdef BUILD_FOR_SIMULATION
/* Increase the clock frequency to reduce simulation time. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				25000000
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	25000000

#elif defined BUILD_FOR_FPGA
/* The MPS3 FPGA clock is much slower, so reduce the SPI clock, too.  Use the same divider value as
 * the production scenario. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				555556
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	555556

#elif defined BUILD_FOR_HAPS
/* The HAPS FPGA runs even slower than MPS3. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				83334
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	83334

#else
/* The clock frequency used on the actual device. */
#define	MANTICORE_FLASH_CLOCK_FREQUENCY_HZ				46875000

/**
 * Account for the slower system clock when A0 Bypass is asserted by running the flash slower.
 */
#define	MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ	833334
#endif

/**
 * Flash capabilities for the internal flash device.
 */
#define	MANTICORE_INTERNAL_FLASH_CAPABILITIES           \
	(FLASH_CAP_DUAL_1_1_2 | FLASH_CAP_QUAD_1_1_4 | FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)

/**
 * Flash capabilities for the external flash device.
 *
 * TODO: Explore the reasons why 1-2-2 and 1-4-4 modes don't work against the external Winbond
 * flash.  Lack of PU on the I/O lines during mode byte?
 */
#define	MANTICORE_EXTERNAL_FLASH_CAPABILITIES           \
	(FLASH_CAP_DUAL_1_1_2 | FLASH_CAP_QUAD_1_1_4 | FLASH_CAP_3BYTE_ADDR | FLASH_CAP_4BYTE_ADDR)

/**
 * Variable context for the SPI master driver.
 */
static struct spi_dwc_ssi_state spi_master_context;

/**
 * Driver for the SPI master connected to flash devices that can be used to store 1SP firmware.
 */
static const struct spi_dwc_ssi spi_master = spi_dwc_ssi_static_init_polling (&spi_master_context,
	(struct DWC_ssi_AHB_Slave*) HSP_ADDR_MAP_CREG_SPI_SPI0_ADDRESS,
	(struct Ssi_regs*) HSP_ADDR_MAP_CREG_SPI_CREG_SSI_GROUP0_ADDRESS);

/**
 * Variable context for the flash connected to SPI0.
 */
static struct flash_master_dwc_ssi_state spi0_context[2];

/**
 * Device interface to the flash connected to SPI.
 */
static const struct flash_master_dwc_ssi spi0[2] = {
	flash_master_dwc_ssi_static_init_with_capabilities (&spi0_context[0], &spi_master, 0,
		MANTICORE_INTERNAL_FLASH_CAPABILITIES),
	flash_master_dwc_ssi_static_init_with_capabilities (&spi0_context[1], &spi_master, 1,
		MANTICORE_EXTERNAL_FLASH_CAPABILITIES)
};

/**
 * Variable context for the internal Manticore SPI flash device.
 */
static struct spi_flash_state flash_internal_context;

/**
 * Driver to communicate with internal Manticore SPI flash.
 */
static const struct spi_flash flash_internal = spi_flash_static_init (SPI_FLASH_API_INIT,
	&flash_internal_context, &spi0[0].base);

/**
 * Variable context for the external Manticore SPI flash device.
 */
static struct spi_flash_state flash_external_context;

/**
 * Driver to communicate with external Manticore SPI flash.
 */
static const struct spi_flash flash_external = spi_flash_static_init (SPI_FLASH_API_INIT,
	&flash_external_context, &spi0[1].base);

/**
 * Real time clock instance for logging.
 */
static const struct real_time_clock_hsp system_rtc =
	real_time_clock_hsp_static_init_no_set_time (
	(struct Creg_regs_creg_rtc_group*) HSP_ADDR_MAP_CREG_RTC_REGS_ADDRESS, 5000);

const struct real_time_clock *const debug_timestamp = &system_rtc.base;

/**
 * Variable context for the ROM log parser.
 */
static struct logging_memory_state rom_log_context;

/**
 * Parser for the ROM error log.
 */
static const struct logging_memory rom_log = logging_memory_static_init (&rom_log_context,
	rom_shared_static->firmware.log_buffer, MANTICORE_ROM_LOG_BUFFER_SIZE,
	sizeof (struct debug_log_entry_info));

/**
 * Variable context for the debug log.
 */
static struct logging_flash_state debug_log_context;

/**
 * Flash logger for storing the debug log.
 */
static const struct logging_flash debug_logger = logging_flash_static_init (&debug_log_context,
	&flash_external, DEBUG_LOGGING_ADDR);


/**
 * Initialize the interfaces to Manticore SPI flash, connected to SPI0.  CS0 is the in-package flash
 * device and CS1 is external.
 *
 * @return 0 if the flash was successfully initialized or an error code.
 */
static int initialize_manticore_flash ()
{
	uint8_t vendor;
	uint16_t device;
	uint32_t bytes;
	int status;

	status = spi_dwc_ssi_init_state (&spi_master, HSP_CLOCK_FREQUENCY_HZ, (is_a0_bypass ()) ?
			MANTICORE_A0_BYPASS_FLASH_CLOCK_FREQUENCY_HZ : MANTICORE_FLASH_CLOCK_FREQUENCY_HZ);
	if (status != 0) {
		return status;
	}

	status = flash_master_dwc_ssi_init_state (&spi0[0]);
	if (status != 0) {
		return status;
	}

	status = flash_master_dwc_ssi_init_state (&spi0[1]);
	if (status != 0) {
		return status;
	}

	status = spi_flash_initialize_device_state (&flash_internal, true, false, SPI_FLASH_RESET_NONE,
		false);
	if (status != 0) {
		return status;
	}

	status = spi_flash_get_device_id (&flash_internal, &vendor, &device);
	spi_flash_get_device_size (&flash_internal, &bytes);
	platform_printf ("Internal flash device: status=0x%x, vendor=0x%x, device=0x%x, bytes=0x%x"
		NEWLINE, status, vendor, device, bytes);
	platform_printf ("Internal Read: cmd=0x%x, dummy=%d, mode=%d, flags=0x%x" NEWLINE,
		flash_internal.state->command.read, flash_internal.state->command.read_dummy,
		flash_internal.state->command.read_mode, flash_internal.state->command.read_flags);

	status = spi_flash_initialize_device_state (&flash_external, true, false, SPI_FLASH_RESET_NONE,
		false);
	if (status != 0) {
		return status;
	}

	status = spi_flash_get_device_id (&flash_external, &vendor, &device);
	spi_flash_get_device_size (&flash_external, &bytes);
	platform_printf ("External flash device: status=0x%x, vendor=0x%x, device=0x%x, bytes=0x%x"
		NEWLINE, status, vendor, device, bytes);
	platform_printf ("External Read: cmd=0x%x, dummy=%d, mode=%d, flags=0x%x" NEWLINE,
		flash_external.state->command.read, flash_external.state->command.read_dummy,
		flash_external.state->command.read_mode, flash_external.state->command.read_flags);

	return 0;
}

/**
 * Initialize the run-time debug log and import any ROM log messages.
 */
static void initialize_log ()
{
	int status;

	status = logging_flash_init_state (&debug_logger);
	if (status == 0) {
		/* TODO:  Put this pointer in a memory location that will be marked RO in the MPU. */
		debug_log = &debug_logger.base;

		/* If log initialization is successful, import the ROM log entries and clear the log
		 * buffer.  This is a best effort migration.  ROM entries could get lost on failure. */
		status = logging_memory_init_state_append_existing (&rom_log);
		if (status == 0) {
			status = logging_memory_copy_entries (&rom_log, &debug_logger.base);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_BOOT,
					BOOT_LOGGING_ROM_LOG_ERROR, status, 0);
			}

			debug_log_flush ();
			memset (rom_shared->firmware.log_buffer, 0, MANTICORE_ROM_LOG_BUFFER_SIZE);
		}
	}
	else {
		platform_printf ("1SP: Failed to initialize debug log: 0x%x" NEWLINE, status);
	}
}

/**
 * Security policy handler for the device.
 */
static const struct security_policy_hsp_manticore sec_policy =
	security_policy_hsp_manticore_static_init (&sp1_shared.sec_policy, ueid);

/**
 * Handler for configuring memory protections for 1SP execution and SoC fences.
 */
static const struct memory_protection_manticore_1sp sp1_mem_protect =
	memory_protection_manticore_1sp_static_init (&mpu.base, &_exe_start, &_ro_start, &_start_data,
	&_memory_end);

/**
 * Variable context for the flash that contains the unlock policy.
 */
static struct flash_store_contiguous_blocks_state unlock_flash_context;

/**
 * Flash block storage for the unlock policy.
 */
static const struct flash_store_contiguous_blocks unlock_flash =
	flash_store_contiguous_blocks_static_init_variable_storage_decreasing (
	FLASH_STORE_CONTIGUOUS_BLOCKS_WITH_HASH_API_INIT, &unlock_flash_context, MAIN_KEYSTORE_ADDR,
	MAIN_KEYSTORE_MAX_KEYS, &flash_internal.base, &hash.base);

/**
 * Buffer to use when loading unlock policies from flash.
 */
static uint8_t unlock_buffer[MANTICORE_UNLOCK_HMAC_BUFFER_SIZE];

/**
 * Variable context for the device security manager.
 */
static struct security_manager_hsp_manticore_state sec_manager_context;

/**
 * Security manager for applying the appropriate device security configuration.
 */
static const struct security_manager_hsp_manticore sec_manager =
	security_manager_hsp_manticore_static_init_only_apply_unlock (&sec_manager_context,
	&sec_policy.base, (const uint8_t*) &locked_device_policy, (const uint8_t*) &mfg_device_policy,
	sizeof (locked_device_policy), &aeb, &fuses.base, HSP_FUSES_ADDRESS (AEB),
	HSP_FUSES_ADDRESS (RSVD1), MANTICORE_1SP_UNLOCK_COUNTER_LENGTH, &sp1_mem_protect.base,
	&hash.base, &ccs_fips.base, DEVICE_KEYS_DICE_CDI, MANTICORE_DEVICE_KEYS_FIPS_DEVICE_ID_KEY,
	MANTICORE_DEVICE_KEYS_UNLOCK_HMAC_KEY, unlock_buffer, sizeof (unlock_buffer),
	&unlock_flash.base, DEVICE_UNLOCK_POLICY);

/**
 * PMP configuration to lock write access to the 1SP boot status sticky register.
 */
static const RiscvPmpConfig boot_status_pmp = {
	.R = 1,
	.W = 0,
	.X = 0,
	.M = RiscvPmpNapot,
	.L = RiscvPmpLocked
};

/* Global pointer to the default security policy instance. */
const struct security_policy *const default_policy = &sec_policy.base.base;


/**
 * Initialize the device security policy for boot and run-time configuration.
 *
 * @return 0 if the security policy was successfully initialized.
 */
static int initialize_security_policy ()
{
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SEC_POLICY_INIT_START);

	status = flash_store_contiguous_blocks_init_state (&unlock_flash, 0);
	if (status != 0) {
		return status;
	}

	status = security_manager_hsp_manticore_init_state (&sec_manager, is_por ());
	if (status != 0) {
		return status;
	}

	/* If there is no owner key, as may be the case with A0 devices, the HMAC key doesn't exist, so
	 * unlock flows cannot be supported.  A0 devices will need to be provisioned with an owner key
	 * to ensure correct behavior.  The owner key can be a dev key, though. */
	platform_printf ("Load Security Policy" NEWLINE);

	status = manticore_device_keys_derive_1sp_keys (&ccs_fips.base, &sec_manager.base,
		&kdf_context_1sp_keys, sp1_shared.fips_certifed_1sp);
	if (status != 0) {
		return status;
	}

	status = sec_manager.base.base.load_security_policy (&sec_manager.base.base);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SEC_POLICY_LOADED ^ status);

	/* Determine if the device has been unlocked.  When an unlock policy has been applied, mark the
	 * boot as not secure.  This is done irrespective of the actual policy being applied. It's
	 * enough to know that the device has been unlocked in some way. */
	status = sec_manager.base.base.has_unlock_policy (&sec_manager.base.base);
	switch (status) {
		case 0:	/* No unlock policy */
			if (has_booted_unlocked ()) {
				/* There is no unlock policy currently loaded, but there has not been a SoC reset
				 * since the last time the device was unlocked.  Continue to generate untrusted
				 * keys.  This also means the device cannot get unlocked again in this boot
				 * context. */
				status = security_manager_hsp_derive_unlocked_device_keys (&sec_manager.base);
				if (status != 0) {
					return status;
				}
			}
			break;

		case 1:	/* Unlock policy */
			secure_boot = false;
			MANTICORE_1SP_SET_UNSECURE_BOOT (&sp1_shared);
			status = 0;
			break;

		default:	/* Error */
			return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_UNLOCKED_KEYS ^ status);

	if (!secure_boot || has_booted_unlocked ()) {
		status = manticore_device_keys_derive_unlocked_1sp_keys (&ccs_fips.base,
			&MANTICORE_UNLOCKED_DEVICE_KDF_CONTEXT);
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SEC_POLICY_INIT_END ^ status);

	return status;
}

/**
 * Apply any device hardware configuration that is required by the active security policy.
 *
 * @return 0 if the security configuration was applied on an error code.
 */
static int apply_security_configuration ()
{
	int status;

	if (!secure_boot) {
		/* Mark the device as having boot with an unlock policy.  This state is sticky until the
		 * next SoC reset. */
		boot_unlocked_device ();
	}

	/* This will block access to both SW_STICKY_RW_31 and INIT_CTRL registers, which are contiguous
	 * in the CREG register space.  SW_STICKY_RW_31 is used for tracking 1SP boot status and
	 * INIT_CTRL can be used to reset PCR0 and PCR2 without a SoC reset.  Blocking both of these
	 * registers means SPRT cannot misuse them. */
	RiscvPmpSetRegion (0, boot_status_pmp,
		NAPOT_ADDRESS ((uintptr_t) MANTICORE_STICKY_REG (MANTICORE_1SP_BOOT_STATUS),
		(sizeof (uint32_t) * 2)));

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_APPLY_SECURITY_CFG);

	/* Wipe keys that should only be available to 1SP. */
	status = manticore_device_keys_clear_1sp_keys (&ccs_fips.base);
	if (status != 0) {
		return status;
	}

	/* Allow access to the AEB fuses so AEMC AEB fuses can be checked/updated. */
	status = aeb.enable_aeb (&aeb, MANTICORE_AEB_FCTRL_ENABLE_ACCESS_AEB_FUSES);
	if (status != 0) {
		return status;
	}

	status = sec_manager.base.base.apply_device_config (&sec_manager.base.base);

	code_path_integrity_secure_message_no_trace (0);	// Need an even number of messages.
	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_APPLY_SECURITY_CFG_DONE);

	return status;
}

/**
 * Regions of HSP SRAM that can be used to load the SPRT image.
 */
static const struct sram_block available_sram[] = {
	{
		.start = (void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS,
		.length = HSP_ADDR_MAP_SP_IRAM_SIZE
	}
	/* TODO: Place the 1SP at the end of DRAM to open up more regions for loading SPRT data. */
	// {
	// 	.start = (void*) HSP_ADDR_MAP_SP_DRAM_ADDRESS,
	// 	.length = (uintptr_t) &_start_data - HSP_ADDR_MAP_SP_DRAM_ADDRESS
	// }
};

/**
 * Regions of SoC SRAM that can be used to load the CP image.  These addresses come from the global
 * SoC address map.
 */
static const struct soc_sram_block available_cp_sram[] = {
	{
		.start = MANTICORE_SOC_CP_ITCM_ADDRESS,
		.length = MANTICORE_SOC_CP_ITCM_SIZE
	},
	{
		.start = MANTICORE_SOC_CP0_DTCM_ADDRESS,
		.length = MANTICORE_SOC_CP0_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_CP1_DTCM_ADDRESS,
		.length = MANTICORE_SOC_CP1_DTCM_SIZE
	}
};

/**
 * Regions of SoC SRAM that can be used to load the FP core 0 image.  These addresses come from the
 * global SoC address map.
 */
static const struct soc_sram_block available_fp0_sram[] = {
	{
		.start = MANTICORE_SOC_FP0_ITCM_ADDRESS,
		.length = MANTICORE_SOC_FP0_ITCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP0_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP0_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP0_FP1_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP0_FP1_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP0_FP2_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP0_FP2_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_PSRAM_ADDRESS,
		.length = MANTICORE_SOC_PSRAM_SIZE
	}
};

/**
 * Regions of SoC SRAM that can be used to load the FP core 1 image.  These addresses come from the
 * global SoC address map.
 */
static const struct soc_sram_block available_fp1_sram[] = {
	{
		.start = MANTICORE_SOC_FP1_ITCM_ADDRESS,
		.length = MANTICORE_SOC_FP1_ITCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP1_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP1_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP1_FP0_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP1_FP0_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP1_FP2_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP1_FP2_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_PSRAM_ADDRESS,
		.length = MANTICORE_SOC_PSRAM_SIZE
	}
};

/**
 * Regions of SoC SRAM that can be used to load the FP core 2 image.  These addresses come from the
 * global SoC address map.
 */
static const struct soc_sram_block available_fp2_sram[] = {
	{
		.start = MANTICORE_SOC_FP2_ITCM_ADDRESS,
		.length = MANTICORE_SOC_FP2_ITCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP2_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP2_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP2_FP0_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP2_FP0_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_FP2_FP1_DTCM_ADDRESS,
		.length = MANTICORE_SOC_FP2_FP1_DTCM_SIZE
	},
	{
		.start = MANTICORE_SOC_PSRAM_ADDRESS,
		.length = MANTICORE_SOC_PSRAM_SIZE
	}
};


/**
 * The location in GSRAM where the PCIe PHY firmware will be staged after loading from flash.  This
 * memory block will contain all three firmware images, concatenated together.  Since this only
 * happens during SoC reset flows, there is no other data present in GSRAM, so there is no need to
 * worry about overlapping with other usage.  However, do not overlap the SP GSRAM region to avoid
 * odd behavior in certain error scenarios.
 */
#define	MANTICORE_PCIE_FW_STAGING_ADDRESS   \
	(MANTICORE_SOC_GSRAM_ADDRESS + sizeof (struct cp_shared_data))

/**
 * Offset from the base staging address where the Common PCIe PHY firmware will be staged.
 */
#define	MANTICORE_PCIE_COMMON_OFFSET		(PCIE_PHY_FW_MAIN_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH)

/**
 * Offset from the base staging address where the Lane PCIe PHY firmware will be staged.
 */
#define	MANTICORE_PCIE_LANE_OFFSET          \
	(MANTICORE_PCIE_COMMON_OFFSET + PCIE_PHY_FW_COMMON_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH)

/**
 * Total amount of GSRAM space used to stage PCIe PHY images.
 */
#define	MANTICORE_PCIE_FW_TOTAL_SPACE       \
	(MANTICORE_PCIE_LANE_OFFSET + PCIE_PHY_FW_LANE_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH)

/**
 * Regions of SoC SRAM that can be used to load the PCIe PHY image.  These addresses come from the
 * global SoC address map.
 *
 * Use GSRAM to stage the PCIe PHY images.
 */
static const struct soc_sram_block available_pcie_sram[] = {
	{
		.start = MANTICORE_PCIE_FW_STAGING_ADDRESS,
		.length = PCIE_PHY_FW_MAIN_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH
	},
	{
		.start = MANTICORE_PCIE_FW_STAGING_ADDRESS + MANTICORE_PCIE_COMMON_OFFSET,
		.length = PCIE_PHY_FW_COMMON_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH
	},
	{
		.start = MANTICORE_PCIE_FW_STAGING_ADDRESS + MANTICORE_PCIE_LANE_OFFSET,
		.length = PCIE_PHY_FW_LANE_MAX_SIZE + PCIE_PHY_FW_HEADER_LENGTH
	},
};

/**
 * Buffer for the DER encoded public key used to verify the 1SP firmware image.  This will be used
 * to verify the firmware key manifest.
 */
static uint8_t key_1sp[ECC_DER_P384_PUBLIC_LENGTH];

/**
 * Variable context for the 1SP loaded into memory.
 */
static struct hsp_fw_1sp_state fw_1sp_context;

/**
 * Handler for running the FIPS integrity check on the 1SP image loaded from internal flash.
 */
static const struct hsp_fw_1sp fw_1sp_internal = hsp_fw_1sp_static_init (&fw_1sp_context, NULL,
	&flash_internal.base);

/**
 * Handler for running the FIPS integrity check on the 1SP image loaded from external flash.
 */
static const struct hsp_fw_1sp fw_1sp_external = hsp_fw_1sp_static_init (&fw_1sp_context, NULL,
	&flash_external.base);

/**
 * HW backed RoT for the firmware key manifest.
 */
static const struct hw_rot_firmware_key_manifest rot_manifest =
	hw_rot_firmware_key_manifest_static_init (&fuses.base,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS, &ccs_fips,
	&rom_shared_static->firmware.pcr_log[1].fw_public_key.data.key, NULL);

/**
 * Handler for loading SPRT images into SP SRAM.
 */
static const struct firmware_loader_hsp_memory sp_loader =
	firmware_loader_hsp_memory_static_init (available_sram, ARRAY_SIZE (available_sram), &aes,
	DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the CP firmware loader.
 */
static struct firmware_loader_hsp_dmb_state cp_loader_context;

/**
 * Handler for loading CP images into CP SRAM.
 */
static const struct firmware_loader_hsp_dmb cp_loader =
	firmware_loader_hsp_dmb_static_init (&cp_loader_context, &dmb, available_cp_sram,
	ARRAY_SIZE (available_cp_sram), &aes, DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the FP core 0 firmware loader.
 */
static struct firmware_loader_hsp_dmb_state fp0_loader_context;

/**
 * Handler for loading FP images into FP core 0 SRAM.
 */
static const struct firmware_loader_hsp_dmb fp0_loader =
	firmware_loader_hsp_dmb_static_init (&fp0_loader_context, &dmb, available_fp0_sram,
	ARRAY_SIZE (available_fp0_sram), &aes, DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the FP core 0 firmware loader.
 */
static struct firmware_loader_hsp_dmb_state fp1_loader_context;

/**
 * Handler for loading FP images into FP core 1 SRAM.
 */
static const struct firmware_loader_hsp_dmb fp1_loader =
	firmware_loader_hsp_dmb_static_init (&fp1_loader_context, &dmb, available_fp1_sram,
	ARRAY_SIZE (available_fp1_sram), &aes, DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the FP core 2 firmware loader.
 */
static struct firmware_loader_hsp_dmb_state fp2_loader_context;

/**
 * Handler for loading FP images into FP core 2 SRAM.
 */
static const struct firmware_loader_hsp_dmb fp2_loader =
	firmware_loader_hsp_dmb_static_init (&fp2_loader_context, &dmb, available_fp2_sram,
	ARRAY_SIZE (available_fp2_sram), &aes, DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the PCIe PHY firmware loader.
 */
static struct firmware_loader_hsp_dmb_state pcie_loader_context;

/**
 * Handler for loading PCIe PHY images into GSRAM.
 */
static const struct firmware_loader_hsp_dmb pcie_loader =
	firmware_loader_hsp_dmb_static_init (&pcie_loader_context, &dmb, available_pcie_sram,
	ARRAY_SIZE (available_pcie_sram), &aes, DEVICE_KEYS_FW_IMAGE_AES_KEY);

/**
 * Variable context for the Manticore bootloader for loading main firmware.
 */
static struct manticore_bootloader_state boot_context;

/**
 * Main bootloader for loading firmware into internal memory from internal flash.
 */
static const struct manticore_bootloader boot_internal =
	manticore_bootloader_static_init (&boot_internal, &boot_context, &flash_internal.base,
	&sp1_shared.fw_keys, &hash.base, &ecc.base, key_1sp, sizeof (key_1sp), &rot_manifest.base,
	&sec_manager.base.base, &sp_loader.base, &cp_loader.base, &fp0_loader.base, &fp1_loader.base,
	&fp2_loader.base, &pcie_loader.base);

/**
 * Main bootloader for loading firmware into internal memory from external flash.
 */
static const struct manticore_bootloader boot_external =
	manticore_bootloader_static_init (&boot_external, &boot_context, &flash_external.base,
	&sp1_shared.fw_keys, &hash.base, &ecc.base, key_1sp, sizeof (key_1sp), &rot_manifest.base,
	&sec_manager.base.base, &sp_loader.base, &cp_loader.base, &fp0_loader.base, &fp1_loader.base,
	&fp2_loader.base, &pcie_loader.base);

/**
 * Measurement for the SPRT firmware.
 */
static SP_MSG_384 sp_measurement;

/**
 * Measurement for the CP firmware.
 */
static SP_MSG_384 cp_measurement;

/**
 * Measurement for the FP core 0 firmware.
 */
static SP_MSG_384 fp0_measurement;

/**
 * Measurement for the FP core 1 firmware.
 */
static SP_MSG_384 fp1_measurement;

/**
 * Measurement for the FP core 2 firmware.
 */
static SP_MSG_384 fp2_measurement;

/**
 * Measurement for the PCIe PHY firmware.
 */
static SP_MSG_384 phy_measurement;

/**
 * Public key used for warm reset PCIe PHY firmware verification.
 */
struct ecc_point_public_key phy_pub_key;

/**
 * Signature of the PCIe PHY main firmware image loaded from flash during POR flows.
 */
struct ecc_ecdsa_signature phy_signature;

/**
 * Length of the PCIe PHY main firmware image that was signed.
 */
uint32_t phy_fw_length;


/**
 * Force all arm cores to enter into a crash-handler if they are running. The crash is induced using
 * tcon wakeup 1 interrupt.
 *
 * This function always initializes the DMB driver state. The tcon wakeup interrupt is only sent if
 * this is not a graceful (fw update) warm reset.  On fw update, all arm cores have been quiesced
 * but still have tcon interrupt enabled.  Sending tcon interrupt at that point would cause the
 * cores to handle it and collect crashdumps unnecessarily.  For all other warm reset cases (SoC
 * level resets, fatal unrecoverable errors, watchdog, hardware errors), arm cores may produce
 * crashdumps alongside HSP crashdumps, which is acceptable as they represent actual failures.
 *
 * @return 0 if the interrupt was successfully triggered or an error code.
 */
static int tcon_wakeup_timer_interrupt (void)
{
	Tcon_t *tcon_regs;
	int status;
	uint32_t wakeup_cntrl_en_mask;
	uint32_t wakeup_cntrl_level_en_mask;
	uint32_t wakeup_cntrl_reg;

	/* Initialize the the DMB driver state before mapping the TCON registers. */
	status = hsp_dmb_init_state (&dmb);
	if (status != 0) {
		return status;
	}

	/* Skip the tcon wakeup interrupt on graceful (fw update) warm reset. */
	if (is_graceful_reset ()) {
		return 0;
	}

	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_TCON_REGISTERS, sizeof (*tcon_regs),
		HSP_DMB_ACCESS_WRITE | HSP_DMB_ACCESS_READ, (void**) &tcon_regs);
	if (status != 0) {
		return status;
	}

	/* Write 1 to wakeup1Cnt. */
	tcon_regs->wakeup1Cnt = 1;

	/* Read current wakeupCtrl value. */
	wakeup_cntrl_reg = tcon_regs->wakeupCtrl.all;

	/* Set WAKEUP_ENABLE bits [1:0] to 0x02. */
	wakeup_cntrl_en_mask = ((1U << WAKEUP_CTR1_WAKEUP_ENABLE_BIT_COUNT) - 1U) <<
		WAKEUP_CTR1_WAKEUP_ENABLE_BIT;
	wakeup_cntrl_reg = (wakeup_cntrl_reg & ~wakeup_cntrl_en_mask) | ((1U <<
		1U) & wakeup_cntrl_en_mask);

	/* Set WKINTR_LEVEL_EN bits [9:8] to 0x02. */
	wakeup_cntrl_level_en_mask = ((1U << WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT_COUNT) - 1U) <<
		WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT;
	wakeup_cntrl_reg = (wakeup_cntrl_reg & ~wakeup_cntrl_level_en_mask) | (((1U << 1U) <<
		WAKEUP_CTR1_WKINTR_LEVEL_EN_BIT) & wakeup_cntrl_level_en_mask);

	/* Write the updated value back. */
	tcon_regs->wakeupCtrl.all = wakeup_cntrl_reg;

	/* No longer need access to the TCON registers. */
	dmb.unmap_soc_address (&dmb, tcon_regs);

	/* Wait for the ISRs to execute in all cores. */
	platform_msleep (TCON_WAKEUP_ISR_DELAY_IN_MS);

	return 0;
}

/**
 * Halt execution for all CP and FP cores and return them to the reset vector.  CPU execution will
 * be stalled and left for SPRT to manage.
 *
 * @return 0 if the CPUs are in reset or an error code.
 */
static int reset_cpu_cores ()
{
	struct por_regs *por_regs;
	int status;

	/* Get access to the SoC registers for CP and FP reset control. */
	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_POR_REGISTERS, sizeof (*por_regs),
		HSP_DMB_ACCESS_WRITE, (void**) &por_regs);
	if (status != 0) {
		return status;
	}

	/* Stop the CP and FP cores during boot.  Every reset will reload all FW images. */
	por_regs->cp_run_stall = MANTICORE_SOC_CP_RUNSTALL_CP0_WAIT |
		MANTICORE_SOC_CP_RUNSTALL_CP1_WAIT;
	por_regs->fp_run_stall = MANTICORE_SOC_FP_RUNSTALL_FP0_WAIT |
		MANTICORE_SOC_FP_RUNSTALL_FP1_WAIT | MANTICORE_SOC_FP_RUNSTALL_FP2_WAIT;

	/* The reset control for all the ARM cores must be deasserted in order for firmware loading to
	 * work.  Accessing the TCMs with reset asserted causes the bus to hang and trigger the SP bus
	 * watchdog.  In addition, all CPU cores must have their reset control pulsed before loading
	 * firmware during warm reset flows.  This will reset the cores internal state and send
	 * execution back to the reset vector.
	 *
	 * None of the cores will start executing until the run/stall bit has been cleared for that
	 * core.
	 *
	 * Resetting the CP cores has the side-effect of clearing ECC enablement on the CP TCMs.  This
	 * means that CP TCM needs to have ECC re-enabled on every reset.  Resetting the CP cores
	 * individually instead of at the same time does not change the impact to ECC enablement.
	 *
	 * Resetting the FPS cores does not clear the ECC configuration on their TCMs.  However, this is
	 * only true if the FPS cores are reset individually.  If all the cores are reset
	 * simultaneously, ECC enablement is cleared. */
	por_regs->reset_control |= (MANTICORE_SOC_RESET_CTRL_CP0 | MANTICORE_SOC_RESET_CTRL_CP1);
	por_regs->reset_control &= ~(MANTICORE_SOC_RESET_CTRL_CP0 | MANTICORE_SOC_RESET_CTRL_CP1);

	por_regs->reset_control |= MANTICORE_SOC_RESET_CTRL_FP0;
	por_regs->reset_control &= ~MANTICORE_SOC_RESET_CTRL_FP0;

	por_regs->reset_control |= MANTICORE_SOC_RESET_CTRL_FP1;
	por_regs->reset_control &= ~MANTICORE_SOC_RESET_CTRL_FP1;

	por_regs->reset_control |= MANTICORE_SOC_RESET_CTRL_FP2;
	por_regs->reset_control &= ~MANTICORE_SOC_RESET_CTRL_FP2;

	/* No longer need access to the reset control registers for CP and FP cores. */
	dmb.unmap_soc_address (&dmb, por_regs);

	return 0;
}

/**
 * Run an integrity check of the 1SP loaded into memory.  This check is necessary for FIPS
 * compliance since the verification in ROM was done without appropriate self-tests.
 *
 * @return 0 if the 1SP image is valid or an error code.
 */
static int fips_integrity_check_running_image ()
{
	const struct hsp_fw_1sp *running_img;
	struct ecc_point_public_key fw_key;
	struct ecc_point_public_key secondary_key;
	int offset;
	int status;

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	uint8_t *corrupt = NULL;

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_algorithm (cmvp_test) ==
		CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_ROM)) {
		enum cmvp_test_case_cast_type cast = cmvp_test_case_get_cast_type (cmvp_test);

		if (cast == CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL) {
			corrupt = (uint8_t*) &ECC_KAT_VECTORS_ECDSA_SIGNED[16];
		}
		else if (cast == CMVP_TEST_CASE_CAST_TYPE_INTEGRITY_1SP) {
			/* Changing any data in the image memory will generate a failure. */
			corrupt = (uint8_t*) &MANTICORE_1SP_FIPS_KDF_CONTEXT.AsBytes[16];
		}
	}

	if (corrupt != NULL) {
		*corrupt ^= 0xff;
		corrupt = NULL;
	}
#endif

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_INTEGRITY_START);

	/* Self-test the ECDSA algorithm that will be used for the integrity check. */
	status = hsp_fw_run_self_test_verify_signed_image (&pka.base, &hash.base);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_ECDSA_1SP_VERIFY ^ status);

	/* Get the firmware keys used for verification from the measurement log. */
	memcpy (fw_key.x, rom_shared->firmware.pcr_log[1].fw_public_key.data.key.Parts.X.AsBytes,
		SP_MSG_384_SIZE);
	memcpy (fw_key.y, rom_shared->firmware.pcr_log[1].fw_public_key.data.key.Parts.Y.AsBytes,
		SP_MSG_384_SIZE);
	fw_key.key_length = ECC_KEY_LENGTH_384;

	memcpy (secondary_key.x,
		rom_shared->firmware.pcr_log[1].secondary_public_key.data.key.Parts.X.AsBytes,
		SP_MSG_384_SIZE);
	memcpy (secondary_key.y,
		rom_shared->firmware.pcr_log[1].secondary_public_key.data.key.Parts.Y.AsBytes,
		SP_MSG_384_SIZE);
	secondary_key.key_length = ECC_KEY_LENGTH_384;

	/* Re-run integrity checks of the 1SP image.  First verify the header signature from flash.
	 * Then, use the digest to check the 1SP image currently in SRAM. */
	if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) {
		offset = key_manifest_hsp_rom_get_size_on_flash (&flash_internal.base,
			MANTICORE_INTERNAL_SLOT_A_FLASH_ADDRESS);
		if (ROT_IS_ERROR (offset)) {
			return offset;
		}

		status = hsp_fw_1sp_init_state (&fw_1sp_internal, NULL,
			MANTICORE_INTERNAL_SLOT_A_FLASH_ADDRESS + offset);

		running_img = &fw_1sp_internal;
	}
	else if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT) {
		offset = key_manifest_hsp_rom_get_size_on_flash (&flash_external.base,
			MANTICORE_EXTERNAL_FLASH_ADDRESS);
		if (ROT_IS_ERROR (offset)) {
			return offset;
		}

		status = hsp_fw_1sp_init_state (&fw_1sp_external, NULL,
			MANTICORE_EXTERNAL_FLASH_ADDRESS + offset);

		running_img = &fw_1sp_external;
	}
	else {
		/* This image does not support booting over recovery. */
		status = -1;
	}
	if (status != 0) {
		return status;
	}

	status = hsp_fw_1sp_verify_signed_header (running_img, &hash.base, &pka.base, &fw_key,
		(sp1_shared.fips_certifed_1sp) ? &secondary_key : NULL,
		rom_shared->firmware.pcr_log[1].fw_svn.data.svn);
	if (status != 0) {
		return status;
	}

	status = hsp_fw_1sp_verify_image_in_memory (running_img, &hash.base);

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_INTEGRITY_END ^ status);

	return status;
}

/**
 * Initialize the bootloader for loading main Manticore firmware.
 *
 * @return 0 if the bootloader was successfully initialized or an error code.
 */
static int initialize_bootloader ()
{
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOTLOADER_INIT_START);

	/* Populate DER encoded 1SP key. */
	status =
		ecc_der_encode_public_key (
		rom_shared->firmware.pcr_log[1].fw_public_key.data.key.Parts.X.AsBytes,
		rom_shared->firmware.pcr_log[1].fw_public_key.data.key.Parts.Y.AsBytes, ECC_KEY_LENGTH_384,
		key_1sp, sizeof (key_1sp));
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	status = firmware_loader_hsp_dmb_init_state (&cp_loader);
	if (status != 0) {
		return status;
	}

	status = firmware_loader_hsp_dmb_init_state (&fp0_loader);
	if (status != 0) {
		return status;
	}

	status = firmware_loader_hsp_dmb_init_state (&fp1_loader);
	if (status != 0) {
		return status;
	}

	status = firmware_loader_hsp_dmb_init_state (&fp2_loader);
	if (status != 0) {
		return status;
	}

	status = firmware_loader_hsp_dmb_init_state (&pcie_loader);
	if (status != 0) {
		return status;
	}

	/* Get a mapping to the CP shared memory location. */
	status = dmb.map_soc_address (&dmb, CP_SHARED_GSRAM_ADDRESS, sizeof (*cp_shared),
		HSP_DMB_ACCESS_WRITE, (void**) &cp_shared);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (status);	// Need an even number of messages.

	/* Only support boot from internal slot A and external flash. */
	if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) {
		platform_printf ("Loading from Internal flash" NEWLINE);

		code_path_integrity_checkpoint_hand_off (&MANTICORE_1SP_CHECKPOINT_INIT_DONE,
			&MANTICORE_1SP_CHECKPOINT_LOAD_INTERNAL);

		code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOTLOADER_INTERNAL);

		status = manticore_bootloader_init_state (&boot_internal,
			MANTICORE_INTERNAL_SLOT_A_FLASH_ADDRESS, &ccs_fips.base, &rot_manifest.base,
			&sp1_shared.fw_descriptor);
	}
	else if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT) {
		platform_printf ("Loading from External flash" NEWLINE);

		code_path_integrity_checkpoint_hand_off (&MANTICORE_1SP_CHECKPOINT_INIT_DONE,
			&MANTICORE_1SP_CHECKPOINT_LOAD_EXTERNAL);

		code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOTLOADER_EXTERNAL);

		status = manticore_bootloader_init_state (&boot_external, MANTICORE_EXTERNAL_FLASH_ADDRESS,
			&ccs_fips.base, &rot_manifest.base, &sp1_shared.fw_descriptor);
	}
	else {
		/* This image does not support booting over recovery.  A different image will be used to
		 * recover device firmware. */
		status = -1;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOTLOADER_INIT_END ^ status);

	return status;
}

/**
 * Fill all FPS TCM with zeros.
 *
 * @return 0 if the FPS TCM was zeroized or an error code.
 */
static int zeroize_fps_tcm ()
{
	static const struct soc_sram_block fps_tcm[] = {
		{
			.start = MANTICORE_SOC_FP0_ITCM_ADDRESS,
			.length = MANTICORE_SOC_FP0_ITCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP0_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP0_DTCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP0_FP1_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP0_FP1_DTCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP1_ITCM_ADDRESS,
			.length = MANTICORE_SOC_FP1_ITCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP1_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP1_DTCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP1_FP2_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP1_FP2_DTCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP2_ITCM_ADDRESS,
			.length = MANTICORE_SOC_FP2_ITCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP2_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP2_DTCM_SIZE
		},
		{
			.start = MANTICORE_SOC_FP2_FP0_DTCM_ADDRESS,
			.length = MANTICORE_SOC_FP2_FP0_DTCM_SIZE
		}
	};

	return sram_erase_soc_memory_blocks (&dmb, fps_tcm, ARRAY_SIZE (fps_tcm));
}

/**
 * Clear additional memory for a reset which was not a POR, preserving certain regions of memory
 * depending on the type of reset.
 *
 * @return 0 if memory clear was successful or an error code.
 */
static int non_por_clear ()
{
	int status = 0;

	/* On every reset, erase the 1SP data region within the CP shared data.  Only the middle of this
	 * block is erased as the data at the beginning and end is only populated after POR and needs to
	 * be preserved. */
	memset (cp_shared, 0, offsetof (struct cp_shared_data, por_log));
	memset (&cp_shared->fips_certified, 0, CP_SHARED_ERASE_LENGTH);

	if (!is_graceful_reset ()) {
		/* On non-graceful reset, erase all FPS TCM before reloading the firmware. */
		status = zeroize_fps_tcm ();
	}

	return status;
}

/**
 * Enable ECC on the SRAM locations in the SoC that could contain firmware.  Also enable ECC for
 * GSRAM.  All memory will be cleared before enabling ECC.
 *
 * @param gsram_cleared Output flag to indicate when GSRAM was cleared as part of the call.
 *
 * @return 0 if ECC was successfully enabled for CP and FP SRAMs.
 */
static int enable_soc_sram_ecc (bool *gsram_cleared)
{
	struct cp_ecc_control *cp_mem_regs;
	struct fp_ecc_control *fp_mem_regs;
	struct gsram_ecc_control *gsram_regs;
	uint8_t gsram_ecc_err_status;
	uint32_t error_address;
	int status;

	*gsram_cleared = false;

	/* Enable ECC for CP TCM. */
	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_CP_ECC_REGISTERS, sizeof (*cp_mem_regs),
		HSP_DMB_ACCESS_WRITE, (void**) &cp_mem_regs);
	if (status != 0) {
		return status;
	}

	/* If either core does not have ECC enabled, treat it as if neither was enabled. */
	if (!(cp_mem_regs->cp0_control & MANTICORE_SOC_CP_CTRL_TCM_ECC_EN) ||
		!(cp_mem_regs->cp1_control & MANTICORE_SOC_CP_CTRL_TCM_ECC_EN)) {
		platform_printf ("CP SRAM ECC Not Enabled" NEWLINE);
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_ENABLE_CP_TCM_ECC, 0, 0);

		status = sram_erase_soc_memory_blocks (&dmb, available_cp_sram,
			ARRAY_SIZE (available_cp_sram));
		if (status != 0) {
			return status;
		}

		cp_mem_regs->cp0_control |= MANTICORE_SOC_CP_CTRL_TCM_ECC_EN;
		cp_mem_regs->cp1_control |= MANTICORE_SOC_CP_CTRL_TCM_ECC_EN;
	}

	dmb.unmap_soc_address (&dmb, cp_mem_regs);

	/* Enable ECC for FP. */
	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_FP_ECC_REGISTERS, sizeof (*fp_mem_regs),
		HSP_DMB_ACCESS_WRITE, (void**) &fp_mem_regs);
	if (status != 0) {
		return status;
	}

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	if (cmvp_mimic_por) {
		/* Force FPS memory wiping. */
		fp_mem_regs->fp_cpu[0].cpu_mem_control &= ~MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;
	}
#endif

	/* If any core does not have ECC enabled, treat it as if no ECC has been enabled. */
	if (!(fp_mem_regs->fp_cpu[0].cpu_mem_control & MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE) ||
		!(fp_mem_regs->fp_cpu[1].cpu_mem_control & MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE) ||
		!(fp_mem_regs->fp_cpu[2].cpu_mem_control & MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE)) {
		platform_printf ("FP SRAM ECC Not Enabled" NEWLINE);

		/* Only enable FP ECC as part of POR processing.  In any other condition, leave the ECC
		 * disabled since FPS may have critical information in portions of this memory. */
		if (is_por ()) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_ENABLE_FP_ECC, 0, 0);

			/* Be sure ECC is disabled on all cores before proceeding. */
			fp_mem_regs->fp_cpu[0].cpu_mem_control &= ~MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;
			fp_mem_regs->fp_cpu[1].cpu_mem_control &= ~MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;
			fp_mem_regs->fp_cpu[2].cpu_mem_control &= ~MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;

			/* Initialize FP PSRAM and enable ECC. */
			fp_mem_regs->bank1.fps_mem_control &= ~MANTICORE_SOC_FP_MEM_CONTROL_INIT_PATTERN_ONES;
			fp_mem_regs->bank1.fps_mem_control |=
				MANTICORE_SOC_FP_MEM_CONTROL_ECC_PARTIAL_WRITE_RMW_EN;

			fp_mem_regs->bank1.psram_mem_control |= (MANTICORE_SOC_FP_PSRAM_MEM_CONTROL_INIT_EN |
				MANTICORE_SOC_FP_PSRAM_MEM_CONTROL_ECC_EN);

			/* Enable ECC generation for all FP TCM. */
			fp_mem_regs->fp_cpu[0].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_GENERATE;
			fp_mem_regs->fp_cpu[1].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_GENERATE;
			fp_mem_regs->fp_cpu[2].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_GENERATE;

			/* Fill all FP TCM with zeros. */
			status = zeroize_fps_tcm ();
			if (status != 0) {
				return status;
			}

			/* Enable parity checking for FP data fabrics. */
			fp_mem_regs->bank1.fabric_error_control = MANTICORE_SOC_FP_FABRIC_PARITY_CHECK_EN;

			/* Enable ECC checking for FP TCM. */
			fp_mem_regs->fp_cpu[0].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;
			fp_mem_regs->fp_cpu[1].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;
			fp_mem_regs->fp_cpu[2].cpu_mem_control |= MANTICORE_SOC_FP_CPU_MEM_CONTROL_ECC_ENABLE;

			/* Clear any existing ECC or parity errors. */
			fp_mem_regs->bank0.fps_control |= MANTICORE_SOC_FP_CONTROL_ERROR_CLEAR;
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_FP_ECC_DISABLED_0, fp_mem_regs->fp_cpu[0].cpu_mem_control,
				fp_mem_regs->fp_cpu[1].cpu_mem_control);
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_FP_ECC_DISABLED_1, fp_mem_regs->fp_cpu[2].cpu_mem_control,
				fp_mem_regs->bank1.psram_mem_control);
		}
	}

	dmb.unmap_soc_address (&dmb, fp_mem_regs);

	/* Enable ECC for GSRAM. */
	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_GSRAM_ECC_REGISTERS, sizeof (*gsram_regs),
		HSP_DMB_ACCESS_WRITE, (void**) &gsram_regs);
	if (status != 0) {
		return status;
	}

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
	if (cmvp_mimic_por) {
		/* Force GSRAM memory wiping. */
		gsram_regs->ecc_enable_clear = MANTICORE_SOC_GSRAM_ECC_ENABLE;
	}
#endif

	/* Check for GSRAM ECC double bit error. */
	gsram_ecc_err_status = (gsram_regs->mem_int_stt &
		MANTICORE_SOC_GSRAM_MEM_ECC_ERR_STATUS) ? 1 : 0;

	if (gsram_ecc_err_status == 1) {
		/* Re-assembled the GSRAM ECC error address */
		error_address = ((gsram_regs->errd_log0) & MANTICORE_SOC_GSRAM_REG_ERRDLOG0_MASK) |
			MANTICORE_SOC_GSRAM_ADDRESS;

		/* As GSRAM ECC error address aligned 32-byte here we will take range */
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_GSRAM_ECC_ERROR, error_address, (error_address + 0x20));

		/* Clear the ECC Error bit */
		gsram_regs->mem_int_stt &= ~MANTICORE_SOC_GSRAM_MEM_ECC_ERR_STATUS;
		/* Clear GSRAM blocks for the ECC error. */
		gsram_regs->ecc_enable_clear = MANTICORE_SOC_GSRAM_ECC_ENABLE;
	}

	/* All GSRAM blocks should have ECC enabled. */
	while (gsram_regs->ecc_enable_state != MANTICORE_SOC_GSRAM_ECC_ENABLE) {
		platform_printf ("GSRAM ECC Not Enabled" NEWLINE);

		/* Only enable GSRAM ECC as part of POR processing or ECC error. */
		if (is_por () || (gsram_ecc_err_status == 1)) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_ENABLE_GSRAM_ECC, 0, 0);
			*gsram_cleared = true;

			/* Make sure there is no active initialization operation running. */
			while (gsram_regs->mem_init & MANTICORE_SOC_GSRAM_MEM_INIT_START) {
			}

			/* Use HW memory initialization flows to clear out the GSRAM. */
			gsram_regs->mem_init = MANTICORE_SOC_GSRAM_MEM_INIT_START |
				MANTICORE_SOC_GSRAM_MEM_INIT_START_FIRST_BLOCK |
				MANTICORE_SOC_GSRAM_MEM_INIT_ALL_BLOCKS;

			/* The START bit will self-clear when the operation has completed. */
			while (gsram_regs->mem_init & MANTICORE_SOC_GSRAM_MEM_INIT_START) {
			}

			gsram_regs->ecc_enable_set = MANTICORE_SOC_GSRAM_ECC_ENABLE;

			if (gsram_ecc_err_status == 1) {
				/* Ensure Soft reset after GSRAM ECC error handling */
				clear_graceful_reset ();

				status = INIT_GSRAM_ECC_ERROR;
			}
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_GSRAM_ECC_DISABLED, 0, 0);
			break;
		}
	}

	dmb.unmap_soc_address (&dmb, gsram_regs);

	return status;
}

/**
 * Reset the PCIe PHY and load the firmware image from flash into PHY memory.
 *
 * @param boot The bootloader to use for loading the firmware image.
 *
 * @return 0 if the PHY firmware load was successful or an error code.
 */
static int load_pcie_phy_firmware (const struct manticore_bootloader *boot)
{
	struct pcie_phy pcie;
	int i;
	int status;

	/* Stage the PHY image into GSRAM and verify it. */
	status = manticore_bootloader_load_pcie_phy (boot, NULL, 0);
	if (status != 0) {
		return status;
	}

	/* Map the GSRAM used to stage the PCIe PHY images, verify the expected images are there, and
	 * push the images into PCIe PHY memory. */
	status = dmb.map_soc_address (&dmb, MANTICORE_PCIE_FW_STAGING_ADDRESS,
		MANTICORE_PCIE_FW_TOTAL_SPACE, 0, (void**) &pcie.fw[0]);
	if (status != 0) {
		return status;
	}

	if ((pcie.fw[0]->type != PCIE_PHY_FW_TYPE_MAIN) ||
		(pcie.fw[0]->length > IN_DWORDS (PCIE_PHY_FW_MAIN_MAX_SIZE))) {
		return INIT_INVALID_MAIN_PHY_IMAGE;
	}

	pcie.fw[1] = (void*) ((uintptr_t) pcie.fw[0] + MANTICORE_PCIE_COMMON_OFFSET);
	if ((pcie.fw[1]->type != PCIE_PHY_FW_TYPE_COMMON) ||
		(pcie.fw[1]->length > IN_DWORDS (PCIE_PHY_FW_COMMON_MAX_SIZE))) {
		return INIT_INVALID_MAIN_PHY_IMAGE;
	}

	pcie.fw[2] = (void*) ((uintptr_t) pcie.fw[0] + MANTICORE_PCIE_LANE_OFFSET);
	if ((pcie.fw[2]->type != PCIE_PHY_FW_TYPE_LANE) ||
		(pcie.fw[2]->length > IN_DWORDS (PCIE_PHY_FW_LANE_MAX_SIZE))) {
		return INIT_INVALID_MAIN_PHY_IMAGE;
	}

	/* Take a measurement of the data loaded into GSRAM rather than using the measurement generated
	 * during the load process.  This ensures that the PHY firmware measurement remains the same for
	 * any given PHY firmware version.  The measurement from image loading includes firmware
	 * component headers in the digest, which change with the firmware package release, meaning the
	 * PHY measurement would change for the same firmware binary.  Having different measurements for
	 * PHY firmware would cause an issue for attestation after an impactless update since the PHY
	 * firmware is only loaded on POR. */
	status = hash.base.start_sha384 (&hash.base);
	if (status != 0) {
		return status;
	}

	for (i = 0; i < 3; i++) {
		status = hash.base.update (&hash.base, (uint8_t*) &pcie.fw[i]->data,
			pcie.fw[i]->length * 4);
		if (status != 0) {
			hash.base.cancel (&hash.base);

			return status;
		}
	}

	status = hash.base.finish (&hash.base, phy_measurement.AsBytes, SP_MSG_384_SIZE);
	if (status != 0) {
		hash.base.cancel (&hash.base);

		return status;
	}

	/* Load the firmware images into PHY memory. */
	status = pcie_phy_load_firmware (&pcie);
	if (status != 0) {
		return status;
	}

	dmb.unmap_soc_address (&dmb, pcie.fw[0]);

	/* Erase the staging GSRAM region.  GSRAM needs to be remapped to get write access. */
	sram_erase_soc_memory_blocks (&dmb, available_pcie_sram, ARRAY_SIZE (available_pcie_sram));

	return status;
}

/**
 * Load the firmware for all cores from flash into target memory.
 *
 * @param boot The bootloader to use for loading the firmware images.
 *
 * @return 0 if all the firmware was successfully loaded or an error code.
 */
static int load_all_firmware_images (const struct manticore_bootloader *boot)
{
	int status;

	/* Only load the PCIe PHY firmware from flash on SoC resets.
	 *
	 * PHY firmware will be handled before the rest of the images since some memory regions are used
	 * for caching data during load/verify flows. */
	if (is_por ()) {
		status = load_pcie_phy_firmware (boot);
		if ((status != 0) && (status != MANTICORE_FW_PACKAGE_NO_PCIE_IMAGES)) {
			return status;
		}
	}

	return manticore_bootloader_load_all_cores (boot, sp_measurement.AsBytes, SP_MSG_384_SIZE,
		cp_measurement.AsBytes, SP_MSG_384_SIZE, fp0_measurement.AsBytes, SP_MSG_384_SIZE,
		fp1_measurement.AsBytes, SP_MSG_384_SIZE, fp2_measurement.AsBytes, SP_MSG_384_SIZE);
}

/**
 * Load the main firmware from flash into Manticore SRAM.
 *
 * @return 0 if the firmware was loaded successfully or an error code.
 */
static int load_main_firmware ()
{
	bool gsram_cleared;
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOAD_START);

	/* Once the cores are stalled, enable ECC for target memories. */
	status = enable_soc_sram_ecc (&gsram_cleared);
	if (status != 0) {
		return status;
	}

	/* Clear the CP shared memory when CP is in reset.  This clearing is done during ECC enablement
	 * after SoC reset, so it can be skipped in that case.  Do not clear the POR measurement log. */
	if (!gsram_cleared) {
		status = non_por_clear ();
		if (status != 0) {
			return status;
		}
	}

	if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) {
		platform_printf ("Booting from Internal flash" NEWLINE);

		code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOAD_INTERNAL);

		status = load_all_firmware_images (&boot_internal);
	}
	else if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT) {
		platform_printf ("Booting from External flash" NEWLINE);

		code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOAD_EXTERNAL);

		status = load_all_firmware_images (&boot_external);
	}
	else {
		/* This should not be possible since it's checked before we get here. */
		status = -1;
	}

	if (status == 0) {
		size_t length;
		bool fips_certified_sprt;
		bool bks_fips_isolation;
		const struct security_manager_hsp *non_fips_unlock = NULL;
		const SP_MSG_384 *bks_fips_context = NULL;

		code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOADED ^ status);

		fips_certified_sprt =
			(manticore_firmware_descriptor_fips_certified (&sp1_shared.fw_descriptor) != 0);
		bks_fips_isolation =
			(manticore_firmware_descriptor_bks_fips_isolation (&sp1_shared.fw_descriptor) != 0);

		/* Set the service indicator based the loaded firmware. */
		if (!secure_boot || has_booted_unlocked ()) {
			/* The device is either unlocked or has not been reset since the last unlock. */
			strcpy (sp1_shared.service_indicator, "Non-FIPS");
		}
		else if (!sp1_shared.fips_certifed_1sp || !fips_certified_sprt) {
			/* If the loaded firmware is not FIPS certified, get new device keys.  Only change the
			 * the global key if specified to do so in the firmware descriptor.  Change the unlock
			 * HMAC key if 1SP is FIPS certified. */
			if (sp1_shared.fips_certifed_1sp) {
				non_fips_unlock = &sec_manager.base;
			}

			status = manticore_device_keys_derive_non_fips_sprt_keys (&ccs_fips.base,
				&MANTICORE_NON_FIPS_DEVICE_KDF_CONTEXT, non_fips_unlock);
			if (status != 0) {
				return status;
			}

			/* The running firmware is not FIPS certified. */
			strcpy (sp1_shared.service_indicator, "Non-FIPS");
		}
		else {
			/* The device is locked and running FIPS certified firmware. */
			strcpy (sp1_shared.service_indicator, "FIPS");

			/* Share this status with CP.  This region is cleared on every reset, so only needs to
			 * be set in this scenario. */
			cp_shared->fips_certified = 1;

			if (bks_fips_isolation) {
				/* Generate different BKS Keys for FIPS operation. */
				bks_fips_context = &MANTICORE_FIPS_BKS_KDF_CONTEXT;
			}
		}

		/* Derive the BKS keys in KSU.  This uses the firmware descriptor SVN since that instance is
		 * directly accessible and is guaranteed to be the same as the SVN in the firmware key
		 * manifest. */
		status = manticore_device_keys_derive_bks_keys (&ccs_fips.base, bks_fips_context,
			&MANTICORE_BKS1_KDF_CONTEXT,
			manticore_firmware_descriptor_get_svn (&sp1_shared.fw_descriptor),
			&MANTICORE_BKS2_KDF_CONTEXT, &MANTICORE_SPRT_OWNER_GLOBAL_KEY_KDF_CONTEXT);
		if (status != 0) {
			return status;
		}

		build_version_to_string (manticore_firmware_descriptor_get_build_version (
			&sp1_shared.fw_descriptor), secure_boot, fips_certified_sprt, sp1_shared.version_sprt,
			sizeof (sp1_shared.version_sprt));
		snprintf (sp1_shared.idfu_version, sizeof (sp1_shared.idfu_version), "%d",
			manticore_firmware_descriptor_image_compatibility_version (&sp1_shared.fw_descriptor));

		/* Copy the same version string to the shared CP location, except this copy needs to be
		 * padded with spaces. */
		length = strlen (sp1_shared.version_sprt);

		memcpy (cp_shared->fw_version, sp1_shared.version_sprt, length);
		memset (&cp_shared->fw_version[length], ' ', sizeof (cp_shared->fw_version) - length);
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOAD_END ^ status);

	return status;
}

/**
 * Get the measurement that should be used for the PCIe PHY firmware for this boot context.  On most
 * POR boots, this is a no-op since the PHY firmware has been loaded and measured.  However, there
 * are error and warm reset scenarios where the the PHY firmware measurement may not be known.  Make
 * sure the PHY measurement container has valid contents for all conditions.
 */
static void get_pcie_phy_firmware_measurement ()
{
	bool por_log_valid = false;
	int status;

	if (is_pcr2_extended ()) {
		/* There exist POR measurements in GSRAM that have been extended to PCR 2.  Check to see if
		 * the log contents match the current HW PCR state.  If so, this indicates they are still
		 * valid and can be referenced during this boot context. */
		status = manticore_measurements_verify_soc_pcr (&cp_shared->por_log.soc, &hash.base,
			&ccs_fips.base, 2, false);
		if (status == 0) {
			por_log_valid = true;
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_BOOT,
				BOOT_LOGGING_UNEXPECTED_PCR_STATE, 2, status);
		}
	}

	if (!is_por ()) {
		/* This is a warm reset scenario, so PHY firmware was not reloaded.  Use the measurement
		 * from the POR log.  If the POR log is not valid, provide an empty measurement since the
		 * measurement is unknown. */
		if (por_log_valid) {
			memcpy (phy_measurement.AsBytes, cp_shared->por_log.soc.phy_image.data.digest.AsBytes,
				SP_MSG_384_SIZE);
		}
		else {
			memset (phy_measurement.AsBytes, 0, SP_MSG_384_SIZE);
		}
	}
	else if (is_pcr2_extended ()) {
		/* This is an error scenario where a full POR initialization has not yet completed, but the
		 * initial measurements have already been extended to PCR 2.  This is not an issue as long
		 * as the current PHY measurement matches the measurement from the POR log and the POR log
		 * matches the state of PCR 2.  If either of these is not true, the device is in a state
		 * that could cause future issues, since the PHY measurement will not be able to be verified
		 * on subsequent resets.  There is no way to self-correct this condition, so the best
		 * approach is to erase the PHY measurement so that attestation fails, triggering external
		 * remediation for the problem.
		 *
		 * While a highly unlikely scenario, it's possible for the PHY measurement for this boot
		 * context to be different from the measurement in the POR log.  It would mean the main
		 * image failed to fully initialize after a SoC reset and the device reverted to the
		 * recovery image.  However, in most cases, the recovery image will match the main image.
		 * The exposure here would primarily be during the first boot after an impactful update. */
		status = memcmp (phy_measurement.AsBytes,
			cp_shared->por_log.soc.phy_image.data.digest.AsBytes, SP_MSG_384_SIZE);
		if (!por_log_valid || (status != 0)) {
			if (por_log_valid) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_BOOT,
					BOOT_LOGGING_UNEXPECTED_MEASUREMENT, INIT_DIFFERENT_PHY_MEASUREMENT, 0);
			}

			memset (phy_measurement.AsBytes, 0, SP_MSG_384_SIZE);

			/* Also clear the POR log entry for the PHY firmware to prevent this unknown scenario
			 * from being trusted before the next SoC reset. */
			memset (&cp_shared->por_log.soc.phy_image, 0,
				sizeof (cp_shared->por_log.soc.phy_image));
		}
		else {
			/* The current PHY firmware measurement matches the value from the PCR 2 log, so boot
			 * can proceed normally. */
		}
	}
	else {
		/* This is a normal POR boot.  Use the measurement that was calculated when the image was
		 * loaded. */
	}
}

/**
 * Generate the full measurement log for the firmware that was loaded.  This includes both SP and
 * SoC measurements.
 *
 * @param boot The bootloader that contains the key manifest used for verification.
 *
 * @return 0 if the measurement log was generated successfully or an error code.
 */
static int generate_firmware_measurement_log (const struct manticore_bootloader *boot)
{
	int status;

	get_pcie_phy_firmware_measurement ();

	status = manticore_measurements_generate_sp_log (&sp1_shared.pcr_log.sp, &hash.base,
		&sp1_shared.sec_policy, &boot->manifest, &sp1_shared.fw_descriptor, &sp_measurement);

	if (status == 0) {
		status = manticore_measurements_generate_soc_log (&sp1_shared.pcr_log.soc, &hash.base,
			&boot->manifest, &sp1_shared.fw_descriptor, &cp_measurement, &fp0_measurement,
			&fp1_measurement, &fp2_measurement, &phy_measurement);
	}

	return status;
}

/**
 * Generate the SP and SoC measurement logs and update the necessary PCRs.
 * - PCRs 1 and 3 will be updated every time firmware is loaded into the respective cores.
 * - PCRs 0 and 2 will be updated once after SoC reset.
 *
 * @return 0 if the measurements were generated successfully or an error code.
 */
static int generate_measurement_logs_and_extend_pcrs ()
{
	int status;

	code_path_integrity_secure_message_no_trace (0);	// Need an even number of messages.

	/* A different key manifest instance is needed depending on which boot slot was used. */
	if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) {
		code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_INTERNAL);
		code_path_integrity_checkpoint_hand_off (&MANTICORE_1SP_CHECKPOINT_LOAD_INTERNAL_DONE,
			&MANTICORE_1SP_CHECKPOINT_START_SPRT_EXECUTE);

		status = generate_firmware_measurement_log (&boot_internal);
	}
	else if (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT) {
		code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_EXTERNAL);
		code_path_integrity_checkpoint_hand_off (&MANTICORE_1SP_CHECKPOINT_LOAD_EXTERNAL_DONE,
			&MANTICORE_1SP_CHECKPOINT_START_SPRT_EXECUTE);

		status = generate_firmware_measurement_log (&boot_external);
	}
	else {
		/* This should not be possible, since there would have been a failure earlier. */
		status = -1;
	}
	if (status != 0) {
		return status;
	}

	/* Extend PCR 1 and 3.  Reinitialize and lock PCR3 before extending to it. */
	status = manticore_measurements_extend_sp_pcr (&sp1_shared.pcr_log.sp, &ccs_fips.base, 1);
	if (status != 0) {
		return status;
	}

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_RESET_PCR3);

	status = aeb.enable_aeb (&aeb, MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT);
	if (status != 0) {
		return status;
	}

	status = ccs_fips.base.reset_pcr (&ccs_fips.base, 3);
	if (status != 0) {
		return status;
	}

	status = aeb.disable_and_lock_aeb (&aeb, MANTICORE_AEB_CCS_ALLOW_PCR3_REINIT);
	if (status != 0) {
		return status;
	}

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_RESET_PCR3_DONE);

	status = manticore_measurements_extend_soc_pcr (&sp1_shared.pcr_log.soc, &ccs_fips.base, 3);
	if (status != 0) {
		return status;
	}

	if (!is_pcr0_extended_with_sprt ()) {
		/* PCR 0 has not yet been extended with SPRT measurements, so do it now.  Set the flag
		 * indicating that this has been done before starting since any errors would leave things in
		 * an unknown state. */

		memcpy (&cp_shared->por_log.rom, &rom_shared->firmware.pcr_log[0],
			sizeof (cp_shared->por_log.rom));
		memcpy (&cp_shared->por_log.sp, &sp1_shared.pcr_log.sp, sizeof (cp_shared->por_log.sp));
		pcr0_extended_with_sprt ();

		status = manticore_measurements_extend_sp_pcr (&sp1_shared.pcr_log.sp, &ccs_fips.base, 0);
		if (status != 0) {
			return status;
		}
	}

	if (!is_pcr2_extended ()) {
		/* PCR 2 has not yet been extended with SoC firmware measurements.  Update it in the same
		 * way as PCR 0. */

		memcpy (&cp_shared->por_log.soc, &sp1_shared.pcr_log.soc, sizeof (cp_shared->por_log.soc));
		pcr2_extended ();

		status = manticore_measurements_extend_soc_pcr (&sp1_shared.pcr_log.soc, &ccs_fips.base, 2);
		if (status != 0) {
			return status;
		}
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_END ^ status);

	return 0;
}

/**
 * Update the SP measurement logs and PCRs to include the current state of device AEBs.
 *
 * @return 0 if the measurements were updated successfully or an error code.
 */
static int update_pcrs_with_aeb_state ()
{
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_AEB_START);

	status = manticore_measurements_update_sp_log_with_aeb_state (&sp1_shared.pcr_log.sp,
		&hash.base, &aeb);
	if (status != 0) {
		return status;
	}

	/* Extend PCR 1 with the AEB measurements. */
	status = manticore_measurements_extend_sp_pcr_with_aeb_state (&sp1_shared.pcr_log.sp,
		&ccs_fips.base, 1);
	if (status != 0) {
		return status;
	}

	/* Just like for SPRT measurements, check if PCR 0 needs to also be updated. */
	if (!is_pcr0_extended_with_aeb_state ()) {
		memcpy (&cp_shared->por_log.sp, &sp1_shared.pcr_log.sp, sizeof (cp_shared->por_log.sp));
		pcr0_extended_with_aeb_state ();

		status = manticore_measurements_extend_sp_pcr_with_aeb_state (&sp1_shared.pcr_log.sp,
			&ccs_fips.base, 0);
		if (status != 0) {
			return status;
		}
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_AEB_END ^ status);

	return 0;
}

/**
 * The CA pathLenConstraint for the Device ID certificate.
 */
#define	MANTICORE_DICE_DEVICE_ID_PATHLEN		1

/**
 * The CA pathLenConstraint for the CP Alias certificate.
 */
#define	MANTICORE_DICE_ALIAS_CP_PATHLEN			0

/**
 * The ECC API to use for DICE keys.
 */
static const struct ecc_engine_ccs ecc_dice = ecc_ccs_static_init (&ccs_fips.base, &ecc.base);

/**
 * Base64 encoder to use for DICE certificates.
 */
static const struct base64_engine_core base64 = base64_core_static_init;

/**
 * X.509 builder to use for DICE certificates.
 *
 * There needs to be a little extra padding to keep the DER encoder happy, even though the extra
 * space won't get used.
 */
static const struct x509_engine_cert_build x509 = x509_cert_build_static_init (&ecc_dice.base,
	&hash.base, MAX_FIPS_DEVID_CERT_LENGTH + 32);

/**
 * DME structure endorsing the FIPS approved DICE identity key.
 */
struct manticore_device_keys_dice_endorsement fips_dme_structure;

/**
 * Buffer to use for building DICE certificate extensions.  A single buffer can be used for all
 * extensions.
 */
static uint8_t ext_buffer[768];

/**
 * List of Extended Key Usage OIDs to add to the layer 0 DICE certificate.
 */
static struct x509_extension_builder_eku_oid layer0_eku[] = {
	{
		.oid = TCG_DICE_OID_IDEVID,
		.length = TCG_DICE_OID_IDEVID_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ATTEST_INIT,
		.length = TCG_DICE_OID_ATTEST_INIT_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ECA,
		.length = TCG_DICE_OID_ECA_LENGTH
	},
	{
		.oid = DICE_OID_MANTICORE,
		.length = DICE_OID_MANTICORE_LENGTH
	}
};

/**
 * Handler for the Extended Key Usage extension for the layer 0 DICE certificate.
 */
static struct x509_extension_builder_eku layer0_eku_ext =
	x509_extension_builder_eku_static_init_with_buffer (layer0_eku, ARRAY_SIZE (layer0_eku), false,
	ext_buffer, sizeof (ext_buffer));

/**
 * List of Extended Key Usage OIDs to add to the layer 1 DICE certificate.
 */
static struct x509_extension_builder_eku_oid layer1_eku[] = {
	{
		.oid = TCG_DICE_OID_LDEVID,
		.length = TCG_DICE_OID_LDEVID_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ATTEST_LOCAL,
		.length = TCG_DICE_OID_ATTEST_LOCAL_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ASSERT_LOCAL,
		.length = TCG_DICE_OID_ASSERT_LOCAL_LENGTH
	},
	{
		.oid = SPDM_OID_RESPONDER_AUTH,
		.length = SPDM_OID_RESPONDER_AUTH_LENGTH
	}
};

/**
 * Handler for the Extended Key Usage extension for the layer 1 DICE certificate.
 */
static struct x509_extension_builder_eku layer1_eku_ext =
	x509_extension_builder_eku_static_init_with_buffer (layer1_eku, ARRAY_SIZE (layer1_eku), false,
	ext_buffer, sizeof (ext_buffer));

/**
 * List of Extended Key Usage OIDs to add to the layer 1 DICE certificate for CP.
 */
static struct x509_extension_builder_eku_oid layer1_cp_eku[] = {
	{
		.oid = TCG_DICE_OID_LDEVID,
		.length = TCG_DICE_OID_LDEVID_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ATTEST_LOCAL,
		.length = TCG_DICE_OID_ATTEST_LOCAL_LENGTH
	},
	{
		.oid = TCG_DICE_OID_ECA,
		.length = TCG_DICE_OID_ECA_LENGTH
	}
};

/**
 * Handler for the Extended Key Usage extension for the layer 1 DICE certificate for CP.
 */
static struct x509_extension_builder_eku layer1_cp_eku_ext =
	x509_extension_builder_eku_static_init_with_buffer (layer1_cp_eku, ARRAY_SIZE (layer1_cp_eku),
	false, ext_buffer, sizeof (ext_buffer));

/**
 * Storage for the big endian representation of the layer 0 SVN value.
 */
static uint8_t layer0_svn[sizeof (uint32_t)];

/**
 * Buffer for the layer 0 FWID digest used to derive the CDI.
 */
static SP_MSG_384 layer0_fwid;

/**
 * Buffer for the measurement of layer 0 firmware which excludes the tenancy counter.  This digest
 * will be common across all devices running the same firmware.
 */
static SP_MSG_384 layer0_fw_digest;

/**
 * List of FWIDs for DICE layer 0.
 */
static const struct tcg_dice_fwid layer0_fwid_list[] = {
	{
		.digest = layer0_fwid.AsBytes,
		.hash_alg = HASH_TYPE_SHA384
	},
};

/**
 * Descriptor for the device-independent firmware measurement for layer 0.
 */
static const struct tcg_dice_fwid layer0_ir_fw_digest[] = {
	{
		.digest = layer0_fw_digest.AsBytes,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * Descriptor for the tenancy counter measurement.
 */
static const struct tcg_dice_fwid layer0_ir_tc_digest[] = {
	{
		.digest = rom_shared_static->firmware.pcr_log[1].tenancy_counter.digest.AsBytes,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * List of named digests to report in the layer 0 TcbInfo.
 */
static const struct tcg_dice_integrity_register layer0_ir_list[] = {
	{
		.name = "FW",
		.number = -1,
		.digests = layer0_ir_fw_digest,
		.digest_count = ARRAY_SIZE (layer0_ir_fw_digest)
	},
	{
		.name = "TC",
		.number = -1,
		.digests = layer0_ir_tc_digest,
		.digest_count = ARRAY_SIZE (layer0_ir_tc_digest)
	}
};

/**
 * Information about the TCB for DICE layer 0.
 */
static const struct tcg_dice_tcbinfo layer0_tcb = {
	.vendor = "Microsoft",
	.model = "Azure Integrated HSM",
	.version = sp1_shared.version_1sp,
	.layer = 0,
	.svn = layer0_svn,
	.svn_length = sizeof (layer0_svn),
	.fwid_list = layer0_fwid_list,
	.fwid_count = ARRAY_SIZE (layer0_fwid_list),
	.ir_list = layer0_ir_list,
	.ir_count = ARRAY_SIZE (layer0_ir_list)
};

/**
 * Handler for the layer 0 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer0_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer0_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Storage for the big endian representation of the layer 1 SVN value.
 */
static uint8_t layer1_svn[sizeof (uint64_t)];

/**
 * Buffer for the layer 1 SPRT FWID value.
 */
static SP_MSG_384 layer1_sprt_fwid;

/**
 * List of FWIDs for SP DICE layer 1.
 */
static const struct tcg_dice_fwid layer1_fwid_list[] = {
	{
		.digest = layer1_sprt_fwid.AsBytes,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * Information about the SP TCB for DICE layer 1.
 */
static const struct tcg_dice_tcbinfo layer1_tcb = {
	.vendor = "Microsoft",
	.model = "Azure Integrated HSM",
	.version = sp1_shared.version_sprt,
	.layer = 1,
	.svn = layer1_svn,
	.svn_length = sizeof (layer1_svn),
	.fwid_list = layer1_fwid_list,
	.fwid_count = ARRAY_SIZE (layer1_fwid_list),
	.ir_list = NULL,
	.ir_count = 0
};

/**
 * Handler for the layer 1 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer1_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer1_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Buffer for the layer 1 HSM FWID value.
 */
static uint8_t layer1_cp_fwid[SHA384_HASH_LENGTH];

/**
 * List of FWIDs for CP DICE layer 1.
 */
static const struct tcg_dice_fwid layer1_cp_fwid_list[] = {
	{
		.digest = layer1_cp_fwid,
		.hash_alg = HASH_TYPE_SHA384
	}
};

/**
 * Descriptor for the SPRT digest to populate in the CP Alias certificate.
 */
static const struct tcg_dice_fwid layer1_cp_ir_digest[] = {
	{
		.digest = layer1_sprt_fwid.AsBytes,
		.hash_alg = HASH_TYPE_SHA384
	},
};

/**
 * List of named digests to report in the CP layer 1 TcbInfo.
 */
static const struct tcg_dice_integrity_register layer1_cp_ir_list[] = {
	{
		.name = "RoT",
		.number = -1,
		.digests = layer1_cp_ir_digest,
		.digest_count = ARRAY_SIZE (layer1_cp_ir_digest)
	}
};

/**
 * Information about the HSM TCB for DICE layer 1.
 */
static const struct tcg_dice_tcbinfo layer1_cp_tcb = {
	.vendor = "Microsoft",
	.model = "Azure Integrated HSM",
	.version = sp1_shared.version_sprt,
	.layer = 1,
	.svn = layer1_svn,
	.svn_length = sizeof (layer1_svn),
	.fwid_list = layer1_cp_fwid_list,
	.fwid_count = ARRAY_SIZE (layer1_cp_fwid_list),
	.ir_list = layer1_cp_ir_list,
	.ir_count = ARRAY_SIZE (layer1_cp_ir_list)
};

/**
 * Handler for the layer 1 TcbInfo extension.
 */
static const struct x509_extension_builder_dice_tcbinfo layer1_cp_tcb_ext =
	x509_extension_builder_dice_tcbinfo_static_init_with_buffer (&layer1_cp_tcb, ext_buffer,
	sizeof (ext_buffer));

/**
 * Handler for the Ueid extension for the layer 0 certificate.
 */
static const struct x509_extension_builder_dice_ueid layer0_ueid_ext =
	x509_extension_builder_dice_ueid_static_init_with_buffer ((uint8_t*) ueid, sizeof (ueid),
	ext_buffer, sizeof (ext_buffer));

/**
 * Current value of the DME renewal counter.
 */
static uint32_t dme_renewal_counter;

/**
 * Information about DME for this device and boot context.
 */
static struct dme_structure_raw_ecc dme;

/**
 * Handler for the DME extension for the layer 0 certificate.
 */
static const struct x509_extension_builder_dme layer0_dme_ext =
	x509_extension_builder_dme_static_init_with_buffer (&dme.base, ext_buffer, sizeof (ext_buffer));

/**
 * List of extensions to add to the DICE layer 0 certificate and CSR.
 */
static const struct x509_extension_builder *const layer0_ext[] = {
	&layer0_eku_ext.base, &layer0_tcb_ext.base, &layer0_ueid_ext.base, &layer0_dme_ext.base
};

/**
 * List of extensions to add to the DICE layer 1 certificate.
 */
static const struct x509_extension_builder *const layer1_ext[] = {
	&layer1_eku_ext.base, &layer1_tcb_ext.base
};

/**
 * List of extensions to add to the DICE layer 1 certificate for CP.
 */
static const struct x509_extension_builder *const layer1_cp_ext[] = {
	&layer1_cp_eku_ext.base, &layer1_cp_tcb_ext.base
};

/**
 * Variable context for DICE layer 0 processing.
 */
static struct riot_core_hsp_fips_state dice_context;

/**
 * DICE layer 0 handler.
 */
static const struct riot_core_hsp_fips dice = riot_core_hsp_fips_static_init (&dice_context,
	&ccs_fips.base, &pka.base, &hash.base, &base64.base, &x509.base, DEVICE_KEYS_DICE_CDI,
	MANTICORE_DEVICE_KEYS_FIPS_DEVICE_ID_KEY, MANTICORE_DEVICE_KEYS_SP_ALIAS_KEY, layer0_ext,
	ARRAY_SIZE (layer0_ext), MANTICORE_DICE_DEVICE_ID_PATHLEN, layer1_ext, ARRAY_SIZE (layer1_ext));

/**
 * Variable context for CP DICE layer 0 processing.
 */
static struct riot_core_hsp_fips_state dice_cp_context;

/**
 * CP DICE layer 0 handler.
 */
static const struct riot_core_hsp_fips dice_cp =
	riot_core_hsp_fips_static_init_alias_ca (&dice_cp_context, &ccs_fips.base, &pka.base,
	&hash.base, &base64.base, &x509.base, DEVICE_KEYS_DICE_CDI,
	MANTICORE_DEVICE_KEYS_FIPS_DEVICE_ID_KEY, MANTICORE_DEVICE_KEYS_CP_ALIAS_KEY, layer0_ext,
	ARRAY_SIZE (layer0_ext), MANTICORE_DICE_DEVICE_ID_PATHLEN, layer1_cp_ext,
	ARRAY_SIZE (layer1_cp_ext), MANTICORE_DICE_ALIAS_CP_PATHLEN);

/**
 * Handler for renewal of the device identity.
 */
static const struct identity_renewal identity = identity_renewal_static_init (&fuses.base);


/**
 * Initialize the handler for generating DICE identity keys and certificates.
 *
 * @return 0 if the DICE handler was initialized successfully or an error code.
 */
static int initialize_dice ()
{
	struct ksu_pcr_slot *pcr = (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS;
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_DICE_INIT_START);

	/* Generate a new DICE identity key that is FIPS compliant. */
	status = manticore_device_keys_convert_1sp_keys (&ccs_fips.base, &ccs.base, &pka.base,
		&hash.base, &MANTICORE_1SP_FIPS_KDF_KEY, &MANTICORE_1SP_FIPS_KDF_CONTEXT,
		&rom_shared->firmware.dme_structure, &rom_shared->firmware.device_id_key,
		&fips_dme_structure);
	if (status != 0) {
		return status;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_KEYS_DONE ^ status);

	/* Configure the TCB structure for layer 0. */
	buffer_reverse_copy (layer0_svn, (uint8_t*) &rom_shared->firmware.pcr_log[1].fw_svn.data.svn,
		sizeof (layer0_svn));
	memcpy (layer0_fwid.AsBytes, (uint32_t*) pcr[1].pcr, SHA384_HASH_LENGTH);

	status =
		boot_measurements_single_root_generate_device_independent_fwid (
		&rom_shared->firmware.pcr_log[1], &hash.base, &layer0_fw_digest);
	if (status != 0) {
		return status;
	}

	/* Configure the DME structure for layer 0. */
	status = dme_structure_raw_ecc_init_chained_ecc384_sha384 (&dme,
		(uint8_t*) &fips_dme_structure.signed_data, sizeof (fips_dme_structure.signed_data),
		rom_shared->firmware.dme_key.Parts.X.AsBytes, rom_shared->firmware.dme_key.Parts.Y.AsBytes,
		ECC_KEY_LENGTH_384, fips_dme_structure.signature.Parts.R.AsBytes,
		fips_dme_structure.signature.Parts.S.AsBytes, HASH_TYPE_SHA384);
	if (status != 0) {
		return status;
	}

	status = identity.get_dme_renewal (&identity, &dme_renewal_counter);
	if (status != 0) {
		return status;
	}

	dme.base.device_oid = DICE_OID_MANTICORE;
	dme.base.dev_oid_length = DICE_OID_MANTICORE_LENGTH;
	dme.base.renewal_counter = (uint8_t*) &dme_renewal_counter;
	dme.base.counter_length = sizeof (dme_renewal_counter);

	status = riot_core_hsp_fips_init_state (&dice);
	if (status != 0) {
		return status;
	}

	status = riot_core_hsp_fips_init_state (&dice_cp);
	if (status != 0) {
		riot_core_hsp_fips_release (&dice);
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_DICE_INIT_END ^ status);

	return status;
}

/**
 * Copy a single DICE key/cert to the shared memory location.
 *
 * @param dest The destination for the DICE key.
 * @param dest_length The maximum length that can be copied.
 * @param length Output for the length of the copied data.  This will be -1 if the key doesn't fit
 * in the destination memory.
 * @param src The DICE key to copy.
 * @param src_length The DICE key length.
 */
static void copy_dice_key (uint8_t *dest, size_t dest_length, int *length, const uint8_t *src,
	size_t src_length)
{
	if (src_length <= dest_length) {
		memcpy (dest, src, src_length);
		*length = src_length;
	}
	else {
		*length = -1;
	}
}

/**
 * Generate the DICE identity keys and certificates for the SPRT firmware.
 *
 * @return 0 if the SPRT DICE keys were generated successfully or an error code.
 */
static int generate_sprt_dice_keys ()
{
	uint64_t svn;
	uint8_t *der;
	size_t length;
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_DICE_START);

	/* CDI is not relevant here. */
	status = dice.base.base.generate_device_id (&dice.base.base, NULL, 0);
	if (status != 0) {
		return status;
	}

	/* Configure the TCB structure for layer 1. */
	svn = manticore_firmware_descriptor_get_svn (&sp1_shared.fw_descriptor);
	buffer_reverse_copy (layer1_svn, (uint8_t*) &svn, sizeof (layer1_svn));

	status = manticore_measurements_generate_sprt_fwid (&sp1_shared.pcr_log.sp, &hash.base,
		&layer1_sprt_fwid);
	if (status != 0) {
		return status;
	}

	status = dice.base.base.generate_alias_key (&dice.base.base, layer1_sprt_fwid.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Derive a seed from the CDI for use during HSM operations. */
	status = ccs_fips.base.derive_key (&ccs_fips.base, DEVICE_KEYS_DICE_CDI,
		&MANTICORE_HSM_FMC_CDI_KDF_CONTEXT, MANTICORE_DEVICE_KEYS_HSM_FMC_CDI_SEED,
		CCS_KSU_ATTR_KDF_KEY_ALLOWED | CCS_KSU_ATTR_KEY_SIZE_384);
	if (status != 0) {
		return status;
	}

	/* Export DICE keys and certs for SPRT. */
	status = dice.base.base.get_device_id_cert (&dice.base.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared.devid_cert_fips, sizeof (sp1_shared.devid_cert_fips),
		&sp1_shared.devid_cert_fips_length, der, length);
	platform_free (der);

	status = dice.base.base.get_device_id_csr (&dice.base.base, NULL, 0, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared.devid_csr_fips, sizeof (sp1_shared.devid_csr_fips),
		&sp1_shared.devid_csr_fips_length, der, length);
	platform_free (der);

	status = dice.base.base.get_alias_key (&dice.base.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared.alias_key, sizeof (sp1_shared.alias_key),
		&sp1_shared.alias_key_length, der, length);
	platform_free (der);

	status = dice.base.base.get_alias_key_cert (&dice.base.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (sp1_shared.alias_cert, sizeof (sp1_shared.alias_cert),
		&sp1_shared.alias_cert_length, der, length);
	platform_free (der);

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_DICE_END ^ status);

	return 0;
}

/**
 * Generate the DICE Alias key and certificate for the CP firmware.
 *
 * @return 0 if the CP Alias key was generated successfully or an error code.
 */
static int generate_cp_dice_keys ()
{
	struct ksu_pcr_slot *pcr = (struct ksu_pcr_slot*) HSP_ADDR_MAP_KSB_PCRS_ADDRESS;
	uint8_t *der;
	size_t length;
	int out_length;
	int status;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CP_DICE_START);

	/* Need to go through the Device ID generation process again in this other context for the CP
	 * Alias key. */
	status = dice_cp.base.base.generate_device_id (&dice_cp.base.base, NULL, 0);
	if (status != 0) {
		return status;
	}

	/* Configure the TCB structure for layer 1.  Only the FWID needs to be updated.  The rest of the
	 * structure is the same as it was for the SP alias key. */
	memcpy (layer1_cp_fwid, (uint32_t*) pcr[3].pcr, SHA384_HASH_LENGTH);

	status = dice_cp.base.base.generate_alias_key (&dice_cp.base.base, layer1_cp_fwid,
		SHA384_HASH_LENGTH);
	if (status != 0) {
		return status;
	}

	/* Export Alias key and cert for CP. */
	status = dice_cp.base.base.get_alias_key (&dice_cp.base.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (cp_shared->alias_key, sizeof (cp_shared->alias_key), &out_length, der, length);
	cp_shared->alias_key_length = out_length;
	platform_free (der);

	status = dice_cp.base.base.get_alias_key_cert (&dice_cp.base.base, &der, &length);
	if (status != 0) {
		return status;
	}

	copy_dice_key (cp_shared->alias_cert, sizeof (cp_shared->alias_cert), &out_length, der, length);
	cp_shared->alias_cert_length = out_length;
	platform_free (der);

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CP_DICE_END ^ status);

	return 0;
}

/**
 * Ensure all the shared data is populated and protected.
 *
 * @return 0 if the shared data is ready for SRPT use or an error code.
 */
static int prepare_shared_data ()
{
	int status;

	/* Copy the ROM log for PCR 1 from shared SRAM to dTCM. */
	memcpy (&sp1_shared.pcr_log.rom, &rom_shared->firmware.pcr_log[1],
		sizeof (sp1_shared.pcr_log.rom));

	sp1_shared.valid_length = MANTICORE_1SP_SHARED_DATA_VALID_LENGTH;

	status = hash.base.calculate_sha384 (&hash.base, sp1_shared.hashed, sizeof (sp1_shared.hashed),
		sp1_shared.digest, sizeof (sp1_shared.digest));
	if (status != 0) {
		return status;
	}

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_LOCK_SHARED_DATA);

	/* Configure MPU to lock write protection on the block of shared memory.  This region contains
	 * keys and other state that SPRT will use to authenticate various actions.  We need to be sure
	 * they can't be modified. */
	status = mpu.base.set_region_attributes (&mpu.base, &sp1_shared, sizeof (sp1_shared),
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_LOCKED);

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_LOCK_SHARED_DATA_DONE);

	return status;
}

/**
 * Encode a single hex digit as an ASCII character.
 *
 * @param value The hex value to to encode.
 *
 * @return The encoded value.
 */
static char ascii_encode_hex_digit (uint8_t value)
{
	if (value < 10) {
		return '0' + value;
	}
	else {
		return 'A' + (value - 10);
	}
}

/**
 * Finalize the CP shared data structure.
 */
static void prepare_cp_shared_data ()
{
	uint8_t *socid = (uint8_t*) ueid;
	size_t i;
	size_t j;

	/* Convert the SOCID to an ASCII hex string for CP consumption. */
	for (i = 0, j = 0; i < sizeof (ueid); i++) {
		cp_shared->socid[j++] = ascii_encode_hex_digit (socid[i] >> 4);
		cp_shared->socid[j++] = ascii_encode_hex_digit (socid[i] & 0xf);
	}

	/* All data to be shared with CP has been written, so release the mapping for the shared
	 * location, which was mapped as part of the firmware loading process. */
	dmb.unmap_soc_address (&dmb, cp_shared);
}

/**
 * Prepare the device for executing the SPRT image.
 *
 * @return 0 if successful, error code otherwise
 */
static int prepare_sprt_execution ()
{
	int status;

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_PREPARE_SPRT);

	status = mpu.base.set_region_attributes (&mpu.base, (const void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS,
		HSP_ADDR_MAP_SP_IRAM_SIZE, MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_WRITE | MPU_PAGE_ATTRIBUTE_EXECUTE);
	if (status != 0) {
		return status;
	}

	/* Do not change dTCM MPU settings at this point to keep MPU protections in place as long as
	 * possible during 1SP execution. */
	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_PREPARE_SPRT_DONE);

	/* Start the watchdog timer now to catch early boot failures in SPRT execution. */
	status = hsp_watchdog_init_timer (&watchdog, HSP_CLOCK_FREQUENCY_HZ,
		MANTICORE_WATCHDOG_TIMEOUT_US);
	if (status != 0) {
		return status;
	}

	return hsp_watchdog_start (&watchdog, false);
}

/**
 * Loop variable for wiping memory regions.
 */
#define erase_addr	((uint32_t**) ((uint32_t) &_start_data + 4))

/**
 * Exit 1SP and start execution of SPRT firmware.  Any configuration or cleanup that needs to wait
 * until the very end of 1SP execution will be done here before jumping to SPRT.
 *
 * Nothing that is done here is expected to fail, and this function will not return.
 */
static void jump_to_sprt ()
{
	code_path_integrity_secure_message_no_trace (0);	// Need an even number of messages.
	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_FINISH);

	/* Grant full access to all of DTCM prior to jumping to SPRT to ensure proper access during
	 * run-time initialization.  Execute permissions are still needed so 1SP can finish running.
	 * SPRT will set an appropriate configuration later. */
	mpu.base.set_region_attributes (&mpu.base, (void*) HSP_ADDR_MAP_SP_DRAM_ADDRESS,
		HSP_ADDR_MAP_SP_DRAM_SIZE - TOTAL_SHARED_DATA_MEMORY,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_WRITE | MPU_PAGE_ATTRIBUTE_EXECUTE);

	/* Wipe all memory used during 1SP execution.  But first make a copy of the load address for the
	 * SPRT image so we know where to jump to.
	 *
	 * Stack variables are avoided in these loops since we are clearing the stack along with the
	 * rest of 1SP memory. */
	_start_data = manticore_firmware_descriptor_sp_reset_vector (&sp1_shared.fw_descriptor);

	*erase_addr = MANTICORE_CRYPTO_SHARED_SRAM_START;
	while (*erase_addr < MANTICORE_CRYPTO_SHARED_SRAM_END) {
		**erase_addr = 0;
		*erase_addr += 1;
	}

	/* Wipe SRAM used for run-time data and stack, leaving only the SPRT load address available. */
	*erase_addr = (uint32_t*) ((uint32_t) &_start_data + 8);
	while (*erase_addr < MANTICORE_1SP_DATA_END) {
		**erase_addr = 0;
		*erase_addr += 1;
	}

	/* Clear all CPU registers and reset the trap vector and the vector table pointer. */
	__asm__ (
		"li ra, 0x0\n"
		"li sp, 0x0\n"
		"li gp, 0x0\n"
		"li tp, 0x0\n"
		"li t0, 0x0\n"
		"li t1, 0x0\n"
		"li t2, 0x0\n"
		"li t3, 0x0\n"
		"li t4, 0x0\n"
		"li t5, 0x0\n"
		"li t6, 0x0\n"
		"li s0, 0x0\n"
		"li s1, 0x0\n"
		"li s2, 0x0\n"
		"li s3, 0x0\n"
		"li s4, 0x0\n"
		"li s5, 0x0\n"
		"li s6, 0x0\n"
		"li s7, 0x0\n"
		"li s8, 0x0\n"
		"li s9, 0x0\n"
		"li s10, 0x0\n"
		"li s11, 0x0\n"
		"li a0, 0x0\n"
		"li a1, 0x0\n"
		"li a2, 0x0\n"
		"li a3, 0x0\n"
		"li a4, 0x0\n"
		"li a5, 0x0\n"
		"li a6, 0x0\n"
		"li a7, 0x0\n"
	);
	WRITE_CSR (mtvec, 0);
	WRITE_CSR_VALUE (RISCV_CSR_MTVT, 0);

	/* Exit 1SP and jump to the loaded FW. */
	__asm__ (
		"la s0, _start_data\n"	// Get the pointer for the SPRT entry address.
		"lw s0, 0(s0)\n"		// Deference the pointer.
		"jalr s0\n"				// Jump to SPRT.
	);

	/* We should never get to this point, but if we do, just halt the processor and wait for a
	 * reset.  We've blown away the stack and registers so we can't return. */
	CEASE;
}

/**
 * Trigger a reset of the device due to an error condition with checkpoint management during boot.
 *
 * Note:  In most cases of checkpoint failure, this function will not get called.  The checkpoint
 * hardware will trigger it's own fatal error.
 *
 * @param status Error code of the checkpoint failure.
 */
static void checkpoint_error_reset (int status)
{
	if (debug_log == NULL) {
		/* Dump errors to UART if they happen before the debug log has been initialized. */
		platform_printf ("CHKPT: %x" NEWLINE, status);
	}
	else {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_BOOT,
			BOOT_LOGGING_UNEXPECTED_EXECUTION, status, 0);
		debug_log_flush ();
	}

	boot_error_reset ();
}


/**
 * Driver for the checkpoint driver to enforce expected code execution.
 */
static const struct checkpoint chkpt =
	checkpoint_static_init (
	(struct Creg_regs_chkpt_regs*) HSP_ADDR_MAP_CREG_CHKPT_REGS_CREG_CHKPT_GROUP_ADDRESS);


/**
 * Singleton instance for handling all code execution tracing and enforcement.
 */
CODE_PATH_INTEGRITY_HANDLER (chkpt_cpi, &rng.base, &chkpt, &fuses.base, checkpoint_error_reset);


/**
 * Entry point for Manticore 1SP.
 */
void main ()
{
	uint32_t random_val = 0;
	const uint32_t *socid = (const uint32_t*) HSP_ADDR_MAP_GFC_SOCID_ADDRESS;
	int error_msg = -1;
	int error_sev = DEBUG_LOG_SEVERITY_ERROR;
	int status;

	code_path_integrity_checkpoint_start (&MANTICORE_1SP_CHECKPOINT_START);

	hsp_trap_init (false, 0);
	traps_init_exception_catch ();
	determine_hsp_clock_frequency ();
	HspUartInitializeEx (HSP_CLOCK_FREQUENCY_HZ, 115200);
	configure_gpios (&gpio);
	configure_error_boot_order ();

	/* Disable AXI watchdog fatal error to prevent possible lock-up conditions when accessing
	 * APB-connected components in the SoC, such as the POR registers.
	 *
	 * CREG is known to be a direct memory access local to HSP, so skipping the map/unmap call.
	 * It also won't fail. */
	creg_regs.base.write32 (&creg_regs.base,
		offsetof (struct Creg_regs, wdt_regs) + CREG_REGS_WDT_REGS_WDT_ERR_EN_OFFSET, 0);

	/* Cache the SOCID in a byte-addressable buffer to allow general purpose use of the data by
	 * firmware. */
	memcpy (ueid, socid, sizeof (ueid));

	enable_emc_errors (ueid);

	/* Attempt to pause all arm cores if running at this time by triggering a tcon wakeup
	 * timer interrupt.  This also initializes the DMB driver state needed by reset_cpu_cores. */
	status = tcon_wakeup_timer_interrupt ();
	if (status != 0) {
		platform_printf ("Trigger TCON FAILED: 0x%x" NEWLINE, status);
	}

	/* Immediately reset the SoC cores to prevent them from executing during 1SP operations.  This
	 * needs to be done before the crypto engines are engaged and used, including RNG. */
	status = reset_cpu_cores ();
	if (status != 0) {
		platform_printf ("Reset Cores FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_ENABLE_CRYPTO);

	status = initialize_rng ();
	if (status != 0) {
		platform_printf ("RNG FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_RNG_INIT ^ status);

	/* Initialize the 1SP version string and boot time start. */
	memset (&sp1_shared, 0, sizeof (sp1_shared));
	platform_init_current_tick (&sp1_shared.start_time_1sp);
	initialize_1sp_version_str ();

	platform_printf (NEWLINE);
	platform_printf ("Manticore Boot: %s" NEWLINE,
		(MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_INTA) ? "Internal" : "External");
	platform_printf ("1SP: %s" NEWLINE, sp1_shared.version_1sp);
	platform_printf ("SOCID: %x.%x.%x.%x" NEWLINE, ueid[0], ueid[1], ueid[2], ueid[3]);
	platform_printf ("Boot Source: 0x%x" NEWLINE, MANTICORE_BOOT_SOURCE);
	platform_printf ("Boot Order: 0x%x" NEWLINE, MANTICORE_BOOT_ORDER);
	platform_printf ("Scratch Reg 0: 0x%x" NEWLINE, MANTICORE_HSP_SCRATCH0_REG);
	platform_printf ("Sticky Reg 1: 0x%x" NEWLINE, MANTICORE_HSP_STICKY1_REG);
	platform_printf ("Scratch Reg 1: 0x%x" NEWLINE, MANTICORE_HSP_SCRATCH1_REG);

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOT_MESSAGES);

	status = initialize_and_increment_reset_counter ();
	if (status != 0) {
		platform_printf ("Reset Ctr FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_RESET_CNTR_INIT ^ status);

	status = hsp_aeb_init_state (&aeb);
	if (status != 0) {
		platform_printf ("AEB Init FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_AEB_INIT ^ status);

	/* This doesn't impact any execution prior to this point and needs to be done after the AEB
	 * driver has been initialized. */
	handle_silicon_errata (&aeb, ueid[0], &MANTICORE_1SP_CHECKPOINT_SILICON_ERRATA,
		&MANTICORE_1SP_CHECKPOINT_SILICON_ERRATA_DONE);

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SILICON_ERRATA);

	status = initialize_manticore_flash ();
	if (status != 0) {
		platform_printf ("SPI Flash Init FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FLASH_INIT ^ status);

	initialize_log ();

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_LOG_INIT);

	if (!is_dice_valid ()) {
		platform_printf ("Regenerate DICE key" NEWLINE);
		status = rom_shared->firmware.pcr_log[1].security_state.data.reset_type;
		error_msg = BOOT_LOGGING_INCORRECT_DICE_KEY;
		error_sev = DEBUG_LOG_SEVERITY_INFO;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_DICE_VALID ^ status);

	status = initialize_crypto ();
	if (status != 0) {
		platform_printf ("Crypto Init FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_INIT_HW_CRYPTO;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CRYPTO_INIT_DONE ^ status);

	status = fips_integrity_check_running_image ();
	if (status != 0) {
		platform_printf ("1SP integrity check FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_INTEGRITY_CHECK_FAIL;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_INTEGRITY_DONE ^ status);

	status = run_crypto_self_tests ();
	if (status != 0) {
		platform_printf ("Crypto self-test FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_CRYPTO_KAT;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FIPS_CAST_DONE ^ status);

	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_CONFIG_1SP_MPU);

	/* Apply a random value to the stack guard.  It can't be done in the context of a function
	 * call.  This also needs to happen after the integrity check since changing the stack guard
	 * will change the image digest.  Unfortunately, this also delays configuration of the MPU.*/
	hsp_rng_hw_get_random_word (&rng_hw, &random_val);
	__stack_chk_guard = random_val;

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_STACK_GUARD_SET);

	/* MPU needs to be configured after the stack guard is configured, since that resides in memory
	 * that will be made read-only. */
	status = sp1_mem_protect.base.configure_hsp_mpu (&sp1_mem_protect.base);
	if (status != 0) {
		platform_printf ("MPU config FAILED: 0x%x" NEWLINE, status);
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MPU_CONFIGURED ^ status);

	code_path_integrity_secure_message_no_trace (0);	// Need an even number of messages.
	code_path_integrity_checkpoint (&MANTICORE_1SP_CHECKPOINT_CONFIG_1SP_MPU_DONE);

	status = initialize_dice ();
	if (status != 0) {
		platform_printf ("DICE Init FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_INIT_RIOT_CORE;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_DICE_INIT_DONE ^ status);

	status = initialize_security_policy ();
	if (status != 0) {
		platform_printf ("Sec Policy Init FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_INIT_SECURITY_POLICY;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SEC_POLICY_INIT_DONE ^ status);

	status = initialize_bootloader ();
	if (status != 0) {
		platform_printf ("Bootloader Init FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_APP_LOAD;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_BOOTLOADER_INIT_DONE ^ status);

	/* Get the start time of load_main_firmware */
	platform_init_current_tick (&sp1_shared.start_time_1sp_load_main_fw);

	status = load_main_firmware ();
	if (status != 0) {
		platform_printf ("Firmware Load FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_APP_LOAD;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_FW_LOAD_DONE ^ status);

	/* Get the end time of load_main_firmware */
	platform_init_current_tick (&sp1_shared.finish_time_1sp_load_main_fw);

	status = generate_measurement_logs_and_extend_pcrs ();
	if (status != 0) {
		platform_printf ("Firmware measurements FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_INITIALIZE_PCRS;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_DONE ^ status);

	/* Get the start time of SPRT & CP dice key generation */
	platform_init_current_tick (&sp1_shared.start_time_1sp_gen_dice_key);

	status = generate_sprt_dice_keys ();
	if (status != 0) {
		platform_printf ("SPRT DICE Keys FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_RIOT_KEYS;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_DICE_DONE ^ status);

	status = generate_cp_dice_keys ();
	if (status != 0) {
		platform_printf ("CP DICE Keys FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_RIOT_KEYS;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CP_DICE_DONE ^ status);

	/* Get the end time of SPRT & CP dice key generation */
	platform_init_current_tick (&sp1_shared.finish_time_1sp_gen_dice_key);

	status = riot_core_hsp_fips_release (&dice);
	if (status != 0) {
		platform_printf ("SPRT DICE Release FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_RIOT_KEYS;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_DICE_RELEASE ^ status);

	status = riot_core_hsp_fips_release (&dice_cp);
	if (status != 0) {
		platform_printf ("CP DICE Release FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_RIOT_KEYS;
		goto error;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CP_DICE_RELEASE ^ status);

	status = apply_security_configuration ();
	if (status != 0) {
		platform_printf ("Apply security config FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_APPLY_SECURITY_CONFIG;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_APPLY_SECURITY_CFG ^ status);

	status = update_pcrs_with_aeb_state ();
	if (status != 0) {
		platform_printf ("AEB measurements FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_UPDATE_PCRS;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_MEASURE_AEB_DONE ^ status);

	status = prepare_shared_data ();
	if (status != 0) {
		platform_printf ("Shared data structure FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_RIOT_KEYS;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_SHARED_DATA_READY ^ status);

	prepare_cp_shared_data ();
	debug_log_flush ();

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_CP_SHARED_DATA_READY);

	status = prepare_sprt_execution ();
	if (status != 0) {
		platform_printf ("SPRT preparation FAILED: 0x%x" NEWLINE, status);
		error_msg = BOOT_LOGGING_PREPARE_MAIN_EXECUTION;
		goto error_no_dice;
	}

	code_path_integrity_secure_message_no_trace (CHKPT_1SP_SPRT_READY ^ status);

	jump_to_sprt ();
	platform_printf ("WHAT!?" NEWLINE);

error:
	riot_core_hsp_fips_release (&dice);
	riot_core_hsp_fips_release (&dice_cp);

error_no_dice:
	if (error_msg >= 0) {
		debug_log_create_entry (error_sev, DEBUG_LOG_COMPONENT_BOOT, error_msg, status, 0);
		debug_log_flush ();
	}

	boot_error_reset ();

	/* Should never get here.  Just wait for a reset. */
	CEASE;
}
