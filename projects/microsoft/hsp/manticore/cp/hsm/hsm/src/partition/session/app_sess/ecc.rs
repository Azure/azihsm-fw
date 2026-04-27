// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::error;
use ecc_constants::*;
use mcr_crypto_pka::*;
use mcr_ddi_types::DdiEccCurve;
use mcr_logging::*;
use mcr_types::SecureByteVec;

use super::*;

/// Elliptic Curve Cryptography (ECC) curve types
#[derive(Clone, Copy, PartialEq)]
pub(crate) enum EccCurve {
    /// NIST P-256 Curve
    P256 = 32,

    /// NIST P-384 Curve
    P384 = 48,

    /// NIST P-521 Curve
    P521 = 66,
}

impl EccCurve {
    /// Maximum length of ECC curve component
    pub const MAX_LEN: usize = 66;

    /// Get the length of the ECC curve componenet
    pub fn len(&self) -> usize {
        usize::from(*self)
    }

    pub fn order_limbs(&self) -> &'static [u64] {
        match self {
            EccCurve::P256 => &P256_ORDER_U64,
            EccCurve::P384 => &P384_ORDER_U64,
            EccCurve::P521 => &P521_ORDER_U64,
        }
    }
}

impl From<PkaEccCurve> for EccCurve {
    fn from(curve: PkaEccCurve) -> Self {
        match curve {
            PkaEccCurve::Ecc256 => EccCurve::P256,
            PkaEccCurve::Ecc384 => EccCurve::P384,
            PkaEccCurve::Ecc521 => EccCurve::P521,
        }
    }
}

impl From<EccCurve> for PkaEccCurve {
    fn from(curve: EccCurve) -> Self {
        match curve {
            EccCurve::P256 => PkaEccCurve::Ecc256,
            EccCurve::P384 => PkaEccCurve::Ecc384,
            EccCurve::P521 => PkaEccCurve::Ecc521,
        }
    }
}

impl TryFrom<DdiEccCurve> for EccCurve {
    type Error = HsmErr;

