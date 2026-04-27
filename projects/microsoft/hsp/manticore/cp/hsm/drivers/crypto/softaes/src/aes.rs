// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::decrypt::*;
use crate::encrypt::*;
use crate::keyschedule_128::*;
use crate::keyschedule_192::*;
use crate::keyschedule_256::*;
use crate::keyschedule_dec::*;

#[derive(Clone)]
pub struct Aes128Encrypt {
    round_key: [u8; 16 * 11],
}
#[derive(Clone)]
pub struct Aes128Decrypt {
    round_key: [u8; 16 * 11],
}
#[derive(Clone)]
pub struct Aes192Encrypt {
    round_key: [u8; 16 * 13],
}

#[derive(Clone)]
pub struct Aes192Decrypt {
    round_key: [u8; 16 * 13],
}

#[derive(Clone)]
pub struct Aes256Encrypt {
    round_key: [u8; 16 * 15],
}

#[derive(Clone)]
pub struct Aes256Decrypt {
    round_key: [u8; 16 * 15],
}

impl Aes128Decrypt {
    pub fn new() -> Aes128Decrypt {
        Aes128Decrypt {
            round_key: [0; 16 * 11],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 16]) -> Aes128Decrypt {
        let mut aes = Aes128Decrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes128_keyschedule_enc(rk, key);
    }

    pub fn raw_key_schedule_dec(rk: *mut u8) {
        cm7_1t_aes_keyschedule_dec(rk, 10);
    }

    pub fn raw_decrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_decrypt(rk, data_in, data_out, 10);
    }

    pub fn set_key(&mut self, key: &[u8; 16]) {
        Aes128Decrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
        Aes128Decrypt::raw_key_schedule_dec((&mut self.round_key).as_mut_ptr());
    }

    pub fn decrypt_block_inplace(&self, block: &mut [u8]) {
        Aes128Decrypt::raw_decrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}

impl Aes128Encrypt {
    pub fn new() -> Aes128Encrypt {
        Aes128Encrypt {
            round_key: [0; 16 * 11],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 16]) -> Aes128Encrypt {
        let mut aes = Aes128Encrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes128_keyschedule_enc(rk, key);
    }

    pub fn raw_encrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_encrypt(rk, data_in, data_out, 10);
    }

    pub fn set_key(&mut self, key: &[u8; 16]) {
        Aes128Encrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
    }

    pub fn encrypt_block_inplace(&self, block: &mut [u8]) {
        Aes128Encrypt::raw_encrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}

impl Aes192Decrypt {
    pub fn new() -> Aes192Decrypt {
        Aes192Decrypt {
            round_key: [0; 16 * 13],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 24]) -> Aes192Decrypt {
        let mut aes = Aes192Decrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes192_keyschedule_enc(rk, key);
    }

    pub fn raw_key_schedule_dec(rk: *mut u8) {
        cm7_1t_aes_keyschedule_dec(rk, 12);
    }

    pub fn raw_decrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_decrypt(rk, data_in, data_out, 12);
    }

    pub fn set_key(&mut self, key: &[u8; 24]) {
        Aes192Decrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
        Aes192Decrypt::raw_key_schedule_dec((&mut self.round_key).as_mut_ptr());
    }

    pub fn decrypt_block_inplace(&self, block: &mut [u8]) {
        Aes192Decrypt::raw_decrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}

impl Aes192Encrypt {
    pub fn new() -> Aes192Encrypt {
        Aes192Encrypt {
            round_key: [0; 16 * 13],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 24]) -> Aes192Encrypt {
        let mut aes = Aes192Encrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes192_keyschedule_enc(rk, key);
    }

    pub fn raw_encrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_encrypt(rk, data_in, data_out, 12);
    }

    pub fn set_key(&mut self, key: &[u8; 24]) {
        Aes192Encrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
    }

    pub fn encrypt_block_inplace(&self, block: &mut [u8]) {
        Aes192Encrypt::raw_encrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}

impl Aes256Decrypt {
    pub fn new() -> Aes256Decrypt {
        Aes256Decrypt {
            round_key: [0; 16 * 15],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 32]) -> Aes256Decrypt {
        let mut aes = Aes256Decrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes256_keyschedule_enc(rk, key);
    }

    pub fn raw_key_schedule_dec(rk: *mut u8) {
        cm7_1t_aes_keyschedule_dec(rk, 14);
    }

    pub fn raw_decrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_decrypt(rk, data_in, data_out, 14);
    }

    pub fn set_key(&mut self, key: &[u8; 32]) {
        Aes256Decrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
        Aes256Decrypt::raw_key_schedule_dec((&mut self.round_key).as_mut_ptr());
    }

    pub fn decrypt_block_inplace(&self, block: &mut [u8]) {
        Aes256Decrypt::raw_decrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}

impl Aes256Encrypt {
    pub fn new() -> Aes256Encrypt {
        Aes256Encrypt {
            round_key: [0; 16 * 15],
        }
    }

    #[allow(unused)]
    pub fn from_key(key: &[u8; 32]) -> Aes256Encrypt {
        let mut aes = Aes256Encrypt::new();
        aes.set_key(key);
        aes
    }

    pub fn raw_key_schedule_enc(rk: *mut u8, key: *const u8) {
        cm7_1t_aes256_keyschedule_enc(rk, key);
    }

    pub fn raw_encrypt_block(rk: *const u8, data_in: *const u8, data_out: *mut u8) {
        cm7_1t_aes_encrypt(rk, data_in, data_out, 14);
    }

    pub fn set_key(&mut self, key: &[u8; 32]) {
        Aes256Encrypt::raw_key_schedule_enc((&mut self.round_key).as_mut_ptr(), key.as_ptr());
    }

    pub fn encrypt_block_inplace(&self, block: &mut [u8]) {
        Aes256Encrypt::raw_encrypt_block(
            (&self.round_key).as_ptr(),
            block.as_ptr(),
            block.as_mut_ptr(),
        );
    }
}
