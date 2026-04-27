// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use mcr_io_controller::*;
use mcr_types::*;
use open_enum::open_enum;
use zerocopy::*;

/// Admin Sqe command opcodes
#[repr(u8)]
#[open_enum]
#[derive(Default, Clone, Copy, PartialEq, Eq, FromBytes, IntoBytes, Immutable)]
pub(crate) enum AdminCommandOpCodes {
    /// Delete IO Submission Queue
    DeleteSq = 0x00,

    /// Create IO Submission Queue
    CreateSq = 0x01,

    /// Delete IO Completion Queue
    DeleteCq = 0x04,

    /// Create IO Completion Queue
    CreateCq = 0x05,

    /// Identify controller
    Identify = 0x06,

    /// Set Features
    SetFeatures = 0x09,

    /// Get Features
    GetFeatures = 0x0A,

    /// Set resource count
    SetRes = 0xC3,

    /// Get Resource Count
    GetRes = 0xC4,

    /// Prepare the VF for Live Migration or VM-PHU
    VfPrep = 0xC5,

    /// Stop the VF
    VfStop = 0xC6,

    /// Start the VF
    VfStart = 0xC7,

    /// Save VF State
    VfSave = 0xC8,

    /// Restore VF State
    VfRestore = 0xC9,
}

impl AdminCommandOpCodes {
    pub fn requires_in_dma(&self) -> bool {
        *self == AdminCommandOpCodes::VfRestore
    }
}

/// HSM submission queue entry command
#[repr(C)]
#[derive(Default, Clone, Copy, IntoBytes, Immutable, FromBytes)]
pub(crate) struct AdminSqeCmd {
    /// Command Opcode
    pub op: AdminCommandOpCodes,

    /// Command Set
    pub psdt: u8,

    /// Command Id
    pub id: u16,
}

/// Admin Submission queue entry
#[repr(C)]
#[derive(Default, Copy, Clone, FromBytes, IntoBytes, Immutable)]
pub(crate) struct AdminSqe {
    /// Admin Sqe command
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Command body
    pub body: [u8; 24],
}
static_assertions::assert_eq_size!(AdminSqe, IoRxEntry);

impl From<IoRxEntry> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: IoRxEntry) -> Self {
        AdminSqe::read_from_bytes(&value[..]).unwrap()
    }
}

/// Resoruce allocate admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct GetSetResourceSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Owner Pcie Function of the resoruce
    pub cntrl_id: u32,

    /// Number of requested resource to be allocated
    pub num_resource: u32,

    /// Launch GUID with which the VF will be identified
    pub vm_launch_guid: [u8; 16],
}
static_assertions::assert_eq_size!(GetSetResourceSqe, IoRxEntry);

impl From<GetSetResourceSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: GetSetResourceSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for GetSetResourceSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Completeion queue creation attributes
#[bitfield(u32)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct CqAttributes {
    /// Physically contiguous
    pub pc: bool,

    /// Interrupt Enable
    pub ien: bool,

    #[bits(14)]
    /// Reserved
    pub _rsvd1: u16,

    /// Interrupt vector
    pub iv: u16,
}

impl Default for CqAttributes {
    fn default() -> Self {
        Self::new()
    }
}

/// Create Completion Queue Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct CreateCqSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Host completion queue id
    pub queue_id: HostCqId,

    /// Length of the completion queue
    pub queue_len: u16,

    /// Completion queue attributes
    pub attr: CqAttributes,

    /// Reserved
    pub _rsvd2: [u32; 4],
}
static_assertions::assert_eq_size!(CreateCqSqe, IoRxEntry);

impl From<CreateCqSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: CreateCqSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for CreateCqSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Delete Completion Queue Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct DeleteCqSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Host completion queue id
    pub queue_id: HostCqId,

    /// Length of the completion queue
    pub _rsvd2: u16,

    /// Reserved
    pub _rsvd3: [u32; 5],
}
static_assertions::assert_eq_size!(DeleteCqSqe, IoRxEntry);

