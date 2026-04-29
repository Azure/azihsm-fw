// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_mbor::MborByteArray;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// Memory Address
#[repr(C)]
#[derive(Default, Clone, Copy, IntoBytes, Immutable, FromBytes)]
pub struct MemoryAddr {
    /// Lower 32-bit of the Memory Address
    pub lo: u32,

    /// Upper 32-bit of the Memory Address
    pub hi: u32,
}

impl From<&[u8]> for MemoryAddr {
    fn from(value: &[u8]) -> Self {
        let addr: u64 = value.as_ptr() as u64;
        Self {
            lo: addr as u32,
            hi: (addr >> 32) as u32,
        }
    }
}

impl MemoryAddr {
    pub fn slice(&self, len: usize) -> &[u8] {
        let addr = self.lo as usize | (self.hi as usize).wrapping_shl(32);
        unsafe { core::slice::from_raw_parts(addr as *const u8, len) }
    }

    pub fn slice_mut(&mut self, len: usize) -> &mut [u8] {
        let addr = self.lo as usize | (self.hi as usize).wrapping_shl(32);
        unsafe { core::slice::from_raw_parts_mut(addr as *mut u8, len) }
    }
}

#[derive(Copy, Clone, PartialEq, Eq)]
pub struct IoMemRange {
    addr: *const u8,
    len: usize,
}

impl IoMemRange {
    pub fn addr(&self) -> *const u8 {
        self.addr
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn is_empty(&self) -> bool {
        self.addr.is_null() || self.len == 0
    }

    pub fn slice(&self) -> &[u8] {
        unsafe { core::slice::from_raw_parts(self.addr, self.len) }
    }

    pub fn slice_mut(&mut self) -> &mut [u8] {
        unsafe { core::slice::from_raw_parts_mut(self.addr as *mut u8, self.len) }
    }
}

impl<const N: usize> From<&MborByteArray<N>> for IoMemRange {
    fn from(value: &MborByteArray<N>) -> Self {
        Self {
            addr: value.ptr(),
            len: value.len(),
        }
    }
}

impl From<&[u8]> for IoMemRange {
    fn from(value: &[u8]) -> Self {
        Self {
            addr: value.as_ptr(),
            len: value.len(),
        }
    }
}

impl From<&mut [u8]> for IoMemRange {
    fn from(value: &mut [u8]) -> Self {
        Self {
            addr: value.as_ptr(),
            len: value.len(),
        }
    }
}
