// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;

use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::sha_regs::RegisterBlock as ShaRegs;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_tcon::Tcon;
use mcr_types::*;
use zeroize::Zeroize;

use crate::*;

const SHA1_BLOCK_SIZE: usize = 64;
const SHA256_BLOCK_SIZE: usize = 64;
const SHA384_BLOCK_SIZE: usize = 128;
const SHA512_BLOCK_SIZE: usize = 128;
const SHA_COMMAND_ID: u32 = 3;

// The CPU TSC increments the count once every 16 ns.
// Timeout of 1 us corresponds to 62.5 counts of TSC.
// Rounding it up to 63. SHA can take 22us in worst
// case (16 KB data for SHA1) so keep it at 40us
// just in case. For 4KB worst case is 6us.
const SHA_TIMEOUT_COUNT: u64 = 63 * 40;

// KBKDF self test contstnats
const KBKDF_512_DATA_LEN: usize = 60;
const KBKDF_512_KEY_DATA_LEN: usize = 64;

// KI, Known key input array
static mut KBKDF_SELF_TEST_512_KI: [u8; KBKDF_512_KEY_DATA_LEN] = [
    0xf5, 0x56, 0x7a, 0x2d, 0xd9, 0x23, 0x6a, 0x99, 0x20, 0x0c, 0x4b, 0xd5, 0x39, 0x07, 0x43, 0xe2,
    0x56, 0x0b, 0xab, 0x4b, 0x19, 0x6e, 0x3c, 0x73, 0x2b, 0x01, 0xab, 0xf9, 0x00, 0xc7, 0x64, 0x9c,
    0xab, 0x5b, 0x95, 0x7d, 0xa6, 0xae, 0x8f, 0xd0, 0x25, 0x60, 0x51, 0x47, 0xb3, 0x65, 0x72, 0xc1,
    0x9f, 0x10, 0x36, 0x70, 0xb1, 0x6f, 0x6b, 0xb5, 0x7c, 0x13, 0x87, 0x54, 0x47, 0x9a, 0xd4, 0x5d,
];

// Input data
static mut KBKDF_SELF_TEST_512_FIXED_INPUT_DATA: [u8; KBKDF_512_DATA_LEN] = [
    0xa8, 0xcc, 0xd4, 0xbd, 0x36, 0xfb, 0x0e, 0xd0, 0x76, 0x5e, 0x96, 0x62, 0xf1, 0x02, 0x8d, 0x60,
    0x0b, 0xd6, 0x50, 0xe4, 0xc2, 0xcd, 0xdd, 0xf9, 0x4b, 0x27, 0xee, 0x88, 0x11, 0x20, 0xaa, 0xf7,
    0x4b, 0x72, 0x7b, 0x02, 0xf0, 0x03, 0x6b, 0x46, 0x16, 0x20, 0x62, 0xe3, 0x9e, 0xd4, 0x3f, 0xa8,
    0x56, 0x87, 0xa5, 0x8d, 0x17, 0x7a, 0xf6, 0xf5, 0x66, 0x81, 0x18, 0x89,
];

// KO, Known Output array
static mut KBKDF_SELF_TEST_512_KO: [u8; 40] = [
    0x2e, 0x55, 0xb7, 0x3d, 0x12, 0x6d, 0xb0, 0xf9, 0x28, 0x10, 0x26, 0x6c, 0x92, 0xe4, 0xdc, 0x7a,
    0x7f, 0x2d, 0x32, 0xcb, 0xed, 0x9e, 0xb4, 0xed, 0xab, 0x51, 0x9e, 0x5c, 0xc9, 0x13, 0x8c, 0x64,
    0x2f, 0xd4, 0xb2, 0x29, 0x78, 0x0c, 0x17, 0xbf,
];

// HKDF self test constants
const L_BYTES: usize = 32;
const PRK_DATA_LEN: usize = 32;

// Salt - 64 bytes
static mut HKDF_SELF_TEST_256_SALT: [u8; 64] = [
    0xB1, 0xD6, 0x09, 0xD7, 0x5E, 0x4D, 0x0B, 0xE8, 0x1D, 0xCA, 0xC2, 0xA8, 0x9A, 0xBC, 0xF0, 0xA8,
    0xAC, 0xD7, 0x7D, 0xB8, 0xB7, 0x64, 0x17, 0x87, 0x21, 0xE6, 0xBE, 0x68, 0xCC, 0xF5, 0x42, 0x51,
    0x38, 0xA0, 0x56, 0xA2, 0xAE, 0x37, 0x1A, 0x1E, 0x72, 0x7C, 0x15, 0xB5, 0x02, 0x6D, 0xCF, 0x8E,
    0xE7, 0x6D, 0x42, 0x08, 0x0E, 0x9C, 0x98, 0x08, 0x78, 0xB7, 0x0E, 0x78, 0x6C, 0x7D, 0x64, 0x32,
];

// IKM (z value) - 32 bytes
static mut HKDF_SELF_TEST_256_IKM: [u8; 32] = [
    0x7E, 0x4A, 0x88, 0x71, 0x5D, 0xE4, 0xD8, 0x78, 0x8F, 0x8F, 0x06, 0x9A, 0xB1, 0x05, 0x8A, 0x42,
    0xA1, 0xBA, 0xC4, 0xB1, 0x9E, 0x7A, 0x4C, 0xE8, 0xCF, 0x7E, 0xA0, 0x4C, 0xC6, 0xBD, 0x64, 0xF7,
];

// Info (concatenation of PartyU ID, PartyV ID, and length) - 36 bytes
static mut HKDF_SELF_TEST_256_INFO: [u8; 36] = [
    // PartyU ID - 16 bytes
    0x23, 0x6B, 0x2B, 0x47, 0x5C, 0xB1, 0xED, 0x72, 0x08, 0x5F, 0x11, 0xC0, 0x53, 0xBF, 0xB2,
    0x40, // PartyV ID - 16 bytes
    0x01, 0x5F, 0x5D, 0x21, 0xBB, 0x74, 0x4D, 0x2D, 0xD4, 0xF0, 0x17, 0x24, 0x03, 0x4D, 0x1B,
    0xAB, // Length (256 bits = 0x0100) - 4 bytes
    0x00, 0x00, 0x01, 0x00,
];

