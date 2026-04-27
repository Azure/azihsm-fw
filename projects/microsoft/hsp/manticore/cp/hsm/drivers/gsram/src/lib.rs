// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

use mcr_registers::gsram::RegisterBlock as GsramRegs;

/// GSRAM Controller
pub enum GsramController {}

impl GsramController {
    /// Initialize GSRAM ECC
    pub fn global_gsram_ecc_init() {
        let reg = GsramRegs::block();

        // Set GSRAM error threshold
        reg.errdthr().write(|w| w.double_bit_error_threshold(0));

        // Config double bit error interrupt
        reg.memintset()
            .write(|w| w.block_mem_double_bit_error_int_enb_set(true));
    }

    /// Handle double bit error on exception
    pub fn handle_double_bit_error() -> Option<(u32, u32)> {
        // Returns errslog0, errslog1 if no double bit error was detected,
        // or None if a double bit error was detected.
        let reg = GsramRegs::block();
        let is_double_bit_error = reg.memintstt().read().block_mem_double_bit_error();

        if is_double_bit_error {
            // Clear the error log valid bit for next error
            reg.errdsts().modify(|w| w.double_bit_error_log_valid(true));

            // Disable the double bit error interrupt
            reg.memintclr()
                .modify(|w| w.block_mem_double_bit_error_int_enb_clr(true));

            None
        } else {
            // For non-double-bit errors, return the error log info
            let log0 = reg.errslog0().read();
            let log1 = reg.errslog1().read();

            Some((log0.into(), log1.into()))
        }
    }
}
