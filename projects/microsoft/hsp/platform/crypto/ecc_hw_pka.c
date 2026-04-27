// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_cmd.h"
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/common_math.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/ecc_hw_pka.h"
#include "crypto/hash.h"
#include "drivers/crypto_hw.h"
#include "drivers/hsp_rng_hw.h"
#include "drivers/sram.h"
#include "splibs/inc/sptypes.h"


/**
 * SEC Standard:  https://www.secg.org/sec2-v2.pdf
 *
 * 2.4.2 Recommended Parameters secp256r1
 * The verifiably random elliptic curve domain parameters over Fp secp256r1 are specified by the
 * sextuple T = (p, a, b, G, n, h) where the finite field Fp is defined by:
 * p = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF
 * FFFFFFFF
 * = 2^224(2^32 − 1) + 2^192 + 2^96 − 1
 * The curve E: y^2 = x^3 + ax + b over Fp is defined by:
 * a = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF
 * FFFFFFFC
 * b = 5AC635D8 AA3A93E7 B3EBBD55 769886BC 651D06B0 CC53B0F6 3BCE3C3E
 * 27D2604B
 * E was chosen verifiably at random as specified in ANSI X9.62 [X9.62] from the seed:
 * S = C49D3608 86E70493 6A6678E1 139D26B7 819F7E90
 * The base point G in compressed form is:
 * G = 03 6B17D1F2 E12C4247 F8BCE6E5 63A440F2 77037D81 2DEB33A0
 * F4A13945 D898C296
 * and in uncompressed form is:
 * G = 04 6B17D1F2 E12C4247 F8BCE6E5 63A440F2 77037D81 2DEB33A0
 * F4A13945 D898C296 4FE342E2 FE1A7F9B 8EE7EB4A 7C0F9E16 2BCE3357
 * 6B315ECE CBB64068 37BF51F5
 * Finally the order n of G and the cofactor are:
 * n = FFFFFFFF 00000000 FFFFFFFF FFFFFFFF BCE6FAAD A7179E84 F3B9CAC2
 * FC632551
 * h = 01
 *
 * The value of p is reversed to little-endian for compatibility with PKA.
 */
