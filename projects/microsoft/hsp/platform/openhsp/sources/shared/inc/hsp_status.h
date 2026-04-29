/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_status.h

Abstract:

    This file contains shared library specific status codes. Only the
    shared library error codes should be added here.

    This file is included from hsp/splibs/inc/spstatus.h

Author:

    Navin Pai (navinp)

--*/

#pragma once

#define CATEGORY_DEBUG_CERT (CATEGORY_LIB_SHARED + 1)
#define CATEGORY_MINCRYPL   (CATEGORY_LIB_SHARED + 2)
#define CATEGORY_ELFPARSER  (CATEGORY_LIB_SHARED + 3)


//
// Status list - Sorted by STAGE, CATEGORY, CODE
//
#define HSP_STATUS_LIST_LIBS(_macro)                                        \
                                                                            \
    /* shared/debugcert library failures */                                 \
    _macro(STATUS_CERT_PARSE_ERROR_KEY,                                     \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0001)) \
    _macro(STATUS_CERT_PARSE_ERROR_VERSION,                                 \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0002)) \
    _macro(STATUS_CERT_PARSE_ERROR_CERTID,                                  \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0003)) \
    _macro(STATUS_CERT_PARSE_ERROR_SIGNATURE,                               \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0004)) \
    _macro(STATUS_CERT_PARSE_ERROR_SIZE,                                    \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0005)) \
    _macro(STATUS_CERT_PARSE_ERROR_SECURITY_STATE,                          \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0006)) \
    _macro(STATUS_CERT_PARSE_ERROR_EXPIRATION,                              \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0007)) \
    _macro(STATUS_CERT_PARSE_ERROR_CERT_ID_TYPE,                            \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_DEBUG_CERT, 0x0008)) \
                                                                            \
    /* shared/elfparser library errors */                                   \
    _macro(STATUS_ELF_INVALID_HEADER,                                       \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0001))  \
    _macro(STATUS_ELF_SIZE_OUT_OF_BOUND,                                    \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0002))  \
    _macro(STATUS_ELF_SIZE_NOT_EXPECTED,                                    \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0003))  \
    _macro(STATUS_ELF_INVALID_PROGRAM_HEADER,                               \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0004))  \
    _macro(STATUS_ELF_MISSING_SECTIONS,                                     \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0005))  \
    _macro(STATUS_ELF_MISALIGNED_POINTER,                                   \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0x0006))  \
    _macro(STATUS_ELF_NO_MORE_PROGRAM_HEADERS,                              \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_ELFPARSER, 0X0007))  \
                                                                            \
    /* shared/mincrypl failures */                                          \
    _macro(STATUS_INVALID_IMAGE_HASH,                                       \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_MINCRYPL, 0x0001))   \
    _macro(STATUS_IMAGE_CERT_EXPIRED,                                       \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_MINCRYPL, 0x0002))   \
    _macro(STATUS_IMAGE_CERT_REVOKED,                                       \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_MINCRYPL, 0x0003))   \
    _macro(STATUS_ERROR_RSA_PKCS_SHA256,                                    \
           MAKE_STATUS(S_FAILURE, STAGE_NONE, CATEGORY_MINCRYPL, 0x0004))

//
// need two new line after last "\"
//
