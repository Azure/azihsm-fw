// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_error::mcr_err_decl;
use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::CmdFsmError;

mcr_err_decl! {
    Admin,
    AdminErr
    {
        // Operation is pending
        Pending = 0x00,

        // Sending an Ipc event on a Ipc event channel that is not configured
        InvalidStateChangeIpcResponse = 0x01,

        // Ipc Tx Message queue full
        InvalidUcdQueryIpcResponse = 0x02,

        // Queue Controller interface error codes
        // Set Resource Count limit exceeded
        SetResCountLimitExceeded = 0x03,

        // Set Resource Count issued on enabled controller
        SetResCountOnEnabled = 0x04,

        // Clear Resource Count issued on enabled controller
        ClearResCountOnEnabled = 0x05,

        // Create Submission Queue issued on diabled controller
        CreateSqOnDisabled = 0x6,

        // Create Completion Queue issued on diabled controller
        CreateCqOnDisabled = 0x7,

        // Failed to create Admin Submission Queue
        AdminSqCreateFailed = 0x08,

        // Failed to identify a matching submission queue to delete
        InvalidHostSqToDelete = 0x09,

        // Failed to identify a matching completion queue to delete
        InvalidHostCqToDelete = 0x0A,

        // Requested completion queue to delete have outstanding refrence count
        DeleteCqReferenceCountNotZero = 0x0B,

        // Failed to identify a matching admin submission queue to delete
        InvalidAdminSqToDelete = 0x0C,

        // Failed to identify a matching admin completion queue to delete
        InvalidAdminCqToDelete = 0x0D,

        // Requested admin completion queue to delete have outstanding refrence count
        DeleteAdminCqReferenceCountNotZero = 0x0E,

        // Invalid completion queue Id
        InvalidQueueId = 0x0F,

        // Host Completion queue is out of range for the function
        QueueIdOutOfRangeForFunction = 0x10,

        // Host Completion is already allocated
        CqAlreadyAllocated = 0x11,

        // Host Completion not available
        CqNotAvailable = 0x12,

        // Host Submission Queue already created
        SqAlreadyAllocated = 0x13,

        // Invalid Submission Queue Id
        InvalidSqId = 0x14,

        // Invalid Completion Queue delete
        InvalidCompletionQueueIdInDelete = 0x15,

        // Create Completion Queue Failed by Queue Controller
        CreateCqFailedByQueueController = 0x16,

        // Create Submission Queue Failed by Queue Controller
        CreateSqFailedByQueueController = 0x17,

        // Invalid Field in create completion queue command
        InvalidFieldInCreateCqCmd = 0x18,

        // Invalid Completion queue delete
        InvalidQueueDelete = 0x19,

        // Invalid Queue Size
        InvalidQueueSize = 0x1A,

        // Invalid interrupt vector
        InvalidInterruptVector = 0x1B,

        // Invalid Host Cq in Create Sq
        InvalidHostCq = 0x1C,

        // Invalid Field in the Create Sq command
        InvalidFieldInCreateSqCmd = 0x1D,

        // Invalid Field in the GetSet Features Command
        InvalidFieldInGetSetFeaturesCmd = 0x1E,

        // Invalid controller state change requested
        InvalidStateChange = 0x1F,

        // Resource Manager interface error codes
        // Submission queue already exist
        AllocSqQueueAlreadyAllocated = 0x20,

        // No associated completion queue found for a requested submission queue
        AllocSqCompletionQueueNotFound = 0x21,

        // Submission queue allocated from a different Queue group than completion queue
        AllocSqQueueGroupMismatch = 0x22,

        // Submission queue allocation limit reached
        AllocSqResourceLimitReached = 0x23,

        // Invalid submission queue requested to be freed
        InvalidHostSqToFree = 0x24,

        // Host submission queue not found
        HostSubmissionQueueNotFound = 0x25,

        // Completion queue already exist
        AllocCqQueueAlreadyAllocated = 0x26,

        // Completion queue allocation limit reached
        AllocCqResourceLimitReached = 0x27,

        // Invalid completion queue requested to be freed
        InvalidHostCqToFree = 0x28,

        // Host completion queue not found
        HostCompletionQueueNotFound = 0x29,

        // Device completion queue not found
        DeviceCompletionQueueNotFound = 0x2A,

        // Invalid Queue Controller Id
        InvalidQueueControllerId = 0x2B,

        // Invalid Get Set Feature Id
        InvalidFeatureId = 0x2C,

        // Io Channel receive calls returned None
        IoChannelRecvNone = 0x30,

        // Io Channel receive error
        IoChannelRecvErr = 0x31,

        // Io Channel send error
        IoChannelSendError = 0x32,

        // Unknown SQE opcode
        SqeUnknownOp = 0x33,

        // DMA output transfer error
        DmaOutError = 0x34,

        // Admin Command FSM not found
        ExpectedCmdFsm = 0x35,

        // DMA transfer start error
        DmaStartError = 0x36,

        // No valid PCIe function found
        ExpectedPcieFn = 0x37,

        // DMA output buffer not found
        ExpectedDmaBuf = 0x38,

        // Cqe not found
        ExpectedCqe = 0x39,

        // Dma completion with error status
        DmaEndErr = 0x3A,

        // Dma completion tag mismatch
        DmaTagMismatch = 0x3B,

        // Empty DMA completions
        DmaCompletionEmpty = 0x3C,

        // No DMA memory available
        NoMemory = 0x3D,

        // Invalid PCIe function supplied from Rx Entry
        InvalidPcieFn = 0x3E,

        // Io Channel End Send returned None
        IoChannelSendCompleteNone = 0x3F,

        // Io Channel End Send returned None
        IoChannelSendCompleteError = 0x40,

        // Io Channel End Send Tag Mismatch
        IoTagMismatch = 0x41,

        // Invalid Set Resource Command for the controller
        InvalidSetResCmd = 0x42,

        // Invalid field found in Sqe body
        InvalidCntrlIdFieldInSqe = 0x43,

        // Invalid field found in Sqe body
        InvalidResCountFieldInSqe = 0x44,

        // Ipc send request error
        IpcSendRequestError = 0x45,

        // Invalid IPC response
        IpcResponseError = 0x46,

        // Invalid IPC heder
        InvalidIpcHeader = 0x47,

        // Expected IPC channel
        ExpectedIpcChannel = 0x48,

        // Spurious IPC Message
        SpuriousIpcMessage = 0x49,

        // Admin Queue object expected
        ExpectedAdminQueue = 0x4A,

        // Invalid Admin Queue
        InvalidAdminQueue = 0x4B,

        // Sending reset event to FP CPU failed
        SendResetEventToFpFailed = 0x4C,

        // Sending reset event to HSM CPU failed
        SendResetEventToHsmFailed = 0x4D,

        // Invalid IPC Shutdown request message
        InvalidIpcShutdownRequest = 0x4E,

        // Invalid IPC message opcode
        InvalidIpcMessageOpcode = 0x4F,

        // Resource Limit reached
        ResourceLimitReached = 0x50,

        // Resource allocation not possible since the function is already running
        SetResFunctionAlreadyRunning = 0x51,

        // Spurious IO queue delete response
        SpuriousIoQueueDeleteResp = 0x52,

        // Failed to send Self Test Request
        FailedToSendSelfTestReq = 0x53,

        // Invalid Self Test Response
        FailedToRecvSelfTestResp = 0x54,

        // Self test failure
        SelfTestFailed = 0x55,

        // Self test timeout
        SelfTestTimeout = 0x56,

        // CDMA IO AES GCM self test failure
        CdmaIoAesGcmSelfTestFailed = 0x57,

        // CDMA IO AES XTS self test failure
        CdmaIoAesXtsSelfTestFailed = 0x58,

        // CDMA IO Key Update failure
        CdmaIoKeyUpdateFailed = 0x59,

        // AES Key Unwrap Self Test Failure
        AesUnwrapSelfTestFailed = 0x5A,

        // AES ECB self test failure
        AesEcbSelfTestFailed = 0x5B,

        // CDMA IO Invalid Cast State
        CdmaIoInvalidCastState = 0x5C,

        // Invalid preop CAST test request
        InvalidPreopCastTestRequest = 0x5D,

        // Invalid event received
        InvalidEvent = 0x5E,

        // Invalid LM context to restore
        InvalidLmContextToRestore = 0x5F,

        // Invalid Resource ID
        InvalidResourceId = 0x60,

        // Invalid Stop Interface request message
        InvalidStopInterfaceRequest = 0x61,

        // HSM Boot Timeout
        HsmBootTimeout = 0x62,

        // AES GCM unaligned data tag correction failure
        AesGcmTagCorrectionFailed = 0x63,

        // Invalid AES GCM unaligned data pointer
        InvalidAesGcmUnalignedDataPtr = 0x64,

        // Invalid source PF for VF Prepare command
        InvalidSourcePfn = 0x65,
    }
}

