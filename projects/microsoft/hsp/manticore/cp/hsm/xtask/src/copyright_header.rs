// Licensed under the Apache-2.0 license

// This implementation is adapted from reference: https://github.com/chipsalliance/caliptra-sw/blob/main/ci-tools/file-header-fix/src/main.rs
// but is modified to suit the needs of this project.

use std::path::Path;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

const REQUIRED_TEXT: &str = "Copyright (c) Microsoft Corporation. All rights reserved.";
const EXTENSIONS: &[&str] = &["rs", "h", "c", "cpp", "cc", "toml", "sh", "ld"];

const IGNORED_PATHS: &[&str] = &[
    "./types/src/volatile_cell.rs",
    "./xtask/src/copyright_header.rs",
    "./xtask/src/doctest_check.rs",
];

const IGNORED_DIRS: &[&str] = &[".cargo", "reg", "target", "vendored"];

pub(crate) fn copyright_header() -> Result<(), DynError> {
    let pwd = Path::new("./");

    let mut files = Vec::new();
    find_files(pwd, &mut files, EXTENSIONS, IGNORED_PATHS, IGNORED_DIRS).unwrap();
    files.sort();
    for file in files.iter() {
        if let Err(e) = check_file(file, REQUIRED_TEXT, 3, false) {
            println!("{e}");
            Err("Copyright check failed.")?;
        }
    }
    Ok(())
}
