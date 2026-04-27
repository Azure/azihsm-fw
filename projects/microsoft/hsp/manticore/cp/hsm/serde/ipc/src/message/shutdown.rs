// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// IDFU drain time info
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes, Clone, Copy, PartialEq, Eq)]
pub struct ShutdownInfo {
    /// IDFU drain time in ms
    pub drain_time_ms: u32,
}

/// Shutdown request message
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageShutdown {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Shutdown command info
    pub info: ShutdownInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageShutdown::LEN],
}
static_assertions::assert_eq_size!(IpcMessageShutdown, IpcMessage);

impl Default for IpcMessageShutdown {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::Shutdown as u32)
                .with_length(IpcMessageShutdown::LEN as u32),
            info: Default::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageShutdown::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageShutdown {
    const OP: IpcMessageOpCode = IpcMessageOpCode::Shutdown;
    const LEN: usize = core::mem::size_of::<ShutdownInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.drain_time_ms, 0..=60000) {
            return Err(IpcMessageErr::InvalidDrainTimeConfig)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageShutdown {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_shutdown_ipc_message() {
        let message = IpcMessageShutdown {
            info: ShutdownInfo {
                drain_time_ms: 0x7D0,
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x04000042);
        assert_eq!(ipc_message.data[1], 0x7D0);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_shutdown_ipc_valid_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04011242;
        ipc_message.data[1] = 0x3E8;

        let result: McrResult<IpcMessageShutdown> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 0x42);
        assert_eq!(message.header.length(), 4);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x12);
        assert_eq!(message.header.status(), 0x1);

        assert_eq!(message.info.drain_time_ms, 0x3E8);
    }

    #[test]
    fn decode_shutdown_ipc_invalid_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04011242;
        ipc_message.data[1] = 0x12345678;

        let result: McrResult<IpcMessageShutdown> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidDrainTimeConfig.into());
        }
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageShutdown> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
