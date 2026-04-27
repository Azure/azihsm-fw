// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;
extern crate ureg;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_cpu::cpu_stall;
use mcr_registers::rng_regs::RegisterBlock as RngRegs;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_tcon::Tcon;

use crate::*;

const RNG_INIT_DELAY: u32 = 100000;

// The CPU TSC increments the count once every 16 ns.
// Using a timeout of 1 us corresponds to 62.5 counts of TSC.
// RNG HW requires 959 cycles for DRBG instantiate operation at 500 MHz clock frequency.
// That equates to 959 / 500000000 = 1.918 us = ~ 2 us.
const RNG_DRBG_INSTANTIATE_MAX_TIMEOUT: u64 = 63 * 4;

// RNG self test input
static mut RNG_SELF_TEST_INPUT: [u32; 16] = [
    0x2cb85c71, 0xdef849bf, 0x534688e3, 0x03bff6bd, 0x9923dfd1, 0x28e0a0d7, 0xf38f606c, 0xcd88cb0b,
    0xd41f21da, 0x16b2d32f, 0x041b8db2, 0xb5b52b5b, 0x170001ef, 0x602d910b, 0x5a17e20f, 0xf9a0b0b9,
];

// RNG self test expected output
static mut RNG_SELF_TEST_EXPECTED_OUTPUT: [u32; 16] = [
    0x2df526c1, 0x4ed2fea1, 0xe03e4e33, 0x773b820d, 0x5363125e, 0x5731b848, 0xf9227325, 0x8364e0f5,
    0xc0fd533e, 0x1572b04f, 0x678f4cdc, 0xf989cd2b, 0x580a18c2, 0xe9d98573, 0x478901b9, 0xf6aa61ed,
];

// RNG self test reseed input
static mut RNG_SELF_TEST_RESEED_INPUT: [u32; 16] = [
    0x9fc0ef1b, 0x69bd5cee, 0x0536d040, 0x84e324ac, 0xe803252f, 0x51f6cb45, 0x9f425836, 0x85a1550b,
    0x1ebffe37, 0xcca0724b, 0x77ec8fcc, 0x1cd99781, 0x00226942, 0x3add2d04, 0xb6777a10, 0xac8f0743,
];

// RNG self test expected reseed output
static mut RNG_SELF_TEST_EXPECTED_RESEED_OUTPUT: [u32; 16] = [
    0xa6fcaa14, 0xd9cb3539, 0x9ba9b84e, 0x9864e8ff, 0x079b82c3, 0x32d318f6, 0x8fac5900, 0x73fd9211,
    0xe2bda914, 0xaafea144, 0x523baf42, 0x97f2049e, 0x32214767, 0xa1fdde1c, 0x141da708, 0x14b65a29,
];

/// The RNG object instance.
#[derive(Clone)]
pub struct Rng {
    pub(crate) rimpl: Rc<RefCell<RngImpl>>,
}

impl Rng {
    /// Create a RNG object.
    ///
    /// # Arguments
    ///
    ///
    ///
    /// # Returns
    ///
    /// * `Self` - The created RNG instance.
    pub fn new(calibration_data: RngCalibration) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(RngImpl::new(calibration_data))),
        }
    }

    pub fn new_without_calibration() -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(RngImpl::new_without_calibration())),
        }
    }
}

impl RngTrait for Rng {
    /// Generate a random data byte buffer.
    fn bytes(&self, data: &mut [u8]) {
        self.rimpl.borrow().bytes(data);
    }

    fn self_test(&self) -> McrResult<()> {
        self.rimpl.borrow().self_test()
    }

    #[cfg(feature = "fips_validation_hooks")]
    fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32) {
        self.rimpl
            .borrow_mut()
            .inject_rng_hw_failure(rng_hw_self_test_id);
    }
}

pub(crate) struct RngImpl {
    #[allow(dead_code)]
    calibration_data: RngCalibration,
    regs: RngRegs,
}

