// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod decoder;
mod encoder;
mod message;

use bitfield_struct::bitfield;
pub use decoder::IpcMessageDecoder;
pub use encoder::IpcMessageEncoder;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_ipc_controller::IpcMessage;
use mcr_types::DevCqId;
use mcr_types::DevSqId;
use mcr_types::IoChannelId;
use mcr_types::IoControllerId;
use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

pub const IPC_MESSAGE_PAYLOAD_LEN: usize = 60;
pub const IPC_HEADER_LEN_IN_BYTES: usize = 4;

pub use encoder::IpcMessageEncoderTrait;
pub use message::cdma_err::*;
pub use message::cdma_io::*;
pub use message::cert_chain::*;
pub use message::doe::*;
pub use message::get_bulk_key::*;
pub use message::key_update::*;
pub use message::neg_self_test::*;
pub use message::pfn_enable_disable::*;
pub use message::rsa_key_gen::*;
pub use message::set_res::*;
pub use message::shutdown::*;
pub use message::sq_create_delete::*;
pub use message::state_change::*;
pub use message::stop_interface::*;
pub use message::tdisp_int::*;
pub use message::trigger_crash::*;
pub use message::trigger_stack_validation::*;
pub use message::ucd_query::*;

/// Various states of IO processing cores
#[repr(u32)]
#[open_enum]
pub enum IoProcessorBootState {
    /// Io processor boot start phase
    Start = 1,

    /// Io Processor boot phase completed
    Done = 2,

    /// Io Processor is in run state
    Run = 3,
}

/// IPC Message Types
#[derive(Default)]
pub enum IpcMessageOpCode {
    /// Notify State change to IO Cores.
    /// Equivalent FP message code identifier: MSG_OP_FP_STATUS_CHANGE
    #[default]
    StateChange = 0x0,

    /// CDMA Vault ECC Error
    /// Equivalent FP message code identifier: MSG_OP_ERR_REQ
    CdmaEccErr = 0x1,

    /// Create or delete submission queue.
    /// Equivalent FP message code identifier: MSG_OP_VF_SLOT_SQ2CQ_MAP_UPDATE
    CreateDeleteSq = 0x3,

    /// Pcie Function enable and disable
    /// Equivalent FP message code identifier: MSG_OP_VF_UPDATE
    PfnEnableDisable = 0x5,

    /// Send CDMA IO
    CdmaIo = 0x6,

    /// AES key update
    /// Equivalent FP message code identifier: MSG_OP_KEY_UPDATE
    AesKeyUpdate = 0x7,

    /// FP Error Log
    /// Equivalent FP message code identifier: MSG_OP_FP_ERR_LOG
    FpErrLog = 0x9,

    /// Get Io Controller channel allocation config for IO Cores.
    /// Equivalent FP message code identifier: MSG_OP_UCD_QUERY
    UcdQuery = 0xa,

    /// CDMA Stat Set
    /// Equivalent FP message code identifier: MSG_OP_CDMA_STAT_SET
    CdmaStatSet = 0xd,

    /// Send and receive a DOE message
    Doe = 0x40,

    /// Send and receive a Shutdown message
    Shutdown = 0x42,

    /// Stop a list of Pcie function interface
    StopInterface = 0x43,

    /// Rsa Key Generation
    RsaKeyGen = 0x44,

    /// Get Cert Chain Lengths
    GetCertChainLengths = 0x45,

    /// Get Cert
    GetCert = 0x46,

    /// Trigger Crash Dump
    TriggerCrash = 0x47,

    /// Negative self test
    NegativeSelfTest = 0x48,

    /// TDISP Interrupt
    TdispInterrupt = 0x49,

    /// Get AES256 Bulk Key
    GetBulkKey = 0x4A,

    /// Test Stack Validation
    TriggerStackValidation = 0x4B,

    /// Set Resource
    SetResource = 0x7f,
}

impl TryFrom<u8> for IpcMessageOpCode {
    type Error = u32;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        let val = match value {
            0x00 => IpcMessageOpCode::StateChange,
            0x01 => IpcMessageOpCode::CdmaEccErr,
            0x03 => IpcMessageOpCode::CreateDeleteSq,
            0x05 => IpcMessageOpCode::PfnEnableDisable,
            0x06 => IpcMessageOpCode::CdmaIo,
            0x07 => IpcMessageOpCode::AesKeyUpdate,
            0x09 => IpcMessageOpCode::FpErrLog,
            0x0A => IpcMessageOpCode::UcdQuery,
            0x0D => IpcMessageOpCode::CdmaStatSet,
            0x40 => IpcMessageOpCode::Doe,
            0x42 => IpcMessageOpCode::Shutdown,
            0x43 => IpcMessageOpCode::StopInterface,
            0x44 => IpcMessageOpCode::RsaKeyGen,
            0x45 => IpcMessageOpCode::GetCertChainLengths,
            0x46 => IpcMessageOpCode::GetCert,
            0x47 => IpcMessageOpCode::TriggerCrash,
            0x48 => IpcMessageOpCode::NegativeSelfTest,
            0x49 => IpcMessageOpCode::TdispInterrupt,
            0x4A => IpcMessageOpCode::GetBulkKey,
            0x4B => IpcMessageOpCode::TriggerStackValidation,
            0x7F => IpcMessageOpCode::SetResource,
            _ => Err(IpcMessageErr::InvalidOpcodeConversion)?,
        };

