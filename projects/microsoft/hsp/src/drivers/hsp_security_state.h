// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_SECURITY_STATE_H_
#define HSP_SECURITY_STATE_H_


/**
 * Representation of the current security state of the HSP.
 */
enum hsp_security_state {
	HSP_SECURITY_STATE_UNKNOWN = 0x00,		/**< The security state could not be determined. */
	HSP_SECURITY_STATE_BLANK = 0x01,		/**< No security state has been set. */
	HSP_SECURITY_STATE_TEST = 0x02,			/**< The device is currently in Test state. */
	HSP_SECURITY_STATE_PRODUCTION = 0x04,	/**< The device is currently in Production state. */
	HSP_SECURITY_STATE_SECURE = 0x08,		/**< The device is currently in Secure state. */
	HSP_SECURITY_STATE_RETEST = 0x10,		/**< The device is currently in ReTest state. */
};

/**
 * Mask to read the security state value.
 */
#define	HSP_SECURITY_STATE_MASK			0x1f

/**
 * Get the current security state for the HSP.
 *
 * @param reg The register value containing the security state.
 */
#define	hsp_security_state_read(reg)	((enum hsp_security_state) (reg & HSP_SECURITY_STATE_MASK))


#endif	/* HSP_SECURITY_STATE_H_ */
