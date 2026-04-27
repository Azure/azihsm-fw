// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "init_attestation.h"
#include "init_cmd.h"
#include "init_crashdump.h"
#include "init_crypto.h"
#include "init_error.h"
#include "init_firmware.h"
#include "init_flash.h"
#include "init_host.h"
#include "init_intrusion.h"
#include "init_ipc.h"
#include "init_log.h"
#include "init_log_flush_handlers.h"
#include "init_manifest.h"
#include "init_system.h"
#include "manticore_soc_rev.h"
#include "manticore_sticky_regs.h"
#include "pcie_phy.h"
#include "periodic_task_freertos_static.h"
#include "platform_config.h"
#include "platform_io_api.h"
#include "reset_counter_init.h"
#include "soc_shared.h"
#include "sp_boot.h"
#include "system_observer_stack_usage.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "traps.h"
#include "asn1/ecc_der_util.h"
#include "cmd_interface/authorized_execution_config_reset_static.h"
#include "common/array_size.h"
#include "common/auth_token_static.h"
#include "common/authorization_allowed_static.h"
#include "common/authorization_challenge_static.h"
#include "common/authorization_global_static.h"
#include "common/authorized_data_token_only_static.h"
#include "common/authorized_data_with_aad_static.h"
#include "common/buffer_util.h"
#include "crypto/hash.h"
#include "crypto/signature_verification_ecc_static.h"
#include "drivers/soc_reset_control_static.h"
#include "firmware/authorized_execution_prepare_firmware_update_static.h"
#include "firmware/manticore_device_keys.h"
#include "firmware/manticore_fw_keys.h"
#include "logging/init_logging.h"
#include "mpu/fence_manticore_static.h"
#include "mpu/hsp_mpu_static.h"
#include "mpu/memory_protection_manticore_sprt_static.h"
#include "riot/authorized_execution_identity_renewal_static.h"
#include "rma/authorized_execution_rma_static.h"
#include "rma/device_rma_transition_hsp_retest_static.h"
#include "rma/rma_unlock_token_static.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "sprt/manticore_sprt.h"
#include "sprt/system/heartbeat_led_handler_static.h"
#include "system/hsp_watchdog_handler_static.h"
#include "telemetry/telemetry_pcie_handler_static.h"
#include "telemetry/telemetry_temperature_handler_static.h"
#include "trap/hsp_trap.h"


/**
 * The default amount of time allowed for draining CP and FP operations during a graceful shutdown.
 * This value can be overridden at run-time via a command.
 */
#define	MANTICORE_SHUTDOWN_DEFAULT_DRAIN_TIME_MS		5000

/**
 * The base amount of time that SP should wait for an IPC response during a graceful shutdown.
 */
#define	MANTICORE_SHUTDOWN_IPC_BASE_TIMEOUT_MS			500

/**
 * The lower temperature threshold for the middle die sensor below which operation should be
 * considered abnormal. The value is in hundredths of degree celsius.
 */
#define MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_LOWER_THRESHOLD		(2 * 100)

/**
 * The higher temperature threshold for the middle die sensor above which operation should be
 * considered abnormal. The value is in hundredths of degree celsius.
 */
#define MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_HIGHER_THRESHOLD	(103 * 100)

/**
 * The higher temperature threshold for the middle die sensor above which operation should be
 * considered abnormal. The value is in hundredths of degree celsius.
 */
#define MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_MAX_DELTA			(0.5 * 100)


/**
 * Data populated by 1SP.
 */
const struct manticore_1sp_shared_data *const sp1_shared =
	(struct manticore_1sp_shared_data*) SP1_SHARED_ADDRESS;

/**
 * Misc SW registers that can be used with local static initialization.
 */
