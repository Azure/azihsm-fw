// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::env_set::EnvVarGuard;
use crate::telemetry_tokenize::telemetry_log_tokenizer;
use crate::util::*;
use std::env;
use std::process::Command;

use colored::Colorize;

type DynError = Box<dyn std::error::Error>;

const DEFAULT_DISABLE_CP_TOKENIZER: u8 = 0;

pub(crate) const RELEASE_ARGS: [&str; 5] = [
    "--target",
    "thumbv7em-none-eabihf",
    "--no-default-features",
    "--profile=firmware",
    "--features=log_level_info",
];

pub(crate) fn app_build_release(extra_features: &[&str]) -> Result<(), DynError> {
    // Run the telemetry tokenizer
    run_telemetry_tokenizer();

    // Process extra features by setting appropriate environment variables
    let mut _env_guard = process_extra_features(extra_features);

    let cargo = env::var("CARGO").unwrap_or_else(|_| "cargo".to_string());

    // Build mcr-hsm
    let mut mcr_hsm_args = vec!["build"];
    mcr_hsm_args.extend_from_slice(&["--bin", "mcr-hsm"]);
    mcr_hsm_args.extend(RELEASE_ARGS.iter());

    let status = Command::new(&cargo)
        .current_dir(project_root().join("hsm"))
        .args(&mcr_hsm_args)
        .args(extra_features)
        .status()?;

    if !status.success() {
        Err("mcr-hsm release build command failed")?;
    }

    // Build mcr-admin
    let mut mcr_admin_args = vec!["build"];
    mcr_admin_args.extend_from_slice(&["--bin", "mcr-admin"]);
    mcr_admin_args.extend(RELEASE_ARGS.iter());

    let status = Command::new(&cargo)
        .current_dir(project_root().join("admin"))
        .args(&mcr_admin_args)
        .args(extra_features)
        .status()?;

    if !status.success() {
        Err("mcr-admin release build command failed")?;
    }

    let target_dir = project_root().join("target/thumbv7em-none-eabihf/firmware");
    let mcr_hsm_bin_path = target_dir.join("mcr-hsm");
    let mcr_admin_bin_path = target_dir.join("mcr-admin");

    println!("Firmware binaries created:");
    if mcr_hsm_bin_path.exists() {
        println!("  {}", mcr_hsm_bin_path.display());
    }
    if mcr_admin_bin_path.exists() {
        println!("  {}", mcr_admin_bin_path.display());
    }

    Ok(())
}

fn run_telemetry_tokenizer() {
    let disable_tokenizer: u8 = env::var("DISABLE_CP_TOKENIZER")
        .ok()
        .and_then(|val| val.parse::<u8>().ok())
        .unwrap_or(DEFAULT_DISABLE_CP_TOKENIZER);

    if disable_tokenizer != 1 {
        // If the telemetry tokenizer is enabled, we run it before building the firmware.
        // This will ensure that the telemetry logs are properly tokenized.
        println!("Running telemetry tokenizer...");
        let _ = telemetry_log_tokenizer();
    } else {
        let msg = "Skipping telemetry tokenizer as it is disabled via env variable 'DISABLE_CP_TOKENIZER'."
            .to_string().bright_yellow();
        println!("{}", msg);
    }
}

fn process_extra_features(extra_features: &[&str]) -> EnvVarGuard {
    let mut env_var_guard = EnvVarGuard::default();

    // Set optimization level for firmware profile if any hook-related feature is enabled
    // to optimize the build for code size rather than performance.
    // Note: extra feature args may include full cargo flags (e.g. "--features=mcr_test_hooks").
    if extra_features
        .iter()
        .any(|f| f.contains("fips_validation_hooks"))
    {
        env_var_guard.set("CARGO_PROFILE_FIRMWARE_OPT_LEVEL", "z");
    }

    env_var_guard
}
