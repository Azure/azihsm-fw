// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn build() -> Result<(), DynError> {
    println!("Running: cargo build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["build"])
        .status()?;

    if !status.success() {
        Err("cargo build failed")?;
    }

    Ok(())
}