static struct Creg_regs_misc_creg_sw_regs *const sw_regs_static =
	(struct Creg_regs_misc_creg_sw_regs*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_ADDRESS;

/**
 * Data populated by ROM that can be used with local static initialization.
 */
static struct manticore_rom_shared_sram *const rom_shared_static =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * Location in shared SRAM for hardware crypto buffers.
 */
static struct manticore_sprt_shared_sram_crypto *const crypto_cmd_static =
	(struct manticore_sprt_shared_sram_crypto*) &rom_shared_static->internal;

/**
 * Register mapping for the entire CREG address space.
 */
const struct mmio_register_block_hsp creg_regs =
	mmio_register_block_hsp_static_init ((uint32_t*) HSP_ADDR_MAP_CREG_ADDRESS,
	sizeof (struct Creg_regs));

/**
 * Variable context for the heartbeat LED handler.
 */
static struct heartbeat_led_handler_state heartbeat_context;

/**
 * Periodic handler for the heartbeat LED on the DC-SCM.
 */
static const struct heartbeat_led_handler heartbeat =
	heartbeat_led_handler_static_init (&heartbeat_context, &gpio);

/**
 * CREG timer driver for hardware watchdog functionality.  The timer is enabled by 1SP, so does not
 * need to be re-enabled here.
 */
const struct hsp_watchdog watchdog = hsp_watchdog_static_init (&creg_regs.base,
	offsetof (struct Creg_regs, timer0_regs), offsetof (struct Creg_regs, fatal_err_regs));

/**
 * Variable context for the watchdog refresh handler.
 */
static struct hsp_watchdog_handler_state watchdog_context;

/**
 * Handler to refresh the watchdog timer.  The refresh rate will be a quarter of the time before
 * reaching a HW fatal error.
 */
static const struct hsp_watchdog_handler watchdog_handler =
	hsp_watchdog_handler_static_init_refresh_only (&watchdog_context, &watchdog,
	MANTICORE_WATCHDOG_TIMEOUT_US / 4000);

/**
 * List of watchdog and monitoring handlers.
 */
static const struct periodic_task_handler *const watchdog_handlers[] = {
	&heartbeat.base, &watchdog_handler.base, &soc_handler.base, &periodic_self_test_handler.base
};

/**
 * Variable context for the watchdog and crash monitoring task.
 */
static struct periodic_task_freertos_state watchdog_task_context;

/**
 * Task to refresh the hardware watchdog and monitor for SoC crashes.
 */
static const struct periodic_task_freertos watchdog_task =
	periodic_task_freertos_static_init (&watchdog_task_context, watchdog_handlers,
	ARRAY_SIZE (watchdog_handlers), WATCHDOG_TASK_LOG_ID);

/**
 * Statically allocated task control block for the SP watchdog task.
 */
static StaticTask_t watchdog_task_tcb;

/**
 * Statically allocated stack for the SP watchdog task.
 */
static StackType_t watchdog_task_stack[WATCHDOG_TASK_STACK_WORDS];

/**
 * List of FW version strings.
 */
const char *const fw_version_list[6] = {
	sp1_shared->version_sprt,
	sp1_shared->version_1sp,
	"M1244265-004 B0",	/* There is only one production version of HW, so just return a fixed value. */
	sp1_shared->service_indicator,
	sp1_shared->idfu_version,
	"Microsoft HSM Cryptographic Module"
};

/**
 * Container for FW version data.
 */
const struct cmd_interface_fw_version firmware_version = {
	.count = ARRAY_SIZE (fw_version_list),
	.id = fw_version_list
};

/**
 * SOCID value used to determine the hardware revision of the device.
 */
#ifndef MANTICORE_ENABLE_A0_SUPPORT
static const uint32_t soc_revision = MANTICORE_ROM_B0_SOCID_TAG;
#else
uint32_t soc_revision;
#endif

/**
 * Flag indicating the system booted from the recovery flash.
 */
SECTION (".sprtro.recovery_boot")
bool recovery_boot;


/**
 * Source of the most recent chip reset.
 */
SECTION (".sprtro.reset_source")
int reset_source;

/**
 * Context for the fuse controller driver.
 */
static struct fuse_controller_state fuse_context;

/**
 * Driver for the HSP fuse controller.
 */
const struct fuse_controller fuses = fuse_controller_manticore_fips_static_init (&fuse_context,
	(struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS);

/**
 * Context for the RNG driver.
 */
static struct hsp_rng_hw_state rng_context;

/**
 * Driver for the HSP random number generator.
 */
const struct hsp_rng_hw rng_hw = hsp_rng_hw_static_init (&rng_context,
	(struct Rng_regs*) HSP_ADDR_MAP_RNG_ADDRESS, &fuses.base, MANTICORE_ROM_MIN_RNG_CLOCK_DIVIDER,
	MANTICORE_ROM_MAX_RNG_CLOCK_DIVIDER);

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
 * Variable context for managing system state.
 */
static struct state_manager_state system_state_context;

/**
 * State information for the system.
 */
const struct state_manager system_state = system_state_manager_static_init (&system_state_context,
	&flash_internal.base, SYSTEM_STATE_ADDR);

/**
 * The total amount of heap space allocated.
 *
 * TODO:  Make this a variable based on linker output.  This would also need to get consumed by
 * FreeRTOS for heap initialization.
 */
static const uint32_t heap_size = configTOTAL_HEAP_SIZE;

/**
 * The interface for device operations.
 */
const struct cmd_device_hsp_freertos device_cmd =
	cmd_device_hsp_freertos_static_init (&reset_count, (struct Gfc_regs*) HSP_ADDR_MAP_GFC_ADDRESS,
	sw_regs_static, &heap_size);

/**
 * Memory fencing registers block state
 */
struct mmio_register_block_soc_state fence_registers_block_state = {};

/**
 * Memory fencing registers block
 */
static const struct mmio_register_block_soc fence_registers_block =
	mmio_register_block_soc_static_init (&fence_registers_block_state, &dmb,
	FENCE_REGISTER_BLOCK_ADDRESS_BEGIN,
	FENCE_REGISTER_BLOCK_ADDRESS_END - FENCE_REGISTER_BLOCK_ADDRESS_BEGIN);

/**
 * Manticore memory fencing driver
 */
static const struct fence_manticore manticore_fence =
	fence_manticore_static_init (&fence_registers_block.base);

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
 * Linker output that marks the start of memory to mark as read-only after initialization has
 * completed.
 */
extern uint32_t _ro_init_start;

/**
 * Memory regions that should be configured for run-time protection using the HSP MPU.
 */
static const struct memory_protection_mpu_only_region sprt_mpu_regions[] = {
	{
		/* Memory region containing the executable firmware image (RX). */
		.start = &_exe_start,
		.end = &_ro_start,
		.protection_level = MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		.page_attributes = MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_EXECUTE |
			MPU_PAGE_ATTRIBUTE_LOCKED
	},
	{
		/* Memory region containing constant data (R).  This consumes the rest of iTCM. */
		.start = &_ro_start,
		.end = (const void*) HSP_ADDR_MAP_SP_IRAM_ADDRESS + HSP_ADDR_MAP_SP_IRAM_SIZE,
		.protection_level = MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
#ifndef MANTICORE_ENABLE_FIPS_CMVP_TESTING
		.page_attributes = MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_LOCKED
#else
		/* For CMVP testing, this region needs to be writable to corrupt KAT vectors. */
		.page_attributes = MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_WRITE
#endif
	},
	{
		/* Memory region containing mutable data (RW). */
		.start = &_start_data,
		.end = &_ro_init_start,
		.protection_level = MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		.page_attributes = MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_WRITE |
			MPU_PAGE_ATTRIBUTE_LOCKED
	}

	/* The region containing RO data that first needs to be initialized cannot be part of this list
	 * and must be handled separately. */
};

/**
 * Handler for configuring memory protections for SPRT execution and SoC fences.
 */
static const struct memory_protection_manticore_sprt sprt_memory_protect =
	memory_protection_manticore_sprt_static_init (&manticore_fence.base, &mpu.base,
	sprt_mpu_regions, ARRAY_SIZE (sprt_mpu_regions));

/**
 * Variable context for handling a graceful SoC shutdown.
 */
static struct graceful_shutdown_state graceful_shutdown_context;

/**
 * Handler to coordinate a graceful shutdown of all the cores.
 */
const struct graceful_shutdown graceful_shutdown =
	graceful_shutdown_static_init (&graceful_shutdown_context, &device_cmd.base.base,
	&ipc_hsp_to_admin_channel, &log_flush, MANTICORE_STICKY_REG (MANTICORE_SHUTDOWN_INDICATOR),
	MANTICORE_SHUTDOWN_IPC_BASE_TIMEOUT_MS, MANTICORE_SHUTDOWN_DEFAULT_DRAIN_TIME_MS);


/**
 * The amount of GSRAM allocated per IPC channel.  Each IPC includes both a Tx and Rx queue, along
 * with producer and consumer indicies for both queues.
 *
 * This uses the Admin -> HSM IPC for size calculation, but all IPC channels are the same size.
 */
#define	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE     \
	(GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_TX_QUEUE_PI_SIZE + \
	GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_TX_QUEUE_CI_SIZE + \
	GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_TX_QUEUE_SIZE + \
	GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_RX_QUEUE_PI_SIZE + \
	GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_RX_QUEUE_CI_SIZE + GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_RX_QUEUE_SIZE)

/**
 * Size of the temporary verification buffer to use for PHY firmware verification.  This must be
 * 4-byte aligned and will be allocated and freed from the heap early during the initialization
 * process.
 */
#define	MANTICORE_PHY_FW_VERIFICATION_BUFFER_SIZE		(16 * 1024)

/**
 * The list of IPC channels and other regions in GSRAM that need to be erased prior to releasing
 * the cores after a graceful reset.
 *
 * Each IPC channel is listed individually to ensure the channels will be cleared correctly even
 * if they are not contiguous in GSRAM.  This assumes that all components for every channel are
 * contiguous in GSRAM and that every channel uses the same amount of memory.
 */
static const struct soc_sram_block GRACEFUL_RESET_ERASE_SRAM[] = {
	{
		.start = CP_SHARED_GSRAM_ADDRESS + offsetof (struct cp_shared_data, cp_logger_lock),
		.length = sizeof (uint32_t)
	},
	{
		.start = GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_TX_QUEUE_PI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
	{
		.start = GSRAM_MEM_MAP_HSM_TO_ADMIN_IPC_TX_QUEUE_CI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
	{
		.start = GSRAM_MEM_MAP_ADMIN_TO_HSP_IPC_TX_QUEUE_CI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
	{
		.start = GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_TX_QUEUE_CI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
	{
		.start = GSRAM_MEM_MAP_HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
	{
		.start = GSRAM_MEM_MAP_HSM_TO_HSP_IPC_TX_QUEUE_CI,
		.length = MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE,
	},
};

/**
 * Regions of SoC SRAM to be wiped after non-graceful resets.
 */
static const struct soc_sram_block NON_GRACEFUL_RESET_ERASE_SRAM[] = {
	{
		/* This is the region of GSRAM before the HSP shared region, which is likely to be zero
		 * length. */
		.start = MANTICORE_SOC_GSRAM_ADDRESS,
		.length = CP_SHARED_GSRAM_ADDRESS - MANTICORE_SOC_GSRAM_ADDRESS
	},
	{
		/* While most of cp_shared_data needs to always be preserved, the logger lock needs to
		 * always be cleared. */
		.start = CP_SHARED_GSRAM_ADDRESS + offsetof (struct cp_shared_data, cp_logger_lock),
		.length = sizeof (uint32_t)
	},
	{
		/* This is all of GSRAM following the block of preserved data excluding the HSM core
		 * persistent store and key vault. */
		.start = CP_SHARED_GSRAM_ADDRESS + sizeof (struct cp_shared_data),
		.length = MANTICORE_SOC_GSRAM_SIZE - sizeof (struct cp_shared_data) -
			(CP_SHARED_GSRAM_ADDRESS - MANTICORE_SOC_GSRAM_ADDRESS) -
			GSRAM_MEM_MAP_HSM_PART_PERSISTENT_STORE_SIZE - GSRAM_MEM_MAP_KEY_VAULT_SIZE
	},
	{
		/* HSM Key vault area in GSRAM. */
		.start = GSRAM_MEM_MAP_KEY_VAULT,
		.length = GSRAM_MEM_MAP_KEY_VAULT_SIZE
	},
	{
		.start = MANTICORE_SOC_PSRAM_ADDRESS,
		.length = MANTICORE_SOC_PSRAM_SIZE
	},
};

/**
 * Regions of SoC SRAM that should be cleared as part of the RETEST transition.
 */
static const struct soc_sram_block RMA_ERASE_SRAM[] = {
	{
		.start = 0x60000000,	/* CP ITCM */
		.length = (512 * 1024)
	},
	{
		.start = 0x60200000,	/* CP0 DTCM */
		.length = (256 * 1024)
	},
	{
		.start = 0x60600000,	/* CP1 DTCM */
		.length = (256 * 1024)
	},
	{
		.start = 0xa2000000,	/* FP0 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3000000,	/* FP0 DTCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa2200000,	/* FP1 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3200000,	/* FP1 DTCM */
		.length = (64 * 1024)
	},
	{
		.start = 0xa2400000,	/* FP2 ITCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3400000,	/* FP2 DTCM */
		.length = (32 * 1024)
	},
	{
		.start = 0xa3020000,	/* FP0-FP1 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3030000,	/* FP0-FP2 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3220000,	/* FP1-FP2 DTCM */
		.length = (16 * 1024)
	},
	{
		.start = 0xa3e00000,	/* PSRAM */
		.length = (32 * 1024)
	},
	{
		.start = 0x61000000,	/* GSRAM */
		.length = (2 * 1024 * 1024)
	}
};


/* Try to catch any changes to the relative ordering of the IPC channel elements in GSRAM.  If any
 * of these asserts fail, the list of IPC channels to clear needs to be updated with a new start
 * address. */
_Static_assert (((GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_TX_QUEUE_PI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE - GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_ADMIN_TO_HSM_IPC_RX_QUEUE), "Admin to HSM Queue Changed ordering");

_Static_assert (((GSRAM_MEM_MAP_ADMIN_TO_HSP_IPC_TX_QUEUE_CI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE - GSRAM_MEM_MAP_ADMIN_TO_HSP_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_ADMIN_TO_HSP_IPC_RX_QUEUE), "Admin to HSP Queue Changed ordering");

_Static_assert (((GSRAM_MEM_MAP_HSM_TO_HSP_IPC_TX_QUEUE_CI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE - GSRAM_MEM_MAP_HSM_TO_HSP_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_HSM_TO_HSP_IPC_RX_QUEUE), "HSM to HSP Queue Changed ordering");

_Static_assert (((GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_TX_QUEUE_CI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE - GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_RX_QUEUE), "HSP to Admin Queue Changed ordering");

_Static_assert (((GSRAM_MEM_MAP_HSM_TO_ADMIN_IPC_TX_QUEUE_CI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE - GSRAM_MEM_MAP_HSM_TO_ADMIN_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_HSM_TO_ADMIN_IPC_RX_QUEUE), "HSM to Admin Queue Changed ordering");

_Static_assert (((GSRAM_MEM_MAP_HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI +
	MANTICORE_SINGLE_IPC_CHANNEL_GSRAM_SIZE -
	GSRAM_MEM_MAP_HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_SIZE) ==
	GSRAM_MEM_MAP_HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE),
	"HSP to Admin Stop Interface Queue Changed ordering");

/**
 * Variable context for managing execution of the ARM cores in the SoC.
 */
static struct soc_reset_control_state arm_reset_ctrl_context;

/**
 * Reset and execution control for for the ARM cores in the SoC.
 */
static const struct soc_reset_control arm_reset_ctrl =
	soc_reset_control_static_init (&arm_reset_ctrl_context, &dmb);


/**
 * Manager for system operations.
 *
 * TODO:  Create a static initializer for this type.  In the meantime, mark as RO after init.
 */
SECTION (".sprtro.system_mgr")
struct system system_mgr;

/**
 * Security policy handler for the device.
 *
 * Need to cast away the 'const' on the security policy for compilation, but the implementation
 * ensures the data will not get modified.  It's also been write-protected by 1SP.
 *
 * Defined as a weak reference to allow it to be overridden in the recovery image.
 */
const struct security_policy_hsp_manticore __attribute__((weak)) sec_policy =
	security_policy_hsp_manticore_static_init_constant_policy (
	(struct security_policy_hsp_manticore_data*) &sp1_shared->sec_policy, &soc_revision);

/**
 * Variable context for the device security manager.
 */
static struct security_manager_hsp_manticore_state sec_manager_context;

/**
 * Manager for the system security policy and unlock operations.
 */
const struct security_manager_hsp_manticore security_mgr =
	security_manager_hsp_manticore_static_init_only_config_unlock (&sec_manager_context,
	&sec_policy.base, &fuses.base, HSP_FUSES_ADDRESS (RSVD1), MANTICORE_1SP_UNLOCK_COUNTER_LENGTH,
	&shared_hash.base, &ccs.base, MANTICORE_DEVICE_KEYS_UNLOCK_HMAC_KEY, crypto_cmd_static->unlock,
	MANTICORE_UNLOCK_HMAC_BUFFER_SIZE, &keystore_flash.base, DEVICE_UNLOCK_POLICY);

/* Global pointer to the default security policy instance. */
const struct security_policy *const default_policy = &sec_policy.base.base;

/**
 * Handler to display stack usage information on system reset.
 *
 * TODO:  Create a static initializer for this type.
 */
static struct system_observer_stack_usage stack_usage;

/**
 * Number of supported operations requiring authorization through a challenge.
 */
#define	MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT			8

/**
 * Number of supported operations requiring global authorization.
 */
#define	MANTICORE_AUTH_OPERATIONS_GLOBAL_COUNT				1

/**
 * Indices for authorization signature verification instances.
 */
enum {
	MANTICORE_AUTH_REVERT_BYPASS = 0,	/**< Revert bypass token signature. */
	MANTICORE_AUTH_FACTORY_DEFAULT = 1,	/**< Factory default token signature. */
	MANTICORE_AUTH_CLEAR_PCD = 2,		/**< Clear PCD token signature. */
	MANTICORE_AUTH_INTRUSION_RESET = 3,	/**< Intrusion reset token signature. */
	MANTICORE_AUTH_RMA = 4,				/**< RMA token signature. */
	MANTICARE_AUTH_CLEAR_CERTS = 5,		/**< Clear certificates token signature. */
	MANTICORE_AUTH_REVOKE_DICE = 6,		/**< Revoke DICE identity token signature. */
	MANTICORE_AUTH_REVOKE_DME = 7,		/**< Revoke DME key token signature. */
	MANTICORE_AUTH_FW_UPDATE = 8,		/**< Firmware update token signature. */

	/**
	 * Total number of operations requiring authorization with signature verification.
	 */
	MANTICORE_AUTH_OPERATIONS_VERIFICATION_COUNT,
};

/**
 * Variable context for request authorization signature verification.
 */
static struct signature_verification_ecc_state
	auth_verification_context[MANTICORE_AUTH_OPERATIONS_VERIFICATION_COUNT];

/**
 * Signature verification for request authorization.
 */
static const struct signature_verification_ecc
	auth_verification[MANTICORE_AUTH_OPERATIONS_VERIFICATION_COUNT] = {
	signature_verification_ecc_static_init (&auth_verification_context[0], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[1], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[2], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[3], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[4], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[5], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[6], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[7], &shared_ecc.base),
	signature_verification_ecc_static_init (&auth_verification_context[8], &shared_ecc.base),
};


/**
 * Variable context for the authorization token handlers.
 */
static struct auth_token_state
	auth_challenge_token_context[MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT];


/**
 * Maximum length of a device unlock token generated by Manticore.
 */
#define	MANTICORE_MAX_AUTH_CHALLENGE_TOKEN_LENGTH	(AUTHORIZATION_CHALLENGE_TAG_LENGTH + \
	AUTHORIZATION_CHALLENGE_NONCE_LENGTH + ECC_DER_P384_ECDSA_MAX_LENGTH)

/**
 * Indicies for authorization challenge token instances.
 */
enum {
	MANTICORE_TOKEN_REVERT_BYPASS = 0,		/**< Revert bypass authorization challenge. */
	MANTICORE_TOKEN_FACTORY_DEFAULT = 1,	/**< Factory default authorization challenge. */
	MANTICORE_TOKEN_CLEAR_PCD = 2,			/**< Clear PCD authorization challenge. */
	MANTICORE_TOKEN_INTRUSION_RESET = 3,	/**< Intrusion reset authorization challenge. */
	MANTICORE_TOKEN_RMA = 4,				/**< RMA authorization challenge. */
	MANTICORE_TOKEN_CLEAR_CERTS = 5,		/**< Clear certificates authorization challenge. */
	MANTICORE_TOKEN_REVOKE_DICE = 6,		/**< Revoke DICE identity authorization challenge. */
	MANTICORE_TOKEN_REVOKE_DME = 7,			/**< Revoke DME key authorization challenge. */
};

/**
 * Data buffers to build and store authorization tokens.
 */
static uint8_t auth_challenge_token_buffer[MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT]
[MANTICORE_MAX_AUTH_CHALLENGE_TOKEN_LENGTH];

/**
 * Authorization token handlers for use with authentication challenges.
 */
static const struct auth_token auth_challenge_token[MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT] = {
	/* Auth token for revert bypass authorization. */
	[MANTICORE_TOKEN_REVERT_BYPASS] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_REVERT_BYPASS],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_REVERT_BYPASS].base, 0,
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION, auth_challenge_token_buffer[MANTICORE_TOKEN_REVERT_BYPASS],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_REVERT_BYPASS])),

	/* Auth token for factory default authorization. */
	[MANTICORE_TOKEN_FACTORY_DEFAULT] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_FACTORY_DEFAULT],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_FACTORY_DEFAULT].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION,
		auth_challenge_token_buffer[MANTICORE_TOKEN_FACTORY_DEFAULT],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_FACTORY_DEFAULT])),

	/* Auth token for clear PCD authorization. */
	[MANTICORE_TOKEN_CLEAR_PCD] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_CLEAR_PCD],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_CLEAR_PCD].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION, auth_challenge_token_buffer[MANTICORE_TOKEN_CLEAR_PCD],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_CLEAR_PCD])),

	/* Auth token for intrusion reset authorization. */
	[MANTICORE_TOKEN_INTRUSION_RESET] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_INTRUSION_RESET],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_INTRUSION_RESET_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_INTRUSION_RESET].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION,
		auth_challenge_token_buffer[MANTICORE_TOKEN_INTRUSION_RESET],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_INTRUSION_RESET])),

	/* Auth token for RMA authorization. */
	[MANTICORE_TOKEN_RMA] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_RMA],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys, MANTICORE_FW_KEYS_RMA_KEY),
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH, &auth_verification[MANTICORE_AUTH_RMA].base,
		sizeof (uint32_t), AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH,
		HASH_TYPE_SHA384, MANTICORE_AUTH_TOKEN_EXPIRATION,
		auth_challenge_token_buffer[MANTICORE_TOKEN_RMA],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_RMA])),

	/* Auth token for clear certificate authorization. */
	[MANTICORE_TOKEN_CLEAR_CERTS] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_CLEAR_CERTS],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_CLEAR_MANIFEST_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICARE_AUTH_CLEAR_CERTS].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION, auth_challenge_token_buffer[MANTICORE_TOKEN_CLEAR_CERTS],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_CLEAR_CERTS])),

	/* Auth token for DICE renewal authorization. */
	[MANTICORE_TOKEN_REVOKE_DICE] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_REVOKE_DICE],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys, MANTICORE_FW_KEYS_ID_RENEWAL_KEY),
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_REVOKE_DICE].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION, auth_challenge_token_buffer[MANTICORE_TOKEN_REVOKE_DICE],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_REVOKE_DICE])),

	/* Auth token for DME renewal authorization. */
	[MANTICORE_TOKEN_REVOKE_DME] =
		auth_token_static_init (&auth_challenge_token_context[MANTICORE_TOKEN_REVOKE_DME],
		&shared_rng.base, &shared_hash.base, &shared_ecc.base, &dice_key_manager,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys, MANTICORE_FW_KEYS_ID_RENEWAL_KEY),
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		&auth_verification[MANTICORE_AUTH_REVOKE_DME].base, sizeof (uint32_t),
		AUTHORIZATION_CHALLENGE_NONCE_LENGTH, ECC_DER_P384_ECDSA_MAX_LENGTH, HASH_TYPE_SHA384,
		MANTICORE_AUTH_TOKEN_EXPIRATION, auth_challenge_token_buffer[MANTICORE_TOKEN_REVOKE_DME],
		sizeof (auth_challenge_token_buffer[MANTICORE_TOKEN_REVOKE_DME])),
};

