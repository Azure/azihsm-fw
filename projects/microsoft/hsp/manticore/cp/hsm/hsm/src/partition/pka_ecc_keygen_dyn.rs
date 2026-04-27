// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccKeyPair;
use mcr_crypto_pka::PkaTrait;

#[allow(dead_code)]
pub(crate) trait PkaEccKeygenDyn {
    fn begin_ecc_gen_key(&self, tag: TagId, curve: PkaEccCurve) -> Result<PkaEccCmd, HsmErr>;
    fn end_ecc_gen_key(&self, tag: TagId, cmd: PkaEccCmd) -> Result<PkaEccKeyPair, HsmErr>;
}

pub(crate) struct PkaEngineRefBox<E: crate::HsmEnvTrait + 'static>(pub PkaEngineRef<E>);

impl<E: crate::HsmEnvTrait + 'static> PkaEccKeygenDyn for PkaEngineRefBox<E> {
    fn begin_ecc_gen_key(&self, tag: TagId, curve: PkaEccCurve) -> Result<PkaEccCmd, HsmErr> {
        self.0
            .deref()
            .begin_ecc_gen_key(tag, curve)
            .map_err(|_| HsmErr::EccGenKeyFailed)
    }
    fn end_ecc_gen_key(&self, tag: TagId, cmd: PkaEccCmd) -> Result<PkaEccKeyPair, HsmErr> {
        self.0
            .deref()
            .end_ecc_gen_key(tag, cmd)
            .map_err(|_| HsmErr::EccGenKeyFailed)
    }
}
