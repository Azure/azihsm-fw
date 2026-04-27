// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "drivers/fuse_controller.h"
#include "drivers/hsp_fuses.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spcryptotypes.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"


/**
 * Determine if the address is aligned for fuse access.
 */
#define	FUSE_CONTROLLER_IS_ADDR_ALIGNED(x)			(!((x) & 0x3u))

/**
 * Align a fuse address to a 32-bit boundary.
 */
#define	FUSE_CONTROLLER_ALIGN_ADDR(x)				((x) & ~0x3u)

/**
 * Definition for the static fuse slot.
 */
#define FUSE_CONTROLLER_FUSE_SLOT(addr, length, ecc) {  \
	.fuse_addr = addr,                                  \
	.fuse_length = length,                              \
	.fuse_ecc = ecc                                     \
}

/**
 * static initialization of the fuse map.
 */
static const struct fuse_controller_fuse_map fuse_map = {
	{
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW0),	HSP_FUSES_LENGTH (SW0),
			HSP_FUSES_ECC (SW0)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW1),	HSP_FUSES_LENGTH (SW1),
			HSP_FUSES_ECC (SW1)),
	},
	{
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW0_ECC),	HSP_FUSES_LENGTH (SW0_ECC),
			HSP_FUSES_ECC (SW0_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW1_ECC),	HSP_FUSES_LENGTH (SW1_ECC),
			HSP_FUSES_ECC (SW1_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW2_ECC),	HSP_FUSES_LENGTH (SW2_ECC),
			HSP_FUSES_ECC (SW2_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW3_ECC),	HSP_FUSES_LENGTH (SW3_ECC),
			HSP_FUSES_ECC (SW3_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW4_ECC),	HSP_FUSES_LENGTH (SW4_ECC),
			HSP_FUSES_ECC (SW4_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW5_ECC),	HSP_FUSES_LENGTH (SW5_ECC),
			HSP_FUSES_ECC (SW5_ECC)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (SW6_ECC),	HSP_FUSES_LENGTH (SW6_ECC),
			HSP_FUSES_ECC (SW6_ECC)),
	},
	{
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (RSVD0), HSP_FUSES_LENGTH (RSVD0),
			HSP_FUSES_ECC (RSVD0)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (RSVD1), HSP_FUSES_LENGTH (RSVD1),
			HSP_FUSES_ECC (RSVD1)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (RSVD2), HSP_FUSES_LENGTH (RSVD2),
			HSP_FUSES_ECC (RSVD2)),
		FUSE_CONTROLLER_FUSE_SLOT (HSP_FUSES_ADDRESS (RSVD3), HSP_FUSES_LENGTH (RSVD3),
			HSP_FUSES_ECC (RSVD3)),
	},
};


/**
 * Send a command to the fuse controller.
 *
 * @param fuses The fuse controller executing the command.
 * @param command Command code to execute.
 * @param address The address of the first fuse word for the command, if necessary.  This must be
 * 32-bit aligned, but will not be verified in this function.
 * @param data Data to program to the fuse word, if necessary.
 * @param words The number of fuse words to program.  In most cases this can only be 1, but some
 * fuses, like SOCID, are not word addressable and need to be programmed as a block.
 *
 * @return 0 if the fuse operation was completed successfully, FUSE_CONTROLLER_NOT_BLANK if the
 * blank check fail has been set, or a negative error code representing the error bits from the fuse
 * controller status register.
 */
int fuse_controller_execute_command (const struct fuse_controller *fuses,
	enum fuse_controller_cmd command, uint16_t address, const uint32_t *data, size_t words)
{
	size_t i;
	uint32_t status;

	platform_mutex_lock (&fuses->state->lock);

	/* Wait for the fuse controller to be available. */
	while (fuses->regs->Command_Status & GFC_REGS_COMMAND_STATUS_BUSY_FIELD_MASK) {
	}

	fuses->regs->Address = address;
	for (i = 0; i < words; i++) {
		fuses->regs->Data.Data[i] = data[i];
	}

	DMB;
	fuses->regs->Command = command;
	DMB;

