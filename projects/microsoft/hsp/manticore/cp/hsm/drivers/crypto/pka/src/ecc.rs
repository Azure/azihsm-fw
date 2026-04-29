// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_types::*;
use pka::PkaImpl;
use zeroize::Zeroize;

use crate::*;

/// Elliptic Curve Cryptography (ECC) curve types
#[derive(Copy, Clone, PartialEq)]
pub enum PkaEccCurve {
    /// NIST P-256 Curve
    Ecc256,

    /// NIST P-384 Curve
    Ecc384,

    /// NIST P-521 Curve
    Ecc521,
}

impl PkaEccCurve {
    /// Maximum length of ECC curve component
    pub const MAX_LEN: usize = 68;

    /// Get the length of the ECC curve component
    #[allow(clippy::len_without_is_empty)]
    pub fn len(&self) -> usize {
        usize::from(*self)
    }

    pub fn ecc_gen_key_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccKeyGenerate256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccKeyGenerate384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccKeyGenerate521,
        }
    }

    pub fn ecc_sign_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccSign256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccSign384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccSign521,
        }
    }

    pub fn ecc_verify_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccVerify256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccVerify384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccVerify521,
        }
    }

    pub fn ecc_gen_pub_key_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccPointMultiplication256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccPointMultiplication384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccPointMultiplication521,
        }
    }

    pub fn ecc_point_multiplication_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccPointMultiplication256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccPointMultiplication384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccPointMultiplication521,
        }
    }

    pub fn ecc_mont_const_calc_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::MontgomeryConstCalc256,
            PkaEccCurve::Ecc384 => PkaCommandCode::MontgomeryConstCalc384,
            PkaEccCurve::Ecc521 => PkaCommandCode::MontgomeryConstCalc521,
        }
    }

    pub fn ecc_mod_reduction_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::PkaModReduction256,
            PkaEccCurve::Ecc384 => PkaCommandCode::PkaModReduction384,
            PkaEccCurve::Ecc521 => PkaCommandCode::PkaModReduction521,
        }
    }

    pub fn ecc_mont_representation_in_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::PkaMontgomeryReprIn256,
            PkaEccCurve::Ecc384 => PkaCommandCode::PkaMontgomeryReprIn384,
            PkaEccCurve::Ecc521 => PkaCommandCode::PkaMontgomeryReprIn521,
        }
    }

    pub fn ecc_mont_representation_out_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::PkaMontgomeryReprOut256,
            PkaEccCurve::Ecc384 => PkaCommandCode::PkaMontgomeryReprOut384,
            PkaEccCurve::Ecc521 => PkaCommandCode::PkaMontgomeryReprOut521,
        }
    }

    pub fn ecc_mod_inverse_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::PkaModInverse256,
            PkaEccCurve::Ecc384 => PkaCommandCode::PkaModInverse384,
            PkaEccCurve::Ecc521 => PkaCommandCode::PkaModInverse521,
        }
    }

    pub fn ecc_mod_multiplication_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::ModMultiplication256,
            PkaEccCurve::Ecc384 => PkaCommandCode::ModMultiplication384,
            PkaEccCurve::Ecc521 => PkaCommandCode::ModMultiplication521,
        }
    }

    pub fn ecc_mod_addition_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::PkaModAddition256,
            PkaEccCurve::Ecc384 => PkaCommandCode::PkaModAddition384,
            PkaEccCurve::Ecc521 => PkaCommandCode::PkaModAddition521,
        }
    }

    pub fn montgomery_size(&self) -> usize {
        match self {
            PkaEccCurve::Ecc256 => 36,
            PkaEccCurve::Ecc384 => 52,
            PkaEccCurve::Ecc521 => 72,
        }
    }

    pub fn get_prime(curve: PkaEccCurve) -> &'static PkaEccPrime {
        match curve {
            PkaEccCurve::Ecc256 => &ecc_constants::PRIME256,
            PkaEccCurve::Ecc384 => &ecc_constants::PRIME384,
            PkaEccCurve::Ecc521 => &ecc_constants::PRIME521,
        }
    }

    pub fn get_base(curve: PkaEccCurve) -> &'static PkaEccBase {
        match curve {
            PkaEccCurve::Ecc256 => &ecc_constants::BASE256,
            PkaEccCurve::Ecc384 => &ecc_constants::BASE384,
            PkaEccCurve::Ecc521 => &ecc_constants::BASE521,
        }
    }

    pub fn get_order(curve: PkaEccCurve) -> &'static PkaEccOrder {
        match curve {
            PkaEccCurve::Ecc256 => &ecc_constants::ORDER256,
            PkaEccCurve::Ecc384 => &ecc_constants::ORDER384,
            PkaEccCurve::Ecc521 => &ecc_constants::ORDER521,
        }
    }

    pub fn ecc_point_validation_opcode(&self) -> PkaCommandCode {
        match self {
            PkaEccCurve::Ecc256 => PkaCommandCode::EccPointValidation256,
            PkaEccCurve::Ecc384 => PkaCommandCode::EccPointValidation384,
            PkaEccCurve::Ecc521 => PkaCommandCode::EccPointValidation521,
        }
    }
}

