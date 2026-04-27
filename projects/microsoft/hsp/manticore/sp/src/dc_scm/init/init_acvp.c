// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_top.h"
#include "init_acvp.h"
#include "init_crypto.h"
#include "manticore_hsp_gpio.h"
#include "acvp/acvp_proto_static.h"
#include "acvp/acvp_proto_tester_adapter_static.h"
#include "acvp/backend_aead.h"
#include "acvp/backend_ecdh.h"
#include "acvp/backend_ecdsa.h"
#include "acvp/backend_ecdsa_hsp_fw.h"
#include "acvp/backend_hkdf.h"
#include "acvp/backend_hmac.h"
#include "acvp/backend_rsa.h"
#include "acvp/backend_sha.h"
#include "acvp/backend_sym.h"
#include "common/array_size.h"
#include "drivers/hsp_gpio.h"
#include "drivers/hsp_gpio_static.h"


/**
 * Driver for HSP GPIOs.
 */
const struct hsp_gpio gpio =
	hsp_gpio_static_init_no_irq_support (
	(struct Creg_regs_gpc_regs*) HSP_ADDR_MAP_CREG_GPIO_REGS_ADDRESS, MANTICORE_HSP_GPIO_COUNT);

/**
 * Variable context for ACVP Proto.
 */
static struct acvp_proto_state acvp_state;

/**
 * Interface for ACVP Proto tester library.
 */
static const struct acvp_proto_tester_adapter acvp_tester_adapter =
	acvp_proto_tester_adapter_static_init;

/**
 * Handler for executing ACVP Proto operations.
 */
const struct acvp_proto acvp = acvp_proto_static_init (&acvp_state, &acvp_tester_adapter.base);


/**
 * List of AES-GCM engines to register with ACVP backend AEAD.
 */
static const struct backend_aead_engine aead_engines[] = {
	{
		.impl_id = 0,
		.gcm_engine = &aes_gcm.base,
		.rng = &shared_rng.base
	}
};

/**
 * List of ECDH engines to register with ACVP backend ECDH.
 */
static const struct backend_ecdh_engine ecdh_engines[] = {
	{
		.impl_id = 0,
		.is_hw = false,
		.ecc.engine = &shared_ecc.base
	}
};

#ifndef ECDSA_ROM_ENABLE_FIPS_ACVP_TESTING
/**
 * Static pointer to ECDSA backend engine registration function.
 */
static void (*backend_ecdsa_register_engines_static) (const struct backend_ecdsa_engine*, size_t) =
	backend_ecdsa_register_engines;

/**
 * Static pointer to ECDSA backend implementation registration function.
 */
static void (*backend_ecdsa_register_impl_static) (void) =
	backend_ecdsa_register_impl;

/**
 * List of ECDSA engine structures to register with ACVP backend ECDSA.
 */
static const struct backend_ecdsa_engine ecdsa_engines[] = {
	{
		.impl_id = 0,
		.is_hw = false,
		.ecc.engine = &shared_ecc.base,
		.hash = &shared_hash.base,
		.keygen_type = BACKEND_ECDSA_KEYGEN_TYPE_TESTING_CANDIDATES,
		.api_type = BACKEND_ECDSA_API_TYPE_MESSAGE
	},
	{
		.impl_id = 1,
		.is_hw = true,
		.ecc.hw = &pka.base,
		.hash = &shared_hash.base,
		.keygen_type = BACKEND_ECDSA_KEYGEN_TYPE_TESTING_CANDIDATES,
		.api_type = BACKEND_ECDSA_API_TYPE_MESSAGE
	},
	{
		.impl_id = 2,
		.is_hw = false,
		.ecc.engine = &shared_ecc.base,
		.hash = &shared_hash.base,
		.keygen_type = BACKEND_ECDSA_KEYGEN_TYPE_TESTING_CANDIDATES,
		.api_type = BACKEND_ECDSA_API_TYPE_HASH_AND_FINISH
	},
	{
		.impl_id = 3,
		.is_hw = true,
		.ecc.hw = &pka.base,
		.hash = &shared_hash.base,
		.keygen_type = BACKEND_ECDSA_KEYGEN_TYPE_TESTING_CANDIDATES,
		.api_type = BACKEND_ECDSA_API_TYPE_HASH_AND_FINISH
	}
};
#else
/**
 * Static pointer to ECDSA HSP FW backend engine registration function.
 */
static void (*backend_ecdsa_register_engines_static) (const struct backend_ecdsa_engine*, size_t) =
	backend_ecdsa_hsp_fw_register_engines;

/**
 * Static pointer to ECDSA HSP FW backend implementation registration function.
 */
static void (*backend_ecdsa_register_impl_static) (void) =
	backend_ecdsa_hsp_fw_register_impl;

/**
 * List of ECDSA HSP FW engine structures to register with ACVP backend ECDSA.
 */
