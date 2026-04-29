// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_top.h"
#include "init_attestation.h"
#include "init_crypto.h"
#include "init_firmware.h"
#include "init_flash.h"
#include "init_host.h"
#include "init_manifest.h"
#include "init_system.h"
#include "manticore_hsp_gpio.h"
#include "manticore_pcr.h"
#include "manticore_sticky_regs.h"
#include "periodic_task_freertos_static.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "crypto/signature_verification_ecc_static.h"
#include "firmware/manticore_fw_keys.h"
#include "host_fw/host_flash_manager_dual_static.h"
#include "host_fw/host_flash_manager_single_static.h"
#include "host_fw/host_gpio_irq_handler_static.h"
#include "host_fw/host_irq_control_hsp_gpio_static.h"
#include "host_fw/host_irq_handler_auth_check_static.h"
#include "host_fw/host_irq_handler_mask_irqs_static.h"
#include "host_fw/host_processor_dual.h"
#include "host_fw/host_processor_dual_full_bypass.h"
#include "host_fw/host_processor_observer_pcr_static.h"
#include "host_fw/host_processor_single.h"
#include "host_fw/host_processor_single_full_bypass.h"
#include "host_fw/host_state_observer_dirty_reset_static.h"
#include "logging/init_logging.h"
#include "manifest/pcd/pcd.h"
#include "spi_filter/flash_mfg_filter_handler_hsp_static.h"
#include "spi_filter/spi_filter_hsp_irq_handler_static.h"
#include "spi_filter/spi_filter_irq_handler_static.h"
#include "splibs/hsprt/riscvcpu.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_interrupt_group_static.h"


/**
 * Configuration options for a single port of host firmware.
 */
struct host_port_config {
	bool dual_flash;			/**< Flag indicating if the port operates in dual flash mode. */
	bool full_bypass;			/**< Flag indicating if the port uses full bypass mode. */
	bool reset_notify;			/**< Flag indicating if the reset control is a notification. */
	bool bmc_recovery;			/**< Flag indicating if BMC watchdog monitoring is enabled. */
	bool run_time_activation;	/**< Flag indicating if run-time activation is supported. */
	int reset_pulse;			/**< Length of the reset pulse when using pulsed resets. */
	uint32_t spi_freq;			/**< Frequency to operate the SPI bus to host flash. */
	bool reset_flash;			/**< Flag indicating if the host flash will reset on host resets. */
};


/**
 * The top-level management instance for the host.
 *
 * This gets initialized dynamically based on PCD configuration.  Mark it as RO after initialization
 * is done.
 */
SECTION (".sprtro.host_manager")
struct host_processor_filtered host_manager;

/**
 * Variable context for host processor management.
 */
static struct host_processor_filtered_state host_manager_context;

/**
 * Variable context for the host processor command handler.
 */
static struct host_cmd_handler_state host_handler_context;

/**
 * Handler for processing host processor commands.
 */
const struct host_cmd_handler host_handler = host_cmd_handler_static_init (&host_handler_context,
	&host_manager.base, &manifest_cmd_task.base);

/**
 * Misc SW registers that can be used with local static initialization.
 */
