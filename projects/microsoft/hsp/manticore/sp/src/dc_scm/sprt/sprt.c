// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "manticore_hsp_gpio.h"
#include "manticore_sticky_regs.h"
#include "platform_api.h"
#include "platform_io_api.h"
#include "queue.h"
#include "reset_counter_init.h"
#include "sp_boot.h"
#include "system_observer_stack_usage.h"
#include "task.h"
#include "task_priority.h"
#include "attestation/attestation_logging.h"
#include "cmd_interface/cerberus_protocol_required_commands.h"
#include "common/unused.h"
#include "crashdump/hsp_crashdump_logging.h"
#include "crashdump/soc_crashdump_handler.h"
#include "freertos/hsp_freertos.h"
#include "host_fw/host_logging.h"
#include "init/init_attestation.h"
#include "init/init_cmd.h"
#include "init/init_crashdump.h"
#include "init/init_crypto.h"
#include "init/init_ephemeral_key.h"
#include "init/init_firmware.h"
#include "init/init_flash.h"
#include "init/init_host.h"
#include "init/init_intrusion.h"
#include "init/init_ipc.h"
#include "init/init_log.h"
#include "init/init_log_flush_handlers.h"
#include "init/init_manifest.h"
#include "init/init_spdm.h"
#include "init/init_system.h"
#include "init/init_tdisp.h"
#include "init/task_stack_size.h"
#include "logging/init_logging.h"
#include "logging/logging_memory_static.h"
#include "logging/manticore_logging.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "system/system_logging.h"
#include "trap/hsp_interrupt.h"
#include "trap/hsp_trap.h"

#ifdef MANTICORE_ENABLE_ACVP
#include "init/init_acvp.h"
#endif


/**
 * Timeout for IPC messages for releasing the SoC cores during device boot.
 */
#define	IPC_RELEASE_SOC_CORES_TIMEOUT				500

#ifndef MANTICORE_ENABLE_ACVP
/**
 * Statically allocated task control block for the host Port 1 handler task.
 */
static StaticTask_t port1_task_tcb;

/**
 * Statically allocated stack for the host Port 1 handler task.
 */
static StackType_t port1_task_stack[HOST_PORT1_TASK_STACK_WORDS];


/**
 * Print the total configuration of the SPI filter to the UART.
 */
static void print_filter_configuration ()
{
	int j;
	uint32_t mask;
	uint8_t data;
	spi_filter_cs ro;
	bool flag;
	spi_filter_address_mode addr;
	bool addr_fixed;
	spi_filter_address_mode addr_reset = SPI_FILTER_ADDRESS_MODE_3;
	bool write_en = false;
	spi_filter_flash_state dirty;
	spi_filter_flash_mode mode;
	bool write_allow = false;
	uint32_t lower[6];
	uint32_t upper[6];
	uint32_t bytes;

	platform_printf (NEWLINE);

	mask = host_filter.get_interrupt_enable (&host_filter);
	platform_printf ("Int Mask: 0x%x" NEWLINE, mask);

	platform_printf ("SPI Filter 1:" NEWLINE);

	host_filter.base.get_mfg_id (&host_filter.base, &data);
	platform_printf ("\tFlash Manufacturer: 0x%x" NEWLINE, data);

	host_filter.base.get_flash_size (&host_filter.base, &bytes);
	platform_printf ("\tFlash Size: 0x%x" NEWLINE, bytes);

	host_filter.base.get_filter_mode (&host_filter.base, &mode);
	platform_printf ("\tFlash Mode: %d" NEWLINE, mode);

	host_filter.base.are_all_single_flash_writes_allowed (&host_filter.base, &write_allow);
	platform_printf ("\tSingle Chip Full Write: %d" NEWLINE, write_allow);

	host_filter.base.get_filter_enabled (&host_filter.base, &flag);
	platform_printf ("\tSPI Filter: %s" NEWLINE, (flag) ? "enabled" : "disabled");

	host_filter.base.get_ro_cs (&host_filter.base, &ro);
	platform_printf ("\tRead-Only Flash: %d" NEWLINE, ro);

	host_filter.base.get_addr_byte_mode (&host_filter.base, &addr);
	platform_printf ("\tAddress Mode: %d" NEWLINE, addr);

	host_filter.base.get_fixed_addr_byte_mode (&host_filter.base, &addr_fixed);
	platform_printf ("\tFixed Address Mode: %d" NEWLINE, addr_fixed);

	host_filter.base.get_reset_addr_byte_mode (&host_filter.base, &addr_reset);
	platform_printf ("\tReset Address Mode: %d" NEWLINE, addr_reset);

	host_filter.base.get_addr_byte_mode_write_enable_required (&host_filter.base, &write_en);
	platform_printf ("\tWrite Enable for Address Mode: %d" NEWLINE, write_en);

	host_filter.base.get_flash_dirty_state (&host_filter.base, &dirty);
	platform_printf ("\tRead-Only Dirty: %d" NEWLINE, dirty);

	for (j = 0; j < 6; j++) {
		host_filter.base.get_filter_rw_region (&host_filter.base, j + 1, &lower[j], &upper[j]);
		platform_printf ("\tRead/Write Region %d: 0x%x - 0x%x" NEWLINE, j + 1, lower[j], upper[j]);
	}

	spi_filter_log_filter_config (1, data, flag, ro, addr, addr_fixed, addr_reset, write_en, dirty,
		mode, write_allow, lower, upper, 6, bytes);

	platform_printf (NEWLINE);
}

