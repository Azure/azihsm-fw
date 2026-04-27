// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]
#![forbid(unsafe_code)]
#![allow(unused)] // [TODO] Remove this when x509 items are used

//! x509 module.
//! This module contains the cert/csr builder and the template generator used to generate x509 certificates and certificate signing requests.

mod build;
mod cert_builder;

pub use cert_builder::{
    Ecdsa384CertBuilder, Ecdsa384CsrBuilder, Ecdsa384Signature, NotAfter, NotBefore,
};

pub use build::{AzihsmCsrTbs, AzihsmCsrTbsParams, AzihsmLeafCertTbs, AzihsmLeafCertTbsParams};
