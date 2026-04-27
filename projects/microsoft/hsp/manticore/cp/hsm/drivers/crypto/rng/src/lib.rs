// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

use bitfield_struct::bitfield;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
pub use rng::Rng;

mod rng;

#[bitfield(u32)]
#[derive(Default)]
pub struct RngCalibrationCutoff {
    /// 2 most significant bits of the ring oscillator sampling clock divider in the TRBG.
    #[bits(2)]
    pub clk_div_msb: u32,

    /// Repitition count test cutoff value.
    #[bits(10)]
    pub repcnt: u32,

    /// Adaptive Proportion test cutoff value.
    #[bits(10)]
    pub apt: u32,

    /// Chi Square test cutoff value.
    #[bits(10)]
    pub chisq: u32,
}

/// RNG Calibration data
#[derive(Default, Clone, Copy)]
pub struct RngCalibration {
    /// Override for the default RNG clock divider value.
    pub clk_div: u8,

    /// MSB of clock divider and cutoff values.
    pub cutoff: RngCalibrationCutoff,
}

/// The completion details for a RNG command.
pub struct RngCompletionDesc {
    /// Result of the RNG transaction.
    pub status: RngCompletionStatus,

    /// Tag to track the transaction completion
    pub tag: u16,
}

/// PKA completion status
#[derive(PartialEq, Eq, Copy, Clone)]
pub enum RngCompletionStatus {
    /// Indicates the engine is in the middle of executing a command
    Busy = 0x1,

    /// Repitition count test failure
    RepCntFault = 0x2,

    /// Adaptive Proportion test failure
    AptFault = 0x4,

    /// Chi Square test failure
    ChisqFault = 0x8,

    /// Random Bit Generator error.
    RbgFault = 0x10,

    /// DRBG error.
    DrbgFault = 0x20,

    /// DRBG instance is busy.
    DrbgInstBusy = 0x40,

    /// DRBG reseed operation error.
    DrbgReseedBusy = 0x80,

    /// The Entropy FIFO is full.
    EntropyFifoFull = 0x100,

    /// The Entropy FIFO is read.
    EntropyFifoRead = 0x200,

    Unknown = 0x400,
}

impl From<u32> for RngCompletionStatus {
    fn from(value: u32) -> Self {
        match value {
            x if x == RngCompletionStatus::Busy as u32 => RngCompletionStatus::Busy,
            x if x == RngCompletionStatus::RepCntFault as u32 => RngCompletionStatus::RepCntFault,
            x if x == RngCompletionStatus::AptFault as u32 => RngCompletionStatus::AptFault,
            x if x == RngCompletionStatus::ChisqFault as u32 => RngCompletionStatus::ChisqFault,
            x if x == RngCompletionStatus::RbgFault as u32 => RngCompletionStatus::RbgFault,
            x if x == RngCompletionStatus::DrbgFault as u32 => RngCompletionStatus::DrbgFault,
            x if x == RngCompletionStatus::DrbgInstBusy as u32 => RngCompletionStatus::DrbgInstBusy,
            x if x == RngCompletionStatus::DrbgReseedBusy as u32 => {
                RngCompletionStatus::DrbgReseedBusy
            }
            x if x == RngCompletionStatus::EntropyFifoFull as u32 => {
                RngCompletionStatus::EntropyFifoFull
            }
            x if x == RngCompletionStatus::EntropyFifoRead as u32 => {
                RngCompletionStatus::EntropyFifoRead
            }
            _ => RngCompletionStatus::Unknown,
        }
    }
}

#[derive(PartialEq, Eq, Copy, Clone)]
#[repr(u32)]
pub enum RngHwFailureTest {
    /// Trigger RNG HW failure Act Test.
    RngAptTest,

    /// Trigger RNG HW failure Rct Test.
    RngRctTest,

    /// Trigger RNG HW failure invalid Test.
    RngInvalidTest,
}

impl From<u32> for RngHwFailureTest {
    fn from(val: u32) -> Self {
        match val {
            0 => RngHwFailureTest::RngAptTest,
            1 => RngHwFailureTest::RngRctTest,
            _ => RngHwFailureTest::RngInvalidTest,
        }
    }
}

pub trait RngTrait {
    /// Generate a random byte data buffer.
    ///
    /// # Arguments
    ///
    /// * `data` - The buffer to fill with random bytes.
    ///
    /// # Returns
    ///
    /// * `error_code` - Success or appropriate error code.
    fn bytes(&self, data: &mut [u8]);

    /// Test the RNG HW to ensure it is working properly.
    ///
    /// # Returns
    ///
    /// * `Ok(())` - if self-test is successful, error code otherwise.
    fn self_test(&self) -> McrResult<()>;

    /// Inject a hardware failure into the RNG.
    ///
    /// # Arguments
    ///
    /// * `rng_hw_self_test_id` - Test ID to define APT error or RCT error.
    #[cfg(feature = "fips_validation_hooks")]
    fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32);
}

// Error codes for RNG Driver
mcr_err_decl! {
    Rng,
    RngErr {
        // RNG engine is busy processing a command.
        EngineBusy = 1,

        // RNG self test failed.
        RngSelfTestFailed = 2,

        // RNG engine is busy processing a command.
        RngStatusBusyTimoeout = 3,

        // RNG engine is not busy processing a command.
        RngStatusNotBusyTimeout = 4,

        // RNG reseed operation is busy.
        RngDrbgReseedBusyTimeout = 5,

        // RNG reseed operation is not busy.
        RngDrbgReseedNotBusyTimeout = 6,

        // RNG DRBG instance is busy.
        RngDrbgInstanceBusyTimeout = 7,

        // RNG DRBG reseed operation is busy.
        RngDrbgFaultError = 8,

    }
}