static struct Creg_regs_misc_creg_sw_regs *const sw_regs_static =
	(struct Creg_regs_misc_creg_sw_regs*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_ADDRESS;

/**
 * Notification handler for reporting GPIO IRQs to the main processing task.  This is also
 * essentially the variable context for GPIO interrupt handling.
 */
struct host_gpio_irq_event_manager gpio_irq;

/**
 * Interrupt handlers for the individual GPIOs.
 */
static const struct host_gpio_irq_handler gpio_irq_handler[2] = {
	host_gpio_irq_handler_static_init (&gpio_irq, PORT1_AUTH_IRQ, GPIO_IRQ_RESET_EXIT,
		GPIO_IRQ_RESET_ENTER),
	host_gpio_irq_handler_static_init (&gpio_irq, PORT1_RESET_IRQ, GPIO_IRQ_HOST_UP,
		GPIO_IRQ_HOST_DOWN)
};

/**
 * List of interrupt handlers for use with the HSP GPIO driver.  This can be a const list because
 * no handlers will be changed and no new GPIOs will be added at run-time.
 */
static const struct hsp_interrupt_handler *const gpio_irq_vector[MANTICORE_HSP_GPIO_COUNT] = {
	[PORT1_AUTH_IRQ] = &gpio_irq_handler[0].base,
	[PORT1_RESET_IRQ] = &gpio_irq_handler[1].base
};

/**
 * Driver for HSP GPIOs.
 *
 * The IRQ vector needs to have the const cast away for compatibility with the GPIO driver
 * structure, since that driver is written generically to support dynamic IRQ registration.
 * However, there are no dynamic workflows in this firmware that are incompatible with a const IRQ
 * vector for GPIO interrupts.
 */
const struct hsp_gpio gpio =
	hsp_gpio_static_init ((struct Creg_regs_gpc_regs*) HSP_ADDR_MAP_CREG_GPIO_REGS_ADDRESS,
	(const struct hsp_interrupt_handler**) gpio_irq_vector, MANTICORE_HSP_GPIO_COUNT);

/**
 * Control interface for enabling and disabling GPIO IRQs for external host events.
 */
static const struct host_irq_control_hsp_gpio gpio_irq_ctrl =
	host_irq_control_hsp_gpio_static_init_reset_irq_only (&gpio, &gpio_irq, &gpio_irq_handler[0],
	&gpio_irq_handler[1]);

/**
 * The GPIOs controlling host hardware functions.
 */
const struct host_control_hsp_gpio host_gpio = host_control_hsp_gpio_static_init (&gpio,
	PORT1_RESET_CTRL, PORT1_RESET_IRQ, PORT1_SPI_FILTER_MUX);

/**
 * Variable context for managing state for the host firmware.
 */
static struct host_state_manager_state host_state_context;

/**
 * State information for the host firmware.
 */
const struct host_state_manager host_state =
	host_state_manager_static_init (&host_state_context, &flash_internal.base, host_state_addr);

/**
 * Reset management for dirty flash for hosts with reset notification.
 */
static const struct host_state_observer_dirty_reset host_state_notify =
	host_state_observer_dirty_reset_static_init (&host_gpio.base);

/**
 * Variable context for the SPI filters.
 */
struct spi_filter_hsp_state host_filter_context;

/**
 * SPI filter arbitrating access to the host flash.
 */
const struct spi_filter_hsp host_filter = spi_filter_hsp_static_init (&host_filter_context,
	(struct Creg_regs_spi_filter_regs*) HSP_ADDR_MAP_CREG_SPI_FILTER1_REGS_ADDRESS, 1);

/**
 * Filter handlers for dealing with different flash manufacturers.
 */
static const struct flash_mfg_filter_handler_hsp filter_mfg =
	flash_mfg_filter_handler_hsp_static_init (&host_filter);

/**
 * Handlers for host IRQs that use reset control.
 */
static const struct host_irq_handler_mask_irqs host_irq_reset =
	host_irq_handler_mask_irqs_static_init_enable_exit_reset (&host_manager.base, &host_hash.base,
	&shared_rsa.base, NULL, &gpio_irq_ctrl.base);

/**
 * Handlers for host IRQs that use reset notification.
 */
static const struct host_irq_handler_auth_check host_irq_notify =
	host_irq_handler_auth_check_static_init (&host_manager.base, &host_hash.base, &shared_rsa.base,
	NULL, &host_gpio.base, &gpio_irq_ctrl.base, &host_state);


/**
 * Configured host IRQ handler based on the port configuration.
 */
SECTION (".sprtro.host_irq")
const struct host_irq_handler *host_irq;

/**
 * Handlers for SPI filter IRQs.
 */
static const struct spi_filter_irq_handler host_filter_irq =
	spi_filter_irq_handler_static_init (&host_state);

/**
 * Variable context for the SPI filter HW interrupt handler.
 */
static struct spi_filter_hsp_irq_handler_state filter_irq_context;

/**
 * Handler for SPI filter HW interrupts.
 */
static const struct spi_filter_hsp_irq_handler filter_irq =
	spi_filter_hsp_irq_handler_static_init (&filter_irq_context, &host_filter, &host_filter_irq);

/**
 * List of individual handlers for SPI filter interrupts that group together to a single HSP
 * interrupt.
 */
static const struct hsp_interrupt_handler *const filter_irq_list[] = {&filter_irq.base_irq};

/**
 * Handler for the aggregated SPI filter interrupt from all SPI filters.
 */
static const struct hsp_interrupt_group filter_irq_group =
	hsp_interrupt_group_static_init (filter_irq_list, ARRAY_SIZE (filter_irq_list), false);

/**
 * List of handlers for the SPI filter interrupt handling tasks.
 */
static const struct periodic_task_handler *const filter_irq_handlers[] = {&filter_irq.base};

/**
 * Variable context for the SPI filter interrupt handling tasks.
 */
static struct periodic_task_freertos_state filter_irq_task_context;

/**
 * Task context for handling SPI filter interrupts.
 */
static const struct periodic_task_freertos filter_irq_task =
	periodic_task_freertos_static_init (&filter_irq_task_context, filter_irq_handlers,
	ARRAY_SIZE (filter_irq_handlers), SPI_FILTER1_IRQ_TASK_LOG_ID);

/**
 * Statically allocated task control block for SPI filter IRQ handler task.
 */
static StaticTask_t filter_irq_task_tcb;

/**
 * Statically allocated stack for the SPI filter IRQ handler task.
 */
static StackType_t filter_irq_task_stack[FILTER_IRQ_TASK_STACK_WORDS];

/**
 * Management for dual host flash devices.
 */
static const struct host_flash_manager_dual host_flash_dual =
	host_flash_manager_dual_static_init_with_managed_flash_initialization (&host_flash_cs0,
	&host_flash_cs1, &host_state, &host_filter.base, &filter_mfg.base, &host_flash_init);

/**
 * Management for a single host flash device.
 */
static const struct host_flash_manager_single host_flash_single =
	host_flash_manager_single_static_init_with_managed_flash_initialization (&host_flash_cs0,
	&host_state, &host_filter.base, &filter_mfg.base, &host_flash_init);

/**
 * PCR management for the state of host firmware.
 */
static const struct host_processor_observer_pcr pcr_host =
	host_processor_observer_pcr_static_init (&host_hash.base, &pcr_storage,
	PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_VALID,
	(uint32_t*) &sw_regs_static->SW_STICKY_RW[MANTICORE_PFM_VALID_PORT1]);


/**
 * Initialize the GPIO driver and enable support for GPIO interrupts.  Individual GPIO interrupt
 * handlers will be initialized per-port with the rest of host firmware support.
 *
 * @return 0 if the HSP GPIOs were successfully initialized or an error code.
 */
int initialize_host_gpios ()
{
	int status;

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_GPIO_INTEN_MSB, &gpio.base);
	if (status != 0) {
		return status;
	}

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_GPIO_INTEN_MSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Get the configuration parameters for protecting host firmware.
 *
 * @param config Output for the port configuration.
 */
static void get_port_configuration (struct host_port_config *config)
{
	struct pcd_port_info pcd_config;
	const struct pcd *active_pcd = platform_config.base.get_active_pcd (&platform_config.base);
	int status;

	config->dual_flash = true;
	config->full_bypass = true;
	config->reset_notify = false;
	config->reset_pulse = 0;
	config->bmc_recovery = false;
	config->run_time_activation = true;
	config->spi_freq = get_default_spi_flash_frequency ();
	config->reset_flash = false;

	if (active_pcd) {
		status = active_pcd->get_port_info (active_pcd, 1, &pcd_config);
		if (status == 0) {
			if (pcd_config.flash_mode != PCD_PORT_FLASH_MODE_RESERVED) {
				config->dual_flash = ((pcd_config.flash_mode == PCD_PORT_FLASH_MODE_DUAL) ||
					(pcd_config.flash_mode == PCD_PORT_FLASH_MODE_DUAL_FILTERED_BYPASS));
				config->full_bypass = ((pcd_config.flash_mode == PCD_PORT_FLASH_MODE_DUAL) ||
					(pcd_config.flash_mode == PCD_PORT_FLASH_MODE_SINGLE));
			}

			if (pcd_config.reset_ctrl != PCD_PORT_RESET_CTRL_RESERVED) {
				config->reset_notify = (pcd_config.reset_ctrl == PCD_PORT_RESET_CTRL_NOTIFY);

				if (pcd_config.reset_ctrl == PCD_PORT_RESET_CTRL_PULSE) {
					config->reset_pulse = pcd_config.pulse_interval;
				}
				else {
					config->reset_pulse = 0;
				}
			}

			config->run_time_activation =
				(pcd_config.runtime_verification == PCD_PORT_RUNTIME_VERIFICATION_ENABLED);
			config->spi_freq = pcd_config.spi_freq;
			config->reset_flash =
				(pcd_config.host_reset_action == PCD_PORT_HOST_RESET_ACTION_RESET_FLASH);
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
				INIT_LOGGING_PORT_CONFIG, 1, status);
		}

		platform_config.base.free_pcd (&platform_config.base, active_pcd);
	}
}