/**
 * Print the list of programmable opcodes from the HSP SPI filter.
 */
static void print_filter_opcodes ()
{
	int i;
	uint32_t opcode;

	platform_printf ("SPI FIlter 1 opcodes:" NEWLINE);
	for (i = 0; i < 60; i++) {
		/* No need to introduce an API for this.  Just access the registers directly. */
		opcode = host_filter.regs->spi_filter_prg_opcodes.spi_filter_prg_opcode[i];
		platform_printf ("\t%d: %x" NEWLINE, i, opcode);
	}

	platform_printf (NEWLINE);
}

/**
 * Print the current information regarding the presence of PFMs.  This information will also be
 * added to the debug log.
 *
 * @param pfm The manager to query to determine PFM status.
 * @param port The port number of the manager.
 * @param log_msg The identifier for the log message.
 */
static void print_pfm_status (const struct pfm_manager *pfm, int port, int log_msg)
{
	const struct pfm *active;
	const struct pfm *pending;

	active = pfm->get_active_pfm (pfm);
	pending = pfm->get_pending_pfm (pfm);

	platform_printf ("Port %d PFM: active=0x%x, pending=0x%x" NEWLINE, port, active, pending);
	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT, log_msg,
		(active != NULL), (pending != NULL));

	pfm->free_pfm (pfm, active);
	pfm->free_pfm (pfm, pending);
}

/**
 * Handle POR processing for host firmware.
 *
 * @param port The port number to process.
 * @param handler The IRQ handler for the host manager.
 * @param pfm The manager for port PFMs.
 * @param pfm_before The log message to use for PFM state before processing.
 * @param pfm_after The log message to use for PFM state after processing.
 */
static void port_por_processing (int port, const struct host_irq_handler *handler,
	const struct pfm_manager *pfm, int pfm_before, int pfm_after)
{
	int status;

	print_pfm_status (pfm, port, pfm_before);

	if (reset_source == RESET_POR) {
		status = handler->power_on (handler, true, &host_hash.base);
		if (status != 0) {
			platform_printf ("Port %d POR failed: 0x%x" NEWLINE, port, status);
		}
	}

	pfm_observer_pcr_record_measurement (&pcr_pfm, pfm);
	print_pfm_status (pfm, port, pfm_after);
}

/**
 * Handle IRQ events from a host processor.  This call will never return.
 *
 * @param port The port number for the host.
 * @param irq The handle for host IRQs.
 * @param notification The notification queue generating events.
 */
