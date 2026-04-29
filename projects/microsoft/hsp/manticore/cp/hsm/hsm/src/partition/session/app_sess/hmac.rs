// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use mcr_crypto_sha::ShaMode;
use mcr_crypto_sha::ShaTrait;

pub(crate) enum HmacHashAlgorithm {
    // SHA-1
    Sha1,

    // SHA-256
    Sha256,

    // SHA-384
    Sha384,

    // SHA-512
    Sha512,
}

impl TryFrom<DdiHashAlgorithm> for HmacHashAlgorithm {
    type Error = HsmErr;

    fn try_from(algorithm: DdiHashAlgorithm) -> Result<Self, Self::Error> {
        match algorithm {
            DdiHashAlgorithm::Sha1 => Ok(HmacHashAlgorithm::Sha1),
            DdiHashAlgorithm::Sha256 => Ok(HmacHashAlgorithm::Sha256),
            DdiHashAlgorithm::Sha384 => Ok(HmacHashAlgorithm::Sha384),
            DdiHashAlgorithm::Sha512 => Ok(HmacHashAlgorithm::Sha512),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl From<HmacHashAlgorithm> for ShaMode {
    fn from(algorithm: HmacHashAlgorithm) -> Self {
        match algorithm {
            HmacHashAlgorithm::Sha1 => ShaMode::Sha1,
            HmacHashAlgorithm::Sha256 => ShaMode::Sha256,
            HmacHashAlgorithm::Sha384 => ShaMode::Sha384,
            HmacHashAlgorithm::Sha512 => ShaMode::Sha512,
        }
    }
}

impl<E: HsmEnvTrait> UserSession<E> {
    /// Helper to execute hmac operation for data blob
    /// HMAC is calculated as per the standard at: https://www.rfc-editor.org/rfc/rfc2104.
    pub(crate) fn hmac_impl(
        &self,
        key_blob: &[u8],
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
        let mut input_buffer: IoMemRange = (input_buffer_gsram.as_ref()).into();

        // HS-SHA hardware expects the output buffer size of 64-bytes for both SHA384 and SHA512,
        // But the host expects the HMAC output to be of length 48-bytes for HMAC384. To cater this
        // hardware engine specific need, create an interim buffer with the length that SHA
        // hardware expects and then copy the expected output data length from the interim buffer to
        // the host output buffer.
        let interim_output_buffer_gsram = self.dma_alloc(hw_sha_digest_len)?;
        let mut interim_output_buffer: IoMemRange = (interim_output_buffer_gsram.as_ref()).into();

        self.state
            .env()
            .sha()
            .hmac(
                key_blob,
                msg,
                sha_mode,
                &mut input_buffer,
                &mut interim_output_buffer,
            )
            .map_err(|_| HsmErr::HmacComputeFailed)?;

        // Copy from interim buffer to the DMA output buffer with expected length by the host
        output_buffer
            .slice_mut()
            .copy_from_slice(&interim_output_buffer.slice()[..output_len]);

        Ok(())
    }

    /// Get the HMAC Key.
    pub(crate) fn hmac_key(&self, key_id: KeyId) -> HsmResult<HmacKey> {
        let key = self.state.vault().hmac_key(
            self.app_vault_id(),
            self.id(),
            key_id,
            Some(HmacKeyUsage::SignVerify),
        )?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }

    /// Get the Variable Length HMAC Key.
    pub(crate) fn var_hmac_key(&self, key_id: KeyId) -> HsmResult<VarLenHmacShaKey> {
        let key = self.state.vault().var_hmac_key(
            self.app_vault_id(),
            self.id(),
            key_id,
            Some(HmacKeyUsage::SignVerify),
        )?;

        if key.disabled()? {
            Err(HsmErr::KeyNotFound)?
        }

        Ok(key)
    }
}

#[cfg(test)]
mod tests {

    use hmac::Hmac;
    use hmac::Mac;
    use mcr_crypto_sha::ShaMode;
    use mcr_ddi_mbor::MborByteArray;
    use mcr_ddi_types::DdiApiRev;
    use mcr_ddi_types::DdiHashAlgorithm;
    use mcr_types::*;
    use openssl::hash::MessageDigest;
    use openssl::pkey::PKey;
    use openssl::sign::Signer;
    use sha2::Sha256;
    use sha2::Sha384;
    use sha2::Sha512;

    use super::CmdScheduler;
    use crate::fsm::ComboFsm;
    use crate::mock::MockDmaAlloc;
    use crate::mock::MockDmaHeap;
    use crate::mock::MockEnv;
    use crate::mock::MockHal;
    use crate::mock::MockIpcMessageChannel;
    use crate::mock::MockPka;
    use crate::mock::MockSha;
    use crate::partition::session::app_sess::hmac::HmacHashAlgorithm;
    use crate::partition::PartEnv;
    use crate::partition::PartState;
    use crate::partition::ShaType;
    use crate::partition::UserSession;
    use crate::recorder::HsmFsmEventRecorder;

    fn test_hmac(key: &[u8], msg: &[u8], hash_algo: DdiHashAlgorithm, expected_hmac: &[u8]) {
        // Set up session:
        let mut pka = MockPka::new();
        pka.expect_clone().times(1).returning(MockPka::new);

        const TOTAL_TABLE_LEN: usize = 17 * 1024;
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];

        let mut hal = MockHal::new();
        hal.expect_pka().once().return_const(vec![pka]);
        hal.expect_vault_addr()
            .return_const(table_memory.as_ptr() as usize);
        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_fp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsp_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut mock_ipc_message_channel = MockIpcMessageChannel::new();
        mock_ipc_message_channel
            .expect_clone()
            .once()
            .returning(MockIpcMessageChannel::new);

        hal.expect_hsm_to_admin_ipc_channel()
            .once()
            .return_const(mock_ipc_message_channel);

        let mut sha = MockSha::new();
        let num_additional_sha = 0;
        sha.expect_hmac().once().returning(
            move |key: &[u8], data: &[u8], sha_mode: ShaMode, _, out_buf: &mut IoMemRange| {
                // Use the rust crypto hmac implementation here. This is to unit test the flow
                let output = match sha_mode {
                    ShaMode::Sha1 => {
                        // Create key object
                        let pkey = PKey::hmac(key).expect("Failed to create key");
                        // Create signer with SHA1
                        let mut signer = Signer::new(MessageDigest::sha1(), &pkey)
                            .expect("Failed to create signer");
                        signer.update(data).expect("Failed to update");
                        signer.sign_to_vec().expect("Failed to sign")
                    }
                    ShaMode::Sha256 => {
                        type HmacSha256 = Hmac<Sha256>;
                        let mut mac = HmacSha256::new_from_slice(key).expect("Unexpected error");
                        mac.update(data);
                        mac.finalize().into_bytes().to_vec()
                    }
                    ShaMode::Sha384 => {
                        type HmacSha384 = Hmac<Sha384>;
                        let mut mac = HmacSha384::new_from_slice(key).expect("Unexpected error");
                        mac.update(data);
                        mac.finalize().into_bytes().to_vec()
                    }
                    ShaMode::Sha512 => {
                        type HmacSha512 = Hmac<Sha512>;
                        let mut mac = HmacSha512::new_from_slice(key).expect("Unexpected error");
                        mac.update(data);
                        mac.finalize().into_bytes().to_vec()
                    }
                };

                let sha_digest_size = ShaType::from(sha_mode) as usize;
                out_buf.slice_mut()[..sha_digest_size].copy_from_slice(&output[..sha_digest_size]);

                Ok(())
            },
        );

        hal.expect_sha()
            .times(1 + num_additional_sha..)
            .return_const(sha);

        let mut heap = MockDmaHeap::new();
        heap.expect_allocate()
            .times(1..)
            .returning(|s| Some(MockDmaAlloc::new(s)));
        hal.expect_dma_heap().times(1..).return_const(heap);

        hal.expect_clone().once().returning(move || {
            let mut hal = MockHal::new();

            let part_persistent_store_memory = [0u8; 2048 * 65];
            hal.expect_part_persistent_store_addr()
                .return_const(part_persistent_store_memory.as_ptr() as usize);

            hal
        });

        let cmd_scheduler =
            CmdScheduler::<ComboFsm<MockEnv>>::new(128, 1, HsmFsmEventRecorder::default());
        let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler);
        let state = PartState::new(PcieFunction(0), env);

        let app_session = UserSession::new(DdiApiRev { major: 1, minor: 0 }, 10, state);

        // Call app_session HMAC
        let hmac_hash_algo: HmacHashAlgorithm = hash_algo
            .try_into()
            .expect("Failed to convert hash algorithm");
        let sha_mode: ShaMode = hmac_hash_algo.into();
        let sha_type: ShaType = sha_mode.into();
        let sha_out_size: usize = sha_type as usize;

        const MAX_SHA_OUT_BUFFERSIZE: usize = 64;
        // Create the output buffer for the expected MAC length given a SHA algorithm
        let mcr_out = vec![0u8; sha_out_size];
        let mcr_out_mborbytearray =
            MborByteArray::<MAX_SHA_OUT_BUFFERSIZE>::new_with_len(mcr_out.as_ptr(), sha_out_size);

        app_session
            .hmac_impl(key, msg, hash_algo, &mut (&mcr_out_mborbytearray).into())
            .unwrap();

        // Compare outputs
        assert_eq!(expected_hmac, &mcr_out[..sha_out_size]);
    }

    #[test]
    fn test_hmac_sha256() {
        // Test vector source: https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/mac/hmactestvectors.zip
        // [L=32]; sha256
        // Count = 30
        // Klen = 40
        // Tlen = 32
        // Key = 9779d9120642797f1747025d5b22b7ac607cab08e1758f2f3a46c8be1e25c53b8c6a8f58ffefa176
        // Msg = b1689c2591eaf3c9e66070f8a77954ffb81749f1b00346f9dfe0b2ee905dcc288baf4a92de3f4001dd9f44c468c3d07d6c6ee82faceafc97c2fc0fc0601719d2dcd0aa2aec92d1b0ae933c65eb06a03c9c935c2bad0459810241347ab87e9f11adb30415424c6c7f5f22a003b8ab8de54f6ded0e3ab9245fa79568451dfa258e
        // Mac = 769f00d3e6a6cc1fb426a14a4f76c6462e6149726e0dee0ec0cf97a16605ac8b

        let key: [u8; 40] = [
            0x97, 0x79, 0xd9, 0x12, 0x06, 0x42, 0x79, 0x7f, 0x17, 0x47, 0x02, 0x5d, 0x5b, 0x22,
            0xb7, 0xac, 0x60, 0x7c, 0xab, 0x08, 0xe1, 0x75, 0x8f, 0x2f, 0x3a, 0x46, 0xc8, 0xbe,
            0x1e, 0x25, 0xc5, 0x3b, 0x8c, 0x6a, 0x8f, 0x58, 0xff, 0xef, 0xa1, 0x76,
        ];

        let msg: [u8; 128] = [
            0xb1, 0x68, 0x9c, 0x25, 0x91, 0xea, 0xf3, 0xc9, 0xe6, 0x60, 0x70, 0xf8, 0xa7, 0x79,
            0x54, 0xff, 0xb8, 0x17, 0x49, 0xf1, 0xb0, 0x03, 0x46, 0xf9, 0xdf, 0xe0, 0xb2, 0xee,
            0x90, 0x5d, 0xcc, 0x28, 0x8b, 0xaf, 0x4a, 0x92, 0xde, 0x3f, 0x40, 0x01, 0xdd, 0x9f,
            0x44, 0xc4, 0x68, 0xc3, 0xd0, 0x7d, 0x6c, 0x6e, 0xe8, 0x2f, 0xac, 0xea, 0xfc, 0x97,
            0xc2, 0xfc, 0x0f, 0xc0, 0x60, 0x17, 0x19, 0xd2, 0xdc, 0xd0, 0xaa, 0x2a, 0xec, 0x92,
            0xd1, 0xb0, 0xae, 0x93, 0x3c, 0x65, 0xeb, 0x06, 0xa0, 0x3c, 0x9c, 0x93, 0x5c, 0x2b,
            0xad, 0x04, 0x59, 0x81, 0x02, 0x41, 0x34, 0x7a, 0xb8, 0x7e, 0x9f, 0x11, 0xad, 0xb3,
            0x04, 0x15, 0x42, 0x4c, 0x6c, 0x7f, 0x5f, 0x22, 0xa0, 0x03, 0xb8, 0xab, 0x8d, 0xe5,
            0x4f, 0x6d, 0xed, 0x0e, 0x3a, 0xb9, 0x24, 0x5f, 0xa7, 0x95, 0x68, 0x45, 0x1d, 0xfa,
            0x25, 0x8e,
        ];

        let expected_hmac: [u8; 32] = [
            0x76, 0x9f, 0x00, 0xd3, 0xe6, 0xa6, 0xcc, 0x1f, 0xb4, 0x26, 0xa1, 0x4a, 0x4f, 0x76,
            0xc6, 0x46, 0x2e, 0x61, 0x49, 0x72, 0x6e, 0x0d, 0xee, 0x0e, 0xc0, 0xcf, 0x97, 0xa1,
            0x66, 0x05, 0xac, 0x8b,
        ];

        test_hmac(&key, &msg, DdiHashAlgorithm::Sha256, &expected_hmac);
    }

    #[test]
    fn test_hmac_sha384() {
        // Test vector source: https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/mac/hmactestvectors.zip
        // [L=48]; sha384
        // Count = 50
        // Klen = 50
        // Tlen = 48
        // Key = d137f3e6cc4af28554beb03ba7a97e60c9d3959cd3bb08068edbf68d402d0498c6ee0ae9e3a20dc7d8586e5c352f605cee19
        // Msg = 64a884670d1c1dff555483dcd3da305dfba54bdc4d817c33ccb8fe7eb2ebf623624103109ec41644fa078491900c59a0f666f0356d9bc0b45bcc79e5fc9850f4543d96bc68009044add0838ac1260e80592fbc557b2ddaf5ed1b86d3ed8f09e622e567f1d39a340857f6a850cceef6060c48dac3dd0071fe68eb4ed2ed9aca01
        // Mac = c550fa53514da34f15e7f98ea87226ab6896cdfae25d3ec2335839f755cdc9a4992092e70b7e5bd422784380b6396cf5

        let key: [u8; 50] = [
            0xd1, 0x37, 0xf3, 0xe6, 0xcc, 0x4a, 0xf2, 0x85, 0x54, 0xbe, 0xb0, 0x3b, 0xa7, 0xa9,
            0x7e, 0x60, 0xc9, 0xd3, 0x95, 0x9c, 0xd3, 0xbb, 0x08, 0x06, 0x8e, 0xdb, 0xf6, 0x8d,
            0x40, 0x2d, 0x04, 0x98, 0xc6, 0xee, 0x0a, 0xe9, 0xe3, 0xa2, 0x0d, 0xc7, 0xd8, 0x58,
            0x6e, 0x5c, 0x35, 0x2f, 0x60, 0x5c, 0xee, 0x19,
        ];

        let msg: [u8; 128] = [
            0x64, 0xa8, 0x84, 0x67, 0x0d, 0x1c, 0x1d, 0xff, 0x55, 0x54, 0x83, 0xdc, 0xd3, 0xda,
            0x30, 0x5d, 0xfb, 0xa5, 0x4b, 0xdc, 0x4d, 0x81, 0x7c, 0x33, 0xcc, 0xb8, 0xfe, 0x7e,
            0xb2, 0xeb, 0xf6, 0x23, 0x62, 0x41, 0x03, 0x10, 0x9e, 0xc4, 0x16, 0x44, 0xfa, 0x07,
            0x84, 0x91, 0x90, 0x0c, 0x59, 0xa0, 0xf6, 0x66, 0xf0, 0x35, 0x6d, 0x9b, 0xc0, 0xb4,
            0x5b, 0xcc, 0x79, 0xe5, 0xfc, 0x98, 0x50, 0xf4, 0x54, 0x3d, 0x96, 0xbc, 0x68, 0x00,
            0x90, 0x44, 0xad, 0xd0, 0x83, 0x8a, 0xc1, 0x26, 0x0e, 0x80, 0x59, 0x2f, 0xbc, 0x55,
            0x7b, 0x2d, 0xda, 0xf5, 0xed, 0x1b, 0x86, 0xd3, 0xed, 0x8f, 0x09, 0xe6, 0x22, 0xe5,
            0x67, 0xf1, 0xd3, 0x9a, 0x34, 0x08, 0x57, 0xf6, 0xa8, 0x50, 0xcc, 0xee, 0xf6, 0x06,
            0x0c, 0x48, 0xda, 0xc3, 0xdd, 0x00, 0x71, 0xfe, 0x68, 0xeb, 0x4e, 0xd2, 0xed, 0x9a,
            0xca, 0x01,
        ];

        let expected_hmac: [u8; 48] = [
            0xc5, 0x50, 0xfa, 0x53, 0x51, 0x4d, 0xa3, 0x4f, 0x15, 0xe7, 0xf9, 0x8e, 0xa8, 0x72,
            0x26, 0xab, 0x68, 0x96, 0xcd, 0xfa, 0xe2, 0x5d, 0x3e, 0xc2, 0x33, 0x58, 0x39, 0xf7,
            0x55, 0xcd, 0xc9, 0xa4, 0x99, 0x20, 0x92, 0xe7, 0x0b, 0x7e, 0x5b, 0xd4, 0x22, 0x78,
            0x43, 0x80, 0xb6, 0x39, 0x6c, 0xf5,
        ];

        test_hmac(&key, &msg, DdiHashAlgorithm::Sha384, &expected_hmac);
    }

    #[test]
    fn test_hmac_sha512() {
        // Test vector source: https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/mac/hmactestvectors.zip
        // [L=64]; sha512
        // Count = 215
        // Klen = 128
        // Tlen = 64
        // Key = aa42b41c544fa928b2f3c7f12c41e5c56c910860ca257cb3080c24e440470e951a2b4a694206fdc41a05b1d3ac55efcde2891078f93c50ee33f724a1cc55ce9d30642e0d6b4fdb01e13a726e3f6e2e76b1b6b9ea5608420ef168d09ce10ad60b53b70710b6716b666f5ab3cbced2ca4b41e0acc0c8d37b9aa929d0dc65af4f67
        // Msg = 2b1f5c46d4b819bfa1ede55a14077644b642aa3963d177a6e823200bd065afa47a489f486f04d991f39de23dda6452d49dc2888bad319c69078b95a80987dc5e8480f15d12795d57aa5fe846718d0b0ad396a854d33ef9c49fc9c74e6879dce27052ba4c65208d59edbb5f3b828a8b2e8046745c7c0076fed8661dc594429578
        // Mac = 16d83f28f335f8d876b2fc85512159147f4cdcbb5c3ace09367d8f1b557bc977cc6cd31db4f93b144302f2712a05fd964f21f5fff11d28b703b9de3a01f87764

        let key: [u8; 128] = [
            0xaa, 0x42, 0xb4, 0x1c, 0x54, 0x4f, 0xa9, 0x28, 0xb2, 0xf3, 0xc7, 0xf1, 0x2c, 0x41,
            0xe5, 0xc5, 0x6c, 0x91, 0x08, 0x60, 0xca, 0x25, 0x7c, 0xb3, 0x08, 0x0c, 0x24, 0xe4,
            0x40, 0x47, 0x0e, 0x95, 0x1a, 0x2b, 0x4a, 0x69, 0x42, 0x06, 0xfd, 0xc4, 0x1a, 0x05,
            0xb1, 0xd3, 0xac, 0x55, 0xef, 0xcd, 0xe2, 0x89, 0x10, 0x78, 0xf9, 0x3c, 0x50, 0xee,
            0x33, 0xf7, 0x24, 0xa1, 0xcc, 0x55, 0xce, 0x9d, 0x30, 0x64, 0x2e, 0x0d, 0x6b, 0x4f,
            0xdb, 0x01, 0xe1, 0x3a, 0x72, 0x6e, 0x3f, 0x6e, 0x2e, 0x76, 0xb1, 0xb6, 0xb9, 0xea,
            0x56, 0x08, 0x42, 0x0e, 0xf1, 0x68, 0xd0, 0x9c, 0xe1, 0x0a, 0xd6, 0x0b, 0x53, 0xb7,
            0x07, 0x10, 0xb6, 0x71, 0x6b, 0x66, 0x6f, 0x5a, 0xb3, 0xcb, 0xce, 0xd2, 0xca, 0x4b,
            0x41, 0xe0, 0xac, 0xc0, 0xc8, 0xd3, 0x7b, 0x9a, 0xa9, 0x29, 0xd0, 0xdc, 0x65, 0xaf,
            0x4f, 0x67,
        ];

        let msg: [u8; 128] = [
            0x2b, 0x1f, 0x5c, 0x46, 0xd4, 0xb8, 0x19, 0xbf, 0xa1, 0xed, 0xe5, 0x5a, 0x14, 0x07,
            0x76, 0x44, 0xb6, 0x42, 0xaa, 0x39, 0x63, 0xd1, 0x77, 0xa6, 0xe8, 0x23, 0x20, 0x0b,
            0xd0, 0x65, 0xaf, 0xa4, 0x7a, 0x48, 0x9f, 0x48, 0x6f, 0x04, 0xd9, 0x91, 0xf3, 0x9d,
            0xe2, 0x3d, 0xda, 0x64, 0x52, 0xd4, 0x9d, 0xc2, 0x88, 0x8b, 0xad, 0x31, 0x9c, 0x69,
            0x07, 0x8b, 0x95, 0xa8, 0x09, 0x87, 0xdc, 0x5e, 0x84, 0x80, 0xf1, 0x5d, 0x12, 0x79,
            0x5d, 0x57, 0xaa, 0x5f, 0xe8, 0x46, 0x71, 0x8d, 0x0b, 0x0a, 0xd3, 0x96, 0xa8, 0x54,
            0xd3, 0x3e, 0xf9, 0xc4, 0x9f, 0xc9, 0xc7, 0x4e, 0x68, 0x79, 0xdc, 0xe2, 0x70, 0x52,
            0xba, 0x4c, 0x65, 0x20, 0x8d, 0x59, 0xed, 0xbb, 0x5f, 0x3b, 0x82, 0x8a, 0x8b, 0x2e,
            0x80, 0x46, 0x74, 0x5c, 0x7c, 0x00, 0x76, 0xfe, 0xd8, 0x66, 0x1d, 0xc5, 0x94, 0x42,
            0x95, 0x78,
        ];

        let expected_hmac: [u8; 64] = [
            0x16, 0xd8, 0x3f, 0x28, 0xf3, 0x35, 0xf8, 0xd8, 0x76, 0xb2, 0xfc, 0x85, 0x51, 0x21,
            0x59, 0x14, 0x7f, 0x4c, 0xdc, 0xbb, 0x5c, 0x3a, 0xce, 0x09, 0x36, 0x7d, 0x8f, 0x1b,
            0x55, 0x7b, 0xc9, 0x77, 0xcc, 0x6c, 0xd3, 0x1d, 0xb4, 0xf9, 0x3b, 0x14, 0x43, 0x02,
            0xf2, 0x71, 0x2a, 0x05, 0xfd, 0x96, 0x4f, 0x21, 0xf5, 0xff, 0xf1, 0x1d, 0x28, 0xb7,
            0x03, 0xb9, 0xde, 0x3a, 0x01, 0xf8, 0x77, 0x64,
        ];

        test_hmac(&key, &msg, DdiHashAlgorithm::Sha512, &expected_hmac);
    }
}
