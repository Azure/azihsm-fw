// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::rc::Rc;
use alloc::slice;
use alloc::vec::Vec;
use core::alloc::Layout;
use core::cell::RefCell;
use core::ptr::NonNull;

use linked_list_allocator::Heap;

/// HSM DMA Allocation Trait
pub trait HsmDmaAllocTrait {
    /// Get reference of the memory as a slice
    ///
    /// # Returns
    ///
    /// * `&[u8]` - The slice
    fn as_ref(&self) -> &[u8];

    /// Get reference of the memory as a mutable slice
    ///
    /// # Returns
    ///
    /// * `&mut [u8]` - The mutable slice
    fn as_ref_mut(&mut self) -> &mut [u8];

    /// Get the length of the allocation
    ///
    /// # Returns
    ///
    /// * `usize` - The length
    fn len(&self) -> usize;

    /// Check if length is zero
    ///
    /// # Returns
    ///
    /// * `bool` - True if length is zero, false otherwise
    #[allow(unused)]
    fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

/// HSM Heap Interface
pub trait HsmDmaHeapTrait: Sized {
    /// Allocation type
    type Alloc: HsmDmaAllocTrait;

    /// Allocate memory from the heap
    ///
    /// # Arguments
    ///
    /// * `size` - The size of the allocation
    ///
    /// # Returns
    ///
    /// * `Option<Self::Alloc>` - The allocation
    fn allocate(&self, size: usize) -> Option<Self::Alloc>;

    /// Allocate memory from the pre-created pool
    ///
    /// # Arguments
    ///
    /// * `size` - The size of the allocation
    ///
    /// # Returns
    ///
    /// * `Option<Self::Alloc>` - The allocation
    fn allocate_from_pool(&self, size: usize) -> Option<Self::Alloc>;

    /// Allocate memory from the heap and copy the slice into the allocation
    ///
    /// # Arguments
    ///
    /// * `slice` - The slice to copy
    ///
    /// # Returns
    ///
    /// * `Option<Self::Alloc>` - The allocation
    fn copy_allocate(&self, slice: &[u8]) -> Option<Self::Alloc> {
        let mut alloc = self.allocate(slice.len())?;
        alloc.as_ref_mut().copy_from_slice(slice);
        Some(alloc)
    }

    /// Get the size of the heap
    ///
    /// # Returns
    ///
    /// * `usize` - The size
    #[allow(unused)]
    fn size(&self) -> usize;

    /// Get free space in the heap
    ///
    /// # Returns
    ///
    /// * `usize` - The free space
    #[allow(unused)]
    fn free(&self) -> usize;

    /// Free the allocation
    ///
    /// # Arguments
    ///
    /// * `ptr` - The ptr of the allocation
    /// * `layout` - The layout of the allocation
    /// * `size` - The size of the allocation
    ///
    fn deallocate(&self, ptr: NonNull<u8>, layout: Layout, size: usize);
}

/// HSM Heap
#[derive(Clone)]
pub struct HsmDmaHeap {
    rimpl: Rc<RefCell<HsmDmaHeapImpl>>,
}

impl HsmDmaHeap {
    /// Create an instance of `HsmDmaHeap`
    ///
    /// # Arguments
    ///
    /// * `start_addr` - Start Address
    /// * `size` - Size of the heap
    pub fn new(start_addr: usize, size: usize) -> Self {
        let heap = Self {
            rimpl: Rc::new(RefCell::new(HsmDmaHeapImpl::new(start_addr, size))),
        };

        heap.rimpl.borrow_mut().populate_pool(heap.clone());

        heap
    }
}

impl HsmDmaHeapTrait for HsmDmaHeap {
    type Alloc = HsmDmaAlloc<Self>;

    /// Allocate memory from the heap
    #[inline(always)]
    fn allocate(&self, size: usize) -> Option<HsmDmaAlloc<Self>> {
        self.rimpl.borrow_mut().allocate(size, self.clone())
    }

