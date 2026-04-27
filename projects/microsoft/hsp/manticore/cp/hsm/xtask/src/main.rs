// Copyright (c) Microsoft Corporation. All rights reserved.

mod app;
mod buildx86;
mod clippy;
mod copyright_header;
mod coverage;
mod coverage_cicd;
mod coverage_lines;
mod coverage_vscode;
mod debug;
mod doctest_check;
mod env_set;
mod fmt;
mod hwtest;
mod precheckin;
mod setup;
mod setup_cicd;
mod telemetry_tokenize;
mod testx86;
mod util;

use std::env;

use colored::Colorize;

use crate::app::*;
use crate::buildx86::*;
use crate::clippy::*;
use crate::copyright_header::*;
use crate::coverage::*;
use crate::coverage_cicd::*;
use crate::coverage_lines::*;
use crate::coverage_vscode::*;
use crate::debug::*;
use crate::doctest_check::*;
use crate::fmt::*;
use crate::hwtest::*;
use crate::precheckin::*;
use crate::setup::*;
use crate::setup_cicd::*;
use crate::telemetry_tokenize::*;
use crate::testx86::*;
use crate::util::*;

type DynError = Box<dyn std::error::Error>;

fn main() {
    if let Err(e) = try_main() {
        eprintln!("{}", e.to_string().red());
        std::process::exit(-1);
    }
}

fn try_main() -> Result<(), DynError> {
    let task = env::args().nth(1);

    // Capture command-line arguments after `cargo xtask <command>`
    let extra_features: Vec<String> = std::env::args().skip(2).collect();
    let extra_features: Vec<&str> = extra_features.iter().map(String::as_str).collect();

    match task.as_deref() {
        Some("setup") => setup()?,
        Some("setup-cicd") => setup_cicd()?,
        Some("coverage") => coverage()?,
        Some("coverage-cicd") => coverage_cicd()?,
        Some("coverage-lines") => coverage_lines()?,
        Some("coverage-vscode") => coverage_vscode()?,
        Some("build") => build()?,
        Some("clippy") => clippy()?,
        Some("fmt") => fmt()?,
        Some("fmt-check") => fmt_check()?,
        Some("test") => test(&extra_features)?,
        Some("app-release") => app_build_release(&extra_features)?,
        Some("precheckin") => precheckin()?,
        Some("app-bloat-release") => app_bloat_release()?,
        Some("app-objdump-release") => app_objdump_release()?,
        Some("app-size-release") => app_size_release()?,
        Some("hw-unit-test") => hw_unit_test()?,
        Some("copyright-header-check") => copyright_header()?,
        Some("doctest-check") => doctest_check()?,
        Some("telemetry-tokenize") => telemetry_log_tokenizer()?,
        _ => print_help(),
    }

    Ok(())
}
