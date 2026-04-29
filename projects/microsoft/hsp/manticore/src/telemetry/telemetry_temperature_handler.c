// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "telemetry_temperature_handler.h"
#include "common/unused.h"


/**
 * Temperature handler states.
 */
enum {
	TELEMETRY_TEMPERATURE_HANDLER_INITIALIZED = 0,	/**< Temperature handler has been initialized. */
	TELEMETRY_TEMPERATURE_HANDLER_RUNNING_NORMAL,	/**< Temperature handler is running. */
	TELEMETRY_TEMPERATURE_HANDLER_RUNNING_ABNORMAL,	/**< Temperature handler is running, but temperature is out of range. */
	TELEMETRY_TEMPERATURE_HANDLER_FAULTED,			/**< Temperature handler has encountered an error. */
};

/**
 * Get the average temperature including the current temperature sample.
 *
 * @param avg_temp_handler Pointer to the structure for the average temperature.
 * @param value Current temperature value in degree celsius.
 * @param avg_temperature Output variable to store the average temperature
 *
 */
static void telemetry_temperature_handler_calculate_average_temperature (
	const struct telemetry_temperature_handler *temperature_handler, int16_t value,
	int16_t *avg_temperature)
{
	if (temperature_handler->state->count <
		TELEMETRY_TEMPERATURE_HANDLER_MOVING_AVERAGE_WINDOW_SIZE) {
		temperature_handler->state->count++;
	}
	else {
		/* If the window is full, remove the oldest value pointed by the index. */
		temperature_handler->state->sum -=
			temperature_handler->state->values[temperature_handler->state->index];
	}
	temperature_handler->state->values[temperature_handler->state->index] = value;
	temperature_handler->state->sum += value;
	/* Wrap around the index */
	temperature_handler->state->index = (temperature_handler->state->index + 1) %
		TELEMETRY_TEMPERATURE_HANDLER_MOVING_AVERAGE_WINDOW_SIZE;
	*avg_temperature = temperature_handler->state->sum / temperature_handler->state->count;
}

/**
 * Check if the current avg temperature value is within the expected range.
 *
 * @param avg_temperature temperature value to check.
 *
 * @return true if temperature value is within expected range, else false.
 */
static bool telemetry_temperature_handler_is_within_range (
	const struct telemetry_temperature_handler *temperature_handler, int16_t temperature)
{
	return (temperature >= temperature_handler->threshold->lower_temperature_threshold) &&
		   (temperature <= temperature_handler->threshold->higher_temperature_threshold);
}

/**
 * Prepare the next valid execution by initializing the timeout.
 *
 * @param handler Handler to the telemetry temperature object
 *
 */
static void telemetry_temperature_handler_prepare_internal (
	const struct telemetry_temperature_handler *temperature_handler)
{
	int status;

	status = platform_init_timeout (temperature_handler->period, &temperature_handler->state->next);
	if (status == 0) {
		temperature_handler->state->next_valid = true;
	}
	else {
		temperature_handler->state->next_valid = false;
	}
}

void telemetry_temperature_handler_prepare (const struct periodic_task_handler *handler)
{
	const struct telemetry_temperature_handler *temperature_handler =
		(const struct telemetry_temperature_handler*) handler;
	int16_t temperature;

	/* Read the temperature once and ignore it, as the first temperature reading returned is incorrect. */
	temperature_handler->sensor->get_temp (temperature_handler->sensor, &temperature);

	telemetry_temperature_handler_prepare_internal (temperature_handler);
}

const platform_clock* telemetry_temperature_handler_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct telemetry_temperature_handler *temperature_handler =
		(const struct telemetry_temperature_handler*) handler;

	if (temperature_handler->state->next_valid) {
		return &temperature_handler->state->next;
	}
	else {
		/* If the next timeout is not valid, just indicate immediate execution. */
		return NULL;
	}
}