/**
 * Authorized data parser for use with authentication challenges that only have a token.
 */
static const struct authorized_data_token_only auth_data_token_only =
	authorized_data_token_only_static_init ();

/**
 * Authorized data parser for use with authorization that can include additional data.
 */
static const struct authorized_data_with_aad auth_data_with_aad =
	authorized_data_with_aad_static_init ();

/**
 * Variable context for secure authorization handlers.
 */
static struct authorization_challenge_state
	auth_challenge_context[MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT];

/**
 * Authorization handlers for secure operations.
 */
static const struct authorization_challenge
	auth_challenge[MANTICORE_AUTH_OPERATIONS_CHALLENGE_COUNT] = {
	[MANTICORE_TOKEN_REVERT_BYPASS] =
		authorization_challenge_static_init (&auth_challenge_context[MANTICORE_TOKEN_REVERT_BYPASS],
		&auth_challenge_token[MANTICORE_TOKEN_REVERT_BYPASS], &auth_data_token_only.base,
		HASH_TYPE_SHA384),
	[MANTICORE_TOKEN_FACTORY_DEFAULT] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_FACTORY_DEFAULT],
		&auth_challenge_token[MANTICORE_TOKEN_FACTORY_DEFAULT], &auth_data_token_only.base,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_FACTORY_RESET),
	[MANTICORE_TOKEN_CLEAR_PCD] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_CLEAR_PCD],
		&auth_challenge_token[MANTICORE_TOKEN_CLEAR_PCD], &auth_data_token_only.base,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_CLEAR_PCD),
	[MANTICORE_TOKEN_INTRUSION_RESET] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_INTRUSION_RESET],
		&auth_challenge_token[MANTICORE_TOKEN_INTRUSION_RESET], &auth_data_token_only.base,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_RESET_INTRUSION),
	[MANTICORE_TOKEN_RMA] =
		authorization_challenge_static_init_with_tag (&auth_challenge_context[MANTICORE_TOKEN_RMA],
		&auth_challenge_token[MANTICORE_TOKEN_RMA], &auth_data_with_aad.base_data, HASH_TYPE_SHA384,
		CERBERUS_PROTOCOL_RMA),
	[MANTICORE_TOKEN_CLEAR_CERTS] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_CLEAR_CERTS],
		&auth_challenge_token[MANTICORE_TOKEN_CLEAR_CERTS], &auth_data_with_aad.base_data,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_CLEAR_CERTS),
	[MANTICORE_TOKEN_REVOKE_DICE] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_REVOKE_DICE],
		&auth_challenge_token[MANTICORE_TOKEN_REVOKE_DICE], &auth_data_with_aad.base_data,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_REVOKE_DICE),
	[MANTICORE_TOKEN_REVOKE_DME] =
		authorization_challenge_static_init_with_tag (
		&auth_challenge_context[MANTICORE_TOKEN_REVOKE_DME],
		&auth_challenge_token[MANTICORE_TOKEN_REVOKE_DME], &auth_data_with_aad.base_data,
		HASH_TYPE_SHA384, CERBERUS_PROTOCOL_REVOKE_DME),
};