    fn try_from(curve: DdiEccCurve) -> Result<Self, Self::Error> {
        match curve {
            DdiEccCurve::P256 => Ok(EccCurve::P256),
            DdiEccCurve::P384 => Ok(EccCurve::P384),
            DdiEccCurve::P521 => Ok(EccCurve::P521),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl From<EccCurve> for usize {
    /// Converts to this type from the input type.
    fn from(curve: EccCurve) -> Self {
        match curve {
            EccCurve::P256 => 32,
            EccCurve::P384 => 48,
            EccCurve::P521 => 66,
        }
    }
}

impl TryFrom<EntryKind> for EccCurve {
    type Error = HsmErr;

    fn try_from(kind: EntryKind) -> Result<Self, Self::Error> {
        match kind {
            EntryKind::Ecc256Private => Ok(EccCurve::P256),
            EntryKind::Ecc384Private => Ok(EccCurve::P384),
            EntryKind::Ecc521Private => Ok(EccCurve::P521),
            _ => Err(HsmErr::InvalidKeyType),
        }
    }
}

/// Elliptic Curve Cryptography (ECC) Public Key
#[derive(Clone, PartialEq)]
pub(crate) struct EccPubKey {
    /// ECC Curve
    pub(crate) curve: EccCurve,

    /// ECC Public Key X component
    pub(crate) x: SecureByteArray<{ EccCurve::MAX_LEN }>,

    /// ECC Public Key Y component
    pub(crate) y: SecureByteArray<{ EccCurve::MAX_LEN }>,
}

impl EccPubKey {
    /// Get the ECC curve
    pub(crate) fn curve(&self) -> EccCurve {
        self.curve
    }

    /// Get the ECC public coordinates
    ///
    /// # Returns
    /// The x and y coordinates of the ECC public key in a pair.
    pub(crate) fn coordinates(&self) -> (&[u8], &[u8]) {
        let comp_len: usize = self.curve.into();
        (
            &self.x[EccCurve::MAX_LEN - comp_len..EccCurve::MAX_LEN],
            &self.y[EccCurve::MAX_LEN - comp_len..EccCurve::MAX_LEN],
        )
    }

    /// Constructs the ECC Public Key in big-endian format from constituent x and y big endian components.
    ///
    /// # Arguments
    /// * `curve` - ECC curve Type
    /// * `x` - X component slice
    /// * `y` - Y component slice
    ///
    /// # Returns
    /// ECC public key data as EccPubKey structure.
    pub(crate) fn from_bytes_be(curve: EccCurve, x: &[u8], y: &[u8]) -> Self {
        EccPubKey {
            curve,
            x: {
                let mut k = SecureByteArray::<{ EccCurve::MAX_LEN }>::new([0u8; EccCurve::MAX_LEN]);
                k[EccCurve::MAX_LEN - x.len()..EccCurve::MAX_LEN].copy_from_slice(x);
                k
            },
            y: {
                let mut k = SecureByteArray::<{ EccCurve::MAX_LEN }>::new([0u8; EccCurve::MAX_LEN]);
                k[EccCurve::MAX_LEN - y.len()..EccCurve::MAX_LEN].copy_from_slice(y);
                k
            },
        }
    }
}

impl PkaConvertible for EccPubKey {
    type Output = Vec<u8>;

    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        let pka_curve: PkaEccCurve = self.curve().into();
        let mut buf = [0; 2 * PkaEccCurve::MAX_LEN];

        // NOTE: self.curve.len() and pka_curve.len() may be different.
        // Since we are preparing a PKA compatible buffer here, the chunk of data copied must be
        // between the range - [0..pka_curve.len()] for x and [pka_curve.len()..2*pka_curve.len()] for y.
        // self.curve.len() <= pka_curve.len(). The copy mechanism below accounts for this logic.
        reverse_copy_from_slice(
            &mut buf[..self.curve.len()],
            &self.x[self.x.len() - self.curve.len()..],
        );
        reverse_copy_from_slice(
            &mut buf[pka_curve.len()..pka_curve.len() + self.curve.len()],
            &self.y[self.y.len() - self.curve.len()..],
        );

        Ok(buf[..2 * pka_curve.len()].to_vec())
    }
}

impl PkaConvertible for PkaEccPublicKey {
    type Output = Vec<u8>;
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        Ok(self.data[..2 * self.curve.len()].to_vec())
    }
}

// Trait to convert data into PKA compatible format.
// Used by structs with underlying little-endian structure to avoid copies
pub(crate) trait PkaConvertibleZc {
    /// Expose slice of pka format data
    ///
    /// # Returns
    /// Slice of bytes containing key data if the call was successful, error code otherwise
    fn pka_as_slice(&self) -> Result<&[u8], HsmErr>;
}

impl PkaConvertibleZc for PkaEccPublicKey {
    fn pka_as_slice(&self) -> Result<&[u8], HsmErr> {
        Ok(&self.data[..2 * self.curve.len()])
    }
}

/// ECC Private Key in big-endian format.
#[derive(Clone)]
#[repr(C)]
pub struct EccPrivKey {
    /// ECC Curve
    pub(crate) curve: EccCurve,

    /// ECC Private Key
    pub(crate) k: SecureByteArray<{ EccCurve::MAX_LEN }>,
}

impl EccPrivKey {
    /// Get the ECC curve
    pub(crate) fn curve(&self) -> EccCurve {
        self.curve
    }
}
// Trait to convert data into PKA compatible format.
pub(crate) trait PkaConvertible {
    type Output;

    /// Convert the input data to PKA compatible format.
    ///
    /// # Returns
    /// Vector of bytes containing key data if the call was successful, error code otherwise.
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr>;
}

// Implement PkaConvertible for EccPrivKey. Since this is private key, we want to convert in the secure way.
impl PkaConvertible for EccPrivKey {
    type Output = SecureByteVec;
    fn to_pka_bytes(&self) -> Result<Self::Output, HsmErr> {
        let pka_curve: PkaEccCurve = self.curve().into();
        let mut buf = SecureByteVec::zeroed(pka_curve.len());

        let k_reversed = ecc_reverse_copy_to_slice(self.k.as_slice());
        let k_pka = extract_from_slice_pka(&k_reversed);

        buf.as_mut_slice()[..pka_curve.len()].copy_from_slice(&k_pka[..pka_curve.len()]);

        Ok(buf)
    }
}

/// Elliptic Curve Cryptography (ECC) Signature in big-endian format.
#[allow(dead_code)]
pub(crate) struct EccSignature {
    /// ECC Curve
    pub(crate) curve: EccCurve,

    /// ECC Signature R component
    pub(crate) r: [u8; EccCurve::MAX_LEN],

