// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]
#![forbid(unsafe_code)]

//! Certificate builder module.
//! This module builds the x509 Certificate or Certificate Signing Request
//! from "To Be Signed" blob and ECDSA-384 Signature.

mod builder;
mod test_utils;
mod tests;

pub use builder::{Ecdsa384CertBuilder, Ecdsa384CsrBuilder, Ecdsa384Signature};

pub const NOT_BEFORE: &str = "20230101000000Z";
pub const NOT_AFTER: &str = "99991231235959Z";

pub struct NotBefore {
    pub value: [u8; 15],
}

impl Default for NotBefore {
    fn default() -> Self {
        let mut nb: NotBefore = NotBefore { value: [0u8; 15] };

        nb.value.copy_from_slice(NOT_BEFORE.as_bytes());
        nb
    }
}

pub struct NotAfter {
    pub value: [u8; 15],
}

impl Default for NotAfter {
    fn default() -> Self {
        let mut nf: NotAfter = NotAfter { value: [0u8; 15] };

        nf.value.copy_from_slice(NOT_AFTER.as_bytes());
        nf
    }
}
