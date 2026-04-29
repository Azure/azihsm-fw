// Copyright (c) Microsoft Corporation. All rights reserved.

#![allow(clippy::manual_unwrap_or_default)]

use darling::export::NestedMeta;
use darling::FromMeta;
use proc_macro::TokenStream;
use quote::format_ident;
use quote::quote;
use syn::parse_macro_input;
use syn::ItemStruct;

#[derive(FromMeta)]
struct MemMapAttr {
    address: syn::LitInt,
    length: syn::LitInt,
}

#[derive(FromMeta)]
struct FieldAttr {
    #[darling(default)]
    alignment: usize,

    #[darling(default)]
    cardinality: usize,

    offset: Option<syn::LitInt>,

    #[darling(default)]
    mutable: bool,

    #[darling(default)]
    volatile: bool,
}

#[derive(FromMeta)]
struct Field {
    field: FieldAttr,
}

#[proc_macro_attribute]
pub fn mem_map(attrs: TokenStream, input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as ItemStruct);
    let mem_attrs: MemMapAttr = parse_attrs(attrs);

    let base_address = mem_attrs.address;
    let length = mem_attrs.length;
    let name = input.ident;
    let vis = input.vis;

    let mut prev_field_offset = format_ident!("BASE_ADDRESS");
    let mut prev_field_size = format_ident!("ZERO_SIZE");
    let mut offset = None;

    let mut consts = vec![];
    let mut funcs = vec![];
    for f in input.fields.iter() {
        let field_ident = f.ident.as_ref().unwrap();
        let field_name = f.ident.as_ref().unwrap().to_string();
        let curr_field_offset = format_ident!("{}_OFFSET", field_name.to_uppercase());
        let curr_field_size = format_ident!("{}_SIZE", field_name.to_uppercase());
        let ty = f.ty.clone();
        let mut alignment = 4;
        let mut mutable = false;
        let mut cardinality = 1;
        let mut volatile = false;

        for meta in f.attrs.iter().map(|a| &a.meta) {
            if meta.path().is_ident("field") {
                let field_attr = parse_attrs::<Field>(quote!(#meta).into());
                offset = field_attr.field.offset;
                mutable = field_attr.field.mutable;
                volatile = field_attr.field.volatile;
                if field_attr.field.cardinality > 0 {
                    cardinality = field_attr.field.cardinality;
                }
                if field_attr.field.alignment > 0 {
                    alignment = field_attr.field.alignment;
                }
            }
        }

        if let Some(ref offset) = offset {
            consts.push(quote!(
                const #curr_field_offset: usize = (Self::BASE_ADDRESS + #offset -1 + #alignment) & !(#alignment - 1);
            ));
        } else {
            consts.push(quote!(
                const #curr_field_offset: usize = (Self::#prev_field_offset + Self::#prev_field_size -1 + #alignment) & !(#alignment - 1);
            ));
        }

        consts.push(quote!(
            const #curr_field_size: usize = core::mem::size_of::<[#ty; #cardinality]>();
        ));

        if volatile {
            assert_eq!(
                "u32",
                quote!(#ty).to_string(),
                "Only u32 is supported for volatile fields"
            );
            funcs.push(quote! {
                #[inline(always)]
                pub fn #field_ident() -> &'static mcr_types::VolatileCell<#ty> {
                    let ptr = Self::#curr_field_offset as *const #ty;
                    let reg: &mcr_types::VolatileCell<#ty> = unsafe {
                        #[allow(clippy::transmute_ptr_to_ref)]
                        core::mem::transmute(ptr)
                    };
                    reg
                }
            });
        } else if mutable {
            funcs.push(quote! {
                    #[inline(always)]
                    pub fn #field_ident() -> &'static mut [#ty] {
                        unsafe { core::slice::from_raw_parts_mut(Self::#curr_field_offset as *mut #ty, #cardinality) }
                    }
                });
        } else {
            funcs.push(quote! {
                #[inline(always)]
                    pub fn #field_ident() -> &'static [#ty] {
                        unsafe { core::slice::from_raw_parts(Self::#curr_field_offset as *const #ty, #cardinality) }
                    }
                });
        }

        prev_field_offset = curr_field_offset;
        prev_field_size = curr_field_size;
        offset = None;
    }

    quote! {
        #vis struct #name {}
        impl #name {
            const BASE_ADDRESS: usize = #base_address;
            const LENGTH: usize = #length;
            const ZERO_SIZE: usize = 0;
            #(#consts)*
            #(#funcs)*
        }
    }
    .into()
}

fn parse_attrs<T: FromMeta>(attrs: TokenStream) -> T {
    let items = NestedMeta::parse_meta_list(attrs.into()).unwrap();
    T::from_list(&items).unwrap()
}
