/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_status.h

Abstract:

    This file contains project application specific status codes.

Author:

    Navin Pai (navinp)

--*/

#pragma once


//
// Status list - Sorted by STAGE, CATEGORY, CODE
// Only the status codes specific to upenhsp project should go here
//
#define HSP_STATUS_LIST_APPS(_macro) \
    /* STAGE_ROM */                                                                                                             \
    _macro(POST_ROM_START,                                  MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0000))     \
    _macro(POST_ROM_RUN_HW_INIT,                            MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0001))     \
    _macro(POST_ROM_READ_CONFIGURATION,                     MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0002))     \
    _macro(POST_ROM_INITIALIZE_KEYS,                        MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0003))     \
    _macro(POST_ROM_FETCH_IMAGE_SLOT1,                      MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0004))     \
    _macro(POST_ROM_FETCH_IMAGE_SLOT2,                      MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0005))     \
    _macro(POST_ROM_REVOCATION_CHECK,                       MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0006))     \
    _macro(POST_ROM_VERIFY_IMAGE,                           MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0007))     \
    _macro(POST_ROM_VERIFY_RSA_SIGNATURE,                   MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0008))     \
    _macro(POST_ROM_VERIFY_ECDSA_SIGNATURE,                 MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x0009))     \
    _macro(POST_ROM_RUN_RECOVERY,                           MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x000A))     \
    _macro(POST_ROM_JTAG_MSG,                               MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x000B))     \
    _macro(POST_ROM_RNG_TIMEOUT,                            MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_NONE,     0x000C))     \
    \
    /* ROM fatal error post codes = these are used to provide info about previous fatal errors in boot. */                      \
    _macro(POST_ROM_FATAL_ERROR,                            MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0000))     \
    _macro(POST_ROM_FATAL_ERROR_CRYPTO,                     MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0001))     \
    _macro(POST_ROM_FATAL_ERROR_ACCESS_VIOLATION,           MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0002))     \
    _macro(POST_ROM_FATAL_ERROR_MEM_EDC,                    MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0004))     \
    _macro(POST_ROM_FATAL_ERROR_MEM_ERASE,                  MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0008))     \
    _macro(POST_ROM_FATAL_ERROR_HWCHKPT,                    MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0010))     \
    _macro(POST_ROM_FATAL_ERROR_AXI,                        MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0020))     \
    _macro(POST_ROM_FATAL_ERROR_BUS,                        MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0040))     \
    _macro(POST_ROM_FATAL_ERROR_DMB,                        MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0080))     \
    _macro(POST_ROM_FATAL_ERROR_SW,                         MAKE_POSTCODE(S_SUCCESS, STAGE_ROM, CATEGORY_HARDWARE, 0x0100))     \
    \
    /* ROM-specific failure codes */                                                                                            \
    _macro(STATUS_ROM_SP1_INVALID_IMAGE_SIZE,               MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0001))           \
    _macro(STATUS_ROM_SP1_REVOCATION_FAILED,                MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0002))           \
    _macro(STATUS_ROM_RSA_VALIDATION_FAILED,                MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0003))           \
    _macro(STATUS_ROM_ECC_VALIDATION_FAILED,                MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0004))           \
    _macro(STATUS_ROM_INVALID_BOOTSLOT,                     MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0005))           \
    _macro(STATUS_ROM_IMAGE_DIGEST_MISMATCH,                MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0006))           \
    _macro(STATUS_ROM_SP1_INVALID_HEADER_VERSION,           MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0007))           \
    _macro(STATUS_ROM_FETCH_INVALID_SLOT,                   MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0008))           \
    _macro(STATUS_ROM_REVOCATION_FAILED,                    MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0009))           \
    _macro(STATUS_ROM_FETCH_IMAGE_INVALID_SIZE,             MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000A))           \
    _macro(STATUS_ROM_CHECKPOINT_DIGEST0,                   MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000B))           \
    _macro(STATUS_ROM_CHECKPOINT_FENCE,                     MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000C))           \
    _macro(STATUS_ROM_SP1_SIZE_INVALID,                     MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000D))           \
    _macro(STATUS_ROM_READ_SLOT1ADDR,                       MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000E))           \
    _macro(STATUS_ROM_READ_SLOT2ADDR,                       MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x000F))           \
    _macro(STATUS_ROM_READ_SP1BINHEADER,                    MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0010))           \
    _macro(STATUS_ROM_READ_SP1IMAGE,                        MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0011))           \
    _macro(STATUS_ROM_READ_SP1UNSECURE,                     MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x0012))           \
    _macro(STATUS_ROM_HALT,                                 MAKE_STATUS(S_FAILURE, STAGE_ROM, CATEGORY_NONE, 0x00FF))           \
    \
    /* SP1 boot post codes  */                                                                                                  \
    _macro(POST_SP1_START,                                  MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0000))         \
    _macro(POST_SP1_FETCH_SP2_IMAGE_FLASH,                  MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0001))         \
    _macro(POST_SP1_FETCH_SP2_IMAGE_FLASH_BACKUP,           MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0002))         \
    _macro(POST_SP1_VERIFY_IMAGE,                           MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0003))         \
    _macro(POST_SP1_VERIFY_ECDSA_SIGNATURE,                 MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0004))         \
    _macro(POST_SP1_JUMP_TO_SP2,                            MAKE_POSTCODE(S_SUCCESS, STAGE_SP1, CATEGORY_NONE, 0x0005))         \
    \
    _macro(STATUS_SP1_HALT,                                 MAKE_STATUS(S_FAILURE, STAGE_SP1, CATEGORY_NONE, 0xFFFF))           \
    \
    /* SP2 boot post codes  */                                                                                                  \
    _macro(POST_SP2_START,                                  MAKE_POSTCODE(S_SUCCESS, STAGE_SP2, CATEGORY_NONE, 0x0000))         \
    _macro(POST_SP2_JUMP_TO_SPRT,                           MAKE_POSTCODE(S_SUCCESS, STAGE_SP2, CATEGORY_NONE, 0x0007))         \
    \
    /* SP2-specific failure codes */                                                                                            \
    _macro(STATUS_SP2_HALT,                                 MAKE_STATUS(S_FAILURE, STAGE_SP2, CATEGORY_NONE, 0xFFFF))           \
    \
    /* SPRT boot post codes */                                                                                                  \
    _macro(POST_SPRT_START,                                 MAKE_POSTCODE(S_SUCCESS, STAGE_SPRT, CATEGORY_NONE, 0x0000))        \
    _macro(POST_SP_FINAL,                                   MAKE_POSTCODE(S_SUCCESS, STAGE_SPRT, CATEGORY_NONE, 0x00FF))        \
    \
    /* SPRT-specific failure codes */                                                                                           \
    _macro(STATUS_SPRT_HALT,                                MAKE_STATUS(S_FAILURE, STAGE_SPRT, CATEGORY_NONE, 0xFFFF))