impl From<DeleteCqSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: DeleteCqSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for DeleteCqSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Submission queue creation attributes
#[bitfield(u16)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct SqAttributes {
    /// Physically contiguous
    pub pc: bool,

    /// Submission queue priority
    #[bits(2)]
    pub priority: u16,

    #[bits(13)]
    /// Reserved
    pub _rsvd1: u16,
}

impl Default for SqAttributes {
    fn default() -> Self {
        Self::new()
    }
}

/// Create Submission Queue Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct CreateSqSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Host submission queue id
    pub queue_id: HostSqId,

    /// Length of the submission queue
    pub queue_len: u16,

    /// Submission queue attributes
    pub attr: SqAttributes,

    /// Host completion queue Id
    pub host_cq_id: HostCqId,

    /// Reserved
    pub _rsvd2: [u32; 4],
}
static_assertions::assert_eq_size!(CreateSqSqe, IoRxEntry);

impl From<CreateSqSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: CreateSqSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for CreateSqSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Delete Submission Queue Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct DeleteSqSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Host submission queue id
    pub queue_id: HostSqId,

    /// Reserved
    pub _rsvd2: u16,

    /// Reserved
    pub _rsvd3: [u32; 5],
}
static_assertions::assert_eq_size!(DeleteSqSqe, IoRxEntry);

impl From<DeleteSqSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: DeleteSqSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for DeleteSqSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Get features ID codes
#[repr(u8)]
#[open_enum]
#[derive(Clone, Copy, IntoBytes, Immutable, FromBytes, PartialEq, PartialOrd, Eq)]
pub enum AdminFeatureId {
    /// Number of HSM IO queues
    NumberOfQueues = 7,

    /// Number of FP queues
    FpNumberOfQueues = 193,

    /// Runtime FW Cap
    RtFwCapabilities = 194,
}

impl Default for AdminFeatureId {
    fn default() -> Self {
        AdminFeatureId::NumberOfQueues
    }
}

/// Get Features Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct GetFeaturesSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Admin feature id
    pub id: AdminFeatureId,

    /// Reserved
    pub _rsvd2: u8,

    /// Reserved
    pub _rsvd3: u16,

    /// Reserved
    pub _rsvd4: [u32; 5],
}
static_assertions::assert_eq_size!(GetFeaturesSqe, IoRxEntry);

impl From<GetFeaturesSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: GetFeaturesSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for GetFeaturesSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Set Features Admin command
#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct SetFeaturesSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Admin feature id
    pub id: AdminFeatureId,

    /// Reserved
    pub _rsvd2: u8,

    /// Reserved
    pub _rsvd3: u16,

    // Number of submission queues requested
    nsqr: u16,

    // Number of completion queues requested
    ncqr: u16,

    /// Reserved
    pub _rsvd4: [u32; 4],
}
static_assertions::assert_eq_size!(SetFeaturesSqe, IoRxEntry);

impl From<SetFeaturesSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: SetFeaturesSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for SetFeaturesSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

#[repr(C)]
#[derive(Default, Clone, Copy, FromBytes, IntoBytes, Immutable)]
pub(crate) struct VfLiveMigrationSqe {
    /// Header
    pub cmd: AdminSqeCmd,

    /// Reserved
    pub _rsvd1: [u32; 5],

    /// First page
    pub prp1: MemoryAddr,

    /// Second page or page list
    pub prp2: MemoryAddr,

    /// Controller Id
    pub cntrl_id: u32,

    /// Reserved
    pub _rsvd4: [u32; 5],
}
static_assertions::assert_eq_size!(VfLiveMigrationSqe, IoRxEntry);