impl From<PkaEccCurve> for usize {
    /// Converts to this type from the input type.
    fn from(curve: PkaEccCurve) -> Self {
        match curve {
            PkaEccCurve::Ecc256 => 32,
            PkaEccCurve::Ecc384 => 48,
            PkaEccCurve::Ecc521 => 68,
        }
    }
}

/// Elliptic Curve Cryptography (ECC) Private Key
#[repr(C)]
pub struct PkaEccPrivateKey {
    /// ECC Private Key
    pub k: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

impl Drop for PkaEccPrivateKey {
    fn drop(&mut self) {
        self.k.zeroize();
    }
}

impl PkaEccPrivateKey {
    /// Get the ECC curve
    pub fn curve(&self) -> PkaEccCurve {
        self.curve
    }

    /// Get the ECC private key
    pub fn k(&self) -> &[u8] {
        &self.k[..self.curve.len()]
    }

    /// Get the ECC private key
    pub fn k_mut(&mut self) -> &mut [u8] {
        &mut self.k[..self.curve.len()]
    }

    /// Get the total data length in bytes
    pub fn data_len(curve: PkaEccCurve) -> usize {
        curve.len()
    }

    pub fn from_bytes(curve: PkaEccCurve, buf: &[u8]) -> McrResult<PkaEccPrivateKey> {
        if buf.len() != PkaEccPrivateKey::data_len(curve) {
            Err(PkaErr::InvalidArg)?
        }

        let private_key = PkaEccPrivateKey {
            curve,
            k: extract_from_slice(buf),
        };

        Ok(private_key)
    }
}

/// Elliptic Curve Cryptography (ECC) Public Key
#[repr(C)]
pub struct PkaEccPublicKey {
    /// ECC Public Key X and Y component in PKA format
    pub data: [u8; PkaEccCurve::MAX_LEN * 2],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

// Implement Drop for PkaEccPublicKey
impl Drop for PkaEccPublicKey {
    fn drop(&mut self) {
        self.data.zeroize();
    }
}

impl PkaEccPublicKey {
    /// Get the ECC curve
    pub fn curve(&self) -> PkaEccCurve {
        self.curve
    }

    /// Get the ECC public key X component
    pub fn x(&self) -> &[u8] {
        &self.data[..self.curve.len()]
    }

    /// Get the ECC public key Y component
    pub fn y(&self) -> &[u8] {
        &self.data[self.curve.len()..self.curve.len() * 2]
    }

    /// Get the ECC public key X component
    pub fn x_mut(&mut self) -> &mut [u8] {
        &mut self.data[..self.curve.len()]
    }

    /// Get the ECC public key Y component
    pub fn y_mut(&mut self) -> &mut [u8] {
        &mut self.data[self.curve.len()..self.curve.len() * 2]
    }

    /// Get the total data length in bytes
    pub fn data_len(curve: PkaEccCurve) -> usize {
        2 * curve.len()
    }

    pub fn from_bytes(curve: PkaEccCurve, buf: &[u8]) -> McrResult<PkaEccPublicKey> {
        if buf.len() != PkaEccPublicKey::data_len(curve) {
            Err(PkaErr::InvalidArg)?
        }

        let pubkey = PkaEccPublicKey {
            curve,
            data: {
                let mut data = [0; PkaEccCurve::MAX_LEN * 2];
                data[..buf.len()].copy_from_slice(buf);
                data
            },
        };

        Ok(pubkey)
    }
}

/// Elliptic Curve Cryptography (ECC) Key Pair
///
#[repr(C)]
pub struct PkaEccKeyPair {
    /// ECC Private Key
    pub priv_key: PkaEccPrivateKey,

