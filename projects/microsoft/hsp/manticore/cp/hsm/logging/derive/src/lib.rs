// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate proc_macro;
use mcr_log_tokens::{admin_log_tokens, hsm_log_tokens};
use proc_macro::TokenStream;
use quote::quote;
use std::env;
use syn::{parse_macro_input, Expr, ExprGroup, ExprLit, Lit, LitStr};

#[proc_macro]
pub fn get_admin_message_index(input: TokenStream) -> TokenStream {
    let message = parse_macro_input!(input as LitStr);
    let message = message.value();

    // Check if the telemetry tokenizer is disabled
    // If the environment variable is not set, we default to 0 (enabled)
    let disable_tokenizer: u8 = env::var("DISABLE_CP_TOKENIZER")
        .ok()
        .and_then(|val| val.parse::<u8>().ok())
        .unwrap_or(0);

    let index = if let Some(index) =
        admin_log_tokens::MANTICORE_ADMIN_LOG_TOKENS_MAP.get(&message.as_str())
    {
        index
    } else if disable_tokenizer == 1 {
        &255 // Return 255 if the tokenizer is disabled
    } else {
        panic!("String not found in the admin map. Please run `cargo xtask telemetry-tokenize`");
    };

    let expanded = quote! {
        #index
    };

    TokenStream::from(expanded)
}

#[proc_macro]
pub fn get_hsm_message_index(input: TokenStream) -> TokenStream {
    let message = parse_macro_input!(input as LitStr);
    let message = message.value();

    // Check if the telemetry tokenizer is disabled
    // If the environment variable is not set, we default to 0 (enabled)
    let disable_tokenizer: u8 = env::var("DISABLE_CP_TOKENIZER")
        .ok()
        .and_then(|val| val.parse::<u8>().ok())
        .unwrap_or(0);

    let index =
        if let Some(index) = hsm_log_tokens::MANTICORE_HSM_LOG_TOKENS_MAP.get(&message.as_str()) {
            index
        } else if disable_tokenizer == 1 {
            &255 // Return 255 if the tokenizer is disabled
        } else {
            panic!("String not found in the hsm map. Please run `cargo xtask telemetry-tokenize`");
        };

    let expanded = quote! {
        #index
    };

    TokenStream::from(expanded)
}

#[proc_macro]
pub fn get_message_index(input: TokenStream) -> TokenStream {
    let args = parse_macro_input!(input as syn::ExprTuple);

    if args.elems.len() != 2 {
        panic!("Expected two arguments: base_dir and message");
    }

    // Check if the telemetry tokenizer is disabled
    // If the environment variable is not set, we default to 0 (enabled)
    let disable_tokenizer: u8 = env::var("DISABLE_CP_TOKENIZER")
        .ok()
        .and_then(|val| val.parse::<u8>().ok())
        .unwrap_or(0);

    let base_dir = if let Expr::Group(ExprGroup { expr, .. }) = &args.elems[0] {
        if let Expr::Lit(ExprLit {
            lit: Lit::Str(ref lit_str),
            ..
        }) = **expr
        {
            lit_str.value()
        } else {
            panic!("Expected a string literal inside the group")
        }
    } else {
        "".to_string()
    };

    match base_dir.as_str() {
        "admin" => (),
        "hsm" => (),
        _ => panic!("Unknown module name"),
    }

    let message = if let Expr::Group(ExprGroup { expr, .. }) = &args.elems[1] {
        if let Expr::Lit(ExprLit {
            lit: Lit::Str(ref lit_str),
            ..
        }) = **expr
        {
            lit_str.value()
        } else {
            panic!("Expected a string literal inside the group")
        }
    } else {
        "".to_string()
    };

    // Try to fetch the index from the respective map based on the base_dir
    // If the index is not found and the tokenizer is disabled, return 255 (last valid index)
    // Otherwise, panic with an error message
    let index = match base_dir.as_str() {
        "admin" => {
            if let Some(index) =
                admin_log_tokens::MANTICORE_ADMIN_LOG_TOKENS_MAP.get(&message.as_str())
            {
                index
            } else if disable_tokenizer == 1 {
                &255
            } else {
                panic!("String not found in the admin map. Please run `cargo xtask telemetry-tokenize`")
            }
        }
        "hsm" => {
            if let Some(index) = hsm_log_tokens::MANTICORE_HSM_LOG_TOKENS_MAP.get(&message.as_str())
            {
                index
            } else if disable_tokenizer == 1 {
                &255
            } else {
                panic!(
                    "String not found in the hsm map. Please run `cargo xtask telemetry-tokenize`"
                )
            }
        }
        _ => panic!("Unknown module name. Could not find the message in the map"),
    };
    let expanded = quote! {
        #index
    };

    TokenStream::from(expanded)
}

