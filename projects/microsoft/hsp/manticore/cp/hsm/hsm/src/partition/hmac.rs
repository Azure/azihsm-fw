// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::ShaTrait;
use mcr_crypto_sha::HMAC_MAX_INPUT_BUF_SIZE;
use part::HmacHashAlgorithm;

use super::*;

use crate::env::HsmEnvTrait;
use crate::error::HsmResult;

impl<E: HsmEnvTrait> Partition<E> {
    /// TODO (task 2013440): duplicate definition in session/app_sess
    /// Helper to execute hmac operation for data blob
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    pub(crate) fn hmac_impl(
        &self,
        secret_key_blob: &[u8],
        msg: &[u8],
        hash_algo: DdiHashAlgorithm,
        output_buffer: &mut IoMemRange,
    ) -> HsmResult<()> {
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo.try_into()?;
        let sha_mode: ShaMode = hmac_hash_algo.into();
        let sha_type: ShaType = sha_mode.into();
        let sha_block_size: usize = sha_type.into();
        let sha_digest_size = sha_type as usize;
        let output_len = output_buffer.len();
        let hw_sha_digest_len = sha_type.get_digest_size_hw();

        let alloc_size = if msg.len() > sha_digest_size {
            msg.len() + sha_block_size
        } else {
            sha_digest_size + sha_block_size
        };

        let input_buffer_gsram = self.dma_alloc(alloc_size)?;
        let input_buffer_mborbytearray = MborByteArray::<HMAC_MAX_INPUT_BUF_SIZE>::new_with_len(
            input_buffer_gsram.as_ref().as_ptr(),
            alloc_size,
        );
        let input_buffer = &mut IoMemRange::from(&input_buffer_mborbytearray);

        // HS-SHA hardware expects the output buffer size of 64-bytes for both SHA384 and SHA512,
        // But the host expects the HMAC output to be of length 48-bytes for HMAC384. To cater this
        // hardware engine specific need, create an interim buffer with the length that SHA
        // hardware expects and then copy the expected output data length from the interim buffer to
        // the host output buffer.
        let interim_output_buffer_gsram = self.dma_alloc(hw_sha_digest_len)?;
        let interim_output_buffer_mborbytearray = MborByteArray::<64>::new_with_len(
            interim_output_buffer_gsram.as_ref().as_ptr(),
            hw_sha_digest_len,
        );
        let interim_output_buffer = &mut IoMemRange::from(&interim_output_buffer_mborbytearray);

        self.state
            .env()
            .sha()
            .hmac(
                secret_key_blob,
                msg,
                sha_mode,
                input_buffer,
                interim_output_buffer,
            )
            .map_err(|_| HsmErr::HmacComputeFailed)?;

        // Copy from interim buffer to the DMA output buffer with expected length by the host
        output_buffer
            .slice_mut()
            .copy_from_slice(&interim_output_buffer.slice()[..output_len]);

        Ok(())
    }
}
