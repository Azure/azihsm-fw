// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

/// RSA CRT private key operation data in big-endian format.
#[derive(Clone)]
pub(crate) struct RsaPrivKeyCrt {
    /// RSA Type
    pub(crate) rsa_type: RsaSize,

    /// p (Prime p, half operand)
    pub(crate) p: SecureByteVec,

    /// q (Prime q, half operand)
    pub(crate) q: SecureByteVec,

    /// dp (dP = d (mod p-1))
    pub(crate) dp: SecureByteVec,

    /// dq (dQ = d (mod q-1))
    pub(crate) dq: SecureByteVec,

    /// n (Modulus)
    pub(crate) n: SecureByteVec,

    /// n1q ((q-1 (mod p)) * q)
    pub(crate) n1q: Option<SecureByteVec>,

    /// n2p ((p-1 (mod q)) * p)
    pub(crate) n2p: Option<SecureByteVec>,

    /// The exponent value of the RSA public key.
    pub(crate) e: SecureByteVec,

    /// Coefficient (q^-1 mod p)
    pub(crate) coefficient: SecureByteVec,
}

impl Default for RsaPrivKeyCrt {
    fn default() -> Self {
        RsaPrivKeyCrt {
            rsa_type: RsaSize::Rsa2k, // Default RSA size
            p: SecureByteVec::new(),
            q: SecureByteVec::new(),
            dp: SecureByteVec::new(),
            dq: SecureByteVec::new(),
            n: SecureByteVec::new(),
            n1q: None,
            n2p: None,
            e: SecureByteVec::new(),
            coefficient: SecureByteVec::new(),
        }
    }
}

/// Convert from a PKA compatible (little-endian) slice to RsaPubKey data structure.
pub(crate) fn pub_key_from_rsa_crt_param2_pka(
    data: &[u8],
    rsa_type: RsaSize,
) -> Result<RsaPubKey, HsmErr> {
    let full_operand_len = rsa_type.len();
    let total_expected_blob_len = 3 * full_operand_len + 4;

    let n_start = 0;
    let n_end = n_start + full_operand_len;
    let n1q_start = n_end;
    let n1q_end = n1q_start + full_operand_len;
    let n2p_start = n1q_end;
    let n2p_end = n2p_start + full_operand_len;
    let e_start = n2p_end;
    let e_end = e_start + 4;

    if data.len() != total_expected_blob_len {
        Err(HsmErr::InvalidState)?
    }

    Ok(RsaPubKey::from_bytes_le(
        rsa_type,
        &data[e_start..e_end],
        &data[n_start..n_end],
    ))
}

impl PkaConvertible for RsaPrivKeyCrt {
    type Output = SecureByteVec;
    /// CRT parameter 1
    /// Data containing the following internal memory structure (in the same order):
    /// p (Prime p, half operand)
    /// q (Prime q, half operand)
    /// dp (dP = d (mod p-1))
    /// dq (dQ = d (mod q-1))
    /// CRT parameter 2
    /// Data containing the following internal memory structure (in the same order):
    /// n (Modulus)
    /// n1q ((q-1 (mod p)) * q)
    /// n2p ((p-1 (mod q)) * p)
    /// e (public exponent)
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        let n1q_val = self.n1q.as_ref().ok_or(HsmErr::InvalidState)?;
        let n2p_val = self.n2p.as_ref().ok_or(HsmErr::InvalidState)?;
        let half_operand_len = self.rsa_type.len() / 2;
        let full_operand_len = self.rsa_type.len();
        // Total size of buffer: 5 * full operand_size + public_exponent max length.
        let mut buf = SecureByteVec::zeroed(5 * full_operand_len + 4);

        let p_start = 0;
        let p_end = half_operand_len;
        let q_start = p_end;
        let q_end = q_start + half_operand_len;
        let dp_start = q_end;
        let dp_end = dp_start + half_operand_len;
        let dq_start = dp_end;
        let dq_end = dq_start + half_operand_len;
        let n_start = dq_end;
        let n_end = n_start + full_operand_len;
        let n1q_start = n_end;
        let n1q_end = n1q_start + full_operand_len;
        let n2p_start = n1q_end;
        let n2p_end = n2p_start + full_operand_len;
        let e_start = n2p_end;

