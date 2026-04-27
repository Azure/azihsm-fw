// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::engine::SyncAesEcb;
use crate::SoftAes;
use crate::SoftAesErr;
use mcr_error::McrResult;

/// Corrects a GCM tag that was computed possibly without the last output block or with
/// AAD or text lengths.
#[allow(clippy::too_many_arguments)]
pub fn gcm_tag_correct(
    engine: &SoftAes,
    encrypt: bool,
    key: &[u8],
    iv: &[u8; 12],
    aad_len: u64,
    text_len: u64,
    partial_aad: Option<&[u8]>,
    bad_tag: Option<&[u8; 16]>,
    partial_last_input_block: &[u8],
    output: &mut [u8],
) -> McrResult<[u8; 16]> {
    if partial_last_input_block.len() > 16 {
        return Err(SoftAesErr::LastInputBlockTooLarge.into());
    }
    if partial_last_input_block.len() != output.len() {
        return Err(SoftAesErr::InsufficientOutputBufferLength.into());
    }
    if partial_aad.is_some() && bad_tag.is_some() {
        return Err(SoftAesErr::AADInvalid.into());
    }

    let bad_tag = match bad_tag {
        Some(bad_tag) => bad_tag,
        None => {
            // No bad tag, so do the whole computation ourselves.
            return aes_gcm(
                engine,
                encrypt,
                partial_last_input_block,
                key,
                iv,
                partial_aad.unwrap_or(&[]),
                output,
            );
        }
    };

    if aad_len % 32 == 0 && text_len % 16 == 0 && partial_last_input_block.is_empty() {
        // no correction necessary
        return Ok(*bad_tag);
    }

    // prepare the calculations needed for the correction
    let counter = u32::try_from(
        text_len
            .div_ceil(16)
            .checked_add(1)
            .ok_or(SoftAesErr::LengthOverflow)?,
    )
    .map_err(|_| SoftAesErr::LengthOverflow)?;

    let h = compute_h(engine, key)?;
    let j0 = compute_j0(engine, key, iv)?;
    let bad_tag: AesBlock = (*bad_tag).into();
    let padded_aad_len = next_multiple_of_32(aad_len).ok_or(SoftAesErr::LengthOverflow)?;

    if text_len % 16 == 0 {
        // we don't need to compute the last output block, but just do a length correction
        let existing_len_block = len_block(padded_aad_len, text_len);
        let correct_len_block = len_block(aad_len, text_len);
        let correction = (existing_len_block ^ correct_len_block) * h;
        let good_tag = bad_tag ^ correction;
        Ok(good_tag.0.to_be_bytes())
    } else {
        // we need to encrypt or decrypt the last block and do a full correction
        let partial_text_len = text_len - text_len % 16;
        let existing_len_block = len_block(padded_aad_len, partial_text_len);
        let correct_len_block = len_block(aad_len, text_len);

        // compute the last block of output
        let mut last_input_block = [0u8; 16];
        last_input_block[..partial_last_input_block.len()]
            .copy_from_slice(partial_last_input_block);
        let mut output_block = gcm_crypt(engine, key, *iv, counter, &last_input_block)?;
        output.copy_from_slice(&output_block[..output.len()]);

        let final_ciphertext_block = if encrypt {
            &mut output_block
        } else {
            &mut last_input_block
        };
        // zero out the unused bytes in the final ciphertext block
        final_ciphertext_block[((text_len % 16) as usize)..].fill(0);
        let final_ciphertext_block: AesBlock = (*final_ciphertext_block).into();

        // Performance note: this correction does two GF(2^128) multiplications.

        // apply a linear correction for the bad len block and final ciphertext block
        let len_correction = (existing_len_block ^ final_ciphertext_block) * h;
        // counteract the J0 and apply the length correction
        let correction = j0 ^ len_correction ^ correct_len_block;
        let good_tag = bad_tag ^ correction;
        // finalize the tag
        let good_tag = h * good_tag;
        let good_tag = good_tag ^ j0;

        Ok(good_tag.0.to_be_bytes())
    }
}