    /// Allocate memory from the pre-created pool
    #[inline(always)]
    fn allocate_from_pool(&self, size: usize) -> Option<HsmDmaAlloc<Self>> {
        self.rimpl.borrow_mut().allocate_from_pool(size)
    }

    /// Get the size of the heap
    fn size(&self) -> usize {
        self.rimpl.borrow().heap.size()
    }

    /// Get free space in the heap
    fn free(&self) -> usize {
        self.rimpl.borrow().heap.free()
    }

    /// Free the allocation
    #[inline(always)]
    fn deallocate(&self, ptr: NonNull<u8>, layout: Layout, size: usize) {
        self.rimpl
            .borrow_mut()
            .deallocate(ptr, layout, self.clone(), size)
    }
}

struct HsmDmaHeapImpl {
    heap: Heap,
    pool: Vec<HsmDmaAlloc<HsmDmaHeap>>,
}

impl HsmDmaHeapImpl {
    const POOL_SIZE: usize = 64; // 32 for Request, 32 for Response
    const POOL_ALLOC_SIZE_REQ_RESP: usize = 3584; // 3.5 KB

    /// Create an instance of `HsmDmaHeapImpl`
    fn new(start_addr: usize, size: usize) -> Self {
        Self {
            heap: unsafe { Heap::new(start_addr as *mut u8, size) },
            pool: Vec::with_capacity(Self::POOL_SIZE),
        }
    }

    /// Populate the memory pool
    fn populate_pool(&mut self, heap: HsmDmaHeap) {
        let layout = Layout::array::<u8>(Self::POOL_ALLOC_SIZE_REQ_RESP).unwrap();
        for _ in 0..Self::POOL_SIZE {
            let ptr = self.heap.allocate_first_fit(layout).unwrap();
            let slice = unsafe { slice::from_raw_parts_mut(ptr.as_ptr(), layout.size()) };
            slice.fill(0);

            self.pool.push(HsmDmaAlloc {
                ptr,
                size: Self::POOL_ALLOC_SIZE_REQ_RESP,
                layout,
                heap: heap.clone(),
            });
        }
    }

    /// Allocate memory from the heap
    #[inline(always)]
    fn allocate(&mut self, size: usize, heap: HsmDmaHeap) -> Option<HsmDmaAlloc<HsmDmaHeap>> {
        let layout = Layout::array::<u8>(size).unwrap();
        if let Ok(ptr) = self.heap.allocate_first_fit(layout) {
            let slice = unsafe { slice::from_raw_parts_mut(ptr.as_ptr(), layout.size()) };
            slice.fill(0);
            Some(HsmDmaAlloc {
                ptr,
                layout,
                size,
                heap,
            })
        } else {
            None
        }
    }

    /// Allocate memory from the pre-created pool
    #[inline(always)]
    fn allocate_from_pool(&mut self, size: usize) -> Option<HsmDmaAlloc<HsmDmaHeap>> {
        if size <= Self::POOL_ALLOC_SIZE_REQ_RESP {
            let mut alloc = self.pool.pop()?;
            alloc.size = size;
            Some(alloc)
        } else {
            None
        }
    }