impl RngImpl {
    fn new(calibration_data: RngCalibration) -> Self {
        let rng = RngImpl {
            calibration_data,
            regs: RngRegs::block(),
        };
        rng.calibrate(calibration_data);

        rng
    }

    fn new_without_calibration() -> Self {
        RngImpl {
            calibration_data: RngCalibration::default(),
            regs: RngRegs::block(),
        }
    }

    fn bytes(&self, data: &mut [u8]) {
        let mut remaining_bytes = data.len();
        let mut current_position = 0;

        // Copy the bytes from the random dword to the data buffer provided via input argument here.
        // Continue copy till the client buffer is filled completely.
        while remaining_bytes > 0 {
            let random_dword = self.get_random_dword();
            let random_bytes = random_dword.to_le_bytes();
            let bytes_to_copy = random_bytes.len().min(remaining_bytes);

            data[current_position..current_position + bytes_to_copy]
                .copy_from_slice(&random_bytes[..bytes_to_copy]);

            remaining_bytes -= bytes_to_copy;
            current_position += bytes_to_copy;
        }
    }

    fn get_random_dword(&self) -> u32 {
        self.rng_wait_for_random_data();
        self.regs.rn_data().read()
    }

    fn calibrate(&self, calibration_data: RngCalibration) {
        // Disable the RNG for applying calibration values.
        self.rng_enable(false);

        // Apply the new calibration values.
        self.regs
            .ctrl()
            .read_and_modify(|_, w| w.clk_div(calibration_data.clk_div as u32));

        self.regs
            .clk_div_msb()
            .write(|_| calibration_data.cutoff.clk_div_msb().into());

        self.regs.apt_cutoff().write(|_| {
            #[cfg(feature = "fips_validation_hooks")]
            {
                use mcr_self_test::NegKind;
                const APT_FAULT_CUTOFF: u32 = 10;

                if SocInfo::default().induce_health_failure(NegKind::RngApt) {
                    return APT_FAULT_CUTOFF.into();
                }
            }
            calibration_data.cutoff.apt().into()
        });

        self.regs
            .chisq_cutoff()
            .write(|_| calibration_data.cutoff.chisq().into());

        self.regs.repcnt_cutoff().write(|_| {
            #[cfg(feature = "fips_validation_hooks")]
            {
                use mcr_self_test::NegKind;
                const REPCNT_FAULT_CUTOFF: u32 = 8;

                if SocInfo::default().induce_health_failure(NegKind::RngRct) {
                    return REPCNT_FAULT_CUTOFF.into();
                }
            }
            calibration_data.cutoff.repcnt().into()
        });

        // Enable the RNG.
        self.rng_enable(true);
    }

    fn rng_enable(&self, action: bool) {
        // Enable or disable the RNG engine based on action requested.
        self.regs.ctrl().read_and_modify(|_, w| w.enable(action));
        if action {
            cpu_stall(RNG_INIT_DELAY);
        }
    }

    /// Wait until the RNG has random data ready to read.  Also monitor the RNG for faults and execute
    /// recovery if faults occur.
    fn rng_wait_for_random_data(&self) {
        loop {
            let status = self.regs.status().read();

            if status.apt_fault_error()
                || status.chisq_fault_error()
                || status.drbg_fault_error()
                || status.drbg_inst_busy()
                || status.drbg_reseed_busy()
                || status.rbg_fault_error()
                || status.repcnt_fault_error()
            {
                // Reset the RNG.
                self.rng_enable(false);
                self.rng_enable(true);
            }
            if !self.regs.status().read().busy() {
                break;
            }
        }
    }

    fn self_test(&self) -> McrResult<()> {
        const GENERATE_INTERVAL: u32 = 2u32;
        const RESEED_INTERVAL: u32 = 2u32;

        // Modify with input in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::Rng, None) {
            unsafe {
                RNG_SELF_TEST_INPUT[0] = 0;
            }
        }

