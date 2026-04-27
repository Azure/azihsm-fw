// Licensed under the Apache-2.0 license

// This implementation is adapted from reference: https://github.com/chipsalliance/caliptra-sw/blob/main/ci-tools/file-header-fix/src/main.rs
// but is modified to suit the needs of this project.

use std::path::Path;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

const REQUIRED_TEXT: &str = "doctest = false";
const EXTENSIONS: &[&str] = &["toml"];

const IGNORED_PATHS: &[&str] = &[
    "./rustfmt.toml",
    "./rust-toolchain.toml",
    "./nextest.toml",
    "./Cargo.toml",
    "./app/Cargo.toml",
    "./admin/Cargo.toml",
    "./hsm/Cargo.toml",
    "./drivers/crypto/aes/tests/Cargo.toml",
    "./drivers/crypto/pka/tests/Cargo.toml",
    "./drivers/crypto/rng/tests/Cargo.toml",
    "./drivers/crypto/sha/tests/Cargo.toml",
    "./tools/host-vfio/Cargo.toml",
    "./xtask/Cargo.toml",
];

const IGNORED_DIRS: &[&str] = &[".cargo", "reg", "target", "vendored"];

pub(crate) fn doctest_check() -> Result<(), DynError> {
    let pwd = Path::new("./");

    let mut files = Vec::new();
    find_files(pwd, &mut files, EXTENSIONS, IGNORED_PATHS, IGNORED_DIRS).unwrap();
    files.sort();
    for file in files.iter() {
        if let Err(e) = check_file(file, REQUIRED_TEXT, usize::MAX, true) {
            println!("{e}");
            Err("doctest check failed.")?;
        }
    }
    Ok(())
}
