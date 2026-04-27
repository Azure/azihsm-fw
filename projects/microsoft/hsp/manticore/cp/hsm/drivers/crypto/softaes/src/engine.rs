// Copyright (c) Microsoft Corporation. All rights reserved.
#[cfg(target_arch = "arm")]
use alloc::rc::Rc;
#[cfg(target_arch = "arm")]
use core::cell::RefCell;
use core::ops::Range;

#[cfg(not(target_arch = "arm"))]
use openssl::symm::Cipher;
#[cfg(not(target_arch = "arm"))]
use openssl::symm::Crypter;
#[cfg(not(target_arch = "arm"))]
use openssl::symm::Mode;

use mcr_error::McrResult;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_types::*;

#[cfg(target_arch = "arm")]
use crate::aes::*;

use crate::gcm_tag_correct;
use crate::gcm_tag_correct_aad_only;
use crate::key_unwrap_inplace;
use crate::SoftAesErr;
use crate::SoftAesTrait;

// Source:
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_ECB.pdf
static mut AES_ECB_256_SELF_TEST_MESSAGE: [u8; 64] = [
    0xF3, 0xEE, 0xD1, 0xBD, 0xB5, 0xD2, 0xA0, 0x3C, 0x06, 0x4B, 0x5A, 0x7E, 0x3D, 0xB1, 0x81, 0xF8,
    0x59, 0x1C, 0xCB, 0x10, 0xD4, 0x10, 0xED, 0x26, 0xDC, 0x5B, 0xA7, 0x4A, 0x31, 0x36, 0x28, 0x70,
    0xB6, 0xED, 0x21, 0xB9, 0x9C, 0xA6, 0xF4, 0xF9, 0xF1, 0x53, 0xE7, 0xB1, 0xBE, 0xAF, 0xED, 0x1D,
    0x23, 0x30, 0x4B, 0x7A, 0x39, 0xF9, 0xF3, 0xFF, 0x06, 0x7D, 0x8D, 0x8F, 0x9E, 0x24, 0xEC, 0xC7,
];

static mut AES_ECB_256_SELF_TEST_KEY: [u8; 32] = [
    0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
    0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4,
];

static mut AES_ECB_256_SELF_TEST_EXPECTED: [u8; 64] = [
    0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
    0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C, 0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51,
    0x30, 0xC8, 0x1C, 0x46, 0xA3, 0x5C, 0xE4, 0x11, 0xE5, 0xFB, 0xC1, 0x19, 0x1A, 0x0A, 0x52, 0xEF,
    0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17, 0xAD, 0x2B, 0x41, 0x7B, 0xE6, 0x6C, 0x37, 0x10,
];

// AES 256 Key Wrap with Padding Authenticated Decryption (KWP-AD)
//
// [PLAINTEXT LENGTH = 64]
// COUNT = 0
// K = 3517f0efa7f0c4d74f91af83ece5e7503bcc5ab82907a6e4b7ed34d87b69ab1d
// P = 897e0456b289ad31
// C = 0b06a9b635d50cda9d4210cb3a71f990
//
// Source:
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/mac/kwtestvectors.zip

#[cfg(feature = "fips_validation_hooks")]
static mut KEK: [u8; 32] = [
    0x35, 0x17, 0xf0, 0xef, 0xa7, 0xf0, 0xc4, 0xd7, 0x4f, 0x91, 0xaf, 0x83, 0xec, 0xe5, 0xe7, 0x50,
    0x3b, 0xcc, 0x5a, 0xb8, 0x29, 0x07, 0xa6, 0xe4, 0xb7, 0xed, 0x34, 0xd8, 0x7b, 0x69, 0xab, 0x1d,
];
#[cfg(not(feature = "fips_validation_hooks"))]
static mut KEK: [u8; 32] = [
    0x35, 0x17, 0xf0, 0xef, 0xa7, 0xf0, 0xc4, 0xd7, 0x4f, 0x91, 0xaf, 0x83, 0xec, 0xe5, 0xe7, 0x50,
    0x3b, 0xcc, 0x5a, 0xb8, 0x29, 0x07, 0xa6, 0xe4, 0xb7, 0xed, 0x34, 0xd8, 0x7b, 0x69, 0xab, 0x1d,
];

static mut AES_256_SELF_TEST_EXPECTED: [u8; 8] = [0x89, 0x7e, 0x04, 0x56, 0xb2, 0x89, 0xad, 0x31];

static mut AES_256_SELF_TEST_INPUT: [u8; 16] = [
    0x0b, 0x06, 0xa9, 0xb6, 0x35, 0xd5, 0x0c, 0xda, 0x9d, 0x42, 0x10, 0xcb, 0x3a, 0x71, 0xf9, 0x90,
];

