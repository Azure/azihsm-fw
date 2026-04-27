// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "FreeRTOS.h"
#include "hsp_top.h"
#include "init_crashdump.h"
#include "init_crypto.h"
#include "init_log_flush_handlers.h"
#include "init_system.h"
#include "periodic_task_freertos_static.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "crashdump/hsp_crashdump_hw_err_handler_access_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_bus_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_chk_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_dmb_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_mem_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_mpu_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_rng_err_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_watchdog_timeout_static.h"
#include "crashdump/hsp_crashdump_hw_err_handler_wdt_err_static.h"
#include "crashdump/hsp_crashdump_logging.h"
#include "crashdump/soc_crashdump_handler_static.h"
#include "crashdump/soc_crashdump_mbx_err_handler_static.h"
#include "dc_scm/manticore_sticky_regs.h"
#include "dc_scm/sp_boot.h"
#include "logging/manticore_logging.h"
#include "mmio/mmio_register_block_soc_static.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_trap.h"
#include "trap/hsp_trap_context.h"
#include "trap/irq_error.h"


/**
 * Offset within CREG of the HSP interrupt registers.
 */
#define	HSP_CRASHDUMP_HW_ERR_HANDLER_HSP_INT_REGS_OFFSET    \
	(offsetof (struct Creg_regs, int_regs) + offsetof (struct Creg_regs_int_addr, creg_int_group))

/**
 * Variable context for the crashdump SoC handler.
 */
static struct soc_crashdump_handler_state soc_state;

/**
 * Data populated by 1SP that can be used with local static initialization.
 */
static const struct manticore_1sp_shared_data *const sp1_shared_static =
	(struct manticore_1sp_shared_data*) SP1_SHARED_ADDRESS;

/**
 * Crashdump HSP handler instance.
 */
const struct hsp_crashdump_handler hsp_crashdump_handler =
	hsp_crashdump_handler_static_init (sp1_shared_static->fw_descriptor.build_ver,
	FW_COMPONENT_BUILD_VERSION_LENGTH, boot_error_reset,
	MANTICORE_STICKY_REG (MANTICORE_CRASHDUMP_0),
	(MANTICORE_CRASHDUMP_18 - MANTICORE_CRASHDUMP_0 + 1) * sizeof (uint32_t));

/**
 * Variable context for accessing TCON registers.
 */
static struct mmio_register_block_soc_state tcon_regs_context;

/**
 * mmio_soc_tcon_regs The interface object used to access TCON via MMIO.
 */
static const struct mmio_register_block_soc mmio_soc_tcon_regs =
	mmio_register_block_soc_static_init (&tcon_regs_context, &dmb, TCON_REGISTERS_BLOCK_ADDRESS,
	sizeof (Tcon_t));

/**
 * HSP DMB MMIO registers block.
 */
static const struct mmio_register_block_hsp dmb_regs =
	mmio_register_block_hsp_static_init ((uint32_t*) HSP_ADDR_MAP_DMB_ADDRESS,
	sizeof (struct Dmb_reg));

/**
 * HSP RNG MMIO registers block.
 */
static const struct mmio_register_block_hsp rng_regs =
	mmio_register_block_hsp_static_init ((uint32_t*) HSP_ADDR_MAP_RNG_ADDRESS,
	sizeof (struct Rng_regs));

/**
 * Crashdump SoC API instance.
 */
const struct soc_crashdump_interface soc_api =
	soc_crashdump_interface_static_init (boot_error_reset, &dmb, &mmio_soc_tcon_regs.base,
	MANTICORE_STICKY_REG (MANTICORE_CRASHDUMP_COUNTER));

/**
 * Crashdump SoC handler instance.
 */
const struct soc_crashdump_handler soc_handler =
	soc_crashdump_handler_static_init (&soc_state, 1000, &soc_api, &log_flush,
	sp1_shared_static->fw_descriptor.build_ver, FW_COMPONENT_BUILD_VERSION_LENGTH);

