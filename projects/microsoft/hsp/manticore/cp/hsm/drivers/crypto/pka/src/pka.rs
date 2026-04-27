// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;
extern crate ureg;

use alloc::rc::Rc;
use alloc::vec;
use core::cell::RefCell;
use zeroize::Zeroize;

use crate::pka::vec::Vec;
use mcr_error::McrResult;
use mcr_interrupt_controller::Interrupt;
use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_registers::upka_regs::upka_instance::RegisterBlock as UpkaRegs;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_tcon::Tcon;
use mcr_types::PkaCommand;
use mcr_types::PkaCommandCode;
use mcr_types::PKA_CONST_MAX_SIZE_BYTES;

use crate::ecc_constants::*;
use crate::*;

/// Imperically measured, the timeout of a single mod mul operation could be
/// 63 * 25_000 TSC. Hence timeout is assigned to a conservative value of 63 * 100_000.
const TIMEOUT_FOR_SYNC_OPERATION: u64 = 63 * 100_000;
const PKA_TEST_TAG: u16 = 123;

/// The PKA object instance.
#[derive(Clone)]
pub struct Pka<I: InterruptControllerTrait> {
    pub(crate) rimpl: Rc<RefCell<PkaImpl<I>>>,
}

impl<I: InterruptControllerTrait> Pka<I> {
    /// Global initialiation.
    ///
    /// # Arguments
    ///
    /// * `ecc_const` - The ECC constants GSRAM buffer.
    pub fn global_init(ecc_const: &'static mut [u8]) -> McrResult<()> {
        PkaImpl::<I>::global_init(ecc_const)
    }

    /// Create a UPKA object.
    ///
    /// # Arguments
    ///
    /// * `id` - UPKA instance ID
    /// * `cmd_buffer` - UPKA cmd_bufferuration to use.
    /// * `intc` - Interrupt Controller instance
    ///
    /// # Returns
    ///
    /// * `Self` - The created UPKA instance.
    pub fn new(
        id: PkaInstanceId,
        cmd_buffer: &'static mut PkaCommand,
        result: &'static mut [u8],
        input: &'static mut [u8],
        ecc_const: &'static [u8],
        intc: I,
        self_test_buf: &'static mut [u8],
    ) -> Pka<I> {
        Self {
            rimpl: Rc::new(RefCell::new(PkaImpl::new(
                id,
                cmd_buffer,
                result,
                input,
                ecc_const,
                intc,
                self_test_buf,
            ))),
        }
    }
}

impl<I: InterruptControllerTrait> PkaTrait for Pka<I> {
    fn peek_tag(&self) -> Option<u16> {
        self.rimpl.borrow().peek_tag()
    }

    fn begin_ecc_gen_key(&self, tag: u16, curve: PkaEccCurve) -> McrResult<PkaEccCmd> {
        self.rimpl.borrow_mut().begin_ecc_gen_key(tag, curve)
    }

    fn end_ecc_gen_key(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccKeyPair> {
        self.rimpl.borrow_mut().end_ecc_gen_key(tag, op)
    }

    fn begin_ecc_sign_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        priv_key: &[u8],
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.rimpl
            .borrow_mut()
            .begin_ecc_sign_zc(tag, curve, priv_key, digest, signature)
    }

    fn end_ecc_sign_zc(&self, tag: u16) -> McrResult<()> {
        self.rimpl.borrow_mut().end_ecc_sign_zc(tag)
    }

