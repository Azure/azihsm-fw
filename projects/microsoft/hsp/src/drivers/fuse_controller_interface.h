// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_INTERFACE_H_
#define FUSE_CONTROLLER_INTERFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hsp_security_state.h"
#include "status/msft_module_id.h"


/**
 * Length of the RNG calibration data to apply to the hardware.
 */
#define	FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH		5

/**
 * Maximum Number of sw fuses supported by fuse controller
 */
#define FUSE_CONTROLLER_SW_FUSES_MAX_CNT			2

/**
 * Maximum Number of sw ecc fuses supported by fuse controller
 */
#define FUSE_CONTROLLER_SW_ECC_FUSES_MAX_CNT		7

/**
 * Maximum Number of rsvd fuses supported by fuse controller
 */
#define FUSE_CONTROLLER_RSVD_FUSES_MAX_CNT			4

/**
 * Below struct defines the fuse slot fields.
 */
struct fuse_controller_fuse_slot {
	uint16_t fuse_addr;	/**< fuse address for that slot */
	size_t fuse_length;	/**< number of fuse in bytes */
	size_t fuse_ecc;	/**< number of ecc in bytes */
};

/**
 * fuse map for the sw fuses
 */
struct fuse_controller_fuse_map {
	struct fuse_controller_fuse_slot sw[FUSE_CONTROLLER_SW_FUSES_MAX_CNT];			/**< sw fuses */
	struct fuse_controller_fuse_slot sw_ecc[FUSE_CONTROLLER_SW_ECC_FUSES_MAX_CNT];	/**< sw ecc fuses */
	struct fuse_controller_fuse_slot rsvd[FUSE_CONTROLLER_RSVD_FUSES_MAX_CNT];		/**< rsvd fuses */
};

/**
 * Driver to access HSP fuses.
 */
struct fuse_controller_interface {
	/**
	 * Determine the current security state of the HSP.
	 *
	 * @param fuses The fuse controller to query.
	 *
	 * @return The current security state.
	 */
	enum hsp_security_state (*get_security_state) (const struct fuse_controller_interface *fuses);

	/**
	 * Change the security state stored in HSP fuses.  The actual security state of the device will
	 * not change until it has been reset.
	 *
	 * @param fuses The fuse controller for security state management.
	 * @param state The security state to configure.
	 *
	 * @return 0 if the security state was changed successfully or an error code.
	 */
	int (*change_security_state) (const struct fuse_controller_interface *fuses,
		enum hsp_security_state state);

	/**
	 * Check the SOCID fuses to see if they have been programmed.
	 *
	 * @param fuses The fuse controller that will execute the check.
	 *
	 * @return 0 if the SOCID is blank or an error code.  If any part of the SOCID has been
	 * programmed, FUSE_CONTROLLER_NOT_BLANK will be returned.
	 */
	int (*blank_check_socid) (const struct fuse_controller_interface *fuses);

	/**
	 * Reads SOCID stored in GFC registers.
	 *
	 * @param fuses The fuse controller for the fuses.
	 * @param socid Output buffer for SOCID read.
	 * @param length Length of the SOCID buffer.
	 *
	 * @return Length of the returned SOCID or an error code.  Use ROT_IS_ERROR to check the return
	 * value.
	 */
	int (*read_registered_socid) (const struct fuse_controller_interface *fuses, uint8_t *socid,
		size_t length);

	/**
	 * Program the SOCID into fuses.
	 *
	 * @param fuses The fuse controller for SOCID fuses.
	 * @param socid The SOCID to program.
	 * @param length Length of the SOCID.
	 *
	 * @return 0 if the SOCID was successfully programmed or an error code.
	 */
	int (*program_socid) (const struct fuse_controller_interface *fuses, const uint8_t *socid,
		size_t length);

	/**
	 * Read a single 32-bit block of EMC fuse data.
	 *
	 * @param fuses The fuse controller for EMC fuses.
	 * @param address Fuse address to read.  This must be 32-bit aligned.
	 * @param value Output for the fuse value stored at the specified address.
	 *
	 * @return 0 if the fuses were read successfully or an error code.
	 */
	int (*read_emc_register) (const struct fuse_controller_interface *fuses, uint16_t address,
		uint32_t *value);

