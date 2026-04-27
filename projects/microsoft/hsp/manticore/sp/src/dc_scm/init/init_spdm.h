// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_SPDM_H_
#define INIT_SPDM_H_

#include "spdm/cmd_interface_spdm_responder_static.h"
#include "spdm/impactful_check_spdm_static.h"


extern const struct cmd_interface_spdm_responder mctp_spdm_handler;
extern const struct cmd_interface_spdm_responder doe_spdm_handler;

extern const struct impactful_check_spdm spdm_impactful;


int initialize_mctp_spdm_responder ();
int initialize_doe_spdm_responder ();


#endif	/* INIT_SPDM_H_ */
