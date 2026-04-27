// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_LOG_H_
#define INIT_LOG_H_

#include "logging/logging_flash_static.h"
#include "system/real_time_clock_hsp.h"


extern const struct logging_flash debug_logger;
extern const struct real_time_clock_hsp system_rtc;


void initialize_debug_log ();


#endif	/* INIT_LOG_H_ */
