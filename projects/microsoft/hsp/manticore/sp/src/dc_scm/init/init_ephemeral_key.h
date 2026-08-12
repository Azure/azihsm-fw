// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_EPHEMERAL_KEY_H_
#define INIT_EPHEMERAL_KEY_H_

#include "keystore/ephemeral_key_manager_static.h"
#include "keystore/key_cache_flash.h"



extern const struct ephemeral_key_manager rsa_ephemeral_key_manager;
extern const struct key_cache_flash rsa_key_cache_flash;


int initialize_ephemeral_key_handler ();
int start_ephemeral_key_manager ();


#endif	// INIT_EPHEMERAL_KEY_H_
