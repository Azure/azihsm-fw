// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;
use mcr_ddi_types::DdiTestStackErrorType;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// Stack Error Type for IPC
#[repr(u32)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes)]
pub enum StackErrorType {
    /// Stack overflow (MemManage fault)
    StackOverflow = 1,

    /// Stack guard violation (MemManage fault)
    StackGuardViolation = 2,
}

impl Default for StackErrorType {
    fn default() -> Self {
        StackErrorType::StackOverflow
    }
}

impl From<DdiTestStackErrorType> for StackErrorType {
    fn from(error_type: DdiTestStackErrorType) -> Self {
        match error_type {
            DdiTestStackErrorType::StackOverflow => StackErrorType::StackOverflow,
            DdiTestStackErrorType::StackGuardViolation => StackErrorType::StackGuardViolation,
            _ => StackErrorType::StackOverflow,
        }
    }
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageTriggerStackValidation {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Stack error type
    pub error_type: StackErrorType,

    /// CPU ID
    pub cpu_id: SocCpuId,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTriggerStackValidation::LEN],
}
static_assertions::assert_eq_size!(IpcMessageTriggerStackValidation, IpcMessage);

impl Default for IpcMessageTriggerStackValidation {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::TriggerStackValidation as u32)
                .with_length(IpcMessageTriggerStackValidation::LEN as u32),
            error_type: Default::default(),
            cpu_id: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTriggerStackValidation::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageTriggerStackValidation {
    const OP: IpcMessageOpCode = IpcMessageOpCode::TriggerStackValidation;
    const LEN: usize = core::mem::size_of::<u32>() + core::mem::size_of::<u32>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageTriggerStackValidation {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use mcr_ddi_types::DdiTestActionSocCpuId;
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    #[test]
    fn test_default() {
        let msg = IpcMessageTriggerStackValidation::default();
        assert_eq!(
            msg.header.msg_op(),
            IpcMessageOpCode::TriggerStackValidation as u32
        );
        assert_eq!(
            msg.header.length(),
            IpcMessageTriggerStackValidation::LEN as u32
        );
    }

    #[test]
    fn test_validate() {
        let msg = IpcMessageTriggerStackValidation::default();
        assert!(msg.validate().is_ok());
    }

    #[test]
    fn test_from_ddi_trigger_stack_error_type() {
        let error_type = DdiTestStackErrorType::StackOverflow;
        let msg = IpcMessageTriggerStackValidation {
            error_type: error_type.into(),
            ..Default::default()
        };
        assert!(msg.error_type == StackErrorType::StackOverflow);

        let error_type = DdiTestStackErrorType::StackGuardViolation;
        let msg = IpcMessageTriggerStackValidation {
            error_type: error_type.into(),
            ..Default::default()
        };
        assert!(msg.error_type == StackErrorType::StackGuardViolation);
    }

    #[test]
    fn test_from_ddi_test_action_soc_cpu_id() {
        let cpu_id = DdiTestActionSocCpuId::Admin;
        let msg = IpcMessageTriggerStackValidation {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Admin);

        let cpu_id = DdiTestActionSocCpuId::Hsm;
        let msg = IpcMessageTriggerStackValidation {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Hsm);

        let cpu_id = DdiTestActionSocCpuId::Fp0;
        let msg = IpcMessageTriggerStackValidation {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp0);

        let cpu_id = DdiTestActionSocCpuId::Fp1;
        let msg = IpcMessageTriggerStackValidation {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp1);

        let cpu_id = DdiTestActionSocCpuId::Fp2;
        let msg = IpcMessageTriggerStackValidation {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp2);
    }

    #[test]
    fn trigger_stack_validation_encode() {
        let msg = IpcMessageTriggerStackValidation {
            error_type: StackErrorType::StackGuardViolation,
            cpu_id: SocCpuId::Admin,
            ..Default::default()
        };

        let ipc_message = msg.encode();
        assert_eq!(ipc_message.data[0], 0x0800004B);
        assert_eq!(ipc_message.data[1], 0x02);
        assert_eq!(ipc_message.data[2], 0x00);
        assert_eq!(ipc_message.data[3..IPC_MESSAGE_LENGTH], [0; 13]);
    }

    #[test]
    fn trigger_stack_validation_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x0800004B;
        ipc_message.data[1] = 0x01;
        ipc_message.data[2] = 0x00;

        let result: McrResult<IpcMessageTriggerStackValidation> =
            IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
        let decoded_msg = result.unwrap();
        let decoded_header = decoded_msg.header;
        assert_eq!(
            decoded_header.msg_op(),
            IpcMessageOpCode::TriggerStackValidation as u32
        );
        assert_eq!(
            decoded_header.length(),
            IpcMessageTriggerStackValidation::LEN as u32
        );
        assert!(decoded_msg.error_type == StackErrorType::StackOverflow);
        assert!(decoded_msg.cpu_id == SocCpuId::Admin);
    }
}