	/* Wait for the command to complete. */
	do {
		status = fuses->regs->Command_Status;
	} while (status & GFC_REGS_COMMAND_STATUS_BUSY_FIELD_MASK);

	platform_mutex_unlock (&fuses->state->lock);

	if (status & GFC_REGS_COMMAND_STATUS_CMD_SUCCESS_FIELD_MASK) {
		if (status & GFC_REGS_COMMAND_STATUS_BLANK_CHECK_FAIL_FIELD_MASK) {
			return FUSE_CONTROLLER_NOT_BLANK;
		}

		return 0;
	}
	else {
		return FUSE_CONTROLLER_HW_ERROR (status);
	}
}

/**
 * Send a command to the fuse controller to read data from fuses.
 *
 * @param fuses The fuse controller executing the command.
 * @param address The address of the fuse data to read.  This must be 32-bit aligned, but will not
 * be verified in this function.
 * @param data Output for the fuse data.
 *
 * @return 0 if the fuse operation was completed successfully or a negative error code representing
 * the error bits from the fuse controller status register.
 */
static int fuse_controller_read_data (const struct fuse_controller *fuses, uint16_t address,
	uint32_t *data)
{
	int status;

	status = fuse_controller_execute_command (fuses, FUSE_CONTROLLER_CMD_READ_DATA, address, NULL,
		0);
	if (status == 0) {
		*data = fuses->regs->Data.Data[0];
	}

	return status;
}

/**
 * Validate that a provided address falls within the specified fuse slot and is 32-bit aligned.
 *
 * @param address Fuse address to validate.
 * @param first First address in the fuse slot.
 * @param last Last address in the fuse slot.
 *
 * @return 0 if the address is valid or an error code.
 */
static int fuse_controller_validate_register_address (uint16_t address, uint16_t first,
	uint16_t last)
{
	if ((address < first) || (address > last)) {
		return FUSE_CONTROLLER_ADDR_OUT_OF_RANGE;
	}

	if (!FUSE_CONTROLLER_IS_ADDR_ALIGNED (address)) {
		return FUSE_CONTROLLER_ADDR_NOT_ALIGNED;
	}

	return 0;
}

/**
 * Read a single 32-bit fuse word.  The destination address must fall within the specified range and
 * be 32-bit aligned.
 *
 * @param fuses The fuse controller for the fuses to read.
 * @param address Address of the fuse word to read.
 * @param first First valid address in the fuse slot.
 * @param last Last valid address in the fuse slot.
 * @param value Output for the register value.
 *
 * @return 0 if the register was read successfully or an error code.
 */
static int fuse_controller_read_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint16_t first, uint16_t last, uint32_t *value)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	int status;

	if ((fuses_hw == NULL) || (value == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuse_controller_validate_register_address (address, first, last);
	if (status != 0) {
		return status;
	}

	return fuse_controller_read_data (fuses_hw, address, value);
}

/**
 * Program a single 32-bit fuse word.  The destination address must fall within the specified range
 * and be 32-bit aligned.
 *
 * @param fuses The fuse controller for the fuses to program.
 * @param address Address of the fuse word to read.
 * @param first First valid address in the fuse slot.
 * @param last Last valid address in the fuse slot.
 * @param value The value to program in fuses.
 *
 * @return 0 if the register was successfully programmed or an error code.
 */
static int fuse_controller_program_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint16_t first, uint16_t last, uint32_t value)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	int status;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuse_controller_validate_register_address (address, first, last);
	if (status != 0) {
		return status;
	}

	return fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_PROGRAM_DATA, address,
		&value, 1);
}

enum hsp_security_state fuse_controller_get_security_state (
	const struct fuse_controller_interface *fuses)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;

	if (fuses_hw == NULL) {
		return HSP_SECURITY_STATE_UNKNOWN;
	}

	return hsp_security_state_read (fuses_hw->regs->One_Hot_SS);
}

