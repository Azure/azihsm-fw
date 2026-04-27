// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_CRASHDUMP_H_
#define INIT_CRASHDUMP_H_

#include "crashdump/hsp_crashdump_handler_static.h"
#include "crashdump/soc_crashdump_handler_static.h"
#include "crashdump/soc_crashdump_interface_static.h"
#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#include "crashdump/soc_crashdump_ras_fault_injection_static.h"
#endif


extern const struct hsp_crashdump_handler hsp_crashdump_handler;

extern const struct soc_crashdump_interface soc_api;
extern const struct soc_crashdump_handler soc_handler;
extern const struct soc_crashdump_ras_fault_injection ras_fault_inj_test;


int initialize_crashdump_hsp (void);
bool hsp_crashdump_exception_handler (uintptr_t param);

#ifdef MANTICORE_ROT_RESET_CRASH
int trigger_crashdump_soc (void);
#endif


#endif	// INIT_CRASHDUMP_H_
