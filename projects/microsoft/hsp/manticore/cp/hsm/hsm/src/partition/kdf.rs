// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::KbkdfInfo;
use mcr_crypto_sha::KbkdfInputData;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::ShaTrait;
use mcr_crypto_sha::KBKDF_MAX_INPUT_BUF_SIZE;
use mcr_crypto_sha::KBKDF_MAX_LABEL_SIZE;

use part::HmacHashAlgorithm;

use super::*;
use crate::env::HsmEnvTrait;
use crate::error::HsmResult;

// Output hash size for SHA512
const MAX_HASH_SIZE: usize = 64;
// Data size for Secret521 key type
const MAX_SECRET_SIZE: usize = 80;
// Maximum length for HKDF info
const MAX_INFO_SIZE: usize = 32;
// Maximum buffer size for sha_zc op
const MAX_SHA_OUT_BUFFERSIZE: usize = 64;

impl<E: HsmEnvTrait> Partition<E> {
    /// TODO (task 2013440): duplicate definition in session/app_sess
    /// Helper to execute HKDF operation for data blob
    /// HKDF is implemented per the standard at: https://www.rfc-editor.org/rfc/rfc5869
    /// Note: output needs to be GSRAM array allocated up to out_len + "digest_size_hw"
    pub(super) fn hkdf_impl(
        &self,
        secret_key_blob: &[u8],
        salt: &[u8],
        info: &[u8],
        hash_algo: DdiHashAlgorithm,
        output: &mut [u8],
        out_len: u16,
    ) -> HsmResult<SecureByteVec> {
        // Get hash output len via ShaType number values
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo.try_into()?;
        let sha_mode: ShaMode = hmac_hash_algo.into();
        let sha_type: ShaType = sha_mode.into();
        let hash_len = sha_type as usize;

        let hash_buffer_len = sha_type.get_digest_size_hw();

        // Sanity checks
        if info.len() > MAX_INFO_SIZE {
            Err(HsmErr::KeyDeriveFailed)?
        }

        if hash_len > MAX_HASH_SIZE {
            Err(HsmErr::KeyDeriveFailed)?
        }

        if out_len > MAX_SECRET_SIZE as u16 {
            Err(HsmErr::KeyDeriveFailed)?
        }

        if output.len() < hash_buffer_len + out_len as usize {
            Err(HsmErr::KeyDeriveFailed)?
        }

        // First: Extract operation. PRK = pseudo random key
        let prk_gsram = self.dma_alloc(hash_buffer_len)?;
        let prk_mborbytearray = MborByteArray::<MAX_SHA_OUT_BUFFERSIZE>::new_with_len(
            prk_gsram.as_ref().as_ptr(),
            hash_buffer_len,
        );
        self.hmac_impl(
            salt,
            secret_key_blob,
            hash_algo,
            &mut (&prk_mborbytearray).into(),
        )?;

        let prk = &prk_gsram.as_ref()[..hash_len];

        // Second: Expand operation
        // The output OKM is calculated as follows:

        // N = ceil(L/HashLen)
        // T = T(1) | T(2) | T(3) | ... | T(N)
        // OKM = first L octets of T

        // where:
        // T(0) = empty string (zero length)
        // T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
        // T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
        // T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
        // ...
        // N = ceil(L/HashLen)
        let iter_num = (out_len as usize).div_ceil(hash_len);
        let mut prev_t_len = 0;
        let mut output_len = 0;

        let mut prev_t: &[u8] = &[0u8; 0];
        let mut msg: SecureByteArray<{ MAX_HASH_SIZE + MAX_INFO_SIZE + 1 }> =
            [0u8; MAX_HASH_SIZE + MAX_INFO_SIZE + 1].into();

        for i in 1..(iter_num + 1) as u8 {
            msg[..prev_t_len].copy_from_slice(&prev_t[..prev_t_len]);
            msg[prev_t_len..prev_t_len + info.len()].copy_from_slice(info);
            msg[prev_t_len + info.len()] = i;

            let hmac_output_mborbytearray = MborByteArray::<MAX_SHA_OUT_BUFFERSIZE>::new_with_len(
                output[output_len..].as_ptr(),
                hash_buffer_len,
            );
            self.hmac_impl(
                prk,
                &msg[..prev_t_len + info.len() + 1],
                hash_algo,
                &mut (&hmac_output_mborbytearray).into(),
            )?;

            prev_t = &output[output_len..output_len + hash_len];
            prev_t_len = hash_len;

            output_len += prev_t_len;
        }

        // Sanity checks
        if output_len < out_len as usize {
            Err(HsmErr::KeyDeriveFailed)?
        }

        // Return data
        Ok(output[..out_len as usize].into())
    }

    /// TODO: (task 2013440): Move kdf/hmac/sha implementation to Partition
    /// Helper to execute KBKDF Counter with HMAC for data blob
    /// KBKDF is implemented per the standard at: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-108r1-upd1.pdf
    ///
    /// # Notes
    ///
    /// Output needs to be GSRAM array allocated up to out_len + "digest_size_hw"
    pub(super) fn kbkdf_impl(
        &self,
        secret_key_blob: &[u8],
        label: &[u8],
        context: &[u8],
        hash_algo: DdiHashAlgorithm,
        output: &mut [u8],
        out_len: u16,
    ) -> HsmResult<()> {
        // Get hash output len via ShaType number values
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo.try_into()?;
        let sha_mode: ShaMode = hmac_hash_algo.into();

        // allocate working buffer for hmac
        let input_buffer_gsram = self.dma_alloc(KBKDF_MAX_INPUT_BUF_SIZE)?;
        let input_buffer_mborbytearray = MborByteArray::<KBKDF_MAX_INPUT_BUF_SIZE>::new_with_len(
            input_buffer_gsram.as_ref().as_ptr(),
            KBKDF_MAX_INPUT_BUF_SIZE,
        );
        let input_buffer = &mut IoMemRange::from(&input_buffer_mborbytearray);

        // Sanity checks
        if label.len() > KBKDF_MAX_LABEL_SIZE {
            Err(HsmErr::KbkdfInvalidInputParam)?
        }

        if context.len() > KBKDF_MAX_LABEL_SIZE {
            Err(HsmErr::KbkdfInvalidInputParam)?
        }

        let kbkdf_info = KbkdfInfo {
            key: secret_key_blob,
            input_data: KbkdfInputData::ConcatData { label, context },
            out_len,
        };

        self.state
            .env()
            .sha()
            .kbkdf_counter_hmac(kbkdf_info, sha_mode, input_buffer, output)
            .map_err(|_| HsmErr::KbkdfError)?;

        Ok(())
    }
}
