/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spcryptotypes.h

Abstract:

    Common crypto types and defines used across HSP

Author:

    Peng Li (pengfeli)

--*/

#pragma once

#include <stdint.h>


#define IN_DWORDS(x)                           ((x) / 4)
#define IN_BYTES(x)                            (x)
#define IN_BITS(x)                             ((x)*8)

#define SP_IV_SIZE                             0x10
#define SP_PSS_SALT_SIZE                       0x20
#define SP_PSS_SALT_SHA1_SIZE                  0x14
#define SP_AES_KEY_SIZE                        0x20
#define SP_AES_BLOCK_SIZE                      0x10
#define SP_SHA_BLOCK_SIZE                      0x40
#define SP_MSG_64_SIZE                         0x08
#define SP_MSG_128_SIZE                        0x10
#define SP_MSG_160_SIZE                        0x14
#define SP_MSG_256_SIZE                        0x20
#define SP_MSG_320_SIZE                        0x28
#define SP_MSG_384_SIZE                        0x30
#define SP_MSG_512_SIZE                        0x40
#define SP_MSG_521_SIZE                        0x42
#define SP_MSG_521_ALIGNED_SIZE                0x44
#define SP_MSG_1024_SIZE                       0x80
#define SP_MSG_2048_SIZE                       0x100
#define SP_MSG_3072_SIZE                       0x180
#define SP_MSG_4096_SIZE                       0x200
#define SP_MSG_8192_SIZE                       0x400

#define SP_ECDSA_P256_PUBLIC_KEY_SIZE          0x40
#define SP_ECDSA_P256_PRIVATE_KEY_SIZE         0x60
#define SP_ECDSA_P256_SIGNATURE_SIZE           0x40

#define SP_ECDSA_P384_PUBLIC_KEY_SIZE          0x60
#define SP_ECDSA_P384_PRIVATE_KEY_SIZE         0x90
#define SP_ECDSA_P384_SIGNATURE_SIZE           0x60

#define SP_ECDSA_P521_PUBLIC_KEY_SIZE          0x84
#define SP_ECDSA_P521_PRIVATE_KEY_SIZE         0xc6
#define SP_ECDSA_P521_SIGNATURE_SIZE           0x84

#define SP_ECDSA_P521_ALIGNED_PUBLIC_KEY_SIZE  0x88
#define SP_ECDSA_P521_ALIGNED_PRIVATE_KEY_SIZE 0xcc
#define SP_ECDSA_P521_ALIGNED_SIGNATURE_SIZE   0x88

#define FLAG_SYSTEM_UPDATE_LICENSE             0x01

#pragma pack(push, 1)
//
// stores a 64-bit message
//
typedef union _SP_MSG_64
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_64_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_64_SIZE)];
} SP_MSG_64, *PSP_MSG_64;

//
// stores a 128-bit message
//
typedef union _SP_MSG_128
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_128_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_128_SIZE)];
} SP_MSG_128, *PSP_MSG_128;

typedef const SP_MSG_128* PCSP_MSG_128;

//
// stores a 160-bit message
//
typedef union _SP_MSG_160
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_160_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_160_SIZE)];
} SP_MSG_160, *PSP_MSG_160;

typedef const SP_MSG_160* PCSP_MSG_160;

//
// stores a 256-bit message
//
typedef union _SP_MSG_256
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_256_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_256_SIZE)];
} SP_MSG_256, *PSP_MSG_256;

typedef const SP_MSG_256* PCSP_MSG_256;

//
// stores a 320-bit message
//
typedef union _SP_MSG_320
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_320_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_320_SIZE)];
} SP_MSG_320, *PSP_MSG_320;

typedef const SP_MSG_320* PCSP_MSG_320;

//
// stores a 384-bit message
//
typedef union _SP_MSG_384
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_384_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_384_SIZE)];
} SP_MSG_384, *PSP_MSG_384;

typedef const SP_MSG_384* PCSP_MSG_384;

//
// stores a 512-bit message
//
typedef union _SP_MSG_512
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_512_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_512_SIZE)];
} SP_MSG_512, *PSP_MSG_512;

