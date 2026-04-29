// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn test(extra_features: &[&str]) -> Result<(), DynError> {
    println!("Running: cargo nextest run {}", extra_features.join(" "));
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["nextest", "run"])
        .args(extra_features)
        .env("RUST_LOG", "info")
        .status()?;

    if !status.success() {
        Err("cargo nextest failed")?;
    }

    Ok(())
}
