// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TEMPERATURE_SENSOR_STATIC_H_
#define TEMPERATURE_SENSOR_STATIC_H_

#include "temperature_sensor.h"


/* Static initializer API for derived types. */

/**
 * Initializes the base API for a static instance of a temperature sensor.
 *
 * There is no validation done on the arguments.
 *
 * @param temp_func A function pointer to handle the temperature getter.  This cannot be NULL.
 */
#define temperature_sensor_static_init(temp_func) { \
		.get_temp = temp_func, \
	}


#endif	/* TEMPERATURE_SENSOR_STATIC_H_ */
