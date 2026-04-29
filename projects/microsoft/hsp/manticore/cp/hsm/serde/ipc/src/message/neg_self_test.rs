// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;
use zerocopy::FromBytes;

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageNegSelfTestReq {
    /// IPC header fields,
    pub header: IpcMessageHeader,

    /// Test ID
    pub test_id: u32,

    /// Reserved
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageNegSelfTestReq::LEN],
}
static_assertions::assert_eq_size!(IpcMessageNegSelfTestReq, IpcMessage);

impl Default for IpcMessageNegSelfTestReq {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::NegativeSelfTest as u32)
                .with_length(IpcMessageNegSelfTestReq::LEN as u32),
            test_id: 0,
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageNegSelfTestReq::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageNegSelfTestReq {
    const OP: IpcMessageOpCode = IpcMessageOpCode::NegativeSelfTest;
    const LEN: usize = core::mem::size_of::<u32>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageNegSelfTestReq {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_default() {
        let msg = IpcMessageNegSelfTestReq::default();
        assert_eq!(
            msg.header.msg_op(),
            IpcMessageOpCode::NegativeSelfTest as u32
        );
        assert_eq!(msg.header.length(), IpcMessageNegSelfTestReq::LEN as u32);
        assert_eq!(msg.test_id, 0);
    }

    #[test]
    fn test_validate() {
        let msg = IpcMessageNegSelfTestReq::default();
        assert!(msg.validate().is_ok());
    }

    #[test]
    fn test_encode() {
        let msg = IpcMessageNegSelfTestReq::default();
        let ipc_msg = msg.encode();
        assert_eq!(ipc_msg.data[0], 0x4000048);
        assert_eq!(ipc_msg.data[1], 0);
    }

    #[test]
    fn test_as_bytes() {
        let msg = IpcMessageNegSelfTestReq {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::NegativeSelfTest as u32)
                .with_length(IpcMessageNegSelfTestReq::LEN as u32),
            test_id: 0x12345678,
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageNegSelfTestReq::LEN],
        };
        let bytes = msg.as_bytes();
        assert_eq!(
            bytes.len(),
            IPC_MESSAGE_PAYLOAD_LEN + size_of::<IpcMessageHeader>()
        );
        assert_eq!(bytes[0], 0x48);
        assert_eq!(bytes[1], 0);
        assert_eq!(bytes[2], 0);
        assert_eq!(bytes[3], 4);
        assert_eq!(bytes[4], 0x78);
        assert_eq!(bytes[5], 0x56);
        assert_eq!(bytes[6], 0x34);
        assert_eq!(bytes[7], 0x12);
        for val in bytes
            .iter()
            .take(IPC_MESSAGE_PAYLOAD_LEN + size_of::<IpcMessageHeader>())
            .skip(8)
        {
            assert_eq!(*val, 0);
        }
    }
}
