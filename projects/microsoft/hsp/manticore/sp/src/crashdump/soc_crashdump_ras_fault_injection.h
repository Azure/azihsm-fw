// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_RAS_FAULT_INJECTION_H_
#define SOC_CRASHDUMP_RAS_FAULT_INJECTION_H_

#include "fips/self_test_interface.h"
#include "mmio/mmio_register_block.h"

/**
 * Support CMVP testing by providing a mechanism to inject RAS HSP faults injection.
 */
struct soc_crashdump_ras_fault_injection {
	struct self_test_interface base;				/**< Base API for on-demand RAS HSP faults injection. */
	const struct mmio_register_block *mmio_creg;	/**< The CREG register interface (struct Creg_regs). */
	const struct hsp_dmb *dmb;						/**< HSP DMB object used to map and unmap memory regions */
};


int soc_crashdump_ras_fault_injection_init (struct soc_crashdump_ras_fault_injection *fault_inj,
	const struct mmio_register_block *mmio_creg, const struct hsp_dmb *dmb);
void soc_crashdump_ras_fault_injection_release (
	const struct soc_crashdump_ras_fault_injection *fault_inj);


#endif	/* SOC_CRASHDUMP_RAS_FAULT_INJECTION_H_ */