/// Trait for synchronous implementations of AES.
/// Encrypts or decrypts only a single block in ECB mode.
pub trait SyncAesEcb {
    /// AES ECB encrypt
    ///
    /// # Arguments
    ///
    /// * `key` - AES key as u8 slice.
    /// * `block` - u8 slice containing the plaintext block, this same block is replaced with
    ///   ciphertext.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if completed successfully or an appropriate error code.
    fn encrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()>;

    /// AES ECB decrypt
    ///
    /// # Arguments
    ///
    /// * `key` - AES key as u8 slice.
    /// * `block` - u8 slice containing the cyphertext block, this same block is replaced with
    ///   plaintext.
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok(()) if completed successfully or an appropriate error code.
    fn decrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()>;
}

#[cfg(target_arch = "arm")]
/// SoftAes Engine
#[derive(Clone)]
pub struct SoftAes {
    encrypt128impl: Rc<RefCell<Aes128Encrypt>>,
    encrypt128key: Rc<RefCell<Option<[u8; 16]>>>,
    decrypt128impl: Rc<RefCell<Aes128Decrypt>>,
    decrypt128key: Rc<RefCell<Option<[u8; 16]>>>,

    encrypt192impl: Rc<RefCell<Aes192Encrypt>>,
    encrypt192key: Rc<RefCell<Option<[u8; 24]>>>,
    decrypt192impl: Rc<RefCell<Aes192Decrypt>>,
    decrypt192key: Rc<RefCell<Option<[u8; 24]>>>,

    encrypt256impl: Rc<RefCell<Aes256Encrypt>>,
    encrypt256key: Rc<RefCell<Option<[u8; 32]>>>,
    decrypt256impl: Rc<RefCell<Aes256Decrypt>>,
    decrypt256key: Rc<RefCell<Option<[u8; 32]>>>,
}

#[cfg(target_arch = "arm")]
impl SoftAes {
    /// Create an instance of `SoftAes`
    ///
    /// # Returns
    ///
    /// * `SoftAes` - An instance of `SoftAes`
    pub fn new() -> Self {
        Self {
            encrypt128impl: Rc::new(RefCell::new(Aes128Encrypt::new())),
            encrypt128key: Rc::new(RefCell::new(None)),
            decrypt128impl: Rc::new(RefCell::new(Aes128Decrypt::new())),
            decrypt128key: Rc::new(RefCell::new(None)),
            encrypt192impl: Rc::new(RefCell::new(Aes192Encrypt::new())),
            encrypt192key: Rc::new(RefCell::new(None)),
            decrypt192impl: Rc::new(RefCell::new(Aes192Decrypt::new())),
            decrypt192key: Rc::new(RefCell::new(None)),
            encrypt256impl: Rc::new(RefCell::new(Aes256Encrypt::new())),
            encrypt256key: Rc::new(RefCell::new(None)),
            decrypt256impl: Rc::new(RefCell::new(Aes256Decrypt::new())),
            decrypt256key: Rc::new(RefCell::new(None)),
        }
    }

    fn load_encrypt_key(&self, key: &[u8]) -> McrResult<()> {
        // check if we can reuse instance
        match key.len() {
            16 => {
                if self.encrypt128key.borrow().is_none()
                    || self.encrypt128key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<16> =
                        SecureByteArray::new(key.try_into().expect("key must be 16 bytes"));
                    self.encrypt128key.replace(Some(*x));
                    self.encrypt128impl
                        .borrow_mut()
                        .set_key(&self.encrypt128key.borrow().unwrap());
                }
            }
            24 => {
                if self.encrypt192key.borrow().is_none()
                    || self.encrypt192key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<24> =
                        SecureByteArray::new(key.try_into().expect("key must be 24 bytes"));
                    self.encrypt192key.replace(Some(*x));
                    self.encrypt192impl
                        .borrow_mut()
                        .set_key(&self.encrypt192key.borrow().unwrap());
                }
            }
            32 => {
                if self.encrypt256key.borrow().is_none()
                    || self.encrypt256key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<32> =
                        SecureByteArray::new(key.try_into().expect("key must be 32 bytes"));
                    self.encrypt256key.replace(Some(*x));
                    self.encrypt256impl
                        .borrow_mut()
                        .set_key(&self.encrypt256key.borrow().unwrap());
                }
            }
            _ => Err(SoftAesErr::InvalidKekLength)?,
        }
        Ok(())
    }

    fn load_decrypt_key(&self, key: &[u8]) -> McrResult<()> {
        // check if we can reuse instance
        match key.len() {
            16 => {
                if self.decrypt128key.borrow().is_none()
                    || self.decrypt128key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<16> =
                        SecureByteArray::new(key.try_into().expect("key must be 16 bytes"));
                    self.decrypt128key.replace(Some(*x));
                    self.decrypt128impl
                        .borrow_mut()
                        .set_key(&self.decrypt128key.borrow().unwrap());
                }
            }
            24 => {
                if self.decrypt192key.borrow().is_none()
                    || self.decrypt192key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<24> =
                        SecureByteArray::new(key.try_into().expect("key must be 24 bytes"));
                    self.decrypt192key.replace(Some(*x));
                    self.decrypt192impl
                        .borrow_mut()
                        .set_key(&self.decrypt192key.borrow().unwrap());
                }
            }
            32 => {
                if self.decrypt256key.borrow().is_none()
                    || self.decrypt256key.borrow().unwrap() != key
                {
                    let x: SecureByteArray<32> =
                        SecureByteArray::new(key.try_into().expect("key must be 32 bytes"));
                    self.decrypt256key.replace(Some(*x));
                    self.decrypt256impl
                        .borrow_mut()
                        .set_key(&self.decrypt256key.borrow().unwrap());
                }
            }
            _ => Err(SoftAesErr::InvalidKekLength)?,
        }
        Ok(())
    }
}