static const uint8_t ECC_HW_PKA_P256_P[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

/**
 * The base point (G) for the P256 curve.  Big endian format.
 */
static const struct ecc_point_public_key ECC_HW_PKA_P256_G = {
	.x = {
		0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47,
		0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2,
		0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0,
		0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
	},
	.y = {
		0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b,
		0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16,
		0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce,
		0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
	},
	.key_length = ECC_KEY_LENGTH_256
};

/**
 * The order (n) of the base point (G) for the P256 curve.  Big endian format.
 */
static const uint8_t ECC_HW_PKA_P256_N[] = {
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51
};

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
/**
 * SEC Standard:  https://www.secg.org/sec2-v2.pdf
 *
 * 2.5.1 Recommended Parameters secp384r1
 * The verifiably random elliptic curve domain parameters over Fp secp384r1 are specified by the
 * sextuple T = (p, a, b, G, n, h) where the finite field Fp is defined by:
 * p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFE FFFFFFFF 00000000 00000000 FFFFFFFF
 * = 2^384 − 2^128 − 2^96 + 2^32 − 1
 * The curve E: y^2 = x^3 + ax + b over Fp is defined by:
 * a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFE FFFFFFFF 00000000 00000000 FFFFFFFC
 * b = B3312FA7 E23EE7E4 988E056B E3F82D19 181D9C6E FE814112 0314088F
 * 5013875A C656398D 8A2ED19D 2A85C8ED D3EC2AEF
 * E was chosen verifiably at random as specified in ANSI X9.62 [X9.62] from the seed:
 * S = A335926A A319A27A 1D00896A 6773A482 7ACDAC73
 * The base point G in compressed form is:
 * G = 03 AA87CA22 BE8B0537 8EB1C71E F320AD74 6E1D3B62 8BA79B98
 * 59F741E0 82542A38 5502F25D BF55296C 3A545E38 72760AB7
 * and in uncompressed form is:
 * G = 04 AA87CA22 BE8B0537 8EB1C71E F320AD74 6E1D3B62 8BA79B98
 * 59F741E0 82542A38 5502F25D BF55296C 3A545E38 72760AB7 3617DE4A
 * 96262C6F 5D9E98BF 9292DC29 F8F41DBD 289A147C E9DA3113 B5F0B8C0
 * 0A60B1CE 1D7E819D 7A431D7C 90EA0E5F
 * Finally the order n of G and the cofactor are:
 * n = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF C7634D81
 * F4372DDF 581A0DB2 48B0A77A ECEC196A CCC52973
 * h = 01
 *
 * The value of p is reversed to little-endian for compatibility with PKA.
 */
static const uint8_t ECC_HW_PKA_P384_P[] = {
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/**
 * The base point (G) for the P384 curve.  Big endian format.
 */
static const struct ecc_point_public_key ECC_HW_PKA_P384_G = {
	.x = {
		0xaa, 0x87, 0xca, 0x22, 0xbe, 0x8b, 0x05, 0x37,
		0x8e, 0xb1, 0xc7, 0x1e, 0xf3, 0x20, 0xad, 0x74,
		0x6e, 0x1d, 0x3b, 0x62, 0x8b, 0xa7, 0x9b, 0x98,
		0x59, 0xf7, 0x41, 0xe0, 0x82, 0x54, 0x2a, 0x38,
		0x55, 0x02, 0xf2, 0x5d, 0xbf, 0x55, 0x29, 0x6c,
		0x3a, 0x54, 0x5e, 0x38, 0x72, 0x76, 0x0a, 0xb7
	},
	.y = {
		0x36, 0x17, 0xde, 0x4a, 0x96, 0x26, 0x2c, 0x6f,
		0x5d, 0x9e, 0x98, 0xbf, 0x92, 0x92, 0xdc, 0x29,
		0xf8, 0xf4, 0x1d, 0xbd, 0x28, 0x9a, 0x14, 0x7c,
		0xe9, 0xda, 0x31, 0x13, 0xb5, 0xf0, 0xb8, 0xc0,
		0x0a, 0x60, 0xb1, 0xce, 0x1d, 0x7e, 0x81, 0x9d,
		0x7a, 0x43, 0x1d, 0x7c, 0x90, 0xea, 0x0e, 0x5f
	},
	.key_length = ECC_KEY_LENGTH_384
};

/**
 * The order (n) of the base point (G) for the P384 curve.  Big endian format.
 */
static const uint8_t ECC_HW_PKA_P384_N[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0x63, 0x4d, 0x81, 0xf4, 0x37, 0x2d, 0xdf,
	0x58, 0x1a, 0x0d, 0xb2, 0x48, 0xb0, 0xa7, 0x7a, 0xec, 0xec, 0x19, 0x6a, 0xcc, 0xc5, 0x29, 0x73
};
#endif

#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_521
/**
 * SEC Standard:  https://www.secg.org/sec2-v2.pdf
 *
 * 2.6.1 Recommended Parameters secp521r1
 * The verifiably random elliptic curve domain parameters over Fp secp521r1 are specified by the
 * sextuple T = (p, a, b, G, n, h) where the finite field Fp is defined by:
 * p = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFF FFFFFFFF FFFFFFFF
 * = 2^521 − 1
 * The curve E: y^2 = x^3 + ax + b over Fp is defined by:
 * a = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFF FFFFFFFF FFFFFFFC
 * b = 0051 953EB961 8E1C9A1F 929A21A0 B68540EE A2DA725B 99B315F3
 * B8B48991 8EF109E1 56193951 EC7E937B 1652C0BD 3BB1BF07 3573DF88
 * 3D2C34F1 EF451FD4 6B503F00
 * E was chosen verifiably at random as specified in ANSI X9.62 [X9.62] from the seed:
 * S = D09E8800 291CB853 96CC6717 393284AA A0DA64BA
 * The base point G in compressed form is:
 * G = 0200C6 858E06B7 0404E9CD 9E3ECB66 2395B442 9C648139 053FB521
 * F828AF60 6B4D3DBA A14B5E77 EFE75928 FE1DC127 A2FFA8DE 3348B3C1
 * 856A429B F97E7E31 C2E5BD66
 * and in uncompressed form is:
 * G = 04 00C6858E 06B70404 E9CD9E3E CB662395 B4429C64 8139053F
 * B521F828 AF606B4D 3DBAA14B 5E77EFE7 5928FE1D C127A2FF A8DE3348
 * B3C1856A 429BF97E 7E31C2E5 BD660118 39296A78 9A3BC004 5C8A5FB4
 * 2C7D1BD9 98F54449 579B4468 17AFBD17 273E662C 97EE7299 5EF42640
 * C550B901 3FAD0761 353C7086 A272C240 88BE9476 9FD16650
 * Finally the order n of G and the cofactor are:
 * n = 01FF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
 * FFFFFFFF FFFFFFFA 51868783 BF2F966B 7FCC0148 F709A5D0 3BB5C9B8
 * 899C47AE BB6FB71E 91386409
 * h = 01
 *
 * The value of p is reversed to little-endian for compatibility with PKA.
 */
static const uint8_t ECC_HW_PKA_P521_P[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x01
};

/**
 * The base point (G) for the P521 curve.  Big endian format.
 */
static const struct ecc_point_public_key ECC_HW_PKA_P521_G = {
	.x = {
		0x00, 0xc6, 0x85, 0x8e, 0x06, 0xb7, 0x04, 0x04,
		0xe9, 0xcd, 0x9e, 0x3e, 0xcb, 0x66, 0x23, 0x95,
		0xb4, 0x42, 0x9c, 0x64, 0x81, 0x39, 0x05, 0x3f,
		0xb5, 0x21, 0xf8, 0x28, 0xaf, 0x60, 0x6b, 0x4d,
		0x3d, 0xba, 0xa1, 0x4b, 0x5e, 0x77, 0xef, 0xe7,
		0x59, 0x28, 0xfe, 0x1d, 0xc1, 0x27, 0xa2, 0xff,
		0xa8, 0xde, 0x33, 0x48, 0xb3, 0xc1, 0x85, 0x6a,
		0x42, 0x9b, 0xf9, 0x7e, 0x7e, 0x31, 0xc2, 0xe5,
		0xbd, 0x66
	},
	.y = {
		0x01, 0x18, 0x39, 0x29, 0x6a, 0x78, 0x9a, 0x3b,
		0xc0, 0x04, 0x5c, 0x8a, 0x5f, 0xb4, 0x2c, 0x7d,
		0x1b, 0xd9, 0x98, 0xf5, 0x44, 0x49, 0x57, 0x9b,
		0x44, 0x68, 0x17, 0xaf, 0xbd, 0x17, 0x27, 0x3e,
		0x66, 0x2c, 0x97, 0xee, 0x72, 0x99, 0x5e, 0xf4,
		0x26, 0x40, 0xc5, 0x50, 0xb9, 0x01, 0x3f, 0xad,
		0x07, 0x61, 0x35, 0x3c, 0x70, 0x86, 0xa2, 0x72,
		0xc2, 0x40, 0x88, 0xbe, 0x94, 0x76, 0x9f, 0xd1,
		0x66, 0x50
	},
	.key_length = ECC_KEY_LENGTH_521
};

/**
 * The order (n) of the base point (G) for the P521 curve.  Big endian format.
 */
static const uint8_t ECC_HW_PKA_P521_N[] = {
	0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xfa, 0x51, 0x86, 0x87, 0x83, 0xbf, 0x2f, 0x96, 0x6b, 0x7f, 0xcc, 0x01, 0x48, 0xf7, 0x09,
	0xa5, 0xd0, 0x3b, 0xb5, 0xc9, 0xb8, 0x89, 0x9c, 0x47, 0xae, 0xbb, 0x6f, 0xb7, 0x1e, 0x91, 0x38,
	0x64, 0x09
};
#endif


/**
 * Wait for the PKA to be ready to accept a new command and erase the command buffer.
 *
 * @param pka The PKA instance that will be executing the command.
 */
static void ecc_hw_pka_start_new_command (const struct ecc_hw_pka *pka)
{
	/* Spin here to make sure the PKA HW is ready.  This is to clean up after possible FW errors
	 * with previous commands or other abnormal states.  Even when configured to use interrupts,
	 * this will still be a busy loop, for simplicity.  It should typically not get executed. */
	while (pka->regs->status & PKA_REGS_STATUS_BUSY_FIELD_MASK) {
	}
	memset (pka->buffer, 0, sizeof (*pka->buffer));
}

/**
 * Execute a PKA engine command.
 *
 * @param pka The PKA engine to use for command execution.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return 0 if the command was successfully executed or an error code.
 */
static int ecc_hw_pka_execute_command (const struct ecc_hw_pka *pka, int error_code)
{
	uint32_t status;

	/* All PKA commands use the RNG.  Don't lock RNG access prior to command execution to prevent
	 * deadlock. */
	hsp_rng_hw_mark_as_in_use (pka->rng);
	hsp_rng_hw_wait_for_reseed (pka->rng);

	status = pka->submit_command (pka, error_code);

	hsp_rng_hw_mark_as_available (pka->rng);

	return status;
}

/**
 * Convert the hardware status register value to an execution status code.
 *
 * @param status The raw status register value immediately upon command completion.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return The error code to return for the command execution.
 */
static int ecc_hw_pka_parse_command_status (uint32_t status, int error_code)
{
	if (status & PKA_REGS_STATUS_COMPLETE_FIELD_MASK) {
		return 0;
	}
	else if (status & PKA_REGS_STATUS_ERROR_CMD_FIELD_MASK) {
		/* For generic command failures, return a more specific error code. */
		return error_code;
	}
	else if (status == 0) {
		/* The command was never accepted by the HW. */
		return ECC_HW_CMD_NOT_STARTED;
	}
	else {
		/* For other errors, return the detailed error bits. */
		return ECC_HW_PKA_HW_ERROR (status);
	}
}

int ecc_hw_pka_submit_command_polling (const struct ecc_hw_pka *pka, int error_code)
{
	return crypto_hw_submit_command_polling (&pka->buffer->cmd, &pka->regs->command,
		&pka->regs->status, PKA_REGS_STATUS_BUSY_FIELD_MASK, ecc_hw_pka_parse_command_status,
		error_code, ECC_HW_CMD_EXE_TIMEOUT);
}

bool ecc_hw_pka_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct ecc_hw_pka *pka = TO_DERIVED_TYPE (handler, const struct ecc_hw_pka, base_irq);

	UNUSED (param);

	return crypto_hw_handle_interrupt (pka->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, PKA_DONE), &pka->state->done);
}

