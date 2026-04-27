// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;
use mcr_ddi_types::DdiTestActionCrashType;
use mcr_ddi_types::DdiTestActionSocCpuId;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// Crash Type
#[repr(u32)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes)]
pub enum CrashType {
    /// Hard fault
    HardFault = 1,

    /// Explicit crash
    ExplicitCrash = 2,

    /// Panic
    Panic = 3,

    /// Hang
    Hang = 4,
}

impl Default for CrashType {
    fn default() -> Self {
        CrashType::HardFault
    }
}

impl From<DdiTestActionCrashType> for CrashType {
    fn from(crash_type: DdiTestActionCrashType) -> Self {
        match crash_type {
            DdiTestActionCrashType::HardFault => CrashType::HardFault,
            DdiTestActionCrashType::ExplicitCrash => CrashType::ExplicitCrash,
            DdiTestActionCrashType::Panic => CrashType::Panic,
            DdiTestActionCrashType::Hang => CrashType::Hang,
            _ => CrashType::HardFault,
        }
    }
}

/// CPU ID
#[repr(u32)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes, Copy, Clone)]
pub enum SocCpuId {
    /// Admin core
    Admin = 0,

    /// HSM core
    Hsm = 1,

    /// FP0 core
    Fp0 = 2,

    /// FP1 core
    Fp1 = 3,

    /// FP2 core
    Fp2 = 4,
}

impl From<DdiTestActionSocCpuId> for SocCpuId {
    fn from(val: DdiTestActionSocCpuId) -> Self {
        match val {
            DdiTestActionSocCpuId::Admin => SocCpuId::Admin,
            DdiTestActionSocCpuId::Hsm => SocCpuId::Hsm,
            DdiTestActionSocCpuId::Fp0 => SocCpuId::Fp0,
            DdiTestActionSocCpuId::Fp1 => SocCpuId::Fp1,
            DdiTestActionSocCpuId::Fp2 => SocCpuId::Fp2,
            _ => SocCpuId::Admin,
        }
    }
}

impl Default for SocCpuId {
    fn default() -> Self {
        SocCpuId::Admin
    }
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageTriggerCrash {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Crash type
    pub crash_type: CrashType,

    /// CPU ID
    pub cpu_id: SocCpuId,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTriggerCrash::LEN],
}
static_assertions::assert_eq_size!(IpcMessageTriggerCrash, IpcMessage);

impl Default for IpcMessageTriggerCrash {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::TriggerCrash as u32)
                .with_length(IpcMessageTriggerCrash::LEN as u32),
            crash_type: Default::default(),
            cpu_id: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageTriggerCrash::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageTriggerCrash {
    const OP: IpcMessageOpCode = IpcMessageOpCode::TriggerCrash;
    const LEN: usize = core::mem::size_of::<u32>() + core::mem::size_of::<u32>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageTriggerCrash {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    #[test]
    fn test_default() {
        let msg = IpcMessageTriggerCrash::default();
        assert_eq!(msg.header.msg_op(), IpcMessageOpCode::TriggerCrash as u32);
        assert_eq!(msg.header.length(), IpcMessageTriggerCrash::LEN as u32);
    }

    #[test]
    fn test_validate() {
        let msg = IpcMessageTriggerCrash::default();
        assert!(msg.validate().is_ok());
    }

    #[test]
    fn test_from_ddi_test_action_crash_type() {
        let crash_type = DdiTestActionCrashType::HardFault;
        let msg = IpcMessageTriggerCrash {
            crash_type: crash_type.into(),
            ..Default::default()
        };
        assert!(msg.crash_type == CrashType::HardFault);

        let crash_type = DdiTestActionCrashType::ExplicitCrash;
        let msg = IpcMessageTriggerCrash {
            crash_type: crash_type.into(),
            ..Default::default()
        };
        assert!(msg.crash_type == CrashType::ExplicitCrash);

        let crash_type = DdiTestActionCrashType::Panic;
        let msg = IpcMessageTriggerCrash {
            crash_type: crash_type.into(),
            ..Default::default()
        };
        assert!(msg.crash_type == CrashType::Panic);

        let crash_type = DdiTestActionCrashType::Hang;
        let msg = IpcMessageTriggerCrash {
            crash_type: crash_type.into(),
            ..Default::default()
        };
        assert!(msg.crash_type == CrashType::Hang);
    }

    #[test]
    fn test_from_ddi_test_action_soc_cpu_id() {
        let cpu_id = DdiTestActionSocCpuId::Admin;
        let msg = IpcMessageTriggerCrash {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Admin);

        let cpu_id = DdiTestActionSocCpuId::Hsm;
        let msg = IpcMessageTriggerCrash {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Hsm);

        let cpu_id = DdiTestActionSocCpuId::Fp0;
        let msg = IpcMessageTriggerCrash {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp0);

        let cpu_id = DdiTestActionSocCpuId::Fp1;
        let msg = IpcMessageTriggerCrash {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp1);

        let cpu_id = DdiTestActionSocCpuId::Fp2;
        let msg = IpcMessageTriggerCrash {
            cpu_id: cpu_id.into(),
            ..Default::default()
        };
        assert!(msg.cpu_id == SocCpuId::Fp2);
    }

    #[test]
    fn test_trigger_crash_encode() {
        let msg = IpcMessageTriggerCrash {
            crash_type: CrashType::Panic,
            cpu_id: SocCpuId::Fp2,
            ..Default::default()
        };

        let ipc_message = msg.encode();
        assert_eq!(ipc_message.data[0], 0x08000047);
        assert_eq!(ipc_message.data[1], 0x03);
        assert_eq!(ipc_message.data[2], 0x04);
        assert_eq!(ipc_message.data[3..IPC_MESSAGE_LENGTH], [0; 13]);
    }

    #[test]
    fn test_trigger_crash_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x08000047;
        ipc_message.data[1] = 0x02;
        ipc_message.data[2] = 0x03;

        let retult: McrResult<IpcMessageTriggerCrash> = IpcMessageDecoder::decode(ipc_message);
        assert!(retult.is_ok());
        let decoded_msg = retult.unwrap();
        let decoded_header = decoded_msg.header;
        assert_eq!(
            decoded_header.msg_op(),
            IpcMessageOpCode::TriggerCrash as u32
        );
        assert_eq!(decoded_header.length(), IpcMessageTriggerCrash::LEN as u32);
        let msg = decoded_msg;
        assert!(msg.crash_type == CrashType::ExplicitCrash);
        assert!(msg.cpu_id == SocCpuId::Fp1);
    }
}