/**
 * Indicies for global authorization instances.
 */
enum {
	MANTICORE_GLOBAL_FW_UPDATE = 0,	/**< Firmware update global token. */
};

/**
 * Authorization handlers for secure operations with global authorization.
 */
static const struct authorization_global auth_global[MANTICORE_AUTH_OPERATIONS_GLOBAL_COUNT] = {
	[MANTICORE_GLOBAL_FW_UPDATE] = authorization_global_static_init (&auth_data_with_aad.base_sig,
		&shared_hash.base, &auth_verification[MANTICORE_AUTH_FW_UPDATE].base,
		KEY_MANIFEST_HSP_FIRMWARE_KEY_DER (&sp1_shared->fw_keys,
		MANTICORE_FW_KEYS_FIRMWARE_UPDATE_KEY), KEY_MANIFEST_HSP_FIRMWARE_KEY_DER_LENGTH,
		HASH_TYPE_SHA384),
};

/**
 * Authorization instance for insecure operation.
 */
static const struct authorization_allowed auth_allowed = authorization_allowed_static_init;


/**
 * Indicies for authorized execution instances.
 */
enum {
	/* Configuration reset handlers. */
	MANTICORE_EXE_REVERT_BYPASS = 0,	/**< Revert bypass execution handler. */
	MANTICORE_EXE_FACTORY_DEFAULT = 1,	/**< Factory default execution handler. */
	MANTICORE_EXE_CLEAR_PCD = 2,		/**< Clear PCD execution handler. */
	MANTICORE_EXE_CLEAR_CERTS = 3,		/**< Clear certificates execution handler. */