/// This will correct a tag if the hardware only computed a tag for an aligned AAD portion and did not
/// encrypt or decrypt the input.
///
/// It returns the correct tag and writes the ciphertext or plaintext to output.
#[allow(clippy::too_many_arguments)]
pub fn gcm_tag_correct_aad_only(
    engine: &SoftAes,
    encrypt: bool,
    key: &[u8],
    iv: &[u8; 12],
    aad_len: u64,
    bad_tag: &[u8; 16],
    input: &[u8],
    output: &mut [u8],
) -> McrResult<[u8; 16]> {
    if input.len() >= 16 {
        return Err(SoftAesErr::LastInputBlockTooLarge.into());
    }
    if output.len() < input.len() {
        return Err(SoftAesErr::InsufficientOutputBufferLength.into());
    }

    // compute the values we'll need
    let h = compute_h(engine, key)?;
    let j0 = compute_j0(engine, key, iv)?;
    let padded_aad_len = next_multiple_of_32(aad_len).ok_or(SoftAesErr::LengthOverflow)?;
    let bad_len = len_block(padded_aad_len, 0);
    let correct_len = len_block(aad_len, input.len() as u64);

    // compute the plaintext or ciphertext
    let mut block = [0u8; 16];
    let text_len = input.len();

    let t: AesBlock = (*bad_tag).into();
    let correct_tag = if text_len > 0 {
        aes_gcm(engine, encrypt, input, key, iv, &[], output)?;
        // grab the ciphertext
        block[..text_len].copy_from_slice(if encrypt {
            &output[..text_len]
        } else {
            &input[..text_len]
        });
        let ciphertext_block: AesBlock = block.into();

        // apply the correction for J_0, the lengths, and the ciphertext block
        let t = t ^ j0 ^ correct_len ^ ((ciphertext_block ^ bad_len) * h);
        // finalize the tag
        (t * h) ^ j0
    } else {
        // no text, just do length correction
        t ^ ((correct_len ^ bad_len) * h)
    };
    Ok(correct_tag.0.to_be_bytes())
}

/// Constructs the length block for GCM from the AAD and text lengths.
fn len_block(aad_len: u64, text_len: u64) -> AesBlock {
    // Avoid overflowing in u64 before widening.
    // GCM encodes both lengths as 64-bit bit-lengths (mod 2^64).
    let mask = u64::MAX as u128;
    let aad_bits = ((aad_len as u128) * 8) & mask;
    let text_bits = ((text_len as u128) * 8) & mask;
    AesBlock((aad_bits << 64) | text_bits)
}

/// Returns the next multiple of 32 greater than or equal to value.
fn next_multiple_of_32(value: u64) -> Option<u64> {
    let rem = value % 32;
    if rem == 0 {
        Some(value)
    } else {
        value.checked_add(32 - rem)
    }
}

/// Represents a 128-bit AES block as a big-endian u128.
#[derive(Clone, Copy, PartialEq, Eq)]
struct AesBlock(u128);

/// The GCM polynomial for GF(2^128) multiplication.
const POLY: AesBlock = AesBlock(0xE1000000000000000000000000000000u128);

impl AesBlock {
    /// Computes a * b in GF(2^128) according to the GCM specification (bit-reversed)
    /// in constant time.
    fn gf2_128_mul(&self, b: AesBlock) -> AesBlock {
        let mut a = *self;
        let mut m = AesBlock(0u128);
        for i in 0..128 {
            let b_bit = ((b >> (127 - i)) & 1.into()).0;
            let b_mask = (1 - b_bit).wrapping_sub(1);
            m ^= a & AesBlock(b_mask);
            let xor_poly = (1 - (a.0 & 1)).wrapping_sub(1);
            a >>= 1;
            a ^= POLY & AesBlock(xor_poly);
        }
        m
    }
}

// Convenience functions for working with AES blocks.

impl core::fmt::Debug for AesBlock {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "{:032x}", self.0)
    }
}

impl core::ops::BitXor for AesBlock {
    type Output = AesBlock;

    fn bitxor(self, rhs: Self) -> Self::Output {
        AesBlock(self.0 ^ rhs.0)
    }
}

impl From<u32> for AesBlock {
    fn from(value: u32) -> Self {
        AesBlock(value as u128)
    }
}

impl From<[u8; 16]> for AesBlock {
    fn from(value: [u8; 16]) -> Self {
        AesBlock(u128::from_be_bytes(value))
    }
}

impl core::ops::BitAnd for AesBlock {
    type Output = AesBlock;

