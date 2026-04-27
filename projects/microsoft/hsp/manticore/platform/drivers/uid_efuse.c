// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdint.h>
#include <string.h>
#include "platform_api.h"
#include "drivers/uid_efuse.h"
#include "marvell/RegIdefuse.h"


/**
 * The number of rows that the full UID spans.
 */
#define UID_EFUSE_ROW_COUNT		3


/**
 * Read the data from a single register row.
 *
 * @param addr The address of the UID fuses.
 * @param row The register row to read.
 *
 * @return Attributes for the key slot.  If the slot is not valid, this will be 0.
 */
static uint32_t uid_efuse_read_row (void *addr, uint8_t row)
{
	uint32_t data;
	Idefuse_t *fuses = (Idefuse_t*) addr;

	fuses->idefuseInt.b.DONE = 0x1;
	fuses->idefuseRowAddr.b.ROW_ADDR = row;

	// Set a read operation and start.
	fuses->idefuseCtrl.b.READ = 0x1;
	fuses->idefuseCtrl.b.START = 0x1;

	// Wait for DONE (0xB0002004 [0]) to be asserted.
	while (fuses->idefuseInt.b.DONE != 0x1) {
	}

	// Delay before reading data.
	platform_msleep (1);

	// Read DATA (0xB0002014 [31:0]) to retrieve the ID eFuse data.
	data = fuses->idefuseDataData;

	// Write to DONE (0xB0002004 [0]) to clear.
	fuses->idefuseInt.b.DONE = 0x1;

	return data;
}

/**
 * Get the device's UID from eFuses.
 *
 * @param uid The UID eFuse context.
 * @param uid_out The output for the UID.
 * @param len The length of the UID output.
 *
 * @return 0 if UID successfully retrieved or an error code.
 */
int uid_efuse_get_uid (const struct uid_efuse *uid, uint8_t *uid_out, size_t len)
{
	uint8_t i;

	if ((uid == NULL) || (uid_out == NULL)) {
		return UID_EFUSE_INVALID_ARGUMENT;
	}

	if (len < UID_EFUSE_LENGTH) {
		return UID_EFUSE_BUFFER_TOO_SMALL;
	}

	platform_mutex_lock (&uid->state->lock);

	// UID begins at row 0.
	for (i = 0; i < UID_EFUSE_ROW_COUNT; i++) {
		((uint32_t*) uid_out)[i] = uid_efuse_read_row (uid->addr, (UID_EFUSE_ROW_COUNT - 1) - i);
	}

	platform_mutex_unlock (&uid->state->lock);

	return 0;
}

/**
 * Initialize a UID eFuse instance.
 *
 * @param uid The UID eFuse driver to initialize.
 * @param addr The address of the UID contents.
 *
 * @return 0 if the driver state was successfully initialized or an error code if an error occurred.
*/
int uid_efuse_init (struct uid_efuse *uid, struct uid_efuse_state *state, void *addr)
{
	if (uid == NULL) {
		return UID_EFUSE_INVALID_ARGUMENT;
	}

	memset (uid, 0, sizeof (struct uid_efuse));

	uid->state = state;
	uid->addr = addr;

	return uid_efuse_init_state (uid);
}

/**
 * Initialize only the variable state for a UID eFuse instance.  The rest of the driver is assumed
 * to have already been initialized.
 *
 * @param uid The UID eFuse driver to initialize.
 * @param addr The address of the UID contents.
 *
 * @return 0 if the driver state was successfully initialized or an error code if an error occurred.
*/
int uid_efuse_init_state (const struct uid_efuse *uid)
{
	if ((uid == NULL) || (uid->state == NULL)) {
		return UID_EFUSE_INVALID_ARGUMENT;
	}

	memset (uid->state, 0, sizeof (struct uid_efuse_state));

	return platform_mutex_init (&uid->state->lock);
}

/**
 * Release the resources used by a UID eFuse instance.
 *
 * @param uid The UID eFuse instance to release.
*/
void uid_efuse_release (const struct uid_efuse *uid)
{
	if (uid && uid->state) {
		platform_mutex_free (&uid->state->lock);
	}
}