// DKM (Derived Keying Material) - 32 bytes
static mut HKDF_SELF_TEST_256_DKM: [u8; L_BYTES] = [
    0x05, 0x69, 0xC2, 0x57, 0x2A, 0x11, 0x3D, 0x89, 0x70, 0x57, 0xDD, 0xAC, 0xCC, 0xF1, 0x19, 0xD9,
    0x83, 0x7D, 0x7A, 0x5A, 0xCD, 0x14, 0xAA, 0xCB, 0xF7, 0xBB, 0xF7, 0x5D, 0x42, 0xBD, 0x5C, 0x1D,
];

// OAEP KEK self test constants
static mut OAEP_KEK_SELF_TEST: [u8; 16] = [
    0xf0, 0x81, 0x50, 0x2a, 0xe5, 0x33, 0x54, 0x59, 0x95, 0xdf, 0x87, 0xc4, 0x2a, 0x19, 0x20, 0x21,
];

/// SHA read message mode
pub(crate) enum ShaReadMessageModeCtrl {
    /// Incr message mode
    IncrMessageMode = 0,
}

impl From<ShaReadMessageModeCtrl> for u32 {
    /// Converts to this type from the input type.
    fn from(value: ShaReadMessageModeCtrl) -> Self {
        value as Self
    }
}

/// SHA pass message mode
pub(crate) enum ShaPassMessageModeCtrl {
    /// No pass message.
    NoPassMessage = 0,
}

impl From<ShaPassMessageModeCtrl> for u32 {
    /// Converts to this type from the input type.
    fn from(value: ShaPassMessageModeCtrl) -> Self {
        value as Self
    }
}

/// Output structure for MGF1 mask generation
#[derive(Clone)]
pub struct Mgf1Output {
    /// Generated mask data
    pub mask: [u8; OAEP_MGF1_MAX_T_BUFFER_SIZE],

    /// Actual length of the mask
    pub mask_len: u32,
}

/// Sha Instance. This is not thread safe.
#[derive(Clone)]
pub struct Sha {
    rimpl: Rc<RefCell<ShaImpl>>,
}

impl Sha {
    /// Create an instance of `Sha`
    ///
    /// # Returns
    ///
    /// * `Sha` - An instance of Sha
    pub fn new(
        cmd_buffer: &'static mut ShaCommandDesc,
        out_buffer: &'static [u8],
        self_test_buffer: &'static mut [u8],
    ) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(ShaImpl::new(
                cmd_buffer,
                out_buffer,
                self_test_buffer,
            ))),
        }
    }
}

impl ShaTrait for Sha {
    /// Compute the SHA digest.
    fn digest_zc(&self, command_info: &ShaDigestCmdInfoZc) -> McrResult<()> {
        self.rimpl.borrow_mut().digest_zc(command_info)
    }

    /// Compute HMAC
    fn hmac(
        &self,
        key: &[u8],
        data: &[u8],
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        out_buf: &mut IoMemRange,
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .hmac(key, data, sha_mode, in_buf, out_buf)
    }

    /// Compute HKDF
    fn hkdf(
        &self,
        hkdf_info: HkdfInfo,
        sha_mode: ShaMode,
        prk_buf: &mut IoMemRange,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .hkdf(hkdf_info, sha_mode, prk_buf, in_buf, output)
    }

    /// Compute KBKDF
    fn kbkdf_counter_hmac(
        &self,
        kbkdf_info: KbkdfInfo,
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .kbkdf_counter_hmac(kbkdf_info, sha_mode, in_buf, output)
    }

    /// Perform HKDF-SHA-256
    fn hkdf_self_test_256(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().hkdf_self_test_256()
    }

    /// Perform KBKDF-SHA-512 self test.
    fn kbkdf_self_test_512(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().kbkdf_self_test_512()
    }

    /// Decode OAEP KEK from unwrapped RSA data
    fn decode_oaep_kek(
        &self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<SecureByteVec> {
        self.rimpl
            .borrow_mut()
            .decode_oaep_kek(unwrapped_data, hash_alg)
    }

    /// Perform self-test for OAEP KEK decoding
    fn decode_oaep_kek_self_test(
        &self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .decode_oaep_kek_self_test(unwrapped_data, hash_alg)
    }
}

/// SHA object definition
pub struct ShaImpl {
    /// SHA command buffer
    cmd_buffer: &'static mut ShaCommandDesc,

    /// SHA output digest buffer
    out_buffer: &'static [u8],

    /// Self test buffer
    self_test_buffer: &'static mut [u8],

    /// SHA register interface
    regs: ShaRegs,
}

impl ShaImpl {
    /// Create an instance of `Sha`
    ///
    /// # Returns
    ///
    /// * `Sha` - An instance of Sha
    pub fn new(
        cmd_buffer: &'static mut ShaCommandDesc,
        out_buffer: &'static [u8],
        self_test_buffer: &'static mut [u8],
    ) -> Self {
        ShaImpl {
            cmd_buffer,
            out_buffer,
            self_test_buffer,
            regs: ShaRegs::block(),
        }
    }

    /// Compute an intermediate digest for a partial message buffer. This can either be
    /// an extended hash calculation from a previous message buffer or the start of a new message.
    /// Data is output to an output_buffer in command_info.
    ///
    /// # Arguments
    ///
    /// * `self` - The Sha object contains the required parameters for programming the HS SHA
    ///   HW interface.
    /// * `command_info` - The SHA digest command related information.
    ///
    /// # Returns
    ///
    /// * `McrResult<ShaDigest>` - Appropriate Err() value if the operation was unsuccessful.
    pub fn digest_zc(&mut self, command_info: &ShaDigestCmdInfoZc) -> McrResult<()> {
        if self.regs.sha_status().read().busy() {
            Err(ShaErr::EngineBusy)?
        }

        self.prepare_cmd_buffer_zc(command_info)?;

        cortex_m::asm::dmb();

        // Trigger the SHA hardware to start processing the command.
        self.regs
            .sha_cmd()
            .write(|_| self.cmd_buffer as *const ShaCommandDesc as u32);

        self.wait_for_completion_zc()
    }