    /// ECC Signature S component
    pub(crate) s: [u8; EccCurve::MAX_LEN],
}

#[allow(dead_code)]
impl EccSignature {
    /// Get the ECC signature R component
    pub fn r(&self) -> &[u8] {
        &self.r[..self.curve.len()]
    }

    /// Get the ECC signature S component
    pub fn s(&self) -> &[u8] {
        &self.s[..self.curve.len()]
    }
}

/// Elliptic Curve Cryptography (ECC) Key Generation Command Info.
pub(crate) struct EccGenKey<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// ECC curve type.
    pub(crate) curve: EccCurve,

    /// Key tag.
    pub(crate) key_tag: Option<u16>,

    /// Key usage parameters.
    pub(crate) usage: EccKeyUsage,

    /// Key availability.
    pub(crate) availability: KeyAvailability,

    /// Command info returned by the PKA driver.
    pub(crate) cmd_info: PkaEccCmd,
}

/// Elliptic Curve Cryptography (ECC) Key Generation Out Data.
pub(crate) struct EccGenKeyOut {
    /// The ECC Key.
    pub ecc_key: EccKey,

    /// The ECC key pair.
    pub pub_key: PkaEccPublicKey,
}

/// Input key for Elliptic Curve Cryptography (ECC) operations.
pub(crate) enum EccKeyIn<'a> {
    /// Key id
    KeyId(KeyId),

    /// Key blob and the curve
    KeyBlobAndCurve(&'a [u8], EccCurve),
}

/// Elliptic Curve Cryptography (ECC) Signing Command Info.
pub(crate) struct EccSign<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// ECC curve type.
    #[allow(dead_code)]
    pub(crate) curve: EccCurve,

    /// Command information returned by the PKA driver.
    #[allow(dead_code)]
    pub(crate) cmd_info: PkaEccCmd,
}

/// Common struct to handle Structural validation state
pub(crate) struct EccStructuralValidationCmd<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    #[allow(unused)]
    pub(crate) tag: TagId,

    /// public key for validation
    pub(crate) pub_key_blob: Vec<u8>,

    /// DMA buffer allocated for the operation.
    pub(crate) dma_buf: DmaBuffer<E>,

    /// An active ECC Public Key Generation
    pub(crate) ecc_op: EccGenPubKeyCmd<E>,
}

/// Elliptic Curve Cryptography (ECC) Public Key Generation Command data.
pub(crate) struct EccGenPubKeyCmd<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Key ID of the private key.
    pub(crate) key_id: KeyId,

    /// ECC curve type.
    pub(crate) curve: EccCurve,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: PkaEccCmd,

    /// State of the ECC Gen Pub Key command.
    #[allow(unused)]
    pub(crate) state: EccPtMultiplicationState,
}

#[derive(PartialEq, Eq)]
pub(crate) enum EcdhComputeCmdState {
    /// Wait for Montgomery Constant Calculation
    MontgomeryConstCal,

    /// Wait for Point Validation
    PointValidation,

    /// WaitForEcdhCompute
    EcdhCompute,
}

/// Elliptic Curve Cryptography (ECC) ECDH Key exchange Command data.
pub(crate) struct EcdhComputeCmd<E: HsmEnvTrait + 'static> {
    /// Tag identifier.
    pub(crate) tag: TagId,

    /// Reference to the PKA engine.
    pub(crate) engine_ref: PkaEngineRef<E>,

    /// Key ID of the private key.
    pub(crate) key_id: KeyId,

    /// ECC curve type.
    pub(crate) curve: EccCurve,

    /// Command information returned by the PKA driver.
    pub(crate) cmd_info: PkaEccCmd,

    /// State of the ECDH compute command
    pub state: EcdhComputeCmdState,
}

impl<E: HsmEnvTrait> UserSession<E> {
    pub(super) fn _begin_ecc_gen_key(
        &self,
        tag: TagId,
        key_tag: Option<u16>,
        curve: EccCurve,
        usage: EccKeyUsage,
        availability: KeyAvailability,
    ) -> HsmResult<EccGenKey<E>> {
        self.state
            .vault()
            .validate_key_params(availability, key_tag)?;

        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, None)?;