#[proc_macro]
pub fn get_message_index_common_macro(input: TokenStream) -> TokenStream {
    let args = parse_macro_input!(input as syn::ExprTuple);

    if args.elems.len() != 2 {
        panic!("Expected two arguments: base_dir and message");
    }

    // Check if the telemetry tokenizer is disabled
    // If the environment variable is not set, we default to 0 (enabled)
    let disable_tokenizer: u8 = env::var("DISABLE_CP_TOKENIZER")
        .ok()
        .and_then(|val| val.parse::<u8>().ok())
        .unwrap_or(0);

    let base_dir = if let Expr::Lit(ExprLit {
        lit: Lit::Str(ref lit_str),
        ..
    }) = &args.elems[0]
    {
        lit_str.value()
    } else {
        panic!("Expected a string literal for base_dir")
    };

    match base_dir.as_str() {
        "admin" => (),
        "hsm" => (),
        _ => panic!("Unknown module name"),
    }

    let message = if let Expr::Group(ExprGroup { expr, .. }) = &args.elems[1] {
        if let Expr::Lit(ExprLit {
            lit: Lit::Str(ref lit_str),
            ..
        }) = **expr
        {
            lit_str.value()
        } else {
            panic!("Expected a string literal inside the group")
        }
    } else {
        "".to_string()
    };

    // Try to fetch the index from the respective map based on the base_dir
    // If the index is not found and the tokenizer is disabled, return 255 (last valid index)
    // Otherwise, panic with an error message
    let index = match base_dir.as_str() {
        "admin" => {
            if let Some(index) =
                admin_log_tokens::MANTICORE_ADMIN_LOG_TOKENS_MAP.get(&message.as_str())
            {
                index
            } else if disable_tokenizer == 1 {
                &255
            } else {
                panic!("String not found in the admin map. Please run `cargo xtask telemetry-tokenize`")
            }
        }
        "hsm" => {
            if let Some(index) = hsm_log_tokens::MANTICORE_HSM_LOG_TOKENS_MAP.get(&message.as_str())
            {
                index
            } else if disable_tokenizer == 1 {
                &255
            } else {
                panic!(
                    "String not found in the hsm map. Please run `cargo xtask telemetry-tokenize`"
                )
            }
        }
        _ => panic!("Unknown module name. Could not find the message in the map"),
    };
    let expanded = quote! {
        #index
    };

    TokenStream::from(expanded)
}

#[proc_macro]
pub fn get_component_id_from_directory(input: TokenStream) -> TokenStream {
    let dir_name = parse_macro_input!(input as LitStr);
    let dir_name = dir_name.value();

    let component = match dir_name.as_str() {
        "admin" => quote! { DebugLogComponent::MsftLoggingComponentManticoreCp0 },
        "hsm" => quote! { DebugLogComponent::MsftLoggingComponentManticoreCp1 },
        _ => panic!("Invalid component"),
    };

    let expanded = quote! {
        #component
    };

    TokenStream::from(expanded)
}