void telemetry_temperature_handler_execute (const struct periodic_task_handler *handler)
{
	const struct telemetry_temperature_handler *temperature_handler =
		(const struct telemetry_temperature_handler*) handler;

	int status = 0;
	int16_t temperature;
	int16_t avg_temperature;

	status = temperature_handler->sensor->get_temp (temperature_handler->sensor, &temperature);
	if (status != 0) {
		if (temperature_handler->state->current_state != TELEMETRY_TEMPERATURE_HANDLER_FAULTED) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_MANTICORE,
				MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_FAULTED, status, 0);
			temperature_handler->state->current_state = TELEMETRY_TEMPERATURE_HANDLER_FAULTED;
		}

		telemetry_temperature_handler_prepare_internal (temperature_handler);

		return;
	}

	telemetry_temperature_handler_calculate_average_temperature (temperature_handler, temperature,
		&avg_temperature);

	switch (temperature_handler->state->current_state) {
		case TELEMETRY_TEMPERATURE_HANDLER_INITIALIZED:
			if (telemetry_temperature_handler_is_within_range (temperature_handler,
				avg_temperature)) {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_RUNNING, avg_temperature, 0);
			}
			else {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_TEMPERATURE, avg_temperature, 0);
				temperature_handler->state->last_avg_temperature = avg_temperature;
			}
			break;

		case TELEMETRY_TEMPERATURE_HANDLER_RUNNING_NORMAL:
			if (!telemetry_temperature_handler_is_within_range (temperature_handler,
				avg_temperature)) {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_TEMPERATURE, avg_temperature, 0);
				temperature_handler->state->last_avg_temperature = avg_temperature;
			}
			break;

		case TELEMETRY_TEMPERATURE_HANDLER_RUNNING_ABNORMAL:
			if (telemetry_temperature_handler_is_within_range (temperature_handler,
				avg_temperature)) {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_RUNNING, avg_temperature, 0);
			}
			else {
				if (abs (avg_temperature - temperature_handler->state->last_avg_temperature) >
					temperature_handler->threshold->max_delta) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING,
						DEBUG_LOG_COMPONENT_MANTICORE,
						MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_TEMPERATURE, avg_temperature, 0);
					temperature_handler->state->last_avg_temperature = avg_temperature;
				}
			}
			break;

		case TELEMETRY_TEMPERATURE_HANDLER_FAULTED:
			if (telemetry_temperature_handler_is_within_range (temperature_handler,
				avg_temperature)) {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_NORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_RUNNING, avg_temperature, 0);
			}
			else {
				temperature_handler->state->current_state =
					TELEMETRY_TEMPERATURE_HANDLER_RUNNING_ABNORMAL;
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_TEMPERATURE, avg_temperature, 0);
				temperature_handler->state->last_avg_temperature = avg_temperature;
			}
			break;
	}

	telemetry_temperature_handler_prepare_internal (temperature_handler);
}

/**
 * Initialize a handler for telemetry temperature monitor.
 *
 * @param handler The telemetry temperature handler to initialize.
 * @param state Variable context for the handler. This must be uninitialized.
 * @param sensor Temperature Sensor to be used with this handler
 * @param period_ms The amount of time between telemetry temperature monitoring requests, in milliseconds.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int telemetry_temperature_handler_init (struct telemetry_temperature_handler *handler,
	struct telemetry_temperature_handler_state *state, const struct temperature_sensor *sensor,
	const struct telemetry_temperature_handler_thresholds *threshold, uint32_t period_ms)
{
	if (handler == NULL) {
		return TELEMETRY_TEMPERATURE_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler, 0, sizeof (*handler));

	handler->base.prepare = telemetry_temperature_handler_prepare;
	handler->base.get_next_execution = telemetry_temperature_handler_get_next_execution;
	handler->base.execute = telemetry_temperature_handler_execute;

	handler->sensor = sensor;
	handler->state = state;
	handler->threshold = threshold;
	handler->period = period_ms;

	return telemetry_temperature_handler_init_state (handler);
}

/**
 * Initialize only the variable state for a telemetry temperature handler.  The rest of the handler
 * is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param handler The telemetry temperature handler that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int telemetry_temperature_handler_init_state (const struct telemetry_temperature_handler *handler)
{
	if ((handler == NULL) || (handler->state == NULL) || (handler->sensor == NULL) ||
		(handler->threshold == NULL)) {
		return TELEMETRY_TEMPERATURE_HANDLER_INVALID_ARGUMENT;
	}

	memset (handler->state, 0, sizeof (struct telemetry_temperature_handler_state));

	return 0;
}

/**
 * Release the resources used by a telemetry temperature handler.
 *
 * @param handler The handler to release.
 */
void telemetry_temperature_handler_release (const struct telemetry_temperature_handler *handler)
{
	UNUSED (handler);
}
