// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TEMPERATURE_SENSOR_TSEN_H_
#define TEMPERATURE_SENSOR_TSEN_H_

#include <stdint.h>
#include "platform_api.h"
#include "drivers/hsp_dmb.h"
#include "system/temperature_sensor.h"


/**
 * The base address of the TSEN in Manticore.
 */
#define TSEN_ADDR			0xB0008000


/**
 * Identifiers for the different areas of the die that can be monitored by the temperature sensor.
 */
enum temperature_sensor_tsen_area_type {
	TEMPERATURE_SENSOR_TSEN_AREA_PCIE0 = 1,			/**< Temperature area near PCIE0 (PHY controller area). */
	TEMPERATURE_SENSOR_TSEN_AREA_PCIE1 = 2,			/**< Temperature area near PCIE1 (MAC IDE area). */
	TEMPERATURE_SENSOR_TSEN_AREA_UPKA0 = 3,			/**< Temperature area near UPKA0 (engine 0 area). */
	TEMPERATURE_SENSOR_TSEN_AREA_UPKA1 = 4,			/**< Temperature area near UPKA1 (engine 1 area). */
	TEMPERATURE_SENSOR_TSEN_AREA_NQM = 5,			/**< Temperature area near NVMe Queue Manager (M7 area). */
	TEMPERATURE_SENSOR_TSEN_AREA_BCP = 6,			/**< Temperature area near Bulk Crypto Processing (CDMA, EDU, and XDU area). */
	TEMPERATURE_SENSOR_TSEN_AREA_MIDDLE_DIE = 7,	/**< Temperature area middle-die. */
	TEMPERATURE_SENSOR_TSEN_AREA_NUM_AREAS = 8,		/**< Count of TSEN area types. */
};

/**
 * Variable context for the TSEN driver.
 */
struct temperature_sensor_tsen_state {
	platform_mutex lock;	/**< Lock for synchronization. */
};

/**
 * Temperature sensor API implementation tha provides a reading for a single area of the die.
 */
struct temperature_sensor_tsen_area {
	struct temperature_sensor base;					/**< Base API for temperature sensor access. */
	enum temperature_sensor_tsen_area_type area;	/**< The area to access for temperature reading. */
};

/**
 * Driver for accessing the TSEN hardware.
 */
struct temperature_sensor_tsen {
	struct temperature_sensor_tsen_state *state;	/**< Variable context for the driver. */
	const struct hsp_dmb *dmb;						/**< DMB for SoC address translation from SP. */
	struct temperature_sensor_tsen_area pcie0;		/**< Temperature reading for the PHY controller. */
	struct temperature_sensor_tsen_area pcie1;		/**< Temperature reading for the MAC IDE. */
	struct temperature_sensor_tsen_area upka0;		/**< Temperature reading for the first set of UPKA blocks. */
	struct temperature_sensor_tsen_area upka1;		/**< Temperature reading for the second set of UPKA blocks. */
	struct temperature_sensor_tsen_area nqm;		/**< Temperature reading for the NVMe Queue Manager. */
	struct temperature_sensor_tsen_area bcp;		/**< Temperature reading for the bulk crypto processing. */
	struct temperature_sensor_tsen_area middle_die;	/**< Temperature reading in the middle of the die. */
};


int temperature_sensor_tsen_init (struct temperature_sensor_tsen *tsen,
	struct temperature_sensor_tsen_state *state, const struct hsp_dmb *dmb);
int temperature_sensor_tsen_init_state (const struct temperature_sensor_tsen *tsen);
void temperature_sensor_tsen_release (const struct temperature_sensor_tsen *tsen);


#endif	/* TEMPERATURE_SENSOR_TSEN_H_ */