int ecc_hw_pka_submit_command_interrupt (const struct ecc_hw_pka *pka, int error_code)
{
	return crypto_hw_submit_command_interrupt (pka->irq,
		CRYPTO_HW_IRQ_BIT_MASK (CRYPTO_DONE_INTSTS, PKA_DONE), &pka->state->done, &pka->buffer->cmd,
		&pka->regs->command, &pka->regs->status, PKA_REGS_STATUS_BUSY_FIELD_MASK,
		ecc_hw_pka_parse_command_status, error_code, ECC_HW_CMD_EXE_TIMEOUT);
}

/**
 * Determine the curve domain parameters for the specified key length.
 *
 * @param key_length Length of the private key for the requested curve.
 * @param op_size Optional output for the PKA operation identifier for the specified key length.
 * @param curve_prime Optional output for the prime (p) used by the curve.
 * @param base_point Optional output for the curve base point (G).
 * @param order Optional output for the order (n) of the curve base point.
 *
 * @return 0 if the key length is supported or an error code.
 */
static int ecc_hw_pka_determine_domain_parameters (size_t key_length, uint8_t *op_size,
	const uint8_t **curve_prime, const struct ecc_point_public_key **base_point,
	const uint8_t **order)
{
	const uint8_t *p;
	const struct ecc_point_public_key *g;
	const uint8_t *n;
	uint8_t pka_op;

	switch (key_length) {
		case ECC_KEY_LENGTH_256:
			p = ECC_HW_PKA_P256_P;
			g = &ECC_HW_PKA_P256_G;
			n = ECC_HW_PKA_P256_N;
			pka_op = HSP_CMD_PKA_ECC_SIZE_ENUM_ECC_SIZE_256;
			break;

		case ECC_KEY_LENGTH_384:
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_384
			p = ECC_HW_PKA_P384_P;
			g = &ECC_HW_PKA_P384_G;
			n = ECC_HW_PKA_P384_N;
			pka_op = HSP_CMD_PKA_ECC_SIZE_ENUM_ECC_SIZE_384;
#else

			return ECC_HW_PUBLIC_KEY_TOO_SMALL;
#endif
			break;

		case ECC_KEY_LENGTH_521:
#if ECC_MAX_KEY_LENGTH >= ECC_KEY_LENGTH_521
			p = ECC_HW_PKA_P521_P;
			g = &ECC_HW_PKA_P521_G;
			n = ECC_HW_PKA_P521_N;
			pka_op = HSP_CMD_PKA_ECC_SIZE_ENUM_ECC_SIZE_521;
#else

			return ECC_HW_PUBLIC_KEY_TOO_SMALL;
#endif
			break;

		default:
			return ECC_HW_UNSUPPORTED_KEY_LENGTH;
	}

	if (op_size) {
		*op_size = pka_op;
	}

	if (curve_prime) {
		*curve_prime = p;
	}

	if (base_point) {
		*base_point = g;
	}

	if (order) {
		*order = n;
	}

	return 0;
}

/**
 * Randomize the Montgomery constants used for other PKA operations.  This invalidates all previous
 * parameters and any currently cached Montgomery value.
 *
 * Since this is a helper function that will get called as part of a larger sequence of steps, it is
 * expected the driver mutex will already have been acquired prior to calling.  The mutex state will
 * not be altered by this call.
 *
 * @param pka The PKA engine to update.
 * @param prime The prime value for the curve that will be used.
 * @param length Length of the curve prime.
 * @param op_size Enumeration for the operation size to execute.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_montgomery_constant_calculation (const struct ecc_hw_pka *pka,
	const uint8_t *prime, size_t length, uint8_t op_size)
{
	ecc_hw_pka_start_new_command (pka);

	memcpy (pka->buffer->input.ecc_prime.bytes, prime, length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_SET (HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_OP_MODE_SET (HSP_CMD_PKA_MOD_MONT_OP_ENUM_MONT_CONST_CALC) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.ecc_prime;

	return ecc_hw_pka_execute_command (pka, ECC_HW_MONT_CONST_FAILED);
}

/**
 * Convert an integer value to Montgomery representation.  The Montgomery constant must have already
 * been calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for Montgomery conversion.
 * @param value The integer value to convert.
 * @param length Length of the integer value.
 * @param op_size PKA code for the operation size to execute.
 * @param mont_value Output for the Montgomery representation.  This must be large enough to hold
 * the extra space needed for the montgomery formatted data.
 *
 * @return 0 if the conversion to Montgomery representation was successful or an error code.
 */
