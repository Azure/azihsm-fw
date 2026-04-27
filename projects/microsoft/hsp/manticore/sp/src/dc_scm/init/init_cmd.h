// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_CMD_H_
#define INIT_CMD_H_

#include "event_task_freertos_static.h"
#include "periodic_task_freertos_static.h"
#include "platform_api.h"
#include "attestation/attestation_responder.h"
#include "fips/fips_self_test_manticore_static.h"
#include "mctp/mctp_interface_static.h"
#include "system/temperature_sensor_tsen_static.h"


/* Default I2C communication parameters. */
#ifndef BUILD_FOR_EVB
#define DEFAULT_I2C_SLAVE_ADDR						0x41
#else
/* There is an I2C address conflict on EVB, so need to change the address for that target. */
#define DEFAULT_I2C_SLAVE_ADDR						0x35
#endif
#define DEFAULT_BMC_SLAVE_ADDRESS					0x10
#define DEFAULT_IS_PA_ROT							false


extern struct attestation_responder system_attestation_responder;
extern const struct event_task_freertos cmd_background_task;
extern const struct periodic_task_freertos system_cmd_task;
extern const struct mctp_interface mctp_transport;
extern const struct temperature_sensor_tsen tsen;
extern const struct fips_self_test_manticore self_test;


int initialize_cmd_interface_recovery ();
int initialize_cmd_interface ();
int start_cmd_interface ();


#endif	/* INIT_CMD_H_ */
