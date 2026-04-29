/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hspcore.h

Abstract:

    Main header file for hardware-assisted crypto, containing necessary includes
    and function declarations for crypto operations.

Author:

    Navin Pai (navinp)
    Timothy Prinz (tiprinz)

--*/

#pragma once


#include "splibs/hsprt/hsprt.h"
#include "splibs/inc/spcryptotypes.h"
#include "splibs/inc/spchkptdefs.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/spregs.h"
#include "splibs/inc/spflashconfig.h"

//
// The following set of files are defined by each project that uses hspcore.h
//
#include "inc/port/hspmailbox.h"
#include "inc/port/hsp_fuses.h"
#include "inc/port/hsp_shackkeys.h"

#include "regrw.h"
#include "uart.h"
#include "spglobal.h"
#include "chkpt.h"
#include "postcode.h"
#include "fusectrl.h"
#include "dmb.h"
#include "timercommon.h"

//
// HSP scratch space:
// Shared RAM will be the only accessible RAM for all crypto engines including
// CCS. Below is how we partition Shared RAM for different operations:
//
// Commands:            00..40    HSP_CCS_COMMAND_ADDR
// Data for CCS:        40+       HSP_CCS_SCRATCH_ADDR
// AES IV:              40..72    HSP_CCS_AES_IV_ADDR
// SHA IV and digest:   72..104   HSP_CCS_RESULT_SCRATCH
// Data for AES/SHA:    128+      HSP_CCS_DATA_BEGIN    (Make this 64-byte
// aligned) PKA Keys (modexp):   40..72    HSP_CCS_SCRATCH_ADDR PKA Data
// (modexp):   72+       HSP_CCS_SCRATCH_ADDR + 1
//

#define HSP_CCS_COMMAND_ADDR HSP_SHAREDRAM_BEGIN
#define HSP_CCS_COMMAND_SIZE sizeof(HSP_CMD_CCS)
#define HSP_CCS_SCRATCH_ADDR (HSP_SHAREDRAM_BEGIN + HSP_CCS_COMMAND_SIZE)
#define HSP_CCS_AES_IV_ADDR  (HSP_SHAREDRAM_BEGIN + HSP_CCS_COMMAND_SIZE)
#define HSP_CCS_RESULT_SCRATCH \
    (HSP_SHAREDRAM_BEGIN + HSP_CCS_COMMAND_SIZE + SP_MSG_256_SIZE)
#define HSP_CCS_DATA_BEGIN             (HSP_SHAREDRAM_BEGIN + 4 * SP_MSG_256_SIZE)
#define HSP_CCS_DATA_SIZE              (HSP_SHAREDRAM_SIZE - 4 * SP_MSG_256_SIZE)

//
// Define algorithm key lengths
//
#define HSP_CRYPTO_ECC_KEY_LENGTH_192  192
#define HSP_CRYPTO_ECC_KEY_LENGTH_224  224
#define HSP_CRYPTO_ECC_KEY_LENGTH_256  256
#define HSP_CRYPTO_ECC_KEY_LENGTH_384  384
#define HSP_CRYPTO_ECC_KEY_LENGTH_521  521
#define HSP_CRYPTO_ECC_KEY_LENGTH_1024 1024
#define HSP_CRYPTO_ECC_KEY_LENGTH_2048 2048
#define HSP_CRYPTO_ECC_KEY_LENGTH_3072 3072
#define HSP_CRYPTO_ECC_KEY_LENGTH_4096 4096

//
// Sanity assert for validating data arguments to crypto libraries
//
#define CCS_DATA_SIZE_ASSERT(x) \
    static_assert(sizeof(x) <= HSP_CCS_DATA_SIZE, "Ensure data fits in Shared RAM, or break up data into multiple calls.")


//
// Scratch Key will be placed into CCS_SCRATCH space
//
#define HSP_CRYPTO_REGISTER_CCS_SCRATCH (HSP_CRYPTO_REGISTER_SHAREDRAM)