static void port_irq_handler (int port, const struct host_irq_handler *irq,
	QueueHandle_t notification)
{
	uint8_t event;
	platform_clock start;
	platform_clock finish;
	int status;

	while (1) {
		xQueueReceive (notification, &event, portMAX_DELAY);

		switch (event) {
			case GPIO_IRQ_RESET_ENTER:
				platform_printf ("Received Port %d Reset Enter IRQ." NEWLINE, port);
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_ENTER_RESET, port, 0);

				platform_init_current_tick (&start);
				print_filter_configuration ();

				status = irq->enter_reset (irq);
				if (status != 0) {
					platform_printf ("Failed host %d reset processing: 0x%x" NEWLINE, port, status);
				}

				print_filter_configuration ();

				if (port == 0) {
					status = counter_manager_registers_increment (&reset_count,
						CERBERUS_PROTOCOL_COMPONENT_RESET, port);
					if (status != 0) {
						debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR,
							DEBUG_LOG_COMPONENT_HOST_FW, HOST_LOGGING_RESET_COUNTER_UPDATE_FAILED,
							port, status);
					}
				}

				platform_init_current_tick (&finish);
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT,
					INIT_LOGGING_RESET_PROCESSING_TIME, (WARM_RESET_PROCESSING << 8) | port,
					platform_get_duration (&start, &finish));

				break;

			case GPIO_IRQ_RESET_EXIT:
				platform_printf ("Received Port %d Reset Exit IRQ." NEWLINE, port);
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_EXIT_RESET, port, 0);

				irq->exit_reset (irq);
				break;

			case GPIO_IRQ_CS0_ASSERTED:
				platform_printf ("Received Port %d CS0 IRQ." NEWLINE, port);
				irq->assert_cs0 (irq);
				break;

			case GPIO_IRQ_CS1_ASSERTED:
				platform_printf ("Received Port %d CS1 IRQ." NEWLINE, port);
				platform_init_current_tick (&start);

				status = irq->assert_cs1 (irq);
				if (status != 0) {
					platform_printf ("Failed host %d CS1 processing: 0x%x" NEWLINE, port, status);
				}

				platform_init_current_tick (&finish);
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT,
					INIT_LOGGING_RESET_PROCESSING_TIME, (BMC_CS1_PROCESSING << 8) | port,
					platform_get_duration (&start, &finish));

				break;

			case GPIO_IRQ_HOST_DOWN:
				platform_printf ("Received Port %d Host Down IRQ." NEWLINE, port);
				if (port == 1) {
					status = counter_manager_registers_increment (&reset_count,
						CERBERUS_PROTOCOL_COMPONENT_RESET, port);
					if (status != 0) {
						debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR,
							DEBUG_LOG_COMPONENT_HOST_FW, HOST_LOGGING_RESET_COUNTER_UPDATE_FAILED,
							port, status);
					}
				}
				break;

			case GPIO_IRQ_HOST_UP:
				platform_printf ("Received Port %d Host Up IRQ." NEWLINE, port);
				break;
		}
	}
}

/**
 * Task that will handle port 1 events.
 *
 * @param init_task Handle for the init task.
 */
static void manticore_port1_handler (void *init_task)
{
	platform_clock start;
	platform_clock finish;
	struct pcd_port_info pcd_config;
	const struct pcd *active_pcd = platform_config.base.get_active_pcd (&platform_config.base);
	bool reset_ctrl;
	int reset_action;
	int status;

	/* Wait for init to complete before starting host validation. */
	ulTaskNotifyTake (pdTRUE, portMAX_DELAY);

	platform_init_current_tick (&start);

	port_por_processing (1, host_irq, &host_fw_manifest.base, INIT_LOGGING_PORT1_PFMS_BEFORE_POR,
		INIT_LOGGING_PORT1_PFMS_AFTER_POR);

	/* Force the current state to be flushed to flash.  Ideally, there would be a way to force this
	 * to run in the persistence task context, but that would require bigger changes to APIs.  Since
	 * the host handling task is higher priority than the persistence task, it should be safe to
	 * force this call here. */
	state_persist.base.execute (&state_persist.base);

	platform_init_current_tick (&finish);
	debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_INIT,
		INIT_LOGGING_RESET_PROCESSING_TIME, (CERBERUS_RESET_PROCESSING << 8) | 1,
		platform_get_duration (&start, &finish));

	host_gpio_irq_event_manager_enable_notifications (&gpio_irq, true);

	/* We need to be sure the reset control line for Port 1 is set to the correct state when
	 * initializing from a soft reset.  Otherwise, we could miss opportunities for host firmware
	 * verification due to host warm reboots.  This only applies to notify reset control. */
	reset_action = host_manager.base.get_next_reset_verification_actions (&host_manager.base);
	reset_ctrl = (reset_source != RESET_POR) && (reset_action != HOST_PROCESSOR_ACTION_NONE);

	if (active_pcd) {
		status = active_pcd->get_port_info (active_pcd, 1, &pcd_config);
		if (status == 0) {
			reset_ctrl = reset_ctrl && (pcd_config.reset_ctrl == PCD_PORT_RESET_CTRL_NOTIFY);
		}
		else {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
				INIT_LOGGING_PORT_CONFIG, 1, status);
		}

		platform_config.base.free_pcd (&platform_config.base, active_pcd);
	}

	/* The reset control line is active low. */
	hsp_gpio_write (&gpio, PORT1_RESET_CTRL, !reset_ctrl);

	print_filter_configuration ();
	print_filter_opcodes ();

	xTaskNotifyGive ((TaskHandle_t) init_task);
	port_irq_handler (1, host_irq, gpio_irq.event_queue);
}
#endif	/* MANTICORE_ENABLE_ACVP */

