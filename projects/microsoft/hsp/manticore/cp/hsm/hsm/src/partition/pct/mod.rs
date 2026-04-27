// Copyright (c) Microsoft Corporation. All rights reserved.

pub(crate) mod ecc;
pub(crate) mod ecc_pct_constants;
pub(crate) mod pct_engine;
pub(crate) mod pct_engine_impl;
pub(crate) mod rsa_pct_constants;

use super::*;
use crate::env::HsmEnvTrait;
pub(crate) use ecc::EccKeyPct;

/// Maximum ECC signature length (in bytes) for supported curves.
pub(crate) const ECC_SIGNATURE_MAX_LEN: usize = 192;

/// List of states in the ECC Gen Pub Key command.
#[derive(Clone, PartialEq)]
pub(crate) enum EccPtMultiplicationState {
    /// Signifies that Montgomery constant calculation is in progress.
    WaitForMontgomeryConstCalc,

    /// Signifies that a point multiplication command is in progress.
    WaitForPointMultiplication,
}

/// Elliptic Curve Cryptography (ECC) ECDH Key exchange Command data.
pub(crate) struct PctEcdhComputeCmd {
    /// Pka Ecc Command
    pub(crate) cmd_info: PkaEccCmd,

    /// State of the ECDH compute command
    pub(crate) state: EccPtMultiplicationState,
}

/// List of states in the ECC PCT Validation command state
#[derive(Clone, PartialEq, Copy)]
pub(crate) enum EccPctValidationState {
    /// Initial state
    Init,

    /// Waiting for ECC sign operation to complete
    WaitForSign,

    /// Waiting for ECC verify operation to complete
    WaitForVerify,

    /// Waiting for first Montgomery constant calculation (ECDH Step 1)
    EcdhMontgomeryConstCalculationFirst,

    /// Waiting for first ECDH compute
    EcdhComputeFirst,

    /// Waiting for second Montgomery constant calculation (ECDH Step 2)
    EcdhMontgomeryConstCalculationSecond,

    /// Waiting for second ECDH compute
    EcdhComputeSecond,

    /// Validation completed successfully
    ValidationComplete,
}
