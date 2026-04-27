// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Stop Interface Information
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes, Clone, Copy)]
pub struct StopInterfaceInfo {
    pub vf_mask: [u32; 2],

    pub pf_mask: u32,
}

/// Stop Interface Ipc Message
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes, Clone, Copy)]
pub struct IpcMessageStopInterface {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Information about the interface to stop
    pub info: StopInterfaceInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageStopInterface::LEN],
}
static_assertions::assert_eq_size!(IpcMessageStopInterface, IpcMessage);

impl Default for IpcMessageStopInterface {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::StopInterface as u32)
                .with_length(IpcMessageStopInterface::LEN as u32),
            info: StopInterfaceInfo::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageStopInterface::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageStopInterface {
    const OP: IpcMessageOpCode = IpcMessageOpCode::StopInterface;
    const LEN: usize = core::mem::size_of::<StopInterfaceInfo>();

    fn validate(&self) -> McrResult<()> {
        let pf_mask = self.info.pf_mask;
        if pf_mask > 0x1 {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageStopInterface {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_stop_interface_ipc_message() {
        let message = IpcMessageStopInterface {
            info: StopInterfaceInfo {
                vf_mask: [0x1, 0x2],
                ..Default::default()
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x0C000043);
        assert_eq!(ipc_message.data[1], 0x1);
        assert_eq!(ipc_message.data[2], 0x2);
        assert_eq!(ipc_message.data[3..IPC_MESSAGE_LENGTH], [0; 13]);
    }

    #[test]
    fn decode_stop_interface_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x0C014043;
        ipc_message.data[1] = 0x1; // vf_mask[0]
        ipc_message.data[2] = 0x2; // vf_mask[1]
        ipc_message.data[3] = 0x1; // pf_mask

        let result: McrResult<IpcMessageStopInterface> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 0x43);
        assert_eq!(message.header.length(), 0x0C);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x40);
        assert_eq!(message.header.status(), 0x1);

        assert_eq!(message.info.vf_mask[0], 0x1);
        assert_eq!(message.info.vf_mask[1], 0x2);
        assert_eq!(message.info.pf_mask, 0x1);
    }

    #[test]
    fn decode_stop_interface_ipc_message_for_invalid_pcie_function() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x0C014043;
        ipc_message.data[3] = 0x1234;

        let result: McrResult<IpcMessageStopInterface> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPcieFnId.into());
        } else {
            panic!();
        }
    }

    #[test]
    fn decode_header_for_stop_interface_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x0C014043;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x43);
        assert!(!header.response());
        assert_eq!(header.tag(), 0x40);
        assert_eq!(header.status(), 0x01);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x0C);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageStopInterface> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
