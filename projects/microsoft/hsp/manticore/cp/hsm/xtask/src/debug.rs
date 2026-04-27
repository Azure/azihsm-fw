// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::app::RELEASE_ARGS;
use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn app_bloat_release() -> Result<(), DynError> {
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());

    // Prepare the args for cargo bloat
    let mut args = vec!["bloat"];
    args.extend(RELEASE_ARGS.iter());
    args.extend(["-n", "100"].iter());

    let status = Command::new(cargo)
        .current_dir(project_root().join("app"))
        .args(args)
        .status()?;

    if !status.success() {
        Err("app-bloat-release command failed")?;
    }

    Ok(())
}

pub(crate) fn app_objdump_release() -> Result<(), DynError> {
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());

    // Prepare the args for objdump
    let mut args = vec!["objdump"];
    args.extend(RELEASE_ARGS.iter());
    args.extend(["--", "-d"].iter());

    let status = Command::new(cargo)
        .current_dir(project_root().join("app"))
        .args(args)
        .status()?;

    if !status.success() {
        Err("app-objdump-release command failed")?;
    }

    Ok(())
}

pub(crate) fn app_size_release() -> Result<(), DynError> {
    println!("Running: app size release");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());

    // Prepare the args for size
    let mut args = vec!["size"];
    args.extend_from_slice(&["--bin", "mcr-app"]);
    args.extend(RELEASE_ARGS.iter());
    args.extend(["--", "-A"].iter());

    let status = Command::new(&cargo)
        .current_dir(project_root().join("app"))
        .args(args)
        .status()?;

    if !status.success() {
        Err("mcr-app-size-release command failed")?;
    }

    // Prepare the args for size
    let mut args = vec!["size"];
    args.extend_from_slice(&["--bin", "admin-app"]);
    args.extend(RELEASE_ARGS.iter());
    args.extend(["--", "-A"].iter());

    let status = Command::new(&cargo)
        .current_dir(project_root().join("admin-app"))
        .args(args)
        .status()?;

    if !status.success() {
        Err("admin-app-size-release command failed")?;
    }

    Ok(())
}
