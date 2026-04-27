// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_INTRUSION_H_
#define INIT_INTRUSION_H_

#include "intrusion/authorized_execution_reset_intrusion_static.h"
#include "intrusion/intrusion_state_hsp_static.h"
#include "msft_protocol/mctp_notifier_msft_const_list_static.h"


extern const struct authorized_execution_reset_intrusion reset_intrusion_execution;
extern const struct intrusion_state_hsp intrusion_state;
extern const struct mctp_notifier_msft_const_list mctp_notifier_msft;


int initialize_intrusion (void);
int start_intrusion (void);


#endif	/* INIT_INTRUSION_H_ */
