// Copyright (c) Microsoft Corporation. All rights reserved.

#[cfg(feature = "mcr_test_hooks")]
use mcr_registers::cdma;

use crate::*;

/// CDMA Error
#[derive(Clone, Default)]
pub struct CdmaErr {}

impl CdmaErrTrait for CdmaErr {
    /// Inject a correctable CDMA ECC error
    #[cfg(feature = "mcr_test_hooks")]
    fn inject_correctable_ecc_error() -> bool {
        let reg = cdma::RegisterBlock::block();
        reg.error_injection().write(|w| w.err_inject_corr_ecc(true));

        true
    }
}