        // Submit the PKA command to the engine
        let cmd_info = engine_ref
            .deref()
            .begin_ecc_gen_key(tag, curve.into())
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        Ok(EccGenKey {
            tag,
            engine_ref,
            curve: cmd_info.curve.into(),
            key_tag,
            usage,
            availability,
            cmd_info,
        })
    }

    // End an ECC gen key operation.
    pub(super) fn _end_ecc_gen_key(&self, tag: TagId, op: EccGenKey<E>) -> HsmResult<EccGenKeyOut> {
        // Perform sanity check on the tag
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, tag)?;

        // Complete the command
        let pka_result = op
            .engine_ref
            .deref()
            .end_ecc_gen_key(tag, op.cmd_info)
            .map_err(|_| HsmErr::EccGenKeyFailed)?;

        // Create a key to import
        let key = EccKeyImported::new(op.curve.into(), op.usage, pka_result.priv_key.k())?;

        // Import the key into the vault
        let ecc_key = self.state.vault().ecc_import_key(
            self.app_vault_id(),
            self.id(),
            op.key_tag,
            true,
            &key,
            op.availability,
        )?;

        // Return the result
        Ok(EccGenKeyOut {
            ecc_key,
            pub_key: pka_result.pub_key,
        })
    }

    /// Begin ECC sign op
    pub(super) fn _begin_ecc_sign_zc(
        &self,
        tag: TagId,
        key_in: EccKeyIn,
        digest: &IoMemRange,
        digest_algo: DdiHashAlgorithm,
        signature: &IoMemRange,
    ) -> HsmResult<EccSign<E>> {
        let key_id = match key_in {
            EccKeyIn::KeyId(key_id) => Some(key_id),
            EccKeyIn::KeyBlobAndCurve(_, _) => None,
        };

        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, key_id)?;

        self._begin_ecc_sign_zc_with_engine(tag, engine_ref, key_in, digest, digest_algo, signature)
    }

    /// Begin ECC sign op
    pub(super) fn _begin_ecc_sign_zc_with_engine(
        &self,
        tag: TagId,
        engine_ref: PkaEngineRef<E>,
        key_in: EccKeyIn,
        digest: &IoMemRange,
        digest_algo: DdiHashAlgorithm,
        signature: &IoMemRange,
    ) -> HsmResult<EccSign<E>> {
        // Digest should not be empty.
        if digest.is_empty() {
            Err(HsmErr::InvalidArgument)?
        }

        // Signature should not be empty.
        if signature.is_empty() {
            Err(HsmErr::InvalidArgument)?
        }

        // Variable bindings for `key` and `key_blob` references.
        let key;
        let key_in_vault;

        // Determine the key is in the vault or memory.
        let (_key_id, key_blob, curve) = match key_in {
            EccKeyIn::KeyId(key_id) => {
                key = self.ecc_key(key_id, Some(EccKeyUsage::SignVerify))?;
                key_in_vault = key.blob()?;
                let kind = key.kind()?;
                let curve = EccCurve::try_from(kind)?;
                (Some(key_id), &key_in_vault as &[u8], curve)
            }
            EccKeyIn::KeyBlobAndCurve(blob, curve) => (None, blob, curve),
        };

        if digest.len() != PkaEccCurve::MAX_LEN {
            // PKA device requires digest array padded to component length.
            // Host side pads to maximum length, regardless of curve type.
            // Return error if digest array is not padded to maximum length:
            Err(HsmErr::InvalidArgument)?
        }

        self.validate_digest_for_fips_approval(curve, digest_algo)?;

        // Submit the PKA command to the engine
        let cmd_info = engine_ref
            .deref()
            .begin_ecc_sign_zc(tag, curve.into(), key_blob, digest, signature)
            .map_err(|_| HsmErr::EccSignFailed)?;

        Ok(EccSign {
            tag,
            engine_ref,
            curve,
            cmd_info,
        })
    }

    fn validate_digest_for_fips_approval(
        &self,
        curve: EccCurve,
        digest_algo: DdiHashAlgorithm,
    ) -> HsmResult<()> {
        if self.state.is_fips_approved() {
            // FIPS 140-3 does not allow SHA1 for ECC signatures.
            if digest_algo == DdiHashAlgorithm::Sha1 {
                Err(HsmErr::NonFipsApprovedMessageDigest)?
            }

            // If the module is FIPS approved, we allow the following combinations only.
            // curve: P256, digests supported: SHA256
            // curve: P384, digests supported: SHA256, SHA384
            // curve: P521, digests supported: SHA256, SHA384 and SHA512
            let valid = match curve {
                EccCurve::P256 => digest_algo == DdiHashAlgorithm::Sha256,
                EccCurve::P384 => matches!(
                    digest_algo,
                    DdiHashAlgorithm::Sha256 | DdiHashAlgorithm::Sha384
                ),
                EccCurve::P521 => matches!(
                    digest_algo,
                    DdiHashAlgorithm::Sha256 | DdiHashAlgorithm::Sha384 | DdiHashAlgorithm::Sha512
                ),
            };
            if !valid {
                Err(HsmErr::DigestHashMismatchWithEccCurve)?
            }
        }

        Ok(())
    }

    /// End ECC sign op
    pub(super) fn _end_ecc_sign_zc(&self, tag: TagId, op: &EccSign<E>) -> HsmResult<()> {
        // Perform sanity check on the tag
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, tag)?;

        // Complete the command
        op.engine_ref
            .deref()
            .end_ecc_sign_zc(tag)
            .map_err(|_| HsmErr::EccSignFailed)
    }

    /// Helper to execute begin_ecc_gen_pub_key operation.
    pub(super) fn _begin_ecc_gen_pub_key(
        &self,
        tag: TagId,
        key_id: KeyId,
    ) -> HsmResult<EccGenPubKeyCmd<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, Some(key_id))?;

        self._begin_ecc_gen_pub_key_with_engine(tag, engine_ref, key_id, None)
    }

    /// Helper to execute begin_ecc_gen_pub_key operation.
    pub(super) fn _begin_ecc_gen_pub_key_with_engine(
        &self,
        tag: TagId,
        engine_ref: PkaEngineRef<E>,
        key_id: KeyId,
        key_usage: Option<EccKeyUsage>,
    ) -> HsmResult<EccGenPubKeyCmd<E>> {
        // Get the required key from the key vault
        let key = self.ecc_key(key_id, key_usage)?;
        let kind = key.kind()?;
        let curve: EccCurve = EccCurve::try_from(kind)?;
        let pka_curve: PkaEccCurve = curve.into();

        // Begin montgomery constant calculation command in PKA HW.
        engine_ref
            .deref()
            .begin_montgomery_constant_calculation(tag, curve.into())
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        Ok(EccGenPubKeyCmd {
            tag,
            engine_ref,
            key_id,
            curve,
            cmd_info: PkaEccCmd { curve: pka_curve },
            state: EccPtMultiplicationState::WaitForMontgomeryConstCalc,
        })
    }

    /// Helper to execute continue_ecc_gen_pub_key operation.
    pub(super) fn _continue_ecc_gen_pub_key_zc(
        &self,
        op: EccGenPubKeyCmd<E>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EccGenPubKeyCmd<E>> {
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, op.tag)?;

        // End montgomery constant calculation command in PKA HW.
        op.engine_ref
            .deref()
            .end_montgomery_constant_calculation(op.tag)
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        let key = self.ecc_key(op.key_id, None)?;
        let kind = key.kind()?;
        let curve: EccCurve = EccCurve::try_from(kind)?;
        let blob = key.blob()?;

        // Begin ECC Generate Public Key command in PKA HW.
        let cmd_info = op
            .engine_ref
            .deref()
            .begin_ecc_gen_pub_key_zc(op.tag, curve.into(), &blob, pub_key)
            .map_err(|_| HsmErr::EccGenPubKeyFailed)?;

        Ok(EccGenPubKeyCmd {
            tag: op.tag,
            engine_ref: op.engine_ref,
            key_id: op.key_id,
            curve: op.curve,
            cmd_info,
            state: EccPtMultiplicationState::WaitForPointMultiplication,
        })
    }

    /// Helper to execute end_ecc_gen_pub_key operation.
    pub(super) fn _end_ecc_gen_pub_key_zc(&self, op: EccGenPubKeyCmd<E>) -> HsmResult<()> {
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, op.tag)?;

        // End ECC Gen Pub Key command in PKA HW.
        op.engine_ref
            .deref()
            .end_ecc_gen_pub_key_zc(op.tag, op.cmd_info)
            .map_err(|_| HsmErr::EccGenPubKeyFailed)
    }

    /// Helper to execute begin_ecdh_compute operation.
    pub(super) fn begin_ecdh_compute_inner(
        &self,
        tag: TagId,
        key_id: KeyId,
        target_key_type: DdiKeyType,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, Some(key_id))?;

        // Get the required key from the key vault
        let key = self.ecc_key(key_id, Some(EccKeyUsage::KeyAgreement))?;
        let kind = key.kind()?;
        let curve: EccCurve = EccCurve::try_from(kind)?;
        let pka_curve: PkaEccCurve = curve.into();

        // Verify the key_type.
        match (target_key_type, curve) {
            (DdiKeyType::Secret256, EccCurve::P256) => (),
            (DdiKeyType::Secret384, EccCurve::P384) => (),
            (DdiKeyType::Secret521, EccCurve::P521) => (),
            (_, _) => Err(HsmErr::InvalidKeyType)?,
        };

        let (curve_len, pka_curve_len) = (curve.len(), pka_curve.len());
        let pub_key_slice = pub_key.slice();

        if pub_key_slice.len() < pka_curve_len * 2 {
            Err(HsmErr::EccDerKeyShorterThanCurve)?
        }

        // PKA curve length and ECC DER curve length are equal for P256 and P384, But for P521,
        // DER curve length is 66 bytes, but PKA curve length is 68 bytes (just to make the size
        // DWORD aligned). The public key slice contains the x and y coordinates as follows:
        //
        // x = pub_key[0.. 66], which is from 0 to DER curve length and the remaining 2 bytes are 00
        // y = pub_key[68.. 68+66], remaining 2 bytes are 00
        //
        // Where as the pub_key slice is of total length PKA Curve length multiplied by 2
        let (x, y) = pub_key_slice.split_at(pka_curve_len);
        if !EccPublicKeyRangeValidation::validate(&x[..curve_len], &y[..curve_len], curve) {
            Err(HsmErr::EccPublicKeyValidationFailed)?
        }

        // Begin montgomery constant calculation command in PKA HW.
        engine_ref
            .deref()
            .begin_montgomery_constant_calculation(tag, curve.into())
            .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

        Ok(EcdhComputeCmd {
            tag,
            engine_ref,
            key_id,
            curve,
            cmd_info: PkaEccCmd { curve: pka_curve },
            state: EcdhComputeCmdState::MontgomeryConstCal,
        })
    }

    /// Helper to execute continue_ecdh_compute operation with zero copy key data.
    pub(super) fn continue_ecdh_compute_zc_inner(
        &self,
        op: EcdhComputeCmd<E>,
        pub_key: &IoMemRange,
    ) -> HsmResult<EcdhComputeCmd<E>> {
        let (cmd_info, state) = match op.state {
            EcdhComputeCmdState::MontgomeryConstCal => {
                // STEP #2: End Montgomery constant calculation and begin ECC point validation
                op.engine_ref
                    .deref()
                    .end_montgomery_constant_calculation(op.tag)
                    .map_err(|_| HsmErr::EccMontgomeryConstCalcFailed)?;

                op.engine_ref
                    .deref()
                    .begin_ecc_point_validation_zc(op.tag, op.curve.into(), pub_key)
                    .map_err(|_| HsmErr::BeginEccPointValidationFailed)?;

                (op.cmd_info, EcdhComputeCmdState::PointValidation)
            }
            EcdhComputeCmdState::PointValidation => {
                let result = op
                    .engine_ref
                    .deref()
                    .end_ecc_point_validation_zc(op.tag)
                    .map_err(|_| HsmErr::EndEccPointValidationFailed)?;

                // Check the result of the ECC public key validation
                // If the result is false, it means the public key validation failed.
                if !result {
                    Err(HsmErr::EccPointValidationFailed)?
                }

                // Get the private key blob.
                let key = self.ecc_key(op.key_id, None)?;
                let priv_key_blob = key.blob()?;

                // Begin ECDH compute command in PKA HW.
                let cmd_info = op
                    .engine_ref
                    .deref()
                    .begin_ecdh_compute_zc(op.tag, op.curve.into(), &priv_key_blob, pub_key)
                    .map_err(|_| HsmErr::EcdhComputeFailed)?;

                (cmd_info, EcdhComputeCmdState::EcdhCompute)
            }
            _ => Err(HsmErr::InvalidState)?,
        };

        Ok(EcdhComputeCmd {
            tag: op.tag,
            engine_ref: op.engine_ref,
            key_id: op.key_id,
            curve: op.curve,
            cmd_info,
            state,
        })
    }

    /// Helper to execute end_ecdh_compute operation.
    pub(super) fn _end_ecdh_compute(
        &self,
        op: EcdhComputeCmd<E>,
        key_usage: DdiKeyUsage,
        key_tag: Option<u16>,
        key_availability: KeyAvailability,
    ) -> HsmResult<KeyId> {
        let secret_val = op
            .engine_ref
            .deref()
            .end_ecdh_compute(op.tag, op.cmd_info)
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        // Create a key to import
        let key =
            EcdhKeyImported::new(op.curve.into(), key_usage.try_into()?, secret_val.secret())?;

        // Import the key into the vault
        let ecdh_key = self.state.vault().ecdh_import_key(
            self.app_vault_id(),
            self.id(),
            key_tag,
            true,
            &key,
            key_availability,
        )?;

        // Return the result
        Ok(ecdh_key.id())
    }

    /// Helper to execute `end_ecdh_compute` operation that retrieves the shared secret.
    pub(super) fn _end_ecdh_compute_shared_secret(
        &self,
        op: &EcdhComputeCmd<E>,
    ) -> HsmResult<SecureByteVec> {
        // Verify the tag.
        self.pka_engine_verify_tag(&op.engine_ref, op.tag, op.tag)?;

        let secret_val = op
            .engine_ref
            .deref()
            .end_ecdh_compute(op.tag, op.cmd_info)
            .map_err(|_| HsmErr::EcdhComputeFailed)?;

        Ok(secret_val.secret().into())
    }

    /// Get the ECC Key.
    fn ecc_key(&self, key_id: KeyId, usage: Option<EccKeyUsage>) -> HsmResult<EccKey> {
        let key = self
            .state
            .vault()
            .ecc_key(self.app_vault_id(), self.id(), key_id, usage)?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }

    /// Get the ECDH Key.
    pub(crate) fn ecdh_key(
        &self,
        key_id: KeyId,
        usage: Option<EcdhKeyUsage>,
    ) -> HsmResult<EcdhKey> {
        let key = self
            .state
            .vault()
            .ecdh_key(self.app_vault_id(), self.id(), key_id, usage)?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }

    /// Helper to execute begin_ecc_structural_validation_inner operation.
    pub(super) fn begin_ecc_structural_validation_inner(
        &self,
        tag: TagId,
        key_id: KeyId,
        entry_usage: DdiKeyUsage,
        pub_key_blob: Vec<u8>,
    ) -> HsmResult<EccStructuralValidationCmd<E>> {
        // Acquire the PKA engine
        //
        // Note: The engine acquisition can fail with HsmErr::Pending if the
        // engine is busy.
        let engine_ref = self.pka_engine_acquire(tag, Some(key_id))?;

        let key_usage = EccKeyUsage::try_from(entry_usage)?;
        let key = self.ecc_key(key_id, Some(key_usage))?;
        let kind = key.kind()?;
        let curve: EccCurve = EccCurve::try_from(kind)?;
        let pka_curve: PkaEccCurve = curve.into();
        let priv_key_blob = key.blob()?;

        // Validate Public key blob
        if pub_key_blob.len() != 2 * pka_curve.len() {
            error!("[ecc] ECC structural validation Public key length mismatch");
            return Err(HsmErr::KeyStructuralValidationFailed);
        }

        // Validate d: 0 < d < n
        validate_scalar_d_from_pka_bytes(tag, curve, priv_key_blob.as_ref())?;

        let ecc_op =
            self._begin_ecc_gen_pub_key_with_engine(tag, engine_ref, key_id, Some(key_usage))?;

        let dma_buf = self
            .state
            .env()
            .dma_heap()
            .allocate(pub_key_blob.len())
            .ok_or(HsmErr::DmaAllocFailure)?;

        Ok(EccStructuralValidationCmd {
            tag,
            pub_key_blob,
            dma_buf,
            ecc_op,
        })
    }

    /// Helper to execute continue_ecc_structural_validation_inner operation.
    pub(super) fn continue_ecc_structural_validation_inner(
        &self,
        mut op: EccStructuralValidationCmd<E>,
    ) -> HsmResult<EccStructuralValidationCmd<E>> {
        let output = op.dma_buf.as_ref().into();

        let ecc_op = self._continue_ecc_gen_pub_key_zc(op.ecc_op, &output)?;
        op.ecc_op = ecc_op;

        Ok(op)
    }

    /// Helper to execute end_ecc_structural_validation_inner operation.
    pub(super) fn end_ecc_structural_validation_inner(
        &mut self,
        op: EccStructuralValidationCmd<E>,
    ) -> HsmResult<()> {
        // Perform hardware finalization and get actual result
        self._end_ecc_gen_pub_key_zc(op.ecc_op)?;

        let actual: &[u8] = op.dma_buf.as_ref();
        let expected = op.pub_key_blob.as_slice();

        if actual != expected {
            error!("[ecc] ECC structural validation Public key mismatch");
            Err(HsmErr::KeyStructuralValidationFailed)?
        }

        Ok(())
    }
}

