/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spstatus.h

Abstract:

    This file contains macros and enums for Hsp return status convention.
    This allows for consistent handling of success and failure cases across Hsp.

    The status code Hsp libraries use is derived from the HRESULT format,
    defined as follows:

    HSP_STATUS:

     3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    +-+-+-------+-------------------+-------------------------------+
    |S|P| Stage |     Category      |             Code              |
    +-+-+-------+-------------------+-------------------------------+

    Where:

        S (success) [31]

            Indicates overall success/failure.

            0 - Success
            1 - Failure

        P (post code) [30]

            Indicates whether the specific status is an informational post code,
            rather than a return status value.

            0 - Is return status
            1 - Is post code

        Stage [29:26]

            Defines the associated stage of the HSP_STATUS.

            0000 - No associated stage
            0001 - Stage ROM
            0010 - Stage Sp1
            0011 - Stage Sp2
            0100 - Reserved
            ...
            0111 - Reserved
            1111 - Stage Sp Firmware

        Category [25:16]

            The Category defining where the status originated. Values defined
            below.

        Code [15:0]

            Status code for the Category.  Custom per Category in table below.

Author:

    Navin Pai (navinp)
    Timothy Prinz (tiprinz)

--*/

#pragma once
#include "inc/port/hsp_status.h"
#include "shared/inc/hsp_status.h"

// clang-format off
//
// ---------------------------------------------------------------------------------------
// Defines for basic status information and categories
// ---------------------------------------------------------------------------------------

#define S_SUCCESS                               0x0
#define S_FAILURE                               0x1

#define P_STATUS                                0x0
#define P_POSTCODE                              0x1

#define STAGE_NONE                              0x0
#define STAGE_ROM                               0x1
#define STAGE_SP1                               0x2
#define STAGE_SP2                               0x3
#define STAGE_SPRT                              0xF

#define CATEGORY_NONE                           0x000
#define CATEGORY_INTERNAL                       0x001
#define CATEGORY_CRYPTO_HW                      0x002
#define CATEGORY_CRYPTO_SW                      0x003
#define CATEGORY_SPLOADER                       0x004
#define CATEGORY_HARDWARE                       0x005

#define CATEGORY_LIB_SHARED                     0X100   // All shared library status codes should start here
#define CATEGORY_LIB_APPS                       0x200   // All app library status codes should start here
#define CATEGORY_MAX                            0x3FF


#define MAKE_STATUS(sts, stg, cat, code)        ((uint32_t)(((uint32_t)(sts      & 0x1)   <<31) | \
                                                            ((uint32_t)(P_STATUS & 0x1)   <<30) | \
                                                            ((uint32_t)(stg      & 0xF)   <<26) | \
                                                            ((uint32_t)(cat      & 0x3FF) <<16) | \
                                                            ((uint32_t)(code     & 0xFFFF))))

#define MAKE_POSTCODE(sts, stg, cat, code)      ((uint32_t)(((uint32_t)(sts         & 0x1)   <<31) | \
                                                            ((uint32_t)(P_POSTCODE  & 0x1)   <<30) | \
                                                            ((uint32_t)(stg         & 0xF)   <<26) | \
                                                            ((uint32_t)(cat         & 0x3FF) <<16) | \
                                                            ((uint32_t)(code        & 0xFFFF))))

