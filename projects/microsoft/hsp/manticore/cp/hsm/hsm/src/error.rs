// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_types::DdiStatus;
use mcr_error::mcr_err_decl;
use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::cmd_scheduler::CmdFsmError;

pub(crate) type HsmResult<T> = Result<T, HsmErr>;

mcr_err_decl! {
    Hsm,
    HsmErr
    {
        // Pending
        Pending = 0,

        // IO Channel received none
        IoChannelRecvNone = 1,

        // IO Channel received error
        IoChannelRecvErr = 2,

        // SQE decode error
        SqeDecodeError = 3,

        // SQE unknown operation
        SqeUnknownOp = 4,

        // SQE invalid source length
        SqeInvalidSrcLen = 5,

        // SQE invalid destination length
        SqeInvalidDstLen = 6,

        // PCIe function not enabled
        PartitionNotEnabled = 7,

        // Queue not enabled
        QueueNotEnabled = 8,

        // Queue not valid
        QueueNotValid = 9,

        // DMA allocation failure
        DmaAllocFailure = 10,

        // Invalid PSDT found in SQE
        SqeInvalidPsdt = 11,

        // DMA completion empty
        DmaCompletionEmpty = 12,

        // DMA end error
        DmaEndErr= 13,

        // DMA tag mismatch
        DmaTagMismatch = 14,

        // Expected IO queue
        ExpectedIoQueue = 15,

        // Expected PCIe function
        ExpectedPcieFn = 16,

        // DMA start error
        DmaStartError= 17,

        // IO channel send error
        IoChannelSendError = 18,

        // Expected DMA buffer
        ExpectedDmaBuf = 19,

        // Request header decode error
        ReqHdrDecodeErr = 20,

        // DDI request data decode error
        DdiDecodeFailed = 21,

        // Expected command FSM
        ExpectedCmdFsm = 22,

        // Invalid source PRP alignment
        SqeInvalidSrcPrpAlgin = 23,

        // Invalid destination PRP alignment
        SqeInvalidDstPrpAlgin = 24,

        // Command error
        CmdError = 25,

        // Invalid manager credentials
        InvalidMgrCredential = 28,

        // Invalid application credentials
        InvalidUserCredential = 29,

        // Application not found
        AppNotFound = 30,

        // Application already exists
        AppAlreadyExists = 31,

        // Session not found
        SessionNotFound = 33,

        // Sessions exceed maximum
        SessionLimitReached = 34,

        // Function not enabled
        FunctionNotEnabled = 35,

        // Spurious IPC message event
        SpuriousIpcMessageEvent = 36,

        // Invalid IPC message
        InvalidIpcMessage = 37,

        // IPC message decode error
        IpcMessageDecodeErr = 38,

        // Invalid message opcode
        InvalidMessageOpcode = 39,

        // Session expected
        SessionExpected = 40,

        // IO channel send complete none
        IoChannelSendCompleteNone = 41,

        // IO tag mismatch
        IoTagMismatch = 42,

        // IO channel send complete error
        IoChannelSendCompleteError = 43,

        // Another key in use
        AnotherKeyInUse = 44,

        // Key not in use
        KeyNotInUse = 45,

        // Invalid argument
        InvalidArgument = 47,

        // Not enough space
        NotEnoughSpace = 48,

        // Defrag needed
        DefragNeeded = 49,

        // Invalid key index
        InvalidKeyIndex = 50,

        // Cannot delete key in use
        CannotDeleteKeyInUse = 51,

        // Cannot delete some keys in use
        CannotDeleteSomeKeysInUse = 52,

        // Key not found
        KeyNotFound = 53,

        // Key tag already exists
        KeyTagAlreadyExists = 54,

        // AES encrypt failed
        AesEncryptFailed = 55,

        // Unsupported revision
        UnsupportedRevision = 56,

        // Session not expected
        SessionNotExpected = 57,

        // Vault not found
        VaultNotFound = 60,

        // Invalid Session Control Opcode
        InvalidSessionControlOpcode = 61,

        // Key DER encoding failed.
        DerEncodeFailed = 62,

        // App limit reached for the vault
        AppLimitReached = 63,

        // Cannot close session while in use
        CannotCloseSessionInUse = 64,

        // Cannot close multiple sessions while in use
        CannotCloseSomeSessionsInUse = 65,

        // Cannot delete key and close session while in use
        CannotDeleteKeyAndCloseSessionInUse = 66,

        // Invalid permissions (aka Invalid KeyUsage)
        InvalidPermissions = 69,

        // AES decrypt failed
        AesDecryptFailed = 70,

        // Ddi Encode Failed
        DdiEncodeFailed = 71,

        // Invalid Key type
        InvalidKeyType = 72,

        // ECC key pair generate failed
        EccGenKeyFailed = 73,

        // ECC verify operations failed
        EccVerifyFailed = 74,

        // Unsupported DDI op code
        UnsupportedCmd = 75,

        // Cannot use default credentials
        CannotUseDefaultCredentials = 76,

        // Invalid memory map entry.
        InvalidMemoryMapEntry = 77,

        // Invalid Event
        InvalidEvent = 78,

        // Invalid State
        InvalidState = 79,

        // PKA Tag Mismatch
        PkaTagMismatch = 83,

        // PKA Engine not busy
        PkaEngineNotBusy = 84,

        // Ecc Sign failed
        EccSignFailed = 85,

        // ECC Montgomery Constant Calculation failed
        EccMontgomeryConstCalcFailed = 87,

        // ECC Gen Public Key failed
        EccGenPubKeyFailed = 88,

        // Key DER decoding failed.
        DerDecodeFailed = 89,

        // SHA command failed
        ShaCmdFailed = 90,

        // RSA Modular Exponentiation failed
        RsaModExpFailed = 91,

        // RSA unwrap internal errors
        RsaUnwrapInternalErr = 92,

        // RSA unwrap invalid request
        RsaUnwrapInvalidReq = 93,

        // RSA unwrap invalid key-encryption key
        RsaUnwrapInvalidKek = 94,

        // RSA unwrap oaep decode failed
        RsaUnwrapOaepDecodeFailed = 95,

        // Reach invalid AES unwrap state during RSA unwrap command
        RsaUnwrapInvalidAesUnwrapState = 96,

        // AES unwrap failed during RSA unwrap command
        RsaUnwrapAesUnwrapFailed = 97,

        // RSA Montgomery Constant Calculation failed
        RsaMontgomeryConstCalcFailed = 98,

        // RSA Montgomery In operation failed
        RsaMontgomeryInFailed = 99,

        // RSA Modular multiplication operation failed
        RsaModularMultiplicationFailed = 100,

        // RSA Montgomery Out operation failed
        RsaMontgomeryOutFailed = 101,

        // RSA Modular inverse operation failed
        RsaModularInverseFailed = 102,

        // Attestation report encoding failed.
        AttestationReportEncodeFailed = 103,

        // COSE key encoding failed.
        CoseKeyEncodeFailed = 104,

        // Attest Key internal errors
        AttestKeyInternalErr = 105,

        // ECDH Compute failed.
        EcdhComputeFailed = 106,

        // Expected IO Queue
        ExpectedIoq = 107,

        // Invalid Io Queue
        InvalidIoq = 108,

        // IPC message send failure
        IpcSendFailure = 109,

        // IPC message response failure
        IpcResponseError = 110,

        // DER doesn't match key type
        DerAndKeyTypeMismatch = 111,

        // Key Derivation Compute failed.
        KeyDeriveFailed = 112,

        // App deletion failed due to active sessions.
        CannotDeleteAppSessionsOpen = 113,

        // AesBulk256 Invalid parameter
        AesBulk256InvalidParameter = 114,

        // Invalid IPC Shutdown request message
        InvalidIpcShutdownRequest = 115,

        // Invalid certificate found in the device.
        InvalidCertificate = 116,

        // Key availability is pending key generation.
        PendingKeyGeneration = 117,

        // Io Timeout
        IoTimeOut = 118,

        // Scheduler drain is busy
        DrainBusy = 119,

        // Cannot delete internal keys, like RSA unwrapping key
        CannotDeleteInternalKeys = 120,

        // Deferred Queue Deletion Notification error
        DeferredQueueDeleteNotifyErr = 121,

        // Failed to Send SoftAes Request
        SoftAesReqSendFailed = 122,

        // Session Param Encryption key generated failed.
        SessionEncryptionKeyGenerateFailed = 123,

        // Open Session Tag validation failed.
        PinDecryptionFailed = 124,

        // IP response decode error
        IpcResponseDecodeError = 125,

        // AES Bulk Key vault exhausted
        ReachedMaxAesBulkKeys = 126,

        // Invalid input array size for HMAC operation
        HmacInvalidInputSize = 127,

        // Unable to successfully compute HMAC
        HmacComputeFailed = 128,

        // Nonce Mismatch
        NonceMismatch = 129,

        // Establish Credential Encryption key generation failed.
        EstablishCredEncryptionKeyGenerateFailed = 130,

        // HKDF Invalid input paramter
        HkdfInvalidInputParam = 131,

        // Unable to successfully compute HKDF
        HkdfError = 132,

        // KBKDF Invalid input paramter
        KbkdfInvalidInputParam = 133,

        // Unable to successfully compute KBKDF
        KbkdfError = 134,

        // Invalid key index
        InvalidKeyTableIndex = 135,

        // Unknown Self Test
        UnknownSelfTest = 136,

        // Missing Self Test Cryptographic engine instance
        MissingSelfTestEngineInstance = 137,

        // Invalid Pin Authentication attempt
        LoginFailed = 138,

        // Invalid Response from Soft Aes Operation
        SoftAesInvalidResp = 139,

        // UPKA Memory Zeroization failed
        PkaMemoryWipeFailed = 140,

        // FSM is pending but ready to drain
        DrainReady = 141,

        // Key structural validation failed
        KeyStructuralValidationFailed = 142,

        // Invalid PoR Measurement Memory access
        InvalidPorMeasurementDataAccess = 143,

        // ECC Gen Key PCT validation failed
        PctValidationEccGenKeyFailed = 144,

        // Get Establish Credential Encryption Key PCT validation failed
        PctValidationEstablishCredEncKeyFailed = 145,

        // Get Session Encryption Key PCT validation failed
        PctValidationSessionEncKeyFailed = 146,

        // Get Unwrapping Key PCT validation failed
        PctValidationUnwrappingKeyFailed = 147,

        // RSA Unwrap ECC Key PCT validation failed
        PctValidationRsaUnwrapEccKeyFailed = 148,

        // RSA Unwrap RSA Key PCT validation failed
        PctValidationRsaUnwrapRsaKeyFailed = 149,

        // Non FIPS approved message digest.
        NonFipsApprovedMessageDigest = 150,

        // Digest does not match the ECC curve used for signing.
        DigestHashMismatchWithEccCurve = 151,

        // Unsupported Digest hash algorithm used.
        UnsupportedDigestHashAlgorithm = 152,

        // Failed to begin Ecc Point Validation
        BeginEccPointValidationFailed = 153,

        // Failed to end Ecc Point Validation
        EndEccPointValidationFailed = 154,

        // Ecc Point Validation failed
        EccPointValidationFailed = 155,

        // Ecc Public Key Validation failed
        EccPublicKeyValidationFailed = 156,

        // Ecc DER Key length shorter than the curve size
        EccDerKeyShorterThanCurve = 157,

        // Partition Identifier generation failed
        PartitionIdGenerationFailed = 158,

        // Partition certificate generation failed
        PartitionCertGenerationFailed = 159,

        // Partition certificate creation failed due to invalid type conversion
        PartitionCertInvalidTypeConversion = 160,

        // FSM allocation failed
        FsmAllocationFailed = 161,

        // Partition certificate too large
        PartitionCertTooLarge = 162,

        // Masked key length is invalid
        MaskedKeyInvalidLength = 163,

        // Masked key pre-encode failed
        MaskedKeyPreEncodeFailed = 164,

        // Masked key encode failed
        MaskedKeyEncodeFailed = 165,

        // Masked key decode failed
        MaskedKeyDecodeFailed = 166,

        // Invalid Algorithm
        InvalidAlgorithm = 167,

        // Insufficient Buffer
        InsufficientBuffer = 168,

        // Invalid Key Length
        InvalidKeyLength = 169,

        // Metadata Encode Error
        MetadataEncodeFailed = 170,

        // Metadata Decode Error
        MetadataDecodeFailed = 171,

        // Session needs to be renegotiated after migration
        SessionNeedsRenegotiation = 172,

        // BK Boot generation failure
        BkBootGenerationFailed = 173,

        // Masking BK3 failed
        MaskingBk3Failed = 174,

        // Unmasking BK3 failed
        UnmaskingBk3Failed = 175,

        // Masking BK Boot failed
        MaskingBkBootFailed = 176,

        // Unmasking BK Boot failed
        UnmaskingBkBootFailed = 177,

        // Masked BK Boot not present
        MaskedBkBootNotPresent = 178,

        // Sealed Bk3 too large
        SealedBk3TooLarge = 179,

        // Partition already provisioned
        PartitionAlreadyProvisioned = 180,

        // Sealed BK3 not present
        SealedBk3NotPresent = 181,

        // Invalid Alias Key
        InvalidAliasKey = 183,

        // Do not allow external call to unmask unwrapping key
        UnmaskUnwrappingKeyNotAllowed = 184,

        // Invalid Partition Id Private Key Internal Error
        InvalidPartIdPrivKeyInternalError = 185,

        // Partition is not provisioned
        PartitionNotProvisioned = 186,

        // Credentials not established
        CredentialsNotEstablished = 187,

        // BK3 already initialized
        Bk3AlreadyInitialized = 188,

        // Sealed BK3 already set
        SealedBk3AlreadySet = 189,

        // Partition ID Key Generation PCT failed
        PartitionIdKeyGenerationPctFailed = 190,

        // Invalid IPC get key bulk request message
        InvalidIpcGetKeyBulkRequest = 191,
    }
}

