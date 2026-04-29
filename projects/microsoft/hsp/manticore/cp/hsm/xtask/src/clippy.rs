// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn clippy() -> Result<(), DynError> {
    println!("Running: cargo clippy");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["clippy", "--all-targets", "--", "-D", "warnings"])
        .status()?;

    if !status.success() {
        Err("cargo clippy failed")?;
    }

    Ok(())
}
