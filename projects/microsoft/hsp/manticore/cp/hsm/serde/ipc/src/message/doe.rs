// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
/// DOE message info
pub struct DoeInfo {
    /// Base address of the DOE message
    pub addr: u32,
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
/// DOE ipc message
pub struct IpcMessageDoe {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// DOE info
    pub info: DoeInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageDoe::LEN],
}
static_assertions::assert_eq_size!(IpcMessageDoe, IpcMessage);

impl Default for IpcMessageDoe {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::Doe as u32)
                .with_length(IpcMessageDoe::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageDoe::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageDoe {
    const OP: IpcMessageOpCode = IpcMessageOpCode::Doe;
    const LEN: usize = core::mem::size_of::<DoeInfo>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageDoe {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_doe_ipc_message() {
        let message = IpcMessageDoe {
            info: DoeInfo { addr: 0x1234ABCD },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x04000040);
        assert_eq!(ipc_message.data[1], 0x1234ABCD);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_doe_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04011240;
        ipc_message.data[1] = 0x5678FEDC;

        let result: McrResult<IpcMessageDoe> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 0x40);
        assert_eq!(message.header.length(), 4);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x12);
        assert_eq!(message.header.status(), 0x1);

        assert_eq!(message.info.addr, 0x5678FEDC);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageDoe> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