    /// Prepare the SHA command buffer for zero copy operations
    ///
    /// # Arguments
    ///
    /// * `command_info` - The SHA digest command related information.
    ///
    /// # Returns
    ///
    /// * `Result<(), u32>` - An empty `Result` indicating success or an error code.
    fn prepare_cmd_buffer_zc(&mut self, command_info: &ShaDigestCmdInfoZc<'_>) -> Result<(), u32> {
        self.cmd_buffer.command_code = ShaCommand::default();
        if command_info.last {
            self.cmd_buffer.command_code.set_auto_pad(true);
        } else {
            // Every chunk, except the last one, needs to be aligned to the hash algorithm block size.
            if !Self::aligned(command_info.mode, command_info.len as usize) {
                Err(ShaErr::BlockAlignmentMismatch)?
            }

            self.cmd_buffer.command_code.set_byte_swap(true);
            self.cmd_buffer.command_code.set_no_truncate(true);
        }
        self.cmd_buffer
            .command_code
            .set_pass_message_mode(ShaPassMessageModeCtrl::NoPassMessage.into());
        self.cmd_buffer
            .command_code
            .set_read_message_mode(ShaReadMessageModeCtrl::IncrMessageMode.into());
        self.cmd_buffer.command_code.set_sha_cmd_id(SHA_COMMAND_ID);
        self.cmd_buffer
            .command_code
            .set_sha_mode(command_info.mode as u32);
        if let Some(init_digest) = command_info.init_digest {
            if init_digest.len() > command_info.mode.get_digest_size_hw() {
                Err(ShaErr::InvalidArgument)?
            }
            self.cmd_buffer.initial_digest = init_digest.addr() as u32;
        }
        self.cmd_buffer.byte_count = command_info.total_len;
        self.cmd_buffer.message_bytes = command_info.len;
        self.cmd_buffer.message_buffer = command_info.buffer.addr() as u32;
        self.cmd_buffer.pass_message_buffer = 0;
        self.cmd_buffer.ref_digest = 0;

        if command_info.output_buffer.len() < command_info.mode.get_digest_size_hw() {
            Err(ShaErr::InvalidArgument)?
        }
        self.cmd_buffer.digest = command_info.output_buffer.addr() as u32;

        Ok(())
    }

    /// Wait for SHA zero copy operation to complete.
    ///
    /// # Arguments
    ///
    /// `command_info` - The SHA digest command related information.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Appropriate Err() value if the operation was unsuccessful.
    fn wait_for_completion_zc(&self) -> McrResult<()> {
        let initial_counter = Tcon::tsc();
        while Tcon::tsc() - initial_counter <= SHA_TIMEOUT_COUNT {
            let status = self.regs.sha_status().read();
            if !status.busy() {
                if !status.complete() {
                    Err(ShaErr::CmdFail)?
                }

                return Ok(());
            }
        }

        Err(ShaErr::CmdTimeout)?
    }

    /// Check if the length is aligned to the SHA block size.
    ///
    /// Arguments
    /// `mode` - SHA mode.
    /// `len` - Input length of the buffer.
    ///
    /// # Returns
    ///
    /// `bool` - True if the buffer is block aligned, false otherwise.
    fn aligned(mode: ShaMode, len: usize) -> bool {
        let block_size: usize = match mode {
            ShaMode::Sha1 => SHA1_BLOCK_SIZE,
            ShaMode::Sha256 => SHA256_BLOCK_SIZE,
            ShaMode::Sha384 => SHA384_BLOCK_SIZE,
            ShaMode::Sha512 => SHA512_BLOCK_SIZE,
        };

        (len & !(block_size - 1)) == len
    }

    fn sha_single_block_zc(
        &mut self,
        sha_mode: ShaMode,
        buffer: &IoMemRange,
        len: usize,
        output_buffer: &mut IoMemRange,
    ) -> McrResult<()> {
        // Prepare the command packet.
        let cmd_info_zc = ShaDigestCmdInfoZc {
            buffer,
            init_digest: None,
            mode: sha_mode,
            last: true,
            len: len as u32,
            total_len: len as u32,
            output_buffer,
        };

        self.digest_zc(&cmd_info_zc)
    }

