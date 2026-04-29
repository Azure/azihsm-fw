// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use alloc::boxed::Box;
use ecc_pct_constants::*;
use mcr_crypto_pka::*;
use mcr_crypto_sha::*;
use mcr_types::IoMemRange;
use pct_engine::*;

/// ECC Key PCT object used to execute PCT operations for ECC keys.
pub(crate) struct EccKeyPct<E: HsmEnvTrait + 'static> {
    /// State of the ECC PCT
    pub(crate) pct_state: EccPctValidationState,

    /// Private Key blob data
    pub(crate) priv_key_blob: IoMemRange,

    /// Public key blob data
    pub(crate) pub_key_blob: IoMemRange,

    /// DMA buffer to store key blob data; This variable is not used and the sole purpose
    /// is to make sure the PCT instance maintatins ownership of the allocated heap buffer.
    pub(crate) _key_blob_dma_buf: DmaBuffer<E>,

    /// PKA ECC curve type
    pub(crate) curve: PkaEccCurve,

    // Erased engine (PKA + SHA)
    engine: Box<dyn PctEngine>,

    /// Working buffer to store data for ECC operations
    pub(crate) op_dma_buf: DmaBuffer<E>,

    /// Stores context for computing ECDH
    pub(crate) ecdh_op: Option<PctEcdhComputeCmd>,
}

