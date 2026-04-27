// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

pub use cortex_m::asm::dmb;
pub use cortex_m::asm::dsb;
pub use cortex_m::asm::isb;
pub use cortex_m::asm::nop;
use mcr_registers::dual_cp_m7::RegisterBlock as DualCpM7;
use mcr_registers::por::RegisterBlock as PorRegisterBlock;

/// CPU Identity
#[derive(Eq, PartialEq)]
pub enum CpuId {
    /// Admin Core
    Admin,

    /// HSM Core
    Hsm,

    /// Unknown Core
    Unknown,
}

/// Get CPU frequency in MHz
pub const fn cpu_freq() -> u32 {
    /// CPU frequency on this platform is 450 MHz
    const MCR_CPU_FREQ: u32 = 450;

    MCR_CPU_FREQ
}

/// Stall the CPU for specified microseconds
///
/// # Arguments
///
/// * `us` - microseconds to stall
///
pub fn cpu_stall(us: u32) {
    // Divide by 2, since we will have a nop instruction and a subtract instruction inside
    // the loop, which would amount for 2 CPU clock cycle in one iteration.
    let nop_count = (us * cpu_freq()) / 2;

    // Execute nop instructions
    for _ in 0..nop_count {
        nop();
    }
}

/// Get CPU Identity
///
/// # Returns
///
/// * `CpuId` - CPU Identity
pub fn cpu_id() -> CpuId {
    const ADMIN_CORE_ID: u32 = 0x2;
    const HSM_CORE_ID: u32 = 0x3;

    let cp_reg = DualCpM7::block();

    match cp_reg.cp_id().read().initiator_id() {
        ADMIN_CORE_ID => CpuId::Admin,
        HSM_CORE_ID => CpuId::Hsm,
        _ => CpuId::Unknown,
    }
}

/// CPU information trait
pub trait CpuInfoTrait {
    /// Run the Fastpath subsystem CPU cores
    ///
    /// # Arguments
    ///
    /// * `run` - Run the Fastpath subsystem CPU cores if true, else stall the cores
    fn run_fp_io_cores(&self, run: bool);
}

/// CPU information
#[derive(Clone)]
pub struct CpuInfo {}

impl CpuInfo {
    /// Run the Fastpath subsystem CPU cores
    pub fn run_fp_io_cores(run: bool) {
        let reg = PorRegisterBlock::block();

        reg.fp_runstall()
            .write(|w| w.fp0_cpuwait(!run).fp1_cpuwait(!run).fp2_cpuwait(!run));
    }
}

impl CpuInfoTrait for CpuInfo {
    /// Run the Fastpath subsystem CPU cores
    fn run_fp_io_cores(&self, run: bool) {
        Self::run_fp_io_cores(run)
    }
}