        // Before switching to FW mode, need to ensure the DRBG is not actively reading any entropy.
        let mut initial_counter = Tcon::tsc();
        while self.regs.status().read().entropy_fifo_read() {
            // Wait until entropy FIFO is empty.

            // If we are waiting for too long, timeout and fail.
            // Re-use the DRBG instantiate max timeout here.
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngSelfTestFailed)?
            }
        }

        // Save the current register value so it can be restored later.
        let saved_generate_interval: u32 = self.regs.generate_interval().read().into();
        let saved_reseed_interval: u32 = self.regs.reseed_interval().read();
        let saved_ctrl_val: u32 = self.regs.ctrl().read().into();

        // Change the generate_interval.
        self.regs
            .generate_interval()
            .write(|_| GENERATE_INTERVAL.into());
        self.regs.reseed_interval().write(|_| RESEED_INTERVAL);

        // Put the DRBG into FW mode to give the test control over the generated data.
        self.regs.ctrl().read_and_modify(|_, w| {
            w.enable(true)
                .fw_mode(true)
                .drbg_instantiate(true)
                .drbg_generate(true)
                .drbg_uninstantiate(false)
        });

        let cleanup = || {
            // Restore the RNG HW to normal mode
            self.regs.ctrl().read_and_modify(|_, w| {
                w.drbg_generate(false)
                    .drbg_instantiate(false)
                    .fw_mode(false)
            });

            self.regs
                .generate_interval()
                .write(|_| saved_generate_interval.into());
            self.regs.reseed_interval().write(|_| saved_reseed_interval);
        };

        // Fill the input FIFO. Once it is full, the DRBG will start processing it to generate output.
        // The busy status bit will indicate when that is done.
        for item in unsafe {
            #[allow(static_mut_refs)]
            &RNG_SELF_TEST_INPUT
        } {
            self.regs.fwin_data().write(|_| *item);
        }

        // Wait until DRBG has completed the instantiate operation in FW mode.
        self.wait_for_drbg_instantiate(Tcon::tsc())
            .inspect_err(|_e| cleanup())?;

        // Wait for DRBG Generate to complete.
        self.wait_for_drbg_generate(Tcon::tsc())
            .inspect_err(|_e| cleanup())?;

        // Once there is data ready, compare it to the expected output.
        // The first set of data needs to be discarded.

        self.compare_output(unsafe {
            #[allow(static_mut_refs)]
            RNG_SELF_TEST_EXPECTED_OUTPUT
        })
        .inspect_err(|_e| cleanup())?;

        // Wait for DRBG reseed to be ready.

        // Step 1: Wait for status to be set.
        self.wait_till_status_is_set_to_busy(Tcon::tsc())
            .inspect_err(|_e| cleanup())?;

        // Step 2: Wait for DRBG reseed to be set.
        self.wait_till_reseed_is_set(Tcon::tsc())
            .inspect_err(|_e| cleanup())?;

        // Fill the input FIFO. Once it is full, the DRBG will start processing it to generate output.
        // The busy status bit will indicate when that is done.
        for item in unsafe {
            #[allow(static_mut_refs)]
            &RNG_SELF_TEST_RESEED_INPUT
        } {
            self.regs.fwin_data().write(|_| *item);
        }

        // Wait for DRBG reseed to be clear.
        initial_counter = Tcon::tsc();
        self.wait_for_reseed_clear(initial_counter)
            .inspect_err(|_e| cleanup())?;

        // Wait for status to be clear.
        initial_counter = Tcon::tsc();
        self.wait_for_busy_status_clear(initial_counter)
            .inspect_err(|_e| cleanup())?;

        // Once there is data ready, compare it to the expected output.
        // The first set of data needs to be discarded.
        for item in unsafe {
            #[allow(static_mut_refs)]
            &RNG_SELF_TEST_EXPECTED_RESEED_OUTPUT
        } {
            if self.regs.fwout_data().read() != *item {
                Err(RngErr::RngSelfTestFailed).inspect_err(|_e| cleanup())?;
            }
        }

        // Restore the RNG to normal mode.
        self.regs
            .generate_interval()
            .write(|_| saved_generate_interval.into());
        self.regs.reseed_interval().write(|_| saved_reseed_interval);
        self.regs.ctrl().write(|_| saved_ctrl_val.into());
        self.regs.ctrl().read_and_modify(|_, w| {
            w.fw_mode(false)
                .drbg_instantiate(false)
                .drbg_generate(false)
        });

        Ok(())
    }

    fn wait_for_busy_status_clear(&self, initial_counter: u64) -> Result<(), u32> {
        while self.regs.status().read().busy() {
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngStatusBusyTimoeout)?
            }
        }

        Ok(())
    }

    fn wait_for_reseed_clear(&self, initial_counter: u64) -> Result<(), u32> {
        while self.regs.status().read().drbg_reseed_busy() {
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngDrbgReseedBusyTimeout)?
            }
        }

        Ok(())
    }

    fn wait_till_reseed_is_set(&self, initial_counter: u64) -> Result<(), u32> {
        while !self.regs.status().read().drbg_reseed_busy() {
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngDrbgReseedNotBusyTimeout)?
            }
        }

        Ok(())
    }

    fn wait_till_status_is_set_to_busy(&self, initial_counter: u64) -> Result<(), u32> {
        while !self.regs.status().read().busy() {
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngStatusNotBusyTimeout)?
            }
        }

        Ok(())
    }

    fn wait_for_drbg_generate(&self, initial_counter: u64) -> Result<(), u32> {
        while self.regs.status().read().busy() {
            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngStatusBusyTimoeout)?
            }
        }

        Ok(())
    }

    fn wait_for_drbg_instantiate(&self, initial_counter: u64) -> Result<(), u32> {
        while self.regs.status().read().drbg_inst_busy() {
            if self.regs.status().read().drbg_fault_error() {
                Err(RngErr::RngDrbgFaultError)?
            }

            if Tcon::tsc() - initial_counter > RNG_DRBG_INSTANTIATE_MAX_TIMEOUT {
                Err(RngErr::RngDrbgInstanceBusyTimeout)?
            }
        }

        Ok(())
    }

    fn compare_output(&self, expected_output: [u32; 16]) -> Result<(), u32> {
        for _ in 0..expected_output.len() {
            let _ = self.regs.fwout_data().read();
        }
        for item in &expected_output {
            if self.regs.fwout_data().read() != *item {
                Err(RngErr::RngSelfTestFailed)?
            }
        }

        Ok(())
    }

    #[cfg(feature = "fips_validation_hooks")]
    fn inject_rng_hw_failure(&mut self, rng_hw_self_test_id: u32) {
        const RESEED_INTERVAL: u32 = 2u32;
        const REPCNT_CUTOFF: u32 = 8u32;
        const APT_CUTOFF: u32 = 10u32;

        let rng_test: RngHwFailureTest = rng_hw_self_test_id.into();

        match rng_test {
            RngHwFailureTest::RngAptTest => {
                // Disable the RNG for applying calibration values.
                self.rng_enable(false);
                // Trigger the ApT RNG HW failure.
                self.regs.apt_cutoff().write(|_| APT_CUTOFF.into());
                self.regs
                    .reseed_interval()
                    .write(|_| RESEED_INTERVAL.into());
                let _ = self.wait_till_reseed_is_set(Tcon::tsc());
                self.rng_enable(true);
            }
            RngHwFailureTest::RngRctTest => {
                // Disable the RNG for applying calibration values.
                self.rng_enable(false);
                // Trigger the RCT RNG HW failure.
                self.regs.repcnt_cutoff().write(|_| REPCNT_CUTOFF.into());
                self.regs
                    .reseed_interval()
                    .write(|_| RESEED_INTERVAL.into());
                let _ = self.wait_till_reseed_is_set(Tcon::tsc());
                self.rng_enable(true);
            }
            _ => (),
        }
    }
}