/**
 * Crashdump access error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_access_err access_err_handler =
	hsp_crashdump_hw_err_handler_access_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, acc_regs));

/**
 * Crashdump bus error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_bus_err bus_err_handler =
	hsp_crashdump_hw_err_handler_bus_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, sp_bus_err_regs));

/**
 * Crashdump HW check point error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_chk_err chk_err_handler =
	hsp_crashdump_hw_err_handler_chk_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, chkpt_regs) + offsetof (struct Creg_regs_chkpt, creg_chkpt_group));

/**
 * Crashdump DMB error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_dmb_err dmb_err_handler =
	hsp_crashdump_hw_err_handler_dmb_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs_acc_reg, dmb_regs), &dmb_regs.base);

/**
 * Crashdump memory error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_mem_err mem_err_handler =
	hsp_crashdump_hw_err_handler_mem_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, mem_err_regs));

/**
 * Crashdump MPU error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_mpu_err mpu_err_handler =
	hsp_crashdump_hw_err_handler_mpu_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, mpu_regs),
	offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, spdram_mpu_regs),
	offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, spiram_mpu_regs),
	offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, sprom_mpu_regs));

/**
 * Crashdump RNG error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_rng_err rng_err_handler =
	hsp_crashdump_hw_err_handler_rng_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, crypto_regs), &rng_regs.base, &error_state_handler.base_error_isr);

/**
 * Crashdump watchdog timeout handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_watchdog_timeout timeout_handler =
	hsp_crashdump_hw_err_handler_watchdog_timeout_static_init (&hsp_crashdump_handler,
	&creg_regs.base, offsetof (struct Creg_regs, timer0_regs));

/**
 * Crashdump WDT error handler instance.
 */
static const struct hsp_crashdump_hw_err_handler_wdt_err wdt_err_handler =
	hsp_crashdump_hw_err_handler_wdt_err_static_init (&hsp_crashdump_handler, &creg_regs.base,
	offsetof (struct Creg_regs, wdt_regs));

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
/**
 * Execution context for RAS HSP fault injection test.
 */
const struct soc_crashdump_ras_fault_injection ras_fault_inj_test =
	soc_crashdump_ras_fault_injection_static_init (&creg_regs.base, &dmb);
#endif

/**
 * Crashdump MBX error handler instance.
 */
static const struct soc_crashdump_mbx_err_handler mbx_err_handler =
	soc_crashdump_mbx_err_handler_static_init (&error_state_handler.base_error_isr, &creg_regs.base,
	offsetof (struct Creg_regs, h2s_mbx0));


/**
 * It should be invoked by a trap handler. It collects a HSP crashdump once HSP is crashed,
 * and saves it into persistent ram.
 *
 * @param[in] param The parmeter passed by the trap handler.
 *
 * @return true if interrupt handled, else false.
 */
bool hsp_crashdump_exception_handler (uintptr_t param)
{
	struct hsp_crashdump_packet_hsp_production_packet packet;

	/* Collect crashdump information, store it on crashdump packet. */
	hsp_crashdump_logging_collect_crashdump (HSP_CRASHDUMP_PACKET_FAULT_CODE_HARDFAULT,
		CRASH_DUMP_CRASH_TYPE_CRASH, (struct hsp_trap_context*) param,
		hsp_crashdump_handler.fw_version, hsp_crashdump_handler.fw_version_len, &packet);

	/* Save crashdump to persistent ram. */
	hsp_crashdump_logging_save_crashdump_to_persistent_ram (&packet,
		hsp_crashdump_handler.persistent_ram, hsp_crashdump_handler.persistent_ram_size);

	/* Reset SPRT, 1SP will make warm boot. */
	hsp_crashdump_handler.reset ();

	/* Never get here. */
	return true;
}

/**
 * Enable interrupt for a IP module, such as memory module or bus module.
 *
 * @param[in] creg The CREG register interface (struct Creg_regs).
 * @param[in] handler The HSP interrupt handler.
 * @param[in] intr_bit The bit index of HSP_INTSTS to register a handler for.
 * @param[in] int_masks The interrupt mask bits.
 * @param[in] intsts_offset the interrupt status register offset.
 * @param[in] err_enable_offset the error enable register offset.
 * Not every module has an error enable register. The err_enable_offset could be set
 * to 0 if the error enable register does not exist in the module.
 * @param[in] err_int_enable_offset the error interrupt enable register offset.
 * Not every module has an error interrupt enable register. The err_int_enable_offset
 * could be set to 0 if the error interrupt enable register does not exist in the module.
 * @param[in] int_enable The interrupt enable flag. If it is false, the interrupt will
 * not be enabled.
 *
 * @return 0 if successful, else an error code
 */
