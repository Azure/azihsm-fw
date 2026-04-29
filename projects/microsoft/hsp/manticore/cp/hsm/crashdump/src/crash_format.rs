// Copyright (c) Microsoft Corporation. All rights reserved.

use core::fmt;
use cortex_m::peripheral::scb;
use cortex_m_rt::ExceptionFrame;
use zerocopy::{FromBytes, Immutable, IntoBytes};

/// CPU register context
#[derive(Default)]
pub struct CpuRegisterContext {
    pub lr: u32,
    pub sp: u32,
    pub pc: u32,
    pub r0: u32,
    pub r1: u32,
    pub r2: u32,
    pub r3: u32,
    pub r12: u32,
}

/// Implementation for CpuRegisterContext
impl CpuRegisterContext {
    /// Convert the exception frame to a CPU register context.
    /// Note: SP available in the exception frame is not the actual SP. It is the SP at the time of exception.
    /// The actual SP should be the address of the exception frame + size of the exception frame.
    /// The crash handler is responsible for calculating the actual SP.
    pub fn from_exception_frame(ef: &ExceptionFrame) -> Self {
        Self {
            lr: ef.lr(),
            sp: 0,
            pc: ef.pc(),
            r0: ef.r0(),
            r1: ef.r1(),
            r2: ef.r2(),
            r3: ef.r3(),
            r12: ef.r12(),
        }
    }
}

/// Register block context contains fault registers CFSR, MMFAR, and BFAR.
pub struct RegisterBlockContext {
    pub cfsr: u32,
    pub mmfar: u32,
    pub bfar: u32,
}

/// Implementation for RegisterBlockContext
impl RegisterBlockContext {
    pub fn new(cfsr: u32, mmfar: u32, bfar: u32) -> Self {
        Self { cfsr, mmfar, bfar }
    }

    pub fn from_register_block(rb: &scb::RegisterBlock) -> Self {
        Self {
            cfsr: rb.cfsr.read(),
            mmfar: rb.mmfar.read(),
            bfar: rb.bfar.read(),
        }
    }
}

/// Crash Dump header format as per hsm/docs/ras/crashdump.md
#[repr(C)]
#[derive(FromBytes, IntoBytes, Immutable, Clone, Copy, PartialEq)]
pub(crate) struct CrashDumpHeader {
    /// Magic number to identify the structure
    pub magic: u32,

    /// Failure code of the crash
    pub failure_code: u32,

    /// Version of the structure
    pub crashdump_version: u16,

    /// Type of the core that generated the dump
    pub core_type: u8,

    /// Type of the dump
    pub dump_type: u8,

    /// Crash type
    pub crash_type: u8,

    /// Reserved for future use
    pub _rsvd: u8,

    /// Size of the payload that follows the header
    pub payload_size: u16,
}

/// The format of the crash dump body is compliant with the format in hsm/docs/ras/crashdump.md
/// section: Production Code Crash Dump Payload CP Admin, CP HSM, and FP core0-2
#[repr(C)]
#[derive(FromBytes, IntoBytes, Immutable, Clone, Copy, PartialEq)]
pub(crate) struct CrashDumpBody {
    /// Stack pointer
    pub stack_ptr: u32,

    /// The xPSR value during exception handling. This indicates the type of exception
    pub xpsr: u32,

    /// General purpose register r0
    pub r0: u32,

    /// General purpose register r1
    pub r1: u32,

    /// General purpose register r2
    pub r2: u32,

    /// General purpose register r3
    pub r3: u32,

    /// General purpose register r12
    pub r12: u32,

    /// Link register
    pub lr: u32,

    /// Return address from the exception. The program counter where exception happened.
    pub return_address: u32,

    /// xPSR register before exception.
    pub xpsr_pre_exception: u32,

    /// HardFault Status Register
    pub hfsr: u32,

    /// Configurable Fault Status Register
    pub cfsr: u32,

    /// Memory Management Fault Address Register
    pub mmfar: u32,

    /// BusFault Address Register
    pub bfar: u32,

    /// Auxiliary Fault Status Register
    pub afsr: u32,
}

/// Crash dump block
#[repr(C)]
#[derive(FromBytes, IntoBytes, Immutable, Clone, Copy, PartialEq)]
pub struct CrashDumpBlock {
    pub(crate) header: CrashDumpHeader,
    pub(crate) body: CrashDumpBody,
}

/// Implementation for CrashDumpBlock
impl CrashDumpBlock {
    /// Get the dump type based on the build type.
    /// Release: 0, Debug: 1.
    #[cfg(not(debug_assertions))]
    pub fn get_dump_type() -> u8 {
        0x0
    }

    #[cfg(debug_assertions)]
    pub fn get_dump_type() -> u8 {
        0x1
    }
}

/// Debug implementation for CrashDumpBlock
impl fmt::Debug for CrashDumpBlock {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("CrashDump")
            .field("header", &self.header)
            .field("body", &self.body)
            .finish()
    }
}
