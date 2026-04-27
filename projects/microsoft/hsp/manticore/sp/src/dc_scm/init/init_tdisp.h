// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_TDISP_H_
#define INIT_TDISP_H_

#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "pcie/cmd_interface_tdisp_event_policy_static.h"


extern const struct cmd_interface_multi_handler spdm_pcisig_handler;
extern const struct cmd_interface_tdisp_event_policy tdisp_event_policy;


int initialize_tdisp (bool is_graceful);


#endif	// INIT_TDISP_H_