        // CRT Param 1: p
        reverse_copy_from_slice(&mut buf[p_start..p_start + self.p.len()], &self.p);

        // CRT Param 1: q
        reverse_copy_from_slice(&mut buf[q_start..q_start + self.q.len()], &self.q);

        // CRT Param 1: dp
        reverse_copy_from_slice(&mut buf[dp_start..dp_start + self.dp.len()], &self.dp);

        // CRT Param 1: dq
        reverse_copy_from_slice(&mut buf[dq_start..dq_start + self.dq.len()], &self.dq);

        // CRT Param 2: n
        reverse_copy_from_slice(&mut buf[n_start..n_start + self.n.len()], &self.n);

        // CRT Param 2: n1q
        reverse_copy_from_slice(&mut buf[n1q_start..n1q_start + n1q_val.len()], n1q_val);

        // CRT Param 2: n2p
        reverse_copy_from_slice(&mut buf[n2p_start..n2p_start + n2p_val.len()], n2p_val);

        // Public exponent
        reverse_copy_from_slice(&mut buf[e_start..e_start + self.e.len()], &self.e);

        Ok(buf)
    }
}

/// RSA CRT param compute data.
pub(crate) struct RsaCrtParamComputeCmd<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    ///  RSA Priv Key CRT
    pub(crate) priv_key_crt: RsaPrivKeyCrt,

    /// RSA Op data
    pub(crate) rsa_op_data: Option<PkaRsaCmd>,

    /// MONT_IN(q^-1 mod p)
    pub(crate) mont_in_q_inv_mod_p: Option<PkaRsaMontData>,

    /// n1q
    pub(crate) n1q: Option<PkaRsaData>,

    // p ^ -1 mod q
    pub(crate) p_inv_mod_q: Option<PkaRsaData>,

    // MONT_IN(p) - full sized operand
    pub(crate) mont_in_p_full: Option<PkaRsaMontData>,

    /// n2p
    pub(crate) n2p: Option<PkaRsaData>,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// State of the RSA CRT param compute command.
    pub(crate) state: RsaCrtParamCalcState,
}

/// List of states in the RSA CRT param computation command.
#[derive(Clone, PartialEq)]
pub(crate) enum RsaCrtParamCalcState {
    /// Idle state
    Idle,

    // Step 1: MONT_CONST_CALC(0xfff....)
    N1qWaitForMontgomeryFullMod,

    // Step 2: MONT_IN(q^-1 mod p)
    N1qWaitForQinvModPToMontIn,

    // Step 3: MONT_IN(q)
    N1qWaitForQToMontIn,

    // Step 4: MODULAR_MULT(MONT_IN(q-1mod p) * MONT_IN(q))
    N1qWaitForModMultiplication,

    // Step 5: q ^ -1 mod p * q
    N1qWaitForMontOut,

    // Step 1: MONT_CONST_CALC(q)
    N2pWaitForMontgomeryModQ,

    // Step 2: MONT_IN(p)
    N2pWaitForPToMontIn,

    // Step 3: MOD_INVERSE(p ^ -1 mod q)
    N2pWaitForModInverseP,

    // Step 4: p ^ -1 mod q
    N2pWaitForPinvModQToMontOut,

    // Step 5: MONT_CONST_CALC(0xfff....)
    N2pWaitForMontgomeryFullMod,

    // Step 6: MONT_IN(p_full)
    N2pWaitForPToMontIn2,

    // Step 7: MONT_IN(p^-1 mod q)
    N2pWaitForPInvModQToMontIn,

    // Step 8: MOD_MULT(p^-1 mod q, p)
    N2pWaitForModMultiplication,

    // Step 9: p^-1 mod q, p
    N2pWaitForMontOut,
}

