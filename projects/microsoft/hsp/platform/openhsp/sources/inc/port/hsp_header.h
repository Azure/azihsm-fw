/*++

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_header.h

Abstract:

    Data structure for signed binary files.

--*/

#pragma once

#include <stdint.h>
#include "hsp_addr_map.h"

#pragma pack(push,4)

// ULONG SP_GetObfVersion (struct _SP1_BIN_HEADER *);
// void SP_SetObfVersion (struct _SP1_BIN_HEADER *, int version);
#define SP_GetObfVersion(binheader, type) \
    ((binheader)->Signed.type \
        ^ (binheader)->Signed.CodePlaintextHash.AsUINT32s[3] \
        ^ (binheader)->Signed.CodePlaintextHash.AsUINT32s[7])

#define SP_SetObfVersion(binheader, type, version) \
    (binheader)->Signed.type \
        = (binheader)->Signed.CodePlaintextHash.AsUINT32s[3] \
        ^ (binheader)->Signed.CodePlaintextHash.AsUINT32s[7] \
        ^ (version);

// BuildVersion is 64bit
#define SP_CURRENT_BUILD_VERSION                    ((uint64_t)HSP_VERSION_MAJOR << 54 | \
                                                     (uint64_t)HSP_VERSION_MINOR << 44 | \
                                                     (uint64_t)HSP_VERSION_PATCH << 34 | \
                                                     (uint64_t)HSP_VERSION_BUILD)

#define SP_BIN_FORMAT_RSA_SIGNATURE_SALT_LENGTH     32

// Security Version
#define SP1_BIN_SECURITY_VERSION_CURRENT            1
#define SP2_BIN_SECURITY_VERSION_CURRENT            1

//
// Define the always-revoked canaries
//
#define SP1_BIN_SECURITY_VERSION_MAX                128
#define SP1_BIN_SECURITY_VERSION_CANARY_0           0
#define SP1_BIN_SECURITY_VERSION_CANARY_1           (SP1_BIN_SECURITY_VERSION_MAX - 1)


//
// Define current header versions
//
#define SP1_BIN_HEADER_VERSION_CURRENT              1
#define SP2_BIN_HEADER_VERSION_CURRENT              1


//
// The binary header for SP1 boot stage
//

typedef struct _SP1_BIN_HEADER
{
    uint8_t                 HeaderVersion;              // Header version
    uint8_t                 Reserved0[3];               // Reserved
    SP_ECDSA_P384_SIGNATURE EccSignature;               // ECC signature of Signed struct
    SP_RSA_SIGNATURE_4096   RsaSignature;               // RSA signature of Signed struct

    struct _SP1_SIGNED
    {
        uint32_t            ObfuscatedSecurityVersion;  // S Obfuscated SP security version number
        uint64_t            ObfuscatedBuildVersion;     // S Obfuscated SP Build Version
        uint32_t            BinarySize;                 // S Size of image, not including header
        SP_MSG_256          CodePlaintextHash;          // S A hash of plain text code that follows this header (up to Size)
    } Signed;

} SP1_BIN_HEADER, * PSP1_BIN_HEADER;



//
// The binary header for SP2 boot stage. The Sp2 uses only RSA signature
//

typedef struct _SP2_BIN_HEADER
{
    uint8_t                 HeaderVersion;              // Header version
    uint8_t                 Reserved0[3];               // Reserved
    SP_RSA_SIGNATURE_4096   RsaSignature;               // RSA signature of Signed struct

    struct _SP2_SIGNED
    {
        uint32_t            ObfuscatedSecurityVersion;  // S Obfuscated SP security version number
        uint64_t            ObfuscatedBuildVersion;     // S Obfuscated SP Build Version
        SP_MSG_256          Sp1CodePlaintextHash;       // S A hash of plain text code for Sp1
        uint32_t            BinarySize;                 // S Size of image, not including header
        SP_MSG_256          CodePlaintextHash;          // S A hash of plain text code that follows this header (up to Size)
    } Signed;

} SP2_BIN_HEADER, *PSP2_BIN_HEADER;


//
// Address table to be placed at the start of the flash file.
// Will contain information about addresses of various information in flash image.
//
// NOTE: this table is immutable - any entries already present can not be changed or removed.
// However, new entries may be added AFTER any existing entries.
//
typedef struct _SP_FLASH_ADDRESS_TABLE
{
    uint32_t    BootSlot1Sp1;                           // Address of Sp1 bootslot 1 image
    uint32_t    BootSlot2Sp1;                           // Address of Sp1 bootslot 2 image
    uint32_t    BootSlot1Sp2;                           // Address of Sp2 bootslot 1 image
    uint32_t    BootSlot2Sp2;                           // Address of Sp2 bootslot 2 image

} SP_FLASH_ADDRESS_TABLE, * PSP_FLASH_ADDRESS_TABLE;

#define SP_TABLE_ADDR       (HSP_SPXIP_DEVICE_BEGIN)

#pragma pack(pop)
