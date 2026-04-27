// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_HOST_ID_H_
#define OVERLAKE_HOST_ID_H_


/**
 * Overlake Host IDs
 */
enum overlake_host_id {
	OVERLAKE_HOST_SOC,	/**< Overlake host ID for SoC FIP. */
	OVERLAKE_HOST_FPGA,	/**< Overlake host ID Cyclone V. */
	OVERLAKE_NUM_HOSTS,	/**< Number of Overlake host. */
};

/**
 * Overlake Host Port IDs
 */
enum overlake_host_port_id {
	OVERLAKE_HOST_PORT_SOC_FIP,		/**< Overlake host port ID for SoC FIP. */
	OVERLAKE_HOST_PORT_SOC_NITRO,	/**< Overlake host port ID for SoC Nitro. */
	OVERLAKE_HOST_PORT_FPGA_C5,		/**< Overlake host port ID Cyclone V. */
	OVERLAKE_HOST_NUM_PORTS,		/**< Number of Overlake host ports. */
};


#endif	/* OVERLAKE_HOST_ID_H_ */
