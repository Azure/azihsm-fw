// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_cpu::CpuId;

#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum CoreId {
    /// HSP core
    HSP = 0,

    /// Admin core
    Admin = 1,

    /// HSM core
    HSM = 2,

    /// FP0 core
    FP0 = 3,

    /// FP1 core
    FP1 = 4,

    /// FP2 core
    FP2 = 5,

    /// Unknown core
    Unknown = 6,
}

/// Convert u8 to CoreId
impl From<u8> for CoreId {
    fn from(value: u8) -> Self {
        match value {
            0 => CoreId::HSP,
            1 => CoreId::Admin,
            2 => CoreId::HSM,
            3 => CoreId::FP0,
            4 => CoreId::FP1,
            5 => CoreId::FP2,
            _ => CoreId::Unknown,
        }
    }
}

/// Convert CpuId to CoreId
impl From<CpuId> for CoreId {
    fn from(value: CpuId) -> Self {
        match value {
            CpuId::Admin => Self::Admin,
            CpuId::Hsm => Self::HSM,
            _ => Self::Unknown,
        }
    }
}