int fuse_controller_change_security_state (const struct fuse_controller_interface *fuses,
	enum hsp_security_state state)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	enum fuse_controller_cmd command;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	switch (state) {
		case HSP_SECURITY_STATE_TEST:
			command = FUSE_CONTROLLER_CMD_CHANGE_TO_TEST;
			break;

		case HSP_SECURITY_STATE_PRODUCTION:
			command = FUSE_CONTROLLER_CMD_CHANGE_TO_PRODUCTION;
			break;

		case HSP_SECURITY_STATE_SECURE:
			command = FUSE_CONTROLLER_CMD_CHANGE_TO_SECURE;
			break;

		case HSP_SECURITY_STATE_RETEST:
			command = FUSE_CONTROLLER_CMD_CHANGE_TO_RETEST;
			break;

		default:
			return FUSE_CONTROLLER_UNSUPPORTED_SS;
	}

	return fuse_controller_execute_command (fuses_hw, command, 0, NULL, 0);
}

int fuse_controller_read_registered_socid (const struct fuse_controller_interface *fuses,
	uint8_t *buffer, size_t length)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	uint32_t dword_buffer[IN_DWORDS (HSP_FUSES_LENGTH (SOCID))];
	size_t i;

	if ((fuses_hw == NULL) || (buffer == NULL)) {
		return FUSE_CONTROLLER_READ_REGISTERED_SOCID_FAILED;
	}

	if (HSP_FUSES_LENGTH (SOCID) > length) {
		return FUSE_CONTROLLER_SOCID_BUFFER_TOO_SMALL;
	}

	for (i = 0; i < IN_DWORDS (HSP_FUSES_LENGTH (SOCID)); i++) {
		/* Need to copy to an intermediate buffer to ensure memory alignment and dword accesses. */
		dword_buffer[i] = fuses_hw->regs->SOCID.SOCID[i];
	}

	memcpy (buffer, (uint8_t*) dword_buffer, HSP_FUSES_LENGTH (SOCID));

	return HSP_FUSES_LENGTH (SOCID);
}

int fuse_controller_program_socid (const struct fuse_controller_interface *fuses, const
	uint8_t *socid, size_t length)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;

	if ((fuses_hw == NULL) || (socid == NULL) || (length != HSP_FUSES_LENGTH (SOCID))) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_PROGRAM_DATA,
		HSP_FUSES_ADDRESS (SOCID), (uint32_t*) socid, length / sizeof (uint32_t));
}

int fuse_controller_read_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value)
{
#if HSP_FUSES_LENGTH (EMC)

	return fuse_controller_read_register (fuses, address, HSP_FUSES_ADDRESS (EMC),
		HSP_FUSES_LAST_ADDRESS (EMC), value);
#else
	UNUSED (fuses);
	UNUSED (address);
	UNUSED (value);

	return FUSE_CONTROLLER_UNSUPPORTED;
#endif
}

int fuse_controller_program_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value)
{
#if HSP_FUSES_LENGTH (EMC)

	return fuse_controller_program_register (fuses, address, HSP_FUSES_ADDRESS (EMC),
		HSP_FUSES_LAST_ADDRESS (EMC), value);
#else
	UNUSED (fuses);
	UNUSED (address);
	UNUSED (value);

	return FUSE_CONTROLLER_UNSUPPORTED;
#endif
}

int fuse_controller_read_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value)
{
#if HSP_FUSES_LENGTH (AEB)

	return fuse_controller_read_register (fuses, address, HSP_FUSES_ADDRESS (AEB),
		HSP_FUSES_LAST_ADDRESS (AEB), value);
#else
	UNUSED (fuses);
	UNUSED (address);
	UNUSED (value);

	return FUSE_CONTROLLER_UNSUPPORTED;
#endif
}

int fuse_controller_program_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value)
{
#if HSP_FUSES_LENGTH (AEB)

	return fuse_controller_program_register (fuses, address, HSP_FUSES_ADDRESS (AEB),
		HSP_FUSES_LAST_ADDRESS (AEB), value);
#else
	UNUSED (fuses);
	UNUSED (address);
	UNUSED (value);

	return FUSE_CONTROLLER_UNSUPPORTED;
#endif
}

