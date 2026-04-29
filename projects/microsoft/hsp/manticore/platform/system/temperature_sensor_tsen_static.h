// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TEMPERATURE_SENSOR_TSEN_STATIC_H_
#define TEMPERATURE_SENSOR_TSEN_STATIC_H_

#include "system/temperature_sensor_tsen.h"


/* Internal functions declared to allow for static initialization. */
int temperature_sensor_tsen_get_temp (const struct temperature_sensor *temp_sensor, int16_t *temp);


/**
 * Constant initializer for the TSEN base temperature sensor API.
 */
#define TEMPERATURE_SENSOR_TSEN_API_INIT { \
		.get_temp = temperature_sensor_tsen_get_temp \
	}

/**
 * Initialize the temperature sensor for a single area of the die.
 *
 * @param area_arg The area to be used for temperature_reading.
 */
#define	TEMPERATURE_SENSOR_TSEN_AREA_INIT(area_arg)	{ \
		.base = TEMPERATURE_SENSOR_TSEN_API_INIT, \
		.area = area_arg, \
	}


/**
 * Initialize a static TSEN driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the TSEN driver.
 * @param dmb_ptr Driver for the DMB to use for SoC address mapping.
 */
#define temperature_sensor_tsen_static_init(state_ptr, dmb_ptr) { \
		.state = state_ptr, \
		.dmb = dmb_ptr, \
		.pcie0 = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_PCIE0), \
		.pcie1 = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_PCIE1), \
		.upka0 = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_UPKA0), \
		.upka1 = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_UPKA1), \
		.nqm = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_NQM), \
		.bcp = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_BCP), \
		.middle_die = TEMPERATURE_SENSOR_TSEN_AREA_INIT (TEMPERATURE_SENSOR_TSEN_AREA_MIDDLE_DIE),\
	}


#endif	/* TEMPERATURE_SENSOR_TSEN_STATIC_H_ */