impl<E: HsmEnvTrait + 'static> EccKeyPct<E> {
    pub(crate) fn new(
        priv_key_blob: IoMemRange,
        pub_key_blob: IoMemRange,
        key_blob_dma_buf: DmaBuffer<E>,
        curve: PkaEccCurve,
        op_dma_buf: DmaBuffer<E>,
        engine: Box<dyn PctEngine>,
    ) -> Self {
        Self {
            pct_state: EccPctValidationState::Init,
            priv_key_blob,
            pub_key_blob,
            _key_blob_dma_buf: key_blob_dma_buf,
            curve,
            engine,
            op_dma_buf,
            ecdh_op: None,
        }
    }

    /// Verify the incoming tag with the current tag used by the crypto FSM.
    fn pka_engine_verify_tag(&self, tag: u16) -> Result<(), HsmErr> {
        let engine_tag = self.engine.peek_tag().ok_or(HsmErr::PkaEngineNotBusy)?;

        if engine_tag != tag {
            Err(HsmErr::PkaTagMismatch)?
        }

        Ok(())
    }

    /// End ECC sign op
    fn end_ecc_verify_zc(&self, tag: TagId) -> HsmResult<bool> {
        // Perform sanity check on the tag
        self.pka_engine_verify_tag(tag)?;

        // Complete the command
        let result = self
            .engine
            .end_ecc_verify_zc(tag)
            .map_err(|_| HsmErr::EccVerifyFailed)?;

        Ok(result)
    }

    /// Helper to continue ECDH compute zero copy operation with reference
    /// private key blob that is not part of user key vault.
    fn pct_ecdh_mont_const_calc_first_w_static_pub_key(
        &mut self,
        tag: TagId,
        mut op: PctEcdhComputeCmd,
    ) -> HsmResult<()> {
        // Confirm that we are in the right state.
        if op.state != EccPtMultiplicationState::WaitForMontgomeryConstCalc {
            Err(HsmErr::InvalidState)?
        }

        // Retrieve the static public key for the curve
        #[allow(static_mut_refs)]
        let static_public_key = unsafe { TEST_ECDH_KEY_PAIRS.get_public_key(self.curve) }
            .ok_or(HsmErr::InvalidArgument)?;

        // Copy the static public key into the buffer
        self.op_dma_buf.as_ref_mut()[..PkaEccCurve::MAX_LEN * 2]
            .copy_from_slice(&static_public_key.data);
        let static_public_key = self.op_dma_buf.as_ref()[0..PkaEccCurve::MAX_LEN * 2].into();

        // End montgomery constant calculation command in PKA HW.
        self.engine
            .end_montgomery_constant_calculation(tag)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        // Begin ECDH compute command in PKA HW.
        self.engine
            .begin_ecdh_compute_zc(
                tag,
                self.curve,
                self.priv_key_blob.slice(),
                &static_public_key,
            )
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        op.state = EccPtMultiplicationState::WaitForPointMultiplication;
        self.ecdh_op = Some(op);

        Ok(())
    }

    fn pct_ecdh_mont_const_calc_first_w_static_priv_key(
        &mut self,
        tag: TagId,
        mut op: PctEcdhComputeCmd,
    ) -> HsmResult<()> {
        // Confirm that we are in the right state.
        if op.state != EccPtMultiplicationState::WaitForMontgomeryConstCalc {
            Err(HsmErr::InvalidState)?
        }

        // Fetch precomputed private key from static test pairs
        #[allow(static_mut_refs)]
        let static_private_key = unsafe {
            TEST_ECDH_KEY_PAIRS
                .get_private_key(self.curve)
                .map(|key| &key.k)
        }
        .ok_or(HsmErr::InvalidArgument)?;

        let private_key_start = PkaEccCurve::MAX_LEN * 2;
        let key_length = usize::from(self.curve);

        // mutate dma_buf to store the key
        let dma_buf_mut = self.op_dma_buf.as_ref_mut();
        dma_buf_mut[private_key_start..private_key_start + key_length]
            .copy_from_slice(&static_private_key[..key_length]);

        let static_private_key: &[u8] =
            &self.op_dma_buf.as_ref()[private_key_start..private_key_start + key_length];

        // End montgomery constant calculation command in PKA HW.
        self.engine
            .end_montgomery_constant_calculation(tag)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        // Begin ECDH compute command in PKA HW.
        self.engine
            .begin_ecdh_compute_zc(tag, self.curve, static_private_key, &self.pub_key_blob)
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        op.state = EccPtMultiplicationState::WaitForPointMultiplication;
        self.ecdh_op = Some(op);

        Ok(())
    }

    /// Helper that begins the second ecdh operation.
    fn start_second_ecdh(
        &mut self,
        tag: TagId,
        mut ecdh_op: PctEcdhComputeCmd,
        secret_val: PkaEccSecretValue,
    ) -> HsmResult<()> {
        let shared_secret_start = PkaEccCurve::MAX_LEN * 3;
        let shared_secret_len = secret_val.secret().len();

        let dma_buf_mut = self.op_dma_buf.as_ref_mut();
        dma_buf_mut[shared_secret_start..shared_secret_start + shared_secret_len]
            .copy_from_slice(secret_val.secret());

        self.engine
            .begin_montgomery_constant_calculation(tag, self.curve)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        ecdh_op.state = EccPtMultiplicationState::WaitForMontgomeryConstCalc;
        self.ecdh_op = Some(ecdh_op);

        Ok(())
    }

    pub(crate) fn begin_ecc_pct_validation_inner(
        &mut self,
        tag: TagId,
        usage: EccKeyUsage,
    ) -> HsmResult<()> {
        match usage {
            EccKeyUsage::SignVerify => {
                let sha_mode = match self.curve {
                    PkaEccCurve::Ecc256 => ShaMode::Sha256,
                    PkaEccCurve::Ecc384 => ShaMode::Sha384,
                    PkaEccCurve::Ecc521 => ShaMode::Sha512,
                };
                let hw_digest_len = sha_mode.get_digest_size_hw();
                if hw_digest_len > PkaEccCurve::MAX_LEN {
                    return Err(HsmErr::InvalidArgument);
                }

                self.op_dma_buf.as_ref_mut()[PkaEccCurve::MAX_LEN..2 * PkaEccCurve::MAX_LEN]
                    .copy_from_slice(&[100u8; PkaEccCurve::MAX_LEN]);
                let input =
                    self.op_dma_buf.as_ref()[PkaEccCurve::MAX_LEN..2 * PkaEccCurve::MAX_LEN].into();

                //output buffer for the digest
                self.op_dma_buf.as_ref_mut()[..PkaEccCurve::MAX_LEN]
                    .copy_from_slice(&[0u8; PkaEccCurve::MAX_LEN]);
                let mut digest_buffer_range: IoMemRange =
                    (&self.op_dma_buf.as_ref()[..hw_digest_len]).into();

                self.engine
                    .sha_single_block_zc(sha_mode, &input, &mut digest_buffer_range)
                    .map_err(|_| HsmErr::ShaCmdFailed)?;

                let digest = self.op_dma_buf.as_ref()[..PkaEccCurve::MAX_LEN].into();
                let signature = self.op_dma_buf.as_ref()[PkaEccCurve::MAX_LEN..].into();

                // Submit the PKA command to the engine to start Sign operation
                self.engine
                    .begin_ecc_sign_zc(
                        tag,
                        self.curve,
                        self.priv_key_blob.slice(),
                        &digest,
                        &signature,
                    )
                    .map_err(|_| HsmErr::EccSignFailed)?;

                self.pct_state = EccPctValidationState::WaitForSign;

                Ok(())
            }
            EccKeyUsage::KeyAgreement => {
                self.engine
                    .begin_montgomery_constant_calculation(tag, self.curve)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                self.pct_state = EccPctValidationState::EcdhMontgomeryConstCalculationFirst;
                self.ecdh_op = Some(PctEcdhComputeCmd {
                    cmd_info: PkaEccCmd { curve: self.curve },
                    state: EccPtMultiplicationState::WaitForMontgomeryConstCalc,
                });

                Ok(())
            }
        }
    }

    /// Helper to execute continue_ecc_pct_validation_inner operation.
    pub(crate) fn continue_ecc_pct_validation_inner(&mut self, tag: TagId) -> HsmResult<()> {
        // Verify the tag.
        self.pka_engine_verify_tag(tag)?;

        match self.pct_state {
            EccPctValidationState::WaitForSign => {
                // Complete the ecc sign command
                self.engine
                    .end_ecc_sign_zc(tag)
                    .map_err(|_| HsmErr::EccSignFailed)?;

                // Prepare buffer for verification
                let digest = self.op_dma_buf.as_ref()[..PkaEccCurve::MAX_LEN].into();
                let signature = self.op_dma_buf.as_ref()[PkaEccCurve::MAX_LEN..].into();

                // Begin the ecc verify command
                self.engine
                    .begin_ecc_verify_zc(tag, self.curve, &self.pub_key_blob, &digest, &signature)
                    .map_err(|_| HsmErr::EccVerifyFailed)?;

                self.pct_state = EccPctValidationState::WaitForVerify;
            }
            EccPctValidationState::EcdhMontgomeryConstCalculationFirst => {
                let ecdh_op = self.ecdh_op.take().ok_or(HsmErr::InvalidState)?;

                // Begin first ECDH computation using generated private key & static public key
                self.pct_ecdh_mont_const_calc_first_w_static_pub_key(tag, ecdh_op)?;

                self.pct_state = EccPctValidationState::EcdhComputeFirst;
            }
            EccPctValidationState::EcdhComputeFirst => {
                let ecdh_op = self.ecdh_op.take().ok_or(HsmErr::InvalidState)?;

                let secret_val = self
                    .engine
                    .end_ecdh_compute(tag, ecdh_op.cmd_info)
                    .map_err(|_| HsmErr::EcdhComputeFailed)?;

                self.start_second_ecdh(tag, ecdh_op, secret_val)?;

                self.pct_state = EccPctValidationState::EcdhMontgomeryConstCalculationSecond;
            }
            EccPctValidationState::EcdhMontgomeryConstCalculationSecond => {
                let ecdh_op = self.ecdh_op.take().ok_or(HsmErr::InvalidState)?;

                // Begin second ECDH computation using static private key & generated public key
                self.pct_ecdh_mont_const_calc_first_w_static_priv_key(tag, ecdh_op)?;

                self.pct_state = EccPctValidationState::EcdhComputeSecond;
            }
            _ => Err(HsmErr::InvalidState)?,
        }

        Ok(())
    }

    /// Helper to execute end_ecc_pct_validation_inner operation.
    pub(crate) fn end_ecc_pct_validation_inner(&mut self, tag: TagId) -> HsmResult<bool> {
        // Verify the tag.
        self.pka_engine_verify_tag(tag)?;

        match self.pct_state {
            EccPctValidationState::WaitForVerify => match self.end_ecc_verify_zc(tag) {
                Ok(verify_result) => {
                    if !verify_result {
                        return Ok(false);
                    }
                }
                Err(err) => {
                    return Err(err);
                }
            },
            EccPctValidationState::EcdhComputeSecond => {
                let ecdh_op = self.ecdh_op.take().ok_or(HsmErr::InvalidState)?;

                let shared_secret_start = PkaEccCurve::MAX_LEN * 3;
                let shared_secret_len = PkaEccSecretValue::data_len(self.curve);

                let first_shared_secret = &self.op_dma_buf.as_ref()
                    [shared_secret_start..shared_secret_start + shared_secret_len];

                let shared_secret = self
                    .engine
                    .end_ecdh_compute(tag, ecdh_op.cmd_info)
                    .map_err(|_| HsmErr::EcdhComputeFailed)?;

                if first_shared_secret != shared_secret.secret() {
                    return Ok(false);
                }
            }
            _ => {}
        }

        self.pct_state = EccPctValidationState::ValidationComplete;

        Ok(true)
    }
}