    fn bitand(self, rhs: Self) -> Self::Output {
        AesBlock(self.0 & rhs.0)
    }
}

impl core::ops::Shr<u32> for AesBlock {
    type Output = AesBlock;

    fn shr(self, rhs: u32) -> Self::Output {
        AesBlock(self.0 >> rhs)
    }
}

impl core::ops::Shl<u32> for AesBlock {
    type Output = AesBlock;

    fn shl(self, rhs: u32) -> Self::Output {
        AesBlock(self.0 << rhs)
    }
}

impl core::ops::Sub for AesBlock {
    type Output = AesBlock;

    fn sub(self, rhs: Self) -> Self::Output {
        AesBlock(self.0.wrapping_sub(rhs.0))
    }
}

impl core::ops::BitXorAssign for AesBlock {
    fn bitxor_assign(&mut self, rhs: Self) {
        self.0 ^= rhs.0;
    }
}

impl core::ops::ShrAssign<u32> for AesBlock {
    fn shr_assign(&mut self, rhs: u32) {
        self.0 >>= rhs;
    }
}

impl core::ops::Mul<AesBlock> for AesBlock {
    type Output = AesBlock;

    fn mul(self, rhs: AesBlock) -> AesBlock {
        self.gf2_128_mul(rhs)
    }
}

/// Computes the GCM H value from the AES key.
fn compute_h(engine: &SoftAes, key: &[u8]) -> McrResult<AesBlock> {
    let (ha, hb) = crate::keywrap::aes(true, engine, key, 0, 0)?;
    Ok(AesBlock(
        ((ha.swap_bytes() as u128) << 64) | (hb.swap_bytes() as u128),
    ))
}

/// Compute the XOR of the input with the GCM key stream for a counter.
fn gcm_crypt(
    engine: &SoftAes,
    key: &[u8],
    iv: [u8; 12],
    counter: u32,
    input: &[u8; 16],
) -> McrResult<[u8; 16]> {
    let mut block = [0u8; 16];
    block[..12].copy_from_slice(&iv);
    block[12..].copy_from_slice(&counter.to_be_bytes());
    engine.encrypt(key, &mut block)?;
    for i in 0..16 {
        block[i] ^= input[i];
    }
    Ok(block)
}

/// Performs AES-GCM encryption or decryption on the input, producing the output and tag.
fn aes_gcm(
    engine: &SoftAes,
    encrypt: bool,
    input: &[u8],
    key: &[u8],
    iv: &[u8; 12],
    aad: &[u8],
    mut output: &mut [u8],
) -> McrResult<[u8; 16]> {
    if output.len() < input.len() {
        return Err(SoftAesErr::InsufficientOutputBufferLength.into());
    }
    let mut running_tag = gcm_aad_compute(engine, key, aad)?;

    let h = compute_h(engine, key)?;

    for (counter, block) in input.chunks(16).enumerate() {
        let mut input_block = [0u8; 16];
        input_block[..block.len()].copy_from_slice(block);

        let counter = gcm_counter_from_block_index(counter)?;

        let mut output_block = gcm_crypt(engine, key, *iv, counter, &input_block)?;
        // zero any extra bytes
        output_block[block.len()..].fill(0);

        if encrypt {
            // for encryption, we need to use the output (ciphertext) block to update the tag
            let ciphertext_block: AesBlock = output_block.into();
            running_tag ^= ciphertext_block;
            running_tag = running_tag * h;
        } else {
            // for decryption, we need to use the input (ciphertext) block to update the tag
            let ciphertext_block: AesBlock = input_block.into();
            running_tag ^= ciphertext_block;
            running_tag = running_tag * h;
        }
        output[0..block.len()].copy_from_slice(&output_block[..block.len()]);
        output = &mut output[block.len()..];
    }
    running_tag ^= len_block(aad.len() as u64, input.len() as u64);
    running_tag = running_tag * h;
    running_tag ^= compute_j0(engine, key, iv)?;
    Ok(running_tag.0.to_be_bytes())
}

/// Computes the GHASH of the GCM AAD computation.
fn gcm_aad_compute(engine: &SoftAes, key: &[u8], aad: &[u8]) -> McrResult<AesBlock> {
    let h = compute_h(engine, key)?;
    let mut ghash = AesBlock(0u128);

    for chunk in aad.chunks(16) {
        let mut block = [0u8; 16];
        block[..chunk.len()].copy_from_slice(chunk);
        let block: AesBlock = block.into();
        ghash = (ghash ^ block) * h;
    }
    Ok(ghash)
}

