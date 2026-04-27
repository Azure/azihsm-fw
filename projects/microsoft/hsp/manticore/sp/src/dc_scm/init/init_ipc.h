// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_IPC_H_
#define INIT_IPC_H_

#include "ipc/ipc_channel.h"


extern struct ipc_channel ipc_hsp_to_admin_channel;
extern struct ipc_channel ipc_hsp_to_admin_stop_intf_channel;


int initialize_ipc ();
int start_ipc_handlers ();


#endif	// INIT_IPC_H_
