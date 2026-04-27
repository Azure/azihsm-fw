// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_MANIFEST_H_
#define INIT_MANIFEST_H_

#include <stdbool.h>
#include "event_task_freertos_static.h"
#include "manifest/manifest_verification.h"
#include "manifest/pcd/manifest_cmd_handler_pcd_static.h"
#include "manifest/pcd/pcd_manager_flash_static.h"
#include "manifest/pcd/pcd_observer_pcr_static.h"
#include "manifest/pfm/manifest_cmd_handler_pfm.h"
#include "manifest/pfm/pfm_manager_flash_static.h"
#include "manifest/pfm/pfm_observer_pcr_static.h"


extern const struct manifest_verification_key_ecc manifest_ecc_key;

extern const struct event_task_freertos manifest_cmd_task;

extern const struct pcd_manager_flash platform_config;
extern const struct manifest_cmd_handler_pcd pcd_handler;
extern const struct pcd_observer_pcr pcr_pcd;
extern bool has_active_pcd;

extern const struct pfm_manager_flash host_fw_manifest;
extern struct manifest_cmd_handler_pfm pfm_handler;
extern const struct pfm_observer_pcr pcr_pfm;


int initialize_pcd_management ();
int initialize_host_pfm_management (bool reset_notify, bool run_time_activation);

int initialize_manifest_command_task ();
int start_manifest_command_task ();


#endif	/* INIT_MANIFEST_H_ */
