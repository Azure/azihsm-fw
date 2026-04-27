// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TELEMETRY_TEMPERATURE_HANDLER_STATIC_H_
#define TELEMETRY_TEMPERATURE_HANDLER_STATIC_H_

#include "telemetry_temperature_handler.h"


/* Internal functions declared to allow for static initialization. */
void telemetry_temperature_handler_prepare (const struct periodic_task_handler *handler);
const platform_clock* telemetry_temperature_handler_get_next_execution (
	const struct periodic_task_handler *handler);
void telemetry_temperature_handler_execute (const struct periodic_task_handler *handler);


/**
 * Constant initializer for the telemetry temperature handler API.
 */
#define	TELEMETRY_TEMPERATURE_HANDLER_API_INIT  { \
		.prepare = telemetry_temperature_handler_prepare, \
		.get_next_execution = telemetry_temperature_handler_get_next_execution, \
		.execute = telemetry_temperature_handler_execute, \
	}


/**
 * Initialize a static instance of a telemetry temperature handler.  This does not initialize the
 * handler state. This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the handler.
 * @param sensor_ptr The temperature sensor to be monitored.
 * @param threshold_ptr The structure containing the threshold values for the sensor.
 * @param period_ms The amount of time between telemetry temperature monitoring requests, in milliseconds.
 */
#define	telemetry_temperature_handler_static_init(state_ptr, sensor_ptr, threshold_ptr, period_ms)	{ \
		.base = TELEMETRY_TEMPERATURE_HANDLER_API_INIT, \
		.state = state_ptr, \
		.sensor = sensor_ptr, \
		.threshold = threshold_ptr, \
		.period = period_ms, \
	}


#endif	/* TELEMETRY_TEMPERATURE_HANDLER_STATIC_H_ */
