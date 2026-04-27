// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
 *
 * @file ErrorCodes.h
 *
 * Header file for Error Code.
 *
 ***********************************************************************************/
#pragma once

/***********************************************************************************
 *                                                                                  *
 * Dependencies                                                                     *
 *                                                                                  *
 ***********************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************************
 *                                                                                  *
 *  Public Constant Definitions                                                     *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Macros Definitions                                                               *
 *                                                                                  *
 ***********************************************************************************/
#define GROUP_OFFSET                                (0x18UL) //24
#define SUBGROUP_OFFSET                             (0x10UL) //16

#define GROUP_MASK                                  (0xFFUL)
#define SUBGROUP_MASK                               (0xFFUL)
#define ERR_CODE_MASK                               (0xFFFFUL)

#define GROUP_CODE(x)                               (((uint32_t)(x) & GROUP_MASK) << GROUP_OFFSET)
#define SUBGROUP_CODE(x)                            (((uint32_t)(x) & SUBGROUP_MASK) << SUBGROUP_OFFSET)
#define ERR_CODE(x)                                 ((uint32_t)(x) & ERR_CODE_MASK)
#define EXTENDED_ENUM(x)                            ((uint32_t)(x))


/// Each group of errors has it's own macro style
#define COMPOSE_GENERIC_ERROR_CODE(subgroup, code)       (EXTENDED_ENUM(GROUP_CODE(0) | SUBGROUP_CODE(subgroup) | ERR_CODE(code)))

/***********************************************************************************
 *                                                                                  *
 *  Public Data Type Definitions                                                    *
 *                                                                                  *
 ***********************************************************************************/

//-----------------------------------------------------------------------------
// System error code enumeration
//-----------------------------------------------------------------------------

//
//  Error code takes 16 bit and can have 3 fields: Group, Subgroup and Status code
//
//  Subgroups are only a convention used for organizational purposes within the group and not always required.
//  There can be no partitioning at all or a group specific partitioning.
//
//  bit[00:07] - Status code
//  bit[08:12] - Subgroup (subgroups are optional and can be group specific)
//  bit[13:15] - Group
//
// RULES
//
// 1. GLOBALLY UNIQUE ERROR CODES
//    Don't reuse error code for different meaning.
// 2. Codes are for pure definition only, DO NOT use conditional compile options.
// 3. ERROR CODES
//    xx00h   : DO NOT USE except 0000h for no error.
//    xx01h   : Usually reserved for general failure.
//
//    Group 0 : Reserved for system generic error codes.
//
/// @brief System error codes
//lint -esym(849, Error_t::cEcAerWriteToInvalidDoorbell) : same enumerator value as another
typedef enum Error_t
{
    /// Group 0-4: Generic Error Codes (0..4)
    /// Subgroup 0: General System Errors - All standard errors such as Success/Failure 0x0000 0000 - 0x0000 FFFF
    cEcNoError                          = COMPOSE_GENERIC_ERROR_CODE(0, 0x0000),    ///< No error, OK or Passed
    cEcInProgress                       = COMPOSE_GENERIC_ERROR_CODE(0, 0x0001),    ///< No error, request is in progress (not complete yet)
    cEcError                            = COMPOSE_GENERIC_ERROR_CODE(0, 0x0002),    ///< Generic Error, Not OK or Failed
    cEcTimeOut                          = COMPOSE_GENERIC_ERROR_CODE(0, 0x0003),    ///< Time out error
    cEcNotReady                         = COMPOSE_GENERIC_ERROR_CODE(0, 0x0004),    ///< Not Ready
    cEcAssertFail                       = COMPOSE_GENERIC_ERROR_CODE(0, 0x0005),    ///< Firmware Assertion failed
    cEcInvalidCommand                   = COMPOSE_GENERIC_ERROR_CODE(0, 0x0006),    ///< Invalid command code
    cEcMissingParam                     = COMPOSE_GENERIC_ERROR_CODE(0, 0x0007),    ///< Missing parameters
    cEcInvalidField                     = COMPOSE_GENERIC_ERROR_CODE(0, 0x0008),    ///< Invalid command field

} Error_t;

#ifdef __cplusplus
}
#endif
