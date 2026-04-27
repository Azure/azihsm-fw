// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn coverage() -> Result<(), DynError> {
    println!("Running coverage tests");

    // cargo llvm-cov --html --hide-instantiations --fail-under-lines 88 --ignore-filename-regex "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio" nextest --config-file nextest.toml --profile ci
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args([
            "llvm-cov",
            "--html",
            "--hide-instantiations",
            "--fail-under-lines",
            "88",
            "--ignore-filename-regex",
            "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio|preop_cdma_io.rs",
            "nextest",
            "--config-file",
            "nextest.toml",
            "--profile",
            "ci",
        ])
        .env("RUST_LOG", "info")
        .status()?;
    if !status.success() {
        Err("Failed to meet minimum 88% code coverage!")?;
    }

    Ok(())
}