typedef const SP_MSG_512* PCSP_MSG_512;

//
// stores a 521-bit message
//
typedef union _SP_MSG_521
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_521_SIZE)];
} SP_MSG_521, *PSP_MSG_521;

//
// stores a word-aligned 521-bit message
//
typedef union _SP_MSG_521_ALIGNED
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_521_ALIGNED_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_521_ALIGNED_SIZE)];
    struct
    {
        SP_MSG_521 Data;
        uint8_t Pad[IN_BYTES(SP_MSG_521_ALIGNED_SIZE - SP_MSG_521_SIZE)];
    } Parts;
} SP_MSG_521_ALIGNED, *PSP_MSG_521_ALIGNED;

typedef const SP_MSG_512* PCSP_MSG_512;

//
// stores a 1024-bit message
//
typedef union _SP_MSG_1024
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_1024_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_1024_SIZE)];
} SP_MSG_1024, *PSP_MSG_1024;

typedef const SP_MSG_1024* PCSP_MSG_1024;

//
// stores a 2048-bit message
//
typedef union _SP_MSG_2048
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_2048_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_2048_SIZE)];
} SP_MSG_2048, *PSP_MSG_2048;

typedef const SP_MSG_2048* PCSP_MSG_2048;

//
// stores a 3072-bit message
//
typedef union _SP_MSG_3072
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_3072_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_3072_SIZE)];
} SP_MSG_3072, *PSP_MSG_3072;

typedef const SP_MSG_3072* PCSP_MSG_3072;

//
// stores a 4096-bit message
//
typedef union _SP_MSG_4096
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_4096_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_4096_SIZE)];
} SP_MSG_4096, *PSP_MSG_4096;

typedef const SP_MSG_4096* PCSP_MSG_4096;

//
// Format for RSA Keys
//
typedef union _SP_RSA_PRIVATE_EXPONENT_2048
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_2048_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_2048_SIZE)];
} SP_RSA_PRIVATE_EXPONENT_2048, *PSP_RSA_PRIVATE_EXPONENT_2048;

typedef union _SP_RSA_PUBLIC_MODULUS_4096
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_4096_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_4096_SIZE)];
} SP_RSA_PUBLIC_MODULUS_4096, *PSP_RSA_PUBLIC_MODULUS_4096;

typedef union _SP_RSA_SIGNATURE_2048
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_2048_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_2048_SIZE)];
} SP_RSA_SIGNATURE_2048, *PSP_RSA_SIGNATURE_2048;

typedef union _SP_RSA_SIGNATURE_3072
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_3072_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_3072_SIZE)];
} SP_RSA_SIGNATURE_3072, *PSP_RSA_SIGNATURE_3072;

typedef union _SP_RSA_SIGNATURE_4096
{
    uint8_t AsBytes[IN_BYTES(SP_MSG_4096_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_MSG_4096_SIZE)];
} SP_RSA_SIGNATURE_4096, *PSP_RSA_SIGNATURE_4096;

//
// Format for ECDSA_P256 keys
//
typedef union _SP_ECDSA_P256_PUBLIC
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P256_PUBLIC_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P256_PUBLIC_KEY_SIZE)];
    struct
    {
        SP_MSG_256 X;
        SP_MSG_256 Y;
    } Parts;
} SP_ECDSA_P256_PUBLIC, *PSP_ECDSA_P256_PUBLIC;

typedef union _SP_ECDSA_P256_PRIVATE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P256_PRIVATE_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P256_PRIVATE_KEY_SIZE)];
    struct
    {
        SP_ECDSA_P256_PUBLIC Public;
        SP_MSG_256 Private;
    } Parts;
} SP_ECDSA_P256_PRIVATE, *PSP_ECDSA_P256_PRIVATE;

typedef union _SP_ECDSA_P256_SIGNATURE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P256_SIGNATURE_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P256_SIGNATURE_SIZE)];
    struct
    {
        SP_MSG_256 R;
        SP_MSG_256 S;
    } Parts;
} SP_ECDSA_P256_SIGNATURE, *PSP_ECDSA_P256_SIGNATURE;

