/*++

Description:

    This file contains the address map for the Hsp Address space.

--*/

#pragma once

//------------------------------------------------------------------------------

#define HSP_DEBUG_DEVICE_BEGIN              0x00000000
#define HSP_DEBUG_DEVICE_SIZE               0x00001000
#define HSP_DEBUG_DEVICE_END                (HSP_DEBUG_DEVICE_BEGIN + HSP_DEBUG_DEVICE_SIZE)

#define HSP_CLIC_DEVICE_BEGIN               0x02000000
#define HSP_CLIC_DEVICE_SIZE                0x01000000
#define HSP_CLIC_DEVICE_END                 (HSP_CLIC_DEVICE_BEGIN + HSP_CLIC_DEVICE_SIZE)

#define HSP_SPROM_BEGIN                     0x20000000
#define HSP_SPROM_SIZE                      0x00010000  // 64 KB
#define HSP_SPROM_END                       (HSP_SPROM_BEGIN + HSP_SPROM_SIZE)

#define HSP_SPROM_SECRETS_BEGIN             0x2000F000
#define HSP_SPROM_SECRETS_SIZE              0x00001000  // 4 KB
#define HSP_SPROM_SECRETS_END               HSP_SPROM_END

//
// Split up SPIRAM into sections for Sp1, Sp2, SpRt
//
#define HSP_SPIRAM_BEGIN                    0x20080000
#define HSP_SPIRAM_SIZE                     0x00080000  // 512 KB
#define HSP_SPIRAM_END                      (HSP_SPIRAM_BEGIN + HSP_SPIRAM_SIZE)

#define HSP_SP1IRAM_BEGIN                   0x20080000
#define HSP_SP1IRAM_SIZE                    0x00020000  // 128 KB
#define HSP_SP1IRAM_END                     (HSP_SP1IRAM_BEGIN + HSP_SP1IRAM_SIZE)

#define HSP_SP2IRAM_BEGIN                   0x200E0000
#define HSP_SP2IRAM_SIZE                    0x00020000  // 128 KB
#define HSP_SP2IRAM_END                     (HSP_SP2IRAM_BEGIN + HSP_SP2IRAM_SIZE)

#define HSP_SPRTIRAM_BEGIN                  0x20080000
#define HSP_SPRTIRAM_SIZE                   0x00060000  // 384 KB
#define HSP_SPRTIRAM_END                    (HSP_SPRTIRAM_BEGIN + HSP_SPRTIRAM_SIZE)

//
// Split up SPDRAM into sections for machine and user mode,
// as well as a shared section.
//
#define HSP_SPDRAM_BEGIN                    0x20100000
#define HSP_SPDRAM_SIZE                     0x00008000  // 32 KB
#define HSP_SPDRAM_END                      (HSP_SPDRAM_BEGIN + HSP_SPDRAM_SIZE)

#define HSP_SPDRAM_MACHINE_BEGIN            0x20100000
#define HSP_SPDRAM_MACHINE_SIZE             0x00004000  // 16 KB
#define HSP_SPDRAM_MACHINE_END              (HSP_SPDRAM_MACHINE_BEGIN + HSP_SPDRAM_MACHINE_SIZE)

#define HSP_SPDRAM_USER_BEGIN               0x20104000
#define HSP_SPDRAM_USER_SIZE                0x00002000  // 8 KB
#define HSP_SPDRAM_USER_END                 (HSP_SPDRAM_USER_BEGIN + HSP_SPDRAM_USER_SIZE)

#define HSP_SPDRAM_SHARED_BEGIN             0x20106000
#define HSP_SPDRAM_SHARED_SIZE              0x00001000  // 4 KB
#define HSP_SPDRAM_SHARED_END               (HSP_SPDRAM_SHARED_BEGIN + HSP_SPDRAM_SHARED_SIZE)

#define HSP_SPDRAM_GLOBAL_BEGIN             0x20107000
#define HSP_SPDRAM_GLOBAL_SIZE              0x00001000  // 4 KB
#define HSP_SPDRAM_GLOBAL_END               (HSP_SPDRAM_GLOBAL_BEGIN + HSP_SPDRAM_GLOBAL_SIZE)


