// Copyright (c) Microsoft Corporation. All rights reserved.

use colored::*;
use hashbrown::HashMap;
use quote::ToTokens;
use regex::Regex;
use std::fs;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use syn::parse::{Parse, ParseStream};
use syn::{visit::Visit, Expr, File, Lit, Macro, Token};
use walkdir::{DirEntry, WalkDir};
type DynError = Box<dyn std::error::Error>;

const ADMIN_LOG_TOKENS_PATH: &str = "logging/log-tokens/src/admin_log_tokens.rs";
const HSM_LOG_TOKENS_PATH: &str = "logging/log-tokens/src/hsm_log_tokens.rs";

/// Get the repository root directory
fn get_repo_root() -> PathBuf {
    let manifest_path = std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set");
    Path::new(&manifest_path).parent().unwrap().to_path_buf()
}

/// Get the absolute path for admin log tokens file
fn get_admin_log_tokens_path() -> PathBuf {
    get_repo_root().join(ADMIN_LOG_TOKENS_PATH)
}

/// Get the absolute path for HSM log tokens file
fn get_hsm_log_tokens_path() -> PathBuf {
    get_repo_root().join(HSM_LOG_TOKENS_PATH)
}

/// A struct that collects macros and provides mappings between messages and indices.
///
/// # Fields
///
/// * `macros` - A vector of tuples where each tuple contains a macro name and a vector of associated strings(macro arguments).
/// * `message_to_index` - A hash map that maps messages (as strings) to their corresponding indices (as u8).
/// * `index_to_message` - A hash map that maps indices (as u8) to their corresponding messages (as strings).
/// * `index_not_found` - A vector to store the indexes which are not found in the current scan when compared to the existing hashmap.
/// * `prefix` - A string that represents the prefix for the macro.
/// * `macro_types` - A vector of strings that represents the types of macros to be collected.
/// * `index_to_prefix` - A hash map that maps indices (as u8) to their corresponding prefixes (as strings).
/// * `message_index_buffer` - A buffer that stores the index of the messages. This is used to check for duplicate messages.
struct MacroCollector {
    macros: Vec<(String, Vec<String>)>,
    message_to_index: HashMap<String, u8>,
    index_to_message: HashMap<u8, String>,
    index_not_found: Vec<u8>,
    prefix: String,
    macro_types: Vec<String>,
    index_to_prefix: HashMap<u8, String>,
    message_index_buffer: [u8; 256],
}

/// A struct that represents the content of a macro.
///
/// # Fields
///
/// * `message` - A string that represents the message of the macro.
/// * `args` - A vector of expressions that represents the arguments of the macro.
struct MacroContent {
    message: String,
    args: Vec<Expr>,
}

/// Implement the Parse trait for the MacroContent struct.
impl Parse for MacroContent {
    fn parse(input: ParseStream) -> syn::Result<Self> {
        let message: Lit = input.parse()?;
        let args = if input.peek(Token![,]) {
            input.parse::<Token![,]>()?;
            let args: syn::punctuated::Punctuated<Expr, Token![,]> =
                input.parse_terminated(Expr::parse, Token![,])?;
            args.into_iter().collect()
        } else {
            Vec::new()
        };
        Ok(MacroContent {
            message: match message {
                Lit::Str(lit_str) => lit_str.value(),
                _ => {
                    return Err(syn::Error::new_spanned(
                        message,
                        "Expected a string literal",
                    ))
                }
            },
            args,
        })
    }
}

