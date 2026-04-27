// Copyright (c) Microsoft Corporation. All rights reserved.

use core::alloc::Layout;
use core::ptr::NonNull;

use mcr_types::MemoryAddr;

const PAGE_SIZE: usize = 4 * 1024;

/// 4KB Aligned Page
pub(crate) struct Page {
    layout: Layout,
    len: usize,
    ptr: NonNull<u8>,
}

impl Drop for Page {
    /// Executes the destructor for this type.
    fn drop(&mut self) {
        unsafe { std::alloc::dealloc(self.ptr.as_ptr(), self.layout) }
    }
}

impl Page {
    /// Create a new page
    pub fn new() -> Option<Self> {
        let (ptr, layout) = unsafe {
            let layout = Layout::from_size_align_unchecked(PAGE_SIZE, PAGE_SIZE);
            (std::alloc::alloc(layout), layout)
        };

        if ptr.is_null() {
            None
        } else {
            Some(Self {
                layout,
                ptr: NonNull::new(ptr).unwrap(),
                len: PAGE_SIZE,
            })
        }
    }

    /// Get the length the data within the page
    pub fn len(&self) -> usize {
        self.len
    }

    /// Set the length of the data within the page
    pub fn set_len(&mut self, len: usize) {
        assert!(len <= PAGE_SIZE);
        self.len = len
    }

    /// Get the capacity of the page
    pub fn cap(&self) -> usize {
        PAGE_SIZE
    }

    /// Get the reference to the page
    pub fn slice(&self) -> &[u8] {
        unsafe {
            &core::slice::from_raw_parts(self.ptr.as_ptr() as *const u8, PAGE_SIZE)[..self.len]
        }
    }

    /// Get the mutable reference to the page
    pub fn slice_mut(&mut self) -> &mut [u8] {
        unsafe { &mut core::slice::from_raw_parts_mut(self.ptr.as_ptr(), PAGE_SIZE)[..self.len] }
    }

    /// Get the address of the page
    pub fn addr(&self) -> MemoryAddr {
        MemoryAddr {
            lo: self.ptr.as_ptr() as u32,
            #[cfg(target_pointer_width = "64")]
            hi: (self.ptr.as_ptr() as usize >> 32) as u32,
            #[cfg(target_pointer_width = "32")]
            hi: 0,
        }
    }
}