#define HSP_SPXIP_DEVICE_BEGIN              0x87000000
#define HSP_SPXIP_DEVICE_SIZE               0x00800000  // 8 MB
#define HSP_SPXIP_DEVICE_END                (HSP_SPXIP_DEVICE_BEGIN + HSP_SPXIP_DEVICE_SIZE)

#define HSP_CREG_DEVICE_BEGIN               0x8F000000
#define HSP_CREG_DEVICE_SIZE                0x00020000
#define HSP_CREG_DEVICE_END                 (HSP_CREG_DEVICE_BEGIN + HSP_CREG_DEVICE_SIZE)

#define HSP_SHAREDRAM_BEGIN                 0x8F020000
#define HSP_SHAREDRAM_SIZE                  0x00002000  // 8 KB
#define HSP_SHAREDRAM_END                   (HSP_SHAREDRAM_BEGIN + HSP_SHAREDRAM_SIZE)

#define HSP_GFC_DEVICE_BEGIN                0x8F060000
#define HSP_GFC_DEVICE_SIZE                 0x00010000
#define HSP_GFC_DEVICE_END                  (HSP_GFC_DEVICE_BEGIN + HSP_GFC_DEVICE_SIZE)

#define HSP_CRYPTO_DEVICE_BEGIN             0x8F070000
#define HSP_CRYPTO_DEVICE_SIZE              0x00060000
#define HSP_CRYPTO_DEVICE_END               (HSP_CRYPTO_DEVICE_BEGIN + HSP_CRYPTO_DEVICE_SIZE)

#define HSP_AES_DEVICE_BEGIN                0x8F070000
#define HSP_AES_DEVICE_SIZE                 0x00010000
#define HSP_AES_DEVICE_END                  (HSP_AES_DEVICE_BEGIN + HSP_AES_DEVICE_SIZE)

#define HSP_PKA_DEVICE_BEGIN                0x8F080000
#define HSP_PKA_DEVICE_SIZE                 0x00010000
#define HSP_PKA_DEVICE_END                  (HSP_PKA_DEVICE_BEGIN + HSP_PKA_DEVICE_SIZE)

#define HSP_SHA_DEVICE_BEGIN                0x8F090000
#define HSP_SHA_DEVICE_SIZE                 0x00010000
#define HSP_SHA_DEVICE_END                  (HSP_SHA_DEVICE_BEGIN + HSP_SHA_DEVICE_SIZE)

#define HSP_RNG_DEVICE_BEGIN                0x8F0A0000
#define HSP_RNG_DEVICE_SIZE                 0x00010000
#define HSP_RNG_DEVICE_END                  (HSP_RNG_DEVICE_BEGIN + HSP_RNG_DEVICE_SIZE)

#define HSP_CCS_DEVICE_BEGIN                0x8F0B0000
#define HSP_CCS_DEVICE_SIZE                 0x00010000
#define HSP_CCS_DEVICE_END                  (HSP_CCS_DEVICE_BEGIN + HSP_CCS_DEVICE_SIZE)

#define HSP_KSU_DEVICE_BEGIN                0x8F0C0000
#define HSP_KSU_DEVICE_SIZE                 0x00010000
#define HSP_KSU_DEVICE_END                  (HSP_KSU_DEVICE_BEGIN + HSP_KSU_DEVICE_SIZE)

#define HSP_DMB_CFG_DEVICE_BEGIN            0x8F0F0000
#define HSP_DMB_CFG_DEVICE_SIZE             0x00001000
#define HSP_DMB_CFG_DEVICE_END              (HSP_DMB_CFG_DEVICE_BEGIN + HSP_DMB_CFG_DEVICE_SIZE)

#define HSP_DMB_DEVICE_BEGIN                0x90000000
#define HSP_DMB_DEVICE_SIZE                 0x70000000
#define HSP_DMB_DEVICE_END                  ((uint32_t)HSP_DMB_DEVICE_BEGIN + HSP_DMB_DEVICE_SIZE)
