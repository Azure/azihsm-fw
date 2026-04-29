// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "msft_base_commands.h"
#include "temperature_sensor_cluster.h"
#include "common/buffer_util.h"
#include "common/unused.h"


/**
 * Initialize a manager for a set of multiple temperature sensors.
 *
 * @param cluster The temperature sensor cluster instance.
 * @param sensors A list of temperature sensors that should be managed.
 * @param sensor_count The number of sensors in the list.
 *
 * @param 0 if successful, else an error code.
 */
int temperature_sensor_cluster_init (struct temperature_sensor_cluster *cluster,
	const struct temperature_sensor *const *sensors, size_t sensor_count)
{
	if ((cluster == NULL) || (sensors == NULL) || (sensor_count == 0)) {
		return TEMP_SENSOR_CLUSTER_INVALID_ARGUMENT;
	}

	memset (cluster, 0, sizeof (*cluster));

	cluster->sensors = sensors;
	cluster->sensor_count = sensor_count;

	return 0;
}

/**
 * Releases any resources held by the sensor cluster instance.
 *
 * @param cluster The sensor cluster instance.
 */
void temperature_sensor_cluster_release (const struct temperature_sensor_cluster *cluster)
{
	UNUSED (cluster);
}

/**
 * Gets the number of temperature sensors that this instance manages.
 *
 * @param cluster The sensor cluster instance.
 *
 * @return The number of temperature sensors or 0 if cluster is NULL.
 */
size_t temperature_sensor_cluster_get_sensor_count (
	const struct temperature_sensor_cluster *cluster)
{
	if (cluster == NULL) {
		return 0;
	}

	return cluster->sensor_count;
}

/**
 * Gets a temperature sensor from an input list index.
 *
 * @param cluster The sensor cluster instance.
 * @param index The index in the sensor entry list.
 * @param sensor The output pointer to return the temperature sensor.
 *
 * @return 0 if successful, else an error code.
 */
int temperature_sensor_cluster_get_sensor (const struct temperature_sensor_cluster *cluster,
	size_t index, const struct temperature_sensor **sensor)
{
	if ((cluster == NULL) || (sensor == NULL)) {
		return TEMP_SENSOR_CLUSTER_INVALID_ARGUMENT;
	}

	if (index >= cluster->sensor_count) {
		return TEMP_SENSOR_CLUSTER_OUT_OF_RANGE;
	}

	*sensor = cluster->sensors[index];

	return 0;
}

/**
 * Obtains temperature readings from a range of sensors in the cluster.
 *
 * @param cluster The sensor cluster instance.
 * @param index The index in the sensor cluster to begin obtaining readings.
 * @param temps The output array to store temperature sensor readings.  This must have at least as
 * many elements as specified by 'count.'
 * @param count The count of sensors to obtain readings.
 *
 * @return The number of temperatures read, else an error code.
 */
int temperature_sensor_cluster_sensor_range_get_temps (
	const struct temperature_sensor_cluster *cluster, size_t index,
	struct msft_base_temperature_reading *temps, size_t count)
{
	const struct temperature_sensor *sensor;
	size_t remain;
	size_t out = 0;
	int16_t temp;
	int status;

	if ((cluster == NULL) || (temps == NULL)) {
		return TEMP_SENSOR_CLUSTER_INVALID_ARGUMENT;
	}

	if (index >= cluster->sensor_count) {
		return TEMP_SENSOR_CLUSTER_OUT_OF_RANGE;
	}

	remain = cluster->sensor_count - index;

	/* Limit the number of values returned if the request is for more sensors than are present in
	 * the list. */
	if (count > remain) {
		count = remain;
	}

	/* Limit the number of values returned if the request is for fewer than the maximum number of
	 * available sensors. */
	if (remain > count) {
		remain = count;
	}

	while (remain > 0) {
		temps[out].sensor_id = index;
		sensor = cluster->sensors[index++];

		status = sensor->get_temp (sensor, &temp);
		if (status != 0) {
			return status;
		}

		buffer_unaligned_write16 ((uint16_t*) &temps[out++].temperature, temp);
		--remain;
	}

	return (int) count;
}