	/* Identity renewal handlers. */
	MANTICORE_EXE_REVOKE_DICE = 0,		/**< Revoke DICE identity execution handler. */
	MANTICORE_EXE_REVOKE_DME = 1,		/**< Revoke DME key execution handler. */
};

/**
 * Authorized execution contexts for configuration reset commands.
 */
static const struct authorized_execution_config_reset config_reset_execution[] = {
	[MANTICORE_EXE_REVERT_BYPASS] =
		authorized_execution_config_reset_static_init_restore_bypass (&config_manager),
	[MANTICORE_EXE_FACTORY_DEFAULT] =
		authorized_execution_config_reset_static_init_restore_defaults (&config_manager),
	[MANTICORE_EXE_CLEAR_PCD] =
		authorized_execution_config_reset_static_init_restore_platform_config (&config_manager),
	[MANTICORE_EXE_CLEAR_CERTS] =
		authorized_execution_config_reset_static_init_clear_provisioned_certificates (
		&config_manager)
};

/**
 * Authorize execution contexts for identity renewal commands.
 */
static const struct authorized_execution_identity_renewal identity_renewal_execution[] = {
	[MANTICORE_EXE_REVOKE_DICE] =
		authorized_execution_identity_renewal_static_init_dice (&identity),
	[MANTICORE_EXE_REVOKE_DME] = authorized_execution_identity_renewal_static_init_dme (&identity)
};


/**
 * Buffer containing the hash that will need to be present in any RMA token.  The contents will be
 * initialized at run-time from the Device ID provided by 1SP.
 *
 * This hash is a hash of 96 bytes of the raw X, Y public key.
 */
SECTION (".sprtro.rma_dice_hash")
static uint8_t rma_dice_hash[SHA384_HASH_LENGTH];

/**
 * RMA token handler for validating RMA requests.
 */
static const struct rma_unlock_token rma_token =
	rma_unlock_token_static_init_no_signature (&device_cmd.base.base, DICE_OID_MANTICORE,
	MANTICORE_OID_LENGTH, rma_dice_hash, SHA384_HASH_LENGTH);

/**
 * Handler to transition the device to RETEST state.
 */
static const struct device_rma_transition_hsp_retest rma_retest =
	device_rma_transition_hsp_retest_static_init_erase_sram (&fuses.base, &ccs.base, &dmb,
	RMA_ERASE_SRAM, ARRAY_SIZE (RMA_ERASE_SRAM));

/**
 * Authorized execution context for handling RMA transition.
 */
static const struct authorized_execution_rma rma_execution =
	authorized_execution_rma_static_init (&rma_token, &rma_retest.base);


/**
 * Indicies for authorized operation instances.
 */
enum {
	MANTICORE_AUTH_OP_REVERT_BYPASS = 0,	/**< Revert to bypass mode, clearing PFMs. */
	MANTICORE_AUTH_OP_FACTORY_DEFAULT = 1,	/**< Clear all configuration, restoring defaults. */
	MANTICORE_AUTH_OP_CLEAR_PCD = 2,		/**< Clear the PCD. */
	MANTICORE_AUTH_OP_INTRUSION_RESET = 3,	/**< Reset the chassis intrusion state. */
	MANTICORE_AUTH_OP_ALLOW_IMPACTFUL = 4,	/**< Allow impactful firmware updates. */
	MANTICORE_AUTH_OP_FW_UPDATE = 5,		/**< Allow a firmware update. */
	MANTICORE_AUTH_OP_RMA = 6,				/**< Transition device to the RMA, end-of-life state. */
	MANTICORE_AUTH_OP_CLEAR_CERTS = 7,		/**< Erase provisioned certificates from flash. */
	MANTICORE_AUTH_OP_REVOKE_DICE = 8,		/**< Revoke the current DICE identity key. */
	MANTICORE_AUTH_OP_REVOKE_DME = 9,		/**< Revoke the current DME key. */
};

/**
 * A list of supported operations for authorization requests.  This is not the list used by run-time
 * code, but provides the list of default operation descriptors.  The actual contexts that will get
 * used for each operation will be set at run-time based on the unlock policy.
 *
 * There needs to be two separate lists so the run-time list can be RO protected after
 * initialization.  The current scheme for managing this only works for .bss data, so this list is
 * used as the array initializer.
 */
static const struct cmd_authorization_operation cmd_auth_operations_default[] = {
	[MANTICORE_AUTH_OP_REVERT_BYPASS] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_REVERT_BYPASS,
		&auth_challenge[MANTICORE_TOKEN_REVERT_BYPASS].base, NULL,
		&config_reset_execution[MANTICORE_EXE_REVERT_BYPASS].base),
	[MANTICORE_AUTH_OP_FACTORY_DEFAULT] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_FACTORY_RESET,
		&auth_challenge[MANTICORE_TOKEN_FACTORY_DEFAULT].base, NULL,
		&config_reset_execution[MANTICORE_EXE_FACTORY_DEFAULT].base),
	[MANTICORE_AUTH_OP_CLEAR_PCD] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_CLEAR_PCD,
		&auth_challenge[MANTICORE_TOKEN_CLEAR_PCD].base, NULL,
		&config_reset_execution[MANTICORE_EXE_CLEAR_PCD].base),
	[MANTICORE_AUTH_OP_INTRUSION_RESET] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_RESET_INTRUSION,
		&auth_challenge[MANTICORE_TOKEN_INTRUSION_RESET].base, NULL,
		&reset_intrusion_execution.base),
	[MANTICORE_AUTH_OP_ALLOW_IMPACTFUL] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_ALLOW_IMPACTFUL,
		&auth_allowed.base, NULL, &allow_impactful_execution.base),	/* No challenge needed for impactful updates. */
	[MANTICORE_AUTH_OP_FW_UPDATE] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_FIRMWARE_UPDATE,
		&auth_global[MANTICORE_GLOBAL_FW_UPDATE].base, &auth_data_with_aad.base_data,
		&prepare_fw_update.base),
	[MANTICORE_AUTH_OP_RMA] = cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_RMA,
		&auth_challenge[MANTICORE_TOKEN_RMA].base, &auth_data_with_aad.base_data,
		&rma_execution.base),
	[MANTICORE_AUTH_OP_CLEAR_CERTS] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_CLEAR_CERTS,
		&auth_challenge[MANTICORE_TOKEN_CLEAR_CERTS].base, NULL,
		&config_reset_execution[MANTICORE_EXE_CLEAR_CERTS].base),
	[MANTICORE_AUTH_OP_REVOKE_DICE] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_REVOKE_DICE,
		&auth_challenge[MANTICORE_TOKEN_REVOKE_DICE].base, NULL,
		&identity_renewal_execution[MANTICORE_EXE_REVOKE_DICE].base),
	[MANTICORE_AUTH_OP_REVOKE_DME] =
		cmd_authorization_operation_static_init (CERBERUS_PROTOCOL_REVOKE_DME,
		&auth_challenge[MANTICORE_TOKEN_REVOKE_DME].base, NULL,
		&identity_renewal_execution[MANTICORE_EXE_REVOKE_DME].base),
};


/**
 * Run-time list of supported operations for authorization requests.  This will get initialized at
 * run-time from the default list and modified based on the unlock policy.
 */
SECTION (".sprtro.cmd_authorization_operation")
static struct cmd_authorization_operation
	cmd_auth_operations[ARRAY_SIZE (cmd_auth_operations_default)];

/**
 * Authorization handler for commands.
 */
const struct cmd_authorization cmd_auth = cmd_authorization_static_init (cmd_auth_operations,
	ARRAY_SIZE (cmd_auth_operations_default));

/**
 * List of manifests to clear for bypass mode.
 */
static const struct manifest_manager *const bypass_manifests[] = {
#ifndef MANTICORE_ENABLE_ACVP
	&host_fw_manifest.base.base
#endif
};

/**
 * List of manifests to clear to platform configuration.
 */
static const struct manifest_manager *const config_manifests[] = {
#ifndef MANTICORE_ENABLE_ACVP
	&platform_config.base.base
#endif
};