        Ok(val)
    }
}

/// Interface to be implemented by each IPC message
pub trait IpcMessageType {
    /// Ipc message opcode
    const OP: IpcMessageOpCode;

    /// Ipc message length
    const LEN: usize;

    /// Validate the message
    ///
    /// # Returns
    ///
    /// True if the message is valid, false otherwise
    fn validate(&self) -> McrResult<()>;
}

/// IPC Message Header Field
#[bitfield(u32)]
#[derive(Default, IntoBytes, Immutable, FromBytes, PartialEq, Eq)]
pub struct IpcMessageHeader {
    /// Message operation
    #[bits(7)]
    pub msg_op: u32,

    /// Request: false (0), Response: true (1)
    pub response: bool,

    /// Software tag to track the messages
    #[bits(8)]
    pub tag: u32,

    /// Message status
    #[bits(4)]
    pub status: u32,

    /// Bit map records message sent to which CPU.
    #[bits(2)]
    pub submit_map: u32,

    /// Bit map records message completed by which CPU.
    #[bits(2)]
    pub complete_map: u32,

    /// Length of the data field
    #[bits(8)]
    pub length: u32,
}

/// IPC Message status code
#[repr(u32)]
#[derive(PartialEq, Clone, Copy)]
pub enum IpcMessageStatusCode {
    /// Success status code
    Success = 0,

    /// Unsupported value in the message opcode field
    MessageNotSupported = 1,

    /// Invalid Field in the message
    InvalidField = 2,

    /// Function Not Enabled
    FunctionNotEnabled = 3,

    /// Operation timed out
    OperationTimeout = 4,

    /// Operation failed
    OperationFailed = 5,

    /// Operation Pending
    Pending = 6,

    /// Unknown Status code
    UnknownStatus = 0xF,
}

impl From<IpcMessageStatusCode> for u32 {
    fn from(value: IpcMessageStatusCode) -> Self {
        value as Self
    }
}

impl From<u32> for IpcMessageStatusCode {
    fn from(value: u32) -> Self {
        match value {
            x if x == IpcMessageStatusCode::Success.into() => IpcMessageStatusCode::Success,
            x if x == IpcMessageStatusCode::MessageNotSupported.into() => {
                IpcMessageStatusCode::MessageNotSupported
            }
            x if x == IpcMessageStatusCode::InvalidField.into() => {
                IpcMessageStatusCode::InvalidField
            }
            x if x == IpcMessageStatusCode::FunctionNotEnabled.into() => {
                IpcMessageStatusCode::FunctionNotEnabled
            }
            x if x == IpcMessageStatusCode::OperationTimeout.into() => {
                IpcMessageStatusCode::OperationTimeout
            }
            x if x == IpcMessageStatusCode::OperationFailed.into() => {
                IpcMessageStatusCode::OperationFailed
            }
            x if x == IpcMessageStatusCode::Pending.into() => IpcMessageStatusCode::Pending,
            _ => IpcMessageStatusCode::UnknownStatus,
        }
    }
}

mcr_err_decl! {
    IpcMessage,
    IpcMessageErr
    {
        // Invalid IO state received
        InvalidIoStateValue = 1,

        // Invalid Ucd Query info received
        InvalidChannelId = 2,

        // Invalid Ucd Query info received
        InvalidControllerId = 3,

        // Invalid Pcie Function Enable Disable info received
        InvalidPcieFnId = 4,

        // Invalid Pcie Function Enable Disable action received
        InvalidPfnEnableDisableAction = 5,

        // Invalid Device Completion queue Id
        InvalidDeviceCqId = 6,

        // Invalid Device Submission queue Id
        InvalidDeviceSqId = 7,

        // Invalid Submission queue action
        InvalidSqAction = 8,

        // Invalid Submission queue priority
        InvalidSqPriority = 9,

        // Invalid Key update action
        InvalidKeyUpdateAction = 10,

        // Invalid input message passed to decode
        InvalidInputMessageForDecode = 11,

        // Invalid message header found during decode
        InvalidMessageHeaderDecode = 12,

        // Invalid message type found during decode
        InvalidOpcodeConversion = 13,

        // Invalid drain time config found
        InvalidDrainTimeConfig = 14,

        // Invalid FP error log length
        InvalidFpErrLogLen = 15,

        // Invalid interrupt source
        InvalidInterruptSource = 16,

        // Invalid Err Request Sub Op Code
        InvalidErrReqSubOpCode = 17,

        // Invalid Cdma Stat Set Action
        InvalidStatSetAction = 18,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn try_u8_from_ipc_message_opcode() {
        assert!(IpcMessageOpCode::try_from(0x0).is_ok());
        assert!(IpcMessageOpCode::try_from(0x1).is_ok());
        assert!(IpcMessageOpCode::try_from(0x03).is_ok());
        assert!(IpcMessageOpCode::try_from(0x05).is_ok());
        assert!(IpcMessageOpCode::try_from(0x07).is_ok());
        assert!(IpcMessageOpCode::try_from(0x09).is_ok());
        assert!(IpcMessageOpCode::try_from(0x0A).is_ok());
        assert!(IpcMessageOpCode::try_from(0x0D).is_ok());
        assert!(IpcMessageOpCode::try_from(0x42).is_ok());
        assert!(IpcMessageOpCode::try_from(0x10).is_err());
    }
}