/*
 * Get the start time of SPRT & calculate 1SP boot time (SPRT_start_time - 1SP_start_time)
 */
static void sprt_boot_time_report ()
{
#ifdef MANTICORE_SPRT_BOOT_TIME_REPORT
	platform_clock start_time_sprt;

	platform_init_current_tick (&start_time_sprt);

	platform_printf (NEWLINE);
	platform_printf ("1SP boot_time %d" NEWLINE,
		platform_get_duration (&sp1_shared->start_time_1sp, &start_time_sprt));
	platform_printf ("1SP load_main_firmware %d" NEWLINE,
		platform_get_duration (&sp1_shared->start_time_1sp_load_main_fw,
		&sp1_shared->finish_time_1sp_load_main_fw));
	platform_printf ("1SP SPRT & CP generate Dice Key %d" NEWLINE,
		platform_get_duration (&sp1_shared->start_time_1sp_gen_dice_key,
		&sp1_shared->finish_time_1sp_gen_dice_key));
#endif
}

/**
 * Task that will run system initialization.
 *
 * @param unused Unused.
 */
static void manticore_init (void *unused)
{
	int status;
	int error_msg = -1;
	int msg_arg = 0;
	bool is_graceful;

#ifndef MANTICORE_ENABLE_ACVP
	TaskHandle_t port1_task;
#endif

	UNUSED (unused);

	/* Initialize core system components before starting any system tasks.  However, immediately
	 * start the watchdog task to ensure it gets properly refreshed during initialization. */

	status = start_watchdog_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_HEARTBEAT;
		goto reset;
	}

	status = initialize_manticore_flash ();
	if (status != 0) {
		goto reset;
	}

	initialize_debug_log ();

	status = initialize_log_flush_handlers ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SOC_TELEMETRY;
		goto reset;
	}

	status = initialize_crashdump_hsp ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_CRASHDUMP_HANDLER;
		goto reset;
	}

	status = initialize_dme_key_export ();
	if (status != 0) {
		error_msg = INIT_LOGGING_DME_PUBLIC_KEY;
		goto reset;
	}

	status = initialize_crypto_hardware ();
	if (status != 0) {
		error_msg = INIT_LOGGING_HW_CRYPTO;
		goto reset;
	}

	status = initialize_system_crypto ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_CRYPTO;
		goto reset;
	}

	status = run_crypto_self_tests ();
	if (status != 0) {
		error_msg = INIT_LOGGING_CRYPTO_KAT;
		goto reset;
	}

	status = verify_1sp_shared_data (&shared_hash.base);
	if (status != 0) {
		error_msg = INIT_LOGGING_PCR_VERIFY;
		goto reset;
	}

	status = initialize_soc ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_SOC;
		goto reset;
	}

	status = initialize_dice_key_manager ();
	if (status != 0) {
		error_msg = INIT_LOGGING_RIOT_MANAGER;
		goto reset;
	}

	status = initialize_manticore_measurements ();
	if (status != 0) {
		error_msg = INIT_LOGGING_PCR_STORE;
		goto reset;
	}

	status = initialize_system_management ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_STATE;
		goto reset;
	}

#ifdef DEBUG_STACK_USAGE
	status = enable_stack_usage_monitoring ();
	if (status != 0) {
		error_msg = INIT_LOGGING_SYSTEM_STATE;
		goto reset;
	}
