// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn hw_unit_test() -> Result<(), DynError> {
    println!("Building: pka-tests build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root().join("drivers/crypto/pka/tests"))
        .args([
            "build",
            "--bin",
            "pka-tests",
            "--target",
            "thumbv7em-none-eabihf",
            "--features",
            "arm",
            "--profile=firmware",
        ])
        .status()?;

    if !status.success() {
        Err("pka-tests command failed")?;
    }

    println!("Building: sha-tests build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root().join("drivers/crypto/sha/tests"))
        .args([
            "build",
            "--bin",
            "sha-tests",
            "--target",
            "thumbv7em-none-eabihf",
            "--features",
            "arm",
            "--profile=firmware",
        ])
        .status()?;

    if !status.success() {
        Err("sha-tests command failed")?;
    }

    println!("Building: aes-tests build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root().join("drivers/crypto/aes/tests"))
        .args([
            "build",
            "--bin",
            "aes-tests",
            "--target",
            "thumbv7em-none-eabihf",
            "--features",
            "arm",
            "--profile=firmware",
        ])
        .status()?;

    if !status.success() {
        Err("aes-tests command failed")?;
    }

    println!("Building: softaes-tests build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root().join("drivers/crypto/softaes/tests"))
        .args([
            "build",
            "--bin",
            "softaes-tests",
            "--target",
            "thumbv7em-none-eabihf",
            "--features",
            "arm",
            "--profile=firmware",
        ])
        .status()?;

    if !status.success() {
        Err("aes-tests command failed")?;
    }

    println!("Building: rng-tests build");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root().join("drivers/crypto/rng/tests"))
        .args([
            "build",
            "--bin",
            "rng-tests",
            "--target",
            "thumbv7em-none-eabihf",
            "--features",
            "arm",
            "--profile=firmware",
        ])
        .status()?;

    if !status.success() {
        Err("rng-tests command failed")?;
    }

    Ok(())
}