impl From<VfLiveMigrationSqe> for AdminSqe {
    /// Converts to this type from the input type.
    fn from(value: VfLiveMigrationSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

impl From<AdminSqe> for VfLiveMigrationSqe {
    /// Converts to this type from the input type.
    fn from(value: AdminSqe) -> Self {
        Self::read_from_bytes(value.as_bytes()).unwrap()
    }
}

/// Prepare Virtual Function for LM or VMPHU Command
pub(crate) type VfPrepSqe = VfLiveMigrationSqe;
pub(crate) type VfStopSqe = VfLiveMigrationSqe;
pub(crate) type VfStartSqe = VfLiveMigrationSqe;
pub(crate) type VfSaveSqe = VfLiveMigrationSqe;
pub(crate) type VfRestoreSqe = VfLiveMigrationSqe;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn admin_sqe_from_io_rx_entry() {
        let mut rx_entry: IoRxEntry = [0; IO_RX_ENTRY_SIZE];
        rx_entry[0] = AdminCommandOpCodes::Identify.0;
        // PRP1 lo offset
        rx_entry[24] = 0xAA;
        rx_entry[25] = 0xBB;
        rx_entry[26] = 0xCC;
        rx_entry[27] = 0xDD;

        //PRP2 lo offser
        rx_entry[32] = 0xEE;
        rx_entry[33] = 0xFF;
        rx_entry[34] = 0x11;
        rx_entry[35] = 0x22;

        let sqe: AdminSqe = rx_entry.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::Identify.0);
        assert_eq!(sqe.prp1.lo, 0xDDCCBBAA);
        assert_eq!(sqe.prp2.lo, 0x2211FFEE);
    }

