/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsppackage.h

Abstract:

    The header file for hsppackage. Hsppackage generates packages
    containing SPRT ELF or MSFT Cores ELFs.

Author:

    Helen Zhang (helzhan)

--*/

#pragma once


typedef struct _HSP_PACKAGE_METADATA
{
    uint32_t    Magic;                              // The magic constant that identify the HSP package file
    uint32_t    Version;                            // Version of the meta data
    uint32_t    Size;                               // Total size of package in bytes. doesn't include header

    uint8_t     Type;                               // The type of the package: e.g. SPRT package or MSFT core package
    uint32_t    ElfConfigSize;                      // The number of count of Elf config

    uint32_t    DynamicDataOffset;                  // The file byte offset to the dynamic metadata
} HSP_PACKAGE_METADATA, * PHSP_PACKAGE_METADATA;


typedef struct _HSP_PACKAGE_METADATA_ELF_CONFIG
{
    uint8_t     BootStage;                          // The boot order of the ELF
                                                    // All ELF with BootStage = 1 will boot first then BootStage = 2 will get booted
                                                    // It's expected the BootStage will be in chronological order.
    uint32_t    Size;                               // The byte size of the ELF file
} HSP_PACKAGE_METADATA_ELF_CONFIG, * PHSP_PACKAGE_METADATA_ELF_CONFIG;


#define METADATA_MAGIC      0x50505348  //"HSPP"
#define METADATA_VERSION    0x31000000  //"1"
#define MAX_ELF_INPUT_SIZE  100
#define SPRT_TYPE           0x53        //"S"
#define MSFT_TYPE           0x4D        //"M"