/**
 * List of states to reset when requested.
 */
static const struct state_manager *const reset_state[] = {
#ifndef MANTICORE_ENABLE_ACVP
	&host_state.base,
#endif
	&system_state
};

/**
 * Manager for handling requests to clear manifests.
 */
const struct config_reset config_manager = config_reset_static_init (bypass_manifests,
	ARRAY_SIZE (bypass_manifests), config_manifests, ARRAY_SIZE (config_manifests), NULL, 0,
	reset_state, ARRAY_SIZE (reset_state), &dice_key_manager, NULL, NULL, NULL, 0);

/**
 * List of state managers to persist to flash.
 */
static const struct state_manager *const persist_list[] = {
#ifndef MANTICORE_ENABLE_ACVP
	&host_state.base,
#endif
	&system_state
};

/**
 * Variable context for storing the current state to flash.
 */
static struct state_persistence_handler_state state_persist_context;

/**
 * Handler for storing the current state to flash.
 */
const struct state_persistence_handler state_persist =
	state_persistence_handler_static_init (&state_persist_context, persist_list,
	ARRAY_SIZE (persist_list), 1000);

/**
 * Variable context for telemetry temperature handler.
 */
static struct telemetry_temperature_handler_state telemetry_temperature_context;

/**
 * The temperature threshold values for the middle die temperature sensor.
 */
const struct telemetry_temperature_handler_thresholds middle_die_thresholds = {
	.lower_temperature_threshold = MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_LOWER_THRESHOLD,
	.higher_temperature_threshold = MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_HIGHER_THRESHOLD,
	.max_delta = MANTICORE_MIDDLE_DIE_TEMPERATURE_SENSOR_MAX_DELTA
};

/**
 * Handler for telemetry temperature monitor for the middle die sensor.
 */
static const struct telemetry_temperature_handler telemetry_temperature_middle_die_handler =
	telemetry_temperature_handler_static_init (&telemetry_temperature_context,
	&tsen.middle_die.base, &middle_die_thresholds, 1000);


/**
 * List of handlers for flushing non-volatile data to flash.
 */
static const struct periodic_task_handler *const persist_handlers[] = {
	&telemetry_temperature_middle_die_handler.base, &log_flush.base,
	&state_persist.base
};

/**
 * Variable context for the task flushing non-volatile data to flash.
 */
static struct periodic_task_freertos_state persist_task_context;

/**
 * Task to flush non-volatile data to flash.
 */
static const struct periodic_task_freertos persist_task =
	periodic_task_freertos_static_init (&persist_task_context, persist_handlers,
	ARRAY_SIZE (persist_handlers), PERSIST_DATA_TASK_LOG_ID);

/**
 * Statically allocated task control block for the state persistence task.
 */
static StaticTask_t persist_task_tcb;

/**
 * Statically allocated stack for the state persistence task.
 */
static StackType_t persist_task_stack[PERSISTENCE_TASK_STACK_WORDS];

/**
 * Constant data to use for exporting BKS1 and BKS2 from the KSU for use by HSM.  This is the
 * SHA256 hash of "BKS".
 */
static const SP_MSG_256 MANTICORE_BKS_EXPORT_CONTEXT = {
	.AsBytes = {
		0x08, 0x8c, 0x8a, 0x32, 0xcd, 0x8a, 0xc1, 0x8d,
		0x72, 0xb3, 0xec, 0x25, 0x9c, 0x74, 0xeb, 0xd4,
		0x7e, 0xe2, 0xa6, 0x56, 0x7c, 0x1e, 0xf9, 0xdb,
		0x13, 0x90, 0x97, 0x7b, 0x32, 0xa4, 0x07, 0x88
	}
};

/**
 * Base address for CP ITCM ECC error counter registers.
 */
#define MANTICORE_SOC_CP_ITCM_ECC_REGISTERS		0xb0200000

/**
 * Maximum count for CP ITCM ECC errors.
 */
enum {
	MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT = 0xffff,			/**< Maximum count for CP ITCM ECC errors. */
	MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT_DEFAULT = 0xB3B0,	/**< Default value for CP ITCM ECC error counters at POR. */
};

/**
 * CP ITCM double bit error counter.
 */
struct cp_itcm_ecc_count {
	uint8_t pad0[0x12c];							/**< Unused. */
	volatile uint32_t cp0_ctl_itcm_uncorrectable;	/**< CP0 register for ITCM ECC errors. */
	uint8_t pad1[0xfc];								/**< Unused. */
	volatile uint32_t cp1_ctl_itcm_uncorrectable;	/**< CP1 register for ITCM ECC errors. */
};


/* Try to catch any changes to the relative ordering of the cp ITCM elements in DUAL_CP_M7.  If any
 * of these asserts fail, */
_Static_assert (offsetof (struct cp_itcm_ecc_count, cp0_ctl_itcm_uncorrectable) == 0x012c,
	"CP0 ITCM register offset wrong.");

_Static_assert (offsetof (struct cp_itcm_ecc_count, cp1_ctl_itcm_uncorrectable) == 0x022c,
	"CP1 ITCM register offset wrong.");


/**
 * Indicate if secure boot is enable with a fused root key.
 *
 * @return true if secure boot is enabled.
 */
bool is_secure_boot_enabled ()
{
	/* TODO:  See if there is anything else SPRT would need to check before determining whether a
	 * device boot securely or not. */
	return MANTICORE_1SP_IS_BOOT_SECURE (sp1_shared);
}

/**
 * Preform basic initialization to get the system ready for operation.  As part of this flow, the
 * reset type is determined.
 *
 * @param reset_str Output for a string representation of the reset type
 */
void hardware_init (const char **reset_str)
{
	hsp_trap_init (true, 0);
	traps_init_exception_catch ();
	determine_hsp_clock_frequency ();
	HspUartInitializeEx (HSP_CLOCK_FREQUENCY_HZ, 115200);

#ifndef MANTICORE_ENABLE_A0_SUPPORT
	recovery_boot = (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT);
#else
	{
		const uint32_t *const socid = (uint32_t*) HSP_ADDR_MAP_GFC_SOCID_ADDRESS;

		/* Store in a global to force the compiler to load a full word and make it easier to use for
		 * revision checking elsewhere during init. */
		soc_revision = *socid;

		if (MANTICORE_IS_A0 (soc_revision)) {
			/* A0 will never boot from recovery due to lack of ROM access to internal flash. */
			recovery_boot = false;
		}
		else {
			recovery_boot = (MANTICORE_BOOT_SOURCE == MANTICORE_BOOT_SOURCE_EXT);
		}
	}
#endif

	if (is_por ()) {
		*reset_str = "POR";
		reset_source = RESET_POR;
	}
	else if (is_sw_reset ()) {
		if (is_graceful_reset ()) {
			*reset_str = "Graceful";
		}
		else {
			*reset_str = "Soft";
		}
		reset_source = RESET_SOFT;
	}
	else if (sw_regs->SW_STICKY_RW[MANTICORE_ROM_FATAL_ERRORS] != 0) {
		/* Use this to indicate any HW triggered interrupt, but it should be that only the watchdog
		 * will trigger HW fatal errors. */
		*reset_str = "WDT";
		reset_source = RESET_WATCHDOG;
	}
	else {
		/* This should not be possible.  Whatever caused the reset should be logged in the sticky
		 * register and there should be no reset path that doesn't map to a fatal error. */
		*reset_str = "Unknown";
		reset_source = RESET_UNKNOWN;
	}

	/* Clear the accumulated fatal errors.
	 *
	 * TODO:  Does anything else need to be done with this first? */
	sw_regs->SW_STICKY_RW[MANTICORE_ROM_FATAL_ERRORS] = 0;
}

/**
 * Verify the integrity of the read-only data shared from 1SP.
 *
 * @param hash The hash engine to use for verification.
 *
 * @return 0 if the verification is successful or an error code.
 */