impl CmdFsmError for AdminErr {
    fn pending(&self) -> bool {
        self == &Self::Pending
    }
}

macro_rules! host_status_code {
    ($type:ident, $code:literal) => {
        (HostStatusCodeType::$type << 8) | $code
    };
}
struct HostStatusCodeType {}

impl HostStatusCodeType {
    // Generic Status Code Type
    const GENERIC: u16 = 0x0;

    // Generic Status Code Type with Do Not Retry
    const GENERIC_DNR: u16 = 0x40;

    // Command Specific Status Code Type with Do Not Retry
    const COMMAND_SPECIFC_DNR: u16 = 0x41;
}

/// Admin Completion Queue Entry Status Code reported to host
#[repr(u16)]
#[open_enum]
#[derive(Debug, Default, IntoBytes, Immutable, FromBytes)]
pub(crate) enum HostStatusCode {
    /// Success status code
    Success = host_status_code!(GENERIC, 0x00),

    /// Unsupported value in the command opcode field
    InvalidCommandOpCode = host_status_code!(GENERIC_DNR, 0x01),

    /// Unsupported value in the command field (other than the opcode field)
    InvalidFieldInCommand = host_status_code!(GENERIC_DNR, 0x02),

    /// Transferring the data or metadata associated with a command had an error
    DataTransferError = host_status_code!(GENERIC_DNR, 0x04),

