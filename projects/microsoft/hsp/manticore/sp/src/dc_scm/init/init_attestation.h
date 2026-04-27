// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_ATTESTATION_H_
#define INIT_ATTESTATION_H_

#include "attestation/pcr_store.h"
#include "firmware/identity_renewal_static.h"
#include "flash/flash_store_contiguous_blocks_static.h"
#include "keystore/keystore_flash_static.h"
#include "riot/riot_key_manager_static.h"


extern const struct flash_store_contiguous_blocks keystore_flash;
extern const struct keystore_flash main_keystore;
extern const struct riot_key_manager dice_key_manager;
extern const struct identity_renewal identity;

extern struct pcr_store pcr_storage;

extern struct pcr_measured_data pcr_cfm_valid_measured_data;


int initialize_dme_key_export ();
int initialize_dice_key_manager ();
int initialize_manticore_measurements ();


#endif	/* INIT_ATTESTATION_H_ */