int verify_1sp_shared_data (const struct hash_engine *hash)
{
	uint8_t digest[SHA384_HASH_LENGTH];
	int status;

	if (sp1_shared->valid_length != MANTICORE_1SP_SHARED_DATA_VALID_LENGTH) {
		/* TODO:  It would be good to convert this to a debug log entry to catch this issue if it
		 * ever happens in production or in dev systems without UART access. */
		platform_printf ("WARNING: 1SP shared data length mismatch (ac=%d, ex=%d)" NEWLINE,
			sp1_shared->valid_length, MANTICORE_1SP_SHARED_DATA_VALID_LENGTH);
	}

	status = hash->calculate_sha384 (hash, sp1_shared->hashed, sizeof (sp1_shared->hashed), digest,
		sizeof (digest));
	if (status != 0) {
		return status;
	}

	if (buffer_compare (sp1_shared->digest, digest, SHA384_HASH_LENGTH) != 0) {
		status = INIT_PCR_VERIFICATION_FAILED;
	}

	return status;
}

/**
 * Initialize the HSP MPU and apply memory protection to SP TCM for SPRT execution.
 *
 * @return 0 if the MPU was configured successfully or an error code.
 */
int initialize_mpu ()
{
	/* MPU and handler are both statically initialized.  Just apply the MPU configuration. */
	return sprt_memory_protect.base.base.configure_hsp_mpu (&sprt_memory_protect.base.base);
}

/**
 * Apply HSP MPU configuration for any regions that needed to be left open during initialization.
 * This will primarily mark the designated section of DTCM containing dynamically initialized pieces
 * as RO once the initialization has completed.
 *
 * @return 0 if the MPU configuration was finalized successfully or an error code.
 */
int finalize_mpu ()
{
	size_t ro_length = (void*) sp1_shared - (void*) &_ro_init_start;

	return mpu.base.set_region_attributes (&mpu.base, &_ro_init_start, ro_length,
		MPU_PROTECTION_LEVEL_USER | MPU_PROTECTION_LEVEL_PRIVILEGE,
		MPU_PAGE_ATTRIBUTE_READ | MPU_PAGE_ATTRIBUTE_LOCKED);
}

/**
 * Initialize the HW random number generator.
 *
 * @return 0 if the RNG was successfully initialized or an error code.
 */
int initialize_rng ()
{
	int status;

	status = fuse_controller_init_state (&fuses);
	if (status != 0) {
		return status;
	}

	/* No need to calibrate the RNG since that is handled by 1SP. */
	status = hsp_rng_hw_init_state (&rng_hw, false);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Populate data in shared memory for use by the SoC.  This includes:
 * - The reset type in PSRAM.
 * - Seed table of BKS1 and BKS2 in GSRAM.
 *
 * @return 0 if the shared data was populated successfully or an error code.
 */
static int populate_soc_shared_data ()
{
	struct soc_shared_data *soc_shared;
	struct manticore_device_keys_bks_table *bks_table;
	int status;

	/* Store the reset type in PSRAM. */
	status = dmb.map_soc_address (&dmb, SOC_RESET_TYPE_ADDRESS, sizeof (*soc_shared),
		HSP_DMB_ACCESS_WRITE, (void**) &soc_shared);
	if (status != 0) {
		return status;
	}

	/* Determine the type of reset that was performed. */
	if (is_por ()) {
		soc_shared->reset_type = SOC_RESET_TYPE_SOC;
	}
	else if (is_graceful_reset ()) {
		soc_shared->reset_type = SOC_RESET_TYPE_FIRMWARE_UPDATE;
	}
	else {
		soc_shared->reset_type = SOC_RESET_TYPE_WARM;
	}

	dmb.unmap_soc_address (&dmb, soc_shared);

	/* Store BKS1 and BKS2 in GSRAM. */
	status = dmb.map_soc_address (&dmb, BKS_TABLE_ADDRESS, sizeof (*bks_table),
		HSP_DMB_ACCESS_WRITE, (void**) &bks_table);
	if (status != 0) {
		return status;
	}

	/* Use the firmware descriptor SVN since it's directly accessible in shared memory and is
	 * guaranteed to be the same as the firmware key manifest SVN. */
	status = manticore_device_keys_export_bks (&aes_hw, &MANTICORE_BKS_EXPORT_CONTEXT, &ccs.base,
		manticore_firmware_descriptor_get_svn (&sp1_shared->fw_descriptor), bks_table);

	dmb.unmap_soc_address (&dmb, bks_table);

	return status;
}

/**
 * Release the SoC CPU cores to start executing.
 *
 * @return 0 if the cores have been started successfully or an error code.
 */
static int unstall_soc_cpu_cores ()
{
	enum soc_reset_control_cp_core cp_run_stall = SOC_RESET_CONTROL_CP_CORE_NONE;
	enum soc_reset_control_fp_core fp_run_stall = SOC_RESET_CONTROL_FP_CORE_NONE;
	void *por_regs;
	int status;

	status = soc_reset_control_init_state (&arm_reset_ctrl);
	if (status != 0) {
		return status;
	}

	/* For each core that was loaded with a firmware image, reset the core and let it run.  A core
	 * will have been loaded if the firmware package contains images for that core. */
	if (manticore_firmware_descriptor_cp_image_count (&sp1_shared->fw_descriptor) != 0) {
		platform_printf ("Run CP" NEWLINE);
		cp_run_stall = SOC_RESET_CONTROL_CP_CORE_BOTH;
	}

	if (manticore_firmware_descriptor_fp0_image_count (&sp1_shared->fw_descriptor) != 0) {
		platform_printf ("Run FP0" NEWLINE);
		fp_run_stall |= SOC_RESET_CONTROL_FP_CORE_0;
	}

	if (manticore_firmware_descriptor_fp1_image_count (&sp1_shared->fw_descriptor) != 0) {
		platform_printf ("Run FP1" NEWLINE);
		fp_run_stall |= SOC_RESET_CONTROL_FP_CORE_1;
	}

	if (manticore_firmware_descriptor_fp2_image_count (&sp1_shared->fw_descriptor) != 0) {
		platform_printf ("Run FP2" NEWLINE);
		fp_run_stall |= SOC_RESET_CONTROL_FP_CORE_2;
	}

	status = arm_reset_ctrl.map_por_registers (&arm_reset_ctrl, &por_regs);
	if (status != 0) {
		return status;
	}

	arm_reset_ctrl.stall_cp_core (&arm_reset_ctrl, por_regs, false, cp_run_stall);
	arm_reset_ctrl.stall_fp_core (&arm_reset_ctrl, por_regs, false, fp_run_stall);

	arm_reset_ctrl.unmap_por_registers (&arm_reset_ctrl, por_regs);

	return 0;
}

/**
 * Check if there has been a double-bit error recorded in the CP ITCM ECC error registers.
 *
 * @return 0 if no double-bit error has been recorded or an error code.
 */
static int is_cp_itcm_double_bit_error_recorded ()
{
	int status;
	struct cp_itcm_ecc_count *itcm_handler;
	uint16_t cp0_err_count;
	uint16_t cp1_err_count;

	status = dmb.map_soc_address (&dmb, MANTICORE_SOC_CP_ITCM_ECC_REGISTERS, sizeof (*itcm_handler),
		HSP_DMB_ACCESS_READ | HSP_DMB_ACCESS_WRITE, (void**) &itcm_handler);
	if (status != 0) {
		return status;
	}

	/* On a POR or Graceful Reset just clear any existing error counts and return.
	* For backward compatibility, updating from older firmware to the newer version
	* through IDFU triggered the condition. it was a side effect of transitioning from
	* firmware without ITCM error handling to firmware that expects the cp0_ctl_itcm_uncorrectable
	* registers to be clean at startup.  */
	if (is_por () || is_graceful_reset ()) {
		itcm_handler->cp0_ctl_itcm_uncorrectable &= ~(MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);
		itcm_handler->cp1_ctl_itcm_uncorrectable &= ~(MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);
		status = 0;
	}
	else {
		/* Get the error counts for each CP cores. */
		cp0_err_count = (itcm_handler->cp0_ctl_itcm_uncorrectable &
			MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);
		cp1_err_count = (itcm_handler->cp1_ctl_itcm_uncorrectable &
			MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);

		if ((cp0_err_count != 0) || (cp1_err_count != 0)) {
			/* Clear ITCM DBE error count */
			itcm_handler->cp0_ctl_itcm_uncorrectable &= ~(MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);
			itcm_handler->cp1_ctl_itcm_uncorrectable &= ~(MANTICORE_SOC_CP_ITCM_ECC_ERROR_COUNT);

			/* Log the ITCM ECC error. */
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_ITCM_ECC_ERROR, cp0_err_count, cp1_err_count);

			/* Return error to indicate ITCM ECC error was recorded. */
			status = INIT_ITCM_ECC_ERROR;
		}
	}

	dmb.unmap_soc_address (&dmb, itcm_handler);

	return status;
}

