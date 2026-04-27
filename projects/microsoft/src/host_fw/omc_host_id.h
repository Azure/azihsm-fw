// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OMC_HOST_ID_H_
#define OMC_HOST_ID_H_


/**
 * OMC Host IDs
 */
enum omc_host_id {
	OMC_HOST_SOC,	/* OMC host ID for SoC */
	OMC_NUM_HOSTS,	/* Number of OMC host */
};

/**
 * OMC Host Port IDs
 */
enum omc_host_port_id {
	OMC_HOST_PORT_SOC,	/* OMC host port ID for SoC */
	OMC_HOST_NUM_PORTS,	/* Number of OMC host ports */
};

/*
 * OMC reset port IDs
*/
#define	OMC_RESET_PORT_SOC				0x0	/* OMC reset port ID for SoC */
#define	OMC_RESET_PORT_HOLD_FLASH		0xD	/* OMC reset port ID for holding flash access */
#define	OMC_RESET_PORT_RELEASE_FLASH	0xE	/* OMC reset port ID for releasing flash control */
#define	OMC_RESET_PORT_TAKE_FLASH		0xF	/* OMC reset port ID for taking flash control */


#endif	/* OMC_HOST_ID_H_ */