	/**
	 * Program a single 32-bit block of EMC fuse data.
	 *
	 * @param fuses The fuse controller for EMC fuses.
	 * @param address Fuse address to program.  This must be 32-bit aligned.
	 * @param value The value to store in the specified fuses.
	 *
	 * @return 0 if the fuses were programmed successfully or an error code.
	 */
	int (*program_emc_register) (const struct fuse_controller_interface *fuses, uint16_t address,
		uint32_t value);

	/**
	 * Read a single 32-bit block of AEB fuse data.
	 *
	 * @param fuses The fuse controller for AEB fuses.
	 * @param address Fuse address to read.  This must be 32-bit aligned.
	 * @param Output for the fuse value stored at the specified address.
	 *
	 * @return 0 if the fuses were read successfully or an error code.
	 */
	int (*read_aeb_register) (const struct fuse_controller_interface *fuses, uint16_t address,
		uint32_t *value);

	/**
	 * Program a single 32-bit block of AEB fuse data.
	 *
	 * @param fuses The fuse controller for AEB fuses.
	 * @param address Fuse address to program.  This must be 32-bit aligned.
	 * @param value The value to store in the specified fuses.
	 *
	 * @return 0 if the fuses were programmed successfully or an error code.
	 */
	int (*program_aeb_register) (const struct fuse_controller_interface *fuses, uint16_t address,
		uint32_t value);

	/**
	 * Read the RNG calibration data that has been programmed into fuses.
	 *
	 * @param fuses The fuse controller for RNG calibration.
	 * @param rng_data Output for the RNG calibration data.
	 *
	 * @return 0 if the data was read correctly or an error code.
	 */
	int (*read_rng_calibration) (const struct fuse_controller_interface *fuses,
		uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);

	/**
	 * Program the RNG calibration data into fuses.
	 *
	 * @param fuses The fuse controller for RNG calibration.
	 * @param rng_data The calibration data to program into fuses.
	 *
	 * @return 0 if the fuses were programmed successfully or an error code.
	 */
	int (*program_rng_calibration) (const struct fuse_controller_interface *fuses,
		const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);

	/**
	 * Check a range of fuses to see if any are programmed.
	 *
	 * @param fuses The fuse controller that will execute the check.
	 * @param start_addr The first fuse address to blank check.  This must be 32-bit aligned.
	 * @param end_addr The last fuse address to blank check.  This must be 32-bit aligned.
	 * @param not_blank Output for the first fuse address that is not blank.  This will only be
	 * populated if the function returns FUSE_CONTROLLER_NOT_BLANK.  This can be null if the first
	 * programmed address is not needed.
	 *
	 * @return 0 if the entire fuse range is blank or an error code.  If some fuses in the range are
	 * not blank, FUSE_CONTROLLER_NOT_BLANK will be returned.
	 */
	int (*blank_check) (const struct fuse_controller_interface *fuses, uint16_t start_addr,
		uint16_t end_addr, uint16_t *not_blank);

	/**
	 * Check a fuse key slot to see if it has been programmed.
	 *
	 * @param fuses The fuse controller that will execute the check.
	 * @param key Key slot to check.  This must be a value from 0 to 3.
	 *
	 * @return 0 if the key slot is blank or an error code.  If any part of the key slot has been
	 * programmed, FUSE_CONTROLLER_NOT_BLANK will be returned.
	 */
	int (*blank_check_key) (const struct fuse_controller_interface *fuses, uint8_t key);

	/**
	 * Get the fuse map for the sw fuses.
	 *
	 * @param fuses The fuse controller that will maintain the fuse map
	 *
	 * @return the fuse map layout for the sw fuses.
	 */
	const struct fuse_controller_fuse_map* (*get_fuse_map) (
		const struct fuse_controller_interface *fuses);

	/**
	 * Read a block of fuse data from SW fuse slots.
	 *
	 * While this will read fuse data from registered slots and well as unregistered ones, it should
	 * generally be preferred to access the data registers for registered slots.
	 *
	 * Attempts to read fuse word addresses that contain ECC data will be rejected by the fuse HW,
	 * resulting in a failure.
	 *
	 * @param fuses The fuse controller for the SW fuses to read.
	 * @param start_addr The first fuse address to read from.  There are no alignment requirements
	 * on this address.
	 * @param data Output buffer for the fuse data that was read.
	 * @param length The amount of data to read.
	 *
	 * @return 0 if the fuse data was read successfully or an error code.
	 */
	int (*read_sw_fuses) (const struct fuse_controller_interface *fuses, uint16_t start_addr,
		uint8_t *data, size_t length);