impl CmdFsmError for HsmErr {
    fn pending(&self) -> bool {
        self == &Self::Pending
    }

    fn drain_ready(&self) -> bool {
        self == &Self::DrainReady
    }
}

macro_rules! host_status_code {
    ($type:ident, $code:literal) => {
        (HostStatusCodeType::$type << 8) | $code
    };
}
struct HostStatusCodeType {}

impl HostStatusCodeType {
    const GENERIC: u16 = 0x0;
    //const COMMAND_SPECIFC: u16 = 0x1;
    //const VENDOR_SPECIFIC: u16 = 0x7;
}

#[repr(u16)]
#[open_enum]
#[derive(Debug, Default, IntoBytes, Immutable, FromBytes)]
pub(crate) enum HostStatusCode {
    Success = host_status_code!(GENERIC, 0x0),
    InvalidCommandOpCode = host_status_code!(GENERIC, 0x1),
    InvalidFieldInCommand = host_status_code!(GENERIC, 0x2),
    InternalError = host_status_code!(GENERIC, 0x7),
    InvalidPsdtFieldInCommand = host_status_code!(GENERIC, 0xC0),
    InvalidSrcLenFieldInCommand = host_status_code!(GENERIC, 0xC1),
    InvalidDstLenFieldInCommand = host_status_code!(GENERIC, 0xC2),
    InvalidSrcPrpFieldInCommand = host_status_code!(GENERIC, 0xC3),
    InvalidDstPrpFieldInCommand = host_status_code!(GENERIC, 0xC4),
    DmaCompletionEmpty = host_status_code!(GENERIC, 0xC5),
    DmaTxnError = host_status_code!(GENERIC, 0xC6),
    DmaTagMismatch = host_status_code!(GENERIC, 0xC7),
    ReqHdrDecodeErr = host_status_code!(GENERIC, 0xC8),
    DmaStartError = host_status_code!(GENERIC, 0xC9),
}

