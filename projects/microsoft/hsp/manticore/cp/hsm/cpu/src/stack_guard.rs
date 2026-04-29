// Copyright (c) Microsoft Corporation. All rights reserved.

/// Guard region size in bytes. Must be power of 2 and >= 32.
pub const GUARD_SIZE_BYTES: u32 = 256;

cfg_if::cfg_if! {
    if #[cfg(not(feature = "std"))] {
        use cortex_m::peripheral::{MPU, SCB};

        const STACK_GUARD_REGION: u32 = 7; // Use last region (highest priority)
        const GUARD_SIZE_ENCODE: u32 = GUARD_SIZE_BYTES.trailing_zeros() - 1; // log2(256) - 1 = 7
    }
}

/// MPU-based stack guard configurator.
#[cfg(not(feature = "std"))]
pub struct StackGuard;

#[cfg(not(feature = "std"))]
impl StackGuard {
    /// Configure an MPU region as a stack guard below `stack_limit`.
    ///
    /// # Safety
    ///
    /// * `cp` must be the sole owner of the MPU and SCB peripherals.
    /// * `stack_limit` must be a valid RAM address with at least
    ///   [`GUARD_SIZE_BYTES`] of addressable memory below it.
    /// * Must be called with interrupts disabled or before any ISR that
    ///   relies on the MPU configuration.
    pub unsafe fn configure(stack_limit: u32) -> bool {
        let mpu = unsafe { &*MPU::PTR };
        let scb = unsafe { &*SCB::PTR };

        // Guard the memory directly below the lowest valid stack address.
        // stack_top = 0x2002_0000 (top of SRAM)
        // stack_limit = 0x2001_6000 (lowest valid stack address - stack_bottom)
        // guard_base = 0x2001_6000 - 0x100 = 0x2001_5F00 (aligned down to 256 bytes)
        let Some(guard_base_unaligned) = stack_limit.checked_sub(GUARD_SIZE_BYTES) else {
            return false;
        };

        // Align guard base downward to guard size
        let guard_base = guard_base_unaligned & !(GUARD_SIZE_BYTES - 1);

        // 2) Disable MPU while configuring
        mpu.ctrl.write(0);
        cortex_m::asm::dsb();
        cortex_m::asm::isb();

        // 3) Select region number
        mpu.rnr.write(STACK_GUARD_REGION);

        // 4) Program RBAR
        // RBAR[4]=VALID, RBAR[3:0]=REGION when VALID=1.
        // Base address must be aligned to region size.
        let rbar = (guard_base & 0xFFFF_FFE0) | (1 << 4) | (STACK_GUARD_REGION & 0xF);
        mpu.rbar.write(rbar);

        // 5) Set region attributes:
        // [0]     ENABLE = 1
        // [5:1]   SIZE   = 7  (256 bytes)
        // [15:8]  SRD    = 0  (no sub-region disable)
        // [16]    B      = 0
        // [17]    C      = 0
        // [18]    S      = 0
        // [21:19] TEX    = 0
        // [26:24] AP     = 0b000 → No access
        // [28]    XN     = 1  (Execute Never)
        let xn = 1u32 << 28;
        let ap = 0u32 << 24; // no access
        let srd = 0u32 << 8;
        let enable = 1u32;
        let rasr = xn | ap | srd | (GUARD_SIZE_ENCODE << 1) | enable;
        mpu.rasr.write(rasr);
        cortex_m::asm::dsb();
        cortex_m::asm::isb();

        // 6) Enable MemManage fault delivery (otherwise MPU faults escalate to HardFault)
        scb.shcsr.modify(|r| r | (1 << 16)); // MEMFAULTENA
        cortex_m::asm::dsb();
        cortex_m::asm::isb();

        // 7) Enable MPU with privileged default memory map.
        // PRIVDEFENA=1 keeps default map for privileged accesses; we only add guard region.
        const MPU_CTRL_ENABLE: u32 = 1 << 0;
        const MPU_CTRL_PRIVDEFENA: u32 = 1 << 2;
        mpu.ctrl.write(MPU_CTRL_ENABLE | MPU_CTRL_PRIVDEFENA);

        // 8) Mandatory memory barriers after MPU reconfiguration
        cortex_m::asm::dsb();
        cortex_m::asm::isb();

        true
    }
}