//
//
// Status list - Sorted by STAGE, CATEGORY, CODE
//
// Guidelines for adding new status codes
// There is only one success status : STATUS_SUCCESS. All other status codes should be S_FAILURE
// All post codes are S_SUCCESS. There should be no post codes with S_FAILURE.
//
// The libraries should never post code but should either return STATUS_SUCCESS OR
// an error code. Only add error codes for unique errors. Do not translate/hide original errors,
// rather bubble up the original error.
//
#define HSP_STATUS_LIST(_macro) \
    \
    /* generic errors */                                                                                                        \
    _macro(STATUS_SUCCESS,                                  MAKE_STATUS(S_SUCCESS, STAGE_NONE, CATEGORY_NONE,      0x0001))     \
    _macro(STATUS_UNHANDLE_EXCEPTION,                       MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_NONE,      0x0002))     \
    _macro(STATUS_UNHANDLE_INTERRUPT,                       MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_NONE,      0x0003))     \
    _macro(STATUS_BUFFER_OVERFLOW,                          MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_NONE,      0x0004))     \
    _macro(STATUS_TIMEDOUT,                                 MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_NONE,      0x0005))     \
    _macro(STATUS_UNHANDLED_SYSCALL,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_NONE,      0x0006))     \
    \
    /* splibs/hspcore errors */                                                                                                 \
    _macro(STATUS_MAILBOX_NOT_EMPTY,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0001))      \
    _macro(STATUS_MAILBOX_NOT_FULL,                         MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0002))      \
    _macro(STATUS_INVALID_ARGUMENT,                         MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0003))      \
    _macro(STATUS_DMB_ACQUISITON_FAULT,                     MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0004))      \
    _macro(STATUS_DMB_NOT_FOUND,                            MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0005))      \
    _macro(STATUS_MAILBOX_NOT_VALID,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0006))      \
    _macro(STATUS_MAILBOX_FLUSH_PENDING,                    MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_INTERNAL, 0x0007))      \
    \
    _macro(STATUS_GFC_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0000))     \
    _macro(STATUS_GFC_ERROR_CMD,                            MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0004))     \
    _macro(STATUS_GFC_ERROR_APB,                            MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0008))     \
    _macro(STATUS_GFC_ERROR_ECC,                            MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0010))     \
    _macro(STATUS_GFC_ERROR_NOT_OWNER,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0020))     \
    _macro(STATUS_GFC_ERROR_SLOT_NOT_ZERO,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0040))     \
    _macro(STATUS_GFC_ERROR_BLANK_CHECK_FAIL,               MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x0080))     \
    _macro(STATUS_GFC_ERROR_USER_CMD,                       MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x00FF))     \
    \
    _macro(STATUS_AES_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1000))     \
    _macro(STATUS_AES_CMD_ERROR_PARAMETER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1001))     \
    _macro(STATUS_AES_CMD_ERROR_CMD,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1004))     \
    _macro(STATUS_AES_CMD_ERROR_BUS,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1008))     \
    _macro(STATUS_AES_CMD_ERROR_FAULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1010))     \
    _macro(STATUS_AES_CMD_ERROR_NOT_OWNER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x1020))     \
    \
    _macro(STATUS_CCS_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2000))     \
    _macro(STATUS_CCS_CMD_ERROR_PARAMETER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2001))     \
    _macro(STATUS_CCS_CMD_ERROR_CMD,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2004))     \
    _macro(STATUS_CCS_CMD_ERROR_BUS,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2008))     \
    _macro(STATUS_CCS_CMD_ERROR_FAULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2010))     \
    _macro(STATUS_CCS_CMD_ERROR_NOT_OWNER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x2020))     \
    \
    _macro(STATUS_PKA_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3000))     \
    _macro(STATUS_PKA_CMD_ERROR_PARAMETER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3001))     \
    _macro(STATUS_PKA_CMD_ERROR_CMD,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3004))     \
    _macro(STATUS_PKA_CMD_ERROR_BUS,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3008))     \
    _macro(STATUS_PKA_CMD_ERROR_FAULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3010))     \
    _macro(STATUS_PKA_CMD_ERROR_NOT_OWNER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x3020))     \
    _macro(STATUS_PKA_ECDSA_NOT_VERIFIED,                   MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x30A2))     \
    _macro(STATUS_PKA_INFINITE_RESULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x30A3))     \
    \
    _macro(STATUS_SHA_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4000))     \
    _macro(STATUS_SHA_CMD_ERROR_PARAMETER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4001))     \
    _macro(STATUS_SHA_CMD_ERROR_CMD,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4004))     \
    _macro(STATUS_SHA_CMD_ERROR_BUS,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4008))     \
    _macro(STATUS_SHA_CMD_ERROR_FAULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4010))     \
    _macro(STATUS_SHA_CMD_ERROR_NOT_OWNER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4020))     \
    _macro(STATUS_SHA_CMD_ERROR_DIGEST_MATCH,               MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x4040))     \
    \
    _macro(STATUS_RNG_ERROR,                                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5000))     \
    _macro(STATUS_RNG_CMD_ERROR_PARAMETER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5001))     \
    _macro(STATUS_RNG_CMD_ERROR_CMD,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5004))     \
    _macro(STATUS_RNG_CMD_ERROR_BUS,                        MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5008))     \
    _macro(STATUS_RNG_CMD_ERROR_FAULT,                      MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5010))     \
    _macro(STATUS_RNG_CMD_ERROR_NOT_OWNER,                  MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x5020))     \
    _macro(STATUS_RNG_FAULT_THRESHOLD_REACHED,              MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x50A2))     \
    _macro(STATUS_RNG_A0BYPASS_TIMEOUT,                     MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_HW, 0x50B2))     \
    \
    /* Crypto SW failures */                                                                                                    \
    _macro(STATUS_CRYPTO_SW_ERROR_PARAMETER,                MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_SW, 0x0001))     \
    _macro(STATUS_CRYPTO_SW_ERROR_RSA_PSS_SHA256,           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_CRYPTO_SW, 0x0002))     \
    \
    /* splibs/hsploader library errors. Following was mis-indexed but unfortunately it is now baked in A0.ROM */                                                                                       \
    _macro(STATUS_SPLOADER_INVALID_HEADER_SIZE,             MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_SPLOADER,  0x0008))     \
    \
    /* shared library error codes */                                                                                            \
    HSP_STATUS_LIST_LIBS(_macro)                                                                                                \
    \
    /* Application specific status codes */                                                                                     \
    HSP_STATUS_LIST_APPS(_macro)                                                                                                \



// ---------------------------------------------------------------------------------------
// Master enum list of HSP_STATUS codes
// ---------------------------------------------------------------------------------------

#define HSP_STATUS_ENUM_ENTRY(name, value)              name = value,

typedef enum _HSP_STATUS
{
    HSP_STATUS_LIST(HSP_STATUS_ENUM_ENTRY)
} HSP_STATUS;


// ---------------------------------------------------------------------------------------
// Macros for success/failure checking
// ---------------------------------------------------------------------------------------

//
// Avoid redefinition for any builds including winerror libraries.
//
#ifdef SUCCEEDED
#undef SUCCEEDED
#endif

#ifdef FAILED
#undef FAILED
#endif

#define SP_SUCCEEDED(x)                     ((x) == STATUS_SUCCESS)
#define SP_FAILED(x)                        (!SP_SUCCEEDED(x))

#define RETURN_STATUS_IF_FAILED(x)          { HSP_STATUS _status = (x); \
                                              if (UNLIKELY(SP_FAILED(_status))) \
                                              { return _status; } }

// clang-format on
