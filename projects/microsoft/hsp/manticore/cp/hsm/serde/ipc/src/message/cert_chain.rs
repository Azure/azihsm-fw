// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

// Maximum number of certificates in the chain due to IPC message size constraints
pub const MAX_DEVICE_ID_CERTS: usize = 13;

#[repr(C, packed)]
#[derive(IntoBytes, Immutable, Clone, Copy, FromBytes)]
pub struct GetCertChainLengthsPayload {
    /// Hash of the cert chain
    pub hash: [u8; 32],

    /// Number of certificates in the chain
    pub num_certs: u8,

    /// Array of cert chain info
    pub cert_lengths: [u16; MAX_DEVICE_ID_CERTS],
}

#[repr(C, packed)]
#[derive(IntoBytes, Immutable, FromBytes)]
/// Get Cert Chain Lengths ipc message
pub struct IpcMessageGetCertChainLengths {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Get Cert Chain Length info
    pub info: GetCertChainLengthsPayload,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetCertChainLengths::LEN],
}
static_assertions::assert_eq_size!(IpcMessageGetCertChainLengths, IpcMessage);

impl Default for IpcMessageGetCertChainLengths {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::GetCertChainLengths as u32)
                .with_length(IpcMessageGetCertChainLengths::LEN as u32),
            info: GetCertChainLengthsPayload {
                hash: Default::default(),
                num_certs: Default::default(),
                cert_lengths: Default::default(),
            },
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetCertChainLengths::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageGetCertChainLengths {
    const OP: IpcMessageOpCode = IpcMessageOpCode::GetCertChainLengths;
    const LEN: usize = core::mem::size_of::<GetCertChainLengthsPayload>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageGetCertChainLengths {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[repr(C, packed)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct GetCertPayload {
    /// Cert ID (input)
    pub cert_id: u8,

    /// Actual cert length (output)
    pub cert_len: u16,

    /// Cert buffer address (input)
    pub addr: u64,

    // size of the allocated buffer (input)
    pub buf_size: u16,
}

#[repr(C, packed)]
#[derive(IntoBytes, Immutable, FromBytes)]
/// Get Cert ipc message
pub struct IpcMessageGetCert {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Get Cert info
    pub info: GetCertPayload,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetCert::LEN],
}
static_assertions::assert_eq_size!(IpcMessageGetCert, IpcMessage);

impl Default for IpcMessageGetCert {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::GetCert as u32)
                .with_length(IpcMessageGetCert::LEN as u32),
            info: GetCertPayload {
                cert_id: Default::default(),
                cert_len: Default::default(),
                addr: Default::default(),
                buf_size: Default::default(),
            },
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetCert::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageGetCert {
    const OP: IpcMessageOpCode = IpcMessageOpCode::GetCert;
    const LEN: usize = core::mem::size_of::<GetCertPayload>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageGetCert {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn test_ipc_message_get_cert_chain_lengths_size() {
        assert_eq!(
            core::mem::size_of::<IpcMessageGetCertChainLengths>(),
            IPC_MESSAGE_LENGTH * core::mem::size_of::<u32>()
        );
    }

    #[test]
    #[allow(clippy::unusual_byte_groupings)]
    fn test_ipc_message_get_chain_lengths_fields_encode() {
        let mut msg = IpcMessageGetCertChainLengths::default();
        msg.info.num_certs = 5;
        for i in 0..5 {
            msg.info.cert_lengths[i] = (0x1234 + i) as u16;
        }

        let ipc_message = msg.encode();
        assert_eq!(ipc_message.data[0], 0x03B000045);
        assert_eq!(ipc_message.data[9], 0x35_1234_05);
        assert_eq!(ipc_message.data[10], 0x37_1236_12);
        assert_eq!(ipc_message.data[11], 0x00_1238_12);
    }

    #[test]
    #[allow(clippy::unusual_byte_groupings)]
    fn test_ipc_message_get_chain_lengths_fields_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x03B000045;
        ipc_message.data[9] = 0x35_1234_05;
        ipc_message.data[10] = 0x37_1236_12;
        ipc_message.data[11] = 0x00_1238_12;

        let result: McrResult<IpcMessageGetCertChainLengths> =
            IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
        let decoded_msg = result.unwrap();
        let decoded_header = decoded_msg.header;
        assert_eq!(
            decoded_header.msg_op(),
            IpcMessageOpCode::GetCertChainLengths as u32
        );
        assert_eq!(
            decoded_header.length(),
            IpcMessageGetCertChainLengths::LEN as u32
        );
        let decoded_payload = decoded_msg.info;
        assert_eq!(decoded_payload.num_certs, 5);
        for i in 0..5 {
            let len = decoded_payload.cert_lengths[i];
            assert_eq!(len, (0x1234 + i) as u16);
        }
    }

    #[test]
    fn test_ipc_message_get_cert_size() {
        assert_eq!(
            core::mem::size_of::<IpcMessageGetCert>(),
            IPC_MESSAGE_LENGTH * core::mem::size_of::<u32>()
        );
    }

    #[test]
    #[allow(clippy::unusual_byte_groupings)]
    fn test_ipc_message_get_cert_fields_encode() {
        let mut msg = IpcMessageGetCert::default();
        msg.info.cert_id = 0x12;
        msg.info.cert_len = 0;
        msg.info.addr = 0x5678ABCD;
        msg.info.buf_size = 0x1234;

        let ipc_message = msg.encode();
        assert_eq!(ipc_message.data[0], 0x0D000046);
        assert_eq!(ipc_message.data[1], 0xCD_0000_12);
        assert_eq!(ipc_message.data[2], 0x005678AB);
        assert_eq!(ipc_message.data[3], 0x34_000000);
        assert_eq!(ipc_message.data[4], 0x000000_12);
    }

    #[test]
    #[allow(clippy::unusual_byte_groupings)]
    fn test_ipc_message_get_cert_fields_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x0D000046;
        ipc_message.data[1] = 0xCD_0000_12;
        ipc_message.data[2] = 0x005678AB;
        ipc_message.data[3] = 0x34_000000;
        ipc_message.data[4] = 0x000000_12;

        let result: McrResult<IpcMessageGetCert> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
        let decoded_msg = result.unwrap();
        let decoded_header: IpcMessageHeader = decoded_msg.header;
        assert_eq!(decoded_header.msg_op(), IpcMessageOpCode::GetCert as u32);
        assert_eq!(decoded_header.length(), IpcMessageGetCert::LEN as u32);
        let decoded_payload: GetCertPayload = decoded_msg.info;

        let cert_id = decoded_payload.cert_id;
        let cert_len = decoded_payload.cert_len;
        let addr = decoded_payload.addr;
        let buf_size = decoded_payload.buf_size;
        assert_eq!(cert_id, 0x12);
        assert_eq!(cert_len, 0);
        assert_eq!(addr, 0x5678ABCD);
        assert_eq!(buf_size, 0x1234);
    }
}
