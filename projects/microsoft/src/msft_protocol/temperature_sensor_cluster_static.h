// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TEMPERATURE_SENSOR_CLUSTER_STATIC_H_
#define TEMPERATURE_SENSOR_CLUSTER_STATIC_H_

#include "temperature_sensor_cluster.h"


/**
 * Initializes a static instance of a manager for a set of individual temperature sensors.
 *
 * There is no validation done on the arguments.
 *
 * @param sensor_array An array of temperature sensors to associate with the controller.
 * @param sensor_arr_count The number of sensors in the array.
 */
#define temperature_sensor_cluster_static_init(sensor_array, sensor_arr_count) { \
		.sensors = sensor_array, \
		.sensor_count = sensor_arr_count, \
	}


#endif	/* TEMPERATURE_SENSOR_CLUSTER_STATIC_H_ */
