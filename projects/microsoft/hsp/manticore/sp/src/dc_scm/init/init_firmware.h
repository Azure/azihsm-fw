// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_FIRMWARE_H_
#define INIT_FIRMWARE_H_

#include "event_task_freertos_static.h"
#include "firmware/authorized_execution_allow_impactful_static.h"
#include "firmware/authorized_execution_prepare_firmware_update_static.h"
#include "firmware/firmware_image_manticore_static.h"
#include "firmware/firmware_update_handler_revoke_after_reset_static.h"
#include "firmware/firmware_update_static.h"
#include "firmware/impactful_update_handler_static.h"
#include "firmware/key_manifest_hsp_firmware_static.h"


extern const struct key_manifest_hsp_firmware img_keys;
extern const struct firmware_image_manticore updating_img;
extern struct firmware_update fw_updater;
extern const struct firmware_update_handler_revoke_after_reset fw_handler;
extern const struct impactful_update_handler impactful_handler;
extern const struct event_task_freertos manticore_update;

extern const struct authorized_execution_prepare_firmware_update prepare_fw_update;
extern const struct authorized_execution_allow_impactful allow_impactful_execution;


int initialize_running_image_access ();
int initialize_firmware_updater (bool impactful_check);

int allocate_firmware_update_task ();


#endif	/* INIT_FIRMWARE_H_ */
