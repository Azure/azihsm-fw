// Copyright (c) Microsoft Corporation. All rights reserved.

mod cqe;
mod identify;
mod sqe;

pub(crate) use cqe::AdminCqe;
pub(crate) use cqe::StatusField;
pub(crate) use identify::McrCntrlIdentify;
pub(crate) use sqe::*;

/// Reset reason to be sent to HSM and FP
#[repr(u32)]
#[derive(Clone, Copy)]
pub(crate) enum PcieResetReason {
    /// Physical function FLR reset reason
    /// ASCII string "PFLR" -> "0x50464C52" when read from LSB to MSB
    Flr = 0x524C4650,

    /// PCIe Reset
    /// ASCII string "PRST" -> "0x54535250" when read from LSB to MSB
    Perst = 0x54535250,
}