/// Implement the Visit trait for the MacroCollector struct.
impl<'ast> Visit<'ast> for MacroCollector {
    fn visit_macro(&mut self, mac: &'ast Macro) {
        let macro_name = mac.path.segments.last().unwrap().ident.to_string();

        // Skip anything with a module prefix (e.g., log::warn!, crate::error!, mymod::info!)
        // and skip paths starting with a leading ::.
        let has_no_path_qualifier =
            mac.path.segments.len() == 1 && mac.path.leading_colon.is_none();

        if self.macro_types.iter().any(|ty| ty == &macro_name) && has_no_path_qualifier {
            let macro_content = mac.tokens.to_string();
            let parsed_macro: MacroContent =
                syn::parse_str(&macro_content).expect("Failed to parse macro");
            let message = parsed_macro.message.to_string();
            let args = parsed_macro
                .args
                .into_iter()
                .map(|arg| arg.to_token_stream().to_string())
                .collect();
            self.macros.push((message.clone(), args));

            let index;
            if !self.message_to_index.contains_key(&message) {
                index = self.message_to_index.len() as u8;
                self.message_to_index.insert(message.clone(), index);
                self.index_to_message.insert(index, message.clone());
                self.index_to_prefix.insert(index, self.prefix.clone());
            } else {
                index = self.message_to_index[&message];
            }

            self.message_index_buffer[index as usize] += 1;

            if self.message_index_buffer[index as usize] > 1 {
                // If the message is already in the existing messages, panic
                panic!(
                    "{}: Duplicate message found: {}",
                    "Error".bright_red(),
                    message.clone()
                );
            }
        }
        // Continue traversing the rest of the file
        syn::visit::visit_macro(self, mac);
    }
}

