// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

extern crate alloc;

pub mod vec {
    pub use alloc::vec::Vec;
}
/// PKA public interfaces.
mod ecc;
mod ecc_constants;
mod pka;
mod rsa;
mod rsa_constants;

use crate::vec::Vec;
pub use ecc::PkaEccBase;
pub use ecc::PkaEccCmd;
pub use ecc::PkaEccCurve;
pub use ecc::PkaEccDigest;
pub use ecc::PkaEccKeyPair;
pub use ecc::PkaEccPrime;
pub use ecc::PkaEccPrivateKey;
pub use ecc::PkaEccPublicKey;
pub use ecc::PkaEccSecretValue;
pub use ecc::PkaEccSignature;
pub use ecc::PkaEccVerifyResult;
pub use mcr_error::mcr_err_decl;
pub use mcr_error::McrResult;
use mcr_interrupt_controller::Interrupt;
use mcr_types::IoMemRange;
use mcr_types::SecureByteArray;
use open_enum::open_enum;
pub use pka::Pka;
pub use rsa::PkaRsaCmd;
pub use rsa::PkaRsaData;
pub use rsa::PkaRsaMontData;
pub use rsa::PkaRsaSize;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;
use zeroize::Zeroize;

// Offsets for PKA ECC Base and Prime constant values.
pub const PKA_BASE_PT_256_START_OFFSET: usize = 0;
pub const PKA_PRIME_256_START_OFFSET: usize = 64;

pub const PKA_BASE_PT_384_START_OFFSET: usize = 96;
pub const PKA_PRIME_384_START_OFFSET: usize = 192;

pub const PKA_BASE_PT_521_START_OFFSET: usize = 240;
pub const PKA_PRIME_521_START_OFFSET: usize = 376;

seq_macro::seq! {
    N in 0..16 {
        /// Enumeration of PKA instance identifiers
        #[repr(u8)]
        #[open_enum]
        #[derive(Clone, Copy)]
        pub enum PkaInstanceId {
            #(
                Id~N = N,
            )*
        }
    }
}

/// PKA instance iterator.
#[derive(Default)]
pub struct PkaInstanceIter {
    val: u8,
}

impl Iterator for PkaInstanceIter {
    /// The type of the elements being iterated over.
    type Item = PkaInstanceId;

    /// Advances the iterator and returns the next value.
    fn next(&mut self) -> Option<Self::Item> {
        if let Ok(pka) = self.val.try_into() {
            self.val += 1;
            Some(pka)
        } else {
            None
        }
    }
}

impl TryFrom<u8> for PkaInstanceId {
    type Error = u32;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0..=15 => Ok(unsafe { core::mem::transmute::<u8, PkaInstanceId>(value) }),
            _ => Err(u32::MAX),
        }
    }
}

impl PkaInstanceId {
    /// PKA Instance iterator
    pub fn iter() -> PkaInstanceIter {
        PkaInstanceIter::default()
    }

    /// PKA instance ID to Interrupt number tuple
    pub fn int_num_tuple(&self) -> (Interrupt, Interrupt) {
        match *self {
            PkaInstanceId::Id0 => (Interrupt::Upka0DoneIrq, Interrupt::Upka0ErrorIrq),
            PkaInstanceId::Id1 => (Interrupt::Upka1DoneIrq, Interrupt::Upka1ErrorIrq),
            PkaInstanceId::Id2 => (Interrupt::Upka2DoneIrq, Interrupt::Upka2ErrorIrq),
            PkaInstanceId::Id3 => (Interrupt::Upka3DoneIrq, Interrupt::Upka3ErrorIrq),
            PkaInstanceId::Id4 => (Interrupt::Upka4DoneIrq, Interrupt::Upka4ErrorIrq),
            PkaInstanceId::Id5 => (Interrupt::Upka5DoneIrq, Interrupt::Upka5ErrorIrq),
            PkaInstanceId::Id6 => (Interrupt::Upka6DoneIrq, Interrupt::Upka6ErrorIrq),
            PkaInstanceId::Id7 => (Interrupt::Upka7DoneIrq, Interrupt::Upka7ErrorIrq),
            PkaInstanceId::Id8 => (Interrupt::Upka8DoneIrq, Interrupt::Upka8ErrorIrq),
            PkaInstanceId::Id9 => (Interrupt::Upka9DoneIrq, Interrupt::Upka9ErrorIrq),
            PkaInstanceId::Id10 => (Interrupt::Upka10DoneIrq, Interrupt::Upka10ErrorIrq),
            PkaInstanceId::Id11 => (Interrupt::Upka11DoneIrq, Interrupt::Upka11ErrorIrq),
            PkaInstanceId::Id12 => (Interrupt::Upka12DoneIrq, Interrupt::Upka12ErrorIrq),
            PkaInstanceId::Id13 => (Interrupt::Upka13DoneIrq, Interrupt::Upka13ErrorIrq),
            PkaInstanceId::Id14 => (Interrupt::Upka14DoneIrq, Interrupt::Upka14ErrorIrq),
            PkaInstanceId::Id15 => (Interrupt::Upka15DoneIrq, Interrupt::Upka15ErrorIrq),
            _ => unreachable!(),
        }
    }
}

