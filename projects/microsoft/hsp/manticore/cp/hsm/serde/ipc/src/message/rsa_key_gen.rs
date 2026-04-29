// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Key update action
#[repr(u8)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes)]
pub enum RsaKeyGenKeyType {
    /// RSA 2k key
    Rsa2k = 0,

    /// RSA 3k key
    Rsa3k = 1,

    /// RSA 4k key
    Rsa4k = 2,
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct RsaKeyGenInfo {
    /// Address to which the Rsa key needs to be copied
    pub addr: u32,

    /// Rsa Key Generation Key Type
    pub key_type: RsaKeyGenKeyType,

    /// Pcie Function Id
    pub pfn: PcieFunction,

    /// Reserved padding
    pub _rsvd: [u8; 2],
}

impl Default for RsaKeyGenInfo {
    fn default() -> Self {
        Self {
            addr: 0,
            key_type: RsaKeyGenKeyType::Rsa2k,
            pfn: PcieFunction::Pf,
            _rsvd: [0; 2],
        }
    }
}

/// RSA Key Generation ipc message
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageRsaKeyGen {
    /// IPC header fields
    pub header: IpcMessageHeader,

    ///RSA Key Gen info
    pub info: RsaKeyGenInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageRsaKeyGen::LEN],
}
static_assertions::assert_eq_size!(IpcMessageRsaKeyGen, IpcMessage);

impl Default for IpcMessageRsaKeyGen {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::RsaKeyGen as u32)
                .with_length(IpcMessageRsaKeyGen::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageRsaKeyGen::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageRsaKeyGen {
    const OP: IpcMessageOpCode = IpcMessageOpCode::RsaKeyGen;
    const LEN: usize = core::mem::size_of::<RsaKeyGenInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.pfn.0, 0..=64) {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageRsaKeyGen {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_rsa_key_gen_ipc_message() {
        let message = IpcMessageRsaKeyGen {
            info: RsaKeyGenInfo {
                addr: 0x12345678,
                key_type: RsaKeyGenKeyType::Rsa3k,
                pfn: PcieFunction::Pf,
                ..Default::default()
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x08000044);
        assert_eq!(ipc_message.data[1], 0x12345678);
        assert_eq!(ipc_message.data[2], 0x00004001);
        assert_eq!(ipc_message.data[3..IPC_MESSAGE_LENGTH], [0; 13]);
    }

    #[test]
    fn validate_rsa_key_gen_ipc_message() {
        let message = IpcMessageRsaKeyGen {
            info: RsaKeyGenInfo {
                addr: 0x12345678,
                key_type: RsaKeyGenKeyType::Rsa3k,
                pfn: PcieFunction::Pf,
                ..Default::default()
            },
            ..Default::default()
        };

        assert!(message.validate().is_ok());
    }

    #[test]
    fn rsa_key_gen_ipc_message_decode_invalid_pfn() {
        let message = IpcMessageRsaKeyGen {
            info: RsaKeyGenInfo {
                addr: 0x12345678,
                key_type: RsaKeyGenKeyType::Rsa3k,
                pfn: PcieFunction(65),
                ..Default::default()
            },
            ..Default::default()
        };

        assert!(message.validate().is_err());
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageRsaKeyGen> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
