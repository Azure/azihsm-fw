// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use mcr_io_controller::*;
use zerocopy::*;

use crate::error::HostStatusCode;

#[bitfield(u16)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
/// Completion queue entry, status field
pub(crate) struct StatusField {
    /// Phase bit, not used in the firmware
    _rsvd1: bool,

    /// Status code
    #[bits(15)]
    pub(crate) status: HostStatusCode,
}

/// Admin completion queue entry
#[repr(C)]
#[derive(Clone, Copy, Default, IntoBytes, Immutable, FromBytes)]
pub struct AdminCqe {
    /// Command specific data
    pub(crate) command_specific: u32,

    /// Reserved
    pub(crate) _rsvd: u32,

    /// Submission queue head index
    pub(crate) sq_head: u16,

    /// Submission queue ID
    pub(crate) sq_id: u16,

    /// Commannd ID
    pub(crate) cmd_id: u16,

    /// Status Field
    pub(crate) psf: StatusField,
}
static_assertions::assert_eq_size!(AdminCqe, IoTxEntry);

impl From<AdminCqe> for IoTxEntry {
    fn from(value: AdminCqe) -> Self {
        let mut entry: Self = [0u8; IO_TX_ENTRY_SIZE];
        entry.clone_from_slice(value.as_bytes());

        entry
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn admin_sqe_from_io_rx_entry() {
        let cqe = AdminCqe {
            command_specific: 0x12345678,
            ..Default::default()
        };

        let tx_entry: IoTxEntry = cqe.into();

        assert_eq!(tx_entry[0], 0x78);
        assert_eq!(tx_entry[1], 0x56);
        assert_eq!(tx_entry[2], 0x34);
        assert_eq!(tx_entry[3], 0x12);
    }
}
