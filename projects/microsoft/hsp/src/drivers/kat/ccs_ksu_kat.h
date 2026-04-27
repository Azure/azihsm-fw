// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_KAT_H_
#define CCS_KSU_KAT_H_

#include "drivers/ccs_ksu_interface.h"


int ccs_ksu_kat_run_self_test_hmac_sha256 (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int ccs_ksu_kat_run_self_test_hmac_sha384 (const struct ccs_ksu_interface *ccs, uint8_t key_slot);

int ccs_ksu_kat_run_self_test_kdf256 (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int ccs_ksu_kat_run_self_test_kdf384 (const struct ccs_ksu_interface *ccs, uint8_t key_slot);


/* Self tests leverage CCS driver error codes. */


#endif	/* CCS_KSU_KAT_H_ */
