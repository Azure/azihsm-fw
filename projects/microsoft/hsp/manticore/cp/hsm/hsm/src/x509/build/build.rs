// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]
#![forbid(unsafe_code)]

//!  File contains the entry point for build time script used for generating various X509 artifacts
//! used by Manticore.

#[cfg(feature = "generate_templates")]
mod cert;
#[cfg(feature = "generate_templates")]
mod code_gen;
#[cfg(feature = "generate_templates")]
mod csr;
#[cfg(feature = "generate_templates")]
mod tbs;
#[cfg(feature = "generate_templates")]
mod x509;

#[cfg(feature = "generate_templates")]
use std::env;

#[cfg(feature = "generate_templates")]
use code_gen::CodeGen;
#[cfg(feature = "generate_templates")]
use x509::EcdsaSha384Algo;
#[cfg(feature = "generate_templates")]
use x509::KeyUsage;

// Main Entry point
fn main() {
    #[cfg(feature = "generate_templates")]
    {
        let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
        if target_os == "windows" {
            panic!("This build script does not support Windows targets.");
        }

        let out_dir_os_str = env::var_os("OUT_DIR").unwrap();
        let out_dir = out_dir_os_str.to_str().unwrap();

        gen_csr(out_dir);
        gen_leaf_cert(out_dir);
    }
}

/// Generated AZIHSM Certificate Signing Request Template.
#[cfg(feature = "generate_templates")]
fn gen_csr(out_dir: &str) {
    let mut usage = KeyUsage::default();
    usage.set_key_cert_sign(true);
    let bldr = csr::CsrTemplateBuilder::<EcdsaSha384Algo>::new()
        .add_basic_constraints_ext(true, 1)
        .add_key_usage_ext(usage);
    let template = bldr.tbs_template();
    CodeGen::gen_code("AzihsmCsrTbs", template, out_dir);
}

/// Generate AZIHSM Leaf Certificate Template.
#[cfg(feature = "generate_templates")]
fn gen_leaf_cert(out_dir: &str) {
    let mut usage = KeyUsage::default();
    usage.set_digital_signature(true);
    let bldr = cert::CertTemplateBuilder::<EcdsaSha384Algo>::new()
        .add_basic_constraints_ext(false, 0)
        .add_key_usage_ext(usage);
    let template = bldr.tbs_template();
    CodeGen::gen_code("AzihsmLeafCertTbs", template, out_dir);
}