/**
 * Initialize SoC components that need to be done while the CP/FP cores are still stalled.  Once all
 * initialization has been completed, release the SoC cores to run.
 *
 * @return 0 if SoC initialization completed successfully or an error code.
 */
int initialize_soc ()
{
	int status;

	/* Check for cp ITCM double bit errors. */
	status = is_cp_itcm_double_bit_error_recorded ();
	if (status != 0) {
		return status;
	}

	/* Erase GSRAM and PSRAM locations based on the reset type. */
	if (is_graceful_reset ()) {
		status = sram_erase_soc_memory_blocks (&dmb, GRACEFUL_RESET_ERASE_SRAM,
			ARRAY_SIZE (GRACEFUL_RESET_ERASE_SRAM));
	}
	else if (!is_por ()) {
		status = sram_erase_soc_memory_blocks (&dmb, NON_GRACEFUL_RESET_ERASE_SRAM,
			ARRAY_SIZE (NON_GRACEFUL_RESET_ERASE_SRAM));
	}
	else {
		status = 0;
	}

	if (status != 0) {
		return status;
	}

	/* Enable memory fencing.  This can be done irrespective of the unlock policy, since a policy
	 * that disables memory fencing will do so through the AEBs, which invalidates any setting
	 * applied here. */
	status = sprt_memory_protect.base.base.configure_soc_fences (&sprt_memory_protect.base.base);
	if (status != 0) {
		return status;
	}

	/* Populate data needed by the ARM cores. */
	status = populate_soc_shared_data ();
	if (status != 0) {
		return status;
	}

	/* Release SoC cores. */
	status = unstall_soc_cpu_cores ();
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize main system management.
 *
 * @return 0 if all components were initialized successfully or an error code.
 */
int initialize_system_management ()
{
	int status;

	status = security_manager_hsp_manticore_init_state (&security_mgr, is_por ());
	if (status != 0) {
		return status;
	}

	status = system_state_manager_init_state (&system_state);
	if (status != 0) {
		return status;
	}

	status = initialize_reset_counters ();
	if (status != 0) {
		return status;
	}

	status = graceful_shutdown_init_state (&graceful_shutdown);
	if (status != 0) {
		return status;
	}

#ifndef MANTICORE_NO_GRACEFUL_SHUTDOWN
	status = system_init (&system_mgr, &graceful_shutdown.base_device);
	if (status != 0) {
		return status;
	}
#else
	status = system_init (&system_mgr, &device_cmd.base.base);
	if (status != 0) {
		return status;
	}
#endif

	status = mmio_register_block_soc_init_state (&fence_registers_block);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize handling of RMA requests.
 *
 * @return 0 if RMA handling was initialized successfully or an error code.
 */
static int initialize_rma_handling ()
{
	struct x509_certificate cert;
	uint8_t *der_key;
	size_t der_length;
	SP_ECDSA_P384_PUBLIC devid_key;
	int status;

	/* Populate the Device ID hash buffer with the Device ID hash.  This needs to be extracted from
	 * from the device ID certificate passed from 1SP. */
	status = shared_x509.base.load_certificate (&shared_x509.base, &cert,
		sp1_shared->devid_cert_fips, sp1_shared->devid_cert_fips_length);
	if (status != 0) {
		return status;
	}

	status = shared_x509.base.get_public_key (&shared_x509.base, &cert, &der_key, &der_length);
	shared_x509.base.release_certificate (&shared_x509.base, &cert);
	if (status != 0) {
		return status;
	}

	status = ecc_der_decode_public_key (der_key, der_length, devid_key.Parts.X.AsBytes,
		devid_key.Parts.Y.AsBytes, SP_MSG_384_SIZE);
	platform_free (der_key);
	if (ROT_IS_ERROR (status)) {
		return status;
	}

	return shared_hash.base.calculate_sha384 (&shared_hash.base, devid_key.AsBytes,
		SP_ECDSA_P384_PUBLIC_KEY_SIZE, rma_dice_hash, sizeof (rma_dice_hash));
}

/**
 * Initialize management of configuration reset requests.
 *
 * @return 0 if the configuration manager was successfully initialized or an error code.
 */
int initialize_config_reset_management ()
{
	size_t i;
	int status;

	status = initialize_rma_handling ();
	if (status != 0) {
		return status;
	}

	for (i = 0; i < ARRAY_SIZE (auth_verification); i++) {
		status = signature_verification_ecc_init_state (&auth_verification[i], NULL, 0);
		if (status != 0) {
			return status;
		}
	}

	for (i = 0; i < ARRAY_SIZE (auth_challenge); i++) {
		status = auth_token_init_state (&auth_challenge_token[i]);
		if (status != 0) {
			return status;
		}

		status = authorization_challenge_init_state (&auth_challenge[i]);
		if (status != 0) {
			return status;
		}
	}

	for (i = 0; i < ARRAY_SIZE (auth_global); i++) {
		status = authorization_global_check_init (&auth_global[i]);
		if (status != 0) {
			return status;
		}
	}

	/* Initialize the authorization handlers for each operation, overriding the default values based
	 * on the security policy. */
	memcpy (cmd_auth_operations, cmd_auth_operations_default, sizeof (cmd_auth_operations));

	if (security_policy_hsp_manticore_allow_no_auth_manifest_erase (&sec_policy)) {
		cmd_auth_operations[MANTICORE_AUTH_OP_REVERT_BYPASS].authorization = &auth_allowed.base;
		cmd_auth_operations[MANTICORE_AUTH_OP_CLEAR_PCD].authorization = &auth_allowed.base;
	}

	if (security_policy_hsp_manticore_allow_no_auth_factory_default (&sec_policy)) {
		cmd_auth_operations[MANTICORE_AUTH_OP_FACTORY_DEFAULT].authorization = &auth_allowed.base;
		cmd_auth_operations[MANTICORE_AUTH_OP_CLEAR_CERTS].authorization = &auth_allowed.base;
	}

	if (security_policy_hsp_manticore_allow_no_auth_intrusion_reset (&sec_policy)) {
		cmd_auth_operations[MANTICORE_AUTH_OP_INTRUSION_RESET].authorization = &auth_allowed.base;
	}

	return 0;
}

/**
 * Initialize and start the task for refreshing the hardware watchdog and monitoring for SoC
 * crashes.
 *
 * @return 0 if the task was successfully started or an error code.
 */
int start_watchdog_task ()
{
	int status;

	status = hsp_dmb_init_state (&dmb);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_init_state (&watchdog_task);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&watchdog_task, &watchdog_task_tcb,
		watchdog_task_stack, WATCHDOG_TASK_STACK_WORDS, "WDT", CERBERUS_PRIORITY_CRITICAL);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&watchdog_task);

	return 0;
}

/**
 * Initialize the task to store non-volatile data to flash.
 *
 * @return 0 if the task was successfully initialized or an error code.
 */
int initialize_persistence_task ()
{
	int status;

	status = state_persistence_handler_init_state (&state_persist);
	if (status != 0) {
		return status;
	}

	status = telemetry_temperature_handler_init_state (&telemetry_temperature_middle_die_handler);
	if (status != 0) {
		return status;
	}

	return periodic_task_freertos_init_state (&persist_task);
}

/**
 * Start the task to store non-volatile data to flash.
 *
 * @return 0 if the task was started successfully or an error code.
 */
int start_persistence_task ()
{
	int status;

	status = periodic_task_freertos_allocate_static (&persist_task, &persist_task_tcb,
		persist_task_stack, PERSISTENCE_TASK_STACK_WORDS, "Flush", CERBERUS_PRIORITY_BACKGROUND);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&persist_task);

	return 0;
}

/**
 * Enable stack usage to be tracked and printed.
 *
 *
 * @return 0 if stack usage monitoring was successfully enabled or an error code.
 */
int enable_stack_usage_monitoring ()
{
	int status;

	status = system_observer_stack_usage_init (&stack_usage);
	if (status != 0) {
		return status;
	}

	status = system_add_observer (&system_mgr, &stack_usage.base);
	if (status != 0) {
		return status;
	}

	return 0;
}
