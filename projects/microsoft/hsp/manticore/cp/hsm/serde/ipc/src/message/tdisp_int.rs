// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// The source of the interrupt event
#[repr(u32)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes, Clone, Copy, PartialEq, PartialOrd)]
pub enum InterruptSource {
    /// TDISP interrupt
    Tdisp = 0,

    /// IDE interrupt
    Ide = 1,

    /// FLR interrupt
    Flr = 2,

    /// PerstUp interrupt
    PerstUp = 3,

    /// PerstDown interrupt
    PerstDown = 4,

    /// Unknown interrupt source
    Unknown = 0xFF,
}

/// TODO: Not sure why with #[repr(u32)], the following conversion is still needed.
impl From<InterruptSource> for u32 {
    fn from(value: InterruptSource) -> Self {
        match value {
            InterruptSource::Tdisp => 0,
            InterruptSource::Ide => 1,
            InterruptSource::Flr => 2,
            InterruptSource::PerstUp => 3,
            InterruptSource::PerstDown => 4,
            InterruptSource::Unknown => 0xFF,
            _ => unreachable!(),
        }
    }
}
impl From<u32> for InterruptSource {
    fn from(value: u32) -> Self {
        match value {
            0 => InterruptSource::Tdisp,
            1 => InterruptSource::Ide,
            2 => InterruptSource::Flr,
            3 => InterruptSource::PerstUp,
            4 => InterruptSource::PerstDown,
            _ => InterruptSource::Unknown,
        }
    }
}
impl Default for InterruptSource {
    fn default() -> Self {
        InterruptSource::Unknown
    }
}

/// TDISP-Related Interrupt Information
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes, Clone, Copy)]
pub struct TdispInterruptInfo {
    /// Indicating the source of the interrupt
    pub source: InterruptSource,

    pub vf_mask: [u32; 2],

    pub pf_mask: u32,

    /// Reserved for different register values carrying interrupt information
    pub reg_values: [u32; 5],
}

/// TDISP-Related Interrupt Ipc Message
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes, Clone, Copy)]
pub struct IpcMessageTdispInterrupt {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Information about the tdisp interrupt
    pub info: TdispInterruptInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTdispInterrupt::LEN],
}
static_assertions::assert_eq_size!(IpcMessageTdispInterrupt, IpcMessage);

impl Default for IpcMessageTdispInterrupt {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::TdispInterrupt as u32)
                .with_length(IpcMessageTdispInterrupt::LEN as u32),
            info: TdispInterruptInfo::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTdispInterrupt::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageTdispInterrupt {
    const OP: IpcMessageOpCode = IpcMessageOpCode::TdispInterrupt;
    const LEN: usize = core::mem::size_of::<TdispInterruptInfo>();

    fn validate(&self) -> McrResult<()> {
        let source = self.info.source;
        if !matches!(
            source,
            InterruptSource::Tdisp
                | InterruptSource::Ide
                | InterruptSource::Flr
                | InterruptSource::PerstUp
                | InterruptSource::PerstDown
        ) {
            return Err(IpcMessageErr::InvalidInterruptSource)?;
        }

        let pf_mask = self.info.pf_mask;
        if pf_mask > 0x1 {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageTdispInterrupt {
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
        let message = IpcMessageTdispInterrupt {
            info: TdispInterruptInfo {
                source: InterruptSource::Tdisp,
                vf_mask: [0x1, 0x2],
                ..Default::default()
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x24000049);
        assert_eq!(ipc_message.data[1], 0x0);
        assert_eq!(ipc_message.data[2], 0x1);
        assert_eq!(ipc_message.data[3], 0x2);
        assert_eq!(ipc_message.data[4..IPC_MESSAGE_LENGTH], [0; 12]);
    }

    #[test]
    fn decode_stop_interface_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x24014049;
        ipc_message.data[1] = 0x0; // source: TDISP
        ipc_message.data[2] = 0x1; // vf_mask[0]
        ipc_message.data[3] = 0x2; // vf_mask[1]
        ipc_message.data[4] = 0x1; // pf_mask

        let result: McrResult<IpcMessageTdispInterrupt> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 0x49);
        assert_eq!(message.header.length(), 0x24);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x40);
        assert_eq!(message.header.status(), 0x1);

        assert!(message.info.source == InterruptSource::Tdisp);
        assert_eq!(message.info.vf_mask[0], 0x1);
        assert_eq!(message.info.vf_mask[1], 0x2);
        assert_eq!(message.info.pf_mask, 0x1);
    }

    #[test]
    fn decode_stop_interface_ipc_message_for_invalid_source() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x24014049;
        ipc_message.data[1] = 0xFF; // invalid source

        let result: McrResult<IpcMessageTdispInterrupt> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidInterruptSource.into());
        } else {
            panic!();
        }
    }

    #[test]
    fn decode_stop_interface_ipc_message_for_invalid_pcie_function() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x24014049;
        ipc_message.data[1] = 0x1;
        ipc_message.data[4] = 0x123; // pf_mask: invalid bits set

        let result: McrResult<IpcMessageTdispInterrupt> = IpcMessageDecoder::decode(ipc_message);
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

        ipc_message.data[0] = 0x24014049;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x49);
        assert!(!header.response());
        assert_eq!(header.tag(), 0x40);
        assert_eq!(header.status(), 0x01);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x24);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageTdispInterrupt> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