impl From<HostStatusCode> for u16 {
    fn from(value: HostStatusCode) -> Self {
        value.0
    }
}

impl From<u16> for HostStatusCode {
    fn from(value: u16) -> Self {
        match value {
            x if x == HostStatusCode::InvalidCommandOpCode.into() => {
                HostStatusCode::InvalidCommandOpCode
            }
            x if x == HostStatusCode::InvalidFieldInCommand.into() => {
                HostStatusCode::InvalidFieldInCommand
            }
            x if x == HostStatusCode::InvalidPsdtFieldInCommand.into() => {
                HostStatusCode::InvalidPsdtFieldInCommand
            }
            x if x == HostStatusCode::InvalidSrcLenFieldInCommand.into() => {
                HostStatusCode::InvalidSrcLenFieldInCommand
            }
            x if x == HostStatusCode::InvalidDstLenFieldInCommand.into() => {
                HostStatusCode::InvalidDstLenFieldInCommand
            }
            x if x == HostStatusCode::InvalidSrcPrpFieldInCommand.into() => {
                HostStatusCode::InvalidSrcPrpFieldInCommand
            }
            x if x == HostStatusCode::InvalidDstPrpFieldInCommand.into() => {
                HostStatusCode::InvalidDstPrpFieldInCommand
            }
            x if x == HostStatusCode::DmaCompletionEmpty.into() => {
                HostStatusCode::DmaCompletionEmpty
            }
            x if x == HostStatusCode::DmaTxnError.into() => HostStatusCode::DmaTxnError,
            x if x == HostStatusCode::DmaTagMismatch.into() => HostStatusCode::DmaTagMismatch,
            x if x == HostStatusCode::ReqHdrDecodeErr.into() => HostStatusCode::ReqHdrDecodeErr,
            x if x == HostStatusCode::DmaStartError.into() => HostStatusCode::DmaStartError,
            x if x == HostStatusCode::Success.into() => HostStatusCode::Success,

            _ => HostStatusCode::InternalError,
        }
    }
}

