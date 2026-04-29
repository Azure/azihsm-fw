// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use mcr_error::McrResult;
use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_types::PkaCommandCode;
use mcr_types::SecureByteVec;
use pka::PkaImpl;
use zeroize::Zeroize;

use crate::*;

/// Enumeration to denote the RSA key sizes.
#[derive(Copy, Clone)]
pub enum PkaRsaSize {
    /// RSA 1k
    Rsa1k,

    /// RSA 2k
    Rsa2k,

    /// RSA 3k
    Rsa3k,

    /// RSA 4k
    Rsa4k,
}

impl PkaRsaSize {
    /// Maximum length of RSA keys.
    pub const MAX_LEN: usize = 512;

    /// Get the RSA key length
    #[allow(clippy::len_without_is_empty)]
    pub fn len(&self) -> usize {
        usize::from(*self)
    }

    pub fn rsa_priv_key_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa2k => PkaCommandCode::RsaPrivateKeyOp2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::RsaPrivateKeyOp3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::RsaPrivateKeyOp4k,
            _ => PkaCommandCode::Unknown,
        }
    }

    pub fn rsa_pub_key_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa2k => PkaCommandCode::RsaPublicKeyOp2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::RsaPublicKeyOp3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::RsaPublicKeyOp4k,
            _ => PkaCommandCode::Unknown,
        }
    }

    pub fn rsa_crt_priv_key_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa2k => PkaCommandCode::RsaPrivateKeyOp2kCrt,
            PkaRsaSize::Rsa3k => PkaCommandCode::RsaPrivateKeyOp3kCrt,
            PkaRsaSize::Rsa4k => PkaCommandCode::RsaPrivateKeyOp4kCrt,
            _ => PkaCommandCode::Unknown,
        }
    }

    pub fn rsa_montgomery_const_calc_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa1k => PkaCommandCode::PkaMontgomeryConstCalc1k,
            PkaRsaSize::Rsa2k => PkaCommandCode::PkaMontgomeryConstCalc2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::PkaMontgomeryConstCalc3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::PkaMontgomeryConstCalc4k,
        }
    }

    pub fn rsa_montgomery_in_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa1k => PkaCommandCode::PkaMontgomeryReprIn1k,
            PkaRsaSize::Rsa2k => PkaCommandCode::PkaMontgomeryReprIn2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::PkaMontgomeryReprIn3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::PkaMontgomeryReprIn4k,
        }
    }

    pub fn rsa_montgomery_out_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa1k => PkaCommandCode::PkaMontgomeryReprOut1k,
            PkaRsaSize::Rsa2k => PkaCommandCode::PkaMontgomeryReprOut2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::PkaMontgomeryReprOut3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::PkaMontgomeryReprOut4k,
        }
    }

    pub fn rsa_modular_multiplication_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa2k => PkaCommandCode::PkaModMultiplication2k,
            PkaRsaSize::Rsa3k => PkaCommandCode::PkaModMultiplication3k,
            PkaRsaSize::Rsa4k => PkaCommandCode::PkaModMultiplication4k,
            _ => PkaCommandCode::Unknown,
        }
    }

    pub fn rsa_modular_inverse_opcode(&self) -> PkaCommandCode {
        match self {
            PkaRsaSize::Rsa1k => PkaCommandCode::PkaModInverse1k,
            PkaRsaSize::Rsa2k => PkaCommandCode::PkaModInverse2k,
            _ => PkaCommandCode::Unknown,
        }
    }

    pub fn montgomery_size(&self) -> usize {
        match self {
            PkaRsaSize::Rsa1k => 132,
            PkaRsaSize::Rsa2k => 260,
            PkaRsaSize::Rsa3k => 388,
            PkaRsaSize::Rsa4k => 516,
        }
    }
}

impl From<PkaRsaSize> for usize {
    /// Converts to this type from the input type.
    fn from(rsa_type: PkaRsaSize) -> Self {
        match rsa_type {
            PkaRsaSize::Rsa1k => 128,
            PkaRsaSize::Rsa2k => 256,
            PkaRsaSize::Rsa3k => 384,
            PkaRsaSize::Rsa4k => 512,
        }
    }
}

/// PKA RSA Command operational data.
#[derive(Copy, Clone)]
pub struct PkaRsaCmd {
    /// RSA key type
    pub rsa_type: PkaRsaSize,
}