    /// Execute hmac operation.
    /// To compute HMAC over the data `text' we perform
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    ///
    /// # Arguments
    ///
    /// `key` -  Key data
    /// `data` - Message Data
    /// `sha_mode` - SHA mode
    /// `in_buf` - working buffer to use for inputs.
    /// `out_buf` - buffer used for outputs.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Appropriate Err() value if the operation was unsuccessful.
    fn hmac(
        &mut self,
        key: &[u8],
        data: &[u8],
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        out_buf: &mut IoMemRange,
    ) -> McrResult<()> {
        // H(K XOR opad, H(K XOR ipad, text))
        // Namely,
        //  (1) append zeros to the end of K to create a B byte string
        // 	 (e.g., if K is of length 20 bytes and B=64, then K will be
        // 	  appended with 44 zero bytes 0x00)
        //  (2) XOR (bitwise exclusive-OR) the B byte string computed in step
        // 	 (1) with ipad
        //  (3) append the stream of data 'text' to the B byte string resulting
        // 	 from step (2)
        //  (4) apply H to the stream generated in step (3)
        //  (5) XOR (bitwise exclusive-OR) the B byte string computed in
        // 	 step (1) with opad
        //  (6) append the H result from step (4) to the B byte string
        // 	 resulting from step (5)
        //  (7) apply H to the stream generated in step (6) and output
        // 	 the result

        let sha_digest_size = sha_mode.get_digest_size();
        let sha_block_size = sha_mode.get_block_size();

        // Sanity check the lengths of the input and output buf
        if (in_buf.len() < sha_block_size + data.len()) || (out_buf.len() < sha_digest_size) {
            Err(ShaErr::HmacInvalidData)?
        }

        // 128 is the maximum sha_block_size
        let mut key_block = [0; SHA_MAX_BLOCK_SIZE];

        // Applications that use keys longer
        // than B bytes will first hash the key using H and then use the
        // resultant L byte string as the actual key to HMAC.
        if key.len() > sha_block_size {
            // Hash the key
            self.sha_single_block_zc(sha_mode, &IoMemRange::from(key), key.len(), out_buf)?;

            key_block[..sha_digest_size].copy_from_slice(&out_buf.slice()[..sha_digest_size]);
        } else {
            key_block[..key.len()].copy_from_slice(key);
        };

        // Create the IPAD and OPAD local buffers.
        let mut ipad = [0x36; SHA_MAX_BLOCK_SIZE];
        let mut opad = [0x5c; SHA_MAX_BLOCK_SIZE];
        let ipad_len = sha_block_size;
        let opad_len = sha_block_size;

        // Compute (K XOR ipad) and (K XOR opad)
        for i in 0..sha_block_size {
            ipad[i] ^= key_block[i];
            opad[i] ^= key_block[i];
        }

        // Update GSRAM buffer for (K XOR ipad, text)
        in_buf.slice_mut()[..ipad_len].copy_from_slice(&ipad[..ipad_len]);
        in_buf.slice_mut()[ipad_len..ipad_len + data.len()].copy_from_slice(data);

        // H((K XOR ipad, text))
        self.sha_single_block_zc(sha_mode, in_buf, ipad_len + data.len(), out_buf)?;

        // Update GSRAM buffer for (K XOR opad, H(K XOR ipad, text))
        // Copy and concatenate H(K XOR ipad, text) as input for next hash operation
        in_buf.slice_mut()[..opad_len].copy_from_slice(&opad[..opad_len]);
        in_buf.slice_mut()[opad_len..opad_len + sha_digest_size]
            .copy_from_slice(&out_buf.slice()[..sha_digest_size]);

        // H(K XOR opad, H(K XOR ipad, text))
        self.sha_single_block_zc(sha_mode, in_buf, opad_len + sha_digest_size, out_buf)?;

        Ok(())
    }

    /// Execute HKDF operation.
    /// HKDF is implemented per the standard at: https://www.rfc-editor.org/rfc/rfc5869
    ///
    /// # Arguments
    ///
    /// `hkdf_info` -  Contains relevant data for calculating HKDF (key, salt, info, out_len).
    /// `sha_mode` - SHA mode.
    /// `prk_buf` - GSRAM buffer for Psuedo-Random Key (PRK).
    /// `in_buf` - working buffer to use for inputs.
    /// `output` - GSRAM buffer used for outputs.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Appropriate Err() value if the operation was unsuccessful.
    fn hkdf(
        &mut self,
        hkdf_info: HkdfInfo,
        sha_mode: ShaMode,
        prk_buf: &mut IoMemRange,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()> {
        let hash_len = sha_mode.get_digest_size();
        let hash_buffer_len = sha_mode.get_digest_size_hw();

        // Sanity checks
        if hkdf_info.out_len > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) as u16 {
            Err(ShaErr::HkdfSanityCheckFailed)?
        }

        if output.len() < hash_buffer_len + hkdf_info.out_len as usize {
            Err(ShaErr::HkdfSanityCheckFailed)?
        }

        // First: Extract operation. PRK = pseudo random key
        self.hmac(hkdf_info.salt, hkdf_info.key, sha_mode, in_buf, prk_buf)?;

        let prk = &prk_buf.slice()[..hash_len];

        // Second: Expand operation
        // The output OKM is calculated as follows:
        //
        // N = ceil(L/HashLen)
        // T = T(1) | T(2) | T(3) | ... | T(N)
        // OKM = first L octets of T
        //
        // where:
        // T(0) = empty string (zero length)
        // T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
        // T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
        // T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
        // ...
        let iter_num = (hkdf_info.out_len as usize).div_ceil(hash_len);
        let mut prev_t_len = 0;
        let mut output_len = 0;

        let mut prev_t: &[u8] = &[0u8; 0];
        for i in 1..(iter_num + 1) as u8 {
            let mut msg = [0u8; SHA_DIGEST_MAX_SIZE_BYTES + HKDF_MAX_INFO_SIZE + 1];
            msg[..prev_t_len].copy_from_slice(&prev_t[..prev_t_len]);
            msg[prev_t_len..prev_t_len + hkdf_info.info.len()].copy_from_slice(hkdf_info.info);
            msg[prev_t_len + hkdf_info.info.len()] = i;

            let mut hmac_output_range: IoMemRange =
                (&output[output_len..output_len + hash_buffer_len]).into();

            self.hmac(
                prk,
                &msg[..prev_t_len + hkdf_info.info.len() + 1],
                sha_mode,
                in_buf,
                &mut hmac_output_range,
            )?;

            prev_t = &output[output_len..output_len + hash_len];
            prev_t_len = hash_len;

            output_len += prev_t_len;
        }

        // Sanity checks
        if output_len < hkdf_info.out_len as usize {
            output.zeroize();
            Err(ShaErr::HkdfKeyDeriveFailed)?
        }

        Ok(())
    }

