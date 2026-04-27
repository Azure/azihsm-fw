// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "platform_api.h"
#include "marvell/RegTsen.h"
#include "system/temperature_sensor_tsen.h"


/**
 * The offset parameter used to convert temperature reading to degrees Celsius.
 */
#define TSEN_OFFSET				128900

/**
 * The gain parameter used to convert temperature reading to degrees Celsius.
 */
#define TSEN_GAIN				394

/**
 * The divisor used on the temperature calculation result to convert to hundredths of a degree C.
 */
#define TSEN_DIVISOR			10

/**
 * The amount of time to wait for the temperature sensor to complete a sample before failing.
 */
#define	TSEN_WAIT_TIMEOUT_MS	20

/**
 * Convert TSEN reading to hundredths of a degree Celsius.
 */
#define TSEN_READING_TO_HUNDREDTHS_C(temp) \
	((TSEN_OFFSET - (TSEN_GAIN * (temp))) / TSEN_DIVISOR)

/**
 * Get the driver container for a single sensor instance.
 *
 * @param sensor Pointer to the temperature sensor instance.
 * @param area Identifier for the die area of the sensor.
 */
#define	TSEN_DRIVER_FOR_SENSOR(sensor, area)    \
	(const struct temperature_sensor_tsen*) (((uintptr_t) sensor) - \
		(offsetof (struct temperature_sensor_tsen, pcie0) + \
			(sizeof (*sensor) * (area - TEMPERATURE_SENSOR_TSEN_AREA_PCIE0))))


int temperature_sensor_tsen_get_temp (const struct temperature_sensor *temp_sensor, int16_t *temp)
{
	const struct temperature_sensor_tsen_area *sensor =
		(const struct temperature_sensor_tsen_area*) temp_sensor;
	const struct temperature_sensor_tsen *tsen;
	int output_raw;
	Tsen_t *tsen_regs;
	platform_clock timeout;
	int status;

	if ((sensor == NULL) || (temp == NULL)) {
		return TEMP_SENSOR_INVALID_ARGUMENT;
	}

	tsen = TSEN_DRIVER_FOR_SENSOR (sensor, sensor->area);

	platform_mutex_lock (&tsen->state->lock);

	status = tsen->dmb->map_soc_address (tsen->dmb, TSEN_ADDR, sizeof (Tsen_t),
		HSP_DMB_ACCESS_WRITE, (void**) &tsen_regs);
	if (status != 0) {
		goto exit;
	}

	status = platform_init_timeout (TSEN_WAIT_TIMEOUT_MS, &timeout);
	if (status != 0) {
		goto unmap;
	}

	// Write 2 to TSEN_ADC_MODE to select the on-die temperature sensor setting.
	tsen_regs->temperatureSensorControl.b.TSEN_ADC_MODE = 0x2;

	tsen_regs->temperatureSensorControl.b.TSEN_CH_SEL = sensor->area;
	tsen_regs->temperatureSensorControl.b.TSEN_ADC_RESET = 0x0;
	tsen_regs->temperatureSensorControl.b.TSEN_ADC_EN = 0x1;
	tsen_regs->temperatureSensorControl.b.TSEN_ADC_START = 0x1;

	// Wait for TSEN_SAMPLE_RDY_INT to be asserted.
	while (!platform_has_timeout_expired (&timeout) &&
		(tsen_regs->temperatureSensorIntrStatus.b.TSEN_SAMPLE_RDY_INT != 1)) {
	}

	if ((tsen_regs->temperatureSensorIntrStatus.b.TSEN_SAMPLE_RDY_INT != 1)) {
		status = TEMP_SENSOR_GET_TEMP_FAILED;
		goto stop;
	}

	// Read on-die temperature from TSEN_SAMPLE_DATA[9:0] and convert to Celsius.
	output_raw = (~(tsen_regs->temperatureSensorSampleStatus.b.TSEN_SAMPLE_DATA)) & 0x3ff;
	*temp = TSEN_READING_TO_HUNDREDTHS_C (output_raw);

	// Write to TSEN_SAMPLE_RDY_INT to clear.
	tsen_regs->temperatureSensorIntrStatus.b.TSEN_SAMPLE_RDY_INT = 0x1;

stop:
	// Stop the temperature sensor so it's idle until the next measurement request.
	tsen_regs->temperatureSensorControl.b.TSEN_ADC_START = 0x0;

unmap:
	tsen->dmb->unmap_soc_address (tsen->dmb, tsen_regs);

exit:
	platform_mutex_unlock (&tsen->state->lock);

	return status;
}

/**
 * Initialize a single temperature sensor managed by the TSEN hardware.
 *
 * @param sensor The sensor to initialize.
 * @param area Identifier for the area reported by the sensor.
 */
static void temperature_sensor_tsen_area_init (struct temperature_sensor_tsen_area *sensor,
	enum temperature_sensor_tsen_area_type area)
{
	sensor->base.get_temp = temperature_sensor_tsen_get_temp;

	sensor->area = area;
}

/**
 * Initialize a driver for reading die temperatures using TSEN.
 *
 * @param tsen The TSEN driver to initialize.
 * @param state The variable context for the TSEN driver.
 * @param dmb Driver for the DMB to use for SoC address mapping.
*/
int temperature_sensor_tsen_init (struct temperature_sensor_tsen *tsen,
	struct temperature_sensor_tsen_state *state, const struct hsp_dmb *dmb)
{
	if ((tsen == NULL) || (dmb == NULL)) {
		return TEMP_SENSOR_INVALID_ARGUMENT;
	}

	memset (tsen, 0, sizeof (struct temperature_sensor_tsen));

	tsen->state = state;
	tsen->dmb = dmb;

	temperature_sensor_tsen_area_init (&tsen->pcie0, TEMPERATURE_SENSOR_TSEN_AREA_PCIE0);
	temperature_sensor_tsen_area_init (&tsen->pcie1, TEMPERATURE_SENSOR_TSEN_AREA_PCIE1);
	temperature_sensor_tsen_area_init (&tsen->upka0, TEMPERATURE_SENSOR_TSEN_AREA_UPKA0);
	temperature_sensor_tsen_area_init (&tsen->upka1, TEMPERATURE_SENSOR_TSEN_AREA_UPKA1);
	temperature_sensor_tsen_area_init (&tsen->nqm, TEMPERATURE_SENSOR_TSEN_AREA_NQM);
	temperature_sensor_tsen_area_init (&tsen->bcp, TEMPERATURE_SENSOR_TSEN_AREA_BCP);
	temperature_sensor_tsen_area_init (&tsen->middle_die, TEMPERATURE_SENSOR_TSEN_AREA_MIDDLE_DIE);

	return temperature_sensor_tsen_init_state (tsen);
}

/**
 * Initialize only the variable state for a TSEN driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * @param tsen The TSEN driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
*/
int temperature_sensor_tsen_init_state (const struct temperature_sensor_tsen *tsen)
{
	if ((tsen == NULL) || (tsen->state == NULL) || (tsen->dmb == NULL)) {
		return TEMP_SENSOR_INVALID_ARGUMENT;
	}

	memset (tsen->state, 0, sizeof (struct temperature_sensor_tsen_state));

	return platform_mutex_init (&tsen->state->lock);
}

/**
 * Release the resources used by a TSEN driver instance.
 *
 * @param tsen The TSEN driver to release.
*/
void temperature_sensor_tsen_release (const struct temperature_sensor_tsen *tsen)
{
	if (tsen && tsen->state) {
		platform_mutex_free (&tsen->state->lock);
	}
}