	/**
	 * Reads a block of fuse data to a SW fuse slot.  The read request must not cross over fuse
	 * slot boundaries.
	 *
	 * Requests to read a fuse slot with ECC protection must read the entire slot.
	 *
	 * @param fuses The fuse controller for the SW fuses to read.
	 * @param start_addr The first fuse address to read from. This must be 32-bit aligned.
	 * @param data Output buffer for the fuse data that was read.
	 * @param length The number of fuse words to program.  Each fuse word is 32 bits.
	 *
	 * @return 0 if sw fuses was read successfully or an error code.
	 */
	int (*read_registered_sw_fuses) (const struct fuse_controller_interface *fuses,
		uint16_t start_addr, uint8_t *data, size_t length);

	/**
	 * Program a block of fuse data to a SW fuse slot.  The program request must not cross over fuse
	 * slot boundaries.
	 *
	 * Requests to program a fuse slot with ECC protection must program the entire slot.
	 *
	 * @param fuses The fuse controller for the SW fuses to program.
	 * @param start_addr The first fuse address to program.  This must be 32-bit aligned.
	 * @param data The data to commit to fuses.
	 * @param fuse_words The number of fuse words to program.  Each fuse word is 32 bits.
	 *
	 * @return 0 if the fuse data was programmed successfully or an error code.
	 */
	int (*program_sw_fuses) (const struct fuse_controller_interface *fuses, uint16_t start_addr,
		const uint32_t *data, size_t fuse_words);

	/**
	 * Write a block of fuse data to GFC registers.  The program request must not cross over fuse
	 * slot boundaries.
	 *
	 * Requests to program a GFC register must be word aligned. These values are reflected
	 * only in the GFC registers.
	 *
	 * @param fuses The fuse controller for the SW fuses to program.
	 * @param start_addr The first fuse address to program.  This must be 32-bit aligned.
	 * @param data The data to commit to fuses.
	 * @param fuse_words The number of fuse words to program.  Each fuse word is 32 bits.
	 *
	 * @return 0 if the fuse data was programmed successfully or an error code.
	 */
	int (*program_registered_sw_fuses) (const struct fuse_controller_interface *fuses,
		uint16_t start_addr, const uint32_t *data, size_t fuse_words);
};

#define	FUSE_CONTROLLER_ERROR(code)		ROT_ERROR (MSFT_MODULE_FUSE_CONTROLLER, code)

/**
 * Error codes that can be generated by a fuse controller.
 */