/**
 * Initialize the PCR entry indicating if host FW is valid.
 *
 * @param single_flash Flag indicating if the host firmware is managed on a single flash device.
 *
 * @return 0 if the PCR was successfully initialized or an error code.
 */
static int initialize_host_firmware_pcr (bool single_flash)
{
	int status;

	if (reset_source == RESET_POR) {
		sw_regs->SW_STICKY_RW[MANTICORE_PFM_VALID_PORT1] = HOST_PROCESSOR_OBSERVER_PCR_INIT;
	}
	else if (sw_regs->SW_STICKY_RW[MANTICORE_PFM_VALID_PORT1] ==
		HOST_PROCESSOR_OBSERVER_PCR_BYPASS) {
		host_state_manager_set_bypass_mode (&host_state, true);
	}

	status = host_processor_observer_pcr_init_state (&pcr_host);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
			INIT_LOGGING_PCR_STORE_UPDATE_BUFFER, PCR_MEASUREMENT_TYPE_CERBERUS_PFM_1_VALID,
			status);

		return status;
	}

	status = host_processor_add_observer (&host_manager.base, &pcr_host.base);
	if (status != 0) {
		return status;
	}

	if (single_flash) {
		status = host_state_manager_add_observer (&host_state, &pcr_host.base_state);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Initialize components for managing host port 1.
 *
 * @param reset_notify Flag indicating the host should be notified of pending authentication using
 * the reset control signal.
 * @param single_flash Flag indicating if the host firmware is managed on a single flash device.
 * @param spi_freq The frequency for the host flash SPI bus.
 *
 * @return 0 if port 1 has been successfully initialized or an error code.
 */
static int initialize_host_port1 (bool reset_notify, bool single_flash, uint32_t spi_freq)
{
	int status;

	if (!reset_notify) {
		host_irq = &host_irq_reset.base;

		status = host_irq_handler_mask_irqs_config_interrupts (&host_irq_reset);
	}
	else {
		host_irq = &host_irq_notify.base;

		status = host_irq_handler_auth_check_config_interrupts (&host_irq_notify);
	}
	if (status != 0) {
		return status;
	}

	status = initialize_host_firmware_pcr (single_flash);
	if (status != 0) {
		return status;
	}

	status = initialize_host_spi_frequency (spi_freq);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize management of the host firmware.
 *
 * @return 0 if host firmware management was successfully initialized or an error code.
 */
int initialize_host_firmware ()
{
	struct host_port_config config;
	int status;

	get_port_configuration (&config);

	status = host_gpio_irq_event_manager_init (&gpio_irq);
	if (status != 0) {
		return status;
	}

	status = host_irq_control_hsp_gpio_enable_irq (&gpio_irq_ctrl);
	if (status != 0) {
		return status;
	}

	status = host_state_manager_init_state (&host_state);
	if (status != 0) {
		return status;
	}

	if (config.reset_notify) {
		status = host_state_manager_add_observer (&host_state, &host_state_notify.base);
		if (status != 0) {
			return status;
		}
	}

	status = spi_filter_hsp_init_state (&host_filter);
	if (status != 0) {
		return status;
	}

	if (reset_source == RESET_POR) {
		/* The filter defaults to be enabled in bypass.  Disable the filter and wait for proper
		 * configuration. */
		status = host_filter.base.enable_filter (&host_filter.base, false);
		if (status != 0) {
			return status;
		}

		status = host_filter.base.set_filter_mode (&host_filter.base, SPI_FILTER_FLASH_DUAL);
		if (status != 0) {
			return status;
		}
	}

	status = spi_filter_hsp_irq_handler_init_state (&filter_irq, (reset_source == RESET_POR));
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_init_state (&filter_irq_task);
	if (status != 0) {
		return status;
	}

	status = initialize_host_pfm_management (config.reset_notify, config.run_time_activation);
	if (status != 0) {
		return status;
	}

	if (config.full_bypass) {
		if (config.dual_flash) {
			if (!config.reset_notify && config.reset_pulse) {
				if (config.reset_flash) {
					status =
						host_processor_dual_full_bypass_init_reset_flash_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
				else {
					status = host_processor_dual_full_bypass_init_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
			}
			else {
				if (config.reset_flash) {
					status = host_processor_dual_full_bypass_init_reset_flash (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
				else {
					status = host_processor_dual_full_bypass_init (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
			}
		}
		else {
			if (!config.reset_notify && config.reset_pulse) {
				if (config.reset_flash) {
					status =
						host_processor_single_full_bypass_init_reset_flash_pulse_reset (
						&host_manager, &host_manager_context, &host_gpio.base, &host_flash_single,
						&host_state, &host_filter.base, &host_fw_manifest.base, NULL,
						config.reset_pulse);
				}
				else {
					status = host_processor_single_full_bypass_init_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
			}
			else {
				if (config.reset_flash) {
					status = host_processor_single_full_bypass_init_reset_flash (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
				else {
					status = host_processor_single_full_bypass_init (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
			}
		}
	}
	else {
		if (config.dual_flash) {
			if (!config.reset_notify && config.reset_pulse) {
				if (config.reset_flash) {
					status = host_processor_dual_init_reset_flash_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
				else {
					status = host_processor_dual_init_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
			}
			else {
				if (config.reset_flash) {
					status = host_processor_dual_init_reset_flash (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_dual, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
				else {
					status = host_processor_dual_init (&host_manager, &host_manager_context,
						&host_gpio.base, &host_flash_dual, &host_state, &host_filter.base,
						&host_fw_manifest.base, NULL);
				}
			}
		}
		else {
			if (!config.reset_notify && config.reset_pulse) {
				if (config.reset_flash) {
					status = host_processor_single_init_reset_flash_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
				else {
					status = host_processor_single_init_pulse_reset (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL, config.reset_pulse);
				}
			}
			else {
				if (config.reset_flash) {
					status = host_processor_single_init_reset_flash (&host_manager,
						&host_manager_context, &host_gpio.base, &host_flash_single, &host_state,
						&host_filter.base, &host_fw_manifest.base, NULL);
				}
				else {
					status = host_processor_single_init (&host_manager, &host_manager_context,
						&host_gpio.base, &host_flash_single, &host_state, &host_filter.base,
						&host_fw_manifest.base, NULL);
				}
			}
		}
	}
	if (status != 0) {
		return status;
	}

	host_processor_set_port (&host_manager.base, 1);

	status = host_cmd_handler_init_state (&host_handler);
	if (status != 0) {
		return status;
	}

	status = initialize_host_port1 (config.reset_notify, !config.dual_flash, config.spi_freq);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Enable SPI filter interrupts and start the interrupt processing tasks.
 *
 * @return 0 if SPI filter interrupts were enabled successfully or an error code.
 */
int enable_spi_filter_interrupts ()
{
	int status;

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_IRQINTEN_SPI_FILT_INTEN_MSB,
		&filter_irq_group.base);
	if (status != 0) {
		return status;
	}

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_SPI_FILT_INTEN_MSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&filter_irq_task, &filter_irq_task_tcb,
		filter_irq_task_stack, FILTER_IRQ_TASK_STACK_WORDS, "FLT1", CERBERUS_PRIORITY_NORMAL);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&filter_irq_task);

	return 0;
}