impl From<HsmErr> for HostStatusCode {
    fn from(value: HsmErr) -> Self {
        match value {
            HsmErr::SqeUnknownOp => HostStatusCode::InvalidCommandOpCode,
            HsmErr::SqeInvalidSrcLen => HostStatusCode::InvalidSrcLenFieldInCommand,
            HsmErr::SqeInvalidDstLen => HostStatusCode::InvalidDstLenFieldInCommand,
            HsmErr::SqeInvalidPsdt => HostStatusCode::InvalidPsdtFieldInCommand,
            HsmErr::DmaEndErr => HostStatusCode::DmaTxnError,
            HsmErr::DmaStartError => HostStatusCode::DmaStartError,
            HsmErr::DmaCompletionEmpty => HostStatusCode::DmaCompletionEmpty,
            HsmErr::DmaTagMismatch => HostStatusCode::DmaTagMismatch,
            HsmErr::ReqHdrDecodeErr => HostStatusCode::ReqHdrDecodeErr,
            HsmErr::SqeInvalidSrcPrpAlgin => HostStatusCode::InvalidSrcPrpFieldInCommand,
            HsmErr::SqeInvalidDstPrpAlgin => HostStatusCode::InvalidDstPrpFieldInCommand,
            _ => HostStatusCode::InternalError,
        }
    }
}

