// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::process::Command;

use crate::util::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn setup() -> Result<(), DynError> {
    println!("Running: cargo install cargo-bloat");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["install", "cargo-bloat"])
        .status()?;

    if !status.success() {
        Err("cargo install cargo-bloat failed")?;
    }

    println!("Running: cargo install cargo-binutils");
    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());
    let status = Command::new(cargo)
        .current_dir(project_root())
        .args(["install", "cargo-binutils"])
        .status()?;

    if !status.success() {
        Err("cargo install cargo-binutils failed")?;
    }

    println!("Installing coverage tools");
    let status = Command::new("curl")
        .current_dir(project_root())
        .args(["-LsSf", "-o" , "cargo-llvm-cov.tar.gz", "https://github.com/taiki-e/cargo-llvm-cov/releases/latest/download/cargo-llvm-cov-x86_64-unknown-linux-gnu.tar.gz"])
        .status()?;

    if !status.success() {
        Err("llvm-cov package download failed")?;
    }

    let status = Command::new("tar")
        .current_dir(project_root())
        .args([
            "xzf",
            "cargo-llvm-cov.tar.gz",
            "-C",
            cargo_bin()
                .join(".cargo")
                .join("bin")
                .as_path()
                .to_str()
                .unwrap(),
        ])
        .status()?;

    if !status.success() {
        Err("llvm-cov package install failed")?;
    }

    let status = Command::new("rm")
        .current_dir(project_root())
        .args(["-rf", "cargo-llvm-cov.tar.gz"])
        .status()?;

    if !status.success() {
        Err("Could not delete cargo-llvm-cov.tar.gz")?;
    }

    let status = Command::new("curl")
        .current_dir(project_root())
        .args([
            "-LsSf",
            "-o",
            "cargo-nextest.tar.gz",
            "https://get.nexte.st/latest/linux",
        ])
        .status()?;

    if !status.success() {
        Err("cargo-nextest package download failed")?;
    }

    let status = Command::new("tar")
        .current_dir(project_root())
        .args([
            "xzf",
            "cargo-nextest.tar.gz",
            "-C",
            cargo_bin()
                .join(".cargo")
                .join("bin")
                .as_path()
                .to_str()
                .unwrap(),
        ])
        .status()?;

    if !status.success() {
        Err("cargo nextest package install failed")?;
    }

    let status = Command::new("rm")
        .current_dir(project_root())
        .args(["-rf", "cargo-nextest.tar.gz"])
        .status()?;

    if !status.success() {
        Err("Could not delete cargo-nextest.tar.gz")?;
    }

    Ok(())
}