    /// Execute KBKDF Counter Mode with HMAC operation
    /// KBKDF is implemented per the standard at:
    /// https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-108r1-upd1.pdf
    ///
    /// # Arguments
    ///
    /// `kbkdf_info` -  Contains relevant data for calculating KBKDF (key, input_data, out_len).
    /// `sha_mode` - SHA mode.
    /// `in_buf` - working buffer to use for inputs.
    /// `output` - GSRAM buffer used for outputs.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Appropriate Err() value if the operation was unsuccessful.
    fn kbkdf_counter_hmac(
        &mut self,
        kbkdf_info: KbkdfInfo,
        sha_mode: ShaMode,
        in_buf: &mut IoMemRange,
        output: &mut [u8],
    ) -> McrResult<()> {
        let hash_len = sha_mode.get_digest_size();
        let hash_buffer_len = sha_mode.get_digest_size_hw();

        // Sanity Checks
        if kbkdf_info.out_len > (KDF_MAX_LENGTH_MULTIPLIER * hash_len) as u16
            || kbkdf_info.out_len as usize == 0
        {
            Err(ShaErr::KbkdfSanityCheckFailed)?
        }

        if output.len() < hash_buffer_len + kbkdf_info.out_len as usize {
            Err(ShaErr::KbkdfSanityCheckFailed)?
        }

        // For KBKDF operation, the maximum label size is 256 bytes, context size is 256 bytes.
        // Thus the maximum input message size is 521 bytes:
        // (4 Bytes || Up to 256 Bytes || 1 Byte || Up to 256 Bytes || 4 Bytes)
        // ([i]2 || Label || 0x00 || Context || [L]2)
        let mut msg = [0u8; KBKDF_MAX_INPUT_DATA_SIZE];
        let mut msg_len = 0;

        match kbkdf_info.input_data {
            KbkdfInputData::ConcatData { label, context } => {
                // Concatenate the input message
                // input message = ([i]2 || Label || 0x00 || Context || [L]2)
                msg_len += KBKDF_RLEN_BYTES;

                msg[msg_len..msg_len + label.len()].copy_from_slice(label);
                msg_len += label.len();

                msg[msg_len] = 0u8;
                msg_len += 1;

                msg[msg_len..msg_len + context.len()].copy_from_slice(context);
                msg_len += context.len();

                let l: u32 = kbkdf_info.out_len as u32 * 8;
                let l_bytes = l.to_be_bytes();
                msg[msg_len..msg_len + KBKDF_RLEN_BYTES].copy_from_slice(&l_bytes);

                msg_len += KBKDF_RLEN_BYTES;
            }
            KbkdfInputData::SelfTestData { fixed_input } => {
                // For self tests fixed input data is (Label || 0x00 || Context || [L]2)
                msg_len = KBKDF_RLEN_BYTES + fixed_input.len();
                msg[KBKDF_RLEN_BYTES..msg_len].copy_from_slice(fixed_input);
            }
        };

        // Process:
        //
        // 1. n := ceil(L/h).
        // 2. If n > 2^r−1, then output an error indicator and stop (i.e., skip steps 3, 4, and 5).
        // 3. result := ∅.
        // 4. For i = 1 to n, do
        //     a. K(i) := PRF (KIN, [i]2 || Label || 0x00 || Context || [L]2),
        //     b. result := result || K(i).
        // 5. KOUT := the leftmost L bits of result
        //
        // If n > 2^r−1, then output an error indicator and stop.
        // This is guaranteed to work because out_len <= KDF_MAX_SECRET_SIZE and hash_len >= 1

        // n := ceil(L/h)
        let iter_num: u32 = kbkdf_info.out_len.div_ceil(hash_len as u16).into();
        let mut current_len = 0;
        // 4. For i = 1 to n, do
        for i in 1..(iter_num + 1) {
            // update [i]2 value
            let i_bytes = i.to_be_bytes();
            msg[..KBKDF_RLEN_BYTES].copy_from_slice(&i_bytes);

            let mut hmac_output_range: IoMemRange =
                (&output[current_len..current_len + hash_buffer_len]).into();

            self.hmac(
                kbkdf_info.key,
                &msg[..msg_len],
                sha_mode,
                in_buf,
                &mut hmac_output_range,
            )?;

            current_len += hash_len;
        }

        // Sanity checks
        if current_len < kbkdf_info.out_len as usize {
            output.zeroize();
            Err(ShaErr::KbkdfKeyDeriveFailed)?
        }

        Ok(())
    }

    /// HKDF Self Test
    ///
    /// # Notes
    ///
    /// "tcId": 7,
    /// "kdfParameter": {
    ///     "kdfType": "hkdf",
    ///     "salt": "B1D609D75E4D0BE81DCAC2A89ABCF0A8ACD77DB8B764178721E6BE68CCF5425138A056A2AE371A1E727C15B5026DCF8EE76D42080E9C980878B70E786C7D6432",
    ///     "z": "7E4A88715DE4D8788F8F069AB1058A42A1BAC4B19E7A4CE8CF7EA04CC6BD64F7",
    ///     "l": 256
    /// },
    /// "fixedInfoPartyU": {
    ///     "partyId": "236B2B475CB1ED72085F11C053BFB240"
    /// },
    /// "fixedInfoPartyV": {
    ///     "partyId": "015F5D21BB744D2DD4F01724034D1BAB"
    /// },
    /// "dkm": "0569C2572A113D897057DDACCCF119D9837D7A5ACD14AACBF7BBF75D42BD5C1D"
    /// Source: test vector generated on ACVP server
    /// All test vectors are in big endian format
    fn hkdf_self_test_256(&mut self) -> McrResult<()> {
        // Modify with input salt in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::Hkdf, None) {
            #[allow(static_mut_refs)]
            unsafe {
                HKDF_SELF_TEST_256_SALT[HKDF_SELF_TEST_256_SALT.len() - 1] =
                    HKDF_SELF_TEST_256_SALT[HKDF_SELF_TEST_256_SALT.len() - 1].wrapping_add(1);
            }
        }

        let sha_mode: ShaMode = ShaMode::Sha256;
        let expected_out_len = L_BYTES as u16;

        self.self_test_buffer.zeroize();

