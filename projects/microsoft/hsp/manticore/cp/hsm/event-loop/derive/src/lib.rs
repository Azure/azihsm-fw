// Copyright (c) Microsoft Corporation. All rights reserved.

use darling::ast;
use darling::FromDeriveInput;
use darling::FromVariant;
use proc_macro::TokenStream;
use quote::quote;
use syn::parse_quote;
use syn::DeriveInput;
use syn::Expr;
use syn::Ident;
use syn::Path;
use syn::Visibility;

#[derive(FromVariant, Clone)]
#[darling(attributes(event))]
struct EventVariant {
    ident: Ident,
    discriminant: Option<Expr>,
    interrupt: Path,
    sensitivity: Option<Path>,
    group: Option<String>,
}

#[derive(FromDeriveInput)]
#[darling(attributes(Event), supports(enum_any))]
struct EventEnum {
    vis: Visibility,
    ident: Ident,
    data: ast::Data<EventVariant, ()>,
}

#[proc_macro_derive(Event, attributes(event))]
pub fn event(input: TokenStream) -> TokenStream {
    let input: DeriveInput = syn::parse(input).unwrap();

    let event_enum = EventEnum::from_derive_input(&input).unwrap();

    let _vis = &event_enum.vis;
    let enum_name = &event_enum.ident;
    let variants = event_enum.data.take_enum().unwrap();
    let match_arms = variants.iter().cloned().enumerate().map(|(i, v)| {
        if v.discriminant.is_some() {
            quote!("Variant must not have discriminat")
        } else {
            let ident = v.ident;
            let discriminat = i;
            quote! {
                #discriminat => #enum_name::#ident,
            }
        }
    });

    let irqs: Vec<Path> = variants.iter().map(|e| e.interrupt.clone()).collect();
    let sensitivity: Vec<Path> = variants
        .iter()
        .map(|e| {
            e.sensitivity
                .clone()
                .unwrap_or_else(|| parse_quote!(IrqSensitivity::Level))
        })
        .collect();
    let group: Vec<String> = variants
        .iter()
        .map(|e| e.group.clone().unwrap_or_else(|| "None".to_string()))
        .collect();

    let tokens = quote! {
        impl mcr_event_loop::Event for #enum_name {
            fn interrupts() -> Vec<IrqGroup> {
                let irqs = [#(#irqs),*];
                let sensitivity = [#(#sensitivity),*];
                let group = [#(#group),*];
                let mut irq_groups: Vec<IrqGroup> = Vec::new();

                for (index, ((irq, sensitivity), group)) in irqs
                    .iter()
                    .zip(sensitivity.iter())
                    .zip(group.iter())
                    .enumerate()
                {
                    let irq_info = IrqInfo {
                        num: *irq,
                        sensitivity: *sensitivity,
                        event_num: index,
                    };

                    if *group != "None" {
                        let irq_group = irq_groups
                            .iter_mut()
                            .find(|g| g.id == Some(group.to_string()));

                        if let Some(existing_group) = irq_group {
                            existing_group.interrupts.push(irq_info);
                        } else {
                            irq_groups.push(IrqGroup {
                                id: Some(group.to_string()),
                                interrupts: vec![irq_info],
                                next: 0,
                            });
                        }
                    } else {
                        irq_groups.push(IrqGroup {
                            id: None,
                            interrupts: vec![irq_info],
                            next: 0,
                        });
                    }
                }

                irq_groups
            }
        }

        impl From<#enum_name> for usize {
            fn from(val: #enum_name) -> Self {
                val as Self
            }
        }

        impl From<usize> for #enum_name {
            fn from(val: usize) -> Self {
                match val {
                    #(#match_arms)*
                    _ => panic!("Invalid discriminant")
                }
            }
        }
    };

    tokens.into()
}
