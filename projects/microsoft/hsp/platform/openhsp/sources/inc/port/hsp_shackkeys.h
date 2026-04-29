/*++

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_shackkeys.h

Abstract:

    This defines the key registers in shack

Author:

    Peng Li (pengfeli)

--*/

#pragma once

//
// SHACK crypto keys
//
typedef enum _HSP_CRYPTO_REGISTER
{
    //
    // THE KEY ORDER BELOW CANNOT BE CHANGED WITHOUT ROM CHANGE
    //
    HSP_CRYPTO_REGISTER_KEY_BASE    = 0,                                            // 0
    HSP_CRYPTO_REGISTER_GLOBAL_KEY  = HSP_CRYPTO_REGISTER_KEY_BASE,                 // 0
    HSP_CRYPTO_REGISTER_DEVICE_KEY,                                                 // 1
    HSP_CRYPTO_REGISTER_FUSE_RESERVED2,                                             // 2
    HSP_CRYPTO_REGISTER_FUSE_RESERVED3,                                             // 3
    HSP_CRYPTO_REGISTER_ECC_SIGN,                                                   // 4
    HSP_CRYPTO_REGISTER_ECDH,                                                       // 5
    HSP_CRYPTO_REGISTER_HASHSTICK_RETAIL,                                           // 6
    HSP_CRYPTO_REGISTER_HASHSTICK_DEVKIT,                                           // 7
    HSP_CRYPTO_REGISTER_HASHSTICK_SHARED,                                           // 8
    HSP_CRYPTO_REGISTER_VER_INDEPENDENT_DEVICE,                                     // 9
    HSP_CRYPTO_REGISTER_VER_INDEPENDENT_GLOBAL,                                     // 10
    HSP_CRYPTO_REGISTER_RESERVED63 = 63,                                            // 63

    //
    // PCRs must be kept starting immediately after last HSP crypto register
    //
    HSP_CRYPTO_REGISTER_PCR0,                                                       // 64
    HSP_CRYPTO_REGISTER_PCR1,                                                       // 65
    HSP_CRYPTO_REGISTER_PCR2,                                                       // 66
    HSP_CRYPTO_REGISTER_PCR3,                                                       // 67

    HSP_CRYPTO_REGISTER_SHAREDRAM,                                                  // 68
    HSP_CRYPTO_REGISTER_SHAREDRAM_END = HSP_CRYPTO_REGISTER_SHAREDRAM + 16,         // 84

    //
    // We may need to send keys to other units - easeier to keep track where sending
    // by using specific "shared key" enum rather than using HSP's KSU enum.
    // Only use in SendKey function
    //
    HSP_CRYPTO_REGISTER_SHARED_KEY,                                                 // 85
    HSP_CRYPTO_REGISTER_SHARED_KEY_END = HSP_CRYPTO_REGISTER_SHARED_KEY + 63,       // 148

    //
    // Used for empty parameter, should not actually be used as key slot
    //
    HSP_CRYPTO_REGISTER_NONE,
} HSP_CRYPTO_REGISTER;
