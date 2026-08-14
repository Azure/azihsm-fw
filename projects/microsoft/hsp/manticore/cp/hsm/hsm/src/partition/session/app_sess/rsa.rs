// Copyright (c) Microsoft Corporation. All rights reserved.

use core::cmp::Ordering;

use super::*;
use crate::error::HsmErr;
use crate::partition::PkaConvertible;
use mcr_crypto_sha::*;
use mcr_types::UnwrappingKeyValidity;

/// RSA 4096 bit public key operation data.
/// Enumeration to denote the RSA key sizes.
#[derive(Copy, Clone, PartialEq, Eq)]
pub(crate) enum RsaSize {
    /// RSA 2k
    Rsa2k,

    /// RSA 3k
    Rsa3k,

    /// RSA 4k
    Rsa4k,
}

impl RsaSize {
    /// Maximum length of RSA keys.
    pub const MAX_LEN: usize = 512;

    /// Get the RSA key length
    #[allow(clippy::len_without_is_empty)]
    pub fn len(&self) -> usize {
        usize::from(*self)
    }
}

impl From<RsaSize> for usize {
    /// Converts to this type from the input type.
    fn from(rsa_type: RsaSize) -> Self {
        match rsa_type {
            RsaSize::Rsa2k => 256,
            RsaSize::Rsa3k => 384,
            RsaSize::Rsa4k => 512,
        }
    }
}

impl From<RsaSize> for PkaRsaSize {
    /// Converts to this type from the input type.
    fn from(rsa_type: RsaSize) -> Self {
        match rsa_type {
            RsaSize::Rsa2k => PkaRsaSize::Rsa2k,
            RsaSize::Rsa3k => PkaRsaSize::Rsa3k,
            RsaSize::Rsa4k => PkaRsaSize::Rsa4k,
        }
    }
}

impl TryFrom<PkaRsaSize> for RsaSize {
    type Error = HsmErr;

