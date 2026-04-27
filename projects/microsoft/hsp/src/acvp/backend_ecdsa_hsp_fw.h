// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BACKEND_ECDSA_HSP_FW_H_
#define BACKEND_ECDSA_HSP_FW_H_

#include "acvp/backend_ecdsa.h"


const struct ecdsa_backend* backend_ecdsa_hsp_fw_get_impl ();


void backend_ecdsa_hsp_fw_register_engines (const struct backend_ecdsa_engine *ecdsa,
	size_t num_engines);
void backend_ecdsa_hsp_fw_register_impl (void);


#endif	/* BACKEND_ECDSA_HSP_FW_H_ */
