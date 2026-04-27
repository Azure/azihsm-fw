// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_error::McrResult;
use mcr_registers::aes_regs::RegisterBlock as AesRegs;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
#[cfg(feature = "fips_validation_hooks")]
use mcr_soc::SocInfo;
use mcr_tcon::Tcon;
use mcr_types::AesCommandCode;
use mcr_types::AesCommandDesc;
use mcr_types::IoMemRange;
use zeroize::Zeroize;

use crate::*;

// The CPU TSC increments the count once every 16 ns.
// Timeout of 1 us corresponds to 62.5 counts of TSC.
// Rounding it up to 63. AES can take 32us in worst
// case (16 KB data for AES CBC 256) so keep it at 50us
// just in case. For 4KB worst case is 9us.
const AES_TIMEOUT_COUNT: u64 = 63 * 50;

// Test vector source:
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_CBC.pdf
static mut AES_SELF_TEST_KEY: [u8; 32] = [
    0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
    0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4,
];

static mut AES_SELF_TEST_IV: [u8; 16] = [
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
];

static mut AES_SELF_TEST_PLAIN_TEXT: [u8; 64] = [
    0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
    0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C, 0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51,
    0x30, 0xC8, 0x1C, 0x46, 0xA3, 0x5C, 0xE4, 0x11, 0xE5, 0xFB, 0xC1, 0x19, 0x1A, 0x0A, 0x52, 0xEF,
    0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17, 0xAD, 0x2B, 0x41, 0x7B, 0xE6, 0x6C, 0x37, 0x10,
];

static mut AES_SELF_TEST_CIPHER_TEXT: [u8; 64] = [
    0xF5, 0x8C, 0x4C, 0x04, 0xD6, 0xE5, 0xF1, 0xBA, 0x77, 0x9E, 0xAB, 0xFB, 0x5F, 0x7B, 0xFB, 0xD6,
    0x9C, 0xFC, 0x4E, 0x96, 0x7E, 0xDB, 0x80, 0x8D, 0x67, 0x9F, 0x77, 0x7B, 0xC6, 0x70, 0x2C, 0x7D,
    0x39, 0xF2, 0x33, 0x69, 0xA9, 0xD9, 0xBA, 0xCF, 0xA5, 0x30, 0xE2, 0x63, 0x04, 0x23, 0x14, 0x61,
    0xB2, 0xEB, 0x05, 0xE2, 0xC3, 0x9B, 0xE9, 0xFC, 0xDA, 0x6C, 0x19, 0x07, 0x8C, 0x6A, 0x9D, 0x1B,
];

/// AES Engine
#[derive(Clone)]
pub struct Aes {
    rimpl: Rc<RefCell<AesImpl>>,
}

impl Aes {
    /// Create an instance of `Aes`
    ///
    /// # Arguments
    ///
    /// * `cmd_buffer` - The AES command buffer to be used for AES operations.
    ///
    /// # Returns
    ///
    /// * `Ok(Aes)` - An instance of `Aes` if initialization successful, error code otherwise
    pub fn new(cmd_buffer: &'static mut AesCommandDesc) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(AesImpl::new(cmd_buffer))),
        }
    }
}

impl AesTrait for Aes {
    /// Perform encrypt or decrypt operation
    fn encrypt_decrypt(&self, cmd_info: &AesCommand) -> McrResult<()> {
        self.rimpl.borrow_mut().encrypt_decrypt(cmd_info)
    }

    fn aes_cbc_self_test(
        &self,
        self_test_input: &mut [u8],
        self_test_output: &mut [u8],
        self_test_iv: &mut [u8],
    ) -> McrResult<()> {
        self.rimpl
            .borrow_mut()
            .aes_cbc_self_test(self_test_input, self_test_output, self_test_iv)
    }
}

/// AES internal implementation
struct AesImpl {
    /// Command buffer to submit AES buffer
    cmd_buffer: &'static mut AesCommandDesc,

    /// AES hardware register access object
    regs: AesRegs,

    /// Tag used to submit AES operation
    tag: Option<u16>,
}

impl AesImpl {
    /// AES IV Length
    const AES_IV_LEN: usize = 16;

    /// Create an instance of `AesImpl`
    fn new(cmd_buffer: &'static mut AesCommandDesc) -> Self {
        AesImpl {
            cmd_buffer,
            regs: AesRegs::block(),
            tag: None,
        }
    }

    /// Perform encrypt or decrypt operation
    fn encrypt_decrypt(&mut self, cmd_info: &AesCommand) -> McrResult<()> {
        if self.regs.status().read().busy() {
            Err(AesErr::EngineBusy)?
        }

        self.prepare_cmd_buffer(cmd_info)?;

        mcr_cpu::dmb();

        self.regs
            .cmd_addr()
            .write(|_| self.cmd_buffer as *const AesCommandDesc as u32);

        self.wait_for_completion(cmd_info)
    }