    /// ECC Public Key
    pub pub_key: PkaEccPublicKey,
}

/// Elliptic Curve Cryptography (ECC) Signature
#[repr(C)]
pub struct PkaEccSignature {
    /// ECC Signature R component
    pub r: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Signature S component
    pub s: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

impl PkaEccSignature {
    /// Get the ECC curve
    pub fn curve(&self) -> PkaEccCurve {
        self.curve
    }

    /// Get the ECC signature R component
    pub fn r(&self) -> &[u8] {
        &self.r[..self.curve.len()]
    }

    /// Get the ECC signature S component
    pub fn s(&self) -> &[u8] {
        &self.s[..self.curve.len()]
    }

    /// Get the ECC signature R component
    pub fn r_mut(&mut self) -> &mut [u8] {
        &mut self.r[..self.curve.len()]
    }

    /// Get the ECC signature S component
    pub fn s_mut(&mut self) -> &mut [u8] {
        &mut self.s[..self.curve.len()]
    }

    /// Get the total data length in bytes
    pub fn data_len(curve: PkaEccCurve) -> usize {
        2 * curve.len()
    }

    pub fn from_bytes(curve: PkaEccCurve, buf: &[u8]) -> McrResult<PkaEccSignature> {
        if buf.len() != PkaEccSignature::data_len(curve) {
            Err(PkaErr::InvalidArg)?
        }

        let signature = PkaEccSignature {
            curve,
            r: extract_from_slice(&buf[..curve.len()]),
            s: extract_from_slice(&buf[curve.len()..]),
        };

        Ok(signature)
    }
}

#[repr(C)]
pub struct PkaEccDigest {
    /// The digest value for the ECC operation.
    pub digest_be: [u8; PkaEccCurve::MAX_LEN],

    /// Digest length
    pub len: usize,
}

// Implement Drop for PkaEccDigest
impl Drop for PkaEccDigest {
    fn drop(&mut self) {
        self.digest_be.zeroize();
    }
}

impl PkaEccDigest {
    /// Get the ECC digest in big-endian format
    pub fn digest_be(&self) -> &[u8] {
        &self.digest_be[..self.len]
    }

    /// Get the ECC digest in big-endian format
    pub fn digest_be_mut(&mut self) -> &mut [u8] {
        &mut self.digest_be[..self.len]
    }
}

#[repr(C)]
pub struct PkaEccBase {
    /// X coordinate for the ECC public key.
    pub x: [u8; PkaEccCurve::MAX_LEN],

    /// Y coordinate for the ECC public key.
    pub y: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

#[repr(C)]
pub struct PkaEccPrime {
    /// X coordinate for the ECC public key.
    pub p: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

#[repr(C)]
pub struct PkaEccOrder {
    /// X coordinate for the ECC public key.
    pub n: [u8; PkaEccCurve::MAX_LEN],

    /// ECC Curve
    pub curve: PkaEccCurve,
}

#[repr(C)]
pub(crate) struct PkaEccKatTestVectors {
    /// ECC Curve
    pub(crate) curve: PkaEccCurve,

    /// ECC digest
    pub(crate) digest: [u8; PkaEccCurve::MAX_LEN],

    /// K
    pub(crate) k: [u8; PkaEccCurve::MAX_LEN],

    /// ECC private key
    pub(crate) private_key: [u8; PkaEccCurve::MAX_LEN],

    /// ECC signature R component
    pub(crate) r: [u8; PkaEccCurve::MAX_LEN],

    /// ECC signature S component
    pub(crate) s: [u8; PkaEccCurve::MAX_LEN],
}

#[repr(C)]
pub struct PkaEccVerifyResult {
    /// Indicator of a successful verify operation. 0 - success, 1 - verification failed.
    pub not_valid: u32,
}

impl From<PkaEccVerifyResult> for bool {
    fn from(value: PkaEccVerifyResult) -> Self {
        value.not_valid == 0
    }
}

#[derive(Copy, Clone)]
pub struct PkaEccCmd {
    pub curve: PkaEccCurve,
}

/// Elliptic Curve Cryptography (ECC) Secret value
#[repr(C)]
pub struct PkaEccSecretValue {
    /// ECC Curve
    pub curve: PkaEccCurve,

