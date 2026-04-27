// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#![no_std]

use mcr_registers::tcon::RegisterBlock as TconRegisterBlock;

/// CPU information
#[derive(Clone)]
pub struct Tcon {}

/// CPU information trait
pub trait TconTrait {
    /// Get time stamp counter.
    ///
    /// # Returns
    ///
    /// * `u64` containing the current time stamp counter (63bit only)
    fn tsc() -> u64;
}

impl Tcon {
    /// Get time stamp counter frequency in Hz.
    ///
    /// # Returns
    ///
    /// * `u32` containing the CPU frequency in Hz.
    pub const fn tsc_freq_hz() -> u32 {
        const MCR_TSC_FREQ: u32 = 62500000;

        MCR_TSC_FREQ
    }

    /// Get time stamp counter.
    ///
    /// # Returns
    ///
    /// * `u64` containing the current time stamp counter (63bit only)
    ///
    /// # Notes
    ///
    /// Uses TCON_timer_lo and TCON_timer_hi.
    /// This is a 63bit timer that increments at a frequency of 62.5MHz.
    /// Overflow detection is built-in and should trigger after >4,679 years.
    pub fn tsc() -> u64 {
        let reg = TconRegisterBlock::block();

        // Read counter hi value 62:31, with bit 31 overlapping with counter lo.
        let hi = reg.timer_hi().read();
        // Read counter lo value 31:0
        let lo = reg.timer_lo().read();

        Tcon::stitch_counter_lo_and_hi(lo, hi)
    }

    /// Stitch the counter lo and hi values.
    ///
    /// # Arguments
    ///
    /// * `lo` - Counter lo value
    /// * `hi` - Counter hi value
    ///
    /// # Returns
    ///
    /// * `u64` containing the stitched counter value.
    fn stitch_counter_lo_and_hi(lo: u32, hi: u32) -> u64 {
        // We read lo after we read hi so lo could have wrapped after we read hi causing bits to mismatch.
        // Since TCON is at 62.5 MHz, lo will wrap every 68.7 seconds so we can safely just add 1 without
        // having to refetch the registers in case of mismatch.
        let hi_add = if (hi & 0x1) == (lo >> 31) { 0 } else { 1 };

        ((hi.wrapping_add(hi_add) as u64) << 31) | (lo as u64)
    }

    /// Get the wakeup_timer0 tick frequency in Hz
    ///
    /// # Returns
    ///
    /// * `u32` containing the CPU frequency in Hz.
    pub const fn wakeup_timer0_freq_hz() -> u32 {
        const MCR_WAKEUP_TIMER0_FREQ: u32 = 4;

        MCR_WAKEUP_TIMER0_FREQ
    }

    /// Get the approximate number of ticks from milliseconds.
    ///
    /// # Arguments
    ///
    /// * `time_ms` - Time in milliseconds
    ///
    /// # Returns
    ///
    /// * `u32` containing the approximate number of ticks.
    pub const fn get_approximate_ticks_from_ms(time_ms: u32) -> u32 {
        time_ms * Tcon::wakeup_timer0_freq_hz() / 1000
    }

    /// Initialize the wakeup timer 0 interrupt.
    ///
    /// # Notes
    ///
    /// Enables tcon_wakeup2_intr_o[0] as a periodic pulse interrupt.
    pub fn init_wakeup_timer0() {
        let reg = TconRegisterBlock::block();

        // Disable - required for idempotent initialization.
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wakeup_enable(r.wakeup_enable() & 0b10));
        reg.wakeup0_cnt()
            .write(|_| Tcon::tsc_freq_hz() / Tcon::wakeup_timer0_freq_hz());
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wakeup_enable(r.wakeup_enable() | 0b01));
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wkintr_level_en(r.wkintr_level_en() & 0b10));
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wkintr_rpt_en(r.wkintr_rpt_en() | 0b01));
    }

    /// Initialize the wakeup timer 1 interrupt.
    ///
    /// # Notes
    ///
    /// Enables tcon_wakeup2_intr_o[1] as a periodic pulse interrupt.
    pub fn disable_wakeup_timer1() {
        let reg = TconRegisterBlock::block();

        // Disable TCON-Wakeup1 timer if it is already configured.
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wakeup_enable(r.wakeup_enable() & 0b01));
    }

    /// Fire the wakeup timer 1 interrupt.
    ///
    /// # Notes
    ///
    /// Fire the wakeup timer 1 interrupt.
    pub fn fire_wakeup_timer1() {
        let reg = TconRegisterBlock::block();
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wkintr_level_en(r.wkintr_level_en() | 0b10));
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wkintr_rpt_en(r.wkintr_rpt_en() & 0b01));
        // Fire!!!
        reg.wakeup1_cnt().write(|_| 1);
        reg.wakeup_ctrl()
            .read_and_modify(|r, w| w.wakeup_enable(r.wakeup_enable() | 0b10));
    }
}

impl TconTrait for Tcon {
    /// Get time stamp counter.
    ///
    /// # Returns
    ///
    /// * `u64` containing the current time stamp counter (63bit only)
    fn tsc() -> u64 {
        Self::tsc()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_stitch_counter_with_no_overflow() {
        let lo = 0x80000001;
        let hi = 0x00000001;

        let counter = Tcon::stitch_counter_lo_and_hi(lo, hi);
        assert_eq!(counter, 0x80000001);
    }

    #[test]
    fn test_stitch_counter_with_no_overflow_large_counter() {
        let lo = 0x80078771;
        let hi = 0xFFFFFFF1;

        let counter = Tcon::stitch_counter_lo_and_hi(lo, hi);
        assert_eq!(counter, 0x7FFFFFF8_80078771);
    }

    #[test]
    fn test_stitch_counter_with_hi_bit_mismatch() {
        let lo = 0x80000000;
        let hi = 0x00000000;

        let counter = Tcon::stitch_counter_lo_and_hi(lo, hi);
        assert_eq!(counter, 0x80000000);
    }

    #[test]
    fn test_stitch_counter_with_lo_bit_mismatch() {
        let lo = 0x00000000;
        let hi = 0x00000001;

        let counter = Tcon::stitch_counter_lo_and_hi(lo, hi);
        assert_eq!(counter, 0x1_00000000);
    }

    #[test]
    fn test_stitch_counter_with_wrap() {
        let lo = 0x00000000;
        let hi = 0xFFFFFFFF;

        let counter = Tcon::stitch_counter_lo_and_hi(lo, hi);
        assert_eq!(counter, 0);
    }

    #[test]
    fn test_get_approximate_ticks_from_ms() {
        assert_eq!(Tcon::get_approximate_ticks_from_ms(0), 0);
        assert_eq!(Tcon::get_approximate_ticks_from_ms(60000), 240);
    }
}
