// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_RNG_HW_KAT_H_
#define HSP_RNG_HW_KAT_H_

#include "drivers/hsp_rng_hw.h"


int hsp_rng_hw_kat_run_self_test (const struct hsp_rng_hw *rng);


/* Self-tests will use RNG hardware error codes.*/


#endif	/* HSP_RNG_HW_KAT_H_ */