    /// ECC secret value in little endian format.
    pub secret: [u8; PkaEccCurve::MAX_LEN],
}

impl PkaEccSecretValue {
    /// Get the ECC secret value in little-endian format.
    pub fn secret(&self) -> &[u8] {
        &self.secret[..self.curve.len()]
    }

    /// Get the total data length in bytes
    pub fn data_len(curve: PkaEccCurve) -> usize {
        curve.len()
    }

    /// Build the structure from a byte blob.
    pub fn from_bytes(curve: PkaEccCurve, buf: &[u8]) -> McrResult<PkaEccSecretValue> {
        if buf.len() != curve.len() {
            Err(PkaErr::InvalidArg)?
        }

        // Ecc521 is padded with two 0 bytes, keep them at the end
        let reverse_copy_len = match curve {
            PkaEccCurve::Ecc256 => curve.len(),
            PkaEccCurve::Ecc384 => curve.len(),
            PkaEccCurve::Ecc521 => 66,
        };

        let secret = PkaEccSecretValue {
            curve,
            secret: reverse_extract_from_slice(&buf[..reverse_copy_len]),
        };

        Ok(secret)
    }
}

impl Drop for PkaEccSecretValue {
    fn drop(&mut self) {
        self.secret.zeroize();
    }
}

/// ECDH Known Answer Test Vectors
/// Struct to contain the Known Answer Test data from the NIST ECCCDH Primitive Test Vectors
/// Source, ECCCDH Primitive Test Vectors: https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/component-testing
#[repr(C)]
pub(crate) struct EcdhKatTestVectors {
    /// ECC Curve
    pub(crate) curve: PkaEccCurve,

    /// ECDH QCAVSx; CAVS's Public Key x coordinate
    pub(crate) qcavs_x: [u8; PkaEccCurve::MAX_LEN],

    /// ECDH QCAVSy; CAVS's Public Key y coordinate
    pub(crate) qcavs_y: [u8; PkaEccCurve::MAX_LEN],

    /// ECDH dIUT; IUT Private Key
    pub(crate) d_iut: [u8; PkaEccCurve::MAX_LEN],

    /// ECDH ZIUT; Shared Secret
    pub(crate) z_iut: [u8; PkaEccCurve::MAX_LEN],
}

impl<I: InterruptControllerTrait> PkaImpl<I> {
    pub fn begin_ecc_gen_key(&mut self, tag: u16, curve: PkaEccCurve) -> McrResult<PkaEccCmd> {
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            0,
            0,
            0,
            curve.ecc_gen_key_opcode(),
        )?;

        Ok(PkaEccCmd { curve })
    }

    pub fn end_ecc_gen_key(&mut self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccKeyPair> {
        self.check_completion(tag)?;

        let curve = op.curve;
        let priv_key_data_len = PkaEccPrivateKey::data_len(curve);
        let pub_key_data_len = PkaEccPublicKey::data_len(curve);

        let priv_key = PkaEccPrivateKey::from_bytes(
            curve,
            &self.output[pub_key_data_len..pub_key_data_len + priv_key_data_len],
        )?;
        let pub_key = PkaEccPublicKey::from_bytes(curve, &self.output[..pub_key_data_len])?;

        // zeroize the output buffer's private + public key data
        self.output[..pub_key_data_len + priv_key_data_len].zeroize();

        Ok(PkaEccKeyPair { priv_key, pub_key })
    }

    pub fn begin_ecc_sign_zc(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
        priv_key: &[u8],
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.pka_execute_command(
            tag,
            signature.addr() as u32,
            digest.addr() as u32,
            priv_key.as_ptr() as u32,
            0,
            curve.ecc_sign_opcode(),
        )?;
        Ok(PkaEccCmd { curve })
    }

    pub fn end_ecc_sign_zc(&mut self, tag: u16) -> McrResult<()> {
        self.check_completion(tag)?;
        Ok(())
    }

    pub fn begin_ecc_gen_pub_key_zc(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        pub_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        // Calculate the base offset from ecc_const buffer.
        let mut base_addr = self.ecc_const.as_ptr() as u32;
        base_addr += self.base_offset(curve);

        self.pka_execute_command(
            tag,
            pub_key.addr() as u32,
            base_addr,
            private_key.as_ptr() as u32,
            0,
            curve.ecc_gen_pub_key_opcode(),
        )?;

        Ok(PkaEccCmd { curve })
    }

    pub fn end_ecc_gen_pub_key_zc(&mut self, tag: u16, _op: PkaEccCmd) -> McrResult<()> {
        self.check_completion(tag)
    }

    pub fn begin_ecdh_compute_zc(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        public_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            public_key.addr() as u32,
            private_key.as_ptr() as u32,
            0,
            curve.ecc_point_multiplication_opcode(),
        )?;

        Ok(PkaEccCmd { curve })
    }

