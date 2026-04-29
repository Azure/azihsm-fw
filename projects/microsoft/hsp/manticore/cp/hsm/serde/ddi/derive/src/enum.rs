// Copyright (c) Microsoft Corporation. All rights reserved.

use darling::ast;
use darling::FromDeriveInput;
use darling::FromVariant;

#[derive(FromVariant)]
#[darling(attributes(ddi))]
struct DdiEnumFieldAttr {
    ident: syn::Ident,
    discriminant: Option<syn::Expr>,
    id: u32,
}

#[derive(FromDeriveInput)]
#[darling(attributes(ddi), supports(enum_any))]
struct DdiEnumAttr {
    ident: syn::Ident,
    data: ast::Data<DdiEnumFieldAttr, ()>,
}

pub(crate) struct DdiEnumVariant {
    pub ident: syn::Ident,
    pub _discriminant: Option<syn::Expr>,
    pub id: u32,
}
pub(crate) struct DdiEnum {
    pub ident: syn::Ident,
    pub variants: Vec<DdiEnumVariant>,
}

pub(crate) fn prase_enum(input: &syn::DeriveInput) -> syn::Result<DdiEnum> {
    let enum_attr = DdiEnumAttr::from_derive_input(input)?;
    let variants = if let Some(vs) = enum_attr.data.take_enum() {
        vs.into_iter()
            .map(|v| DdiEnumVariant {
                ident: v.ident,
                _discriminant: v.discriminant,
                id: v.id,
            })
            .collect::<Vec<_>>()
    } else {
        vec![]
    };

    Ok(DdiEnum {
        ident: enum_attr.ident,
        variants,
    })
}