impl<E: HsmEnvTrait> UserSession<E> {
    /// Begin computing RSA CRT parameters 1 and 2.
    pub(super) fn begin_compute_rsa_crt_params_inner(
        &self,
        tag: TagId,
        priv_key_crt: RsaPrivKeyCrt,
    ) -> HsmResult<RsaCrtParamComputeCmd<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, None)?;
        let slice = [0xffu8; PkaRsaSize::MAX_LEN];
        let pka_rsa_type = priv_key_crt.rsa_type.into();

        // Step 1 begin: MONT_CONST_CALC(0xfff....)
        engine_ref
            .deref()
            .begin_rsa_montgomery_constant_calculation(
                tag,
                pka_rsa_type,
                &slice[..pka_rsa_type.into()],
            )
            .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

        Ok(RsaCrtParamComputeCmd {
            tag,
            priv_key_crt,
            mont_in_q_inv_mod_p: None,
            engine_ref,
            state: RsaCrtParamCalcState::N1qWaitForMontgomeryFullMod,
            rsa_op_data: None,
            n1q: None,
            mont_in_p_full: None,
            p_inv_mod_q: None,
            n2p: None,
        })
    }

    /// Continue computing RSA CRT parameters 1 and 2.
    pub(super) fn continue_compute_rsa_crt_params_inner(
        &self,
        tag: TagId,
        op: RsaCrtParamComputeCmd<E>,
    ) -> HsmResult<RsaCrtParamComputeCmd<E>> {
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, tag)?;

        let mut rsa_op_data = None;
        let mut mont_in_q_inv_mod_p: Option<PkaRsaMontData> = op.mont_in_q_inv_mod_p;
        let mut p_inv_mod_q_half: Option<PkaRsaData> = op.p_inv_mod_q;
        let mut mont_in_p_full: Option<PkaRsaMontData> = op.mont_in_p_full;
        let mut n1q = op.n1q;
        let mut n2p = op.n2p;

        let next_state = match op.state {
            RsaCrtParamCalcState::N1qWaitForMontgomeryFullMod => {
                // Step 1 end: MONT_CONST_CALC(0xfff....)
                op.engine_ref
                    .deref()
                    .end_rsa_montgomery_constant_calculation(tag)
                    .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

                // Step 2 begin: MONT_IN(q^-1 mod p)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_in(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            op.priv_key_crt.coefficient.as_slice(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn
            }
            RsaCrtParamCalcState::N1qWaitForQinvModPToMontIn => {
                // Step 2 end: MONT_IN(q^-1 mod p).
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                mont_in_q_inv_mod_p = Some(
                    op.engine_ref
                        .deref()
                        .end_rsa_montgomery_in(tag, rsa_montgomery_in_op)
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                // Step 3 begin: MONT_IN(q)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_in(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            op.priv_key_crt.q.as_slice(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                RsaCrtParamCalcState::N1qWaitForQToMontIn
            }
            RsaCrtParamCalcState::N1qWaitForQToMontIn => {
                // Step 3 end: MONT_IN(q)
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mont_in_q_val = op
                    .engine_ref
                    .deref()
                    .end_rsa_montgomery_in(tag, rsa_montgomery_in_op)
                    .map_err(|_| HsmErr::RsaMontgomeryInFailed)?;

                // Step 4 begin: MOD_MULT(q^-1 mod p, q)
                let mont_in_q_inv_mod_p_val =
                    mont_in_q_inv_mod_p.as_ref().ok_or(HsmErr::InvalidState)?;
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_modular_multiplication(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            mont_in_q_inv_mod_p_val.data_be(),
                            mont_in_q_val.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaModularMultiplicationFailed)?,
                );

                RsaCrtParamCalcState::N1qWaitForModMultiplication
            }
            RsaCrtParamCalcState::N1qWaitForModMultiplication => {
                // Step 4 end: MOD_MULT(q^-1 mod p, q)
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mod_mult_mont = op
                    .engine_ref
                    .deref()
                    .end_rsa_modular_multiplication(tag, rsa_montgomery_in_op)
                    .map_err(|_| HsmErr::RsaModularMultiplicationFailed)?;

                // Step 5 begin: MONT_OUT(result)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_out(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            mod_mult_mont.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                RsaCrtParamCalcState::N1qWaitForMontOut
            }
            RsaCrtParamCalcState::N1qWaitForMontOut => {
                // Step 5 end: MONT_OUT(result).
                let rsa_montgomery_out_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                n1q = Some(
                    op.engine_ref
                        .deref()
                        .end_rsa_montgomery_out(tag, rsa_montgomery_out_op)
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                let pka_rsa_type = Self::map_half_operand_size_pka(op.priv_key_crt.rsa_type);

                // Step 1 begin: MONT_CONST_CALC(q)
                op.engine_ref
                    .deref()
                    .begin_rsa_montgomery_constant_calculation(
                        tag,
                        pka_rsa_type,
                        op.priv_key_crt.q.as_slice(),
                    )
                    .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

                RsaCrtParamCalcState::N2pWaitForMontgomeryModQ
            }
            RsaCrtParamCalcState::N2pWaitForMontgomeryModQ => {
                // Step 1 end: MONT_CONST_CALC(q)
                op.engine_ref
                    .deref()
                    .end_rsa_montgomery_constant_calculation(tag)
                    .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

                let pka_rsa_type = Self::map_half_operand_size_pka(op.priv_key_crt.rsa_type);

                // Step 2 begin: MONT_IN(p)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_in(tag, pka_rsa_type, op.priv_key_crt.p.as_slice())
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForPToMontIn
            }
            RsaCrtParamCalcState::N2pWaitForPToMontIn => {
                // Step 2 end: MONT_IN(p).
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mont_in_p_half_val = op
                    .engine_ref
                    .deref()
                    .end_rsa_montgomery_in(tag, rsa_montgomery_in_op)
                    .map_err(|_| HsmErr::RsaMontgomeryInFailed)?;

                // Step 3 begin: MOD_INVERSE(p^-1 mod q)
                let pka_rsa_type = Self::map_half_operand_size_pka(op.priv_key_crt.rsa_type);
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_modular_inverse(tag, pka_rsa_type, mont_in_p_half_val.data_be())
                        .map_err(|_| HsmErr::RsaModularInverseFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForModInverseP
            }
            RsaCrtParamCalcState::N2pWaitForModInverseP => {
                // Step 3 end: MOD_INVERSE(p^-1 mod q)
                let rsa_mod_inverse_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mod_inverse_mont_result = op
                    .engine_ref
                    .deref()
                    .end_rsa_modular_inverse(tag, rsa_mod_inverse_op)
                    .map_err(|_| HsmErr::RsaModularInverseFailed)?;

                // Step 4 begin: MONT_OUT(MOD_INVERSE(p^-1 mod q))
                let pka_rsa_type = Self::map_half_operand_size_pka(op.priv_key_crt.rsa_type);
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_out(
                            tag,
                            pka_rsa_type,
                            mod_inverse_mont_result.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut
            }
            RsaCrtParamCalcState::N2pWaitForPinvModQToMontOut => {
                // Step 4 end: MONT_OUT(MOD_INVERSE(p^-1 mod q)).
                let rsa_montgomery_out_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                p_inv_mod_q_half = Some(
                    op.engine_ref
                        .deref()
                        .end_rsa_montgomery_out(tag, rsa_montgomery_out_op)
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                // Step 5 begin: MONT_CONST_CALC(0xfff...)
                let slice = [0xffu8; PkaRsaSize::MAX_LEN];
                let pka_rsa_type = op.priv_key_crt.rsa_type.into();
                op.engine_ref
                    .deref()
                    .begin_rsa_montgomery_constant_calculation(
                        tag,
                        pka_rsa_type,
                        &slice[..pka_rsa_type.into()],
                    )
                    .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

                RsaCrtParamCalcState::N2pWaitForMontgomeryFullMod
            }
            RsaCrtParamCalcState::N2pWaitForMontgomeryFullMod => {
                // Step 5 end: MONT_CONST_CALC(0xfff...)
                op.engine_ref
                    .deref()
                    .end_rsa_montgomery_constant_calculation(tag)
                    .map_err(|_| HsmErr::RsaMontgomeryConstCalcFailed)?;

                // Step 6 begin: MONT_IN(p_full)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_in(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            op.priv_key_crt.p.as_slice(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForPToMontIn2
            }
            RsaCrtParamCalcState::N2pWaitForPToMontIn2 => {
                // Step 6 end: MONT_IN(p_full)
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                mont_in_p_full = Some(
                    op.engine_ref
                        .deref()
                        .end_rsa_montgomery_in(tag, rsa_montgomery_in_op)
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                // Step 7 begin: MONT_IN(p ^ -1 mod q)
                let p_inv_mod_q_val = p_inv_mod_q_half.as_ref().ok_or(HsmErr::InvalidState)?;
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_in(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            p_inv_mod_q_val.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryInFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn
            }
            RsaCrtParamCalcState::N2pWaitForPInvModQToMontIn => {
                // Step 7 end: MONT_IN(p ^ -1 mod q)
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mont_p_inv_mod_q_full = op
                    .engine_ref
                    .deref()
                    .end_rsa_montgomery_in(tag, rsa_montgomery_in_op)
                    .map_err(|_| HsmErr::RsaMontgomeryInFailed)?;

                // Step 8 begin: MOD_MULT (p ^ -1 mod q, p)
                let mont_in_p_full_val = mont_in_p_full.as_ref().ok_or(HsmErr::InvalidState)?;
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_modular_multiplication(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            mont_in_p_full_val.data_be(),
                            mont_p_inv_mod_q_full.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaModularMultiplicationFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForModMultiplication
            }
            RsaCrtParamCalcState::N2pWaitForModMultiplication => {
                //  Step 8 end: MOD_MULT (p ^ -1 mod q, p)
                let rsa_montgomery_in_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                let mont_n2p = op
                    .engine_ref
                    .deref()
                    .end_rsa_modular_multiplication(tag, rsa_montgomery_in_op)
                    .map_err(|_| HsmErr::RsaModularMultiplicationFailed)?;

                // Step 9 begin: MONT_OUT(n2p)
                rsa_op_data = Some(
                    op.engine_ref
                        .deref()
                        .begin_rsa_montgomery_out(
                            tag,
                            op.priv_key_crt.rsa_type.into(),
                            mont_n2p.data_be(),
                        )
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                RsaCrtParamCalcState::N2pWaitForMontOut
            }
            RsaCrtParamCalcState::N2pWaitForMontOut => {
                // Step 9 end: MONT_OUT(n2p).
                let rsa_montgomery_out_op = op.rsa_op_data.ok_or(HsmErr::InvalidState)?;
                n2p = Some(
                    op.engine_ref
                        .deref()
                        .end_rsa_montgomery_out(tag, rsa_montgomery_out_op)
                        .map_err(|_| HsmErr::RsaMontgomeryOutFailed)?,
                );

                RsaCrtParamCalcState::Idle
            }
            _ => Err(HsmErr::InvalidState)?,
        };

        Ok(RsaCrtParamComputeCmd {
            tag,
            priv_key_crt: op.priv_key_crt,
            mont_in_q_inv_mod_p,
            engine_ref: op.engine_ref,
            state: next_state,
            rsa_op_data,
            n1q,
            p_inv_mod_q: p_inv_mod_q_half,
            mont_in_p_full,
            n2p,
        })
    }

    /// End computing RSA CRT parameters 1 and 2.
    pub(super) fn end_compute_rsa_crt_params_inner(
        &self,
        mut op: RsaCrtParamComputeCmd<E>,
    ) -> HsmResult<RsaPrivKeyCrt> {
        let n1q = op.n1q.ok_or(HsmErr::InvalidState)?;
        let n2p = op.n2p.ok_or(HsmErr::InvalidState)?;

        let mut priv_key_crt = core::mem::take(&mut op.priv_key_crt);
        priv_key_crt.n1q = Some(SecureByteVec::from(n1q.data_be()));
        priv_key_crt.n2p = Some(SecureByteVec::from(n2p.data_be()));

        // Release the PKA engine

        Ok(priv_key_crt)
    }

    /// Compute size of half-operand required by the PKA RSA operation.
    fn map_half_operand_size_pka(rsa_type: RsaSize) -> PkaRsaSize {
        match rsa_type {
            RsaSize::Rsa2k => PkaRsaSize::Rsa1k,
            RsaSize::Rsa3k => PkaRsaSize::Rsa2k,
            RsaSize::Rsa4k => PkaRsaSize::Rsa2k,
        }
    }
}
