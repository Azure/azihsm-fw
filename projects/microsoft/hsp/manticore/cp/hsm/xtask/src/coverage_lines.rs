// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn coverage_lines() -> Result<(), DynError> {
    println!("Running Lines coverage tests");

    // NOTE: this is a temporary workaround until we can get coverage working in CI
    // Without running check if the coverage lines is at least 88%
    // cargo llvm-cov report --lcov --output-path lcov.info --fail-under-lines 88 --ignore-filename-regex "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio"
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args([
            "llvm-cov",
            "report",
            "--lcov",
            "--output-path",
            "lcov.info",
            "--fail-under-lines",
            "85",
            "--ignore-filename-regex",
            "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio|preop_cdma_io.rs",
        ])
        .env("RUST_LOG", "info")
        .status()?;
    if !status.success() {
        Err("Failed to meet minimum 85% code coverage!")?;
    }

    Ok(())
}
