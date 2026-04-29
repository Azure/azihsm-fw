// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]
#![no_main]

use cortex_m_rt::entry;
use mcr_cpu::cpu_stall;
use mcr_crypto_rng::*;
#[allow(unused_imports)]
use mcr_interrupt_controller::Interrupt;
#[allow(unused_imports)]
use mcr_interrupt_controller::InterruptController;
#[allow(unused_imports)]
use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_registers::rng_regs::RegisterBlock as RngRegs;
use mcr_test_harness::*;

pub const RNG_DATA_BUF: u32 = 0x61152000;

pub fn create_data_buff(len: usize) -> &'static mut [u8] {
    unsafe { core::slice::from_raw_parts_mut(RNG_DATA_BUF as *mut u8, len) }
}

fn init_rng() -> Rng {
    let mut cal_data: RngCalibration = RngCalibration::default();
    cal_data.clk_div = 0x60u8;
    cal_data.cutoff.set_clk_div_msb(0);
    cal_data.cutoff.set_repcnt(0x29);
    cal_data.cutoff.set_apt(0x318);
    cal_data.cutoff.set_chisq(0x82);

    Rng::new(cal_data)
}

fn test_rng_generate() {
    let rng = init_rng();
    let rng_data_buf = create_data_buff(64);
    let intc = InterruptController::default();

    rng.bytes(rng_data_buf);

    let rng_data_buf = create_data_buff(1024);

    intc.clear(Interrupt::rng_done_irq);
    intc.clear(Interrupt::rng_error_irq);
    rng.bytes(rng_data_buf);
    assert_eq!(intc.pending(Interrupt::rng_error_irq), false);
}

fn test_rng_error_interrupt() {
    let _rng = init_rng();

    let intc = InterruptController::default();
    intc.clear(Interrupt::rng_done_irq);
    intc.clear(Interrupt::rng_error_irq);
    let rng_regs = RngRegs::block();
    rng_regs.apt_cutoff().write(|_| 0x2.into());
    cpu_stall(10000);
    assert_eq!(intc.pending(Interrupt::rng_error_irq), true);
}

fn test_rng_self_test() {
    let rng = init_rng();

    assert!(rng.self_test().is_ok());
}

test_suite! {
    test_rng_self_test,
    test_rng_generate,
    test_rng_error_interrupt,
}