impl From<PkaInstanceId> for u8 {
    fn from(value: PkaInstanceId) -> Self {
        value.0 as Self
    }
}

impl From<PkaInstanceId> for usize {
    fn from(value: PkaInstanceId) -> Self {
        value.0 as Self
    }
}

pub trait PkaTrait {
    /// Peek the tag
    ///
    /// # Returns
    ///
    /// * `Option<u16>` - The tag
    fn peek_tag(&self) -> Option<u16>;

    /// Begin to generate random ECC key pair.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - The ECC curve type.
    ///
    /// # Returns
    ///
    /// * `McrResult<PkaEccGenKey>` - Ok(PkaEccGenkey) or appropriate Err() value
    fn begin_ecc_gen_key(&self, tag: u16, curve: PkaEccCurve) -> McrResult<PkaEccCmd>;

    /// Check if ECC key generation operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational data for this command.
    ///
    /// # Returns
    ///
    /// * `McrResult<PkaEccKeyPair>` - EccKeyPair data if operation completed successfully, error code otherwise.
    fn end_ecc_gen_key(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccKeyPair>;

    /// Begin to create an ECDSA signature with a private key.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - The ECC curve type.
    /// * `priva_key` - The private key to use to generate the signature.
    /// * `digest` - The digest data to sign
    /// * `signature` - The output signature
    ///
    /// # Returns
    ///
    /// * `McrResult<PkaEccSign>` - ECC sign metadata or appropriate Err() value
    fn begin_ecc_sign_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        priv_key: &[u8],
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<PkaEccCmd>;

    /// Check if ECC sign operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational data for the command.
    ///
    /// # Returns
    ///
    /// * `McrResult<usize>` - Length of signature data if operation completed successfully, error code otherwise.
    fn end_ecc_sign_zc(&self, tag: u16) -> McrResult<()>;

    /// Begin to verify an digital signature generated using ECDSA.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - The ECC curve type.
    /// * `public_key` - The public key to verify the signature with.
    /// * `digest` - The digest data to be used for signature verification.
    /// * `signature` - The signature to verify
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) or appropriate Err() value
    fn begin_ecc_verify_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<()>;

    /// Check if ECC verify operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    ///
    /// # Returns
    ///
    /// * `McrResult<bool>` - Ok(true) if the signature verification is successful, Ok(false) if the
    ///   signature verification is un-successful, Err() if there is an error.
    fn end_ecc_verify_zc(&self, tag: u16) -> McrResult<bool>;

    /// Begin a get ECC public key from ECC private key operation.
    /// PKA places output directly to pub_key memory address
    /// NOTE: montgomery_const_calc() operation should precede this operation
    /// in order to set the right PKA internal modulus.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - ECC curve type.
    /// * `private_key` - The private key data.
    /// * `pub_key` - Pointer to public key data memory
    ///
    /// # Returns
    ///
    /// * `McrResult` - command data or appropriate Err() value
    fn begin_ecc_gen_pub_key_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        pub_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd>;

