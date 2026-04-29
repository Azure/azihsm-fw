// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Key update action
#[repr(u8)]
#[open_enum]
#[derive(IntoBytes, Immutable, FromBytes, Copy, Clone)]
pub enum KeyUpdateAction {
    /// Delete key
    Delete = 0,

    /// Delete Ephemeral key
    DeleteEphemeral = 1,

    /// Delete all keys
    DeleteAll = 2,

    /// Create key
    Create = 3,
}

impl Default for KeyUpdateAction {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self::Create
    }
}

/// AES Bulk Key Type
#[repr(u8)]
#[open_enum]
#[derive(Debug, IntoBytes, Immutable, FromBytes)]
pub enum AesBulkKeyType {
    /// XTS
    Xts = 0,

    /// GCM
    Gcm = 1,

    /// GCM unapproved
    GcmUnapproved = 2,

    /// Reserved
    _Rsvd = 3,
}

impl From<AesBulkKeyType> for u8 {
    fn from(value: AesBulkKeyType) -> Self {
        value.0
    }
}

impl From<u8> for AesBulkKeyType {
    fn from(value: u8) -> Self {
        match value {
            0 => AesBulkKeyType::Xts,
            1 => AesBulkKeyType::Gcm,
            2 => AesBulkKeyType::GcmUnapproved,
            _ => AesBulkKeyType::_Rsvd,
        }
    }
}

/// AES key flag
#[bitfield(u8)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct AesKeyFlag {
    /// Key is Session or App
    pub session_only: bool,

    /// Bulk Key Type
    #[bits(2)]
    pub key_type: AesBulkKeyType,

    /// Reserved
    #[bits(5)]
    _rsvd: u8,
}

/// Key update information
#[repr(C)]
#[derive(IntoBytes, Immutable, Clone, FromBytes)]
pub struct KeyUpdateInfo {
    /// Key index of the AES key that is created
    pub key_index: u8,

    /// Resource group index that this key belongs to
    pub resource_id: u8,

    /// PcieFunction that this key belongs to
    pub pfn: PcieFunction,

    /// Action to create or delete a key
    pub action: KeyUpdateAction,

    /// Session Id this key belongs to
    pub session_id: u16,

    /// App Id
    pub app_id: u8,

    /// Key flag
    pub flag: AesKeyFlag,
}

impl Default for KeyUpdateInfo {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            key_index: Default::default(),
            resource_id: Default::default(),
            pfn: PcieFunction::Pf,
            action: Default::default(),
            session_id: Default::default(),
            app_id: Default::default(),
            flag: Default::default(),
        }
    }
}

/// Key update ipc message
/// Equivalent FP message code identifier: MSG_OP_KEY_UPDATE
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageKeyUpdate {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Key update info
    pub info: KeyUpdateInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageKeyUpdate::LEN],
}
static_assertions::assert_eq_size!(IpcMessageKeyUpdate, IpcMessage);

impl Default for IpcMessageKeyUpdate {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::AesKeyUpdate as u32)
                .with_length(IpcMessageKeyUpdate::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageKeyUpdate::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageKeyUpdate {
    const OP: IpcMessageOpCode = IpcMessageOpCode::AesKeyUpdate;
    const LEN: usize = core::mem::size_of::<KeyUpdateInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(self.info.pfn.0, 0..=65) {
            return Err(IpcMessageErr::InvalidPcieFnId)?;
        }

        if !matches!(
            self.info.action,
            KeyUpdateAction::Delete
                | KeyUpdateAction::DeleteEphemeral
                | KeyUpdateAction::DeleteAll
                | KeyUpdateAction::Create
        ) {
            return Err(IpcMessageErr::InvalidKeyUpdateAction)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageKeyUpdate {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_aes_key_update_ipc_message() {
        let message = IpcMessageKeyUpdate {
            info: KeyUpdateInfo {
                key_index: 0x07,
                resource_id: 0x01,
                pfn: PcieFunction::Vf13,
                action: KeyUpdateAction::Create,
                ..Default::default()
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x08000007);
        assert_eq!(ipc_message.data[1], 0x030D0107);
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn aes_key_update_ipc_message_decode_invalid_pfn() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000087;
        ipc_message.data[1] = 0x00880107;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x7);
        assert!(header.response());
        assert_eq!(header.tag(), 0);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x4);

        let result: McrResult<IpcMessageKeyUpdate> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidPcieFnId.into());
        }
    }

    #[test]
    fn aes_key_update_ipc_message_decode_invalid_action() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000087;
        ipc_message.data[1] = 0x100D0107;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x7);
        assert!(header.response());
        assert_eq!(header.tag(), 0);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x4);

        let result: McrResult<IpcMessageKeyUpdate> = IpcMessageDecoder::decode(ipc_message);
        if let Err(e) = result {
            assert_eq!(e, IpcMessageErr::InvalidKeyUpdateAction.into());
        }
    }

    #[test]
    fn aes_key_update_ipc_message_decode() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x04000087;
        ipc_message.data[1] = 0x000D0107;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0x7);
        assert!(header.response());
        assert_eq!(header.tag(), 0);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x4);

        let result: McrResult<IpcMessageKeyUpdate> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageKeyUpdate> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
