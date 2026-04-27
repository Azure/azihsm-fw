// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use mcr_crypto_pka::*;
use mcr_crypto_sha::*;
use mcr_types::IoMemRange;

pub trait PctEngine {
    fn peek_tag(&self) -> Option<TagId>;

    // Montgomery constant calc
    fn begin_montgomery_constant_calculation(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
    ) -> McrResult<()>;

    fn end_montgomery_constant_calculation(&mut self, tag: TagId) -> McrResult<()>;

    // ECDH compute (zero-copy)
    fn begin_ecdh_compute_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        priv_key_blob: &[u8],
        pub_key_blob: &IoMemRange,
    ) -> McrResult<PkaEccCmd>;

    fn end_ecdh_compute(&self, tag: TagId, cmd: PkaEccCmd) -> McrResult<PkaEccSecretValue>;

    fn begin_ecc_sign_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        priv_blob: &[u8],
        digest: &IoMemRange,
        signature_out: &IoMemRange,
    ) -> McrResult<PkaEccCmd>;

    fn end_ecc_sign_zc(&mut self, tag: TagId) -> McrResult<()>;

    fn begin_ecc_verify_zc(
        &mut self,
        tag: TagId,
        curve: PkaEccCurve,
        pub_blob: &IoMemRange,
        digest: &IoMemRange,
        signature: &IoMemRange,
    ) -> McrResult<()>;

    // ECDSA verify (zero-copy) completion
    fn end_ecc_verify_zc(&self, tag: TagId) -> McrResult<bool>;

    // SHA (single-block zero-copy used by Sign/Verify PCT)
    /// Helper to execute single block SHA operation in zero copy mode.
    fn sha_single_block_zc(
        &mut self,
        mode: ShaMode,
        input: &IoMemRange,
        output: &mut IoMemRange,
    ) -> McrResult<()>;
}
