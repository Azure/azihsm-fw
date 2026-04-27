// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_BOARD_ID_H_
#define OVERLAKE_BOARD_ID_H_


/**
 * Board identifiers for Overlake.
 */
enum overlake_board_id {
	OVERLAKE_BOARD_ID_PIONEER_PEAK = 0,			/**< Board ID for Pioneer Peak POC. */
	OVERLAKE_BOARD_ID_BONANZA_PEAK = 0x10,		/**< Board ID for Bonanza Peak POC. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_POC = 0x20,	/**< Board ID for Glacier Peak POC. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_EV1 = 0x21,	/**< Board ID for Glacier Peak EV1. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_EV2 = 0x22,	/**< Board ID for Glacier Peak EV2. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_DV1 = 0x23,	/**< Board ID for Glacier Peak DV1. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_DV2 = 0x24,	/**< Board ID for Glacier Peak DV2. */
	OVERLAKE_BOARD_ID_GLACIER_PEAK_PV = 0x25,	/**< Board ID for Glacier Peak PV. */
	OVERLAKE_BOARD_ID_CASTLE_PEAK_EV0 = 0x30,	/**< TEMPORARY Board ID for Castle Peak EV0. */
	OVERLAKE_BOARD_ID_CELESTIAL_PEAK = 0xFF,	/**< Board ID for Celestial Peak */
};


/**
 * Identifiers for different types of Overlake board.
 */
enum overlake_board_type {
	OVERLAKE_PIONEER_PEAK = 0,		/**< A Pioneer Peak board. */
	OVERLAKE_BONANZA_PEAK = 0x10,	/**< A Bonanza Peak board. */
	OVERLAKE_GLACIER_PEAK = 0x20,	/**< A Glacier Peak board. */
	OVERLAKE_CASTLE_PEAK = 0x30,	/**< TEMPORARY ID for Castle Peak board. */
	OVERLAKE_CELESTIAL_PEAK = 0x70,	/**< A Celestial Peak board. */
};

/**
 * Get the Overlake board type for a specific board ID.
 */
#define	overlake_get_board_type(id)		(enum overlake_board_type) (((uint8_t) id) & 0x70)


#endif	/* OVERLAKE_BOARD_ID_H_ */
