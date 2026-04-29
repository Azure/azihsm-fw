// Copyright (c) Microsoft Corporation. All rights reserved.

mod attributes;
mod entry;
mod key_number;
mod key_store;
mod table;
mod tracker;

use alloc::rc::Rc;
use core::cell::Ref;
use core::cell::RefCell;

use crate::error::HsmErr;
pub(crate) use attributes::*;
use bitfield_struct::bitfield;
pub(crate) use entry::*;
pub(crate) use key_number::*;
pub(crate) use key_store::*;
pub(crate) use table::*;
use tracker::*;
use zerocopy::*;

/// The maximum number of tables across all key stores.
pub(crate) const MAX_TABLE_COUNT: usize = 65;

/// Maximum number of entries in the table.
pub(crate) const MAX_TABLE_ENTRY_COUNT: usize = 256;

/// Maximum number of bytes in the table.
pub(crate) const TOTAL_TABLE_LEN: usize = 17 * 1024;

/// Size of the attributes blob for each entry.
pub(crate) const ATTRIBUTES_BLOB_SIZE: usize =
    COMMON_ATTRIBUTES_BLOB_SIZE + ENTRY_SPECIFIC_ATTRIBUTES_BLOB_SIZE;

// Compile time assertion to ensure the attributes blob size is correct.
static_assertions::const_assert_eq!(ATTRIBUTES_BLOB_SIZE, 32);