//
// Format for ECDSA_P384 keys
//
typedef union _SP_ECDSA_P384_PUBLIC
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P384_PUBLIC_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P384_PUBLIC_KEY_SIZE)];
    struct
    {
        SP_MSG_384 X;
        SP_MSG_384 Y;
    } Parts;
} SP_ECDSA_P384_PUBLIC, *PSP_ECDSA_P384_PUBLIC;

typedef union _SP_ECDSA_P384_PRIVATE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P384_PRIVATE_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P384_PRIVATE_KEY_SIZE)];
    struct
    {
        SP_ECDSA_P384_PUBLIC Public;
        SP_MSG_384 Private;
    } Parts;
} SP_ECDSA_P384_PRIVATE, *PSP_ECDSA_P384_PRIVATE;

typedef union _SP_ECDSA_P384_SIGNATURE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P384_SIGNATURE_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P384_SIGNATURE_SIZE)];
    struct
    {
        SP_MSG_384 R;
        SP_MSG_384 S;
    } Parts;
} SP_ECDSA_P384_SIGNATURE, *PSP_ECDSA_P384_SIGNATURE;

//
// Format for ECDSA_P521 keys
//
typedef union _SP_ECDSA_P521_PUBLIC
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_PUBLIC_KEY_SIZE)];
    struct
    {
        SP_MSG_521 X;
        SP_MSG_521 Y;
    } Parts;
} SP_ECDSA_P521_PUBLIC, *PSP_ECDSA_P521_PUBLIC;

typedef union _SP_ECDSA_P521_PRIVATE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_PRIVATE_KEY_SIZE)];
    struct
    {
        SP_ECDSA_P521_PUBLIC Public;
        SP_MSG_521 Private;
    } Parts;
} SP_ECDSA_P521_PRIVATE, *PSP_ECDSA_P521_PRIVATE;

typedef union _SP_ECDSA_P521_SIGNATURE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_SIGNATURE_SIZE)];
    struct
    {
        SP_MSG_521 R;
        SP_MSG_521 S;
    } Parts;
} SP_ECDSA_P521_SIGNATURE, *PSP_ECDSA_P521_SIGNATURE;

//
// Word-aligned format for ECDSA_P521 keys necessary for interfacing with HW components.
//
typedef union _SP_ECDSA_P521_ALIGNED_PUBLIC
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_ALIGNED_PUBLIC_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P521_ALIGNED_PUBLIC_KEY_SIZE)];
    struct
    {
        SP_MSG_521_ALIGNED X;
        SP_MSG_521_ALIGNED Y;
    } Parts;
} SP_ECDSA_P521_ALIGNED_PUBLIC, *PSP_ECDSA_P521_ALIGNED_PUBLIC;

typedef union _SP_ECDSA_P521_ALIGNED_PRIVATE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_ALIGNED_PRIVATE_KEY_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P521_ALIGNED_PRIVATE_KEY_SIZE)];
    struct
    {
        SP_ECDSA_P521_ALIGNED_PUBLIC Public;
        SP_MSG_521_ALIGNED Private;
    } Parts;
} SP_ECDSA_P521_ALIGNED_PRIVATE, *PSP_ECDSA_P521_ALIGNED_PRIVATE;

typedef union _SP_ECDSA_P521_ALIGNED_SIGNATURE
{
    uint8_t AsBytes[IN_BYTES(SP_ECDSA_P521_ALIGNED_SIGNATURE_SIZE)];
    uint32_t AsUINT32s[IN_DWORDS(SP_ECDSA_P521_ALIGNED_SIGNATURE_SIZE)];
    struct
    {
        SP_MSG_521_ALIGNED R;
        SP_MSG_521_ALIGNED S;
    } Parts;
} SP_ECDSA_P521_ALIGNED_SIGNATURE, *PSP_ECDSA_P521_ALIGNED_SIGNATURE;
#pragma pack(pop)
