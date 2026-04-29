// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_SHACK1_H_
#define CCS_KSU_SHACK1_H_


#include "drivers/ccs_ksu.h"


int ccs_ksu_shack1_init_polling (struct ccs_ksu *ccs, struct ccs_ksu_state *state,
	struct Ccs_regs *regs, const struct hs_sha *sha, const struct hsp_aes *aes,
	const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs);
int ccs_ksu_shack1_init_interrupt (struct ccs_ksu *ccs, struct ccs_ksu_state *state,
	struct Ccs_regs *regs, struct Creg_regs_creg_crypto_group *irq_regs, const struct hs_sha *sha,
	const struct hsp_aes *aes, const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng,
	struct ccs_cmd_buffer *cmd_buffer, const struct ksu_key_slot *keys, size_t num_keys,
	const struct ksu_pcr_slot *pcrs, size_t num_pcrs);


#endif	/* CCS_KSU_SHACK1_H_ */
