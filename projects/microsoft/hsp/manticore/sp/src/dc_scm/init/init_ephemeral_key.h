// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_EPHEMERAL_KEY_H_
#define INIT_EPHEMERAL_KEY_H_

#include "keystore/ephemeral_key_manager_static.h"


/*
 * Maximum size of a DER-encoded RSA 2048-bit key stored in a flash sector.
 */
#define RSA_2K_DER_KEY_MAX_SIZE			1300


extern const struct ephemeral_key_manager rsa_ephemeral_key_manager;


int initialize_ephemeral_key_handler ();
int start_ephemeral_key_manager ();


#endif	// INIT_EPHEMERAL_KEY_H_