#endif

	status = initialize_running_image_access ();
	if (status != 0) {
		error_msg = INIT_LOGGING_RUNNING_IMG;
		goto reset;
	}

	status = initialize_firmware_updater (true);
	if (status != 0) {
		error_msg = INIT_LOGGING_FW_UPDATER;
		goto reset;
	}

#ifndef MANTICORE_ENABLE_ACVP
	status = initialize_pcd_management ();
	if (status != 0) {
		error_msg = INIT_LOGGING_PCD_MANAGEMENT;
		goto reset;
	}
#endif

#ifndef MANTICORE_ENABLE_ACVP
	status = initialize_host_gpios ();
	if (status != 0) {
		error_msg = INIT_LOGGING_GPIO_IRQ;
		goto reset;
	}
#endif

	status = initialize_host_flash_access ();
	if (status != 0) {
		error_msg = INIT_LOGGING_HOST_FLASH;
		msg_arg = 1;
		goto reset;
	}

#ifndef MANTICORE_ENABLE_ACVP
	status = initialize_host_firmware ();
	if (status != 0) {
		error_msg = INIT_LOGGING_HOST_FW;
		msg_arg = 1;
		goto reset;
	}
#endif

	status = initialize_config_reset_management ();
	if (status != 0) {
		error_msg = INIT_LOGGING_CONFIG_MGMT;
		goto reset;
	}

	status = initialize_intrusion ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_INTRUSION;
		goto reset;
	}

	status = initialize_persistence_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_LOG_TASK;
		goto reset;
	}

#ifndef MANTICORE_ENABLE_ACVP
	status = initialize_manifest_command_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_CONFIG_CMD_TASK;
		goto reset;
	}
#endif

	status = initialize_cmd_interface ();
	if (status != 0) {
		error_msg = INIT_LOGGING_COMMAND_HANDLER;
		goto reset;
	}

	status = initialize_doe_spdm_responder ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_SPDM_RESPONDER;
		goto reset;
	}

	status = initialize_ipc ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_IPC;
		goto reset;
	}

	status = initialize_ephemeral_key_handler ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_EPHEMERAL_KEY_MANAGER;
		goto reset;
	}

#ifdef MANTICORE_ENABLE_ACVP
	status = initialize_acvp ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_ACVP;
		goto reset;
	}
#endif

	status = initialize_error_state_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_ERROR_STATE_TASK;
		goto reset;
	}

	/* Everything has been initialized.  Allocate and start running system tasks. */

#ifndef MANTICORE_ENABLE_ACVP
	status = enable_spi_filter_interrupts ();
	if (status != 0) {
		/* Not really CPLD interrupts, but represents the same functionality. */
		error_msg = INIT_LOGGING_CPLD_IRQ;
		goto reset;
	}

	status = start_manifest_command_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_CONFIG_CMD_TASK;
		goto reset;
	}
#endif

	status = start_error_state_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_START_ERROR_STATE_TASK;
		goto reset;
	}

	status = start_persistence_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_LOG_TASK;
		goto reset;
	}

	status = start_intrusion ();
	if (status != 0) {
		error_msg = INIT_LOGGING_START_INTRUSION;
		goto reset;
	}

	status = start_ipc_handlers ();
	if (status != 0) {
		error_msg = INIT_LOGGING_INIT_IPC;
		goto reset;
	}

	status = start_ephemeral_key_manager ();
	if (status != 0) {
		error_msg = INIT_LOGGING_START_EPHEMERAL_KEY_MANAGER;
		goto reset;
	}

	/* Do not start the FW update task until after full SoC initialization has completed.  This
	 * prevents copying of the active image from happening prematurely. */
	status = allocate_firmware_update_task ();
	if (status != 0) {
		error_msg = INIT_LOGGING_FW_UPDATE_TASK;
		goto reset;
	}

	status = start_cmd_interface ();
	if (status != 0) {
		error_msg = INIT_LOGGING_COMMAND_HANDLER_START;
		goto reset;
	}

#ifndef MANTICORE_ENABLE_ACVP
	/* TODO:  Confirm stack requirements on this platform. */
	port1_task = xTaskCreateStatic (manticore_port1_handler, "Port1", HOST_PORT1_TASK_STACK_WORDS,
		xTaskGetCurrentTaskHandle (), CERBERUS_PRIORITY_NORMAL, port1_task_stack, &port1_task_tcb);
