// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::app::*;
use crate::buildx86::*;
use crate::clippy::*;
use crate::copyright_header::*;
use crate::coverage_lines::*;
use crate::coverage_vscode::*;
use crate::doctest_check::*;
use crate::fmt::*;
use crate::hwtest::*;
use crate::testx86::*;

type DynError = Box<dyn std::error::Error>;

pub(crate) fn precheckin() -> Result<(), DynError> {
    let mut status = fmt();
    if status.is_err() {
        Err("cargo fmt failed")?;
    }

    status = copyright_header();
    if status.is_err() {
        Err("copyright-header-check failed")?;
    }

    status = doctest_check();
    if status.is_err() {
        Err("doctest-check failed")?;
    }

    status = build();
    if status.is_err() {
        Err("cargo build failed")?;
    }

    status = clippy();
    if status.is_err() {
        Err("cargo clippy failed")?;
    }

    status = fmt_check();
    if status.is_err() {
        Err("cargo fmt --all failed")?;
    }

    status = test(&[]);
    if status.is_err() {
        Err("cargo test failed")?;
    }

    status = test(&["--features", "mcr_test_hooks"]);
    if status.is_err() {
        Err("cargo test with mcr_test_hooks failed")?;
    }

    status = test(&["--features", "fips_validation_hooks"]);
    if status.is_err() {
        Err("cargo test with fips_validation_hooks failed")?;
    }

    status = hw_unit_test();
    if status.is_err() {
        Err("hw-unit-test failed")?;
    }

    status = app_build_release(&[]);
    if status.is_err() {
        Err("app-release-build failed")?;
    }

    status = app_build_release(&["--features=mcr_test_hooks"]);
    if status.is_err() {
        Err("app-release-build with mcr_test hooks failed")?;
    }

    status = app_build_release(&["--features=mcr_test_hooks,fips_validation_hooks"]);
    if status.is_err() {
        Err("app-release-build with fips and mcr_test_hooks failed")?;
    }

    status = coverage_vscode();
    if status.is_err() {
        Err("generating coverage failed")?;
    }

    status = coverage_lines();
    if status.is_err() {
        Err("coverage_lines_test failed: coverage less than 90%")?;
    }

    Ok(())
}