    fn try_from(rsa_type: PkaRsaSize) -> Result<Self, Self::Error> {
        match rsa_type {
            PkaRsaSize::Rsa2k => Ok(RsaSize::Rsa2k),
            PkaRsaSize::Rsa3k => Ok(RsaSize::Rsa3k),
            PkaRsaSize::Rsa4k => Ok(RsaSize::Rsa4k),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

impl TryFrom<usize> for RsaSize {
    type Error = HsmErr;

    fn try_from(rsa_size: usize) -> Result<Self, Self::Error> {
        match rsa_size {
            256 => Ok(RsaSize::Rsa2k),
            384 => Ok(RsaSize::Rsa3k),
            512 => Ok(RsaSize::Rsa4k),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

/// RSA Public Key data in little-endian format.
#[derive(PartialEq, Clone)]
pub struct RsaPubKey {
    /// RSA Type
    /// This is marked as dead code, since the firmware never consumes the value of this field
    #[allow(dead_code)]
    pub(crate) rsa_type: RsaSize,

    /// The exponent + modulus in little endian
    pub data: SecureByteArray<{ RsaSize::MAX_LEN + 4 }>,

    /// Length of the modulus.
    pub n_len: usize,
}

impl RsaPubKey {
    /// Get RSA exponent
    pub fn exponent_le(&self) -> &[u8] {
        &self.data[self.n_len..self.n_len + 4]
    }

    /// Get RSA modulus
    pub fn modulus_le(&self) -> &[u8] {
        &self.data[..self.n_len]
    }

    /// Get RSA exponent, big-endian
    pub fn exponent_be(&self) -> Vec<u8> {
        let mut out = [0u8; 4];
        reverse_copy(&mut out, self.exponent_le());
        out.to_vec()
    }

    /// Get RSA modulus, big-endian
    pub fn modulus_be(&self) -> Vec<u8> {
        let mut out = [0u8; RsaSize::MAX_LEN];
        reverse_copy(&mut out[..self.n_len], self.modulus_le());
        out[..self.n_len].to_vec()
    }

    /// Constructs the RSA Public Key in little-endian format from constituent big-endian components.
    ///
    /// # Arguments
    /// * `RSA type` - RSA key type
    /// * `exponent` - exponent slice
    /// * `modulus` - modulus slice
    ///
    /// # Returns
    /// RSA public key data as RsaPubKey structure.
    pub(crate) fn from_bytes_be(rsa_type: RsaSize, exponent: &[u8], modulus: &[u8]) -> Self {
        RsaPubKey {
            rsa_type,
            data: {
                let mut data =
                    SecureByteArray::<{ RsaSize::MAX_LEN + 4 }>::new([0u8; RsaSize::MAX_LEN + 4]);
                // reverse_copy to convert big endian to little endian
                reverse_copy(
                    &mut data[rsa_type.len() - modulus.len()..rsa_type.len()],
                    modulus,
                );
                reverse_copy(
                    &mut data[rsa_type.len()..rsa_type.len() + exponent.len()],
                    exponent,
                );
                data
            },
            n_len: modulus.len(),
        }
    }

    pub(crate) fn from_bytes_le(rsa_type: RsaSize, exponent: &[u8], modulus: &[u8]) -> Self {
        RsaPubKey {
            rsa_type,
            data: {
                let mut data =
                    SecureByteArray::<{ RsaSize::MAX_LEN + 4 }>::new([0u8; RsaSize::MAX_LEN + 4]);
                data[..modulus.len()].copy_from_slice(modulus);
                data[rsa_type.len()..rsa_type.len() + exponent.len()].copy_from_slice(exponent);
                data
            },
            n_len: modulus.len(),
        }
    }

    /// Convert from private key PKA data to create RsaPubKey data structure.
    pub(crate) fn from_priv_pka_slice(data: &[u8], rsa_type: RsaSize) -> Result<Self, HsmErr> {
        let single_operand_len = rsa_type.len();
        let double_operand_len = single_operand_len * 2;
        let n_start = single_operand_len;
        let e_end = double_operand_len + 4;
        Ok(RsaPubKey {
            rsa_type,
            data: {
                let mut new_data =
                    SecureByteArray::<{ RsaSize::MAX_LEN + 4 }>::new([0u8; RsaSize::MAX_LEN + 4]);
                new_data[..4 + rsa_type.len()].copy_from_slice(&data[n_start..e_end]);
                new_data
            },
            n_len: rsa_type.len(),
        })
    }

    /// Convert from private crt key PKA data to create RsaPubKey data structure.
    pub(crate) fn from_priv_crt_pka_slice(data: &[u8], rsa_type: RsaSize) -> Result<Self, HsmErr> {
        let half_len = rsa_type.len() / 2;
        let full_len = rsa_type.len();

        // Calculate offsets directly where needed
        let n_start = half_len * 4; // After p, q, dp, dq
        let e_start = n_start + full_len * 3; // After n, n1q, n2p
        let mut new_data =
            SecureByteArray::<{ RsaSize::MAX_LEN + 4 }>::new([0u8; RsaSize::MAX_LEN + 4]);
        new_data[..full_len].copy_from_slice(&data[n_start..n_start + full_len]);
        new_data[full_len..full_len + 4].copy_from_slice(&data[e_start..e_start + 4]);

        Ok(RsaPubKey {
            rsa_type,
            data: new_data,
            n_len: rsa_type.len(),
        })
    }
}

impl PkaConvertible for RsaPubKey {
    type Output = Vec<u8>;
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        Ok(self.data[..4 + self.n_len].to_vec())
    }
}

impl PkaConvertibleZc for RsaPubKey {
    fn pka_as_slice(&self) -> Result<&[u8], HsmErr> {
        Ok(&self.data[..4 + self.n_len])
    }
}

/// RSA private key operation data in big-endian format.
#[repr(C)]
pub struct RsaPrivKey {
    /// RSA Type
    pub(crate) rsa_type: RsaSize,

    /// The exponent value of the RSA private key.
    pub(crate) d: SecureByteArray<{ RsaSize::MAX_LEN }>,

    /// The modulus value of the RSA private key.
    pub(crate) n: SecureByteArray<{ RsaSize::MAX_LEN }>,

    /// The exponent value of the RSA public key.
    pub(crate) e: SecureByteArray<4>,
}

impl RsaPrivKey {
    /// Constructs the RSA Private Key in big-endian format from constituent big-endian components.
    ///
    /// # Arguments
    /// * `RSA type` - RSA key type
    /// * `private_exponent` - Private exponent slice
    /// * `modulus` - modulus slice
    /// * `public_exponent` - Public exponent slice
    ///
    /// # Returns
    /// RSA private key data as RsaPrivKey structure.
    pub(crate) fn from_bytes_be(
        rsa_type: RsaSize,
        private_exponent: &[u8],
        modulus: &[u8],
        public_exponent: &[u8],
    ) -> Self {
        RsaPrivKey {
            rsa_type,
            d: {
                let mut d = SecureByteArray::<{ RsaSize::MAX_LEN }>::new([0; RsaSize::MAX_LEN]);
                d[RsaSize::MAX_LEN - private_exponent.len()..RsaSize::MAX_LEN]
                    .copy_from_slice(private_exponent);
                d
            },
            n: {
                let mut n = SecureByteArray::<{ RsaSize::MAX_LEN }>::new([0; RsaSize::MAX_LEN]);
                n[RsaSize::MAX_LEN - modulus.len()..RsaSize::MAX_LEN].copy_from_slice(modulus);
                n
            },
            e: {
                let mut e = SecureByteArray::<4>::new([0; 4]);
                e[4 - public_exponent.len()..4].copy_from_slice(public_exponent);
                e
            },
        }
    }
}

impl PkaConvertible for RsaPrivKey {
    type Output = SecureByteVec;

    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        let mut binding = SecureByteVec::zeroed(RsaSize::MAX_LEN * 2 + 4);
        let buf = binding.as_mut_slice();

        let d_start = 0;
        let d_end = d_start + self.rsa_type.len();
        let n_start = d_end;
        let n_end = n_start + self.rsa_type.len();
        let e_start = n_end;
        let e_end = e_start + 4;

        let d_reversed = rsa_reverse_copy_to_slice(self.d.as_slice());
        buf[d_start..d_end].copy_from_slice(&d_reversed[..self.rsa_type.len()]);

        let n_reversed = rsa_reverse_copy_to_slice(self.n.as_slice());
        buf[n_start..n_end].copy_from_slice(&n_reversed[..self.rsa_type.len()]);

        let public_exponent_reversed = rsa_reverse_copy_to_slice(self.e.as_slice());
        buf[e_start..e_end].copy_from_slice(&public_exponent_reversed[..4]);

        Ok(buf[..2 * self.rsa_type.len() + 4].into())
    }
}

impl<E: HsmEnvTrait> UserSession<E> {
    /// Helper to execute begin_rsa_mod_exp_zc operation.
    pub(super) fn _begin_rsa_mod_exp_zc(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: Option<RsaKeyUsage>,
        input: &IoMemRange,
        output: &IoMemRange,
        app_vault_id: AppVaultId,
    ) -> HsmResult<RsaModExp<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, Some(key_id))?;

        self._begin_rsa_mod_exp_zc_with_engine(
            tag,
            engine_ref,
            key_id,
            usage,
            input,
            output,
            app_vault_id,
        )
    }

    /// Helper to execute begin_rsa_mod_exp_zc_with_engine operation.
    #[allow(clippy::too_many_arguments)]
    pub(super) fn _begin_rsa_mod_exp_zc_with_engine(
        &self,
        tag: TagId,
        engine_ref: PkaEngineRef<E>,
        key_id: KeyId,
        usage: Option<RsaKeyUsage>,
        input: &IoMemRange,
        output: &IoMemRange,
        app_vault_id: AppVaultId,
    ) -> HsmResult<RsaModExp<E>> {
        // Input or Output should not be empty.
        if input.is_empty() || output.is_empty() {
            return Err(HsmErr::InvalidArgument);
        }

        // Get the required key from the key vault
        let key = self.rsa_key(key_id, app_vault_id, usage)?;
        let kind = key.kind()?;

        let expected_data_len = key.expected_data_len()?;
        let is_crt = key.is_crt_key()?;

        if input.len() != expected_data_len {
            Err(HsmErr::InvalidArgument)?
        }

        // Make sure the input data value, m, meets the criteria: 1 < m < n - 1
        // where n is the modulus of the RSA key; based on NIST ACVP test vector requirements.
        let key_modulus = key.n()?;
        if !Self::check_valid_input(&key_modulus, input.slice()) {
            Err(HsmErr::InvalidArgument)?;
        }

        let rsa_type = match kind {
            EntryKind::Rsa2kPrivate | EntryKind::Rsa2kPrivateCrt => PkaRsaSize::Rsa2k,
            EntryKind::Rsa3kPrivate | EntryKind::Rsa3kPrivateCrt => PkaRsaSize::Rsa3k,
            EntryKind::Rsa4kPrivate | EntryKind::Rsa4kPrivateCrt => PkaRsaSize::Rsa4k,
            _ => Err(HsmErr::InvalidKeyType)?,
        };

        // Submit the PKA command to the engine
        let cmd_info = if is_crt {
            let crt_param1 = key.crt_param1()?;
            let crt_param2 = key.crt_param2()?;

            engine_ref
                .deref()
                .begin_rsa_private_key_op_crt_zc(
                    tag,
                    rsa_type,
                    &crt_param1,
                    &crt_param2,
                    input,
                    output,
                )
                .map_err(|_| HsmErr::RsaModExpFailed)?
        } else {
            let key = key.blob()?;

            engine_ref
                .deref()
                .begin_rsa_private_key_op_zc(tag, rsa_type, &key, input, output)
                .map_err(|_| HsmErr::RsaModExpFailed)?
        };

        Ok(RsaModExp {
            tag,
            engine_ref,
            is_crt,
            cmd_info,
        })
    }

    /// Helper to execute end_rsa_mod_exp_zc operation.
    pub(super) fn _end_rsa_mod_exp_zc(&self, tag: TagId, op: &RsaModExp<E>) -> HsmResult<()> {
        // Perform sanity check on the tag
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, tag)?;

        // Complete the command
        if op.is_crt {
            op.engine_ref
                .deref()
                .end_rsa_private_key_op_crt_zc(tag, op.cmd_info)
                .map_err(|_| HsmErr::RsaModExpFailed)
        } else {
            op.engine_ref
                .deref()
                .end_rsa_private_key_op_zc(tag, op.cmd_info)
                .map_err(|_| HsmErr::RsaModExpFailed)
        }
    }

    /// Helper to execute begin_rsa_pub_mod_exp_inner_zc_with_engine operation.
    fn begin_rsa_pub_mod_exp_inner_zc_with_engine(
        &self,
        tag: TagId,
        engine_ref: PkaEngineRef<E>,
        rsa_type: PkaRsaSize,
        pub_key_blob: &IoMemRange,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> HsmResult<RsaModExp<E>> {
        if input.is_empty() || output.is_empty() {
            return Err(HsmErr::InvalidArgument);
        }

        let cmd_info = engine_ref
            .deref()
            .begin_rsa_public_key_op_zc(tag, rsa_type, pub_key_blob, input, output)
            .map_err(|_| HsmErr::RsaModExpFailed)?;

        Ok(RsaModExp {
            tag,
            engine_ref,
            is_crt: false,
            cmd_info,
        })
    }

    /// Helper to execute end_rsa_pub_mod_exp_inner_zc operation.
    pub(super) fn end_rsa_pub_mod_exp_inner_zc(
        &self,
        tag: TagId,
        op: &RsaModExp<E>,
    ) -> HsmResult<()> {
        // Perform sanity check on the tag
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, tag)?;

        // Complete the command
        op.engine_ref
            .deref()
            .end_rsa_public_key_op_zc(tag, op.cmd_info)
            .map_err(|_| HsmErr::RsaModExpFailed)
    }

    /// Helper to execute begin_rsa_pct_validation_inner operation.
    pub(super) fn begin_rsa_pct_validation_inner(
        &self,
        tag: TagId,
        key_id: KeyId,
        usage: RsaKeyUsage,
        rsa_type: PkaRsaSize,
        n: &[u8],
        e: &[u8],
    ) -> HsmResult<RsaPctValidationCmd<E>> {
        let engine_ref = self.pka_engine_acquire(tag, Some(key_id))?;
        let expected_len: usize = rsa_type.into();
        let pub_key_blob_len = e.len() + expected_len;
        let pub_key_offset = expected_len * 3;

        // Layout: [digest | signature | result | public_key_blob], where each section is `expected_len` bytes,
        // and the final public key blob is appended at the end.
        let mut dma_buf = self
            .state
            .env()
            .dma_heap()
            .allocate(expected_len * 3 + pub_key_blob_len)
            .ok_or(HsmErr::DmaAllocFailure)?;

        // Copy e and n to DMA buffer at the correct offset
        dma_buf.as_ref_mut()[pub_key_offset..pub_key_offset + 4].copy_from_slice(e);
        dma_buf.as_ref_mut()[pub_key_offset + 4..pub_key_offset + 4 + expected_len]
            .copy_from_slice(n);

        let state;
        let rsa_op;

        match usage {
            RsaKeyUsage::SignVerify => {
                let sha_type = match rsa_type {
                    PkaRsaSize::Rsa1k => ShaType::Sha256,
                    PkaRsaSize::Rsa2k => ShaType::Sha256,
                    PkaRsaSize::Rsa3k => ShaType::Sha384,
                    PkaRsaSize::Rsa4k => ShaType::Sha512,
                };
                let hw_digest_len = sha_type.get_digest_size_hw();

                dma_buf.as_ref_mut()[expected_len..expected_len * 2]
                    .copy_from_slice(&[100u8; 512][..expected_len]);
                let input = dma_buf.as_ref()[expected_len..expected_len * 2].into();

                dma_buf.as_ref_mut()[hw_digest_len..expected_len]
                    .copy_from_slice(&[0u8; 512][hw_digest_len..expected_len]);
                let digest_buffer_mbor: MborByteArray<64> = MborByteArray::new_with_len(
                    dma_buf.as_ref()[..hw_digest_len].as_ptr(),
                    hw_digest_len,
                );

                self.sha_single_block_zc(sha_type, &input, &mut (&digest_buffer_mbor).into())?;

                let digest = dma_buf.as_ref()[..expected_len].into();
                let signature = dma_buf.as_ref()[expected_len..expected_len * 2].into();

                rsa_op = self._begin_rsa_mod_exp_zc_with_engine(
                    tag,
                    engine_ref,
                    key_id,
                    Some(usage),
                    &digest,
                    &signature,
                    self.app_vault_id(),
                )?;
                state = RsaPctValidationState::WaitForSign;
            }
            RsaKeyUsage::EncryptDecrypt | RsaKeyUsage::Unwrap => {
                dma_buf.as_ref_mut()[..expected_len].copy_from_slice(&[0x5A; 512][..expected_len]);
                dma_buf.as_ref_mut()[expected_len - 1] = 0;

                let input = dma_buf.as_ref()[..expected_len].into();
                let output = dma_buf.as_ref()[expected_len..expected_len * 2].into();
                let pub_key =
                    dma_buf.as_ref()[pub_key_offset..pub_key_offset + pub_key_blob_len].into();
                rsa_op = self.begin_rsa_pub_mod_exp_inner_zc_with_engine(
                    tag, engine_ref, rsa_type, &pub_key, &input, &output,
                )?;
                state = RsaPctValidationState::WaitForEncrypt;
            }
        }

        Ok(RsaPctValidationCmd {
            tag,
            key_id,
            rsa_type,
            usage,
            dma_buf,
            rsa_op,
            state,
        })
    }

    /// Helper to execute continue_rsa_pct_validation_inner operation.
    pub(super) fn continue_rsa_pct_validation_inner(
        &self,
        mut op: RsaPctValidationCmd<E>,
    ) -> HsmResult<RsaPctValidationCmd<E>> {
        let expected_len: usize = op.rsa_type.into();

        match op.state {
            RsaPctValidationState::WaitForSign => {
                self._end_rsa_mod_exp_zc(op.tag, &op.rsa_op)?;
                let engine_ref = op.rsa_op.engine_ref;
                let pub_key_offset = expected_len * 3;
                let pub_key_blob_len = 4 + expected_len;

                let signature = op.dma_buf.as_ref()[expected_len..expected_len * 2].into();
                let digest = op.dma_buf.as_ref()[expected_len * 2..expected_len * 3].into();
                let pub_key =
                    op.dma_buf.as_ref()[pub_key_offset..pub_key_offset + pub_key_blob_len].into();

                let pub_op = self.begin_rsa_pub_mod_exp_inner_zc_with_engine(
                    op.tag,
                    engine_ref,
                    op.rsa_type,
                    &pub_key,
                    &signature,
                    &digest,
                )?;

                op.rsa_op = pub_op;
                op.state = RsaPctValidationState::WaitForVerify;
            }
            RsaPctValidationState::WaitForEncrypt => {
                self.end_rsa_pub_mod_exp_inner_zc(op.tag, &op.rsa_op)?;
                let engine_ref = op.rsa_op.engine_ref;

                let input = op.dma_buf.as_ref()[expected_len..expected_len * 2].into();
                let output = op.dma_buf.as_ref()[expected_len * 2..expected_len * 3].into();
                let app_vault_id = match op.usage {
                    RsaKeyUsage::Unwrap => APP_VAULT_ID_FOR_INTERNAL_KEYS,
                    _ => self.app_vault_id(),
                };

                let priv_op = self._begin_rsa_mod_exp_zc_with_engine(
                    op.tag,
                    engine_ref,
                    op.key_id,
                    Some(op.usage),
                    &input,
                    &output,
                    app_vault_id,
                )?;

                op.rsa_op = priv_op;
                op.state = RsaPctValidationState::WaitForDecrypt;
            }
            _ => return Err(HsmErr::InvalidState),
        }

        Ok(op)
    }

    /// Helper to execute end_rsa_pct_validation_inner operation.
    pub(super) fn end_rsa_pct_validation_inner(
        &self,
        mut op: RsaPctValidationCmd<E>,
    ) -> HsmResult<bool> {
        let expected_len: usize = op.rsa_type.into();

        let expected = &op.dma_buf.as_ref()[..expected_len];
        let result = &op.dma_buf.as_ref()[expected_len * 2..expected_len * 3];

        let is_valid = match op.state {
            RsaPctValidationState::WaitForVerify => {
                self.end_rsa_pub_mod_exp_inner_zc(op.tag, &op.rsa_op)?;
                result == expected
            }
            RsaPctValidationState::WaitForDecrypt => {
                self._end_rsa_mod_exp_zc(op.tag, &op.rsa_op)?;
                result == expected
            }
            _ => return Err(HsmErr::InvalidState),
        };

        op.state = RsaPctValidationState::ValidationComplete;

        Ok(is_valid)
    }

    /// Helper to get the unwrapping key.
    pub(super) fn get_unwrapping_key_inner(
        &self,
        _tag: TagId,
        key_id: Option<KeyId>,
        _pfn: PcieFunction,
    ) -> HsmResult<GetUnwrappingKeyCtx> {
        if let Some(key_id) = key_id {
            self.get_unwrapping_key_from_vault(key_id)
        } else if self
            .state
            .part_persistent_store_ref()
            .unwrapping_key_bk_valid
            != UnwrappingKeyValidity::Empty as u8
        {
            Ok(GetUnwrappingKeyCtx {
                output: Some(self.import_unwrapping_key_from_bk()?),
            })
        } else {
            Err(HsmErr::PendingKeyGeneration)
        }
    }

    /// Get unwrapping key from key vault
    fn get_unwrapping_key_from_vault(&self, key_id: u16) -> Result<GetUnwrappingKeyCtx, HsmErr> {
        let key = self.rsa_key(
            key_id,
            APP_VAULT_ID_FOR_INTERNAL_KEYS,
            Some(RsaKeyUsage::Unwrap),
        )?;

        let blob = key.blob()?;
        let key_data = blob.as_bytes();

        let pub_key = RsaPubKey::from_priv_pka_slice(key_data, RsaSize::Rsa2k)?;

        let unwrapping_key_ctx = GetUnwrappingKeyCtx {
            output: Some(GetUnwrappingKeyOut {
                id: key_id,
                data: pub_key,
            }),
        };

        Ok(unwrapping_key_ctx)
    }

    /// Import unwrapping key from partition persistent store
    fn import_unwrapping_key_from_bk(&self) -> HsmResult<GetUnwrappingKeyOut> {
        if self.state.unwrapping_key_id().is_some() {
            Err(HsmErr::InvalidState)?;
        }

        let key_data = self
            .state
            .part_persistent_store_ref()
            .unwrapping_key_bk
            .as_slice();

        // Store the key in the HSM key vault
        let key_id = self.import_unwrapping_key(key_data)?;

        // Set the key id once it is in the key vault
        self.state.set_unwrapping_key_id(Some(key_id));

        // Construct RsaPubKey from the raw key data
        let pub_key = RsaPubKey::from_priv_pka_slice(key_data, RsaSize::Rsa2k)?;

        Ok(GetUnwrappingKeyOut {
            id: key_id,
            data: pub_key,
        })
    }

    /// Get the RSA Key.
    pub(crate) fn rsa_key(
        &self,
        key_id: KeyId,
        app_vault_id: AppVaultId,
        usage: Option<RsaKeyUsage>,
    ) -> HsmResult<RsaKey> {
        let key = self
            .state
            .vault()
            .rsa_key(app_vault_id, self.id(), key_id, usage)?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }

    /// Validate the message/input meets the condition: 1 < m < n - 1
    /// In order to ensure the input is valid, we need to make sure that 1 < m < n - 1,
    /// To do this we first check from Most Significant Byte to Least Significant Byte.
    /// Since n and m are both in little endian format, we need to check from
    /// slice[len - 1] to slice[0]. Note: n and m are in little endian format.
    pub fn check_valid_input(n: &[u8], m: &[u8]) -> bool {
        // sanity check
        if n.len() != m.len() {
            return false;
        }

        // used to keep track of whether m > 1
        let mut m_gt_one = false;

        // Compare bytes from most to least significant (right to left in little-endian)
        // At non-LSB positions (i > 0):
        // - If n[i] > m[i]: The difference is at least 2^8 (256), which guarantees m < n-1
        // - If m[i] > n[i]: The value m exceeds n, so m < n-1 is false
        // - If equal: Continue checking lower bytes
        for i in (1..n.len()).rev() {
            // check if m[i] > 0, when i > 0
            m_gt_one |= m[i] > 0;

            match n[i].cmp(&m[i]) {
                Ordering::Greater => {
                    let gap = n[i] - m[i];
                    // When gap == 1, m could equal n - 1.
                    // This happens if n's lower bytes are all 0x00
                    // and m's lower bytes are all 0xFF (the borrow pattern).
                    if gap == 1
                        && n[..i].iter().all(|&b| b == 0)
                        && m[..i].iter().all(|&b| b == 0xFF)
                    {
                        return false;
                    }
                    // m < n - 1 confirmed; just check m > 1
                    if m_gt_one {
                        return true;
                    }

                    // Since we know m < n - 1 here, we need to check if m > 1
                    // return true if 1 < m < n - 1, false if m <= 1
                    return m[1..i].iter().rev().any(|&byte| byte > 0) || m[0] > 1;
                }
                Ordering::Less => {
                    // if m > n - 1, then m < n - 1 is false
                    return false;
                }
                Ordering::Equal => {}
            }
        }

        // The non least significant bytes are equal, so we need to
        // check the least significant byte if m > 1 and m < n - 1
        (m_gt_one || m[0] > 1) && n[0] > m[0] && n[0] - m[0] > 1
    }

    pub(super) fn decode_oaep_kek_inner(
        &self,
        unwrapped_data: &[u8],
        padding: DdiRsaCryptoPadding,
        hash_alg: DdiHashAlgorithm,
    ) -> HsmResult<SecureByteVec> {
        if DdiRsaCryptoPadding::Oaep != padding {
            return Err(HsmErr::InvalidArgument);
        }

        let oaep_hash_alg = match hash_alg {
            DdiHashAlgorithm::Sha1 => HashAlgorithm::Sha1,
            DdiHashAlgorithm::Sha256 => HashAlgorithm::Sha256,
            DdiHashAlgorithm::Sha384 => HashAlgorithm::Sha384,
            DdiHashAlgorithm::Sha512 => HashAlgorithm::Sha512,
            _ => {
                return Err(HsmErr::InvalidArgument);
            }
        };

        self.state
            .env()
            .sha()
            .decode_oaep_kek(unwrapped_data, oaep_hash_alg)
            .map_err(|_| HsmErr::RsaUnwrapOaepDecodeFailed)
    }
}

/// RSA Modular Exponentiation Output.
#[allow(unused)]
pub(crate) struct RsaModExpOutput {
    /// RSA size
    pub size: RsaSize,