static int hsp_crashdump_enable_module_interrupt (
	const struct mmio_register_block *creg, const struct hsp_interrupt_handler *handler,
	unsigned intr_bit, uint32_t int_masks, uintptr_t intsts_offset, uintptr_t err_enable_offset,
	uintptr_t err_int_enable_offset, bool int_enable)
{
	int status;

	status = creg->map (creg);
	if (status != 0) {
		return status;
	}

	/* Clear the pending interrupt from the module. */
	status = creg->write32 (creg, intsts_offset, int_masks);
	if (status != 0) {
		goto exit;
	}

	/* Write 1 to clear HSP pending interrupt. */
	status = mmio_register_block_set_bit (creg,
		HSP_CRASHDUMP_HW_ERR_HANDLER_HSP_INT_REGS_OFFSET + CREG_REGS_INT_HSP_INTSTS_OFFSET,
		intr_bit);
	if (status != 0) {
		goto exit;
	}

	if (int_enable) {
		/* Register the interrupt handler as an IRQ interrupt ISR. */
		status = hsp_interrupt_register (intr_bit, handler);
		if (status != 0) {
			goto exit;
		}

		/* Enable the interrupt from the HSP interrupt controller. */
		status = hsp_interrupt_enable (intr_bit, HSP_INTERRUPT_IRQ_LEVEL_FIQ);
		if (status != 0) {
			goto exit;
		}

		if (err_int_enable_offset) {
			/* Enable module error interrupt. */
			status = creg->write32 (creg, err_int_enable_offset, int_masks);
			if (status != 0) {
				goto exit;
			}
		}
	}

	if (err_enable_offset) {
		/* Disable module error. */
		status = creg->write32 (creg, err_enable_offset, 0);
		if (status != 0) {
			goto exit;
		}
	}

exit:
	creg->unmap (creg);

	return status;
}

/**
 * Enable interrupt for the mailbox0 error.
 *
 * @param[in] creg The CREG register interface.
 * @param[in] handler The HSP interrupt handler.
 * @param[in] mbx_regs_offset The mailbox register offset.
 *
 * @return 0 if successful, else an error code
 */
int soc_crashdump_enable_mbx_err_interrupt (const struct mmio_register_block *creg,
	const struct hsp_interrupt_handler *handler, size_t mbx_regs_offset)
{
	int status;

	status = creg->map (creg);
	if (status != 0) {
		return status;
	}

	/* Clear the pending interrupt from the module. */
	status = creg->write32 (creg, mbx_regs_offset + CREG_REGS_SYS_MBX_S2H_MBX_INSTS_OFFSET,
		CREG_REGS_SYS_MBX_S2H_MBX_INSTS_ERR_BIT_FIELD_MASK);
	if (status != 0) {
		goto exit;
	}

	/* Enable the mbx error bit interrupt */
	status = creg->write32 (creg, mbx_regs_offset + CREG_REGS_SYS_MBX_S2H_MBX_CTRL_OFFSET,
		CREG_REGS_SYS_MBX_S2H_MBX_CTRL_ERR_INT_EN_FIELD_MASK);
	if (status != 0) {
		goto exit;
	}

	/* Enable MBX Error interrupts in HSP. */
	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_C2PMSG_INTEN_MSB, handler);
	if (status != 0) {
		goto exit;
	}

	/* Enable the interrupt from the HSP interrupt controller. */
	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_C2PMSG_INTEN_MSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		goto exit;
	}

exit:
	creg->unmap (creg);

	return status;
}

/**
 * Initialize crashdump HSP data structures.
 *
 * @return 0 if the crashdump initialization succeeded or an error code.
 */