    fn begin_ecc_verify_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .begin_ecc_verify_zc(tag, curve, public_key, digest, signature)
    }

    fn end_ecc_verify_zc(&self, tag: u16) -> McrResult<bool> {
        self.rimpl.borrow_mut().end_ecc_verify_zc(tag)
    }

    fn begin_ecc_point_validation_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .begin_ecc_point_validation_zc(tag, curve, public_key)
    }

    fn end_ecc_point_validation_zc(&self, tag: u16) -> McrResult<bool> {
        self.rimpl.borrow_mut().end_ecc_point_validation_zc(tag)
    }

    fn begin_ecc_gen_pub_key_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        pub_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.rimpl
            .borrow_mut()
            .begin_ecc_gen_pub_key_zc(tag, curve, private_key, pub_key)
    }

    fn end_ecc_gen_pub_key_zc(&self, tag: u16, op: PkaEccCmd) -> McrResult<()> {
        self.rimpl.borrow_mut().end_ecc_gen_pub_key_zc(tag, op)
    }

    fn begin_montgomery_constant_calculation(&self, tag: u16, curve: PkaEccCurve) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .begin_montgomery_constant_calculation(tag, curve)
    }

    fn end_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .end_montgomery_constant_calculation(tag)
    }

    fn end_ecdh_compute(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccSecretValue> {
        self.rimpl.borrow_mut().end_ecdh_compute(tag, op)
    }

    fn begin_ecdh_compute_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        public_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.rimpl
            .borrow_mut()
            .begin_ecdh_compute_zc(tag, curve, private_key, public_key)
    }

    fn begin_rsa_private_key_op_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        priv_key: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_private_key_op_zc(tag, rsa_type, priv_key, input, output)
    }

    fn end_rsa_private_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()> {
        self.rimpl.borrow_mut().end_rsa_private_key_op_zc(tag, op)
    }

    fn begin_rsa_public_key_op_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        public_key: &IoMemRange,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_public_key_op_zc(tag, rsa_type, public_key, input, output)
    }

    fn end_rsa_public_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()> {
        self.rimpl.borrow_mut().end_rsa_public_key_op_zc(tag, op)
    }

    fn begin_rsa_private_key_op_crt_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        crt_param1: &[u8],
        crt_param2: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_private_key_op_crt_zc(tag, rsa_type, crt_param1, crt_param2, input, output)
    }

    fn end_rsa_private_key_op_crt_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .end_rsa_private_key_op_crt_zc(tag, op)
    }

    fn begin_rsa_montgomery_constant_calculation(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        modulus_be: &[u8],
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_montgomery_constant_calculation(tag, rsa_type, modulus_be)
    }

    fn end_rsa_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .end_rsa_montgomery_constant_calculation(tag)
    }

    fn begin_rsa_montgomery_in(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_montgomery_in(tag, rsa_type, data)
    }

    fn end_rsa_montgomery_in(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
        self.rimpl.borrow_mut().end_rsa_montgomery_in(tag, op)
    }

    fn begin_rsa_modular_inverse(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_modular_inverse(tag, rsa_type, data)
    }

    fn end_rsa_modular_inverse(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
        self.rimpl.borrow_mut().end_rsa_modular_inverse(tag, op)
    }

    fn begin_rsa_montgomery_out(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_montgomery_out(tag, rsa_type, data)
    }

    fn end_rsa_montgomery_out(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaData> {
        self.rimpl.borrow_mut().end_rsa_montgomery_out(tag, op)
    }

    fn begin_rsa_modular_multiplication(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        value1: &[u8],
        value2: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.rimpl
            .borrow_mut()
            .begin_rsa_modular_multiplication(tag, rsa_type, value1, value2)
    }

    fn end_rsa_modular_multiplication(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
        self.rimpl
            .borrow_mut()
            .end_rsa_modular_multiplication(tag, op)
    }

    fn ecdsa_self_test(&self) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .ecdsa_self_test_internal(PkaEccCurve::Ecc384)
    }

    fn ecdh_self_test(&self) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .ecdh_self_test_internal(PkaEccCurve::Ecc384)
    }

    fn rsa_mod_exp_self_test(&self) -> McrResult<Vec<u8>> {
        self.rimpl
            .borrow_mut()
            .rsa_mod_exp_self_test_internal(PkaRsaSize::Rsa2k)
    }

    fn rsa_mod_exp_crt_self_test(&self) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .rsa_mod_exp_crt_self_test_internal(PkaRsaSize::Rsa2k)
    }

    fn begin_memory_wipe(&self, tag: u16) -> McrResult<()> {
        self.rimpl.borrow_mut().begin_memory_wipe(tag)
    }

    fn end_memory_wipe(&self, tag: u16) -> McrResult<()> {
        self.rimpl.borrow_mut().end_memory_wipe(tag)
    }
}