/// RSA Encrypted/Decrypted data.
#[derive(Clone)]
#[repr(C)]
pub struct PkaRsaData {
    /// RSA size
    pub size: PkaRsaSize,

    /// RSA data
    pub data_be: SecureByteVec,
}

impl PkaRsaData {
    /// Get the RSA key length
    pub fn size(&self) -> PkaRsaSize {
        self.size
    }

    /// Get the RSA data
    pub fn data_be(&self) -> &[u8] {
        &self.data_be[..self.size.len()]
    }

    /// Construct the PkaRsaData structure data in big-endian format from raw data.
    pub fn from_bytes_be(size: PkaRsaSize, buf: &[u8]) -> McrResult<PkaRsaData> {
        if buf.len() != size.len() {
            Err(PkaErr::InvalidArg)?
        }

        let mut vec = SecureByteVec::zeroed(buf.len());
        reverse_copy_from_slice(&mut vec, buf);

        let data = PkaRsaData { size, data_be: vec };

        Ok(data)
    }
}

/// RSA Encrypted/Decrypted data.
#[derive(Clone)]
#[repr(C)]
pub struct PkaRsaMontData {
    /// RSA size
    pub size: PkaRsaSize,

    /// RSA data
    pub data_be: SecureByteVec,
}

impl PkaRsaMontData {
    pub const MAX_LEN: usize = 516;

    /// Get the RSA key length
    pub fn size(&self) -> PkaRsaSize {
        self.size
    }

    /// Get the RSA montgomery format data
    pub fn data_be(&self) -> &[u8] {
        &self.data_be[..self.size.montgomery_size()]
    }

    pub fn from_bytes_be(size: PkaRsaSize, buf: &[u8]) -> McrResult<PkaRsaMontData> {
        if buf.len() != size.montgomery_size() {
            Err(PkaErr::InvalidArg)?
        }

        let mut vec = SecureByteVec::zeroed(buf.len());
        reverse_copy_from_slice(&mut vec, buf);

        let data = PkaRsaMontData { size, data_be: vec };

        Ok(data)
    }
}

/// Struct to store Test vectors sourced from the NIST Cryptographic Algorithm Validation Program (CAVP)
pub(crate) struct Rsa2kCrtKatTestVectors {
    /// RSA CRT param1 (512 bytes for RSA-2k)
    pub(crate) param1: Rsa2kCrtParam1,

    /// RSA CRT param2 (768 bytes for RSA-2k)
    pub(crate) param2: Rsa2kCrtParam2,

    /// RSA message (256 bytes for RSA-2k)
    pub(crate) message: [u8; 256],

    /// Signature (256 bytes for RSA-2k)
    pub(crate) expected_signature: [u8; 256],
}

/// Struct to store Test vectors sourced from the NIST Cryptographic Algorithm Validation Program (CAVP)
pub(crate) struct RsaKatTestVectors {
    /// RSA modulus private key exponent (256 bytes for RSA-2k)
    pub(crate) n: [u8; 256],

    /// RSA private key exponent (256 bytes for RSA-2k)
    pub(crate) d: [u8; 256],

    /// RSA cipher-text (256 bytes for RSA-2k)
    pub(crate) c: [u8; 256],

    /// Plain text (256 bytes for RSA-2k)
    pub(crate) k: [u8; 256],
}

