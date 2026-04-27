// Copyright (c) Microsoft Corporation. All rights reserved.

#![allow(missing_docs)]
#![forbid(unsafe_code)]
#![allow(elided_lifetimes_in_paths)]

//! Certificate template module.
//! This module contains the templates used by the certificate builder module to generate certificates and certificate signing requests.

mod azihsm_csr_tbs;
mod azihsm_leaf_cert_tbs;

pub use azihsm_csr_tbs::{AzihsmCsrTbs, AzihsmCsrTbsParams};
pub use azihsm_leaf_cert_tbs::{AzihsmLeafCertTbs, AzihsmLeafCertTbsParams};