static int ecc_hw_pka_convert_to_montgomery_representation (const struct ecc_hw_pka *pka,
	const uint8_t *value, size_t length, uint8_t op_size, uint8_t *mont_value)
{
	int status;

	ecc_hw_pka_start_new_command (pka);

	buffer_reverse_copy (pka->buffer->input.int_arg1.bytes, value, length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_SET (HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_OP_MODE_SET (HSP_CMD_PKA_MOD_MONT_OP_ENUM_MONT_REP_IN) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.int_arg1;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_INT_TO_MONT_FAILED);
	if (status != 0) {
		return status;
	}

	memcpy (mont_value, pka->buffer->output.mont_result.bytes,
		ECC_HW_PKA_MONT_LENGTH_FROM_BYTES (length));

	return 0;
}

/**
 * Convert a Montgomery value to an integer.  The Montgomery constant must have already been
 * calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for Montgomery conversion.
 * @param mont_value The Montgomery value to convert.
 * @param length Length of the integer value.  This should not include the Montgomery format
 * overhead.
 * @param op_size PKA code for the operation size to execute.
 * @param value Output for the integer value.  This must be at least the specified integer value
 * length.
 *
 * @return 0 if the conversion from Montgomery representation was successful or an error code.
 */
static int ecc_hw_pka_convert_from_montgomery_representation (const struct ecc_hw_pka *pka,
	const uint8_t *mont_value, size_t length, uint8_t op_size, uint8_t *value)
{
	int status;

	ecc_hw_pka_start_new_command (pka);

	memcpy (pka->buffer->input.mont_arg1.bytes, mont_value,
		ECC_HW_PKA_MONT_LENGTH_FROM_BYTES (length));

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_SET (HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_OP_MODE_SET (HSP_CMD_PKA_MOD_MONT_OP_ENUM_MONT_REP_OUT) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.mont_arg1;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_MONT_TO_INT_FAILED);
	if (status != 0) {
		return status;
	}

	buffer_reverse_copy (value, pka->buffer->output.int_result.bytes, length);

	return 0;
}

/**
 * Perform a modular reduction an integer value.  The Montgomery constant must have already been
 * calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for the operation.
 * @param value The integer value to reduce.
 * @param length Length of the integer.
 * @param op_size PKA code for the operation size to execute.
 * @param reduced_value Output for the modular reduced value.  This must be at least the same length
 * as the input value.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_modular_reduction (const struct ecc_hw_pka *pka, const uint8_t *value,
	size_t length, uint8_t op_size, uint8_t *reduced_value)
{
	int status;

	ecc_hw_pka_start_new_command (pka);

	buffer_reverse_copy (pka->buffer->input.int_arg1.bytes, value, length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_SET (HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_OP_MODE_SET (HSP_CMD_PKA_MOD_MONT_OP_ENUM_MOD_RED) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.int_arg1;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_MOD_REDUCE_FAILED);
	if (status != 0) {
		return status;
	}

	buffer_reverse_copy (reduced_value, pka->buffer->output.int_result.bytes, length);

	return 0;
}

/**
 * Execute a modular operation that operates against Montgomery inputs.  The Montgomery constant
 * must have already been calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for the operation.
 * @param mont_arg1 The Montgomery representation of the first integer value.
 * @param mont_arg2 The Montgomery representation of the second integer value.  Set this to null if
 * there is only a single argument.
 * @param mont_length Length of the Montgomery values.  Both values must be the same length and must
 * include the Montgomery format overhead.
 * @param op_size PKA code for the operation size to execute.
 * @param mod_op PKA code for the modular operation to execute.
 * @param op_error Error code to return if the operation fails.
 * @param mont_result Output for the Montgomery representation of the result.  This must be at least
 * the same length as the input values.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_montgomery_modular_operation (const struct ecc_hw_pka *pka,
	const uint8_t *mont_arg1, const uint8_t *mont_arg2, size_t mont_length, uint8_t op_size,
	uint8_t mod_op, int op_error, uint8_t *mont_result)
{
	int status;

	ecc_hw_pka_start_new_command (pka);

	memcpy (pka->buffer->input.mont_arg1.bytes, mont_arg1, mont_length);
	if (mont_arg2) {
		memcpy (pka->buffer->input.mont_arg2.bytes, mont_arg2, mont_length);
	}

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_SET (HSP_CMD_PKA_CMD_CODE_MOD_MONT_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_OP_MODE_SET (mod_op) |
		HSP_CMD_PKA_CMD_CODE_MOD_MONT_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.mont_arg1;
	if (mont_arg2) {
		pka->buffer->cmd.arg2 = (uint32_t) &pka->buffer->input.mont_arg2;
	}

	status = ecc_hw_pka_execute_command (pka, op_error);
	if (status != 0) {
		return status;
	}

	memcpy (mont_result, pka->buffer->output.mont_result.bytes, mont_length);

	return 0;
}

/**
 * Calculate the modular product of two integer values.  The Montgomery constant must have already
 * been calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for the operation.
 * @param mont_value1 The Montgomery representation of the first integer value.
 * @param mont_value2 The Montgomery representation of the second integer value.
 * @param length Length of the integer values.  Both values must be the same length and must include
 * the Montgomery format overhead.
 * @param op_size PKA code for the operation size to execute.
 * @param mont_product Output for the modular multiplication result.  This must be at least the same
 * length as the input values.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_modular_multiply (const struct ecc_hw_pka *pka, const uint8_t *mont_value1,
	const uint8_t *mont_value2, size_t length, uint8_t op_size, uint8_t *mont_product)
{
	return ecc_hw_pka_montgomery_modular_operation (pka, mont_value1, mont_value2, length, op_size,
		HSP_CMD_PKA_MOD_MONT_OP_ENUM_MOD_MUL, ECC_HW_MOD_MULTIPLY_FAILED, mont_product);
}

/**
 * Calculate the modular sum of two integer values.  The Montgomery constant must have already been
 * calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for the operation.
 * @param mont_value1 The Montgomery representation of the first integer value.
 * @param mont_value2 The Montgomery representation of the second integer value.
 * @param length Length of the values.  Both values must be the same length and must include
 * the Montgomery format overhead.
 * @param op_size PKA code for the operation size to execute.
 * @param mont_sum Output for the modular addition result.  This must be at least the same length as
 * the input values.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_modular_add (const struct ecc_hw_pka *pka, const uint8_t *mont_value1,
	const uint8_t *mont_value2, size_t length, uint8_t op_size, uint8_t *mont_sum)
{
	return ecc_hw_pka_montgomery_modular_operation (pka, mont_value1, mont_value2, length, op_size,
		HSP_CMD_PKA_MOD_MONT_OP_ENUM_MOD_ADD, ECC_HW_MOD_ADD_FAILED, mont_sum);
}

/**
 * Calculate the modular inverse of an integer value.  The Montgomery constant must have already
 * been calculated.
 *
 * The driver mutex must be externally acquired prior to this call.  The mutex state will not be
 * altered.
 *
 * @param pka The PKA engine to use for the operation.
 * @param mont_value The Montgomery representation of the integer value.
 * @param length Length of the value.  This must include the Montgomery format overhead.
 * @param op_size PKA code for the operation size to execute.
 * @param mont_inverse Output for the modular inverse.  This must be at least the same length as
 * the input value.
 *
 * @return 0 if the operation was successful or an error code.
 */
static int ecc_hw_pka_modular_inverse (const struct ecc_hw_pka *pka, const uint8_t *mont_value,
	size_t length, uint8_t op_size, uint8_t *mont_inverse)
{
	return ecc_hw_pka_montgomery_modular_operation (pka, mont_value, NULL, length, op_size,
		HSP_CMD_PKA_MOD_MONT_OP_ENUM_MOD_INV, ECC_HW_MOD_INVERSE_FAILED, mont_inverse);
}

/**
 * Generate an ECC private key.
 *
 * @param pka The PKA engine to use for key generation.
 * @param rng RNG to use for key generation.  If this is null, the HW DRBG will be used.
 * @param priv_key Output for the generated private key.
 * @param key_length Length of the private key to generate.
 *
 * @return 0 if the private key was generated successfully or an error code.
 */
static int ecc_hw_pka_generate_private_key (const struct ecc_hw_pka *pka,
	const struct rng_engine *rng, uint8_t *priv_key, size_t key_length)
{
	const uint8_t *order;
	bool is_zero;
	bool less_than_n;
	int retries = 16;
	int status;

	status = ecc_hw_pka_determine_domain_parameters (key_length, NULL, NULL, NULL, &order);
	if (status != 0) {
		return (status == ECC_HW_PUBLIC_KEY_TOO_SMALL) ? ECC_HW_UNSUPPORTED_KEY_LENGTH : status;
	}

	/* Randomly generate the private key, d, where 0 < d < n.  Retry until a valid key is found. */
	do {
		if (rng != NULL) {
			status = rng->generate_random_buffer (rng, key_length, priv_key);
		}
		else {
			status = hsp_rng_hw_get_random_buffer (pka->rng, priv_key, key_length);
		}

		if (status == 0) {
			/* P-521 keys have extra random bits generated that need to be discarded.  Only the
			 * first 521 bits that were generated are kept.  This is done by right shifting the
			 * buffer by 7 bits, which discards the last bits in the buffer. */
			if (key_length == ECC_KEY_LENGTH_521) {
				common_math_right_shift_array (priv_key, key_length, 7);
			}

			is_zero = common_math_is_array_zero (priv_key, key_length);
			less_than_n = (common_math_compare_array (priv_key, key_length, order, key_length) < 0);
		}
	} while (((status == 0) && (is_zero || !less_than_n)) && (--retries > 0));

	if ((status == 0) && (retries <= 0)) {
		status = ECC_HW_PRIV_KEY_GEN_FAILED;
	}

	return status;
}

/**
 * Execute ECC point multiplication.
 *
 * @param pka The PKA engine to use for execution.
 * @param scalar The scalar value to multiply the point by.
 * @param length Length of the scalar and prime values.  This will determine the curve being used.
 * @param point A point on the curve to multiply.  Set to null to use the base point for the curve.
 * @param result_x Output buffer for the X result of the multiplication.
 * @param result_y Output buffer for the Y result of the multiplication.  Set to null if only the X
 * coordinate is required from the result.
 *
 * @return 0 if the multiplication was successful or an error code.
 */
static int ecc_hw_pka_ecc_point_multiplication (const struct ecc_hw_pka *pka, const uint8_t *scalar,
	size_t length, const struct ecc_point_public_key *point, uint8_t *result_x, uint8_t *result_y)
{
	const uint8_t *curve_prime;
	const struct ecc_point_public_key *base_point;
	uint8_t op_size;
	size_t aligned_length = (length + 0x3) & ~0x3;
	int status;

	status = ecc_hw_pka_determine_domain_parameters (length, &op_size, &curve_prime, &base_point,
		NULL);
	if (status != 0) {
		return status;
	}

	if (point == NULL) {
		point = base_point;
	}

	status = ecc_hw_pka_montgomery_constant_calculation (pka, curve_prime, length, op_size);
	if (status != 0) {
		goto exit;
	}

	ecc_hw_pka_start_new_command (pka);

	/* X, Y */
	buffer_reverse_copy (pka->buffer->input.point.bytes, point->x, length);
	buffer_reverse_copy (&pka->buffer->input.point.bytes[aligned_length], point->y, length);

	buffer_reverse_copy (pka->buffer->input.private.bytes, scalar, length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_ECC_ID_SET (HSP_CMD_PKA_CMD_CODE_ECC_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_ECC_OP_MODE_SET (HSP_CMD_PKA_ECC_OP_ENUM_ECC_PT_MULT) |
		HSP_CMD_PKA_CMD_CODE_ECC_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.point;
	pka->buffer->cmd.arg2 = (uint32_t) &pka->buffer->input.private;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_ECC_PUBLIC_FAILED);
	if (status == 0) {
		/* X, Y */
		buffer_reverse_copy (result_x, pka->buffer->output.point.bytes, length);

		if (result_y) {
			buffer_reverse_copy (result_y, &pka->buffer->output.point.bytes[aligned_length],
				length);
		}
	}

exit:

	return status;
}

int ecc_hw_pka_get_ecc_public_key (const struct ecc_hw *ecc_hw, const uint8_t *priv_key,
	size_t key_length, struct ecc_point_public_key *pub_key)
{
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;
	int status;

	if ((pka == NULL) || (priv_key == NULL) || (pub_key == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&pka->state->lock);

	status = ecc_hw_pka_ecc_point_multiplication (pka, priv_key, key_length, NULL, pub_key->x,
		pub_key->y);

	buffer_zeroize (pka->buffer, sizeof (*pka->buffer));
	platform_mutex_unlock (&pka->state->lock);

	if (status == 0) {
		pub_key->key_length = key_length;
	}

	return status;
}

int ecc_hw_pka_verify_ecc_public_key (const struct ecc_hw *ecc_hw,
	const struct ecc_point_public_key *pub_key)
{
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;
	const uint8_t *curve_prime;
	uint8_t curve_prime_be[ECC_MAX_KEY_LENGTH];
	uint8_t op_size;
	size_t aligned_length;
	size_t key_length;
	int status;

	if ((ecc_hw == NULL) || (pub_key == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	key_length = pub_key->key_length;
	aligned_length = (key_length + 0x3) & ~0x3;

	status = ecc_hw_pka_determine_domain_parameters (key_length, &op_size, &curve_prime, NULL,
		NULL);
	if (status != 0) {
		return status;
	}

	if (common_math_is_array_zero (pub_key->x, pub_key->key_length)) {
		/* If the X coordinate is zero, this can't be a valid public key.  Explicitly checking Y for
		 * zero isn't necessary since such a point would fail HW validation. */
		return ECC_HW_INVALID_PUBLIC_KEY;
	}

	/* The curve prime is defined in little endian format for compatibility with PKA.  It needs to
	 * be byte reversed for comparison against the public key coordinates. */
	buffer_reverse_copy (curve_prime_be, curve_prime, key_length);

	if ((common_math_compare_array (pub_key->x, key_length, curve_prime_be, key_length) >= 0) ||
		(common_math_compare_array (pub_key->y, key_length, curve_prime_be, key_length) >= 0)) {
		/* Either X or Y is greater than or equal to the prime for the curve.  This key is not
		 * valid. */
		return ECC_HW_INVALID_PUBLIC_KEY;
	}

	platform_mutex_lock (&pka->state->lock);

	status = ecc_hw_pka_montgomery_constant_calculation (pka, curve_prime, key_length, op_size);
	if (status != 0) {
		goto exit;
	}

	ecc_hw_pka_start_new_command (pka);

	/* X, Y */
	buffer_reverse_copy (&pka->buffer->input.public.bytes[0], pub_key->x, key_length);
	buffer_reverse_copy (&pka->buffer->input.public.bytes[aligned_length], pub_key->y, key_length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_ECC_ID_SET (HSP_CMD_PKA_CMD_CODE_ECC_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_ECC_OP_MODE_SET (HSP_CMD_PKA_ECC_OP_ENUM_ECC_PT_VAL) |
		HSP_CMD_PKA_CMD_CODE_ECC_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.public;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_VERIFY_PUBLIC_FAILED);
	if (status == 0) {
		if (pka->buffer->output.is_not_valid) {
			status = ECC_HW_INVALID_PUBLIC_KEY;
		}
	}

exit:
	buffer_zeroize (pka->buffer, sizeof (*pka->buffer));
	platform_mutex_unlock (&pka->state->lock);

	return status;
}

int ecc_hw_pka_generate_ecc_key_pair (const struct ecc_hw *ecc_hw, size_t key_length,
	uint8_t *priv_key, struct ecc_point_public_key *pub_key)
{
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;
	int status;

	if ((pka == NULL) || (priv_key == NULL) || (pub_key == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	status = ecc_hw_pka_generate_private_key (pka, NULL, priv_key, key_length);
	if (status != 0) {
		return status;
	}

	return ecc_hw_pka_get_ecc_public_key (ecc_hw, priv_key, key_length, pub_key);
}

int ecc_hw_pka_ecdsa_sign (const struct ecc_hw *ecc_hw, const uint8_t *priv_key, size_t key_length,
	const uint8_t *digest, size_t digest_length, const struct rng_engine *rng,
	struct ecc_ecdsa_signature *signature)
{
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;
	const uint8_t *order;
	uint8_t op_size;
	size_t mont_length;
	union ecc_hw_pka_int_521 n = {0};	/* Curve order for modular arithmetic */
	union ecc_hw_pka_int_521 e = {0};	/* Message digest */
	union ecc_hw_pka_int_521 k = {0};	/* Per-message random value */
	union ecc_hw_pka_int_521 r = {0};	/* Signature r value */
	union ecc_hw_pka_int_521 d = {0};	/* Private key */
	int retries_r;
	int retries_s = 16;
	int status;

	if ((pka == NULL) || (priv_key == NULL) || (digest == NULL) || (signature == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	/* Calculate the ECDSA signature using the procedure from FIPS 186-5, section 6.4.1.
	 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.186-5.pdf
	 *
	 * The PKA ECC Signature Generation command cannot be used because it violates FIPS
	 * requirements since PKA hardware will read random data directly from the RNG for both the k
	 * value and counter measures.  PKA does not ensure random number reads consume full blocks of
	 * random data. */

	status = ecc_hw_pka_determine_domain_parameters (key_length, &op_size, NULL, NULL, &order);
	if (status != 0) {
		return (status == ECC_HW_PUBLIC_KEY_TOO_SMALL) ? ECC_HW_SIGNATURE_TOO_SMALL : status;
	}

	buffer_reverse_copy (n.value.AsBytes, order, key_length);
	signature->length = key_length;
	mont_length = ECC_HW_PKA_MONT_LENGTH_FROM_BYTES (key_length);

	/* Derive the integer e from H.  If len(n) ≥ hashlen, set e = H. Otherwise, set e equal to the
	 * leftmost ceil (log2(n)) bits of H. */
	if (digest_length > key_length) {
		memcpy (&e.value.AsBytes, digest, key_length);
	}
	else {
		memcpy (&e.value.AsBytes[key_length - digest_length], digest, digest_length);
	}

	platform_mutex_lock (&pka->state->lock);

	do {
		retries_r = 16;

		do {
			/* Generate a per-message secret number k, such that 0 < k < n.  This is basically just
			 * a private key */
			status = ecc_hw_pka_generate_private_key (pka, rng, k.value.AsBytes, key_length);
			if (status != 0) {
				goto exit;
			}

			/* Compute the elliptic curve point R = [k]G, as (xR, yR). */
			status = ecc_hw_pka_ecc_point_multiplication (pka, k.value.AsBytes, key_length, NULL,
				r.value.AsBytes, NULL);
			if (status != 0) {
				goto exit;
			}

			/* Set r = xR mod n, such that r != 0. */
			status = ecc_hw_pka_montgomery_constant_calculation (pka, n.value.AsBytes, key_length,
				op_size);
			if (status != 0) {
				goto exit;
			}

			status = ecc_hw_pka_modular_reduction (pka, r.value.AsBytes, key_length, op_size,
				signature->r);
			if (status != 0) {
				goto exit;
			}
		} while (common_math_is_array_zero (signature->r, key_length) && (--retries_r > 0));

		if (retries_r <= 0) {
			status = ECC_HW_ECDSA_SIGN_FAILED;
			goto exit;
		}

		/* Convert integers to Montgomery representation for modular arithmetic, modulo n. */
		status = ecc_hw_pka_convert_to_montgomery_representation (pka, k.value.AsBytes, key_length,
			op_size, k.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_convert_to_montgomery_representation (pka, r.value.AsBytes, key_length,
			op_size, r.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_convert_to_montgomery_representation (pka, e.value.AsBytes, key_length,
			op_size, e.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_convert_to_montgomery_representation (pka, priv_key, key_length,
			op_size, d.mont);
		if (status != 0) {
			goto exit;
		}

		/* Compute k^-1 mod n (i.e. the modular inverse). */
		status = ecc_hw_pka_modular_inverse (pka, k.mont, mont_length, op_size, k.mont);
		if (status != 0) {
			goto exit;
		}

		/* Compute s = k^−1 ⋅ (e + r ⋅ d) mod n.  This will be achieved through the following steps:
		 * - s = k^−1 ⋅ e
		 * - t = k^−1 ⋅ d
		 * - t = t ⋅ r
		 * - s = s + t
		 *
		 * The storage for e will be reused for s, and the storage for k will be reused for t. */
		status = ecc_hw_pka_modular_multiply (pka, k.mont, e.mont, mont_length, op_size, e.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_modular_multiply (pka, k.mont, d.mont, mont_length, op_size, k.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_modular_multiply (pka, k.mont, r.mont, mont_length, op_size, k.mont);
		if (status != 0) {
			goto exit;
		}

		status = ecc_hw_pka_modular_add (pka, e.mont, k.mont, mont_length, op_size, e.mont);
		if (status != 0) {
			goto exit;
		}

		/* Convert the s value (stored in e) from Montgomery representation. */
		status = ecc_hw_pka_convert_from_montgomery_representation (pka, e.mont, key_length,
			op_size, signature->s);
		if (status != 0) {
			goto exit;
		}
	} while (common_math_is_array_zero (signature->s, key_length) && (--retries_s > 0));

	if (retries_s <= 0) {
		status = ECC_HW_ECDSA_SIGN_FAILED;
	}

exit:
	buffer_zeroize (pka->buffer, sizeof (*pka->buffer));
	platform_mutex_unlock (&pka->state->lock);

	/* Erase the private key and per-message secret, as well as other temporary values. */
	buffer_zeroize (&k, sizeof (k));
	buffer_zeroize (&d, sizeof (d));
	buffer_zeroize (&r, sizeof (r));
	buffer_zeroize (&n, sizeof (n));
	buffer_zeroize (&e, sizeof (e));

	return status;
}

int ecc_hw_pka_ecdsa_verify (const struct ecc_hw *ecc_hw,
	const struct ecc_point_public_key *pub_key, const struct ecc_ecdsa_signature *signature,
	const uint8_t *digest, size_t digest_length)
{
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;
	uint8_t op_size;
	size_t aligned_length;
	size_t key_length;
	int status;

	if ((pka == NULL) || (pub_key == NULL) || (signature == NULL) || (digest == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	if (digest_length > SHA512_HASH_LENGTH) {
		return ECC_HW_DIGEST_TOO_LONG;
	}

	if (signature->length != pub_key->key_length) {
		return ECC_HW_SIGNATURE_WRONG_LENGTH;
	}

	key_length = pub_key->key_length;
	aligned_length = (key_length + 0x3) & ~0x3;

	if (digest_length > key_length) {
		digest_length = key_length;
	}

	status = ecc_hw_pka_determine_domain_parameters (key_length, &op_size, NULL, NULL, NULL);
	if (status != 0) {
		return status;
	}

	platform_mutex_lock (&pka->state->lock);
	ecc_hw_pka_start_new_command (pka);

	/* X, Y */
	buffer_reverse_copy (&pka->buffer->input.public.bytes[0], pub_key->x, key_length);
	buffer_reverse_copy (&pka->buffer->input.public.bytes[aligned_length], pub_key->y, key_length);

	/* r, s */
	buffer_reverse_copy (&pka->buffer->input.signature.bytes[0], signature->r, key_length);
	buffer_reverse_copy (&pka->buffer->input.signature.bytes[aligned_length], signature->s,
		key_length);

	buffer_reverse_copy (pka->buffer->input.digest.bytes, digest, digest_length);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_CMD_CODE_ECC_ID_SET (HSP_CMD_PKA_CMD_CODE_ECC_ID_RESET) |
		HSP_CMD_PKA_CMD_CODE_ECC_OP_MODE_SET (HSP_CMD_PKA_ECC_OP_ENUM_ECC_SIGVER) |
		HSP_CMD_PKA_CMD_CODE_ECC_SIZE_MODE_SET (op_size);

	pka->buffer->cmd.result = (uint32_t) &pka->buffer->output;
	pka->buffer->cmd.arg1 = (uint32_t) &pka->buffer->input.digest;
	pka->buffer->cmd.arg2 = (uint32_t) &pka->buffer->input.public;
	pka->buffer->cmd.arg3 = (uint32_t) &pka->buffer->input.signature;

	status = ecc_hw_pka_execute_command (pka, ECC_HW_ECDSA_VERIFY_FAILED);
	if (status == 0) {
		if (pka->buffer->output.is_not_valid) {
			status = ECC_HW_ECDSA_BAD_SIGNATURE;
		}
	}

	buffer_zeroize (pka->buffer, sizeof (*pka->buffer));
	platform_mutex_unlock (&pka->state->lock);

	return status;
}

int ecc_hw_pka_ecdh_compute (const struct ecc_hw *ecc_hw, const uint8_t *priv_key,
	size_t key_length, const struct ecc_point_public_key *pub_key, uint8_t *secret, size_t length)
{
	int status;
	const struct ecc_hw_pka *pka = (const struct ecc_hw_pka*) ecc_hw;

	if ((pka == NULL) || (priv_key == NULL) || (pub_key == NULL) || (secret == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	if (length < key_length) {
		return ECC_HW_ECDH_TOO_SMALL;
	}

	if (pub_key->key_length != key_length) {
		return ECC_HW_PUBLIC_WRONG_LENGTH;
	}

	platform_mutex_lock (&pka->state->lock);

	status = ecc_hw_pka_ecc_point_multiplication (pka, priv_key, key_length, pub_key, secret, NULL);

	buffer_zeroize (pka->buffer, sizeof (*pka->buffer));
	platform_mutex_unlock (&pka->state->lock);

	return status;
}

#ifdef HSP_ADDR_MAP_UPKA_ADDRESS
int ecc_hw_pka_memory_wipe (const struct ecc_hw_pka *pka)
{
	int status = 0;

	if (pka == NULL) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&pka->state->lock);

	ecc_hw_pka_start_new_command (pka);

	pka->buffer->cmd.command_code =
		HSP_CMD_PKA_MEM_WIPE_ID_SET (HSP_CMD_PKA_MEM_WIPE_ID_RESET) |
		HSP_CMD_PKA_MEM_WIPE_OP_MODE_SET (HSP_CMD_PKA_MEM_WIPE);

	status = ecc_hw_pka_execute_command (pka, ECC_HW_MEM_WIPE_FAILED);

	platform_mutex_unlock (&pka->state->lock);

	return status;
}
#endif

int ecc_hw_pka_memory_wipe_unsupported (const struct ecc_hw_pka *pka)
{
	if (pka == NULL) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	return ECC_HW_UNSUPPORTED_OP;
}

/**
 * Initialize the PKA driver components.
 *
 * @param pka The PKA driver to initialize.
 * @param state Variable context for the PKA driver instance.  The must be uninitialized.
 * @param regs Base address for the PKA registers.
 * @param rng Interface to the RNG used by this PKA instance.
 * @param cmd_buffer Location in HSP shared RAM where PKA commands should be constructed.  This
 * must be a 32-bit aligned address.
 *
 * @return 0 if the PKA driver was successfully initialized or an error code.
 */
static int ecc_hw_pka_init (struct ecc_hw_pka *pka, struct ecc_hw_pka_state *state,
	struct Pka_regs *regs, const struct hsp_rng_hw *rng, struct ecc_hw_pka_cmd_buffer *cmd_buffer)
{
	if ((pka == NULL) || (state == NULL) || (regs == NULL) || (rng == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	memset (pka, 0, sizeof (struct ecc_hw_pka));

	pka->base.get_ecc_public_key = ecc_hw_pka_get_ecc_public_key;
	pka->base.verify_ecc_public_key = ecc_hw_pka_verify_ecc_public_key;
	pka->base.generate_ecc_key_pair = ecc_hw_pka_generate_ecc_key_pair;
	pka->base.ecdsa_sign = ecc_hw_pka_ecdsa_sign;
	pka->base.ecdsa_verify = ecc_hw_pka_ecdsa_verify;
	pka->base.ecdh_compute = ecc_hw_pka_ecdh_compute;

#ifdef HSP_ADDR_MAP_UPKA_ADDRESS
	pka->memory_wipe = ecc_hw_pka_memory_wipe;
#else
	pka->memory_wipe = ecc_hw_pka_memory_wipe_unsupported;
#endif

	pka->state = state;
	pka->regs = regs;
	pka->rng = rng;
	pka->buffer = cmd_buffer;

	return ecc_hw_pka_init_state (pka);
}

/**
 * Initialize the PKA hardware and the driver to communicate with it.  PKA operations will enter a
 * busy waiting loop, actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * @param pka The PKA driver to initialize.
 * @param state Variable context for the PKA driver instance.  The must be uninitialized.
 * @param regs Base address for the PKA registers.
 * @param rng Interface to the RNG used by this PKA instance.
 * @param cmd_buffer Location in HSP shared RAM where PKA commands should be constructed.  This
 * must be a 32-bit aligned address.
 *
 * @return 0 if the PKA driver was successfully initialized or an error code.
 */
int ecc_hw_pka_init_polling (struct ecc_hw_pka *pka, struct ecc_hw_pka_state *state,
	struct Pka_regs *regs, const struct hsp_rng_hw *rng, struct ecc_hw_pka_cmd_buffer *cmd_buffer)
{
	int status;

	status = ecc_hw_pka_init (pka, state, regs, rng, cmd_buffer);
	if (status == 0) {
		pka->submit_command = ecc_hw_pka_submit_command_polling;
	}

	return status;
}

/**
 * Initialize the PKA hardware and the driver to communicate with it.  PKA operations will block,
 * waiting for an interrupt to indicate when the hardware has finished.
 *
 * @param pka The PKA driver to initialize.
 * @param state Variable context for the PKA driver instance.  The must be uninitialized.
 * @param regs Base address for the PKA registers.
 * @param irq_regs Base address for the CREG registers to control PKA interrupts.
 * @param rng Interface to the RNG used by this PKA instance.
 * @param cmd_buffer Location in HSP shared RAM where PKA commands should be constructed.  This
 * must be a 32-bit aligned address.
 *
 * @return 0 if the PKA driver was successfully initialized or an error code.
 */
int ecc_hw_pka_init_interrupt (struct ecc_hw_pka *pka, struct ecc_hw_pka_state *state,
	struct Pka_regs *regs, struct Creg_regs_creg_crypto_group *irq_regs,
	const struct hsp_rng_hw *rng, struct ecc_hw_pka_cmd_buffer *cmd_buffer)
{
	int status;

	if (irq_regs == NULL) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	status = ecc_hw_pka_init (pka, state, regs, rng, cmd_buffer);
	if (status == 0) {
		pka->base_irq.handle_interrupt = ecc_hw_pka_handle_interrupt;
		pka->submit_command = ecc_hw_pka_submit_command_interrupt;

		pka->irq = irq_regs;
	}

	return status;
}

/**
 * Initialize only the variable state for a PKA driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param pka The PKA driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int ecc_hw_pka_init_state (const struct ecc_hw_pka *pka)
{
	int status;

	if ((pka == NULL) || (pka->state == NULL) || (pka->regs == NULL) || (pka->rng == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	if ((pka->base_irq.handle_interrupt != NULL) && (pka->irq == NULL)) {
		return ECC_HW_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (pka->buffer, sizeof (*pka->buffer))) {
		return ECC_HW_INVALID_ADDRESS;
	}

	if ((uintptr_t) pka->buffer & 0x3) {
		return ECC_HW_ADDRESS_NOT_ALIGNED;
	}

	memset (pka->state, 0, sizeof (struct ecc_hw_pka_state));

	status = platform_semaphore_init (&pka->state->done);
	if (status != 0) {
		return status;
	}

	status = platform_mutex_init (&pka->state->lock);
	if (status != 0) {
		platform_semaphore_free (&pka->state->done);
	}

	return status;
}

/**
 * Release the resources used by a PKA driver instance.
 *
 * @param pka The PKA driver to release.
 */
void ecc_hw_pka_release (const struct ecc_hw_pka *pka)
{
	if (pka) {
		platform_mutex_free (&pka->state->lock);
		platform_semaphore_free (&pka->state->done);
	}
}

/**
 * Mark the PKA as being used by another HW block, such as CCS.  This prevents PKA access from FW
 * while being used for other HW purposes.
 *
 * This will block until the PKA is available to use.
 *
 * This must be followed by a call to ecc_hw_pka_mark_as_available for the PKA to be used again by
 * FW.
 *
 * @param pka The PKA that will be used by HW.
 */
void ecc_hw_pka_mark_as_in_use (const struct ecc_hw_pka *pka)
{
	if (pka) {
		platform_mutex_lock (&pka->state->lock);
	}
}

/**
 * Mark the PKA as no longer being used by another HW block.
 *
 * @param pka The PKA that is now available for use.
 */
void ecc_hw_pka_mark_as_available (const struct ecc_hw_pka *pka)
{
	if (pka) {
		platform_mutex_unlock (&pka->state->lock);
	}
}
