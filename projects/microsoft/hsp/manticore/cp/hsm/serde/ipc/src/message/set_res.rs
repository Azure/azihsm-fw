// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes, Copy, Clone, PartialEq, Eq)]
/// Key update information
pub struct SetResInfo {
    /// Resource mask
    pub mask: [u8; 16],

    /// PcieFunction that this key belongs to
    pub pfn: PcieFunction,

    /// Reserved padding
    pub vm_launch_guid: [u8; 16],
}

impl Default for SetResInfo {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            mask: Default::default(),
            pfn: PcieFunction::Pf,
            vm_launch_guid: [0u8; 16],
        }
    }
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes, Copy, Clone, PartialEq, Eq)]
/// Key update ipc message
pub struct IpcMessageSetRes {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Key update info
    pub info: SetResInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageSetRes::LEN],
}
static_assertions::assert_eq_size!(IpcMessageSetRes, IpcMessage);

impl Default for IpcMessageSetRes {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::SetResource as u32)
                .with_length(IpcMessageSetRes::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageSetRes::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageSetRes {
    const OP: IpcMessageOpCode = IpcMessageOpCode::SetResource;
    const LEN: usize = core::mem::size_of::<SetResInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.pfn.0, 0..=64) {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageSetRes {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_set_resource_ipc_message() {
        let mask: u128 = 0x112233445566778899AABBCCDDEEFF00;
        let message = IpcMessageSetRes {
            info: SetResInfo {
                mask: mask.to_le_bytes(),
                pfn: PcieFunction::Pf,
                vm_launch_guid: [
                    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD,
                    0xEE, 0xFF, 0x00,
                ],
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x2100007F);
        assert_eq!(ipc_message.data[1], 0xDDEEFF00);
        assert_eq!(ipc_message.data[2], 0x99AABBCC);
        assert_eq!(ipc_message.data[3], 0x55667788);
        assert_eq!(ipc_message.data[4], 0x11223344);
        assert_eq!(ipc_message.data[5], 0x33221140);
        assert_eq!(ipc_message.data[6], 0x77665544);
        assert_eq!(ipc_message.data[7], 0xBBAA9988);
        assert_eq!(ipc_message.data[8], 0xFFEEDDCC);

        assert_eq!(ipc_message.data[9..IPC_MESSAGE_LENGTH], [0; 7]);
    }

    #[test]
    fn set_resource_ipc_message_decode_invalid_pfn() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x110000FF;
        ipc_message.data[1] = 0xDDEEFF00;
        ipc_message.data[2] = 0x99AABBCC;
        ipc_message.data[3] = 0x55667788;
        ipc_message.data[4] = 0x11223344;
        ipc_message.data[5] = 80;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x7f);
        assert!(header.response());
        assert_eq!(header.tag(), 0);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x11);

        let result: McrResult<IpcMessageSetRes> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPcieFnId.into());
        }
    }

    #[test]
    fn set_resource_ipc_message_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x110000FF;
        ipc_message.data[1] = 0xDDEEFF00;
        ipc_message.data[2] = 0x99AABBCC;
        ipc_message.data[3] = 0x55667788;
        ipc_message.data[4] = 0x11223344;
        ipc_message.data[5] = 64;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x7f);
        assert!(header.response());
        assert_eq!(header.tag(), 0);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x11);

        let result: McrResult<IpcMessageSetRes> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageSetRes> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