        // n := ceil(L/h)
        // KDF_OUT_BUF_SIZE = n * digest_size
        const N: usize = L_BYTES.div_ceil(SHA_DIGEST_MAX_SIZE_BYTES);
        const KDF_OUT_BUF_SIZE: usize = N * SHA_MAX_BLOCK_SIZE;

        // Split the self_test_buf into key buffer + output buffer + input buffer
        // key buffer = self_test_buf[..PRK_DATA_LEN]
        // output buffer = self_test_buf[PRK_DATA_LEN..PRK_DATA_LEN + KDF_OUT_BUF_SIZE]
        // input buffer = self_test_buf[PRK_DATA_LEN + KDF_OUT_BUF_SIZE..]
        // Partition self_test_buffer into (key | output | input) without MborByteArray wrappers.
        // Use an inner scope to end the mutable borrow of self before calling self.hkdf.
        let (mut prk_range, mut input_range, output_ptr, output_len) = {
            let (key_slice, rest) = self.self_test_buffer.split_at_mut(PRK_DATA_LEN);
            let (output_buf, input_slice) = rest.split_at_mut(KDF_OUT_BUF_SIZE);
            let prk_range: IoMemRange = key_slice.into();
            let input_range: IoMemRange = input_slice.into();
            (
                prk_range,
                input_range,
                output_buf.as_mut_ptr(),
                output_buf.len(),
            )
        };
        // Recreate the mutable output slice after the prior borrows ended.
        let output_buf = mcr_mem_map::mem_addr_to_slice(output_ptr as usize, output_len);

        let hkdf_info = HkdfInfo {
            key: unsafe {
                #[allow(static_mut_refs)]
                &HKDF_SELF_TEST_256_IKM
            },
            salt: unsafe {
                #[allow(static_mut_refs)]
                &HKDF_SELF_TEST_256_SALT
            },
            info: unsafe {
                #[allow(static_mut_refs)]
                &HKDF_SELF_TEST_256_INFO
            },
            out_len: expected_out_len,
        };

        self.hkdf(
            hkdf_info,
            sha_mode,
            &mut prk_range,
            &mut input_range,
            output_buf,
        )?;

        // Ensure that the output matches the expected known output vector
        if unsafe { HKDF_SELF_TEST_256_DKM } != output_buf[..expected_out_len as usize] {
            Err(ShaErr::HkdfSelfTestFailed)?
        }

        self.self_test_buffer.zeroize();

        Ok(())
    }

    /// KBKDF Self Test
    ///
    /// # Notes
    ///
    /// [PRF=HMAC_SHA512]
    /// [CTRLOCATION=BEFORE_FIXED]
    /// [RLEN=32_BITS]
    ///
    /// COUNT=32
    /// L = 320
    /// KI = f5567a2dd9236a99200c4bd5390743e2560bab4b196e3c732b01abf900c7649cab5b957da6ae8fd025605147b36572c19f103670b16f6bb57c138754479ad45d
    /// FixedInputDataByteLen = 60
    /// FixedInputData = a8ccd4bd36fb0ed0765e9662f1028d600bd650e4c2cdddf94b27ee881120aaf74b727b02f0036b46162062e39ed43fa85687a58d177af6f566811889
    ///     Binary rep of i = 00000001
    ///     instring = 00000001a8ccd4bd36fb0ed0765e9662f1028d600bd650e4c2cdddf94b27ee881120aaf74b727b02f0036b46162062e39ed43fa85687a58d177af6f566811889
    /// KO = 2e55b73d126db0f92810266c92e4dc7a7f2d32cbed9eb4edab519e5cc9138c642fd4b229780c17bf
    ///
    /// Source: https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/KBKDF800-108/CounterMode.zip
    /// All test vectors are in big endian format
    fn kbkdf_self_test_512(&mut self) -> McrResult<()> {
        const BITS_IN_BYTE: usize = 8;
        const L: usize = 320;
        const L_BYTES: usize = L / BITS_IN_BYTE;

        // Modify with input salt in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::Kbkdf, None) {
            #[allow(static_mut_refs)]
            unsafe {
                KBKDF_SELF_TEST_512_KI[KBKDF_SELF_TEST_512_KI.len() - 1] =
                    KBKDF_SELF_TEST_512_KI[KBKDF_SELF_TEST_512_KI.len() - 1].wrapping_add(1);
            }
        }

        let sha_mode: ShaMode = ShaMode::Sha512;
        let expected_out_len = L_BYTES as u16;

        self.self_test_buffer.zeroize();

        // n := ceil(L/h)
        // KDF_OUT_BUF_SIZE = n * digest_size
        const N: usize = L_BYTES.div_ceil(SHA_DIGEST_MAX_SIZE_BYTES);
        const KDF_OUT_BUF_SIZE: usize = N * SHA_MAX_BLOCK_SIZE;

        // Split the self_test_buf into key buffer + output buffer + input buffer
        // key buffer = self_test_buf[..KEY_DATA_LEN]
        // output buffer = self_test_buf[KEY_DATA_LEN..KEY_DATA_LEN + KDF_OUT_BUF_SIZE]
        // input buffer = self_test_buf[KEY_DATA_LEN + KDF_OUT_BUF_SIZE..]
        // Partition self_test_buffer into (key | output | input) and wrap only the needed input slice in IoMemRange.
        // Scope the borrow to avoid holding &mut self references across the call to self.kbkdf_counter_hmac.
        let (mut input_range, output_ptr, output_len, key_ptr) = {
            let (key_slice, rest) = self.self_test_buffer.split_at_mut(KBKDF_512_KEY_DATA_LEN);
            let (output_buf, input_slice) = rest.split_at_mut(KDF_OUT_BUF_SIZE);
            // Copy known key input into key slice
            key_slice[..unsafe {
                #[allow(static_mut_refs)]
                KBKDF_SELF_TEST_512_KI.len()
            }]
                .copy_from_slice(unsafe {
                    #[allow(static_mut_refs)]
                    &KBKDF_SELF_TEST_512_KI
                });
            let input_range: IoMemRange = input_slice.into();
            (
                input_range,
                output_buf.as_mut_ptr(),
                output_buf.len(),
                key_slice.as_ptr(),
            )
        };
        // Recreate slices after inner borrow ends
        let output_buf = mcr_mem_map::mem_addr_to_slice(output_ptr as usize, output_len);
        let key_slice = mcr_mem_map::mem_addr_to_slice(key_ptr as usize, KBKDF_512_KEY_DATA_LEN);