/// Computes the J0 (encrypted initial counter) value from the IV.
fn compute_j0(engine: &SoftAes, key: &[u8], iv: &[u8; 12]) -> McrResult<AesBlock> {
    let mut j0 = [0u8; 16];
    j0[..12].copy_from_slice(iv);
    j0[15] = 1;
    engine.encrypt(key, &mut j0)?;
    Ok(j0.into())
}

/// Computes the GCM counter value from a block index, checking for overflow.
fn gcm_counter_from_block_index(block_index: usize) -> Result<u32, SoftAesErr> {
    u32::try_from(block_index)
        .map_err(|_| SoftAesErr::LengthOverflow)?
        .checked_add(2)
        .ok_or(SoftAesErr::LengthOverflow)
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::SoftAes;
    use alloc::vec;
    use alloc::vec::Vec;
    use core::convert::TryInto;
    use hex_literal::hex;

    #[test]
    fn test_compute_h() {
        let engine = SoftAes::new();
        let key = [0u8; 32];
        let h = compute_h(&engine, &key).unwrap();
        assert_eq!(h.0, 0xdc95c078a2408989ad48a21492842087u128);
    }

    #[test]
    fn test_gf2_128_mul() {
        let a = AesBlock(0x66e94bd4ef8a2c3b884cfa59ca342b2eu128);
        let b = AesBlock(0x5e2ec746917062882c85b0685353de37u128);
        let result = a * b;
        assert_eq!(result.0, 0xf38cbb1ad69223dcc3457ae5b6b0f885u128);
    }

    fn pad_text(text: &[u8]) -> Vec<u8> {
        if text.len() % 16 == 0 {
            return text.to_vec();
        }
        let mut padded = text.to_vec();
        let padding_needed = (16 - (text.len() % 16)) % 16;
        padded.extend(vec![0u8; padding_needed]);
        padded
    }

    fn pad_aad(aad: &[u8]) -> Vec<u8> {
        if aad.len() % 32 == 0 {
            return aad.to_vec();
        }
        let mut result = aad.to_vec();

        if aad.len() % 32 <= 16 {
            let mut padded = vec![0u8; 16];
            padded.extend_from_slice(aad);
            result = padded;
        }

        let padding_needed = (32 - (result.len() % 32)) % 32;
        result.extend(vec![0u8; padding_needed]);
        result
    }

    #[test]
    fn test_gcm_counter_overflow_try_from() {
        // On 32-bit targets, `usize` can't exceed `u32::MAX`, so this specific overflow
        // can't be constructed.
        if usize::BITS <= 32 {
            return;
        }

        let block_index = (u32::MAX as usize) + 1;
        assert!(matches!(
            gcm_counter_from_block_index(block_index),
            Err(SoftAesErr::LengthOverflow)
        ));
    }

    #[test]
    fn test_gcm_counter_overflow_checked_add() {
        // Conversion succeeds, but +2 overflows u32.
        let block_index = u32::MAX as usize;
        assert!(matches!(
            gcm_counter_from_block_index(block_index),
            Err(SoftAesErr::LengthOverflow)
        ));
    }

    #[test]
    fn test_gcm_crypt() {
        let engine = SoftAes::new();
        let plaintext = &hex!("b526ba1050177d05b0f72f8d67");
        let key = &hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");
        let padded_plaintext = pad_text(plaintext);
        let ciphertext =
            gcm_crypt(&engine, key, iv, 2, &padded_plaintext.try_into().unwrap()).unwrap();
        let expected_ciphertext = hex!("8b29e66e924ecae84f6d8f7d68379149");
        assert_eq!(expected_ciphertext, ciphertext);
    }

    #[test]
    fn test_aes_gcm() {
        let engine = SoftAes::new();
        let plaintext = hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c8900");
        let aad = hex!("6e43784a91851a77667a02198e28dc32");
        let key = hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");
        let expected_ciphertext =
            hex!("ec366384b69a6b7eed12a0f3bb7af1e6129f53575ac56e36e5cd4bb090f6");
        let expected_tag = hex!("1c2612db2afa9a994d5da965ff4aa99b");

        let mut ciphertext = vec![0u8; plaintext.len()];
        let tag = aes_gcm(&engine, true, &plaintext, &key, &iv, &aad, &mut ciphertext).unwrap();

        assert_eq!(&ciphertext, &expected_ciphertext);
        assert_eq!(&tag, &expected_tag);
    }

    #[test]
    fn test_gcm_aad_only_tag_encrypt_known() {
        let engine = SoftAes::new();
        let plaintext = &hex!("881dc6c7a5d4509f3c4bd2daab08f1");
        let iv = &hex!("bd587321566c7f1a5dd8652d");
        let key = &hex!("5fe01c4baf01cbe07796d5aaef6ec1f45193a98a223594ae4f0ef4952e82e330");
        let aad: &[u8; 16] = &hex!("9013617817dda947e135ee6dd3653382");
        let expected_ciphertext = &hex!("16e375b4973b339d3f746c1c5a568b");
        let expected_tag = &hex!("71fdf867369d0d4933b23253b88ed383");

        let mut ciphertext = [0u8; 15];
        let tag = aes_gcm(&engine, true, plaintext, key, iv, aad, &mut ciphertext).unwrap();
        assert_eq!(&tag, expected_tag);
        assert_eq!(&ciphertext, expected_ciphertext);

        let engine_tag = &hex!("b95111a5fdcede40ff7ae18a4accedfe");
        let correct_tag = gcm_tag_correct_aad_only(
            &engine,
            true,
            key,
            iv,
            aad.len() as u64,
            engine_tag,
            plaintext,
            &mut ciphertext,
        )
        .unwrap();
        assert_eq!(&correct_tag, expected_tag);
    }

    #[test]
    fn test_gcm_aad_only_tag_encrypt_exhaustive() {
        let engine = SoftAes::new();
        let text = [0xffu8; 15];
        let aad = [0xffu8; 44];
        let key = [0xffu8; 32];
        let iv = [0xffu8; 12];

        for encrypt in [false, true] {
            for text_len in 0..=text.len() {
                for aad_len in 0..=aad.len() {
                    let text = &text[..text_len];
                    let aad = &aad[..aad_len];
                    let padded_aad = pad_aad(aad);

                    let mut expected_output = vec![0u8; text.len()];
                    let expected_tag =
                        aes_gcm(&engine, encrypt, text, &key, &iv, aad, &mut expected_output)
                            .unwrap();

                    let bad_tag =
                        aes_gcm(&engine, encrypt, &[], &key, &iv, &padded_aad, &mut []).unwrap();

                    let mut output = vec![0u8; text.len()];
                    let tag = gcm_tag_correct_aad_only(
                        &engine,
                        encrypt,
                        &key,
                        &iv,
                        aad.len() as u64,
                        &bad_tag,
                        text,
                        &mut output,
                    )
                    .unwrap();

                    assert_eq!(expected_output, output.as_slice());
                    assert_eq!(expected_tag, tag);
                }
            }
        }
    }

    #[test]
    fn test_correct_tag_pad_both() {
        let engine = SoftAes::new();
        // 30-byte plaintext
        let plaintext = &hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c8900");
        // 16-byte AAD
        let aad = &hex!("6e43784a91851a77667a02198e28dc32");
        // 256-bit key
        let key = &hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        // 96-bit nonce
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");
        let expected_tag = hex!("1c2612db2afa9a994d5da965ff4aa99b");

        let padded_plaintext = pad_text(plaintext);
        let padded_aad = pad_aad(aad);
        assert_eq!(
            padded_plaintext,
            hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c89000000")
        );
        assert_eq!(
            padded_aad,
            hex!("000000000000000000000000000000006e43784a91851a77667a02198e28dc32")
        );
        let expected_bad_tag = hex!("749c4f379ae743832ded2dc11976ea81");

        let bad_tag = aes_gcm(
            &engine,
            true,
            &plaintext[0..16],
            key,
            &iv,
            &padded_aad,
            &mut vec![0u8; plaintext.len()],
        )
        .unwrap();

        assert_eq!(expected_bad_tag, bad_tag);

        let mut new_output = vec![0u8; plaintext.len() - 16];

        let tag = gcm_tag_correct(
            &engine,
            true,
            key,
            &iv,
            aad.len() as u64,
            plaintext.len() as u64,
            None,
            Some(&bad_tag),
            &plaintext[16..],
            &mut new_output,
        )
        .unwrap();

        assert_eq!(expected_tag, tag);
    }

    fn chop_text(text: &[u8]) -> Vec<u8> {
        if text.len() % 16 == 0 {
            text.to_vec()
        } else {
            text[..(text.len() - text.len() % 16)].to_vec()
        }
    }

    #[test]
    fn test_correct_tag_pad_text() {
        let engine = SoftAes::new();
        // 30-byte plaintext
        let plaintext = &hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c8900");
        // 32-byte AAD
        let aad = &hex!("6e43784a91851a77667a02198e28dc326e43784a91851a77667a02198e28dc32");
        // 256-bit key
        let key = &hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        // 96-bit nonce
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");

        let mut expected_ciphertext = vec![0u8; plaintext.len()];
        let expected_tag = aes_gcm(
            &engine,
            true,
            plaintext,
            key,
            &iv,
            aad,
            &mut expected_ciphertext,
        )
        .unwrap();

        let padded_aad = pad_aad(aad);
        let chopped_plaintext = chop_text(plaintext);
        let leftover_plaintext = &plaintext[chopped_plaintext.len()..];
        let mut ciphertext = vec![0u8; chopped_plaintext.len()];

        let bad_tag = aes_gcm(
            &engine,
            true,
            &chopped_plaintext,
            key,
            &iv,
            &padded_aad,
            &mut ciphertext,
        )
        .unwrap();

        let mut final_output_block = vec![0u8; leftover_plaintext.len()];

        let tag = gcm_tag_correct(
            &engine,
            true,
            key,
            &iv,
            aad.len() as u64,
            plaintext.len() as u64,
            None,
            Some(&bad_tag),
            leftover_plaintext,
            &mut final_output_block,
        )
        .unwrap();

        ciphertext.extend_from_slice(&final_output_block);
        assert_eq!(expected_ciphertext, ciphertext.as_slice());
        assert_eq!(expected_tag, tag);
    }

    #[test]
    fn test_correct_tag_pad_aad() {
        let engine = SoftAes::new();
        // 32-byte plaintext
        let plaintext = &hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c8900fffe");
        // 15-byte AAD
        let aad = &hex!("6e43784a91851a77667a02198e28dc");
        // 256-bit key
        let key = &hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        // 96-bit nonce
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");

        let mut expected_ciphertext = vec![0u8; plaintext.len()];
        let expected_tag = aes_gcm(
            &engine,
            true,
            plaintext,
            key,
            &iv,
            aad,
            &mut expected_ciphertext,
        )
        .unwrap();

        let padded_aad = pad_aad(aad);
        let chopped_plaintext = chop_text(plaintext);
        let leftover_plaintext = &plaintext[chopped_plaintext.len()..];
        let mut ciphertext = vec![0u8; chopped_plaintext.len()];

        let bad_tag = aes_gcm(
            &engine,
            true,
            &chopped_plaintext,
            key,
            &iv,
            &padded_aad,
            &mut ciphertext,
        )
        .unwrap();

        let mut final_output_block = vec![0u8; leftover_plaintext.len()];

        let tag = gcm_tag_correct(
            &engine,
            true,
            key,
            &iv,
            aad.len() as u64,
            plaintext.len() as u64,
            None,
            Some(&bad_tag),
            leftover_plaintext,
            &mut final_output_block,
        )
        .unwrap();

        ciphertext.extend_from_slice(&final_output_block);
        assert_eq!(expected_ciphertext, ciphertext.as_slice());
        assert_eq!(expected_tag, tag);
    }

    #[test]
    fn test_correct_tag_no_padding() {
        let engine = SoftAes::new();
        // 32-byte plaintext
        let plaintext = &hex!("d2393ffa74c3dc9312880003b44d60af805b7fe6af576d4d6d42786c8900fffe");
        // 32-byte AAD
        let aad = &hex!("6e43784a91851a77667a02198e28dc326e43784a91851a77667a02198e28dc32");
        // 256-bit key
        let key = &hex!("dad89d9be9bba138cdcf8752c45b579d7e27c3dbb40f53e771dd8cfd500aa2d5");
        // 96-bit nonce
        let iv = hex!("cfb2aec82cfa6c7d89ee72ff");

        let mut expected_ciphertext = vec![0u8; plaintext.len()];
        let expected_tag = aes_gcm(
            &engine,
            true,
            plaintext,
            key,
            &iv,
            aad,
            &mut expected_ciphertext,
        )
        .unwrap();

        let mut _ignore = vec![];
        let tag = gcm_tag_correct(
            &engine,
            true,
            key,
            &iv,
            aad.len() as u64,
            plaintext.len() as u64,
            None,
            Some(&expected_tag),
            &[],
            &mut _ignore,
        )
        .unwrap();

        assert_eq!(expected_tag, tag);
    }

    #[test]
    fn test_correct_tag_with_nist_test_vectors() {
        let engine = SoftAes::new();
        // 51-byte plaintext
        let plaintext = &hex!("881dc6c7a5d4509f3c4bd2daab08f165ddc204489aa8134562a4eac3d0bcad7965847b102733bb63d1e5c598ece0c3e5dadddd");
        // 16-byte AAD
        let aad = &hex!("9013617817dda947e135ee6dd3653382");
        // 256-bit key
        let key = &hex!("5fe01c4baf01cbe07796d5aaef6ec1f45193a98a223594ae4f0ef4952e82e330");
        // 96-bit nonce
        let iv = hex!("bd587321566c7f1a5dd8652d");

        let mut expected_ciphertext = vec![0u8; plaintext.len()];
        let expected_tag = aes_gcm(
            &engine,
            true,
            plaintext,
            key,
            &iv,
            aad,
            &mut expected_ciphertext,
        )
        .unwrap();

        let padded_aad = pad_aad(aad);
        let chopped_plaintext = chop_text(plaintext);
        let leftover_plaintext = &plaintext[chopped_plaintext.len()..];
        let mut ciphertext = vec![0u8; chopped_plaintext.len()];

        let bad_tag = aes_gcm(
            &engine,
            true,
            &chopped_plaintext,
            key,
            &iv,
            &padded_aad,
            &mut ciphertext,
        )
        .unwrap();

        let mut final_output_block = vec![0u8; leftover_plaintext.len()];

        let tag = gcm_tag_correct(
            &engine,
            true,
            key,
            &iv,
            aad.len() as u64,
            plaintext.len() as u64,
            None,
            Some(&bad_tag),
            leftover_plaintext,
            &mut final_output_block,
        )
        .unwrap();

        ciphertext.extend_from_slice(&final_output_block);
        assert_eq!(expected_ciphertext, ciphertext.as_slice());
        assert_eq!(expected_tag, tag);
    }

    #[test]
    fn test_correct_tag_exhaustive() {
        let engine = SoftAes::new();
        let text = [0xffu8; 44];
        let aad = [0xffu8; 44];
        let key = [0xffu8; 32];
        let iv = [0xffu8; 12];

        for encrypt in [false, true] {
            for text_len in 0..text.len() {
                for aad_len in 0..aad.len() {
                    let text = &text[..text_len];
                    let aad = &aad[..aad_len];
                    let padded_aad = pad_aad(aad);
                    let chopped_text = chop_text(text);

                    let mut expected_output = vec![0u8; text.len()];
                    let expected_tag =
                        aes_gcm(&engine, encrypt, text, &key, &iv, aad, &mut expected_output)
                            .unwrap();

                    let mut output = vec![0u8; chopped_text.len()];
                    let bad_tag = match (text_len, aad_len) {
                        (0..=15, 0..=31) => None,
                        _ => Some(
                            aes_gcm(
                                &engine,
                                encrypt,
                                &chopped_text,
                                &key,
                                &iv,
                                &padded_aad,
                                &mut output,
                            )
                            .unwrap(),
                        ),
                    };

                    let mut new_output = vec![0u8; text.len() - chopped_text.len()];
                    let tag = gcm_tag_correct(
                        &engine,
                        encrypt,
                        &key,
                        &iv,
                        aad.len() as u64,
                        text.len() as u64,
                        if bad_tag.is_none() { Some(aad) } else { None },
                        bad_tag.as_ref(),
                        &text[chopped_text.len()..],
                        &mut new_output,
                    )
                    .unwrap();

                    output.extend_from_slice(&new_output);
                    assert_eq!(expected_output, output.as_slice());
                    assert_eq!(expected_tag, tag);
                }
            }
        }
    }
}