int fuse_controller_blank_check (const struct fuse_controller_interface *fuses, uint16_t start_addr,
	uint16_t end_addr, uint16_t *not_blank)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	int status;

	if (fuses_hw == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	if ((start_addr > HSP_FUSES_MAX_ADDRESS) || (end_addr > HSP_FUSES_MAX_ADDRESS) ||
		(start_addr > end_addr)) {
		return FUSE_CONTROLLER_ADDR_OUT_OF_RANGE;
	}

	if (!FUSE_CONTROLLER_IS_ADDR_ALIGNED (start_addr) ||
		!FUSE_CONTROLLER_IS_ADDR_ALIGNED (end_addr)) {
		return FUSE_CONTROLLER_ADDR_NOT_ALIGNED;
	}

	for (; start_addr <= end_addr; start_addr += sizeof (uint32_t)) {
		status = fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_BLANK_CHECK,
			start_addr, NULL, 0);
		if (status != 0) {
			if (not_blank != NULL) {
				*not_blank = start_addr;
			}

			return status;
		}
	}

	return 0;
}

int fuse_controller_blank_check_socid (const struct fuse_controller_interface *fuses)
{
	return fuse_controller_blank_check (fuses, HSP_FUSES_ADDRESS (SOCID),
		HSP_FUSES_LAST_WORD_ADDRESS (SOCID), NULL);
}

int fuse_controller_blank_check_key (const struct fuse_controller_interface *fuses, uint8_t key)
{
	switch (key) {
#if HSP_FUSES_LENGTH (KEY0)
		case 0:
			return fuse_controller_blank_check (fuses, HSP_FUSES_ADDRESS (KEY0),
				HSP_FUSES_LAST_WORD_ADDRESS (KEY0), NULL);
#endif

#if HSP_FUSES_LENGTH (KEY1)
		case 1:
			return fuse_controller_blank_check (fuses, HSP_FUSES_ADDRESS (KEY1),
				HSP_FUSES_LAST_WORD_ADDRESS (KEY1), NULL);
#endif

#if HSP_FUSES_LENGTH (KEY2)
		case 2:
			return fuse_controller_blank_check (fuses, HSP_FUSES_ADDRESS (KEY2),
				HSP_FUSES_LAST_WORD_ADDRESS (KEY2), NULL);
#endif

#if HSP_FUSES_LENGTH (KEY3)
		case 3:
			return fuse_controller_blank_check (fuses, HSP_FUSES_ADDRESS (KEY3),
				HSP_FUSES_LAST_WORD_ADDRESS (KEY3), NULL);
#endif

		default:
			return FUSE_CONTROLLER_UNSUPPORTED;
	}
}

const struct fuse_controller_fuse_map* fuse_controller_get_fuse_map (
	const struct fuse_controller_interface *fuses)
{
	UNUSED (fuses);

	return &fuse_map;
}