        let kbkdf_info = KbkdfInfo {
            key: key_slice,
            input_data: KbkdfInputData::SelfTestData {
                fixed_input: unsafe {
                    #[allow(static_mut_refs)]
                    &KBKDF_SELF_TEST_512_FIXED_INPUT_DATA
                },
            },
            out_len: expected_out_len,
        };

        self.kbkdf_counter_hmac(kbkdf_info, sha_mode, &mut input_range, output_buf)?;

        // Ensure that the output matches the expected known output vector
        if output_buf[..expected_out_len as usize] != unsafe { KBKDF_SELF_TEST_512_KO } {
            Err(ShaErr::KbkdfSelfTestFailed)?
        }

        self.self_test_buffer.zeroize();

        Ok(())
    }

    /// Compute SHA hash using the internal engine
    ///
    /// # Arguments
    ///
    /// * `hash_alg` - Hash algorithm type
    /// * `data` - Data to hash
    ///
    /// # Returns
    ///
    /// * `Ok([u8; SHA_DIGEST_MAX_SIZE_BYTES])` - The computed hash
    /// * `Err(ShaErr)` - If hashing fails
    fn sha_hash(
        &mut self,
        hash_alg: HashAlgorithm,
        data: &[u8],
    ) -> McrResult<[u8; SHA_DIGEST_MAX_SIZE_BYTES]> {
        let sha_mode = ShaMode::from(hash_alg);
        let hash_buffer_len = sha_mode.get_digest_size_hw();

        // Check buffer size limits
        if data.len() > self.self_test_buffer.len() {
            Err(ShaErr::InvalidArgument)?
        }

        // Copy input buffer from DTCM to GSRAM self test buffer
        // This ensures the SHA engine can properly access the data
        self.self_test_buffer.zeroize();
        self.self_test_buffer[..data.len()].copy_from_slice(data);

        // Create IoMemRange for input data using self_test_buffer
        let input_range = IoMemRange::from(&self.self_test_buffer[..data.len()]);

        // Create IoMemRange for output directly over the out_buffer slice (no MborByteArray wrapper)
        let mut output_range = IoMemRange::from(&self.out_buffer[..hash_buffer_len]);

        // Use sha_single_block_zc instead of digest_oaep
        self.sha_single_block_zc(sha_mode, &input_range, data.len(), &mut output_range)?;

        // Copy result to return buffer
        let mut result = [0u8; SHA_DIGEST_MAX_SIZE_BYTES];
        let digest_size = sha_mode.get_digest_size();
        result[..digest_size].copy_from_slice(&output_range.slice()[..digest_size]);

        Ok(result)
    }

    /// Decode message with OAEP padding (private implementation method)
    ///
    /// This implements the OAEP decoding algorithm as specified in RFC 8017.
    ///
    /// # Arguments
    ///
    /// * `encoded_message` - Encoded message buffer (will be modified in-place)
    /// * `key_size` - Size of the RSA key in bytes
    /// * `hash_alg` - Hash algorithm type
    ///
    /// # Returns
    ///
    /// * `Ok(usize)` - Starting index of the decoded message within `encoded_message`
    /// * `Err(ShaErr)` - If decoding fails
    fn decode_oaep(
        &mut self,
        encoded_message: &mut [u8],
        key_size: usize,
        hash_alg: HashAlgorithm,
    ) -> McrResult<usize> {
        let h_len = hash_alg.digest_size();

        // Step 1: Compute lHash = Hash(L) for empty label L
        let l_hash = self.sha_hash(hash_alg, b"")?;
        let l_hash = &l_hash[..h_len];

        // Step 2: Separate the encoded message EM into Y || maskedSeed || maskedDB
        // Let seedMask = MGF(maskedDB, hLen)
        // Let seed = maskedSeed ⊕ seedMask
        let masked_db = &encoded_message[h_len + 1..];
        let seed_mask = self.mgf1(masked_db, h_len, hash_alg)?;
        let seed_mask = &seed_mask.mask[..seed_mask.mask_len as usize];
        let masked_seed = &mut encoded_message[1..h_len + 1];
        xor_slices(masked_seed, seed_mask);

        // Step 3: Let dbMask = MGF(seed, k - hLen - 1)
        // Let DB = maskedDB ⊕ dbMask
        let seed = &encoded_message[1..h_len + 1];
        let db_mask = self.mgf1(seed, key_size - h_len - 1, hash_alg)?;
        let db_mask = &db_mask.mask[..db_mask.mask_len as usize];
        let masked_db = &mut encoded_message[h_len + 1..];
        xor_slices(masked_db, db_mask);

        // Step 4: Separate DB into lHash' || PS || 0x01 || M
        let db = &encoded_message[h_len + 1..];
        let l_hash_em = &db[..h_len];
        let label_mismatch = l_hash_em != l_hash;

        // Find the 0x01 separator
        let ps_len = db
            .iter()
            .skip(h_len)
            .position(|&x| x == 0x01)
            .ok_or_else(|| -> u32 { ShaErr::OaepInvalidSeparator.into() })?;

        let y_non_zero = encoded_message[0] != 0;

        // Step 5: Validate decoding (constant-time check for security)
        if label_mismatch || y_non_zero {
            return Err(ShaErr::OaepDecodeFailed.into());
        }
        // Calculate the starting index of the message M
        // 1 (Y) + h_len (maskedSeed) + h_len (lHash') + ps_len (PS) + 1 (0x01)
        Ok(1 + h_len + h_len + ps_len + 1)
    }

    /// Generate MGF1 mask (private implementation method)
    ///
    /// MGF1 mask generation function based on a hash function as specified in RFC 8017 Appendix B.2.1.
    ///
    /// # Arguments
    ///
    /// * `mgf_seed` - Seed from which mask is generated
    /// * `mask_len` - Intended length of the mask in octets
    /// * `hash_alg` - Hash algorithm type
    ///
    /// # Returns
    ///
    /// * `Ok(Mgf1Output)` - Generated mask
    /// * `Err(ShaErr)` - If mask generation fails
    fn mgf1(
        &mut self,
        mgf_seed: &[u8],
        mask_len: usize,
        hash_alg: HashAlgorithm,
    ) -> McrResult<Mgf1Output> {
        let h_len = hash_alg.digest_size();

        // Initialize output buffer
        let mut t = [0u8; OAEP_MGF1_MAX_T_BUFFER_SIZE];
        let mut counter: i32 = 0;
        let mut t_len: usize = 0;
        let mut hash_input = [0u8; OAEP_MGF1_MAX_HASH_INPUT_SIZE];
        let hash_input_len = mgf_seed.len() + MGF1_COUNTER_SIZE;

        // Copy seed to input buffer
        hash_input[..mgf_seed.len()].copy_from_slice(mgf_seed);

        // Generate mask by iterating until we have enough bytes
        while t_len < mask_len {
            // Convert counter to big-endian bytes (I2OSP)
            let c = counter.to_be_bytes();

            // Concatenate mgfSeed || C and hash it
            hash_input[mgf_seed.len()..mgf_seed.len() + MGF1_COUNTER_SIZE].copy_from_slice(&c);
            let d_hash = self.sha_hash(hash_alg, &hash_input[..hash_input_len])?;
            // Append hash to T
            let copy_len = core::cmp::min(h_len, mask_len - t_len);
            t[t_len..t_len + copy_len].copy_from_slice(&d_hash[..copy_len]);

            t_len += copy_len;
            counter += 1;
        }

        Ok(Mgf1Output {
            mask: t,
            mask_len: mask_len as u32,
        })
    }

    /// Decode OAEP KEK from unwrapped RSA data
    ///
    /// This function handles the specific case of unwrapping a KEK using OAEP padding.
    /// It's designed for HSM key unwrapping operations.
    ///
    /// # Arguments
    ///
    /// * `unwrapped_data` - The data after RSA decryption but before OAEP decoding
    /// * `hash_alg` - Hash algorithm used in OAEP
    ///
    /// # Returns
    ///
    /// * `Ok(SecureByteVec)` - The decoded KEK
    /// * `Err(ShaErr)` - If decoding fails
    pub fn decode_oaep_kek(
        &mut self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<SecureByteVec> {
        // Check minimum size
        if unwrapped_data.len() < UNWRAPPING_KEY_SIZE {
            return Err(ShaErr::OaepInputTooSmall.into());
        }

        // Create secure buffer for the encoded message
        let mut encoded_message =
            SecureByteArray::<UNWRAPPING_KEY_SIZE>::new([0u8; UNWRAPPING_KEY_SIZE]);

        // Reverse copy the unwrapped data (HSM-specific operation)
        reverse_copy(
            encoded_message.as_mut_slice(),
            &unwrapped_data[..UNWRAPPING_KEY_SIZE],
        );

        // Decode OAEP
        let decoded_start = self.decode_oaep(
            encoded_message.as_mut_slice(),
            UNWRAPPING_KEY_SIZE,
            hash_alg,
        )?;

        // Extract the KEK
        let kek = SecureByteVec::from(&encoded_message.as_slice()[decoded_start..]);
        Ok(kek)
    }

    /// Perform self-test for OAEP KEK decoding
    /// This function is used to validate the OAEP KEK decoding implementation
    /// by running a self-test with predefined data.
    ///
    /// # Arguments
    /// * `unwrapped_data` - The data after RSA decryption but before OAEP decoding
    /// * `hash_alg` - Hash algorithm used in OAEP
    ///
    /// # Returns
    ///
    /// * `Ok(())` - If the self-test passes
    /// * `Err(ShaErr)` - If the self-test fails
    pub fn decode_oaep_kek_self_test(
        &mut self,
        unwrapped_data: &[u8],
        hash_alg: HashAlgorithm,
    ) -> McrResult<()> {
        let kek = self.decode_oaep_kek(unwrapped_data, hash_alg)?;

        unsafe {
            // Compare the decoded KEK with the expected oaep_kek_self_test
            if kek.as_slice() != OAEP_KEK_SELF_TEST {
                return Err(ShaErr::OaepSelfTestFailed.into());
            }
        }

        Ok(())
    }
}

/// XOR two byte slices in-place
///
/// # Arguments
///
/// * `a` - First slice (will be modified)
/// * `b` - Second slice (source for XOR)
///
/// # Panics
///
/// Panics if the slices have different lengths
fn xor_slices(a: &mut [u8], b: &[u8]) {
    assert_eq!(
        a.len(),
        b.len(),
        "Slice lengths must be equal for XOR operation"
    );
    for (a_byte, b_byte) in a.iter_mut().zip(b.iter()) {
        *a_byte ^= *b_byte;
    }
}

/// Reverse copy bytes from source to destination
///
/// This is a helper function for HSM-specific operations that require
/// byte order reversal during OAEP operations.
///
/// # Arguments
///
/// * `dest` - Destination buffer
/// * `src` - Source buffer
///
/// # Panics
///
/// Panics if dest and src have different lengths
fn reverse_copy(dest: &mut [u8], src: &[u8]) {
    assert_eq!(
        dest.len(),
        src.len(),
        "Destination and source must have equal lengths"
    );
    for (i, &byte) in src.iter().enumerate() {
        dest[dest.len() - 1 - i] = byte;
    }
}