    /// Check if ECC public key from private key has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Error code if operation did not complete successfully.
    fn end_ecc_gen_pub_key_zc(&self, tag: u16, op: PkaEccCmd) -> McrResult<()>;

    /// Montgomery constant calculation to set the internal modulus of the PKA HW.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - The ECC curve type.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok() or appropriate Err() value
    fn begin_montgomery_constant_calculation(&self, tag: u16, curve: PkaEccCurve) -> McrResult<()>;

    /// Check if the montgomery constant calculation operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok() or appropriate Err() value
    fn end_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()>;

    /// Begin an ECDH compute operation with pka-format key data
    /// NOTE: montgomery_const_calc() operation should precede this operation
    /// in order to set the right PKA internal modulus.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - ECC curve type.
    /// * `private_key` - The private key data in little-endian format.
    /// * `public_key` - The public key data in little-endian format.
    ///
    /// # Returns
    ///
    /// * `McrResult` - command data or appropriate Err() value
    fn begin_ecdh_compute_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        private_key: &[u8],
        public_key: &IoMemRange,
    ) -> McrResult<PkaEccCmd>;

    /// Check if the ECDH compute operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational command data.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Secret value data or appropriate Err() value
    fn end_ecdh_compute(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccSecretValue>;

    /// Begin to validate if a given public key is in the ECC P curve.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `curve` - The ECC curve type.
    /// * `public_key` - The public key to verify the signature with.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) or appropriate Err() value
    fn begin_ecc_point_validation_zc(
        &self,
        tag: u16,
        curve: PkaEccCurve,
        public_key: &IoMemRange,
    ) -> McrResult<()>;

    /// Check if ECC verify operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    ///
    /// # Returns
    ///
    /// * `McrResult<bool>` - Ok(true) if the given public key is in the ECC P curve, Ok(false)
    ///   if the not in the ECC P curve
    fn end_ecc_point_validation_zc(&self, tag: u16) -> McrResult<bool>;

    /// Begin perform a zero copy RSA operation with an RSA private key.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - The type of RSA command.
    /// * `priv_key` - The RSA private key to use.
    /// * `input` - The input data (message to encrypt/digest to sign etc.) in little-endian format.
    /// * `output` - The output of modular exponentiation in little-endian format
    ///
    /// # Returns
    ///
    /// * `McrResult<op>` - Ok(op) containing command operational data or appropriate Err() value
    fn begin_rsa_private_key_op_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        priv_key: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd>;

    /// End an zero copy RSA private key operation.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational data for the command.
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok(())) containing result data or appropriate Err() value
    fn end_rsa_private_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

    /// Begin perform a zero copy RSA operation with an RSA Public key.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - The type of RSA command.
    /// * `public_key` - The RSA public key to use.
    /// * `input` - The input data (message to encrypt/digest to sign etc.) in little-endian format.
    /// * `output` - The output of modular exponentiation in little-endian format
    ///
    /// # Returns
    ///
    /// * `McrResult<op>` - Ok(op) containing command operational data or appropriate Err() value
    fn begin_rsa_public_key_op_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        public_key: &IoMemRange,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd>;

    /// End an zero copy RSA public key operation.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational data for the command.
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok(())) containing result data or appropriate Err() value
    fn end_rsa_public_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

    /// Perform a zero copy RSA private key operation using CRT.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - RSA key type.
    /// * `crt_param1` - CRT parameter 1.
    /// * `crt_param2` - CRT parameter 2.
    /// * `input` - The input data (message to encrypt/digest to sign etc.) in little-endian format.
    /// * `output` - The output in little-endian format.
    ///
    /// # Returns
    ///
    /// * `McrResult<op>` - Ok(op) containing command operational data or appropriate Err() value
    fn begin_rsa_private_key_op_crt_zc(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        crt_param1: &[u8],
        crt_param2: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd>;

    /// End zero copy RSA private key operation using CRT.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The operational data for the command.
    ///
    /// # Returns
    ///
    /// * `McrResult<data>` - Ok(data) containing result data or appropriate Err() value
    fn end_rsa_private_key_op_crt_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

    /// Begin performing a montgomery constant operation to set the prime modulus.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag
    /// * `rsa_type ` - RSA key type
    /// * `modulus_be` - The prime modulus value in big-endian format
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok() or appropriate Err() value
    fn begin_rsa_montgomery_constant_calculation(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        modulus_be: &[u8],
    ) -> McrResult<()>;

    /// Check if the montgomery constant calculation operation has completed.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok() or appropriate Err() value
    fn end_rsa_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()>;

    /// Perform a montgomery input conversion operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - The RSA key type
    /// * `data_be` - The input data in big-endian format.
    ///
    /// # Returns
    ///
    /// * Command data on success, error code otherwise
    fn begin_rsa_montgomery_in(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd>;

    /// End a montgomery input conversion operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The command data
    ///
    /// # Returns
    ///
    /// * Ok(montgomery in format data) if operation is successful, error code otherwise
    fn end_rsa_montgomery_in(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData>;

    /// Perform a modular inverse operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - RSA key type`
    /// * `data` - The input data in big-endian format.
    ///
    /// # Returns
    ///
    /// * Operational data if successful, error code otherwise
    fn begin_rsa_modular_inverse(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd>;

    /// End a modular inverse operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - operational command data
    ///
    /// # Returns
    ///
    /// * Modular inverse data if successful, error code otherwise
    fn end_rsa_modular_inverse(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData>;

    /// Perform a montgomery output conversion operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - The RSA key type
    /// * `data` - The input data in big-endian format.
    ///
    /// # Returns
    ///
    /// * Command data on success, error code otherwise
    fn begin_rsa_montgomery_out(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd>;

    /// End a montgomery output conversion operation
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `op` - The command data
    ///
    /// # Returns
    ///
    /// * Ok(montgomery in format data) if operation is successful, error code otherwise
    fn end_rsa_montgomery_out(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaData>;

    /// Perform a modular multiplication operation.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - RSA key type.
    /// * `value1` - The first value to be multiplied in big-endian format.
    /// * `value2` - The second value to be multiplied in big-endian format.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok() or appropriate Err() value
    fn begin_rsa_modular_multiplication(
        &self,
        tag: u16,
        rsa_type: PkaRsaSize,
        value1_be: &[u8],
        value2_be: &[u8],
    ) -> McrResult<PkaRsaCmd>;

    /// End a modular multiplication operation.
    ///
    /// # Arguments
    /// * `tag` - The user identifier tag.
    /// * `rsa_type` - RSA key type.
    /// * `op` - Operational command data.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(data) or appropriate Err() value
    fn end_rsa_modular_multiplication(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData>;

    /// Self test for ECDSA operation.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(()) or appropriate Err() value
    fn ecdsa_self_test(&self) -> McrResult<()>;

    /// Self test for ECDH key exchange operation.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(()) or appropriate Err() value
    fn ecdh_self_test(&self) -> McrResult<()>;

    /// Self-test for RSA-2k modular exponentiation operation.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(()) or appropriate Err() value
    fn rsa_mod_exp_self_test(&self) -> McrResult<Vec<u8>>;

    /// Self-test for RSA-2k modular exponentiation CRT operation.
    ///
    /// # Returns
    ///
    /// * `McrResult` - Ok(()) or appropriate Err() value
    fn rsa_mod_exp_crt_self_test(&self) -> McrResult<()>;

    /// Zeroize memory for the PKA instance
    ///
    /// # Arguments
    /// * `self` - Pka instance
    fn begin_memory_wipe(&self, tag: u16) -> McrResult<()>;

    /// Complete memory zeroization for the PKA instance
    ///
    /// # Arguments
    /// * `self` - Pka instance
    fn end_memory_wipe(&self, tag: u16) -> McrResult<()>;
}

pub fn reverse_copy_from_slice(dst: &mut [u8], src: &[u8]) {
    for (item1, item2) in src.iter().rev().zip(dst.iter_mut()) {
        *item2 = *item1;
    }
}

/// PKA completion status
#[derive(PartialEq, Eq, Copy, Clone)]
pub enum PkaCompletionStatus {
    /// Indicates the engine is in the middle of executing a command
    Busy = 0x1,

    /// Indicates the engine has successfully finished the last command it accepted
    Complete = 0x2,

    /// Indicates the engine is unable to decode the command structure successfully
    ErrorCmd = 0x4,

    /// Indicates the engine encountered an error when the engine initiated a transaction on the AXI master interface
    ErrorBus = 0x8,

    /// Indicates the engine detected a fault while working on the operation
    ErrorFault = 0x10,

    /// Indicates the engine was working on a command initiated by a different master than the one who is reading the status register
    NotOwner = 0x20,

    /// Unknown HW error.
    Unknown = 0x40,
}

impl From<u8> for PkaCompletionStatus {
    fn from(value: u8) -> Self {
        match value {
            x if x == PkaCompletionStatus::Busy as u8 => PkaCompletionStatus::Busy,
            x if x == PkaCompletionStatus::Complete as u8 => PkaCompletionStatus::Complete,
            x if x == PkaCompletionStatus::ErrorCmd as u8 => PkaCompletionStatus::ErrorCmd,
            x if x == PkaCompletionStatus::ErrorBus as u8 => PkaCompletionStatus::ErrorBus,
            x if x == PkaCompletionStatus::ErrorFault as u8 => PkaCompletionStatus::ErrorFault,
            x if x == PkaCompletionStatus::NotOwner as u8 => PkaCompletionStatus::NotOwner,
            _ => PkaCompletionStatus::Unknown,
        }
    }
}

/// The completion details for a PKA command.
#[derive(Copy, Clone)]
pub struct PkaCompletionDesc {
    /// Result of the SHA transaction.
    pub status: PkaCompletionStatus,

    /// Tag to track the transaction completion
    pub tag: u16,
}

/// RSA 2048 bit operation data.
#[repr(C)]
pub struct RsaData2k {
    /// Data for the RSA operation.
    pub data: [u8; 256],
}

/// RSA 2048 bit public key operation data.
#[repr(C)]
pub struct RsaPublicKey2k {
    /// The exponent value of the RSA public key.
    pub e: SecureByteArray<4>,

    /// The modulus value of the RSA public key.
    pub n: SecureByteArray<256>,
}

/// RSA 2048 bit private key extension.
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct RsaPrivateKeyExt {
    /// The exponent value of the RSA public key.
    pub e: [u8; 4],
}

/// RSA 2048 bit private key operation data.
#[repr(C)]
pub struct RsaPrivateKey2k {
    /// The exponent value of the RSA private key.
    pub d: SecureByteArray<256>,

    /// The modulus value of the RSA private key.
    pub n: SecureByteArray<256>,

    /// Private key extension data.
    pub ext: RsaPrivateKeyExt,
}

/// Drop implementation for RsaPrivateKey2k.
impl Drop for RsaPrivateKey2k {
    fn drop(&mut self) {
        // Clear the private key data
        self.ext.e.zeroize();
    }
}

/// RSA 2048 bit CRT parameter 1.
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct Rsa2kCrtParam1 {
    /// Data containing the following internal memory structure (in the same order):
    /// p (Prime p, half operand)
    /// q (Prime q, half operand)
    /// dp (dP = d (mod p-1))
    /// dq (dQ = d (mod q-1))
    ///    Byte 0                                           Byte 511
    ///   -------------------------------------------------------------------
    ///  |               |               |                 |                 |
    ///  | p[0]...p[127] | q[0]...q[127] | dp[0]...dp[127] | dq[0]...dq[127] |
    ///  |               |               |                 |                 |
    ///   -------------------------------------------------------------------
    pub p: [u8; 128],
    pub q: [u8; 128],
    pub dp: [u8; 128],
    pub dq: [u8; 128],
}

/// RSA 2048 bit CRT parameter 2.
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct Rsa2kCrtParam2 {
    /// Data containing the following internal memory structure (in the same order):
    /// n (Modulus)
    /// n1q ((q-1 (mod p)) * q)
    /// n2p ((p-1 (mod q)) * p)
    ///   Byte 0                              Byte 767
    ///   -------------------------------------------------------
    ///  |               |                   |                   |
    ///  | n[0]...n[255] | n1q[0]...n1q[255] | n2p[0]...n2p[255] |
    ///  |               |                   |                   |
    ///   -------------------------------------------------------
    pub n: [u8; 256],
    pub n1q: [u8; 256],
    pub n2p: [u8; 256],
    pub ext: RsaPrivateKeyExt,
}

/// RSA 2048 bit CRT parameters container structure.
#[repr(C)]
pub struct Rsa2kCrtParams {
    /// Structure containing data pertaining to CRT parameter 1.
    pub param1: Rsa2kCrtParam1,

    /// Structure containing data pertaining to CRT parameter 2.
    pub param2: Rsa2kCrtParam2,
}

/// RSA 3072 bit operation data.
#[repr(C)]
pub struct RsaData3k {
    /// Data for the RSA operation.
    pub data: [u8; 384],
}

/// RSA 3072 bit public key operation data.
#[repr(C)]
pub struct RsaPublicKey3k {
    /// The exponent value of the RSA public key.
    pub e: SecureByteArray<4>,

    /// The modulus value of the RSA public key.
    pub n: SecureByteArray<384>,
}

/// RSA 3072 bit private key operation data.
#[repr(C)]
pub struct RsaPrivateKey3k {
    /// The exponent value of the RSA private key.
    pub d: SecureByteArray<384>,

    /// The modulus value of the RSA private key.
    pub n: SecureByteArray<384>,

    /// Private key extension data.
    pub ext: RsaPrivateKeyExt,
}

/// Drop implementation for RsaPrivateKey3k.
impl Drop for RsaPrivateKey3k {
    fn drop(&mut self) {
        self.ext.e.zeroize();
    }
}

/// RSA 3072 bit CRT parameter 1.
#[repr(C)]
pub struct Rsa3kCrtParam1 {
    /// Data containing the following internal memory structure (in the same order):
    /// p (Prime p, half operand)
    /// q (Prime q, half operand)
    /// dp (dP = d (mod p-1))
    /// dq (dQ = d (mod q-1))
    ///    Byte 0                                           Byte 767
    ///   -------------------------------------------------------------------
    ///  |               |               |                 |                 |
    ///  | p[0]...p[191] | q[0]...q[191] | dp[0]...dp[191] | dq[0]...dq[191] |
    ///  |               |               |                 |                 |
    ///   -------------------------------------------------------------------
    pub p: [u8; 192],
    pub q: [u8; 192],
    pub dp: [u8; 192],
    pub dq: [u8; 192],
}

/// RSA 3072 bit CRT parameter 2.
#[repr(C)]
pub struct Rsa3kCrtParam2 {
    /// Data containing the following internal memory structure (in the same order):
    /// n (Modulus)
    /// n1q ((q-1 (mod p)) * q)
    /// n2p ((p-1 (mod q)) * p)
    ///    Byte 0                              Byte 1151
    ///   -------------------------------------------------------
    ///  |               |                   |                   |
    ///  | n[0]...n[383] | n1q[0]...n1q[383] | n2p[0]...n2p[383] |
    ///  |               |                   |                   |
    ///   -------------------------------------------------------
    pub n: [u8; 384],
    pub n1q: [u8; 384],
    pub n2p: [u8; 384],
    pub ext: RsaPrivateKeyExt,
}

/// RSA 3072 bit CRT parameters container structure.
#[repr(C)]
pub struct Rsa3kCrtParams {
    /// Structure containing data pertaining to CRT parameter 1.
    pub param1: Rsa3kCrtParam1,

    /// Structure containing data pertaining to CRT parameter 2.
    pub param2: Rsa3kCrtParam2,
}

/// RSA 4096 bit operation data.
#[repr(C)]
pub struct RsaData4k {
    /// Data for the RSA operation.
    pub data: [u8; 512],
}

/// RSA 4096 bit public key operation data.
#[repr(C)]
pub struct RsaPublicKey4k {
    /// The exponent value of the RSA public key.
    pub e: SecureByteArray<4>,

    /// The modulus value of the RSA public key.
    pub n: SecureByteArray<512>,
}

/// RSA 4096 bit private key operation data.
#[repr(C)]
pub struct RsaPrivateKey4k {
    /// The exponent value of the RSA private key.
    pub d: SecureByteArray<512>,

    /// The modulus value of the RSA private key.
    pub n: SecureByteArray<512>,

    /// Private key extension data.
    ext: RsaPrivateKeyExt,
}

/// Drop implementation for RsaPrivateKey4k.
impl Drop for RsaPrivateKey4k {
    fn drop(&mut self) {
        // Clear the private key data
        self.ext.e.zeroize();
    }
}

/// RSA 4096 bit CRT parameter 1.
#[repr(C)]
pub struct Rsa4kCrtParam1 {
    /// Data containing the following internal memory structure (in the same order):
    /// p (Prime p, half operand)
    /// q (Prime q, half operand)
    /// dp (dP = d (mod p-1))
    /// dq (dQ = d (mod q-1))
    ///    Byte 0                                            Byte 1023
    ///   -------------------------------------------------------------------
    ///  |               |               |                 |                 |
    ///  | p[0]...p[255] | q[0]...q[255] | dp[0]...dp[255] | dq[0]...dq[255] |
    ///  |               |               |                 |                 |
    ///   ------------------------------------------------------------------
    pub p: [u8; 256],
    pub q: [u8; 256],
    pub dp: [u8; 256],
    pub dq: [u8; 256],
}

/// RSA 4096 bit CRT parameter 2.
#[repr(C)]
pub struct Rsa4kCrtParam2 {
    /// Data containing the following internal memory structure (in the same order):
    /// n (Modulus)
    /// n1q ((q-1 (mod p)) * q)
    /// n2p ((p-1 (mod q)) * p )
    ///    Byte 0                              Byte 1535
    ///   -------------------------------------------------------
    ///  |               |                   |                   |
    ///  | n[0]...n[511] | n1q[0]...n1q[511] | n2p[0]...n2p[511] |
    ///  |               |                   |                   |
    ///   -------------------------------------------------------
    pub n: [u8; 512],
    pub n1q: [u8; 512],
    pub n2p: [u8; 512],
    pub ext: RsaPrivateKeyExt,
}

/// RSA 4096 bit CRT parameters container structure.
#[repr(C)]
pub struct Rsa4kCrtParams {
    /// Structure containing data pertaining to CRT parameter 1.
    pub param1: Rsa4kCrtParam1,

    /// Structure containing data pertaining to CRT parameter 2.
    pub param2: Rsa4kCrtParam2,
}

/// Data required for modular inverse 1k operation.
#[repr(C)]
pub struct PkaModInverseData1k {
    /// Structure containing data to perform modular inverse on.
    pub data: [u8; 132],
}

/// Data required for modular inverse 2k operation.
#[repr(C)]
pub struct PkaModInverseData2k {
    /// Structure containing data to perform modular inverse on.
    pub data: [u8; 260],
}

/// Data required for modular inverse 3k operation.
#[repr(C)]
pub struct PkaModInverseData3k {
    /// Structure containing data to perform modular inverse on.
    pub data: [u8; 388],
}

/// Data required for modular inverse 4k operation.
#[repr(C)]
pub struct PkaModInverseData4k {
    /// Structure containing data to perform modular inverse on.
    pub data: [u8; 516],
}

/// Data required for montgomery 1k input representation.
#[repr(C)]
pub struct PkaMontgomeryInData1k {
    /// Structure containing  normal operand data to be converted to montgomery format.
    pub data: [u8; 128],
}

/// Data required for montgomery 1k output representation.
#[repr(C)]
pub struct PkaMontgomeryOutData1k {
    /// Structure containing montgomery format output data.
    pub data: [u8; 132],
}

/// Data required for montgomery 2k input representation.
#[repr(C)]
pub struct PkaMontgomeryInData2k {
    /// Structure containing  normal operand data to be converted to montgomery format.
    pub data: [u8; 256],
}

/// Data required for montgomery 2k output representation.
#[repr(C)]
pub struct PkaMontgomeryOutData2k {
    /// Structure containing montgomery format output data.
    pub data: [u8; 260],
}

/// Data required for montgomery 3k input representation.
#[repr(C)]
pub struct PkaMontgomeryInData3k {
    /// Structure containing normal operand data to be converted to montgomery format.
    pub data: [u8; 384],
}

/// Data required for montgomery 3k output representation.
#[repr(C)]
pub struct PkaMontgomeryOutData3k {
    /// Structure containing montgomery format output data.
    pub data: [u8; 388],
}

/// Data required for montgomery 4k input representation.
#[repr(C)]
pub struct PkaMontgomeryInData4k {
    /// Structure containing  normal operand data to be converted to montgomery format.
    pub data: [u8; 512],
}

/// Data required for montgomery 4k output representation.
#[repr(C)]
pub struct PkaMontgomeryOutData4k {
    /// Structure containing montgomery format output data.
    pub data: [u8; 516],
}

/// Data required for montgomery 1k constant calculation.
#[repr(C)]
pub struct PkaModPrime1k {
    /// Structure containing prime modulus data.
    pub data: [u8; 128],
}

/// Data required for montgomery 2k constant calculation.
#[repr(C)]
pub struct PkaModPrime2k {
    /// Structure containing prime modulus data.
    pub data: [u8; 256],
}

/// Data required for montgomery 3k constant calculation.
#[repr(C)]
pub struct PkaModPrime3k {
    /// Structure containing prime modulus data.
    pub data: [u8; 384],
}

/// Data required for montgomery 4k constant calculation.
#[repr(C)]
pub struct PkaModPrime4k {
    /// Structure containing prime modulus data.
    pub data: [u8; 512],
}

/// Result data for montgomery 1k constant calculation operation
#[repr(C)]
pub struct PkaMontgomeryConstCalc1kResult {
    /// Structure containing data for montgomery constant calculation.
    pub data: [u8; 128],
}

/// Result data for montgomery 2k constant calculation operation
#[repr(C)]
pub struct PkaMontgomeryConstCalc2kResult {
    /// Structure containing data for montgomery constant calculation.
    pub data: [u8; 256],
}

/// Result data for montgomery 3k constant calculation operation
#[repr(C)]
pub struct PkaMontgomeryConstCalc3kResult {
    /// Structure containing data for montgomery constant calculation.
    pub data: [u8; 384],
}

/// Result data for montgomery 4k constant calculation operation
#[repr(C)]
pub struct PkaMontgomeryConstCalc4kResult {
    /// Structure containing data for montgomery constant calculation.
    pub data: [u8; 512],
}

// Error codes for PKA Driver
mcr_err_decl! {
    Pka,
    PkaErr {
        // PKA engine is busy processing a command.
        EngineBusy = 1,

        // The result buffer address is misaligned.
        ResultAddrMisaligned = 2,

        // The arg1 buffer address is misaligned.
        Arg1AddrMisaligned = 3,

        // The arg2 buffer address is misaligned.
        Arg2AddrMisaligned = 4,

        // The arg3 buffer address is misaligned.
        Arg3AddrMisaligned = 5,

        // Invalid argument.
        InvalidArg = 6,

        // Buffer size mismatch
        BufSizeMismatch = 7,

        // PKA command code mismatch
        PkaCommandCodeMismatch = 8,

        // Tag mismatch
        TagMismatch = 9,

        // Invalid state
        InvalidState = 10,

        // Error while converting result into compatible type.
        ConversionError = 11,

        // ECC Key generation failed.
        EccKeyGenFailed = 12,

        // ECC Sign failed.
        EccSignFailed = 13,

        // ECC verify failed.
        EccVerifyFailed = 14,

        // ECC gen pub key failed.
        EccGenPubKeyFailed = 15,

        // ECC montgomery constant calculation failed.
        EccMontConstCalcFailed = 16,

        // PKA HW failure
        PkaHwCmdFail = 17,

        // PKA ECC Sign Self test failure
        PkaEccSignSelfTestFailed = 18,

        // PKA ECC Sign Self test invalid input parameter
        PkaEccSignSelfTestInvalidInputParam = 19,

        // PKA ECDH Self test failure
        PkaEcdhSelfTestFail = 20,

        // PKA RSA Self test failure
        PkaRsaSelfTestFail = 21,
    }
}
