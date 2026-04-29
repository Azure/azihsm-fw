// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

// CDMA ECC Error Sub Op Code
#[repr(u8)]
#[open_enum]
#[derive(Debug, IntoBytes, FromBytes, Immutable)]
pub enum CdmaErrSubOpCode {
    /// 0x0: Abort I/O command
    AbortIO = 0x0,

    /// 0x1: CDMA ECC Error correction command
    EccCorrectionCmd = 0x1,
}

/// CMDA Ecc Error IPC message Error info
#[repr(C)]
#[derive(Debug, IntoBytes, FromBytes, Immutable)]
pub struct IpcMessageCdmaErrInfo {
    /// Sub Op Code for CDMA Ecc Error
    pub op_code: CdmaErrSubOpCode,

    /// Reserved Padding to make Sub Op Code 4 bytes
    pub _rsvd: [u8; 3],
}

/// CDMA Ecc Error Ipc Message
#[repr(C)]
#[derive(Debug, IntoBytes, FromBytes, Immutable)]
pub struct IpcMessageCdmaErr {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// CDMA Sub Op code
    pub info: IpcMessageCdmaErrInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaErr::LEN],
}
static_assertions::assert_eq_size!(IpcMessageCdmaErr, IpcMessage);

impl Default for IpcMessageCdmaErr {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageCdmaErr::OP as u32)
                .with_length(IpcMessageCdmaErr::LEN as u32),
            info: IpcMessageCdmaErrInfo {
                op_code: CdmaErrSubOpCode::EccCorrectionCmd,
                _rsvd: [0; 3],
            },
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaErr::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageCdmaErr {
    const OP: IpcMessageOpCode = IpcMessageOpCode::CdmaEccErr;
    const LEN: usize = core::mem::size_of::<IpcMessageCdmaErrInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.op_code, CdmaErrSubOpCode::EccCorrectionCmd) {
            return Err(IpcMessageErr::InvalidErrReqSubOpCode)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageCdmaErr {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_cdma_err_ipc_message() {
        let message = IpcMessageCdmaErr::default();

        let ipc_message = message.encode();

        // Message header; Message len of 4 (0x04) bytes, IpcMessageCdmaErr opcode is 0x1
        assert_eq!(ipc_message.data[0], 0x04000001);
        // Message sub opcode for correctable CDMA ECC error is 0x1
        assert_eq!(ipc_message.data[1], 0x00000001);
        // Reserved/unused section of message should be all 0x0 by default
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_err_log_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        // message header
        ipc_message.data[0] = 0x04011201;
        // sub opcode
        ipc_message.data[1] = 0x00000001;

        let result: McrResult<IpcMessageCdmaErr> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        // Check message header values
        assert_eq!(message.header.msg_op(), 0x01);
        assert_eq!(message.header.length(), 4);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x12);
        assert_eq!(message.header.status(), 0x1);

        // Check sub opcode
        assert_eq!(message.info.op_code, CdmaErrSubOpCode::EccCorrectionCmd);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageCdmaErr> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }

    #[test]
    fn test_validate_failure() {
        let mut message = IpcMessageCdmaErr::default();

        message.info.op_code = CdmaErrSubOpCode::AbortIO; // Set an invalid sub opcode

        assert!(message.validate().is_err());
    }
}
