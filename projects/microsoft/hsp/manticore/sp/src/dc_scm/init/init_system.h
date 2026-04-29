// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_SYSTEM_H_
#define INIT_SYSTEM_H_

#include <stdbool.h>
#include "hsp_top.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cmd_authorization_static.h"
#include "cmd_interface/cmd_device_hsp_freertos_static.h"
#include "cmd_interface/config_reset_static.h"
#include "cmd_interface/counter_manager_registers.h"
#include "dc_scm/1sp/manticore_1sp.h"
#include "dc_scm/rot_memory_map.h"
#include "drivers/fuse_controller_manticore_fips_static.h"
#include "drivers/hsp_dmb_static.h"
#include "drivers/hsp_rng_hw_static.h"
#include "firmware/graceful_shutdown_static.h"
#include "mmio/mmio_register_block_hsp_static.h"
#include "state_manager/state_persistence_handler_static.h"
#include "system/hsp_watchdog_static.h"
#include "system/security_manager_hsp_manticore_static.h"
#include "system/security_policy_hsp_manticore_static.h"
#include "system/system.h"
#include "system/system_state_manager_static.h"


/**
 * The validity time for any one-time authorization token issued by the device.
 */
#define	MANTICORE_AUTH_TOKEN_EXPIRATION		(24 * 60 * 60 * 1000)	/* 24 hours */


extern const struct manticore_1sp_shared_data *const sp1_shared;
extern const struct mmio_register_block_hsp creg_regs;
extern const struct hsp_watchdog watchdog;

extern const char *const fw_version_list[6];
extern const struct cmd_interface_fw_version firmware_version;

#ifdef MANTICORE_ENABLE_A0_SUPPORT
extern uint32_t soc_revision;
#endif
extern bool recovery_boot;
extern int reset_source;

extern const struct fuse_controller fuses;
extern const struct hsp_rng_hw rng_hw;
extern const struct hsp_dmb dmb;

extern const struct state_manager system_state;
extern const struct state_persistence_handler state_persist;
extern struct counter_manager_registers reset_counter;
extern const struct cmd_device_hsp_freertos device_cmd;
extern const struct graceful_shutdown graceful_shutdown;
extern struct system system_mgr;
extern const struct security_policy_hsp_manticore sec_policy;
extern const struct security_manager_hsp_manticore security_mgr;
extern const struct cmd_authorization cmd_auth;
extern const struct config_reset config_manager;


bool is_a0_bypass ();
bool is_secure_boot_enabled ();

void determine_hsp_clock_frequency ();
void hardware_init (const char **reset_str);

int verify_1sp_shared_data (const struct hash_engine *hash);

int initialize_mpu ();
int finalize_mpu ();

int initialize_rng ();
int initialize_soc ();
int initialize_system_management ();
int initialize_config_reset_management ();

int start_watchdog_task ();

int initialize_persistence_task ();
int start_persistence_task ();

int enable_stack_usage_monitoring ();


#endif	/* INIT_SYSTEM_H_ */