    /// Prepare the AES command buffer.
    ///
    /// # Arguments
    ///
    /// * `cmd_info` - The AES command related information.
    ///
    /// # Returns
    ///
    /// * `Result<(), u32>` - An empty `Result` indicating success or an error code.
    fn prepare_cmd_buffer(&mut self, cmd_info: &AesCommand) -> McrResult<()> {
        if cmd_info.message.len() % 16 != 0 {
            Err(AesErr::InvalidMessageLength)?
        }

        if cmd_info.mode == AesMode::Cbc {
            if cmd_info.iv.is_none() {
                Err(AesErr::IVMissing)?
            }

            if let Some(iv) = cmd_info.iv {
                if iv.len() != Self::AES_IV_LEN {
                    Err(AesErr::InvalidIVLength)?
                }
            }
        }

        if cmd_info.result.len() < cmd_info.message.len() {
            Err(AesErr::ResultBufferTooSmall)?
        }

        let key_len: AesKeyLength = cmd_info.key.len().try_into()?;

        let cmd_code = AesCommandCode::default()
            .with_mode(cmd_info.mode.into())
            .with_update_iv(cmd_info.update_iv)
            .with_decrypt_encrypt(cmd_info.op.into())
            .with_key_length(key_len.into());

        self.cmd_buffer.cmd_code = cmd_code;
        self.cmd_buffer.byte_count = cmd_info.message.len() as u32;
        self.cmd_buffer.message = cmd_info.message.addr() as u32;

        if let Some(iv) = cmd_info.iv {
            self.cmd_buffer.iv = iv.addr() as u32;
        }

        self.cmd_buffer.key = cmd_info.key.as_ptr() as u32;
        self.cmd_buffer.result = cmd_info.result.addr() as u32;

        self.tag = Some(cmd_info.tag);

        Ok(())
    }

    /// Wait for AES operation to complete.
    ///
    /// # Arguments
    /// `cmd_info` - The AES command related information.
    ///
    /// # Returns
    /// * `McrResult<ShaDigest>` - Resultant SHA digest or appropriate Err() value
    fn wait_for_completion(&self, _command_info: &AesCommand) -> McrResult<()> {
        let initial_counter = Tcon::tsc();

        while Tcon::tsc() - initial_counter <= AES_TIMEOUT_COUNT {
            let status = self.regs.status().read();

            if !status.busy() {
                if !status.complete() {
                    Err(AesErr::CmdFail)?
                }

                return Ok(());
            }
        }

        Err(AesErr::CmdTimeout)?
    }

    // AES CBC self test
    fn aes_cbc_self_test(
        &mut self,
        self_test_input: &mut [u8],
        self_test_output: &mut [u8],
        self_test_iv: &mut [u8],
    ) -> McrResult<()> {
        // Modify aes key used in self test to induce failure for FIPS validation
        // if this test is expected to be failed based on the FIPS validation hooks.
        #[cfg(feature = "fips_validation_hooks")]
        if SocInfo::default().induce_cast_failure(SelfTest::AesCbc, None) {
            unsafe {
                AES_SELF_TEST_KEY[0] = 0x00;
            }
        }

        if self_test_input.len() != AES_SELF_TEST_INPUT_BUF_MAX_SIZE_BYTES {
            Err(AesErr::SelfTestFailed)?
        }

        if self_test_output.len() != AES_SELF_TEST_OUTPUT_BUF_MAX_SIZE_BYTES {
            Err(AesErr::SelfTestFailed)?
        }

        if self_test_iv.len() != AES_SELF_TEST_IV_BUF_MAX_SIZE_BYTES {
            Err(AesErr::SelfTestFailed)?
        }

        // Key, IV, input, output
        self_test_input.zeroize();
        self_test_output.zeroize();

        let aes_key_start = 0;
        let aes_key_end = aes_key_start
            + unsafe {
                #[allow(static_mut_refs)]
                AES_SELF_TEST_KEY.len()
            };
        let input_start = aes_key_end;
        let input_end = input_start
            + unsafe {
                #[allow(static_mut_refs)]
                AES_SELF_TEST_PLAIN_TEXT.len()
            };

        // Key
        self_test_input[aes_key_start..aes_key_end].copy_from_slice(unsafe {
            #[allow(static_mut_refs)]
            &AES_SELF_TEST_KEY
        });

        // Input
        self_test_input[input_start..input_end].copy_from_slice(unsafe {
            #[allow(static_mut_refs)]
            &AES_SELF_TEST_PLAIN_TEXT
        });

        // IV
        self_test_iv.copy_from_slice(unsafe {
            #[allow(static_mut_refs)]
            &AES_SELF_TEST_IV
        });

        let tag = 0xFE;

        let mut message_io: IoMemRange = (&self_test_input[input_start..input_end]).into();
        let iv_io: IoMemRange = (&self_test_iv[..]).into();
        let mut result_io: IoMemRange = (&self_test_output[..]).into();

        // Perform self test to verify AES encryption.
        let cmd_info = AesCommand {
            tag,
            message: &message_io,
            iv: Some(&iv_io),
            key: &self_test_input[aes_key_start..aes_key_end],
            mode: AesMode::Cbc,
            op: AesOp::Encrypt,
            update_iv: false,
            result: &result_io,
        };

        self.encrypt_decrypt(&cmd_info)?;

        if self_test_output
            != unsafe {
                #[allow(static_mut_refs)]
                &AES_SELF_TEST_CIPHER_TEXT
            }
        {
            Err(AesErr::SelfTestFailed)?
        }

        // Prepare Input
        self_test_input[input_start..input_end].copy_from_slice(unsafe {
            #[allow(static_mut_refs)]
            &AES_SELF_TEST_CIPHER_TEXT
        });

        // Zeroize the output.
        self_test_output.zeroize();

        let tag = 0xFE;

        // Update IoMemRange wrappers to point to the now-prepared input/cipher buffers
        message_io = (&self_test_input[input_start..input_end]).into();
        // iv_io remains the same pointer to IV buffer in this test
        result_io = (&self_test_output[..]).into();

        // Perform self test to verify AES decryption.
        let cmd_info = AesCommand {
            tag,
            message: &message_io,
            iv: Some(&iv_io),
            key: &self_test_input[aes_key_start..aes_key_end],
            mode: AesMode::Cbc,
            op: AesOp::Decrypt,
            update_iv: false,
            result: &result_io,
        };

        self.encrypt_decrypt(&cmd_info)?;

        if self_test_output != unsafe { AES_SELF_TEST_PLAIN_TEXT } {
            Err(AesErr::SelfTestFailed)?
        }

        Ok(())
    }
}