int fuse_controller_read_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint8_t *data, size_t length)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;

	union {
		uint32_t word;
		uint8_t bytes[4];
	} temp;
	int status = 0;

	if ((fuses_hw == NULL) || (data == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	if (((start_addr + length) > (HSP_FUSES_MAX_ADDRESS + 1)) ||
		(start_addr < HSP_FUSES_ADDRESS (SW0))) {
		return FUSE_CONTROLLER_ADDR_OUT_OF_RANGE;
	}

	while ((status == 0) && (length > 0)) {
		status = fuse_controller_read_data (fuses_hw, FUSE_CONTROLLER_ALIGN_ADDR (start_addr),
			&temp.word);
		if (status == 0) {
			/* The first read might not be aligned, so we need to account for that in how we copy
			 * the data from the fuse word.  The last read may be aligned, but might not read the
			 * entire word. */
			uint8_t len = min (length, (size_t) (4 - (start_addr & 0x3)));

			memcpy (data, &temp.bytes[start_addr & 0x3], len);

			data += len;
			length -= len;
			start_addr += len;
		}
	}

	return status;
}

/**
 * Check if a program request correctly targets a specified fuse slot.
 *
 * @param start_addr First address requested for programming.
 * @param length The number of bytes that will be programmed, without ECC words.
 * @param slot_start First address for the fuse slot to check.
 * @param slot_bytes Number of data bytes supported by the fuse slot.
 * @param slot_ecc Number of ECC bytes supported by the fuse slot.
 * @param slot_offset Output for the word offset within the slot that will used for programming.
 *
 * @return 0 if the program request falls within the specified fuse slot or an error code.  If the
 * request is completely outside this slot, -1 will be returned.
 */
static int fuse_controller_check_slot (uint16_t start_addr, size_t length, uint32_t slot_start,
	uint32_t slot_bytes, uint32_t slot_ecc, size_t *slot_offset)
{
	uint32_t end_address = slot_start + slot_bytes + slot_ecc;

	if ((start_addr >= slot_start) && (start_addr < end_address)) {
		if ((start_addr + length + slot_ecc) > end_address) {
			return FUSE_CONTROLLER_SLOT_OVERFLOW;
		}

		if (slot_ecc && ((start_addr != slot_start) || (length < slot_bytes))) {
			return FUSE_CONTROLLER_PARTIAL_ECC;
		}

		*slot_offset = (start_addr - slot_start) / sizeof (uint32_t);

		return 0;
	}

	return -1;
}

/**
 * Validate that a program request falls within a single fuse slot.  Also determine other properties
 * of the slot being programmed.
 *    - ECC protected
 *    - Register cache
 *
 * @param fuses The fuse controller executing the request.
 * @param start_addr First address requested for programming.
 * @param fuse_words The number of full fuse words that will be programmed.
 * @param slot_regs Output for the register containing the current fuse values.
 * @param ecc Output indicating if the slot is ECC protected.
 *
 * @return 0 if the request is valid or an error code.
 */
static int fuse_controller_validate_sw_program_request (const struct fuse_controller *fuses,
	uint16_t start_addr, size_t fuse_words, volatile uint32_t **slot_regs, bool *ecc)
{
	size_t bytes = fuse_words * sizeof (uint32_t);
	size_t word_offset = 0;
	int status;

	// In case only RSVDn fuses are supported
	UNUSED (fuses);

#if HSP_FUSES_LENGTH (SW0)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW0),
		HSP_FUSES_LENGTH (SW0), HSP_FUSES_ECC (SW0), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW0_fuse.SW0_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW0) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW1)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW1),
		HSP_FUSES_LENGTH (SW1), HSP_FUSES_ECC (SW1), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW1_fuse.SW1_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW1) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW0_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW0_ECC),
		HSP_FUSES_LENGTH (SW0_ECC), HSP_FUSES_ECC (SW0_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW0_ecc_fuse.SW0_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW0_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW1_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW1_ECC),
		HSP_FUSES_LENGTH (SW1_ECC), HSP_FUSES_ECC (SW1_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW1_ecc_fuse.SW1_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW1_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW2_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW2_ECC),
		HSP_FUSES_LENGTH (SW2_ECC), HSP_FUSES_ECC (SW2_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW2_ecc_fuse.SW2_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW2_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW3_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW3_ECC),
		HSP_FUSES_LENGTH (SW3_ECC), HSP_FUSES_ECC (SW3_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW3_ecc_fuse.SW3_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW3_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW4_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW4_ECC),
		HSP_FUSES_LENGTH (SW4_ECC), HSP_FUSES_ECC (SW4_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW4_ecc_fuse.SW4_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW4_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW5_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW5_ECC),
		HSP_FUSES_LENGTH (SW5_ECC), HSP_FUSES_ECC (SW5_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW5_ecc_fuse.SW5_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW5_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (SW6_ECC)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (SW6_ECC),
		HSP_FUSES_LENGTH (SW6_ECC), HSP_FUSES_ECC (SW6_ECC), &word_offset);
	if (status >= 0) {
		*slot_regs = &fuses->regs->SW6_ecc_fuse.SW6_ecc_fuse[word_offset];
		*ecc = (HSP_FUSES_ECC (SW6_ECC) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (RSVD0)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (RSVD0),
		HSP_FUSES_LENGTH (RSVD0), HSP_FUSES_ECC (RSVD0), &word_offset);
	if (status >= 0) {
		*slot_regs = NULL;
		*ecc = (HSP_FUSES_ECC (RSVD0) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (RSVD1)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (RSVD1),
		HSP_FUSES_LENGTH (RSVD1), HSP_FUSES_ECC (RSVD1), &word_offset);
	if (status >= 0) {
		*slot_regs = NULL;
		*ecc = (HSP_FUSES_ECC (RSVD1) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (RSVD2)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (RSVD2),
		HSP_FUSES_LENGTH (RSVD2), HSP_FUSES_ECC (RSVD2), &word_offset);
	if (status >= 0) {
		*slot_regs = NULL;
		*ecc = (HSP_FUSES_ECC (RSVD2) != 0);

		return status;
	}
#endif

#if HSP_FUSES_LENGTH (RSVD3)
	status = fuse_controller_check_slot (start_addr, bytes, HSP_FUSES_ADDRESS (RSVD3),
		HSP_FUSES_LENGTH (RSVD3), HSP_FUSES_ECC (RSVD3), &word_offset);
	if (status >= 0) {
		*slot_regs = NULL;
		*ecc = (HSP_FUSES_ECC (RSVD3) != 0);

		return status;
	}
#endif

	return FUSE_CONTROLLER_ADDR_OUT_OF_RANGE;
}

int fuse_controller_read_registered_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint8_t *data, size_t length)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	int status = 0;
	uint32_t dword_buffer;
	volatile uint32_t *slot_reg;
	bool ecc;
	size_t i;
	uint32_t fuse_words;

	if ((fuses_hw == NULL) || (data == NULL)) {
		return FUSE_CONTROLLER_READ_REGISTERED_SW_FAILED;
	}

	fuse_words = IN_DWORDS (length);

	status = fuse_controller_validate_sw_program_request (fuses_hw, start_addr, fuse_words,
		&slot_reg, &ecc);
	if (status != 0) {
		return status;
	}
	if (slot_reg == NULL) {
		return FUSE_CONTROLLER_FUSES_NOT_REGISTERED;
	}

	for (i = 0; i < fuse_words; i++) {
		/* Need to copy to an intermediate buffer to ensure memory alignment and dword accesses. */
		dword_buffer = slot_reg[i];
		buffer_unaligned_write32 ((uint32_t*) data, dword_buffer);
		data += sizeof (dword_buffer);
	}

	return 0;
}