pub(crate) struct PkaImpl<I: InterruptControllerTrait> {
    /// PKA instance ID.
    pub(crate) id: PkaInstanceId,

    /// PKA (Done, Error) interrupt number tuple for this instance
    pub(crate) int_nums: (Interrupt, Interrupt),

    /// PKA command buffer submitted to the hardware engine.
    pub(crate) cmd_buffer: &'static mut PkaCommand,

    /// PKA register access object.
    pub(crate) regs: UpkaRegs,

    /// Owner identifier of the current PKA operation.
    pub(crate) tag: Option<u16>,

    /// Requested current command opcode.
    pub(crate) command_code: PkaCommandCode,

    /// Output buffer for the PKA operation.
    pub(crate) output: &'static mut [u8],

    /// Input buffer for the PKA operation.
    pub(crate) input: &'static mut [u8],

    /// Input buffer for the PKA operation.
    pub(crate) self_test_buffer: &'static mut [u8],

    /// ECC constants buffer.
    pub(crate) ecc_const: &'static [u8],

    /// Interrupt controller instance.
    pub(crate) intc: I,
}

impl<I: InterruptControllerTrait> PkaImpl<I> {
    const PKA_ADDR_ALIGNMENT: u32 = 4;

    pub fn global_init(ecc_const: &'static mut [u8]) -> McrResult<()> {
        if ecc_const.len() < PKA_CONST_MAX_SIZE_BYTES {
            Err(PkaErr::BufSizeMismatch)?
        }

        let len256 = PkaEccCurve::Ecc256.len();
        let len384 = PkaEccCurve::Ecc384.len();
        let len521 = PkaEccCurve::Ecc521.len();

        ecc_const[PKA_BASE_PT_256_START_OFFSET..PKA_BASE_PT_256_START_OFFSET + len256]
            .copy_from_slice(&BASE256.x[0..len256]);
        ecc_const[PKA_BASE_PT_256_START_OFFSET + len256..PKA_BASE_PT_256_START_OFFSET + 2 * len256]
            .copy_from_slice(&BASE256.y[0..len256]);
        ecc_const[PKA_PRIME_256_START_OFFSET..PKA_PRIME_256_START_OFFSET + len256]
            .copy_from_slice(&PRIME256.p[0..len256]);
        ecc_const[PKA_BASE_PT_384_START_OFFSET..PKA_BASE_PT_384_START_OFFSET + len384]
            .copy_from_slice(&BASE384.x[0..len384]);
        ecc_const[PKA_BASE_PT_384_START_OFFSET + len384..PKA_BASE_PT_384_START_OFFSET + 2 * len384]
            .copy_from_slice(&BASE384.y[0..len384]);
        ecc_const[PKA_PRIME_384_START_OFFSET..PKA_PRIME_384_START_OFFSET + len384]
            .copy_from_slice(&PRIME384.p[0..len384]);
        ecc_const[PKA_BASE_PT_521_START_OFFSET..PKA_BASE_PT_521_START_OFFSET + len521]
            .copy_from_slice(&BASE521.x[0..len521]);
        ecc_const[PKA_BASE_PT_521_START_OFFSET + len521..PKA_BASE_PT_521_START_OFFSET + 2 * len521]
            .copy_from_slice(&BASE521.y[0..len521]);
        ecc_const[PKA_PRIME_521_START_OFFSET..PKA_PRIME_521_START_OFFSET + len521]
            .copy_from_slice(&PRIME521.p[0..len521]);

        Ok(())
    }

    pub fn new(
        id: PkaInstanceId,
        cmd_buffer: &'static mut PkaCommand,
        result: &'static mut [u8],
        input: &'static mut [u8],
        ecc_const: &'static [u8],
        intc: I,
        self_test_buf: &'static mut [u8],
    ) -> Self {
        Self {
            id,
            int_nums: id.int_num_tuple(),
            cmd_buffer,
            regs: UpkaRegs::block(),
            tag: None,
            output: result,
            command_code: PkaCommandCode::Unknown,
            input,
            self_test_buffer: self_test_buf,
            ecc_const,
            intc,
        }
    }

