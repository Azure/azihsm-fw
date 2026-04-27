// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Physical Function enable/disable action
#[repr(u8)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes)]
pub enum PfnEnableDisableAction {
    /// Disable Function
    Disable = 0,

    /// Enable Function
    Enable = 1,

    /// Reset Function
    Migrate = 2,
}

/// Pcie Function Enable Disable Information
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct PfnEnableDisableInfo {
    /// PcieFunction identifier to be enabled or disabled
    pub pfn: PcieFunction,

    /// Action to be performed, 0 - disable or 1 - enable
    pub action: PfnEnableDisableAction,
}

/// Pcie Function Enable Disable Ipc Message
/// Equivalent FP message code identifier: MSG_OP_VF_UPDATE
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessagePfnEnableDisable {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// PcieFunction enable or disable Info
    pub info: PfnEnableDisableInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessagePfnEnableDisable::LEN],
}
static_assertions::assert_eq_size!(IpcMessagePfnEnableDisable, IpcMessage);

impl Default for IpcMessagePfnEnableDisable {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::PfnEnableDisable as u32)
                .with_length(IpcMessagePfnEnableDisable::LEN as u32),
            info: PfnEnableDisableInfo {
                pfn: PcieFunction::Pf,
                action: PfnEnableDisableAction::Disable,
            },
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessagePfnEnableDisable::LEN],
        }
    }
}

impl IpcMessageType for IpcMessagePfnEnableDisable {
    const OP: IpcMessageOpCode = IpcMessageOpCode::PfnEnableDisable;
    const LEN: usize = core::mem::size_of::<PfnEnableDisableInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.pfn.0, 0..=64) {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        if !matches!(
            self.info.action,
            PfnEnableDisableAction::Enable
                | PfnEnableDisableAction::Disable
                | PfnEnableDisableAction::Migrate
        ) {
            return Err(IpcMessageErr::InvalidPfnEnableDisableAction)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessagePfnEnableDisable {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_pfn_enable_disable_ipc_message() {
        let message = IpcMessagePfnEnableDisable {
            info: PfnEnableDisableInfo {
                pfn: PcieFunction::Vf13,
                action: PfnEnableDisableAction::Disable,
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x02000005);
        assert_eq!(ipc_message.data[1], 0x000D);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_pcie_function_enable_disable_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x02014005;
        ipc_message.data[1] = 0x0023;

        let result: McrResult<IpcMessagePfnEnableDisable> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 5);
        assert_eq!(message.header.length(), 2);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x40);
        assert_eq!(message.header.status(), 0x1);

        assert!(message.info.action == PfnEnableDisableAction::Disable);
        assert!(message.info.pfn == PcieFunction::Vf35);
    }

    #[test]
    fn decode_pcie_function_enable_disable_ipc_message_for_invalid_pcie_function() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x02014005;
        ipc_message.data[1] = 0x0150;

        let result: McrResult<IpcMessagePfnEnableDisable> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPcieFnId.into());
        } else {
            panic!("Expected an error for invalid PcieFunction");
        }
    }

    #[test]
    fn decode_pcie_function_enable_disable_ipc_message_invalid_action() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x02014005;
        ipc_message.data[1] = 0x1923;

        let result: McrResult<IpcMessagePfnEnableDisable> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPfnEnableDisableAction.into());
        } else {
            panic!("Expected an error for invalid action");
        }
    }

    #[test]
    fn decode_header_for_pcie_function_enable_disable_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x02014005;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x5);
        assert!(!header.response());
        assert_eq!(header.tag(), 0x40);
        assert_eq!(header.status(), 0x01);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x2);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessagePfnEnableDisable> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