/**
 * Internal function to program sw fuses to fuses or GFC registers
 *
 * @param fuses The fuse controller for the SW fuses to program.
 * @param start_addr The first fuse address to program.  This must be 32-bit aligned.
 * @param data The data to commit to fuses.
 * @param fuse_words The number of fuse words to program.  Each fuse word is 32 bits.
 * @param update_fuses If True programs to fuse slots or if false updates GFC registers.
 *
 * @return 0 if the fuse data was programmed successfully or an error code.
 */
static int fuse_controller_update_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words, bool update_fuses)
{
	const struct fuse_controller *fuses_hw = (const struct fuse_controller*) fuses;
	volatile uint32_t *slot_reg;
	bool ecc;
	uint8_t ecc_count;
	int status;

	if ((fuses_hw == NULL) || (data == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	if (!FUSE_CONTROLLER_IS_ADDR_ALIGNED (start_addr)) {
		return FUSE_CONTROLLER_ADDR_NOT_ALIGNED;
	}

	status = fuse_controller_validate_sw_program_request (fuses_hw, start_addr, fuse_words,
		&slot_reg, &ecc);
	if (status != 0) {
		return status;
	}

	ecc_count = 0;
	while ((status == 0) && (fuse_words > 0)) {
		/* The first word of every 4 is for ECC, so we need to skip it. */
		if (update_fuses) {
			if (ecc && (ecc_count == 0)) {
				start_addr += sizeof (uint32_t);
			}

			status = fuse_controller_execute_command (fuses_hw, FUSE_CONTROLLER_CMD_PROGRAM_DATA,
				start_addr, data, 1);
		}

		if (status == 0) {
			/* If there is a register representing the current fused data, update it after a
			 * successful programming.  OR the data bits with the existing value since, once
			 * programmed, fuses can never be cleared. */
			if (slot_reg) {
				*slot_reg |= *data;
				slot_reg++;
			}

			data++;
			fuse_words--;
			start_addr += sizeof (uint32_t);
			ecc_count = (ecc_count + 1) & 0x3;
		}
	}

	return status;
}

int fuse_controller_program_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words)
{
	return fuse_controller_update_sw_fuses (fuses, start_addr, data, fuse_words, true);
}

int fuse_controller_program_registered_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words)
{
	return fuse_controller_update_sw_fuses (fuses, start_addr, data, fuse_words, false);
}

