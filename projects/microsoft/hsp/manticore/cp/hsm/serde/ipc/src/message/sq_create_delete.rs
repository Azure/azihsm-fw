// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Submission queue create or delete action
#[repr(u8)]
#[open_enum]
#[derive(Clone, Copy, PartialEq, Eq, FromBytes, IntoBytes, Immutable, Default)]
pub enum SqAction {
    /// Delete a submission queue
    Delete = 0,

    /// Create a submission queue
    Create = 1,
}

/// Submission queues creation or deletion information
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct SqCreateDeleteInfo {
    /// PcieFunction identifier that this submission queue belongs to
    pub pfn: PcieFunction,

    /// Device submission queue identifier
    pub device_sq_id: DevSqId,

    /// Device completion queue identifier corresponding to this submission queue
    pub device_cq_id: DevCqId,

    /// Submission queue create or delete action
    pub action: SqAction,
}

impl Default for SqCreateDeleteInfo {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            pfn: PcieFunction::Pf,
            device_sq_id: DevSqId::Id0,
            device_cq_id: DevCqId::Id0,
            action: SqAction::Delete,
        }
    }
}

/// Submission queue create delete ipc message
/// Equivalent FP message code identifier: MSG_OP_VF_SLOT_SQ2CQ_MAP_UPDATE
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageCreateDeleteSq {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Submission queue Info
    pub info: SqCreateDeleteInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCreateDeleteSq::LEN],
}
static_assertions::assert_eq_size!(IpcMessageCreateDeleteSq, IpcMessage);

impl Default for IpcMessageCreateDeleteSq {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::CreateDeleteSq as u32)
                .with_length(IpcMessageCreateDeleteSq::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCreateDeleteSq::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageCreateDeleteSq {
    const OP: IpcMessageOpCode = IpcMessageOpCode::CreateDeleteSq;
    const LEN: usize = core::mem::size_of::<SqCreateDeleteInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.pfn.0, 0..=64) {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        if !matches!(self.info.device_cq_id.0, 0..=129) {
            return Err(IpcMessageErr::InvalidDeviceCqId)?;
        }

        if !matches!(self.info.device_sq_id.0, 0..=129) {
            return Err(IpcMessageErr::InvalidDeviceSqId)?;
        }

        if !matches!(self.info.action, SqAction::Delete | SqAction::Create) {
            return Err(IpcMessageErr::InvalidSqAction)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageCreateDeleteSq {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_delete_sq_ipc_message() {
        let message = IpcMessageCreateDeleteSq {
            info: SqCreateDeleteInfo {
                pfn: PcieFunction::Vf13,
                device_sq_id: DevSqId::Id10,
                device_cq_id: DevCqId::Id11,
                action: SqAction::Delete,
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x04000003);
        assert_eq!(ipc_message.data[1], 0x000B0A0D);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn encode_create_sq_ipc_message() {
        let message = IpcMessageCreateDeleteSq {
            info: SqCreateDeleteInfo {
                pfn: PcieFunction::Vf13,
                device_sq_id: DevSqId::Id10,
                device_cq_id: DevCqId::Id11,
                action: SqAction::Create,
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x04000003);
        assert_eq!(ipc_message.data[1], 0x010B0A0D);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_create_delete_sq_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000003;
        ipc_message.data[1] = 0x010B0A0D;

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        assert_eq!(message.header.msg_op(), 3);
        assert_eq!(message.header.length(), 4);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x00);
        assert_eq!(message.header.status(), 0x0);

        assert_eq!(message.info.pfn.0, PcieFunction(13).0);
        assert_eq!(message.info.device_sq_id.0, DevSqId(10).0);
        assert_eq!(message.info.device_cq_id.0, DevCqId(11).0);
        assert!(message.info.action == SqAction::Create);
    }

    #[test]
    fn decode_create_delete_sq_ipc_message_invalid_pfn() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000003;
        ipc_message.data[1] = 0x010B0AFF;

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPcieFnId.into());
        }
    }

    #[test]
    fn decode_create_delete_sq_ipc_message_invalid_hw_dev_sqid() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000003;
        ipc_message.data[1] = 0x010BE00D;

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidDeviceSqId.into());
        }
    }

    #[test]
    fn decode_create_delete_sq_ipc_message_invalid_hw_dev_cqid() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000003;
        ipc_message.data[1] = 0x01F00A0D;

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidDeviceCqId.into());
        }
    }

    #[test]
    fn decode_create_delete_sq_ipc_message_invalid_action() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000003;
        ipc_message.data[1] = 0xE00B0A0D;

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidSqAction.into());
        }
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageCreateDeleteSq> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
