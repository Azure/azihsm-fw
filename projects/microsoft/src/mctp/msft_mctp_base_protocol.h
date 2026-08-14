// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_MCTP_BASE_PROTOCOL_H_
#define MSFT_MCTP_BASE_PROTOCOL_H_


enum {
	MCTP_BASE_PROTOCOL_TIP_PA_ROT_EID = 0x0C,				/**< TIP PA-ROT EID */
	MCTP_BASE_PROTOCOL_OVERLAKE_SOC_EID = 0x0D,				/**< Overlake card EID */
	MCTP_BASE_PROTOCOL_OVERLAKE_AC_ROT_EID = 0x0E,			/**< Overlake slave RoT EID */
	MCTP_BASE_PROTOCOL_MANTICORE_AC_ROT_EID = 0x40,			/**< Manticore multi-master RoT EID */
	MCTP_BASE_PROTOCOL_CASTLE_PEAK_OMC_EID = 0x41,			/**< Castlepeak OMC EID */
	MCTP_BASE_PROTOCOL_CASTLE_PEAK_AC_CMC_ROT_EID = 0x42,	/**< Castlepeak CMC EID */
	MCTP_BASE_PROTOCOL_BOSTON_PEAK_OMC_1_EID = 0x43,		/**< Boston Peak OMC instance 1 EID */
	MCTP_BASE_PROTOCOL_CASTLE_PEAK_AC_CMC_1_ROT_EID = 0x44,	/**< Castlepeak CMC EID for 2nd MCTP instance */
};


#endif	/* MSFT_MCTP_BASE_PROTOCOL_H_ */