/// Parses a Rust source string and extracts a HashMap<String, u8> from a lazy_static block.
///
/// # Arguments
/// * `src` - The Rust source code as a string.
/// * `map_name` - The name of the HashMap to extract (e.g., "MANTICORE_ADMIN_LOG_TOKENS_MAP").
///
/// # Returns
/// * HashMap<String, u8> parsed from the source.
pub fn parse_hashmap_from_file(src: &str, map_name: &str) -> HashMap<String, u8> {
    let mut map = HashMap::new();

    // Regex to find the lazy_static block for the given map name
    let block_pattern = format!(
        r"(?s)pub\s+static\s+ref\s+{}\s*:\s*HashMap<[^>]+>\s*=\s*\{{\s*let\s+mut\s+m\s*=\s*HashMap::new\(\);\s*(.*?)\s*m\s*\}};",
        regex::escape(map_name)
    );
    let block_re = Regex::new(&block_pattern).unwrap();

    // Regex to match m.insert("key", value);
    let insert_re = Regex::new(r#"m\.insert\(\s*"((?:\\"|[^"])*)"\s*,\s*([0-9]+)\s*\);"#).unwrap();

    if let Some(caps) = block_re.captures(src) {
        let body = &caps[1];

        for line in body.lines() {
            if let Some(cap) = insert_re.captures(line) {
                let key = cap[1].to_string();
                if let Ok(value) = cap[2].parse::<u8>() {
                    map.insert(key, value);
                }
            }
        }
    }

    map
}

/// Parses a Rust source string and extracts a HashMap<u8, String> from a lazy_static block.
///
/// # Arguments
/// * `src` - The Rust source code as a string.
/// * `map_name` - The name of the HashMap to extract (e.g., "MANTICORE_ADMIN_LOG_TOKENS_INDEX_TO_MESSAGE_MAP").
///
/// # Returns
/// * HashMap<u8, String> parsed from the source.
pub fn parse_index_to_message_hashmap_from_file(src: &str, map_name: &str) -> HashMap<u8, String> {
    let mut map = HashMap::new();

    // Regex to find the lazy_static block for the given map name
    let block_pattern = format!(
        r"(?s)pub\s+static\s+ref\s+{}\s*:\s*HashMap<[^>]+>\s*=\s*\{{\s*let\s+mut\s+m\s*=\s*HashMap::new\(\);\s*(.*?)\s*m\s*\}};",
        regex::escape(map_name)
    );
    let block_re = Regex::new(&block_pattern).unwrap();

    // Regex to match m.insert(index, "message");
    let insert_re = Regex::new(r#"m\.insert\(\s*([0-9]+)\s*,\s*"((?:\\"|[^"])*)"\s*\);"#).unwrap();

    if let Some(caps) = block_re.captures(src) {
        let body = &caps[1];

        for line in body.lines() {
            if let Some(cap) = insert_re.captures(line) {
                if let Ok(index) = cap[1].parse::<u8>() {
                    let message = cap[2].to_string();
                    map.insert(index, message);
                }
            }
        }
    }

    map
}

/// Checkout the log tokens files from the repository.
///
/// This function checks out the log tokens files from the main branch of repository to ensure that
/// the tokenizer runs only on the last checked-in version and not on any local changes.
///
fn checkout_log_token_files() {
    let admin_log_tokens_path = get_admin_log_tokens_path();
    let hsm_log_tokens = get_hsm_log_tokens_path();

    // Checkout the log tokens files from the repository main branch
    if admin_log_tokens_path.exists() {
        println!("Checking out admin log tokens file...");
        std::process::Command::new("git")
            .args([
                "restore",
                "--source=main",
                admin_log_tokens_path.to_str().unwrap(),
            ])
            .status()
            .expect("Failed to checkout admin log tokens file");
    }

    if hsm_log_tokens.exists() {
        println!("Checking out HSM log tokens file...");
        std::process::Command::new("git")
            .args(["restore", "--source=main", hsm_log_tokens.to_str().unwrap()])
            .status()
            .expect("Failed to checkout HSM log tokens file");
    }
}

/// Tokenize the logging messages in the source code.
///
/// # Returns
///
/// * `Result` - A result that represents the success or failure of the operation.
pub(crate) fn telemetry_log_tokenizer() -> Result<(), DynError> {
    println!("Running: telemetry-log-tokenizer");

    // Checkout the log tokens files from the main branch of the repository
    // before running the tokenizer
    checkout_log_token_files();

    tokenize_logging();

    Ok(())
}

/// Convert the format specifiers from Rust to C.
///
/// # Arguments
///
/// * `input` - A string that represents the input message.
///
/// # Returns
///
/// * `String` - A string that represents the message with the format specifiers converted to C.
fn convert_format_specifiers(input: &str) -> String {
    let mut result = input.to_string();
    result = result.replace("{:?}", "%d");
    result = result.replace("{:x}", "0x%x");
    result = result.replace("{:X}", "0x%X");
    result = result.replace("{:#x}", "0x%x");
    result = result.replace("{:#X}", "0x%X");
    result = result.replace("0x{:08x}", "0x%08x");
    result = result.replace("{:08X?}", "%08X");
    result = result.replace("{}", "%d");
    result = result.replace("\n", " ");
    result
}

/// Load the index_to_message hashmap from log_tokens.rs files
fn load_index_to_message_map(module_name: &str) -> Option<HashMap<u8, String>> {
    match module_name {
        "MANTICORE_ADMIN_LOG_TOKENS" => {
            let src =
                std::fs::read_to_string(get_admin_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_index_to_message_hashmap_from_file(
                &src,
                format!("{}_INDEX_TO_MESSAGE_MAP", module_name).as_str(),
            ))
        }
        "MANTICORE_HSM_LOG_TOKENS" => {
            let src = std::fs::read_to_string(get_hsm_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_index_to_message_hashmap_from_file(
                &src,
                format!("{}_INDEX_TO_MESSAGE_MAP", module_name).as_str(),
            ))
        }
        _ => None,
    }
}

/// Load the message_to_index hashmap from log_tokens.rs files
///
/// # Arguments
///
/// * `module_name` - A string that represents the module name.
///
/// # Returns
///
/// * `Option` - An optional reference to the hashmap that maps messages to their corresponding indices.
fn load_message_to_index_map(module_name: &str) -> Option<HashMap<String, u8>> {
    match module_name {
        "MANTICORE_ADMIN_LOG_TOKENS" => {
            let src =
                std::fs::read_to_string(get_admin_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_hashmap_from_file(
                &src,
                format!("{}_MAP", module_name).as_str(),
            ))
        }
        "MANTICORE_HSM_LOG_TOKENS" => {
            let src = std::fs::read_to_string(get_hsm_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_hashmap_from_file(
                &src,
                format!("{}_MAP", module_name).as_str(),
            ))
        }
        _ => None,
    }
}

/// Load the index_to_prefix hashmap from log_tokens.rs files
///
/// # Arguments
///
/// * `module_name` - A string that represents the module name.
///
/// # Returns
///
/// * `Option` - An optional reference to the hashmap that maps indices to their corresponding prefixes.
fn load_index_to_prefix_map(module_name: &str) -> Option<HashMap<u8, String>> {
    match module_name {
        "MANTICORE_ADMIN_LOG_TOKENS" => {
            let src =
                std::fs::read_to_string(get_admin_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_index_to_message_hashmap_from_file(
                &src,
                format!("{}_INDEX_TO_PREFIX_MAP", module_name).as_str(),
            ))
        }
        "MANTICORE_HSM_LOG_TOKENS" => {
            let src = std::fs::read_to_string(get_hsm_log_tokens_path().to_str().unwrap()).unwrap();

            Some(parse_index_to_message_hashmap_from_file(
                &src,
                format!("{}_INDEX_TO_PREFIX_MAP", module_name).as_str(),
            ))
        }
        _ => None,
    }
}

/// Load existing message_to_index and index_to_message hashmap from log_tokens.rs files
///
/// # Arguments
///
/// * `collector` - A mutable reference to the MacroCollector struct.
/// * `module_name` - A string that represents the module name.
fn load_existing_hash_maps(collector: &mut MacroCollector, module_name: &str) {
    if let Some(message_to_index) = load_message_to_index_map(module_name) {
        collector.message_to_index = message_to_index;
    }

    if let Some(index_to_message) = load_index_to_message_map(module_name) {
        collector.index_to_message = index_to_message;
    }

    if let Some(index_to_prefix) = load_index_to_prefix_map(module_name) {
        collector.index_to_prefix = index_to_prefix;
    }
}

/// Populate the hash map with the logging macros from the source code files.
///
/// # Arguments
///
/// * `dir_name` - A string that represents the directory name.
/// * `root_dir` - A string that represents the root directory.
/// * `extensions` - A slice of strings that represents the file extensions to search for.
/// * `exclude_folders` - An optional slice of strings that represents the folders to exclude from the search.
fn populate_hash_map(
    dir_name: &str,
    root_dir: &str,
    extensions: &[&str],
    exclude_folders: Option<&[&str]>,
    collector: &mut MacroCollector,
) {
    let module_name = format!("MANTICORE_{}_LOG_TOKENS", dir_name.to_uppercase());

    println!("Module name: {}", module_name);

    for entry in WalkDir::new(root_dir)
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| {
            has_extension(e, extensions)
                && not_in_paths(e.path().to_str().unwrap(), exclude_folders)
        })
    {
        let path = entry.path();
        print!("\t {}\r", entry.path().display());

        if path.is_file() {
            search_file(path, collector);
        }
        print!(
            "\t {}\r",
            " ".repeat(entry.path().display().to_string().len())
        );
    }
}

/// Search the folder for files with the specified extensions and exclude certain folders.
///
/// # Arguments
///
/// * `root_dir` - A string that represents the root directory to search.
/// * `extensions` - A slice of strings that represents the file extensions to search for.
/// * `exclude_folders` - An optional slice of strings that represents the folders to exclude from the search.
fn search_folder(
    root_dir: &str,
    extensions: &[&str],
    exclude_folders: Option<&[&str]>,
    collector: &mut MacroCollector,
) {
    println!("{} : {}", "Scanning".bright_blue(), root_dir.bright_white());

    let dir_name = if root_dir.contains("admin") {
        String::from("admin")
    } else if root_dir.contains("hsm") {
        String::from("hsm")
    } else {
        String::from("common")
    };

    match dir_name.as_str() {
        "admin" => populate_hash_map("admin", root_dir, extensions, exclude_folders, collector),
        "hsm" => populate_hash_map("hsm", root_dir, extensions, exclude_folders, collector),
        "common" => {
            if collector.macro_types[0].contains("admin") {
                populate_hash_map("admin", root_dir, extensions, exclude_folders, collector)
            } else {
                populate_hash_map("hsm", root_dir, extensions, exclude_folders, collector)
            }
        }
        _ => panic!("Unknown directory name"),
    }
}

/// Generate the .h file with the messages and indices.
///
/// # Arguments
///
/// * `collector` - A mutable reference to the MacroCollector struct.
/// * `module_name` - A string that represents the module name.
fn generate_h_file(collector: &mut MacroCollector, dir_name: &str) {
    let module_name = format!("MANTICORE_{}_LOG_TOKENS", dir_name.to_uppercase());
    // Generate the .h file
    let h_file = format!("export/{}.h", module_name.to_lowercase());
    let h_file_path = get_repo_root().join(h_file).display().to_string();

    // Ensure the export directory exists
    let export_dir = get_repo_root().join("export");
    if !export_dir.exists() {
        fs::create_dir_all(export_dir).expect("Unable to create export directory");
    }

    let mut h_file = fs::File::create(h_file_path.clone()).expect("Unable to create file");

    writeln!(
        h_file,
        "// Copyright (c) Microsoft Corporation. All rights reserved.\n\
			 // This is an auto-generated file. Please do not modify manually.\n"
    )
    .expect("Unable to write to file");

    writeln!(h_file, "#ifndef {}_H_", module_name.to_uppercase()).expect("Unable to write to file");
    writeln!(h_file, "#define {}_H_\n", module_name.to_uppercase())
        .expect("Unable to write to file");

    writeln!(h_file, "// *INDENT-OFF*").expect("Unable to write to file");
    writeln!(
        h_file,
        "const char *manticore_{}_log_tokens_str [] = {{",
        dir_name.to_lowercase()
    )
    .expect("Unable to write to file");

    for index in 0..collector.index_to_message.len() as u8 {
        if let Some(message) = collector.index_to_message.get(&index) {
            let converted_string = convert_format_specifiers(message);

            if collector.index_not_found.contains(&index) {
                writeln!(
                    h_file,
                    "    [{}] = \"{} {}\", // Deprecated message",
                    index,
                    collector.index_to_prefix.get(&index).unwrap(),
                    converted_string
                )
                .expect("Unable to write to file");
            } else {
                writeln!(
                    h_file,
                    "    [{}] = \"{} {}\",",
                    index,
                    collector.index_to_prefix.get(&index).unwrap(),
                    converted_string
                )
                .expect("Unable to write to file");
            }
        }
    }

    writeln!(h_file, "}};\n").expect("Unable to write to file");
    writeln!(h_file, "#endif // {}_H_", module_name.to_uppercase())
        .expect("Unable to write to file");
    writeln!(h_file, "// *INDENT-ON*").expect("Unable to write to file");
    println!("Generated C header file at: {}", h_file_path);
}

/// Generate the .rs file with the hashmaps.
///
/// # Arguments
///
/// * `collector` - A mutable reference to the MacroCollector struct.
/// * `dir_name` - A string that represents the directory name.
/// * `module_name` - A string that represents the module name.
fn generate_rs_file(collector: &mut MacroCollector, dir_name: &str) {
    let module_name = format!("MANTICORE_{}_LOG_TOKENS", dir_name.to_uppercase());

    let rs_file_path = match dir_name {
        "admin" => get_admin_log_tokens_path().display().to_string(),
        "hsm" => get_hsm_log_tokens_path().display().to_string(),
        _ => panic!("Unknown directory name: {}", dir_name),
    };
    let mut rs_file = fs::File::create(rs_file_path.clone()).expect("Unable to create file");

    writeln!(
        rs_file,
        "// Copyright (c) Microsoft Corporation. All rights reserved.\n\
		 // This is an auto-generated file. Please do not modify manually.\n\
		 // To regenerate use command: `cargo xtask telemetry-tokenize`\n"
    )
    .expect("Unable to write to file");
    writeln!(rs_file, "use hashbrown::HashMap;\n").expect("Unable to write to file");
    writeln!(rs_file, "lazy_static::lazy_static! {{").expect("Unable to write to file");
    writeln!(
        rs_file,
        "    pub static ref {}_MAP: HashMap<&'static str, u8> = {{",
        module_name
    )
    .expect("Unable to write to file");
    writeln!(rs_file, "        let mut m = HashMap::new();").expect("Unable to write to file");

    let mut entries: Vec<(&String, &u8)> = collector.message_to_index.iter().collect();
    // Sort the vector by the index (value)
    entries.sort_by(|a, b| a.1.cmp(b.1));

    for (message, index) in entries {
        if collector.index_not_found.contains(index) {
            writeln!(
                rs_file,
                "        m.insert(\"{}\", {}); // Deprecated message",
                message, index
            )
            .expect("Unable to write to file");
        } else {
            writeln!(rs_file, "        m.insert(\"{}\", {});", message, index)
                .expect("Unable to write to file");
        }
    }

    writeln!(rs_file, "        m").expect("Unable to write to file");
    writeln!(rs_file, "    }};").expect("Unable to write to file");
    writeln!(rs_file, "}}\n").expect("Unable to write to file");

    writeln!(rs_file, "lazy_static::lazy_static! {{").expect("Unable to write to file");
    writeln!(
        rs_file,
        "    pub static ref {}_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {{",
        module_name
    )
    .expect("Unable to write to file");
    writeln!(rs_file, "        let mut m = HashMap::new();").expect("Unable to write to file");

    let mut entries: Vec<(&u8, &String)> = collector.index_to_message.iter().collect();
    entries.sort_by_key(|&(key, _)| key);

    for (index, message) in entries {
        if collector.index_not_found.contains(index) {
            writeln!(
                rs_file,
                "        m.insert({}, \"{}\"); // Deprecated message",
                index, message
            )
            .expect("Unable to write to file");
        } else {
            writeln!(rs_file, "        m.insert({}, \"{}\");", index, message)
                .expect("Unable to write to file");
        }
    }

    writeln!(rs_file, "        m").expect("Unable to write to file");
    writeln!(rs_file, "    }};").expect("Unable to write to file");
    writeln!(rs_file, "}}\n").expect("Unable to write to file");

    writeln!(rs_file, "lazy_static::lazy_static! {{").expect("Unable to write to file");
    writeln!(
        rs_file,
        "    pub static ref {}_INDEX_TO_PREFIX_MAP: HashMap<u8, &'static str> = {{",
        module_name
    )
    .expect("Unable to write to file");
    writeln!(rs_file, "        let mut m = HashMap::new();").expect("Unable to write to file");

    let mut entries: Vec<(&u8, &String)> = collector.index_to_prefix.iter().collect();
    entries.sort_by_key(|&(key, _)| key);

    // Print the sorted entries
    for (message, index) in entries {
        writeln!(rs_file, "        m.insert({}, \"{}\");", message, index)
            .expect("Unable to write to file");
    }

    writeln!(rs_file, "        m").expect("Unable to write to file");
    writeln!(rs_file, "    }};").expect("Unable to write to file");
    writeln!(rs_file, "}}\n").expect("Unable to write to file");

    writeln!(rs_file, "#[allow(dead_code)]").expect("Unable to write to file");

    writeln!(
        rs_file,
        "pub const {}: [&str; {}] = [",
        module_name,
        collector.index_to_message.len()
    )
    .expect("Unable to write to file");

    for index in 0..collector.index_to_message.len() as u8 {
        if let Some(message) = collector.index_to_message.get(&index) {
            writeln!(rs_file, "    \"{}\",", message).expect("Unable to write to file");
        }
    }

    writeln!(rs_file, "];").expect("Unable to write to file");
    println!("Generated Rust file at: {}", rs_file_path);
}

/// Search the file for logging macros.
///
/// # Arguments
///
/// * `path` - A reference to the Path struct that represents the file path.
/// * `collector` - A mutable reference to the MacroCollector struct.
fn search_file(path: &Path, collector: &mut MacroCollector) {
    println!("Searching file: {}", path.display());

    let code = fs::read_to_string(path).expect("Unable to read file");

    let syntax_tree: File = syn::parse_file(&code).unwrap_or_else(|err| {
        // If there is an error parsing the file, print the error and panic
        println!(
            "{} : {}",
            "Error received while parsing file".bright_yellow(),
            path.display().to_string().bright_white()
        );
        println!(
            "{}: {}",
            "Parse error".bright_red(),
            err.to_string().bright_white()
        );

        panic!("Unable to parse file: {}", path.display());
    });

    // Strip the repository root to get the relative path
    let repo_root = get_repo_root();
    let rel_path = path.strip_prefix(&repo_root).unwrap_or(path);

    // Find the crate name
    let mut components = rel_path.components();
    let crate_dir = components
        .next()
        .and_then(|c| c.as_os_str().to_str())
        .unwrap_or("unknown");

    let crate_name = match crate_dir {
        "admin" => "mcr_admin",
        "hsm" => "mcr_hsm",
        // During the refactor of the admin-app as a separate binary, we moved exception-handlers to a
        // common crate, which was originally under app/ dir. For the compatibility with the exisiting logs,
        // we rename the exception-handler prefix with app, which is used by the existing logs from the interrupt
        // handlers.
        "exception-handlers" => "app",
        other => other,
    };

    // Skip "src" if present, then collect the rest as module path
    let mut module_parts = Vec::new();
    for comp in components {
        let part = comp.as_os_str().to_str().unwrap();
        if part == "src" || part == "lib.rs" {
            continue;
        }
        module_parts.push(part);
    }

    // Remove .rs extension from the last part
    if let Some(last) = module_parts.last_mut() {
        if let Some(stripped) = last.strip_suffix(".rs") {
            *last = stripped;
        }
    }

    let module_path = if !module_parts.is_empty() {
        format!("[{}::{}]", crate_name, module_parts.join("::"))
    } else {
        format!("[{}]", crate_name)
    };
    collector.prefix = module_path;

    collector.visit_file(&syntax_tree);
}

/// Load the existing hash maps for admin and hsm once in the memory.
///
/// # Arguments
///
/// * `dir_name` - A string that represents the directory name.
/// * `collector` - A mutable reference to the MacroCollector struct.
fn load_hash_map(dir_name: &str, collector: &mut MacroCollector) {
    let module_name = format!("MANTICORE_{}_LOG_TOKENS", dir_name.to_uppercase());

    // let path: &str = &format!("./logging/log-tokens/src/{}_log_tokens.rs", dir_name);
    let path = match dir_name {
        "admin" => get_admin_log_tokens_path(),
        "hsm" => get_hsm_log_tokens_path(),
        _ => panic!("Unknown directory name"),
    };

    println!("Loading existing hash maps from: {}", path.display());

    // Check if the file exists before loading the hash maps
    if Path::new(&path).exists() {
        load_existing_hash_maps(collector, &module_name);
    } else {
        println!("The path does not exist");
    }

    // For "admin" and "hsm" folders, search for macros with types "info", "error", and "warn"
    // For other folders, search for macros with type "log_common_error_message"
    collector.macro_types = vec!["info".to_string(), "error".to_string(), "warn".to_string()];
}

/// Tokenize the logging messages in the source code.
fn tokenize_logging() {
    let mut admin_collector = MacroCollector {
        macros: Vec::new(),
        message_to_index: HashMap::new(),
        index_to_message: HashMap::new(),
        prefix: String::new(),
        macro_types: Vec::new(),
        index_to_prefix: HashMap::new(),
        message_index_buffer: [0; 256],
        index_not_found: Vec::new(),
    };
    let mut hsm_collector = MacroCollector {
        macros: Vec::new(),
        message_to_index: HashMap::new(),
        index_to_message: HashMap::new(),
        prefix: String::new(),
        macro_types: Vec::new(),
        index_to_prefix: HashMap::new(),
        message_index_buffer: [0; 256],
        index_not_found: Vec::new(),
    };

    // Load the existing hash maps for admin and hsm once in the memory
    load_hash_map("admin", &mut admin_collector);
    load_hash_map("hsm", &mut hsm_collector);

    search_folder(
        get_repo_root().join("admin").to_str().unwrap(),
        &["rs"],
        Some(&["xtask", "tests"]),
        &mut admin_collector,
    );
    search_folder(
        get_repo_root().join("hsm").to_str().unwrap(),
        &["rs"],
        Some(&["xtask", "tests"]),
        &mut hsm_collector,
    );

    // For common folder, search for macros with type "log_common_error_message"
    admin_collector.macro_types = vec!["log_admin_error_message".to_string()];
    hsm_collector.macro_types = vec!["log_hsm_error_message".to_string()];

    // Search for macros in the common folder and append it to the admin log tokens.
    search_folder(
        get_repo_root().join("exception-handlers").to_str().unwrap(),
        &["rs"],
        Some(&["xtask", "tests"]),
        &mut admin_collector,
    );
    // Search for macros in the common folder and append it to the hsm log tokens.
    search_folder(
        get_repo_root().join("exception-handlers").to_str().unwrap(),
        &["rs"],
        Some(&["xtask", "tests"]),
        &mut hsm_collector,
    );

    for (message, key) in &admin_collector.message_to_index {
        if !admin_collector
            .macros
            .iter()
            .any(|(info_macro_msg, _)| info_macro_msg == &message.to_string())
        {
            admin_collector.index_not_found.push(*key);
        }
    }

    for (message, key) in &hsm_collector.message_to_index {
        if !hsm_collector
            .macros
            .iter()
            .any(|(info_macro_msg, _)| info_macro_msg == &message.to_string())
        {
            hsm_collector.index_not_found.push(*key);
        }
    }

    // Generate .rs and .h files after scanning all folders
    generate_h_file(&mut admin_collector, "admin");
    generate_rs_file(&mut admin_collector, "admin");

    generate_h_file(&mut hsm_collector, "hsm");
    generate_rs_file(&mut hsm_collector, "hsm");
}

/// Check if the file has the specified extension.
///
/// # Arguments
///
/// * `entry` - A reference to the DirEntry struct that represents the file entry.
/// * `extensions` - A slice of strings that represents the file extensions to search for.
///
/// # Returns
///
/// * `bool` - A boolean that represents if the file has the specified extension.
fn has_extension(entry: &DirEntry, extensions: &[&str]) -> bool {
    entry
        .path()
        .extension()
        .and_then(|ext| ext.to_str())
        .map(|ext| extensions.contains(&ext))
        .unwrap_or(false)
}

/// Check if the file is not in the specified paths.
///
/// # Arguments
///
/// * `entry` - A string that represents the file entry.
/// * `ignored_paths` - An optional slice of strings that represents the paths to ignore.
///
/// # Returns
///
/// * `bool` - A boolean that represents if the file is not in the specified paths.
fn not_in_paths(entry: &str, ignored_paths: Option<&[&str]>) -> bool {
    if let Some(ignored_paths) = ignored_paths {
        !ignored_paths.iter().any(|path| entry.contains(path))
    } else {
        true
    }
}