    /// Free the allocation
    #[inline(always)]
    fn deallocate(&mut self, ptr: NonNull<u8>, layout: Layout, heap: HsmDmaHeap, size: usize) {
        let slice = unsafe { slice::from_raw_parts_mut(ptr.as_ptr(), size) };
        slice.fill(0);

        if layout.size() == Self::POOL_ALLOC_SIZE_REQ_RESP && self.pool.len() < Self::POOL_SIZE {
            let alloc = HsmDmaAlloc {
                ptr,
                size: Self::POOL_ALLOC_SIZE_REQ_RESP,
                layout,
                heap,
            };
            self.pool.push(alloc);
        } else {
            unsafe {
                self.heap.deallocate(ptr, layout);
            }
        }
    }
}

/// Heap Allocation
pub struct HsmDmaAlloc<H: HsmDmaHeapTrait> {
    ptr: NonNull<u8>,
    size: usize,
    layout: Layout,
    heap: H,
}

impl<H: HsmDmaHeapTrait> Drop for HsmDmaAlloc<H> {
    /// Executes the destructor for this type.
    fn drop(&mut self) {
        self.heap.deallocate(self.ptr, self.layout, self.size)
    }
}

impl<H: HsmDmaHeapTrait> HsmDmaAllocTrait for HsmDmaAlloc<H> {
    /// Get reference of the memory as a slice
    fn as_ref(&self) -> &[u8] {
        unsafe { slice::from_raw_parts(self.ptr.as_ptr(), self.size) }
    }

    /// Get reference of the memory as a mutable slice
    fn as_ref_mut(&mut self) -> &mut [u8] {
        unsafe { slice::from_raw_parts_mut(self.ptr.as_ptr(), self.size) }
    }

    /// Get the length of the allocation
    fn len(&self) -> usize {
        self.size
    }
}

#[cfg(test)]
mod tests {
    use core::mem::MaybeUninit;

    use super::*;

    const HEAP_SIZE_WITH_POOL: usize = 640 * 1024;

    #[test]
    fn test_heap_size() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.free(), HEAP_SIZE_WITH_POOL - 64 * 3584);
    }

    #[test]
    fn test_heap_allocate() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);

        let start_free_size = heap.free();
        let alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 4096]);
        assert_eq!(heap.free(), start_free_size - 4096);

        let start_free_size = heap.free();
        let alloc = heap.allocate_from_pool(40);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 40]);
        assert_eq!(heap.free(), start_free_size);

        let alloc = heap.allocate_from_pool(4096);
        assert!(alloc.is_none());
    }

    #[test]
    fn test_heap_allocate_with_update() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);

        let mut alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 4096]);
        alloc.as_mut().unwrap().as_ref_mut().fill(0xAB);
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0xABu8; 4096]);
    }

    #[test]
    fn test_heap_allocate_max() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);

        let alloc = heap.allocate(416 * 1024);
        assert!(alloc.is_some());
        assert_eq!(alloc.as_ref().unwrap().len(), 416 * 1024);
        assert_eq!(alloc.as_ref().unwrap().as_ref(), [0x0u8; 416 * 1024]);
        assert_eq!(heap.free(), 0);
    }

    #[test]
    fn test_heap_allocate_failue() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);

        let alloc = heap.allocate(416 * 1024 + 1);
        assert!(alloc.is_none());
        assert_eq!(heap.free(), 416 * 1024);
    }

    #[test]
    fn test_heap_deallocate() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);

        let alloc = heap.allocate(4096);
        assert!(alloc.is_some());
        assert_eq!(heap.free(), 416 * 1024 - 4096);

        drop(alloc);
        assert_eq!(heap.free(), 416 * 1024);
    }

    #[test]
    fn test_heap_pool() {
        let mem: [MaybeUninit<u8>; HEAP_SIZE_WITH_POOL] =
            [MaybeUninit::uninit(); HEAP_SIZE_WITH_POOL];

        let heap = HsmDmaHeap::new(mem.as_ptr() as usize, HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.size(), HEAP_SIZE_WITH_POOL);
        assert_eq!(heap.free(), 416 * 1024);

        let mut pool_alloc = Vec::with_capacity(64);
        for _ in 0..64 {
            let alloc = heap.allocate_from_pool(40);
            assert!(alloc.is_some());
            pool_alloc.push(alloc.unwrap());
        }
        let alloc = heap.allocate_from_pool(40);
        assert!(alloc.is_none());

        let popped_alloc = pool_alloc.pop();
        drop(popped_alloc);

        let alloc = heap.allocate_from_pool(40);
        assert!(alloc.is_some());
    }
}
