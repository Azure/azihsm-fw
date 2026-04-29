// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::io::Error;
use std::path::Path;
use std::path::PathBuf;

// Gets the project root directory path.
pub(crate) fn project_root() -> PathBuf {
    Path::new(&env!("CARGO_MANIFEST_DIR"))
        .ancestors()
        .nth(1)
        .unwrap()
        .to_path_buf()
}

// Gets the cargo bin directory path.
pub(crate) fn cargo_bin() -> PathBuf {
    Path::new(&env!("HOME")).to_path_buf()
}

// Prints a help menu to list available commands for execution.
pub(crate) fn print_help() {
    eprintln!(
        "Tasks:

setup                   setup developer tools
setup-cicd              setup ci/cd tools
coverage                code coverage report
coverage-cicd           ci/cd code coverage report
coverage-vscode         LCov coverage report readable by VSCode Coverage Gutters Extension
build                   cargo build
clippy                  cargo clippy
fmt                     cargo fmt
fmt-check               cargo fmt --check --all
test                    cargo test
app-release             app release build
app-debug               app debug build
precheckin              run build, clippy, fmt, test, app-release, app-debug
app-bloat-release       cargo bloat command for app
app-bloat-debug         cargo bloat command  for app debug
app-objdump-release     cargo objdump for app
app-objdump-debug       cargo objdump for app debug
app-size-release        cargo size for app
app-size-debug          cargo size for app debug
hw-unit-test            build unit tests that run on hardware
copyright-header-check  check that each file contains the Microsoft copyright header
doctest-check           check if doctests are disabled for all modules
telemetry-tokenize      generate the CP Admin and CP HSM debug log tokens
"
    )
}

// Private helper function to add the path to the current execution context.
fn add_path(path: &Path) -> impl Fn(Error) -> Error + Copy + '_ {
    move |e: Error| Error::new(e.kind(), format!("{path:?}: {e}"))
}

/// Helper function to check the file content for the required text.
///
/// # Arguments
///
/// * `path` - The file path
/// * `contents` - Buffer containing the contents of that line.
/// * `required_text` - Target text for matching the content against.
/// * `max_lines` - Determine how many lines in the file to parse and check for required text match.
/// * `absolute_comparison` - The line will be matched exactly with required_text if `true`.
///   When this is false, the required text can be matched with a subset of that line.
///
/// # Returns
///
/// * `Result` - Ok, or appropriate error code.
fn check_file_contents(
    path: &Path,
    contents: impl BufRead,
    required_text: &str,
    max_lines: usize,
    absolute_comparison: bool,
) -> Result<(), Error> {
    let wrap_err = add_path(path);

    for line in contents.lines().take(max_lines) {
        if absolute_comparison {
            if line.map_err(wrap_err)?.eq(required_text) {
                return Ok(());
            }
        } else if line.map_err(wrap_err)?.contains(required_text) {
            return Ok(());
        }
    }
    Err(Error::other(format!(
        "File {path:?} doesn't contain {required_text:?} in the first {max_lines} lines"
    )))
}

/// Check the file content for the required text.
///
/// # Arguments
///
/// * `path` - The file path
/// * `contents` - Buffer containing the contents of that line.
/// * `required_text` - Target text for matching the content against.
/// * `max_lines` - Determine how many lines in the file to parse and check for required text match.
/// * `absolute_comparison` - The line will be matched exactly with required_text if `true`.
///   When this is false, the required text can be matched with a subset of that line.
///
/// # Returns
///
/// * `Result` - Ok, or appropriate error code.
pub(crate) fn check_file(
    path: &Path,
    required_text: &str,
    max_lines: usize,
    absolute_comparison: bool,
) -> Result<(), Error> {
    let wrap_err = add_path(path);
    if max_lines == usize::MAX {
        check_file_contents(
            path,
            BufReader::new(File::open(path).map_err(wrap_err)?),
            required_text,
            BufReader::new(File::open(path).map_err(wrap_err)?)
                .lines()
                .count(),
            absolute_comparison,
        )
    } else {
        check_file_contents(
            path,
            BufReader::new(File::open(path).map_err(wrap_err)?),
            required_text,
            max_lines,
            absolute_comparison,
        )
    }
}

/// Find files of a specific types and list them in a container.
///
/// # Arguments
///
/// * `dir` - The current directory
/// * `result` - Vector containing list of files matching the criteria.
/// * `extensions` - Target file types to find.
/// * `ignored_paths` - Filter certain file paths and skip adding them to the list.
/// * `ignored_dirs` - Filter certain directories and skip finding files in these directories.
///
/// # Returns
///
/// * `Result` - Ok, or appropriate error code.
pub(crate) fn find_files(
    dir: &Path,
    result: &mut Vec<PathBuf>,
    extensions: &[&str],
    ignored_paths: &[&str],
    ignored_dirs: &[&str],
) -> Result<(), Error> {
    let wrap_err = add_path(dir);
    for file in std::fs::read_dir(dir).map_err(wrap_err)? {
        let file = file.map_err(wrap_err)?;
        let file_path = &file.path();
        let wrap_err = add_path(file_path);
        let file_type = file.file_type().map_err(wrap_err)?;
        if let Some(file_path) = file_path.to_str() {
            if ignored_paths.contains(&file_path) {
                continue;
            }
        }
        if file_type.is_dir() {
            if let Some(file_name) = file.file_name().to_str() {
                if ignored_dirs.contains(&file_name) {
                    continue;
                }
            }
            find_files(file_path, result, extensions, ignored_paths, ignored_dirs)?;
        }
        if let Some(Some(extension)) = file.path().extension().map(|s| s.to_str()) {
            if file_type.is_file() && extensions.contains(&extension) {
                result.push(file_path.into());
            }
        }
    }
    Ok(())
}
