// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod cdma;
mod dtcm_admin;
mod dtcm_hsm;
mod gsram;
mod psram;
mod soc;

pub use cdma::CdmaMemMap;
pub use dtcm_admin::AdminDtcmMemMap;
pub use dtcm_hsm::HsmDtcmMemMap;
pub use gsram::GsRamMemMap;
use mcr_types::VolatileCell;
pub use psram::PsRamMemMap;
pub use soc::SocMemMap;

/// Convert raw memory addresses to a slice of type T
///
/// # Arguments
///
/// * `addr` - The address of the memory to convert
/// * `len` - The length of the slice
///
/// # Returns
///
/// A slice of type T
///
/// # Safety
///
/// This function is unsafe because it dereferences a raw pointer.
pub fn mem_addr_to_slice<T>(addr: usize, len: usize) -> &'static mut [T] {
    unsafe { core::slice::from_raw_parts_mut(addr as *mut T, len) }
}

/// Convert an unsigned integer to reference to a volatile cell
///
/// # Arguments
///
/// * `addr` - The address of the memory to convert
///
/// # Returns
///
/// Reference to a volatile cell
///
pub fn mem_addr_to_volatile_ptr(addr: u32) -> &'static VolatileCell<u32> {
    unsafe {
        #[allow(clippy::transmute_ptr_to_ref)]
        core::mem::transmute(addr as *const u32)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mem_addr_to_slice() {
        let addr = 0x1000;
        let len = 10;
        let slice: &mut [u32] = mem_addr_to_slice(addr, len);
        assert_eq!(slice.len(), len);
        assert_eq!(slice.as_ptr() as usize, addr);
    }

    #[test]
    fn test_mem_add_to_volatile_ptr() {
        let addr = 0x1000;
        let ptr: &VolatileCell<u32> = mem_addr_to_volatile_ptr(addr);
        assert_eq!(ptr.as_ptr() as u32, addr);
    }
}
