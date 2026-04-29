// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RESET_COUNTER_INIT_H_
#define RESET_COUNTER_INIT_H_

#include "cmd_interface/counter_manager_registers.h"


extern struct counter_manager_registers reset_count;


int initialize_and_increment_reset_counter ();
int initialize_reset_counters ();


#endif	/* RESET_COUNTER_INIT_H_ */
