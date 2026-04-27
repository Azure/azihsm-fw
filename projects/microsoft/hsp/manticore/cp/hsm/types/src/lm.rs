// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use zerocopy::*;

use crate::*;

/// Submission queue info present in Live Migration context
#[repr(C)]
#[derive(Copy, Clone, FromBytes, IntoBytes, Immutable)]
pub struct LmSqInfo {
    /// Host submission queue ID
    pub id: u16,

    /// Length of the submission queue
    pub len: u16,

    /// Host completion queue identifier associated with this submission queue id
    pub cq_id: u16,

    /// Reserved
    pub rsvd: u16,

    /// Host submission queue base address
    pub addr: MemoryAddr,

    /// Host submission queue current head pointer
    pub head: u16,

    /// Host submission queue current tail pointer
    pub tail: u16,
}

/// Completion queue attributes associated with Live Migration context
#[bitfield(u16)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct LmCqAttributes {
    /// Interrupt Enable
    pub ien: bool,

    /// Phase bit
    pub ph: bool,

    /// Reserved
    #[bits(14)]
    pub _rsvd: u16,
}

/// Completion queue info present in Live Migration context
#[repr(C)]
#[derive(Copy, Clone, FromBytes, IntoBytes, Immutable)]
pub struct LmCqInfo {
    /// Host completion queue ID
    pub id: u16,

    /// Length of the completion queue
    pub len: u16,

    /// Host completion queue base address
    pub addr: MemoryAddr,

    /// Completion queue attributes associated with Live Migration context
    pub attr: LmCqAttributes,

    /// Interrupt vector number assosiated with this completion queue
    pub iv: u16,

    /// Host completion queue current head pointer
    pub head: u16,

    /// Host completion queue current tail pointer
    pub tail: u16,
}

/// Live Migration Info version number
#[repr(C, packed)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct LmVersionInfo {
    /// Version Minimum
    pub minor: u16,

    /// Version Major
    pub major: u16,
}

/// Controlller protocol version
#[bitfield(u32)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct ControllerProtocolVersion {
    /// Reserved
    #[bits(8)]
    pub rsvd: u32,

    /// Protocol minor version number
    #[bits(8)]
    pub minor: u32,

    /// Protocol major version number
    #[bits(16)]
    pub major: u32,
}

/// Interrupt vector mask set
#[bitfield(u32)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct InterruptMaskSet {
    /// Interrupt vector mask set
    #[bits(4)]
    pub ivms: u32,

    /// Reserved
    #[bits(28)]
    pub rsvd: u32,
}

/// Admin queue attributes
#[bitfield(u32)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct AqAttributes {
    /// Admin submission queue size
    #[bits(12)]
    pub aqa_asqs: u32,

    /// Reserved
    #[bits(4)]
    pub rsvd1: u32,

    /// Admin completion queue size
    #[bits(12)]
    pub aqa_acqs: u32,

    /// Reserved
    #[bits(4)]
    pub rsvd2: u32,
}

/// Controller Information as programmed by the VM device driver
#[repr(C, packed)]
#[derive(Default, FromBytes, IntoBytes, Immutable)]
pub struct ControllerLmInfo {
    /// Protocol version
    pub version: u32,

    /// Interrupt vector mask set
    pub ivms: u32,

    /// Controller configuration information
    pub configuration: u32,

    /// Admin queue attributes
    pub aq_attr: u32,

    /// Admin submission queue base address low
    pub asq_addr_lo: u32,

    /// Admin submission queue base address high
    pub asq_addr_hi: u32,

    /// Admin completion queue base address low
    pub acq_addr_lo: u32,

    /// Admin completion queue base address high
    pub acq_addr_hi: u32,

    /// Controller memory buffer location
    pub memory_buffer_location: u32,

    /// Controller memory buffer size
    pub memory_buffer_size: u32,
}

/// Virtual machine live migtation info
#[repr(C)]
#[derive(FromBytes, IntoBytes, Immutable)]
pub struct VmLiveMigrationInfo {
    /// Version of the live migration info
    pub version: LmVersionInfo,

    /// Resource count assigned to the controller
    pub resource_cnt: u32,

    /// Submission queue count
    pub sq_cnt: u16,

    /// Completion queue count
    pub cq_cnt: u16,

    /// Controller information
    pub cntrl_info: ControllerLmInfo,

    /// Admin completion queue info
    pub admin_cq_info: LmCqInfo,

    /// Admin submission queue info
    pub admin_sq_info: LmSqInfo,

    /// IO completion queue info
    pub io_cq_info: [LmCqInfo; 195],

    /// IO submission queue info
    pub io_sq_info: [LmSqInfo; 195],

    /// Session allocation mask
    pub session_allocation_mask: u8,

    /// Reserved to align the structure
    #[allow(dead_code)]
    pub reserved: [u8; 3],

    /// Masked Boot Key
    pub masked_bk_boot: MaskedBkBoot,

    /// Sealed Backup Key 3
    pub sealed_bk3: SealedBk3,

    /// CRC-32 checksum of the live migration info
    /// This is used to verify the integrity of the live migration info
    pub crc: u32,
}