    /// Peek the tag
    fn peek_tag(&self) -> Option<u16> {
        self.tag
    }

    fn aligned(addr: u32, alignment: u32) -> bool {
        (addr & !(alignment - 1)) == addr
    }

    pub(crate) fn pka_execute_command(
        &mut self,
        tag: u16,
        result: u32,
        arg1: u32,
        arg2: u32,
        arg3: u32,
        command_code: PkaCommandCode,
    ) -> McrResult<()> {
        if !Self::aligned(result, Self::PKA_ADDR_ALIGNMENT) {
            Err(PkaErr::ResultAddrMisaligned)?
        }
        if !Self::aligned(arg1, Self::PKA_ADDR_ALIGNMENT) {
            Err(PkaErr::Arg1AddrMisaligned)?
        }
        if !Self::aligned(arg2, Self::PKA_ADDR_ALIGNMENT) {
            Err(PkaErr::Arg2AddrMisaligned)?
        }
        if !Self::aligned(arg3, Self::PKA_ADDR_ALIGNMENT) {
            Err(PkaErr::Arg3AddrMisaligned)?
        }

        if command_code == PkaCommandCode::Unknown {
            Err(PkaErr::InvalidArg)?
        }

        let upka_regs = self.regs.at(self.id.into());
        if upka_regs.status().read().busy() {
            Err(PkaErr::EngineBusy)?
        }

        if self.tag.is_some() {
            Err(PkaErr::InvalidState)?
        }

        self.tag = Some(tag);
        self.cmd_buffer.result_addr = result;
        self.cmd_buffer.arg1_addr = arg1;
        self.cmd_buffer.arg2_addr = arg2;
        self.cmd_buffer.arg3_addr = arg3;
        self.cmd_buffer.command_code = command_code;
        self.command_code = command_code;

        cortex_m::asm::dmb();

        // Trigger the PKA hardware to start processing the cmd_buffer.
        upka_regs
            .command()
            .write(|_| self.cmd_buffer as *const PkaCommand as u32);

        Ok(())
    }

    pub fn check_completion(&mut self, tag: u16) -> Result<(), u32> {
        let regs = self.regs.at(self.id.into());
        let value = regs.status().read();
        if value.busy() {
            Err(PkaErr::EngineBusy)?
        }

        let interrupt = if value.complete() {
            self.int_nums.0
        } else {
            self.int_nums.1
        };

        self.intc.clear(interrupt);

        let current_tag = self.tag.ok_or(PkaErr::InvalidState)?;
        if tag != current_tag {
            Err(PkaErr::TagMismatch)?
        }

        // Consume the tag.
        self.tag.take();

        let completion_status = PkaCompletionStatus::from(u32::from(value) as u8);

        if completion_status != PkaCompletionStatus::Complete {
            Err(PkaErr::PkaHwCmdFail)?
        };
        Ok(())
    }

    /// Wait for PKA operation completion synchronously.
    fn wait_for_operation_completion_sync(&mut self, tag: u16) -> McrResult<()> {
        let initial_counter = Tcon::tsc();
        let mut completion: Result<(), u32> = Err(PkaErr::PkaHwCmdFail.into());

        while Tcon::tsc() - initial_counter <= TIMEOUT_FOR_SYNC_OPERATION {
            completion = self.check_completion(tag);
            if completion.is_ok() {
                return Ok(());
            }
        }
        completion
    }

    /// Execute unary PKA operation synchronously.
    fn execute_unary_operation_sync(
        &mut self,
        tag: u16,
        result: Option<&mut [u8]>,
        arg1: &[u8],
        command_code: PkaCommandCode,
    ) -> McrResult<()> {
        self.self_test_buffer[0..arg1.len()].copy_from_slice(arg1);
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.self_test_buffer.as_ptr() as u32,
            0,
            0,
            command_code,
        )?;