    /// Internal controller error
    InternalError = host_status_code!(GENERIC_DNR, 0x07),

    /// Completion Queue Invalid
    ///
    /// # Notes
    ///
    /// Affected command: Create I/O Submission Queue
    InvalidCqId = host_status_code!(COMMAND_SPECIFC_DNR, 0x00),

    /// Invalid Queue Identifier
    ///
    /// # Notes
    ///
    /// Affected command: Create I/O Submission Queue, Create I/O
    /// Completion Queue, Delete I/O Completion Queue, Delete I/O
    /// Submission Queue
    InvalidQueueId = host_status_code!(COMMAND_SPECIFC_DNR, 0x01),

    /// Invalid Queue size
    ///
    /// # Notes
    ///
    /// Affected command: Create I/O Submission Queue, Create I/O
    /// Completion Queue
    InvalidQueueSize = host_status_code!(COMMAND_SPECIFC_DNR, 0x02),

    /// Invalid Interrupt Vector
    ///
    /// # Notes
    ///
    /// Affected command: Create I/O Completion Queue
    InvalidInterruptVector = host_status_code!(COMMAND_SPECIFC_DNR, 0x08),

    /// Invalid Queue Deletion
    ///
    /// # Notes
    ///
    /// Affected command: Delete I/O Completion Queue
    InvalidQueueDelete = host_status_code!(COMMAND_SPECIFC_DNR, 0x0C),

    /// Feature Identifier Not Saveable
    ///
    /// # Notes
    ///
    /// Affected command: Set Features
    FeatureNotSaveable = host_status_code!(COMMAND_SPECIFC_DNR, 0x0D),

    /// Invalid Number of Resource Count Requested
    ///
    /// # Notes
    ///
    /// Affected command: Set Resource Count
    InvalidResourceCnt = host_status_code!(COMMAND_SPECIFC_DNR, 0xC0),
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
            x if x == HostStatusCode::DataTransferError.into() => HostStatusCode::DataTransferError,
            x if x == HostStatusCode::InvalidCqId.into() => HostStatusCode::InvalidCqId,
            x if x == HostStatusCode::InvalidQueueId.into() => HostStatusCode::InvalidQueueId,
            x if x == HostStatusCode::InvalidQueueSize.into() => HostStatusCode::InvalidQueueSize,
            x if x == HostStatusCode::InvalidQueueDelete.into() => {
                HostStatusCode::InvalidQueueDelete
            }
            x if x == HostStatusCode::FeatureNotSaveable.into() => {
                HostStatusCode::FeatureNotSaveable
            }
            x if x == HostStatusCode::InvalidResourceCnt.into() => {
                HostStatusCode::InvalidResourceCnt
            }
            x if x == HostStatusCode::Success.into() => HostStatusCode::Success,
            _ => HostStatusCode::InternalError,
        }
    }
}

impl From<AdminErr> for HostStatusCode {
    fn from(value: AdminErr) -> Self {
        match value {
            AdminErr::SqeUnknownOp | AdminErr::InvalidSetResCmd | AdminErr::InvalidFeatureId => {
                HostStatusCode::InvalidCommandOpCode
            }
            AdminErr::InvalidCntrlIdFieldInSqe
            | AdminErr::InvalidResCountFieldInSqe
            | AdminErr::InvalidFieldInGetSetFeaturesCmd
            | AdminErr::InvalidSourcePfn => HostStatusCode::InvalidFieldInCommand,
            AdminErr::DmaOutError
            | AdminErr::DmaStartError
            | AdminErr::DmaEndErr
            | AdminErr::DmaTagMismatch
            | AdminErr::DmaCompletionEmpty => HostStatusCode::DataTransferError,
            AdminErr::HostCompletionQueueNotFound | AdminErr::InvalidQueueId => {
                HostStatusCode::InvalidQueueId
            }
            AdminErr::InvalidHostCq => HostStatusCode::InvalidCqId,
            AdminErr::InvalidQueueDelete => HostStatusCode::InvalidQueueDelete,
            AdminErr::InvalidQueueSize => HostStatusCode::InvalidQueueSize,
            AdminErr::InvalidInterruptVector => HostStatusCode::InvalidInterruptVector,
            _ => HostStatusCode::InternalError,
        }
    }
}
