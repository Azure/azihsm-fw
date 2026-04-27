// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
/// Get AES Bulk Key info
pub struct GetBulkKeyInfo {
    /// Key index of the AES key that is created
    pub key_index: u8,

    /// Resource group index that this key belongs to
    pub resource_id: u8,

    /// PcieFunction that this key belongs to
    pub pfn: PcieFunction,

    /// Reserved
    pub _rsvd: u8,

    /// AES256 Key data
    pub key: [u32; 8],
}

impl Default for GetBulkKeyInfo {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self {
            key_index: Default::default(),
            resource_id: Default::default(),
            pfn: PcieFunction::Pf,
            _rsvd: Default::default(),
            key: [0; 8],
        }
    }
}

#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
/// Get Bulk Key ipc message
pub struct IpcMessageGetBulkKey {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Get bulk key info
    pub info: GetBulkKeyInfo,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetBulkKey::LEN],
}
static_assertions::assert_eq_size!(IpcMessageGetBulkKey, IpcMessage);

impl Default for IpcMessageGetBulkKey {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::GetBulkKey as u32)
                .with_length(IpcMessageGetBulkKey::LEN as u32),
            info: Default::default(),
            _rsvd: [0u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageGetBulkKey::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageGetBulkKey {
    const OP: IpcMessageOpCode = IpcMessageOpCode::GetBulkKey;
    const LEN: usize = core::mem::size_of::<GetBulkKeyInfo>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageGetBulkKey {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {

    use super::*;

    #[test]
    fn ipc_message_len() {
        // Verify GetBulkKeyInfo fits within IPC payload
        assert!(core::mem::size_of::<GetBulkKeyInfo>() <= IPC_MESSAGE_PAYLOAD_LEN);
    }
}