    /// RSA data
    pub data: SecureByteArray<{ RsaSize::MAX_LEN }>,
}

/// RSA Modular Exponentiation Command Info.
pub(crate) struct RsaModExp<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// RSA CRT operation or not.
    pub(crate) is_crt: bool,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: PkaRsaCmd,
}

/// RSA Pairwise Consistency Test (PCT) Command Info.
pub(crate) struct RsaPctValidationCmd<E: HsmEnvTrait + 'static> {
    /// Tag identifier
    pub(crate) tag: TagId,

    /// Key ID being validated.
    pub(crate) key_id: KeyId,

    /// RSA key size used for the operation (2k, 3k, or 4k).
    pub(crate) rsa_type: PkaRsaSize,

    /// The intended key usage (sign/verify, encrypt/decrypt, unwrap).
    pub(crate) usage: RsaKeyUsage,

    /// DMA buffer used for zero-copy input/output during modular exponentiation.
    pub(crate) dma_buf: DmaBuffer<E>,

    /// RSA modular exponentiation operation (sign or decrypt).
    pub(crate) rsa_op: RsaModExp<E>,

    /// List of states in the RSA PCT Validation command state
    pub(crate) state: RsaPctValidationState,
}

/// List of states in the RSA PCT Validation command state
#[derive(Clone, PartialEq)]
pub(crate) enum RsaPctValidationState {
    /// Waiting for RSA sign operation to complete
    WaitForSign,

    /// Waiting for RSA verify operation to complete
    WaitForVerify,

    /// Waiting for RSA encrypt operation to complete
    WaitForEncrypt,

    /// Waiting for RSA decrypt operation to complete
    WaitForDecrypt,

    /// Validation completed successfully
    ValidationComplete,
}

/// Reverse copy the data from input slice into destination slice.
pub(crate) fn rsa_reverse_copy_to_slice(slice: &[u8]) -> [u8; RsaSize::MAX_LEN] {
    let mut k = [0; RsaSize::MAX_LEN];
    reverse_copy(&mut k[..slice.len()], slice);
    k
}