#endif

	/* Internal initialization has completed, so apply final MPU settings. */
	status = finalize_mpu ();
	if (status != 0) {
		error_msg = INIT_LOGGING_MPU_PROTECTION;
		goto reset;
	}

	is_graceful = is_graceful_reset ();

	status = graceful_shutdown_resume_normal_operation (&graceful_shutdown,
		IPC_RELEASE_SOC_CORES_TIMEOUT);
	if (status != 0) {
#ifndef MANTICORE_DISABLE_CRASHDUMP
		/* The handshaking between SP and CP failed. Collect ARM crash dumps if they exist. */
		error_msg = soc_crashdump_handler_get_crashdumps (&soc_handler, NULL, NULL);
		if (error_msg != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_SOC_CRASHDUMP_COLLECTION_FAILED, error_msg, 0);
		}
#endif	// MANTICORE_DISABLE_CRASHDUMP
		error_msg = INIT_LOGGING_IPC_SYNC;
		goto reset;
	}

	status = initialize_tdisp (is_graceful);
	if (status != 0) {
		error_msg = INIT_LOGGING_TDISP;
		goto reset;
	}

	impactless_update_done ();
	configure_normal_boot_order ();
	fips_self_test_manticore_on_demand_done (&self_test);
	soc_crashdump_handler_start_crash_monitor (&soc_handler);
	platform_printf ("System Initialized." NEWLINE NEWLINE);

	/* Starting the command handler after full system initialization
	 * to ensure no external commands can be processed until all cores are running normally. */
	periodic_task_freertos_start (&system_cmd_task);

#ifndef MANTICORE_ENABLE_ACVP
	/* Authenticate Port 1. */
	xTaskNotifyGive (port1_task);
	ulTaskNotifyTake (pdFALSE, portMAX_DELAY);
#endif

	/* At this point, core initialization has completed and the hosts are running.  Set the SPRT
	 * flag to detect the state correctly on the next reset and not interrupt host operation.  The
	 * init flag is not set before all authentication is done to ensure proper handling of
	 * unexpected reset events during flash validation. */
	system_init_done ();

	/* Start the FW updater task, which will execute image copy and revocation flows.  Before this
	 * is done, FW update commands other than a request for status will not be handled.  Notably,
	 * this means waiting for host flash authentication to finish, which isn't really an issue until
	 * there is a scenario where PFMs are used. */
	event_task_freertos_start (&manticore_update);

#ifdef DEBUG_STACK_USAGE
	system_observer_stack_usage_print_all_tasks_usage ();
#endif

	/* Full initialization has completed successfully. */
	boot_error_clear_counter (sw_regs);

	/* The init task is done, so free it. */
	vTaskDelete (NULL);

reset:
	/* If we haven't completed system initialization yet, make sure the external hosts are still
	 * kept in reset. */
	if ((reset_source == RESET_POR) && !is_sys_init_done ()) {
		hsp_gpio_write (&gpio, PORT1_RESET_CTRL, 0);
	}

	if (error_msg >= 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT, error_msg,
			status, msg_arg);

		platform_printf ("System initialization failed: msg=%d, status=0x%x, arg=%d" NEWLINE,
			error_msg, status, msg_arg);
	}
	else {
		platform_printf ("System initialization failed: status=0x%x" NEWLINE, status);
	}
	log_flush_handler_immediate_flush (soc_handler.log_flush);
	debug_log_flush ();

	/* Never halt the system.  Reboot and try again. */
	platform_printf (NEWLINE);

	/* In impactless recovery scenarios, the graceful reset flag can cause repeated boot
	 * failures if left uncleared.This check clears the flag to allow a successful boot on
	 * the next recovery attempt.*/
	if (recovery_boot) {
		clear_graceful_reset ();
	}

	boot_error_reset (sw_regs);
}

/**
 * Callback for monitoring the FreeRTOS tick.
 *
 * This is only used when configUSE_TICK_HOOK is enabled.
 */
void vApplicationTickHook ()
{
	platform_printf ("TICK" NEWLINE);
}

/**
 * Callback when FreeRTOS detects a task has overflowed its stack.
 *
 * @param xTask The task whose stack has overflowed.
 * @param pcTaskName Name of the task.
 */