int initialize_crashdump_hsp (void)
{
	int status;

	/* Always check for stack overflow crashes and FW panics first since they get logged differently
	 * from other crashes. */
	hsp_crashdump_logging_save_stack_overflow_from_persistent_ram_to_debug_log (
		hsp_crashdump_handler.persistent_ram);
	hsp_crashdump_logging_save_panic_from_persistent_ram_to_debug_log (
		hsp_crashdump_handler.persistent_ram);

	status =
		hsp_crashdump_logging_save_crashdump_from_persistent_ram_to_debug_log (
		hsp_crashdump_handler.persistent_ram);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_CRASHDUMP_SAVE_CRASHDUMP_TO_DEBUG_LOG_FAILURE, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &access_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_ACC_VIO_INTEN_MSB,
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, acc_regs) +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTSTS_OFFSET,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, acc_regs) +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_ERR_EN_OFFSET,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, acc_regs) +
		CREG_REGS_CREG_ACC_REGS_CREG_ACC_VIO_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_ACCESS_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &bus_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_SP_BUS_ERR_INTEN_MSB,
		CREG_REGS_CREG_SP_BUS_ERR_SP_BUS_ERR_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs,
		sp_bus_err_regs) + CREG_REGS_CREG_SP_BUS_ERR_SP_BUS_ERR_INTSTS_OFFSET,
		offsetof (struct Creg_regs,
		sp_bus_err_regs) + CREG_REGS_CREG_SP_BUS_ERR_SP_BUS_ERR_EN_OFFSET,
		offsetof (struct Creg_regs,
		sp_bus_err_regs) + CREG_REGS_CREG_SP_BUS_ERR_SP_BUS_ERR_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_BUS_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &chk_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_HWCHKPT_ERR_INTEN_MSB,
		CREG_REGS_CHKPT_REGS_HWCKPT_ERR_INSTS_WRITE_MASK,
		offsetof (struct Creg_regs, chkpt_regs) + offsetof (struct Creg_regs_chkpt,
		creg_chkpt_group) +
		CREG_REGS_CHKPT_REGS_HWCKPT_ERR_INSTS_OFFSET,
		offsetof (struct Creg_regs, chkpt_regs) + offsetof (struct Creg_regs_chkpt,
		creg_chkpt_group) +
		CREG_REGS_CHKPT_REGS_HWCKPT_ERR_EN_OFFSET,
		offsetof (struct Creg_regs, chkpt_regs) + offsetof (struct Creg_regs_chkpt,
		creg_chkpt_group) +
		CREG_REGS_CHKPT_REGS_HWCKPT_ERR_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_CHK_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &dmb_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_DMB_INTEN_MSB, CREG_REGS_CREG_DMB_REGS_DMB_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, dmb_regs) +
		CREG_REGS_CREG_DMB_REGS_DMB_INTSTS_OFFSET,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, dmb_regs) +
		CREG_REGS_CREG_DMB_REGS_DMB_ERR_EN_OFFSET,
		offsetof (struct Creg_regs, acc_regs) + offsetof (struct Creg_regs_acc_reg, dmb_regs) +
		CREG_REGS_CREG_DMB_REGS_DMB_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_DMB_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &mem_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_MEM_ERR_INTEN_MSB, CREG_REGS_MEM_ERR_MEM_ERR_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, mem_err_regs) + CREG_REGS_MEM_ERR_MEM_ERR_INTSTS_OFFSET,
		offsetof (struct Creg_regs, mem_err_regs) + CREG_REGS_MEM_ERR_MEM_ERR_EN_OFFSET,
		offsetof (struct Creg_regs, mem_err_regs) + CREG_REGS_MEM_ERR_MEM_ERR_INTEN_OFFSET,	true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_MEM_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &mpu_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_MPU_INTEN_MSB, CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, mpu_regs) +
		CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTSTS_OFFSET, 0,
		offsetof (struct Creg_regs, mpu_regs) + offsetof (struct Creg_regs_mpu, mpu_regs) +
		CREG_REGS_MPU_CREG_MPU_REGS_MPU_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_MPU_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &rng_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_CRYPTO_ERR_INTEN_MSB,
		HSP_CRASHDUMP_HW_ERR_HANDLER_RNG_ERR_INT_MASKS,
		offsetof (struct Creg_regs,
		crypto_regs) + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTSTS_OFFSET,
		offsetof (struct Creg_regs, crypto_regs) + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_EN_OFFSET,
		offsetof (struct Creg_regs,
		crypto_regs) + CREG_REGS_CREG_CRYPTO_GROUP_CRYPTO_ERR_INTEN_OFFSET, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_RNG_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &timeout_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_TIMER_INTEN_MSB,	CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, timer0_regs) + CREG_REGS_CREG_TIMER_CREG_TMR_INTSTS_OFFSET,	0,
		0, true);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_WATCHDOG_TIMER_INT_FAILED, status, 0);
	}

	status = hsp_crashdump_enable_module_interrupt (&creg_regs.base, &wdt_err_handler.base.base,
		CREG_REGS_INT_HSP_IRQINTEN_AXI_WD_INTEN_MSB, CREG_REGS_WDT_REGS_WDT_INTSTS_WRITE_MASK,
		offsetof (struct Creg_regs, wdt_regs) + CREG_REGS_WDT_REGS_WDT_INTSTS_OFFSET,
		offsetof (struct Creg_regs, wdt_regs) + CREG_REGS_WDT_REGS_WDT_ERR_EN_OFFSET,
		offsetof (struct Creg_regs, wdt_regs) + CREG_REGS_WDT_REGS_WDT_INTEN_OFFSET, false);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_WDT_INT_FAILED, status, 0);
	}

	status = soc_crashdump_enable_mbx_err_interrupt (&creg_regs.base, &mbx_err_handler.base,
		offsetof (struct Creg_regs, h2s_mbx0));
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
			MANTICORE_LOGGING_HW_ERR_ENABLE_MBX_INT_FAILED, status, 0);
	}

	return 0;
}

#ifdef MANTICORE_ROT_RESET_CRASH
/**
 * Trigger crashdump
 */
int trigger_crashdump_soc (void)
{
	int status;
	enum soc_crashdump_arm_core_id failed_core_id;
	bool available[SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS];

	status = soc_handler.soc_api->trigger_crash_int (soc_handler.soc_api);
	if (status != 0) {
		return status;
	}

	return soc_handler.soc_api->get_crashdumps_from_cores (soc_handler.soc_api,
		soc_handler.fw_version, available, &failed_core_id);
}
#endif	/* MANTICORE_ROT_RESET_CRASH */
