// Copyright (c) Microsoft Corporation. All rights reserved.

use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn fmt_check() -> Result<(), DynError> {
    println!("Running: cargo fmt --check --all");
    let cargo = "cargo";
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["fmt", "--check", "--all"])
        .status()?;

    if !status.success() {
        Err("cargo fmt --check --all failed")?;
    }

    Ok(())
}

pub(crate) fn fmt() -> Result<(), DynError> {
    println!("Running: cargo fmt");
    let cargo = "cargo";
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["fmt"])
        .status()?;

    if !status.success() {
        Err("cargo fmt failed")?;
    }

    Ok(())
}