#[cfg(target_arch = "arm")]
impl SyncAesEcb for SoftAes {
    fn encrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        if block.len() != 16 {
            Err(SoftAesErr::InsufficientInputLength)?
        }
        self.load_encrypt_key(key)?;
        match key.len() {
            16 => self.encrypt128impl.borrow().encrypt_block_inplace(block),
            24 => self.encrypt192impl.borrow().encrypt_block_inplace(block),
            32 => self.encrypt256impl.borrow().encrypt_block_inplace(block),
            _ => Err(SoftAesErr::InvalidKekLength)?,
        };
        Ok(())
    }

    fn decrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        if block.len() != 16 {
            Err(SoftAesErr::InsufficientInputLength)?
        }
        self.load_decrypt_key(key)?;
        match key.len() {
            16 => self.decrypt128impl.borrow().decrypt_block_inplace(block),
            24 => self.decrypt192impl.borrow().decrypt_block_inplace(block),
            32 => self.decrypt256impl.borrow().decrypt_block_inplace(block),
            _ => Err(SoftAesErr::InvalidKekLength)?,
        };
        Ok(())
    }
}

/// SoftAes Implementation for unit-testing.
#[cfg(not(target_arch = "arm"))]
#[derive(Clone)]
pub struct SoftAes {}

impl Default for SoftAes {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(not(target_arch = "arm"))]
impl SoftAes {
    /// Create an instance of `SoftAes`
    ///
    /// # Returns
    ///
    /// * `SoftAes` - An instance of `SoftAes`
    pub fn new() -> Self {
        Self {}
    }

    /// AES ECB Block encryption in place
    fn aes_encrypt_block_ecb(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        let cipher = self.get_cipher(key.len())?;

        let mut crypter = Crypter::new(cipher, Mode::Encrypt, key, None)
            .map_err(|_| SoftAesErr::EngineInitFailed)?;
        let mut ciphertext = SecureByteVec::zeroed(block.len() + cipher.block_size());

        // Disable Padding
        crypter.pad(false);

        let count = crypter
            .update(block, &mut ciphertext)
            .map_err(|_| SoftAesErr::EncryptFailed)?;
        let _rest = crypter
            .finalize(&mut ciphertext[count..])
            .map_err(|_| SoftAesErr::FinalyzingEncryptFailed)?;

        block.copy_from_slice(&ciphertext[..block.len()]);

        Ok(())
    }

    /// AES ECB Block decryption in place
    fn aes_decrypt_block_ecb(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        let cipher = self.get_cipher(key.len())?;

        let mut crypter = Crypter::new(cipher, Mode::Decrypt, key, None)
            .map_err(|_| SoftAesErr::EngineInitFailed)?;
        let mut plaintext = SecureByteVec::zeroed(block.len() + cipher.block_size());

        // Disable Padding
        crypter.pad(false);

        let count = crypter
            .update(block, &mut plaintext)
            .map_err(|_| SoftAesErr::DecryptFailed)?;
        let _rest = crypter
            .finalize(&mut plaintext[count..])
            .map_err(|_| SoftAesErr::FinalyzingDecryptFailed)?;

        block.copy_from_slice(&plaintext[..block.len()]);

        Ok(())
    }