void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName)
{
	struct hsp_crashdump_packet_hsp_production_packet packet;
	struct hsp_trap_context ctx = {0};
	TaskStatus_t stats;

	/* A task has overflowed its stack and corrupted memory.  The system needs to be considered
	 * unreliable and memory in an indeterminate state.  As such, do not rely on anything that is
	 * not protected from modification while handling this condition.
	 *
	 * Get the task stack pointer, which is used to indicate the offending task. */
	vTaskGetInfo (xTask, &stats, pdFALSE, eDeleted);
	ctx.regs.sp = (uintptr_t) stats.pxStackBase;
	platform_printf (NEWLINE "!!! STACK OVERFLOW: %s !!!" NEWLINE NEWLINE, pcTaskName);

	/* Drop a breadcrumb indicating that the stack overflow has happened so that a log message can
	 * be generated after the device is reset.  No crash dump is necessary for this flow since the
	 * crash would just point to this location, which is not where the stack actually overflowed, so
	 * is not interesting from a diagnostics perspective.  However, the crash dump collection is
	 * reused in this case to save this overflow condition. */
	hsp_crashdump_logging_collect_crashdump (HSP_CRASHDUMP_PACKET_FAULT_CODE_STACK_OVER_FLOW,
		CRASH_DUMP_CRASH_TYPE_NORMAL, &ctx, sp1_shared->fw_descriptor.build_ver,
		FW_COMPONENT_BUILD_VERSION_LENGTH, &packet);

	hsp_crashdump_logging_save_crashdump_to_persistent_ram (&packet,
		MANTICORE_STICKY_REG (MANTICORE_CRASHDUMP_0),
		(MANTICORE_CRASHDUMP_18 - MANTICORE_CRASHDUMP_0 + 1) * sizeof (uint32_t));

	/* Crash warm reset */
	boot_error_reset ();
}

/**
 * Stack guard to check for overflows.
 */
extern uint32_t __stack_chk_guard;


/**
 * Entry point for Manticore SPRT.
 */
int main ()
{
	int status;
	const char *reset;
	uint32_t random_val;

	hardware_init (&reset);

	sprt_boot_time_report ();

#ifdef MANTICORE_DISABLE_WATCHDOG
	/* Stop the hardware timer so the watchdog will never trip. */
	hsp_watchdog_stop (&watchdog);
#endif

	platform_printf (NEWLINE);
	platform_printf ("Manticore SPRT: %s" NEWLINE, sp1_shared->version_sprt);
	platform_printf ("System Clock: %d Hz" NEWLINE, HSP_CLOCK_FREQUENCY_HZ);
	platform_printf ("Boot Device: %s" NEWLINE, recovery_boot ? "Recovery" : "Main");
	platform_printf ("Reset Cause: %s" NEWLINE, reset);
	platform_printf (NEWLINE);

	status = initialize_rng ();
	if (status != 0) {
		platform_printf ("RNG FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* Apply a random value to the stack guard.  It can't be done in the context of a function
	 * call */
	hsp_rng_hw_get_random_word (&rng_hw, &random_val);
	__stack_chk_guard = random_val;

	/* Now that the stack guard is configured (and before starting the RTOS), configure the MPU
	 * settings. */
	status = initialize_mpu ();
	if (status != 0) {
		platform_printf ("MPU FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* Initialize and enable interrupts. */
	status = hsp_interrupt_init (true);
	if (status != 0) {
		platform_printf ("IRQ INIT FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	status = hsp_freertos_init ();
	if (status != 0) {
		platform_printf ("FreeRTOS INIT FAILED: 0x%x" NEWLINE, status);
		goto error;
	}

	/* This can be a low priority task since nothing else is running during initialization.  Once
	 * init has completed, it will run additional background operations that need to be low
	 * priority. */
	status = xTaskCreate (manticore_init, "Init", 7 * 256, NULL, CERBERUS_PRIORITY_BACKGROUND,
		NULL);
	if (status == pdPASS) {
		vTaskStartScheduler ();
		platform_printf ("Returned from FreeRTOS scheduler!?" NEWLINE);
	}
	else {
		platform_printf ("Failed to create init task (0x%x)!" NEWLINE, status);
	}

error:
	/* Never halt the system.  Reboot and try again. */
	boot_error_reset ();

	/* Should never get here.  Just wait for a reset. */
	CEASE;
}