    #[test]
    fn set_res_sqe_to_admin_sqe() {
        let mut set_res_sqe = GetSetResourceSqe::default();
        set_res_sqe.cmd.op = AdminCommandOpCodes::SetRes;
        set_res_sqe.cntrl_id = 0x12345678;
        set_res_sqe.num_resource = 0x1234;
        set_res_sqe.prp1.lo = 0x11223344;
        set_res_sqe.prp2.lo = 0x55667788;

        let sqe: AdminSqe = set_res_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::SetRes.0);
        assert_eq!(sqe.prp1.lo, 0x11223344);
        assert_eq!(sqe.prp2.lo, 0x55667788);
    }

    #[test]
    fn admin_sqe_to_set_res_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::SetRes;
        sqe.prp1.lo = 0x11223344;
        sqe.prp2.lo = 0x55667788;

        let set_res_sqe: GetSetResourceSqe = sqe.into();

        assert_eq!(set_res_sqe.cmd.op.0, AdminCommandOpCodes::SetRes.0);
        assert_eq!(set_res_sqe.prp1.lo, 0x11223344);
        assert_eq!(set_res_sqe.prp2.lo, 0x55667788);
    }

    #[test]
    fn create_completion_queue_sqe_to_admin_sqe() {
        let mut create_cq_sqe = CreateCqSqe::default();
        create_cq_sqe.cmd.op = AdminCommandOpCodes::CreateCq;
        create_cq_sqe.queue_id = HostCqId::Id500;
        create_cq_sqe.queue_len = 0x1000;
        create_cq_sqe.prp1.lo = 0x11223344;
        create_cq_sqe.prp2.lo = 0x55667788;
        create_cq_sqe.attr.set_pc(true);
        create_cq_sqe.attr.set_ien(true);
        create_cq_sqe.attr.set_iv(0x10);

        let sqe: AdminSqe = create_cq_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::CreateCq.0);
        assert_eq!(sqe.prp1.lo, 0x11223344);
        assert_eq!(sqe.prp2.lo, 0x55667788);
        assert_eq!(sqe.body[0], 0xf4);
        assert_eq!(sqe.body[1], 0x01);
        assert_eq!(sqe.body[2], 0x00);
        assert_eq!(sqe.body[3], 0x10);
        assert_eq!(sqe.body[4], 0x03);
        assert_eq!(sqe.body[5], 0x00);
        assert_eq!(sqe.body[6], 0x10);
        assert_eq!(sqe.body[7], 0x00);
    }

    #[test]
    fn admin_sqe_to_create_completion_queue_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::CreateCq;
        sqe.prp1.lo = 0x11223344;
        sqe.prp2.lo = 0x55667788;
        sqe.body[0] = 0x00;
        sqe.body[1] = 0x01;
        sqe.body[2] = 0x00;
        sqe.body[3] = 0x20;
        sqe.body[4] = 0x01;
        sqe.body[5] = 0x00;
        sqe.body[6] = 0x20;
        sqe.body[7] = 0x00;

        let create_cq_sqe: CreateCqSqe = sqe.into();

        assert_eq!(create_cq_sqe.cmd.op.0, AdminCommandOpCodes::CreateCq.0);
        assert_eq!(create_cq_sqe.prp1.lo, 0x11223344);
        assert_eq!(create_cq_sqe.prp2.lo, 0x55667788);
        assert_eq!({ create_cq_sqe.queue_id.0 }, HostCqId::Id256.into());
        assert_eq!(create_cq_sqe.queue_len, 0x2000);
        assert!(create_cq_sqe.attr.pc());
        assert!(!create_cq_sqe.attr.ien());
        assert_eq!(create_cq_sqe.attr.iv(), 0x20);
    }

    #[test]
    fn delete_completion_queue_sqe_to_admin_sqe() {
        let mut create_cq_sqe = DeleteCqSqe::default();
        create_cq_sqe.cmd.op = AdminCommandOpCodes::DeleteCq;
        create_cq_sqe.queue_id = HostCqId::Id501;

        let sqe: AdminSqe = create_cq_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::DeleteCq.0);
        assert_eq!(sqe.body[0], 0xf5);
        assert_eq!(sqe.body[1], 0x01);
    }

    #[test]
    fn admin_sqe_to_delete_completion_queue_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::DeleteCq;
        sqe.body[0] = 0x00;
        sqe.body[1] = 0x01;

        let create_cq_sqe: DeleteCqSqe = sqe.into();

        assert_eq!(create_cq_sqe.cmd.op.0, AdminCommandOpCodes::DeleteCq.0);
        assert_eq!({ create_cq_sqe.queue_id.0 }, HostCqId::Id256.into());
    }

    #[test]
    fn create_submission_queue_sqe_to_admin_sqe() {
        let mut create_sq_sqe = CreateSqSqe::default();
        create_sq_sqe.cmd.op = AdminCommandOpCodes::CreateSq;
        create_sq_sqe.queue_id = HostSqId::Id500;
        create_sq_sqe.queue_len = 0x1000;
        create_sq_sqe.prp1.lo = 0x11223344;
        create_sq_sqe.prp2.lo = 0x55667788;
        create_sq_sqe.attr.set_pc(true);
        create_sq_sqe.attr.set_priority(1);

        let sqe: AdminSqe = create_sq_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::CreateSq.0);
        assert_eq!(sqe.prp1.lo, 0x11223344);
        assert_eq!(sqe.prp2.lo, 0x55667788);
        assert_eq!(sqe.body[0], 0xf4);
        assert_eq!(sqe.body[1], 0x01);
        assert_eq!(sqe.body[2], 0x00);
        assert_eq!(sqe.body[3], 0x10);
        assert_eq!(sqe.body[4], 0x03);
    }

    #[test]
    fn admin_sqe_to_create_submission_queue_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::CreateSq;
        sqe.prp1.lo = 0x11223344;
        sqe.prp2.lo = 0x55667788;
        sqe.body[0] = 0x00;
        sqe.body[1] = 0x01;
        sqe.body[2] = 0x00;
        sqe.body[3] = 0x20;
        sqe.body[4] = 0x07;

        let create_sq_sqe: CreateSqSqe = sqe.into();

        assert_eq!(create_sq_sqe.cmd.op.0, AdminCommandOpCodes::CreateSq.0);
        assert_eq!(create_sq_sqe.prp1.lo, 0x11223344);
        assert_eq!(create_sq_sqe.prp2.lo, 0x55667788);
        assert_eq!({ create_sq_sqe.queue_id.0 }, HostSqId::Id256.into());
        assert_eq!(create_sq_sqe.queue_len, 0x2000);
        assert!(create_sq_sqe.attr.pc());
        assert_eq!(create_sq_sqe.attr.priority(), 3);
    }

    #[test]
    fn delete_submission_queue_sqe_to_admin_sqe() {
        let mut delete_sq_sqe = DeleteSqSqe::default();
        delete_sq_sqe.cmd.op = AdminCommandOpCodes::DeleteSq;
        delete_sq_sqe.queue_id = HostSqId::Id500;

        let sqe: AdminSqe = delete_sq_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::DeleteSq.0);
        assert_eq!(sqe.body[0], 0xf4);
        assert_eq!(sqe.body[1], 0x01);
    }

    #[test]
    fn admin_sqe_to_delete_submission_queue_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::DeleteSq;
        sqe.body[0] = 0x00;
        sqe.body[1] = 0x01;

        let delete_sq_sqe: DeleteSqSqe = sqe.into();

        assert_eq!(delete_sq_sqe.cmd.op.0, AdminCommandOpCodes::DeleteSq.0);
        assert_eq!({ delete_sq_sqe.queue_id.0 }, HostSqId::Id256.into());
    }

    #[test]
    fn get_features_sqe_to_admin_sqe() {
        let mut get_features_sqe = GetFeaturesSqe::default();
        get_features_sqe.cmd.op = AdminCommandOpCodes::GetFeatures;
        get_features_sqe.id = AdminFeatureId::NumberOfQueues;

        let sqe: AdminSqe = get_features_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::GetFeatures.0);
        assert_eq!(sqe.body[0], 7);
    }

    #[test]
    fn admin_sqe_to_get_features_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::GetFeatures;
        sqe.body[0] = 193;

        let get_features_sqe: GetFeaturesSqe = sqe.into();

        assert_eq!(
            get_features_sqe.cmd.op.0,
            AdminCommandOpCodes::GetFeatures.0
        );
        assert_eq!(get_features_sqe.id.0, AdminFeatureId::FpNumberOfQueues.0);
    }

    #[test]
    fn admin_sqe_to_get_features_rt_fw_cap() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::GetFeatures;
        sqe.body[0] = 194;

        let get_features_sqe: GetFeaturesSqe = sqe.into();

        assert_eq!(
            get_features_sqe.cmd.op.0,
            AdminCommandOpCodes::GetFeatures.0
        );
        assert_eq!(get_features_sqe.id.0, AdminFeatureId::RtFwCapabilities.0);
    }

    #[test]
    fn set_features_sqe_to_admin_sqe() {
        let mut set_features_sqe = SetFeaturesSqe::default();
        set_features_sqe.cmd.op = AdminCommandOpCodes::SetFeatures;
        set_features_sqe.id = AdminFeatureId::NumberOfQueues;
        set_features_sqe.nsqr = 0x1000;
        set_features_sqe.ncqr = 0x2000;

        let sqe: AdminSqe = set_features_sqe.into();

        assert_eq!(sqe.cmd.op.0, AdminCommandOpCodes::SetFeatures.0);
        assert_eq!(sqe.body[0], 7);
        assert_eq!(sqe.body[4], 0x00);
        assert_eq!(sqe.body[5], 0x10);
        assert_eq!(sqe.body[6], 0x00);
        assert_eq!(sqe.body[7], 0x20);
    }

    #[test]
    fn admin_sqe_to_set_features_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::SetFeatures;
        sqe.body[0] = 193;
        sqe.body[4] = 0x00;
        sqe.body[5] = 0x10;
        sqe.body[6] = 0x00;
        sqe.body[7] = 0x20;

        let set_features_sqe: SetFeaturesSqe = sqe.into();

        assert_eq!(
            set_features_sqe.cmd.op.0,
            AdminCommandOpCodes::SetFeatures.0
        );
        assert_eq!(set_features_sqe.id.0, AdminFeatureId::FpNumberOfQueues.0);
    }

    #[test]
    fn admin_sqe_to_vf_prep_sqe() {
        let mut sqe = AdminSqe::default();
        sqe.cmd.op = AdminCommandOpCodes::VfPrep;
        sqe.prp1.lo = 0x11223344;
        sqe.prp2.lo = 0x55667788;

        let set_res_sqe: GetSetResourceSqe = sqe.into();

        assert_eq!(set_res_sqe.cmd.op.0, AdminCommandOpCodes::VfPrep.0);
        assert_eq!(set_res_sqe.prp1.lo, 0x11223344);
        assert_eq!(set_res_sqe.prp2.lo, 0x55667788);
    }
}