        self.wait_for_operation_completion_sync(tag)?;
        if result.is_some() {
            let result_len = result.as_ref().unwrap().len();
            result.unwrap().copy_from_slice(&self.output[0..result_len]);
        }
        Ok(())
    }

    /// Execute binary PKA operation synchronously.
    fn execute_binary_opertion_sync(
        &mut self,
        tag: u16,
        result: Option<&mut [u8]>,
        arg1: &[u8],
        arg2: &[u8],
        command_code: PkaCommandCode,
    ) -> McrResult<()> {
        self.self_test_buffer[0..arg1.len()].copy_from_slice(arg1);
        self.self_test_buffer[arg1.len()..arg1.len() + arg2.len()].copy_from_slice(arg2);
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.self_test_buffer[0..arg1.len()].as_ptr() as u32,
            self.self_test_buffer[arg1.len()..].as_ptr() as u32,
            0,
            command_code,
        )?;

        self.wait_for_operation_completion_sync(tag)?;
        if result.is_some() {
            let result_len = result.as_ref().unwrap().len();
            result.unwrap().copy_from_slice(&self.output[0..result_len]);
        }
        Ok(())
    }

    /// Execute pka ecc point multiplcaition operation synchronously.
    /// The symantics is a little different from the bianry operations in the way
    /// arg1 contains both x and y coordinates of the point and arg2 contains the scalar.
    fn ecc_point_multiplication_sync(
        &mut self,
        tag: u16,
        result: Option<&mut [u8]>,
        x: &[u8],
        y: &[u8],
        scalar: &[u8],
        curve: PkaEccCurve,
    ) -> McrResult<()> {
        self.input[0..x.len()].copy_from_slice(x);
        self.input[x.len()..x.len() + y.len()].copy_from_slice(y);
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input.as_ptr() as u32,
            scalar.as_ptr() as u32,
            0,
            curve.ecc_point_multiplication_opcode(),
        )?;
        self.wait_for_operation_completion_sync(tag)?;

        if result.is_some() {
            let result_len = result.as_ref().unwrap().len();
            result.unwrap().copy_from_slice(&self.output[0..result_len]);
        }

        // zeroize the input buffer because now its filled with x, y.
        self.input.zeroize();
        Ok(())
    }

    /// Self test for ECC sign operation for a given curve.
    fn ecdsa_self_test_internal(&mut self, curve: PkaEccCurve) -> McrResult<()> {
        let tag = PKA_TEST_TAG;
        self.output.zeroize();
        self.input.zeroize();
        self.self_test_buffer.zeroize();

        // 0. Set Montgomery constant to the curve prime.
        self.execute_unary_operation_sync(
            tag,
            None,
            &PkaEccCurve::get_prime(curve).p[0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_const_calc_opcode(),
        )?;

        // 1. Perftom ECC point multiplication. {xR, yR} = k * {xG, yG}
        // where k is the secret key and {xG, yG} is the base point.
        let mut x_r = vec![0u8; usize::from(curve)];
        let curve_len = usize::from(curve);
        // Copy k into the DMA-accessible self_test_buffer so hardware can read it.
        // (Previous change placed k on the stack causing a hardfault when the PKA engine DMA'd it.)
        self.self_test_buffer[..curve_len]
            .copy_from_slice(&ecc_constants::get_ecc_sign_test_vectors().k[..curve_len]);
        // Optionally mutate for FIPS negative test.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default()
            .induce_cast_failure(SelfTest::EcdsaEngineInstance0, Some(self.id.into()))
        {
            self.self_test_buffer[0] = self.self_test_buffer[0].wrapping_add(1);
        }

        let test_buffer =
            mcr_mem_map::mem_addr_to_slice(self.self_test_buffer.as_ptr() as usize, curve_len);
        self.ecc_point_multiplication_sync(
            tag,
            Some(&mut x_r),
            &PkaEccCurve::get_base(curve).x[0..curve_len],
            &PkaEccCurve::get_base(curve).y[0..curve_len],
            test_buffer,
            curve,
        )?;

        // Zeroize sensitive scalar bytes.
        self.self_test_buffer[..curve_len].zeroize();

        self.execute_unary_operation_sync(
            tag,
            None,
            &PkaEccCurve::get_order(curve).n[0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_const_calc_opcode(),
        )?;

        // 3. Perform modular reduction of xR. Set r = xR mod n, such that r != 0
        let mut r = vec![0u8; From::<PkaEccCurve>::from(curve)];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut r),
            &x_r,
            curve.ecc_mod_reduction_opcode(),
        )?;

        // 4. Verify that r is not zero. Given the test inputs, we dont expect r to be zero.
        // reverse_copy_from_slice(&mut r, &self.output[..32]);
        if r.iter().all(|&x| x == 0) {
            Err(PkaErr::PkaEccSignSelfTestInvalidInputParam)?
        }

        // 5. Convert k to mongomery representation.
        let mut k_mont = vec![0u8; curve.montgomery_size()];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut k_mont),
            &ecc_constants::get_ecc_sign_test_vectors().k[0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_representation_in_opcode(),
        )?;

        // 6. Convert r to mongomery representation.
        let mut r_mont = vec![0u8; curve.montgomery_size()];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut r_mont),
            &r,
            curve.ecc_mont_representation_in_opcode(),
        )?;

        // 7. Convert digest to mongomery representation.
        let mut e_mont = vec![0u8; curve.montgomery_size()];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut e_mont),
            &ecc_constants::get_ecc_sign_test_vectors().digest[0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_representation_in_opcode(),
        )?;

        // 8. Convert d to mongomery representation.
        let mut d_mont = vec![0u8; curve.montgomery_size()];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut d_mont),
            &ecc_constants::get_ecc_sign_test_vectors().private_key
                [0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_representation_in_opcode(),
        )?;

        // 9. Compute k^-1 mod n (i.e. the modular inverse)
        let mut k_mont_inverse = vec![0u8; curve.montgomery_size()];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut k_mont_inverse),
            &k_mont,
            curve.ecc_mod_inverse_opcode(),
        )?;

        // 10. Compute s = k^-1 * (e + r*d) mod n. This will be achived in the following steps.
        //   (10.1) s = k^−1 ⋅ e
        //   (10.2) t = k^−1 ⋅ d
        //   (10.3) t = t ⋅ r
        //   (10.4) s = s + t

        // 10.1 s = k^−1 ⋅ e
        let mut s_mont = vec![0u8; curve.montgomery_size()];
        self.execute_binary_opertion_sync(
            tag,
            Some(&mut s_mont),
            &k_mont_inverse,
            &e_mont,
            curve.ecc_mod_multiplication_opcode(),
        )?;

        // 10.2 t = k^−1 ⋅ d
        let mut t_mont = vec![0u8; curve.montgomery_size()];
        self.execute_binary_opertion_sync(
            tag,
            Some(&mut t_mont),
            &k_mont_inverse,
            &d_mont,
            curve.ecc_mod_multiplication_opcode(),
        )?;

        // 10.3 t = t ⋅ r
        let mut t_mont_dot_r = vec![0u8; curve.montgomery_size()];
        self.execute_binary_opertion_sync(
            tag,
            Some(&mut t_mont_dot_r),
            &t_mont,
            &r_mont,
            curve.ecc_mod_multiplication_opcode(),
        )?;

        // 10.4 s = s + t
        let mut s_mont_plus_t = vec![0u8; curve.montgomery_size()];
        self.execute_binary_opertion_sync(
            tag,
            Some(&mut s_mont_plus_t),
            &s_mont,
            &t_mont_dot_r,
            curve.ecc_mod_addition_opcode(),
        )?;

        // 11. Convert from montgomery representation to normal representation.
        let mut s = vec![0u8; From::<PkaEccCurve>::from(curve)];
        self.execute_unary_operation_sync(
            tag,
            Some(&mut s),
            &s_mont_plus_t,
            curve.ecc_mont_representation_out_opcode(),
        )?;

        // verify that s is not zero.
        if s.iter().all(|&x| x == 0) {
            Err(PkaErr::PkaEccSignSelfTestFailed)?
        }

        // 12. Verify signature components.
        if r != ecc_constants::get_ecc_sign_test_vectors().r[0..From::<PkaEccCurve>::from(curve)] {
            Err(PkaErr::PkaEccSignSelfTestFailed)?
        }

        if s != ecc_constants::get_ecc_sign_test_vectors().s[0..From::<PkaEccCurve>::from(curve)] {
            Err(PkaErr::PkaEccSignSelfTestFailed)?
        }

        self.input.zeroize();
        self.output.zeroize();
        self.self_test_buffer.zeroize();

        Ok(())
    }

    /// Self test for ECDH key exchange operation for a given curve
    fn ecdh_self_test_internal(&mut self, curve: PkaEccCurve) -> McrResult<()> {
        let tag = PKA_TEST_TAG;

        self.output.zeroize();
        self.self_test_buffer.zeroize();

        // Set Montgomery constant to the curve prime
        self.execute_unary_operation_sync(
            tag,
            None,
            &PkaEccCurve::get_prime(curve).p[0..From::<PkaEccCurve>::from(curve)],
            curve.ecc_mont_const_calc_opcode(),
        )?;

        // Run Point Multiplications
        let curve_len = curve.len();

        let mut public_key_buf = [0u8; PkaEccCurve::MAX_LEN * 2];

        // Create the public key using the given x-y coordinates
        public_key_buf[0..curve_len]
            .copy_from_slice(&ecc_constants::get_ecdh_test_vectors().qcavs_x[0..curve_len]);
        public_key_buf[curve_len..2 * curve_len]
            .copy_from_slice(&ecc_constants::get_ecdh_test_vectors().qcavs_y[0..curve_len]);

        // Modify test vector in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default()
            .induce_cast_failure(SelfTest::EcdhEngineInstance0, Some(self.id.into()))
        {
            public_key_buf[0] = public_key_buf[0].wrapping_add(1);
        }

        // Create buffer to store the output of the operation
        let mut secret_buf = vec![0u8; PkaEccSecretValue::data_len(curve)];

        self.execute_binary_opertion_sync(
            tag,
            Some(&mut secret_buf),
            &public_key_buf[0..2 * curve_len],
            &ecc_constants::get_ecdh_test_vectors().d_iut[0..curve_len],
            curve.ecc_point_multiplication_opcode(),
        )?;

        // Get the shared secret
        let calc_secret_val = PkaEccSecretValue::from_bytes(curve, &secret_buf)?;
        let calc_secret_key = calc_secret_val.secret();

        // Compare the shared secret key
        let expected_secret_key =
            &ecc_constants::get_ecdh_test_vectors().z_iut[0..From::<PkaEccCurve>::from(curve)];

        if calc_secret_key != expected_secret_key {
            Err(PkaErr::PkaEcdhSelfTestFail)?
        }

        self.self_test_buffer.zeroize();
        self.output.zeroize();

        Ok(())
    }

    /// Self test for rsa mod exp operation for a given test vector
    fn rsa_mod_exp_self_test_internal(&mut self, rsa_type: PkaRsaSize) -> McrResult<Vec<u8>> {
        let tag = PKA_TEST_TAG;

        // Initialize the input and output
        self.output.zeroize();
        self.self_test_buffer.zeroize();

        let rsa_len = rsa_type.len();

        let mut ciphertext = vec![0u8; rsa_len];
        ciphertext.copy_from_slice(&rsa_constants::get_rsa_2k_test_vectors().c[0..rsa_len]);

        let mut private_key = vec![0u8; rsa_len * 2];
        // Copy the private exponent (d) into the first half of the private_key buffer
        private_key[0..rsa_len]
            .copy_from_slice(&rsa_constants::get_rsa_2k_test_vectors().d[0..rsa_len]);
        // Copy the modulus (n) into the second half of the private_key buffer
        private_key[rsa_len..]
            .copy_from_slice(&rsa_constants::get_rsa_2k_test_vectors().n[0..rsa_len]);

        // Modify test vector in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default()
            .induce_cast_failure(SelfTest::Rsa2KModExpEngineInstance0, Some(self.id.into()))
        {
            private_key[0] = private_key[0].wrapping_add(1);
        }

        let mut expected_plaintext = vec![0u8; rsa_len];
        expected_plaintext.copy_from_slice(&rsa_constants::get_rsa_2k_test_vectors().k[0..rsa_len]);

        // Create buffer to store the output of the operation
        let mut calculated_plaintext = vec![0u8; rsa_len];

        self.execute_binary_opertion_sync(
            tag,
            Some(&mut calculated_plaintext),
            &ciphertext,
            &private_key,
            rsa_type.rsa_priv_key_opcode(),
        )?;

        // Compare the calculated plaintext with the expected plaintext
        if calculated_plaintext != expected_plaintext[..] {
            Err(PkaErr::PkaRsaSelfTestFail)?
        }

        self.self_test_buffer.zeroize();
        self.output.zeroize();

        Ok(calculated_plaintext)
    }

    /// Self test for rsa mod exp crt operation for a given test vector
    fn rsa_mod_exp_crt_self_test_internal(&mut self, rsa_type: PkaRsaSize) -> McrResult<()> {
        let tag = PKA_TEST_TAG;

        // Initialize the input and output
        self.output.zeroize();
        self.self_test_buffer.zeroize();

        let rsa_len = rsa_type.len();

        // Create buffer to store the output of the operation
        let mut calculated_plaintext = vec![0u8; rsa_len];

        let message = &rsa_constants::get_rsa_2k_crt_test_vectors().message;
        let crt_param1 = &rsa_constants::get_rsa_2k_crt_test_vectors()
            .param1
            .as_bytes();
        let crt_param2 = &rsa_constants::get_rsa_2k_crt_test_vectors()
            .param2
            .as_bytes()[0..rsa_len * 3];

        let message_range = 0..message.len();
        let param1_range = message_range.end..message_range.end + crt_param1.len();
        let param2_range = param1_range.end..param1_range.end + crt_param2.len();

        self.self_test_buffer[message_range.clone()].copy_from_slice(message);

        // Modify test vector in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(
            SelfTest::Rsa2KModExpCrtEngineInstance0,
            Some(self.id.into()),
        ) {
            self.self_test_buffer[0] = self.self_test_buffer[0].wrapping_add(1);
        }

        self.self_test_buffer[param1_range.clone()].copy_from_slice(crt_param1);
        self.self_test_buffer[param2_range.clone()].copy_from_slice(crt_param2);

        // Execute the operation with the self test parameters
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.self_test_buffer[message_range].as_ptr() as u32,
            self.self_test_buffer[param1_range].as_ptr() as u32,
            self.self_test_buffer[param2_range].as_ptr() as u32,
            rsa_type.rsa_crt_priv_key_opcode(),
        )?;

        // Synchronously wait for the operation to complete
        self.wait_for_operation_completion_sync(tag)?;

        let len = calculated_plaintext.len();
        calculated_plaintext.copy_from_slice(&self.output[0..len]);

        // Compare the calculated plaintext with the expected plaintext
        if calculated_plaintext != rsa_constants::get_rsa_2k_crt_test_vectors().expected_signature {
            Err(PkaErr::PkaRsaSelfTestFail)?
        }

        self.self_test_buffer.zeroize();
        self.output.zeroize();

        Ok(())
    }

    fn begin_memory_wipe(&mut self, tag: u16) -> McrResult<()> {
        if let Some(tag) = self.tag {
            let _ = self.wait_for_operation_completion_sync(tag);
        }

        self.pka_execute_command(tag, 0, 0, 0, 0, PkaCommandCode::PkaMemWipe)
    }

    fn end_memory_wipe(&mut self, tag: u16) -> McrResult<()> {
        self.check_completion(tag)
    }
}