impl From<HsmErr> for DdiStatus {
    fn from(value: HsmErr) -> Self {
        match value {
            HsmErr::Pending => DdiStatus::PendingIo,
            HsmErr::IoChannelRecvNone => DdiStatus::ReceivedEmptyIoEvent,
            HsmErr::IoChannelRecvErr => DdiStatus::IoChannelReceiveError,
            HsmErr::SqeDecodeError => DdiStatus::IoChannelDecodeError,
            HsmErr::SqeUnknownOp => DdiStatus::IoChannelUnknownOp,
            HsmErr::UnsupportedCmd => DdiStatus::UnsupportedCmd,
            HsmErr::SqeInvalidSrcLen => DdiStatus::IoChannelInvalidSrcLen,
            HsmErr::SqeInvalidDstLen => DdiStatus::IoChannelInvalidDstLen,
            HsmErr::PartitionNotEnabled => DdiStatus::PartitionNotEnabled,
            HsmErr::QueueNotEnabled => DdiStatus::IoChannePipelNotEnabled,
            HsmErr::QueueNotValid => DdiStatus::IoChannePipeNotValid,
            HsmErr::DmaAllocFailure => DdiStatus::DmaBufferAllocFailure,
            HsmErr::SqeInvalidPsdt => DdiStatus::IoChannelInvalidBufferDescriptor,
            HsmErr::DmaCompletionEmpty => DdiStatus::DmaHardwareEmptyCompletionFound,
            HsmErr::DmaEndErr => DdiStatus::DmaCompletedWithError,
            HsmErr::DmaTagMismatch => DdiStatus::DmaIoIdentifierMismatch,
            HsmErr::ExpectedIoQueue => DdiStatus::IoChannelPipeNotFound,
            HsmErr::ExpectedPcieFn => DdiStatus::FailedToAssociateIoWithPartition,
            HsmErr::DmaStartError => DdiStatus::FailedToStartDmaTransaction,
            HsmErr::IoChannelSendError => DdiStatus::IoChannelFailedToSendResponse,
            HsmErr::ExpectedDmaBuf => DdiStatus::FailedToIdentifyDmaBuffer,
            HsmErr::ReqHdrDecodeErr => DdiStatus::IoChannelRequestDecodeError,
            HsmErr::ExpectedCmdFsm => DdiStatus::IoCommandNotFound,
            HsmErr::SqeInvalidSrcPrpAlgin => DdiStatus::IoChannelInvalidSrcAlignment,
            HsmErr::SqeInvalidDstPrpAlgin => DdiStatus::IoChannelInvalidDstAlignment,
            HsmErr::CmdError => DdiStatus::IoCommandError,
            HsmErr::InvalidMgrCredential => DdiStatus::InvalidManagerCredentials,
            HsmErr::InvalidUserCredential => DdiStatus::InvalidAppCredentials,
            HsmErr::AppNotFound => DdiStatus::AppNotFound,
            HsmErr::AppAlreadyExists => DdiStatus::AppAlreadyExists,
            HsmErr::SessionNotFound => DdiStatus::SessionNotFound,
            HsmErr::SessionLimitReached => DdiStatus::VaultSessionLimitReached,
            HsmErr::FunctionNotEnabled => DdiStatus::FunctionNotEnabled,
            HsmErr::SpuriousIpcMessageEvent => DdiStatus::SpuriousIpcMessageReceived,
            HsmErr::InvalidIpcMessage => DdiStatus::InvalidIpcMessageReceived,
            HsmErr::IpcMessageDecodeErr => DdiStatus::FailedToDecodeIpcMessage,
            HsmErr::InvalidMessageOpcode => DdiStatus::InvalidIpcMessageOpCodeFound,
            HsmErr::SessionExpected => DdiStatus::SessionExpected,
            HsmErr::IoChannelSendCompleteNone => DdiStatus::IoChannelTxEmptyCompletionFound,
            HsmErr::IoTagMismatch => DdiStatus::FailedToAssociateIoWithCompletion,
            HsmErr::IoChannelSendCompleteError => DdiStatus::IoChannelFailedToSendCompletion,
            HsmErr::AnotherKeyInUse => DdiStatus::AnotherKeyInUse,
            HsmErr::KeyNotInUse => DdiStatus::KeyNotInUse,
            HsmErr::InvalidArgument => DdiStatus::InvalidArg,
            HsmErr::NotEnoughSpace => DdiStatus::NotEnoughSpace,
            HsmErr::DefragNeeded => DdiStatus::DefragmentationNeeded,
            HsmErr::InvalidKeyIndex => DdiStatus::KeyNotFound,
            HsmErr::CannotDeleteKeyInUse => DdiStatus::CannotDeleteKeyInUse,
            HsmErr::CannotDeleteSomeKeysInUse => DdiStatus::CannotDeleteSomeKeysInUse,
            HsmErr::KeyNotFound => DdiStatus::KeyNotFound,
            HsmErr::KeyTagAlreadyExists => DdiStatus::KeyTagAlreadyExists,
            HsmErr::AesEncryptFailed => DdiStatus::AesEncryptFailed,
            HsmErr::UnsupportedRevision => DdiStatus::UnsupportedRevision,
            HsmErr::SessionNotExpected => DdiStatus::SessionNotExpected,
            HsmErr::VaultNotFound => DdiStatus::VaultNotFound,
            HsmErr::InvalidSessionControlOpcode => DdiStatus::InvalidSessionControlOpcode,
            HsmErr::DerEncodeFailed => DdiStatus::DerDecodeFailed,
            HsmErr::AppLimitReached => DdiStatus::VaultAppLimitReached,
            HsmErr::CannotCloseSessionInUse => DdiStatus::CannotCloseSessionInUse,
            HsmErr::CannotCloseSomeSessionsInUse => DdiStatus::CannotCloseSomeSessionsInUse,
            HsmErr::CannotDeleteKeyAndCloseSessionInUse => {
                DdiStatus::CannotDeleteKeyAndCloseSessionInUse
            }
            HsmErr::InvalidPermissions => DdiStatus::InvalidPermissions,
            HsmErr::AesDecryptFailed => DdiStatus::AesDecryptFailed,
            HsmErr::DdiDecodeFailed => DdiStatus::DdiDecodeFailed,
            HsmErr::DdiEncodeFailed => DdiStatus::DdiEncodeFailed,
            HsmErr::InvalidKeyType => DdiStatus::InvalidKeyType,
            HsmErr::EccGenKeyFailed => DdiStatus::EccGenerateError,
            HsmErr::EccVerifyFailed => DdiStatus::EccVerifyFailed,
            HsmErr::CannotUseDefaultCredentials => DdiStatus::CannotUseDefaultCredentials,
            HsmErr::InvalidMemoryMapEntry => DdiStatus::InvalidMemoryMapEntry,
            HsmErr::InvalidEvent => DdiStatus::ProcessedInvalidIoEvent,
            HsmErr::InvalidState => DdiStatus::ProcessedIoEventInInvalidState,
            HsmErr::PkaTagMismatch => DdiStatus::CannotAssociateIoWithPkaCompletion,
            HsmErr::PkaEngineNotBusy => DdiStatus::IdentifiedPkaEngineNotBusy,
            HsmErr::EccSignFailed => DdiStatus::EccSignFailed,
            HsmErr::EccMontgomeryConstCalcFailed => DdiStatus::IdentifiedEccCalculationFailure,
            HsmErr::EccGenPubKeyFailed => DdiStatus::FailedToGenerateEccPublicKey,
            HsmErr::DerDecodeFailed => DdiStatus::KeyDecodeFailed,
            HsmErr::ShaCmdFailed => DdiStatus::ShaError,
            HsmErr::RsaModExpFailed => DdiStatus::RsaDecryptFailed,
            HsmErr::RsaUnwrapInternalErr => DdiStatus::RsaUnwrapError,
            HsmErr::RsaUnwrapInvalidReq => DdiStatus::RsaUnwrapInvalidRequest,
            HsmErr::RsaUnwrapInvalidKek => DdiStatus::RsaUnwrapInvalidKek,
            HsmErr::RsaUnwrapOaepDecodeFailed => DdiStatus::RsaUnwrapOaepDecodeFailed,
            HsmErr::RsaUnwrapInvalidAesUnwrapState => DdiStatus::RsaUnwrapInvalidAesUnwrapState,
            HsmErr::RsaUnwrapAesUnwrapFailed => DdiStatus::RsaUnwrapAesUnwrapFailed,
            HsmErr::RsaMontgomeryConstCalcFailed => DdiStatus::IdentifiedRsaCalculationFailure,
            HsmErr::RsaMontgomeryInFailed => DdiStatus::FailedToBeginRsaCalculation,
            HsmErr::RsaModularMultiplicationFailed => DdiStatus::FailedToPerformRsaMultiplication,
            HsmErr::RsaMontgomeryOutFailed => DdiStatus::FailedToEndRsaCalculation,
            HsmErr::RsaModularInverseFailed => DdiStatus::FailedToPerformRsaModularInverse,
            HsmErr::AttestationReportEncodeFailed => DdiStatus::AttestationReportEncodeFailed,
            HsmErr::CoseKeyEncodeFailed => DdiStatus::CoseKeyEncodeFailed,
            HsmErr::AttestKeyInternalErr => DdiStatus::AttestKeyInternalError,
            HsmErr::EcdhComputeFailed => DdiStatus::FailedToComputeEcdhSharedSecret,
            HsmErr::ExpectedIoq => DdiStatus::FailedToIdentifyIoChannelPipe,
            HsmErr::InvalidIoq => DdiStatus::IdentifiedInvalidIoChannelPipe,
            HsmErr::IpcSendFailure => DdiStatus::FailedToSendIpMessage,
            HsmErr::IpcResponseError => DdiStatus::IpcResponseFailure,
            HsmErr::DerAndKeyTypeMismatch => DdiStatus::DerAndKeyTypeMismatch,
            HsmErr::KeyDeriveFailed => DdiStatus::KeyDerivationFailure,
            HsmErr::CannotDeleteAppSessionsOpen => DdiStatus::CannotCloseSomeSessionsInUse,
            HsmErr::AesBulk256InvalidParameter => DdiStatus::DerDecodeFailedForAesBulkKey,
            HsmErr::InvalidIpcShutdownRequest => DdiStatus::InvalidIpcShutdownMessage,
            HsmErr::InvalidCertificate => DdiStatus::InvalidCertificate,
            HsmErr::SessionEncryptionKeyGenerateFailed => {
                DdiStatus::SessionEncryptionKeyGenerateFailed
            }
            HsmErr::PinDecryptionFailed => DdiStatus::PinDecryptionFailed,
            HsmErr::PendingKeyGeneration => DdiStatus::PendingKeyGeneration,
            HsmErr::IoTimeOut => DdiStatus::IoTimedOut,
            HsmErr::DrainBusy => DdiStatus::IoDrainInProgress,
            HsmErr::CannotDeleteInternalKeys => DdiStatus::CannotDeleteInternalKeys,
            HsmErr::DeferredQueueDeleteNotifyErr => DdiStatus::IoChannelPipeDeleteError,
            HsmErr::SoftAesReqSendFailed => DdiStatus::FailedToSendSoftAesRequest,
            HsmErr::IpcResponseDecodeError => DdiStatus::IpcResponseDecodeError,
            HsmErr::ReachedMaxAesBulkKeys => DdiStatus::ReachedMaxAesBulkKeys,
            HsmErr::HmacInvalidInputSize => DdiStatus::HmacInvalidInputSize,
            HsmErr::HmacComputeFailed => DdiStatus::HmacError,
            HsmErr::NonceMismatch => DdiStatus::NonceMismatch,
            HsmErr::EstablishCredEncryptionKeyGenerateFailed => {
                DdiStatus::EstablishCredEncryptionKeyGenerateFailed
            }
            HsmErr::HkdfInvalidInputParam => DdiStatus::HkdfInvalidInputParam,
            HsmErr::HkdfError => DdiStatus::HkdfError,
            HsmErr::KbkdfInvalidInputParam => DdiStatus::KbkdfInvalidInputParam,
            HsmErr::KbkdfError => DdiStatus::KbkdfError,
            HsmErr::InvalidKeyTableIndex => DdiStatus::KeyNotFound,
            HsmErr::UnknownSelfTest => DdiStatus::UnknownSelfTestRequestReceived,
            HsmErr::MissingSelfTestEngineInstance => DdiStatus::SelfTestMissingInstance,
            HsmErr::LoginFailed => DdiStatus::LoginFailed,
            HsmErr::SoftAesInvalidResp => DdiStatus::FailedSoftAesResponse,
            HsmErr::PkaMemoryWipeFailed => DdiStatus::FailedToWipePkaMemory,
            HsmErr::DrainReady => DdiStatus::IoDrainReady,
            HsmErr::KeyStructuralValidationFailed => DdiStatus::KeyStructuralValidationFailed,
            HsmErr::InvalidPorMeasurementDataAccess => DdiStatus::InvalidPackageInfo,
            HsmErr::PctValidationEccGenKeyFailed => DdiStatus::PctValidationEccGenKeyFailed,
            HsmErr::PctValidationEstablishCredEncKeyFailed => {
                DdiStatus::PctValidationEstablishCredEncKeyFailed
            }
            HsmErr::PctValidationSessionEncKeyFailed => DdiStatus::PctValidationSessionEncKeyFailed,
            HsmErr::PctValidationUnwrappingKeyFailed => DdiStatus::PctValidationUnwrappingKeyFailed,
            HsmErr::PctValidationRsaUnwrapEccKeyFailed => {
                DdiStatus::PctValidationRsaUnwrapEccKeyFailed
            }
            HsmErr::PctValidationRsaUnwrapRsaKeyFailed => {
                DdiStatus::PctValidationRsaUnwrapRsaKeyFailed
            }
            HsmErr::NonFipsApprovedMessageDigest => DdiStatus::NonFipsApprovedDigest,
            HsmErr::DigestHashMismatchWithEccCurve => DdiStatus::DigestHashMismatchWithEccCurve,
            HsmErr::UnsupportedDigestHashAlgorithm => DdiStatus::UnsupportedDigestHashAlgorithm,
            HsmErr::BeginEccPointValidationFailed => DdiStatus::FailedToStartPublicKeyValidation,
            HsmErr::EndEccPointValidationFailed => DdiStatus::FailedToEndEccPublicKeyValidation,
            HsmErr::EccPointValidationFailed => DdiStatus::EccPointValidationFailed,
            HsmErr::EccPublicKeyValidationFailed => DdiStatus::EccPublicKeyValidationFailed,
            HsmErr::EccDerKeyShorterThanCurve => DdiStatus::EccDerKeyShorterThanCurve,
            HsmErr::PartitionIdGenerationFailed => DdiStatus::PartitionIdGenerationFailed,
            HsmErr::PartitionCertGenerationFailed => DdiStatus::PartitionCertGenerationFailed,
            HsmErr::PartitionCertInvalidTypeConversion => {
                DdiStatus::PartitionCertInvalidTypeConversion
            }
            HsmErr::FsmAllocationFailed => DdiStatus::FsmAllocationFailed,
            HsmErr::PartitionCertTooLarge => DdiStatus::PartitionCertTooLarge,
            HsmErr::MaskedKeyDecodeFailed => DdiStatus::MaskedKeyDecodeFailed,
            HsmErr::MaskedKeyEncodeFailed => DdiStatus::MaskedKeyEncodeFailed,
            HsmErr::MaskedKeyInvalidLength => DdiStatus::MaskedKeyInvalidLength,
            HsmErr::MaskedKeyPreEncodeFailed => DdiStatus::MaskedKeyPreEncodeFailed,
            HsmErr::InvalidAlgorithm => DdiStatus::InvalidAlgorithm,
            HsmErr::InsufficientBuffer => DdiStatus::InsufficientBuffer,
            HsmErr::InvalidKeyLength => DdiStatus::InvalidKeyLength,
            HsmErr::MetadataEncodeFailed => DdiStatus::MetadataEncodeFailed,
            HsmErr::MetadataDecodeFailed => DdiStatus::MetadataDecodeFailed,
            HsmErr::SessionNeedsRenegotiation => DdiStatus::SessionNeedsRenegotiation,
            HsmErr::BkBootGenerationFailed => DdiStatus::BkBootGenerationFailed,
            HsmErr::MaskingBk3Failed => DdiStatus::MaskingBk3Failed,
            HsmErr::UnmaskingBk3Failed => DdiStatus::UnmaskingBk3Failed,
            HsmErr::MaskingBkBootFailed => DdiStatus::MaskingBkBootFailed,
            HsmErr::UnmaskingBkBootFailed => DdiStatus::UnmaskingBkBootFailed,
            HsmErr::MaskedBkBootNotPresent => DdiStatus::MaskedBkBootNotPresent,
            HsmErr::SealedBk3TooLarge => DdiStatus::SealedBk3TooLarge,
            HsmErr::PartitionAlreadyProvisioned => DdiStatus::PartitionAlreadyProvisioned,
            HsmErr::SealedBk3NotPresent => DdiStatus::SealedBk3NotPresent,
            HsmErr::InvalidAliasKey => DdiStatus::InvalidAliasKey,
            HsmErr::UnmaskUnwrappingKeyNotAllowed => DdiStatus::UnmaskUnwrappingKeyNotAllowed,
            HsmErr::InvalidPartIdPrivKeyInternalError => DdiStatus::InvalidPartitionIdContent,
            HsmErr::PartitionNotProvisioned => DdiStatus::PartitionNotProvisioned,
            HsmErr::CredentialsNotEstablished => DdiStatus::CredentialsNotEstablished,
            HsmErr::Bk3AlreadyInitialized => DdiStatus::Bk3AlreadyInitialized,
            HsmErr::SealedBk3AlreadySet => DdiStatus::SealedBk3AlreadySet,
            HsmErr::PartitionIdKeyGenerationPctFailed => {
                DdiStatus::PartitionIdKeyGenerationPctFailed
            }
            HsmErr::InvalidIpcGetKeyBulkRequest => DdiStatus::InternalError,
        }
    }
}
