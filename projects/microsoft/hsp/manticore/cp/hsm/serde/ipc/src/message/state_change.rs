// Copyright (c) Microsoft Corporation. All rights reserved.

use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Various states of IO processing cores
#[repr(u8)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes)]
pub enum IoProcessorState {
    /// Normal boot from a POR
    NormalBoot = 3,

    /// Move the IO cores to start state to perform IO operations
    Start = 6,

    /// iDFU WAIT_PREPARE_RELEASE state
    PrepareRelease = 10,

    /// iDFU WAIT_RELEASE state
    Release = 11,
}

/// Io state change message
/// Equivalent FP message code identifier: MSG_OP_FP_STATUS_CHANGE
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageIoStateChange {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Io Processor state field
    pub state: IoProcessorState,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageIoStateChange::LEN],
}
static_assertions::assert_eq_size!(IpcMessageIoStateChange, IpcMessage);

impl Default for IpcMessageIoStateChange {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::StateChange as u32)
                .with_length(IpcMessageIoStateChange::LEN as u32),
            state: IoProcessorState::NormalBoot,
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageIoStateChange::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageIoStateChange {
    const OP: IpcMessageOpCode = IpcMessageOpCode::StateChange;
    const LEN: usize = core::mem::size_of::<IoProcessorState>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(
            self.state,
            IoProcessorState::NormalBoot
                | IoProcessorState::Start
                | IoProcessorState::PrepareRelease
                | IoProcessorState::Release
        ) {
            return Err(IpcMessageErr::InvalidIoStateValue)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageIoStateChange {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_state_change_ipc_message() {
        let message = IpcMessageIoStateChange {
            state: IoProcessorState::NormalBoot,
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x01000000);
        assert_eq!(ipc_message.data[1], 0x03);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_state_ipc_change_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x01002000;
        ipc_message.data[1] = 0x03;

        let result: McrResult<IpcMessageIoStateChange> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
        let message = result.unwrap();

        assert!(message.state == IoProcessorState::NormalBoot);
    }

    #[test]
    fn decode_invalid_state_ipc_change_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x01002000;
        ipc_message.data[1] = 0x04;

        let result: McrResult<IpcMessageIoStateChange> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_err());
    }

    #[test]
    fn decode_header_for_state_change_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x01052080;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x00);
        assert!(header.response());
        assert_eq!(header.tag(), 0x20);
        assert_eq!(header.status(), 0x05);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x1);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageIoStateChange> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
