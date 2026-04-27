// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::env::HsmEnvTrait;
use mcr_crypto_pka::*;
use mcr_crypto_sha::*;
use mcr_types::IoMemRange;
use pct_engine::*;

pub struct PctEngineImpl<E: HsmEnvTrait + 'static> {
    pka: PkaEngineRef<E>,
    sha: <E::Hal as HsmHalTrait>::Sha,
}

impl<E: HsmEnvTrait + 'static> PctEngineImpl<E> {
    pub fn new(pka: PkaEngineRef<E>, sha: <E::Hal as HsmHalTrait>::Sha) -> Self {
        Self { pka, sha }
    }
}

impl<E: HsmEnvTrait + 'static> PctEngine for PctEngineImpl<E> {
    fn peek_tag(&self) -> Option<TagId> {
        self.pka.deref().peek_tag()
    }

    fn begin_montgomery_constant_calculation(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
    ) -> McrResult<()> {
        self.pka
            .deref()
            .begin_montgomery_constant_calculation(tag, curve)
    }

    fn end_montgomery_constant_calculation(&mut self, tag: TagId) -> McrResult<()> {
        self.pka.deref().end_montgomery_constant_calculation(tag)
    }

    fn begin_ecdh_compute_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        priv_blob: &[u8],
        pub_blob: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.pka
            .deref()
            .begin_ecdh_compute_zc(tag, curve, priv_blob, pub_blob)
    }

    fn end_ecdh_compute(&self, tag: TagId, cmd: PkaEccCmd) -> McrResult<PkaEccSecretValue> {
        self.pka.deref().end_ecdh_compute(tag, cmd)
    }

    fn begin_ecc_sign_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        priv_blob: &[u8],
        digest: &IoMemRange,
        signature_out: &IoMemRange,
    ) -> McrResult<PkaEccCmd> {
        self.pka
            .deref()
            .begin_ecc_sign_zc(tag, curve, priv_blob, digest, signature_out)
    }

    fn end_ecc_sign_zc(&mut self, tag: TagId) -> McrResult<()> {
        self.pka.deref().end_ecc_sign_zc(tag)
    }

    fn begin_ecc_verify_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        pub_blob: &IoMemRange,
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<()> {
        self.pka
            .deref()
            .begin_ecc_verify_zc(tag, curve, pub_blob, digest, signature)
    }

    fn end_ecc_verify_zc(&self, tag: TagId) -> McrResult<bool> {
        self.pka.deref().end_ecc_verify_zc(tag)
    }

    fn sha_single_block_zc(
        &mut self,
        mode: ShaMode,
        input: &IoMemRange,
        output: &mut IoMemRange,
    ) -> McrResult<()> {
        let info = ShaDigestCmdInfoZc {
            buffer: input,
            init_digest: None,
            mode,
            last: true,
            len: input.len() as u32,
            total_len: input.len() as u32,
            output_buffer: output,
        };
        self.sha.digest_zc(&info)
    }
}