enum {
	FUSE_CONTROLLER_INVALID_ARGUMENT = FUSE_CONTROLLER_ERROR (0x00),				/**< Input parameter is null or not valid. */
	FUSE_CONTROLLER_NO_MEMORY = FUSE_CONTROLLER_ERROR (0x01),						/**< Memory allocation failed. */
	FUSE_CONTROLLER_CHANGE_SS_FAILED = FUSE_CONTROLLER_ERROR (0x02),				/**< Failed to change the security state. */
	FUSE_CONTROLLER_PROG_SOCID_FAILED = FUSE_CONTROLLER_ERROR (0x03),				/**< Failed to program the SOCID. */
	FUSE_CONTROLLER_READ_EMC_FAILED = FUSE_CONTROLLER_ERROR (0x04),					/**< Failed to read an EMC register. */
	FUSE_CONTROLLER_PROG_EMC_FAILED = FUSE_CONTROLLER_ERROR (0x05),					/**< Failed to program an EMC register. */
	FUSE_CONTROLLER_READ_RNG_FAILED = FUSE_CONTROLLER_ERROR (0x06),					/**< Failed to read RNG calibration data. */
	FUSE_CONTROLLER_PROG_RNG_FAILED = FUSE_CONTROLLER_ERROR (0x07),					/**< Failed to program RNG calibration data. */
	FUSE_CONTROLLER_BLANK_CHECK_FAILED = FUSE_CONTROLLER_ERROR (0x08),				/**< Failed to execute a fuse blank check. */
	FUSE_CONTROLLER_READ_SW_FAILED = FUSE_CONTROLLER_ERROR (0x09),					/**< Failed to read SW fuse data. */
	FUSE_CONTROLLER_PROG_SW_FAILED = FUSE_CONTROLLER_ERROR (0x0a),					/**< Failed to program SW fuses. */
	FUSE_CONTROLLER_NOT_BLANK = FUSE_CONTROLLER_ERROR (0x0b),						/**< Some fuses failed blank check. */
	FUSE_CONTROLLER_UNSUPPORTED = FUSE_CONTROLLER_ERROR (0x0c),						/**< The operation is not supported by the fuse implementation. */
	FUSE_CONTROLLER_ADDR_NOT_ALIGNED = FUSE_CONTROLLER_ERROR (0x0d),				/**< A fuse address was not properly aligned. */
	FUSE_CONTROLLER_ADDR_OUT_OF_RANGE = FUSE_CONTROLLER_ERROR (0x0e),				/**< A provided address is not valid for the operation. */
	FUSE_CONTROLLER_UNSUPPORTED_SS = FUSE_CONTROLLER_ERROR (0x0f),					/**< The fuse controller cannot change to the specified security state. */
	FUSE_CONTROLLER_SLOT_OVERFLOW = FUSE_CONTROLLER_ERROR (0x10),					/**< A program request overflows the fuse slot boundary. */
	FUSE_CONTROLLER_PARTIAL_ECC = FUSE_CONTROLLER_ERROR (0x11),						/**< A program request for an ECC protected slot does not fill the slot. */
	FUSE_CONTROLLER_LOAD_ERROR = FUSE_CONTROLLER_ERROR (0x12),						/**< The fuse hardware failed to load successfully. */
	FUSE_CONTROLLER_READ_AEB_FAILED = FUSE_CONTROLLER_ERROR (0x13),					/**< Failed to read an AEB register. */
	FUSE_CONTROLLER_PROG_AEB_FAILED = FUSE_CONTROLLER_ERROR (0x14),					/**< Failed to program an AEB register. */
	FUSE_CONTROLLER_READ_REGISTERED_SOCID_FAILED = FUSE_CONTROLLER_ERROR (0x15),	/**< Failed to read socid. */
	FUSE_CONTROLLER_SOCID_BUFFER_TOO_SMALL = FUSE_CONTROLLER_ERROR (0x16),			/**< Buffer size too small */
	FUSE_CONTROLLER_READ_REGISTERED_SW_FAILED = FUSE_CONTROLLER_ERROR (0x17),		/**< Failed to read registered SW fuse data. */
	FUSE_CONTROLLER_PROGRAM_REGISTERED_SW_FAILED = FUSE_CONTROLLER_ERROR (0x18),	/**< Failed to program registered SW fuse data. */
	FUSE_CONTROLLER_FUSES_NOT_REGISTERED = FUSE_CONTROLLER_ERROR (0x19),			/**< Some fuses are not registered. */

	/* Error codes >0x80 are reserved for reporting bits from the HW status register. */
};

/**
 * An error has occurred with the hardware block.  The error code represents the status register
 * output.
 *
 * This will mask out the lower two bits, which are busy and success.  It also masks out the blank
 * check fail status (bit 7), since that is only valid in the successful case.  The resulting error
 * code bits [0:4] map to fuse command status bits [2:6] and error code bits [5:6] are fuse status
 * bits [8:9].
 */
#define	FUSE_CONTROLLER_HW_ERROR(reg)   \
	FUSE_CONTROLLER_ERROR (0x80 | ((reg & 0x7c) >> 2) | ((reg & 0x300) >> 3))

/**
 * Determine if a fuse controller error code represents a hardware error.
 */
#define	FUSE_CONTROLLER_IS_HW_ERROR(code)	((((status & 0xff000000) == ROT_ERROR_MARKER)) && \
	(((code & 0x00ffff00) >> 8) == MSFT_MODULE_FUSE_CONTROLLER) && ((status & 0x00000080) == 0x80))


#endif	/* FUSE_CONTROLLER_INTERFACE_H_ */