/// Helper to execute `validate_scalar_d_from_pka_bytes` operation.
/// Validates that the scalar d is in the range.
fn validate_scalar_d_from_pka_bytes(
    _tag: TagId,
    curve: EccCurve,
    d_pka: &[u8],
) -> Result<(), HsmErr> {
    let pka_curve: PkaEccCurve = curve.into();
    let pka_len = pka_curve.len();
    let ecc_len = curve.len();

    if d_pka.len() != pka_len {
        error!("[ecc] ECC structural validation Private key length mismatch");
        Err(HsmErr::KeyStructuralValidationFailed)?
    }

    let is_valid = match curve {
        EccCurve::P256 => {
            let mut scalar_le = SecureByteArray::<32>::new([0u8; 32]);
            scalar_le[..ecc_len].copy_from_slice(&d_pka[..ecc_len]);
            let d_limbs = d_le_to_limbs::<4>(scalar_le.as_slice());
            is_valid_scalar(&d_limbs, curve.order_limbs())
        }
        EccCurve::P384 => {
            let mut scalar_le = SecureByteArray::<48>::new([0u8; 48]);
            scalar_le[..ecc_len].copy_from_slice(&d_pka[..ecc_len]);
            let d_limbs = d_le_to_limbs::<6>(scalar_le.as_slice());
            is_valid_scalar(&d_limbs, curve.order_limbs())
        }
        EccCurve::P521 => {
            let mut scalar_le = SecureByteArray::<66>::new([0u8; 66]);
            scalar_le[..ecc_len].copy_from_slice(&d_pka[..ecc_len]);
            let d_limbs = d_le_to_limbs::<9>(scalar_le.as_slice());
            is_valid_scalar(&d_limbs, curve.order_limbs())
        }
    };
    if !is_valid {
        error!("ECC structural validation Scalar d is not in the range 0 < d < n");
        Err(HsmErr::KeyStructuralValidationFailed)?
    }

    Ok(())
}