/**
 * Initialize the base driver for interfacing with the HSP fuse controller.
 *
 * This is not a complete initialization of the fuse controller API.  The handlers for RNG
 * calibration data are left for derived types to define depending on how this data is handled.
 *
 * @param fuses The fuse driver instance to initialize.
 * @param state The variable context for a fuse driver instance.  This must be uninitialized.
 * @param regs Base address for the GFC registers.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_init (struct fuse_controller *fuses, struct fuse_controller_state *state,
	struct Gfc_regs *regs)
{
	if ((fuses == NULL) || (state == NULL) || (regs == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	memset (fuses, 0, sizeof (struct fuse_controller));

	fuses->base.get_security_state = fuse_controller_get_security_state;
	fuses->base.change_security_state = fuse_controller_change_security_state;
	fuses->base.blank_check_socid = fuse_controller_blank_check_socid;
	fuses->base.read_registered_socid = fuse_controller_read_registered_socid;
	fuses->base.program_socid = fuse_controller_program_socid;
	fuses->base.read_emc_register = fuse_controller_read_emc_register;
	fuses->base.program_emc_register = fuse_controller_program_emc_register;
	fuses->base.read_aeb_register = fuse_controller_read_aeb_register;
	fuses->base.program_aeb_register = fuse_controller_program_aeb_register;
	fuses->base.blank_check = fuse_controller_blank_check;
	fuses->base.blank_check_key = fuse_controller_blank_check_key;
	fuses->base.get_fuse_map = fuse_controller_get_fuse_map;
	fuses->base.read_sw_fuses = fuse_controller_read_sw_fuses;
	fuses->base.read_registered_sw_fuses = fuse_controller_read_registered_sw_fuses;
	fuses->base.program_sw_fuses = fuse_controller_program_sw_fuses;
	fuses->base.program_registered_sw_fuses = fuse_controller_program_registered_sw_fuses;

	fuses->state = state;
	fuses->regs = regs;

	return fuse_controller_init_state (fuses);
}

/**
 * Initialize only the variable state for a fuse controller driver.  The rest of the driver is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param fuses The fuse controller driver that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int fuse_controller_init_state (const struct fuse_controller *fuses)
{
	uint32_t load_status;

	if ((fuses == NULL) || (fuses->state == NULL) || (fuses->regs == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	memset (fuses->state, 0, sizeof (struct fuse_controller_state));

	/* Wait for the fuses to complete initialization. */
	do {
		load_status = fuses->regs->Fuse_Load_Status.Fuse_Load_Status[1];
	} while (!(load_status &
		(GFC_REGS_FUSE_LOAD_STATUS_FUSE_LOAD_STATUS_1_2_LOAD_PASS_FIELD_MASK |
		GFC_REGS_FUSE_LOAD_STATUS_FUSE_LOAD_STATUS_1_2_LOAD_ERROR_FIELD_MASK)));

	if (load_status & GFC_REGS_FUSE_LOAD_STATUS_FUSE_LOAD_STATUS_1_2_LOAD_ERROR_FIELD_MASK) {
		return FUSE_CONTROLLER_LOAD_ERROR;
	}

	return platform_mutex_init (&fuses->state->lock);
}

/**
 * Release the resources used by a fuse controller driver.
 *
 * @param fuses The driver instance to release.
 */
void fuse_controller_release (const struct fuse_controller *fuses)
{
	if (fuses) {
		platform_mutex_free (&fuses->state->lock);
	}
}