//
// Attributes of above SHACK crypto keys
//
typedef enum ShackKeyAttribute
{
    IsDeviceSecret = 0x000001,
    AesEncryptAllowed = 0x000002,
    AesDecryptAllowed = 0x000004,
    Aes128bitAllowed = 0x000008,
    AesXtsOnly = 0x000010,
    SendKeyAllowed = 0x000020,
    LoadKeyAllowed = 0x000040,
    DecryptLegacyKeyAllowed = 0x000080,
    StoreKeyAllowed = 0x000100,
    SaveKeyAllowed = 0x000200,
    KdfKeyAllowed = 0x000400,
    KdfPcrAllowed = 0x000800,
    EccSignAllowed = 0x001000,
    EcdhAllowed = 0x002000,
    MustAppendPcr = 0x004000,
    KeySize384 = 0x008000,
    IsEphemeralKey = 0x010000,
    HmacAllowed = 0x020000
} ShackKeyAttribute;


//
//      SHACK functions
//

HSP_API
void HspShackInitialize();


HSP_API
void HspSwapMsg(pchar_t pMsg, uint32_t length);


HSP_API
HSP_STATUS
HspShackKeyById(HSP_CRYPTO_REGISTER Key, puint32_t KeyAddress);


HSP_API
HSP_STATUS
HspShackSetKey(HSP_CRYPTO_REGISTER Target,
               PCSP_MSG_256 KeyData,
               uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackGenRandomKey(HSP_CRYPTO_REGISTER Target, uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackSendKey(HSP_CRYPTO_REGISTER Target, HSP_CRYPTO_REGISTER Source);


HSP_API
HSP_STATUS
HspShackLoadKey(HSP_CRYPTO_REGISTER Target,
                HSP_CRYPTO_REGISTER Source,
                PCSP_MSG_384 KeyBlob);


HSP_API
HSP_STATUS
HspShackDecryptLegacyKey(HSP_CRYPTO_REGISTER Target,
                         HSP_CRYPTO_REGISTER Source,
                         PCSP_MSG_256 KeyBlob,
                         uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackStoreKey(PCSP_MSG_256 Target,
                 HSP_CRYPTO_REGISTER Source,
                 PSP_MSG_384 KeyBlob,
                 uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackSaveKey(HSP_CRYPTO_REGISTER Target,
                HSP_CRYPTO_REGISTER Source,
                PSP_MSG_384 KeyBlob);


HSP_API
HSP_STATUS
HspShackKdfKey(HSP_CRYPTO_REGISTER Target,
               HSP_CRYPTO_REGISTER Source,
               PCSP_MSG_256 Data,
               uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackSelfKdfKey(HSP_CRYPTO_REGISTER Target, uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackKdfPcr(HSP_CRYPTO_REGISTER Target,
               HSP_CRYPTO_REGISTER Source,
               HSP_CRYPTO_REGISTER PcrReg,
               uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackKdfAsPcr(HSP_CRYPTO_REGISTER Target,
                 HSP_CRYPTO_REGISTER Source,
                 PCSP_MSG_256 TargetPcr,
                 uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackDeriveEccPublic(HSP_CRYPTO_REGISTER Target, PSP_MSG_512 KeyData);


HSP_API
HSP_STATUS
HspShackEccPcrSign(HSP_CRYPTO_REGISTER Target,
                   HSP_CRYPTO_REGISTER PcrReg,
                   PCSP_MSG_256 Data,
                   PSP_MSG_512 Signature);


HSP_API
HSP_STATUS
HspShackEccSign(HSP_CRYPTO_REGISTER Target,
                PCSP_MSG_256 Data,
                PSP_MSG_512 Signature);


HSP_API
HSP_STATUS
HspShackEcdhPcrKeyExchange(HSP_CRYPTO_REGISTER Target,
                           HSP_CRYPTO_REGISTER EccReg,
                           HSP_CRYPTO_REGISTER PcrReg,
                           PSP_MSG_512 PublicKey,
                           uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackEcdhKeyExchange(HSP_CRYPTO_REGISTER Target,
                        HSP_CRYPTO_REGISTER EccReg,
                        PSP_MSG_512 PublicKey,
                        uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackExtendPcr(HSP_CRYPTO_REGISTER PcrReg, PCSP_MSG_256 Data);


HSP_API
HSP_STATUS
HspShackBurnKey(HSP_CRYPTO_REGISTER Target);


//
//      SHACK2 functions
//


HSP_API
HSP_STATUS
HspShackSetKey2(HSP_CRYPTO_REGISTER Target,
                PCSP_MSG_384 KeyData,
                uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackKdfKey2(HSP_CRYPTO_REGISTER Target,
                HSP_CRYPTO_REGISTER Source,
                PCSP_MSG_384 Data,
                uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackSelfKdfKey2(HSP_CRYPTO_REGISTER Target, uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackKdfPcr2(HSP_CRYPTO_REGISTER Target,
                HSP_CRYPTO_REGISTER Source,
                HSP_CRYPTO_REGISTER PcrReg,
                uint32_t Attributes);


HSP_API
HSP_STATUS
HspShackEccSign2(HSP_CRYPTO_REGISTER Target,
                 PCSP_MSG_384 Data,
                 PSP_MSG_512 Signature);


HSP_API
HSP_STATUS
HspShackExtendPcr384(HSP_CRYPTO_REGISTER PcrReg, PCSP_MSG_384 Data);


HSP_API
HSP_STATUS
HspShackReInitPcr(HSP_CRYPTO_REGISTER PcrReg);


HSP_API
HSP_STATUS
HspShackDeriveEccKey(HSP_CRYPTO_REGISTER Target,
                     HSP_CRYPTO_REGISTER Source,
                     uint32_t Attributes);


//
//      AES functions
//


HSP_API
HSP_STATUS
HspShackAesCompute(uint32_t Encrypt,
                   uint32_t Mode,
                   HSP_CRYPTO_REGISTER Key,
                   uint32_t KeySize,
                   PCSP_MSG_128 IV,
                   PCSP_MSG_128 DataIn,
                   PSP_MSG_128 DataOut,
                   uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAesEcbEncrypt(HSP_CRYPTO_REGISTER Key,
                      PCSP_MSG_128 DataIn,
                      PSP_MSG_128 DataOut,
                      uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAesEcbDecrypt(int Key,
                      PCSP_MSG_128 DataIn,
                      PSP_MSG_128 DataOut,
                      uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAes128EcbEncrypt(HSP_CRYPTO_REGISTER Key,
                         PCSP_MSG_128 DataIn,
                         PSP_MSG_128 DataOut);


HSP_API
HSP_STATUS
HspShackAes128EcbDecrypt(int Key, PCSP_MSG_128 DataIn, PSP_MSG_128 DataOut);


HSP_API
HSP_STATUS
HspShackAesCbcEncrypt(HSP_CRYPTO_REGISTER Key,
                      PCSP_MSG_128 IV,
                      PCSP_MSG_128 DataIn,
                      PSP_MSG_128 DataOut,
                      uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAesCbcDecrypt(HSP_CRYPTO_REGISTER Key,
                      PCSP_MSG_128 IV,
                      PCSP_MSG_128 DataIn,
                      PSP_MSG_128 DataOut,
                      uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAes128CbcEncrypt(HSP_CRYPTO_REGISTER Key,
                         PCSP_MSG_128 IV,
                         PCSP_MSG_128 DataIn,
                         PSP_MSG_128 DataOut,
                         uint32_t DataBlockCount);


HSP_API
HSP_STATUS
HspShackAes128CbcDecrypt(HSP_CRYPTO_REGISTER Key,
                         PCSP_MSG_128 IV,
                         PCSP_MSG_128 DataIn,
                         PSP_MSG_128 DataOut,
                         uint32_t DataBlockCount);


//
//      PKA functions
//


HSP_API
HSP_STATUS
HspShackPkaCalcConstant(uint16_t CurveBits, pcvoid_t Modulus);


HSP_API
HSP_STATUS
HspShackEcdsaVerify(uint16_t CurveBits,
                    pcvoid_t SignatureR,
                    pcvoid_t SignatureS,
                    pcvoid_t Digest,
                    uint16_t DigestSize,
                    pcvoid_t EccPubKeyX,
                    pcvoid_t EccPubKeyY);


HSP_API
HSP_STATUS
HspShackEcdsa384Verify(const SP_ECDSA_P384_SIGNATURE* Signature,
                       PCSP_MSG_256 Digest,
                       PCSP_MSG_384 EccPubKeyX,
                       PCSP_MSG_384 EccPubKeyY);


HSP_API
HSP_STATUS
HspShackEcdsa384Verify2(const SP_ECDSA_P384_SIGNATURE* Signature,
                        PCSP_MSG_384 Digest,
                        PCSP_MSG_384 EccPubKeyX,
                        PCSP_MSG_384 EccPubKeyY);


//
// Requires SppCcpPkaCalcConstantHsp to be called first.
//
HSP_API
HSP_STATUS
HspShackEccComputePointMult(uint16_t CurveBits,
                            pcvoid_t PointX,
                            pcvoid_t PointY,
                            pcvoid_t Scalar,
                            pvoid_t ResultX,
                            pvoid_t ResultY);


HSP_API
HSP_STATUS
HspShackEccPointMult(uint16_t CurveBits,
                     pcvoid_t PointX,
                     pcvoid_t PointY,
                     pcvoid_t Scalar,
                     pvoid_t OutX,
                     pvoid_t OutY);


//
// Requires SppCcpPkaCalcConstantHsp to be called first.
//
HSP_API
HSP_STATUS
HspShackEccComputePointAdd(uint16_t CurveBits,
                           pcvoid_t Point1X,
                           pcvoid_t Point1Y,
                           pcvoid_t Point2X,
                           pcvoid_t Point2Y,
                           pvoid_t ResultX,
                           pvoid_t ResultY);


HSP_API
HSP_STATUS
HspShackEccPointAdd(uint16_t CurveBits,
                    pcvoid_t Point1X,
                    pcvoid_t Point1Y,
                    pcvoid_t Point2X,
                    pcvoid_t Point2Y,
                    pvoid_t OutX,
                    pvoid_t OutY);


HSP_API
HSP_STATUS
HspShackEcdsaSign(uint16_t CurveBits,
                  pvoid_t SignatureR,
                  pvoid_t SignatureS,
                  pcvoid_t Digest,
                  uint16_t DigestSize,
                  pcvoid_t EccPrivKey);


HSP_API
HSP_STATUS
HspShackRsaModExpLarge(pcvoid_t Modulus,
                       pcvoid_t Exponent,
                       pcvoid_t DataIn,
                       pvoid_t DataOut,
                       uint32_t Bits,
                       bool Private);


HSP_API
HSP_STATUS
HspShackRsaModExpSmall(pcvoid_t Modulus,
                       uint32_t Exponent,
                       pcvoid_t DataIn,
                       pvoid_t DataOut,
                       uint32_t Bits);


HSP_API
HSP_STATUS
HspShackMontIn(uint16_t Bits, pcvoid_t Value, pvoid_t Result);


HSP_API
HSP_STATUS
HspShackMontOut(uint16_t Bits, pcvoid_t Value, pvoid_t Result);


HSP_API
HSP_STATUS
HspShackBigIntAdd(uint16_t Bits, pcvoid_t Value1, pcvoid_t Value2, pvoid_t Result);


HSP_API
HSP_STATUS
HspShackModAdd(PCSP_MSG_384 Modulus,
               PCSP_MSG_384 X,
               PCSP_MSG_384 Y,
               PSP_MSG_384 Result);


HSP_API
HSP_STATUS
HspShackModRed(uint16_t Bits, pcvoid_t Value, pvoid_t Result);


HSP_API
HSP_STATUS
HspShackMontMult(uint16_t Bits, pcvoid_t Value1, pcvoid_t Value2, pvoid_t Result);


//
// Modular multiplication.
// Note that on some platforms this call has a lot of overhead, so it's not
// suitable for a large number of consecutive multiplications where
// performance is a concern.
//
HSP_API
HSP_STATUS
HspShackModMult(PCSP_MSG_384 Modulus,
                PCSP_MSG_384 X,
                PCSP_MSG_384 Y,
                PSP_MSG_384 Result);


HSP_API
HSP_STATUS
HspShackMontInv(uint16_t Bits, pcvoid_t Value, pvoid_t Result);


HSP_API
HSP_STATUS
HspShackModInv(PCSP_MSG_384 Modulus, PCSP_MSG_384 X, PSP_MSG_384 Result);


//
//      RNG functions
//


HSP_API
HSP_STATUS
HspRngInitialize();


HSP_API
HSP_STATUS
HspRngGetRandom32(puint32_t Random);


HSP_API
HSP_STATUS
HspRngGetRandom16(puint16_t Random, uint16_t Max);


HSP_API
HSP_STATUS
HspRngGetRandom(pvoid_t Data, uint16_t Length);


HSP_API
HSP_STATUS
HspRngSetBits(uint32_t* Data,
              uint32_t BitsTotal,
              uint32_t PresetBits,
              uint32_t Ones);


HSP_API
HSP_STATUS
HspStallRandom(uint32_t MaxBits);


HSP_API
HSP_STATUS
HspRngWaitForReseedCount();

//
//      SHA functions
//

#define SHA_256_BLOCKSIZE (64)
#define SHA_384_BLOCKSIZE (128)

//
// Struct to keep track of SHA operation state.
// Required for below operations - caller is responsible for allocating space.
// Call the relevant *Init function to intialize the structure for the desired
// operation. See function descriptions for more information on usage.
//
typedef struct _SHA_CONTEXT
{
    bool ComputeInit;
    bool HashInit;
    uint32_t ShaType;
    uint32_t HashDataCursor;
    uint32_t SHABits;
    pvoid_t PassThroughBuffer;
    char HashDataBuffer[SHA_384_BLOCKSIZE];
    char HmacKeyBuffer[SHA_384_BLOCKSIZE];
} SHA_CONTEXT, *PSHA_CONTEXT;


HSP_API
void HspShackShaInit(PSHA_CONTEXT Context,
                     uint32_t ShaType,
                     pvoid_t PassThroughBuffer);


HSP_API
HSP_STATUS
HspShackSha1Compute(PSHA_CONTEXT Context,
                    pcvoid_t Data,
                    uint32_t DataLength,
                    PSP_MSG_160 Result);


HSP_API
HSP_STATUS
HspShackHmacSha1Init(PSHA_CONTEXT Context,
                     pcvoid_t Key,
                     uint32_t KeyLength,
                     pvoid_t PassThroughBuffer);


HSP_API
HSP_STATUS
HspShackHmacSha1Compute(PSHA_CONTEXT Context,
                        pcvoid_t Data,
                        uint32_t DataLength,
                        PSP_MSG_160 Result);


HSP_API
HSP_STATUS
HspShackSha256Compute(PSHA_CONTEXT Context,
                      pcvoid_t Data,
                      uint32_t DataLength,
                      PSP_MSG_256 Result);


HSP_API
HSP_STATUS
HspShackSha384Compute(PSHA_CONTEXT Context,
                      pcvoid_t Data,
                      uint32_t DataLength,
                      PSP_MSG_384 Result);


HSP_API
HSP_STATUS
HspShackHmacSha256Init(PSHA_CONTEXT Context,
                       pcvoid_t Key,
                       uint32_t KeyLength,
                       pvoid_t PassThroughBuffer);


HSP_API
HSP_STATUS
HspShackHmacSha256Compute(PSHA_CONTEXT Context,
                          pcvoid_t Data,
                          uint32_t DataLength,
                          PSP_MSG_256 Result);


HSP_API
HSP_STATUS
HspCryptoValidateRsaPssSha256(pcchar_t LocalDigest, pchar_t Message, uint32_t Bits);


//
//      SPI functions
//

HSP_API
void HspSpiInitialize(uint16_t Speed, uint8_t SlaveSelect);


//
//      Flash functions
//

HSP_API
void HspFlashInitialize(uint16_t Speed);


HSP_API
void HspFlashEnableQuadMode(FLASH_JEDEC_ID ConnectedFlash);


HSP_API
uint32_t HspFlashQuadRead(pvoid_t Destination, pvoid_t Source, uint32_t NumBytes);


HSP_API
uint32_t HspFlashRead(pvoid_t Destination, pvoid_t Source, uint32_t NumBytes);


HSP_API
void HspFlashWrite(pvoid_t Destination,
                   pvoid_t Data,
                   uint32_t NumBytes,
                   uint32_t PageSize);


HSP_API
void HspFlashEraseAndWrite(pvoid_t Destination,
                           pvoid_t Data,
                           uint32_t NumBytes,
                           uint32_t PageSize,
                           uint32_t SectorSize);


HSP_API
void HspFlashEraseSector(pvoid_t StartAddress);


HSP_API
void HspFlashEraseChip();


//
//      CREG functions
//

HSP_API
bool HspGetA0Bypass();


HSP_API
void HspMemoryErase(uint32_t MemEraseEn);


//
//      Timer common APIs
//

HSP_API
void HspDelayMs(uint32_t MilliSeconds);


HSP_API
void HspDelayUs(uint32_t MicroSeconds);


//
//      Timer2 APIs
//

HSP_API
void HspEnableWatchdogTimerMs(uint32_t MilliSeconds);


HSP_API
void HspKickWatchdogTimer();


HSP_API
void HspDisableWatchdogTimer();


//
//      Mailbox functions and constants
//

#define HSP_MAILBOX_OFFSET          0x10
#define HSP_MAILBOX_OFFSET_OUT_INST 0x1
#define HSP_MAILBOX_OFFSET_PUSH     0x2
#define HSP_MAILBOX_OFFSET_IN_INST  0x5
#define HSP_MAILBOX_OFFSET_POP      0x7

#define HSP_MAILBOX_FIFO_EMPTY_CNT  0x0
#define HSP_MAILBOX_FIFO_FULL_CNT   0x4

#define HSP_MAILBOX_FIFO_VALID_INT  0x4
#define HSP_MAILBOX_FIFO_ERRBIT_INT 0x8

#ifdef __clang__
static INLINE HSP_API uint32_t HspMailboxGetFifoStatus(HSP_MAILBOX_SLOT Slot)
#else
// GCC doesn't like the static function used from a non-static inline function.
INLINE HSP_API uint32_t HspMailboxGetFifoStatus(HSP_MAILBOX_SLOT Slot)
#endif
/*++

Description:

    return the status of the fifo

--*/
{
    return HspReadRegister32(CREG_REG(H2S_MBX0_MBX_REGS_H2S_MBX_CTRL) +
                             Slot * HSP_MAILBOX_OFFSET +
                             HSP_MAILBOX_OFFSET_IN_INST);
}


HSP_API
INLINE
bool HspMailboxIsIncomingFifoFull(HSP_MAILBOX_SLOT Slot)
/*++

Description:

    return true if the fifo is full and ready to read

--*/
{
    CREG_H2S_MBX0_MBX_REGS_S2H_MBX_INSTS status = {0};

    status.u = HspMailboxGetFifoStatus(Slot);

    return status.Fifo_Cnt == HSP_MAILBOX_FIFO_FULL_CNT;
}


HSP_API
HSP_STATUS
HspMailboxWrite(HSP_MAILBOX_SLOT Slot, PHSP_MAILBOX_MSG Msg);


HSP_API
HSP_STATUS
HspMailboxRead(HSP_MAILBOX_SLOT Slot, PHSP_MAILBOX_MSG Msg);


HSP_API
HSP_STATUS
HspMailboxReadIfReady(HSP_MAILBOX_SLOT Slot, PHSP_MAILBOX_MSG Msg);


HSP_API
void HspMailboxRequestFlush(HSP_MAILBOX_SLOT Slot);

//
// UTC Time synchornization functions
//

HSP_API
void HspSetUtcTime(uint64_t UtcTime);

HSP_API
uint64_t HspGetUtcTime();


//
//      I2C functions
//

typedef enum _I2C_IC_SPEED_MODE
{
    I2cSpeedModeStandard = 1,    // 100 kbit/s
    I2cSpeedModeFast,            // 400 kbit/s or 1000 kbit/s (fast mode plus)
    I2cSpeedModeHigh             // 3.4 Mbit/s
} I2C_IC_SPEED_MODE, *PI2C_IC_SPEED_MODE;


HSP_API
void HspI2cDisable();


HSP_API
void HspI2cEnable();


HSP_API
void HspI2cInitialize(bool MasterMode, uint8_t Address, I2C_IC_SPEED_MODE Speed);


HSP_API
bool HspI2cSlaveReadRequest();


HSP_API
bool HspI2cIsByteAvailable();


HSP_API
void HspI2cSendByte(uint8_t Data);


HSP_API
HSP_STATUS
HspI2cSendBuffer(uint8_t* Buffer, uint32_t ByteCount, uint64_t Timeout);


HSP_API
HSP_STATUS
HspI2cRecvBuffer(uint8_t* Buffer, uint32_t ByteCount, uint64_t Timeout);

HSP_API
uint8_t HspI2cRecvByte();