/// Converts a little-endian byte slice into a fixed-size array of u64 limbs.
/// Pads with zeros as needed
pub(crate) fn d_le_to_limbs<const N: usize>(input: &[u8]) -> [u64; N] {
    let mut out = [0u64; N];

    for (i, chunk) in input.chunks(8).enumerate() {
        let mut limb = 0u64;
        for (j, byte) in chunk.iter().enumerate() {
            limb |= (*byte as u64) << (8 * j);
        }
        out[i] = limb;
    }

    out
}

// The scalar `d` must satisfy: 0 < d < order, where `order` is the curve's group order.
// The limbs are in little-endian order (limb[0] = LSB), so we compare from most-significant limb to least.
// This check returns early when a difference is found.
pub fn is_valid_scalar(d: &[u64], order: &[u64]) -> bool {
    // Check if d == 0
    if d.iter().all(|&x| x == 0) {
        return false;
    }

    // Compare d < order from MSB to LSB
    for (&a, &b) in d.iter().rev().zip(order.iter().rev()) {
        match a.cmp(&b) {
            core::cmp::Ordering::Less => return true,
            core::cmp::Ordering::Greater => return false,
            core::cmp::Ordering::Equal => (),
        }
    }

    false
}

/// Reverse copy the data from input slice into destination slice.
pub(crate) fn ecc_reverse_copy_to_slice(slice: &[u8]) -> [u8; EccCurve::MAX_LEN] {
    let mut k = [0; EccCurve::MAX_LEN];
    reverse_copy(&mut k[..slice.len()], slice);
    k
}

/// Extract into a slice from a PKA HW compatible data type.
fn extract_from_slice_pka(slice: &[u8]) -> [u8; PkaEccCurve::MAX_LEN] {
    let mut k = [0; PkaEccCurve::MAX_LEN];
    k[..slice.len()].copy_from_slice(slice);
    k
}