static const struct backend_ecdsa_engine ecdsa_engines[] = {
	{
		.impl_id = 4,
		.is_hw = true,
		.ecc.hw = &pka.base,
		.hash = &shared_hash.base,
		.keygen_type = BACKEND_ECDSA_KEYGEN_TYPE_KEYGEN_UNSUPPORTED,
		.api_type = BACKEND_ECDSA_API_TYPE_MESSAGE
	}
};
#endif

/**
 * List of HKDF engines to register with the ACVP backend HKDF.
 */
static const struct backend_hkdf_engine hkdf_engines[] = {
	{
		.impl_id = 0,
		.intf = &hkdf.base
	}
};

/**
 * List of HMAC engines to register with the ACVP backend HMAC.
 */
static const struct backend_hmac_engine hmac_engines[] = {
	{
		.impl_id = 0,
		.engine = &shared_hash.base
	}
};

/**
 * List of RSA engines to register with the ACVP backend RSA.
 */
static const struct backend_rsa_engine rsa_engines[] = {
	{
		.impl_id = 0,
		.random_e_supported = false,
		.engine = &shared_rsa.base
	}
};

/**
 * List of SHA engines to register with ACVP backend SHA.
 */
static const struct backend_sha_engine sha_engines[] = {
	{
		.impl_id = 0,
		.is_one_shot = false,
		.engine = &shared_hash.base
	},
	{
		.impl_id = 1,
		.is_one_shot = true,
		.engine = &shared_hash.base
	}
};

/**
 * List of symmetric encryption engines to register with ACVP backend SYM.
 */
static const struct backend_sym_engine sym_engines[] = {
	{
		.impl_id = 0,
		.aes_kw = &aes_kwp.base.base,
		.type = BACKEND_SYM_ENGINE_TYPE_AES_KWP
	}
};


/**
 * Register the ACVP Proto AEAD implementations.
 */
extern void _init_register_proto_aead (void);

/**
 * Register the ACVP Proto ECDH implementations.
 */
extern void _init_register_proto_ecdh (void);

/**
 * Register the ACVP Proto ECDSA implementations.
 */
extern void _init_register_proto_ecdsa (void);

/**
 * Register the ACVP Proto HKDF implementations.
 */
extern void _init_register_proto_hkdf (void);

/**
 * Register the ACVP Proto HMAC implementations.
 */
extern void _init_register_proto_hmac (void);

/**
 * Register the ACVP Proto RSA implementations.
 */
extern void _init_register_proto_rsa (void);

/**
 * Register the ACVP Proto SHA implementations.
 */
extern void _init_register_proto_sha (void);

/**
 * Register the ACVP Proto symmetric cipher implementations.
 */
extern void _init_register_proto_sym (void);


/**
 * Register the crypto engines with their respective backends.
 */
void register_crypto_engines ()
{
	backend_aead_register_engines (aead_engines, ARRAY_SIZE (aead_engines));
	backend_ecdh_register_engines (ecdh_engines, ARRAY_SIZE (ecdh_engines));
	backend_ecdsa_register_engines_static (ecdsa_engines, ARRAY_SIZE (ecdsa_engines));
	backend_hkdf_register_engines (hkdf_engines, ARRAY_SIZE (hkdf_engines));
	backend_hmac_register_engines (hmac_engines, ARRAY_SIZE (hmac_engines));
	backend_rsa_register_engines (rsa_engines, ARRAY_SIZE (rsa_engines));
	backend_sha_register_engines (sha_engines, ARRAY_SIZE (sha_engines));
	backend_sym_register_engines (sym_engines, ARRAY_SIZE (sym_engines));
}

/**
 * Register the ACVP Proto backend tester handlers.
 */
void register_proto_backends ()
{
	_init_register_proto_aead ();
	_init_register_proto_ecdh ();
	_init_register_proto_ecdsa ();
	_init_register_proto_hkdf ();
	_init_register_proto_hmac ();
	_init_register_proto_rsa ();
	_init_register_proto_sha ();
	_init_register_proto_sym ();
}

/**
 * Register the core backend implementations.
 */
void register_core_backends ()
{
	backend_aead_register_impl ();
	backend_ecdh_register_impl ();
	backend_ecdsa_register_impl_static ();
	backend_hkdf_register_impl ();
	backend_hmac_register_impl ();
	backend_rsa_register_impl ();
	backend_sha_register_impl ();
	backend_sym_register_impl ();
}


/**
 * Initialize the handling for ACVP testing.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int initialize_acvp ()
{
	int status;

	/* When ACVP testing is enabled, normal host initialization flows are disabled.  Initialize the
	 * host GPIOs here to allow the host to boot. */
	status = hsp_gpio_write (&gpio, PORT1_SPI_FILTER_MUX, true);
	if (status != 0) {
		return status;
	}

	status = hsp_gpio_write (&gpio, PORT1_RESET_CTRL, true);
	if (status != 0) {
		return status;
	}

	status = acvp_proto_init_state (&acvp);
	if (status != 0) {
		return status;
	}

	register_crypto_engines ();

	register_proto_backends ();

	register_core_backends ();

	return status;
}
