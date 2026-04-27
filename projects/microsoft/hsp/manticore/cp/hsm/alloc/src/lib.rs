// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

extern crate alloc;

use alloc::rc::Rc;
use alloc::slice;
use core::alloc::Layout;
use core::cell::RefCell;
use core::ptr::NonNull;

use linked_list_allocator::Heap;

pub trait DmaAllocTrait {
    fn as_ref(&self) -> &[u8];

    fn as_ref_mut(&mut self) -> &mut [u8];

    fn len(&self) -> usize;

    fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

/// Heap Interface
pub trait DmaHeapTrait: Sized {
    /// Allocation type
    type Alloc: DmaAllocTrait;

    /// Allocate memory on the heap and put `x` at the location
    ///
    /// # Arguments
    ///
    /// * `size` - The size of the allocation
    ///
    /// # Returns
    ///
    /// * `Option<McrBox<T>>` - The allocation
    fn allocate(&self, size: usize) -> Option<Self::Alloc>;

    /// Allocate memory and copy the slice into the allocation
    ///
    /// # Arguments
    ///
    /// * `slice` - The slice to copy
    ///
    /// # Returns
    ///
    /// * `Option<McrBox<T>>` - The allocation
    fn copy_allocate(&self, slice: &[u8]) -> Option<Self::Alloc> {
        let mut alloc = self.allocate(slice.len())?;
        alloc.as_ref_mut().copy_from_slice(slice);
        Some(alloc)
    }

    /// Get size of the heap
    fn size(&self) -> usize;

    /// Get free space in the heap
    fn free(&self) -> usize;

    /// Free the allocation
    fn deallocate(&self, ptr: NonNull<u8>, layout: Layout);
}

/// Heap
#[derive(Clone)]
pub struct DmaHeap {
    rimpl: Rc<RefCell<McrHeapImpl>>,
}

impl DmaHeap {
    /// Create an instance of `McrHeap`
    ///
    /// # Arguments
    ///
    /// * `start_addr` - Start Address
    /// * `size` - Size of the heap
    pub fn new(start_addr: usize, size: usize) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(McrHeapImpl::new(start_addr, size))),
        }
    }
}

impl DmaHeapTrait for DmaHeap {
    type Alloc = DmaAlloc<Self>;

    /// Allocate memory on the heap and put `x` at the location
    #[inline(always)]
    fn allocate(&self, size: usize) -> Option<DmaAlloc<Self>> {
        self.rimpl.borrow_mut().allocate(size, self.clone())
    }

    /// Get size of the heap
    fn size(&self) -> usize {
        self.rimpl.borrow().heap.size()
    }

    /// Get free space in the heap
    fn free(&self) -> usize {
        self.rimpl.borrow().heap.free()
    }

    /// Free the allocation
    #[inline(always)]
    fn deallocate(&self, ptr: NonNull<u8>, layout: Layout) {
        self.rimpl.borrow_mut().deallocate(ptr, layout)
    }
}

/// Heap implementation
struct McrHeapImpl {
    heap: Heap,
}

impl McrHeapImpl {
    /// Create an instance of `McrHeap`
    fn new(start_addr: usize, size: usize) -> Self {
        Self {
            heap: unsafe { Heap::new(start_addr as *mut u8, size) },
        }
    }

    /// Allocate memory on the heap and put `x` at the location
    #[inline(always)]
    fn allocate(&mut self, size: usize, heap: DmaHeap) -> Option<DmaAlloc<DmaHeap>> {
        let layout = Layout::array::<u8>(size).unwrap();
        if let Ok(ptr) = self.heap.allocate_first_fit(layout) {
            let slice = unsafe { slice::from_raw_parts_mut(ptr.as_ptr(), layout.size()) };
            slice.fill(0);
            Some(DmaAlloc { ptr, layout, heap })
        } else {
            None
        }
    }

    /// Free the allocation
    #[inline(always)]
    fn deallocate(&mut self, ptr: NonNull<u8>, layout: Layout) {
        let slice = unsafe { slice::from_raw_parts_mut(ptr.as_ptr(), layout.size()) };
        slice.fill(0);
        unsafe {
            self.heap.deallocate(ptr, layout);
        }
    }
}

/// Heap Allocation
pub struct DmaAlloc<H: DmaHeapTrait> {
    ptr: NonNull<u8>,
    layout: Layout,
    heap: H,
}

impl<H: DmaHeapTrait> Drop for DmaAlloc<H> {
    /// Executes the destructor for this type.
    fn drop(&mut self) {
        self.heap.deallocate(self.ptr, self.layout)
    }
}

impl<H: DmaHeapTrait> DmaAllocTrait for DmaAlloc<H> {
    fn as_ref(&self) -> &[u8] {
        unsafe { slice::from_raw_parts(self.ptr.as_ptr(), self.layout.size()) }
    }

    fn as_ref_mut(&mut self) -> &mut [u8] {
        unsafe { slice::from_raw_parts_mut(self.ptr.as_ptr(), self.layout.size()) }
    }

    fn len(&self) -> usize {
        self.layout.size()
    }
}

#[cfg(test)]
mod tests {
    use core::mem::MaybeUninit;

    use super::*;

    const HEAP_SIZE: usize = 64 * 1024;

    #[test]
    fn test_heap_size() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);
        assert_eq!(heap.free(), HEAP_SIZE);
    }

    #[test]
    fn test_heap_allocate() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);

        let alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 4096]);
        assert_eq!(heap.free(), HEAP_SIZE - 4096);
    }

    #[test]
    fn test_heap_allocate_with_update() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);

        let mut alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 4096]);
        assert_eq!(heap.free(), HEAP_SIZE - 4096);
        alloc.as_mut().unwrap().as_ref_mut().fill(0xAB);
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0xABu8; 4096]);
    }

    #[test]
    fn test_heap_allocate_max() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);

        let alloc = heap.allocate(64 * 1024);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().len(), 64 * 1024);
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 64 * 1024]);
        assert_eq!(heap.free(), 0);
    }

    #[test]
    fn test_heap_allocate_failue() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);

        let alloc = heap.allocate(64 * 1024 + 1);
        assert!(alloc.is_none());
        assert_eq!(heap.free(), HEAP_SIZE);
    }

    #[test]
    fn test_heap_deallocate() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];

        let heap = DmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE);
        assert_eq!(heap.size(), HEAP_SIZE);

        let alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(heap.free(), HEAP_SIZE - 4096);

        drop(alloc);
        assert_eq!(heap.free(), HEAP_SIZE);
    }
}