impl<I: InterruptControllerTrait> PkaImpl<I> {
    pub fn begin_rsa_private_key_op_zc(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        private_key: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.pka_execute_command(
            tag,
            output.addr() as u32,
            input.addr() as u32,
            private_key.as_ptr() as u32,
            0,
            rsa_type.rsa_priv_key_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_private_key_op_zc(&mut self, tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
        self.check_completion(tag)?;

        Ok(())
    }

    pub fn begin_rsa_public_key_op_zc(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        public_key: &IoMemRange,
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.pka_execute_command(
            tag,
            output.addr() as u32,
            input.addr() as u32,
            public_key.addr() as u32,
            0,
            rsa_type.rsa_pub_key_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_public_key_op_zc(&mut self, tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
        self.check_completion(tag)?;

        Ok(())
    }

    pub fn begin_rsa_private_key_op_crt_zc(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        crt_param1: &[u8],
        crt_param2: &[u8],
        input: &IoMemRange,
        output: &IoMemRange,
    ) -> McrResult<PkaRsaCmd> {
        self.pka_execute_command(
            tag,
            output.addr() as u32,
            input.addr() as u32,
            crt_param1.as_ptr() as u32,
            crt_param2.as_ptr() as u32,
            rsa_type.rsa_crt_priv_key_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_private_key_op_crt_zc(&mut self, tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
        self.check_completion(tag)?;

        Ok(())
    }

    pub fn begin_rsa_montgomery_in(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.input.zeroize();

        reverse_copy_from_slice(&mut self.input[..data_be.len()], data_be);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input.as_ptr() as u32,
            0,
            0,
            rsa_type.rsa_montgomery_in_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_montgomery_in(&mut self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
        self.check_completion(tag)?;

        let pka_rsa_mont_data = PkaRsaMontData::from_bytes_be(
            op.rsa_type,
            &self.output[..op.rsa_type.montgomery_size()],
        );
        self.output[..op.rsa_type.montgomery_size()].zeroize();

        pka_rsa_mont_data
    }

    pub fn begin_rsa_modular_inverse(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.input.zeroize();

        reverse_copy_from_slice(&mut self.input[..data_be.len()], data_be);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input.as_ptr() as u32,
            0,
            0,
            rsa_type.rsa_modular_inverse_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_modular_inverse(
        &mut self,
        tag: u16,
        op: PkaRsaCmd,
    ) -> McrResult<PkaRsaMontData> {
        self.check_completion(tag)?;

        let pka_rsa_mon_dta = PkaRsaMontData::from_bytes_be(
            op.rsa_type,
            &self.output[..op.rsa_type.montgomery_size()],
        );
        self.output[..op.rsa_type.montgomery_size()].zeroize();

        pka_rsa_mon_dta
    }

    pub fn begin_rsa_montgomery_out(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        data_be: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        self.input.zeroize();

        reverse_copy_from_slice(&mut self.input[..data_be.len()], data_be);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input.as_ptr() as u32,
            0,
            0,
            rsa_type.rsa_montgomery_out_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_montgomery_out(&mut self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaData> {
        self.check_completion(tag)?;

        let pka_rsa_data =
            PkaRsaData::from_bytes_be(op.rsa_type, &self.output[..op.rsa_type.len()]);
        self.output[..op.rsa_type.len()].zeroize();

        pka_rsa_data
    }

    pub fn begin_rsa_montgomery_constant_calculation(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        modulus_be: &[u8],
    ) -> McrResult<()> {
        self.input.zeroize();

        reverse_copy_from_slice(&mut self.input[..modulus_be.len()], modulus_be);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input.as_ptr() as u32,
            0,
            0,
            rsa_type.rsa_montgomery_const_calc_opcode(),
        )?;

        Ok(())
    }

    pub fn end_rsa_montgomery_constant_calculation(&mut self, tag: u16) -> McrResult<()> {
        self.check_completion(tag)?;

        Ok(())
    }

    pub fn begin_rsa_modular_multiplication(
        &mut self,
        tag: u16,
        rsa_type: PkaRsaSize,
        value1_be: &[u8],
        value2_be: &[u8],
    ) -> McrResult<PkaRsaCmd> {
        let val1_start: usize = 0;
        let val1_end: usize = value1_be.len();
        let val2_start: usize = val1_end;
        let val2_end: usize = val2_start + value2_be.len();

        self.input.zeroize();

        reverse_copy_from_slice(&mut self.input[val1_start..val1_end], value1_be);
        reverse_copy_from_slice(&mut self.input[val2_start..val2_end], value2_be);

        self.pka_execute_command(
            tag,
            self.output.as_ptr() as u32,
            self.input[val1_start..].as_ptr() as u32,
            self.input[val2_start..].as_ptr() as u32,
            0,
            rsa_type.rsa_modular_multiplication_opcode(),
        )?;

        Ok(PkaRsaCmd { rsa_type })
    }

    pub fn end_rsa_modular_multiplication(
        &mut self,
        tag: u16,
        op: PkaRsaCmd,
    ) -> McrResult<PkaRsaMontData> {
        self.check_completion(tag)?;

        let pka_rsa_mont_data = PkaRsaMontData::from_bytes_be(
            op.rsa_type,
            &self.output[..op.rsa_type.montgomery_size()],
        );
        self.output[..op.rsa_type.montgomery_size()].zeroize();

        pka_rsa_mont_data
    }
}
