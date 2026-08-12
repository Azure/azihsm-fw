// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_EPHEMERAL_KEY_MONITOR_H_
#define INIT_EPHEMERAL_KEY_MONITOR_H_

#include "keystore/ephemeral_key_monitor.h"


extern const struct ephemeral_key_monitor rsa_ephemeral_key_monitor;


int initialize_ephemeral_key_monitor ();
int start_ephemeral_key_monitor ();


#endif	// INIT_EPHEMERAL_KEY_MONITOR_H_
