// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn coverage_vscode() -> Result<(), DynError> {
    println!("Running coverage-cicd tests");

    // cargo llvm-cov --lcov --ignore-filename-regex "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio" --output-path lcov.info nextest --config-file nextest.toml --profile ci
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["llvm-cov", "clean", "--workspace"])
        .status()?;
    if !status.success() {
        Err("Failed to clean workspace")?;
    }

    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args([
            "llvm-cov",
            "--lcov",
            "--ignore-filename-regex",
            "drivers|xtask|vendored|reg|cpu|env.rs|tools/host-vfio|preop_cdma_io.rs",
            "--output-path",
            "lcov.info",
            "nextest",
            "--config-file",
            "nextest.toml",
            "--profile",
            "ci",
        ])
        .env("RUST_LOG", "info")
        .status()?;
    if !status.success() {
        Err("Failed to capture coverage-vscode")?;
    }

    Ok(())
}