    /// Get the corresponding openssl-cipher based on key length
    fn get_cipher(&self, key_len: usize) -> McrResult<Cipher> {
        let cipher = match key_len {
            16 => Cipher::aes_128_ecb(),
            24 => Cipher::aes_192_ecb(),
            32 => Cipher::aes_256_ecb(),
            _ => Err(SoftAesErr::InvalidKekLength)?,
        };

        Ok(cipher)
    }
}

#[cfg(not(target_arch = "arm"))]
impl SyncAesEcb for SoftAes {
    fn encrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        if block.len() != 16 {
            Err(SoftAesErr::InsufficientInputLength)?
        }

        self.aes_encrypt_block_ecb(key, block)
    }

    fn decrypt(&self, key: &[u8], block: &mut [u8]) -> McrResult<()> {
        if block.len() != 16 {
            Err(SoftAesErr::InsufficientInputLength)?
        }

        self.aes_decrypt_block_ecb(key, block)
    }
}

impl SoftAesTrait for SoftAes {
    fn key_unwrap_inplace(&self, kek: &[u8], input: &mut [u8]) -> McrResult<Range<usize>> {
        key_unwrap_inplace(self, kek, input)
    }

    fn ecb_decrypt(&self, key: &[u8], inout: &mut [u8]) -> McrResult<Range<usize>> {
        const AES_BLOCK_SIZE: usize = 16;

        if !matches!(key.len(), 16 | 24 | 32) {
            Err(SoftAesErr::InvalidKekLength)?
        }

        if inout.len() % 16 != 0 {
            Err(SoftAesErr::InsufficientInputLength)?
        }

        for block in inout.chunks_mut(AES_BLOCK_SIZE) {
            self.decrypt(key, block)?;
        }

        Ok(0..inout.len())
    }

    #[allow(clippy::too_many_arguments)]
    fn aes_gcm_tag_correction(
        &self,
        encrypt: bool,
        key: &[u8],
        iv: &[u8; 12],
        aad_len: u64,
        text_len: u64,
        aad: Option<&[u8]>,
        bad_tag: Option<&[u8; 16]>,
        unaligned_input_block: &[u8],
        aligned_input_len: usize,
        output: &mut [u8],
    ) -> McrResult<[u8; 16]> {
        if aad_len != 0 && aligned_input_len == 0 && !unaligned_input_block.is_empty() {
            let intermediate_tag = bad_tag.ok_or(SoftAesErr::IntermediateTagMissing)?;
            gcm_tag_correct_aad_only(
                self,
                encrypt,
                key,
                iv,
                aad_len,
                intermediate_tag,
                unaligned_input_block,
                output,
            )
        } else {
            gcm_tag_correct(
                self,
                encrypt,
                key,
                iv,
                aad_len,
                text_len,
                aad,
                bad_tag,
                unaligned_input_block,
                output,
            )
        }
    }

    fn aes_ecb_256_decrypt_self_test(&self) -> McrResult<()> {
        const AES_BLOCK_SIZE: usize = 16;

        // Corraborate with ec key used in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::AesEcb, None) {
            #[allow(static_mut_refs)]
            unsafe {
                AES_ECB_256_SELF_TEST_KEY[AES_ECB_256_SELF_TEST_KEY.len() - 1] =
                    AES_ECB_256_SELF_TEST_KEY[AES_ECB_256_SELF_TEST_KEY.len() - 1].wrapping_add(1);
            }
        }

        let mut message = unsafe { AES_ECB_256_SELF_TEST_MESSAGE };

        for block in message.chunks_mut(AES_BLOCK_SIZE) {
            self.decrypt(
                unsafe {
                    #[allow(static_mut_refs)]
                    &AES_ECB_256_SELF_TEST_KEY
                },
                block,
            )?;
        }

        if message != unsafe { AES_ECB_256_SELF_TEST_EXPECTED } {
            Err(SoftAesErr::SoftAesSelfTestDecryptFailure)?
        }

        Ok(())
    }

    fn aes_256_key_unwrap_self_test(&self) -> McrResult<()> {
        // Modify with aes key used in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::AesUnwrapWithPadding, None) {
            #[allow(static_mut_refs)]
            unsafe {
                KEK[KEK.len() - 1] = KEK[KEK.len() - 1].wrapping_add(1);
            }
        }

        let mut input = unsafe { AES_256_SELF_TEST_INPUT };

        let range = key_unwrap_inplace(
            self,
            unsafe {
                #[allow(static_mut_refs)]
                &KEK
            },
            &mut input,
        )?;

        if &input[range] != unsafe { &AES_256_SELF_TEST_EXPECTED[..] } {
            Err(SoftAesErr::SoftAesSelfTestKeyUnwrapFailure)?
        }

        Ok(())
    }
}