    pub fn end_ecdh_compute(&mut self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccSecretValue> {
        self.check_completion(tag)?;

        let curve = op.curve;

        let secret = PkaEccSecretValue::from_bytes(
            curve,
            &self.output[0..PkaEccSecretValue::data_len(curve)],
        );
        // zeroize the output buffer's secret data
        self.output[0..PkaEccSecretValue::data_len(curve)].zeroize();

        secret
    }

    pub fn begin_montgomery_constant_calculation(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
    ) -> McrResult<()> {
        // Calculate the prime offset from ecc_const buffer.
        let mut prime_addr = self.ecc_const.as_ptr() as u32;
        prime_addr += self.prime_offset(curve);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            prime_addr,
            0,
            0,
            curve.ecc_mont_const_calc_opcode(),
        )?;

        Ok(())
    }

    pub fn end_montgomery_constant_calculation(&mut self, tag: u16) -> McrResult<()> {
        self.check_completion(tag)?;

        Ok(())
    }

    pub fn begin_ecc_verify_zc(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<()> {
        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            digest.addr() as u32,
            public_key.addr() as u32,
            signature.addr() as u32,
            curve.ecc_verify_opcode(),
        )
    }

    pub fn end_ecc_verify_zc(&mut self, tag: u16) -> McrResult<bool> {
        self.check_completion(tag)?;

        Ok(PkaEccVerifyResult {
            not_valid: (self.output[0] & 1) as u32,
        }
        .into())
    }

    pub fn begin_ecc_point_validation_zc(
        &mut self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
    ) -> McrResult<()> {
        self.output.zeroize();

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            public_key.addr() as u32,
            0,
            0,
            curve.ecc_point_validation_opcode(),
        )
    }

    pub fn end_ecc_point_validation_zc(&mut self, tag: u16) -> McrResult<bool> {
        self.check_completion(tag)?;

        let result = self.output[0] == 0;

        self.output.zeroize();

        Ok(result)
    }

    fn base_offset(&self, curve: PkaEccCurve) -> u32 {
        match curve {
            PkaEccCurve::Ecc256 => PKA_BASE_PT_256_START_OFFSET as u32,
            PkaEccCurve::Ecc384 => PKA_BASE_PT_384_START_OFFSET as u32,
            PkaEccCurve::Ecc521 => PKA_BASE_PT_521_START_OFFSET as u32,
        }
    }

    pub fn prime_offset(&self, curve: PkaEccCurve) -> u32 {
        match curve {
            PkaEccCurve::Ecc256 => PKA_PRIME_256_START_OFFSET as u32,
            PkaEccCurve::Ecc384 => PKA_PRIME_384_START_OFFSET as u32,
            PkaEccCurve::Ecc521 => PKA_PRIME_521_START_OFFSET as u32,
        }
    }
}

fn extract_from_slice(slice: &[u8]) -> [u8; PkaEccCurve::MAX_LEN] {
    let mut k = [0; PkaEccCurve::MAX_LEN];
    k[..slice.len()].copy_from_slice(slice);
    k
}

fn reverse_extract_from_slice(slice: &[u8]) -> [u8; PkaEccCurve::MAX_LEN] {
    let mut k = [0; PkaEccCurve::MAX_LEN];
    reverse_copy_from_slice(&mut k[..slice.len()], slice);
    k
}
