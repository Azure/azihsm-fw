// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]
use cfg_if::cfg_if;

cfg_if! {
    if #[cfg(feature = "mcr_test_hooks")] {
        mod cdma_err;
        pub use cdma_err::CdmaErr;
    }
}

/// Cdma Error trait
pub trait CdmaErrTrait {
    /// Inject a correctable CDMA ECC error
    #[cfg(feature = "mcr_test_hooks")]
    fn inject_correctable_ecc_error() -> bool;
}
