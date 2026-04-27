// Copyright (c) Microsoft Corporation. All rights reserved.

use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

pub const MAX_PCIE_FUNCTIONS: usize = 65;

// Pcie Function Ids
seq_macro::seq! {
    N in 0..64 {
        /// Pcie Function Ids
        #[repr(u8)]
        #[open_enum]
        #[derive(IntoBytes, Immutable, Clone, Copy, FromBytes)]
        pub enum PcieFunction {
            #(
                Vf~N = N,
            )*
            Pf = 64,
        }
    }
}

impl PcieFunction {
    /// Returns an iterator over all PcieFunction values
    ///
    /// # Returns
    ///
    /// An iterator over all PcieFunction values
    pub fn iter() -> PcieFnIter {
        PcieFnIter::default()
    }

    /// Returns number of valid Pcie functions
    pub fn len() -> usize {
        let mut len = 0usize;
        for _ in Self::iter() {
            len += 1;
        }

        len
    }
}

/// Pcie Function Ids iterator
#[derive(Default)]
pub struct PcieFnIter {
    /// Temporary value used to track the current iteration
    val: u8,
}

impl Iterator for PcieFnIter {
    /// The type of the elements being iterated over.
    type Item = PcieFunction;

    /// Advances the iterator and returns the next value.
    ///
    /// #Returns
    ///
    /// The next value in the iterator
    fn next(&mut self) -> Option<Self::Item> {
        if let Ok(pfn) = self.val.try_into() {
            self.val += 1;
            Some(pfn)
        } else {
            None
        }
    }
}

impl From<PcieFunction> for u8 {
    fn from(value: PcieFunction) -> Self {
        value.0 as Self
    }
}

impl From<PcieFunction> for usize {
    fn from(value: PcieFunction) -> Self {
        value.0 as Self
    }
}

impl From<PcieFunction> for u32 {
    fn from(value: PcieFunction) -> Self {
        value.0 as Self
    }
}

// Constants for pcie function Ids
pub const PCIE_PF_FUNCTION_ID: u8 = 64;
const PCIE_VF_FUNCTION_ID_START: u8 = 0;

impl TryFrom<u8> for PcieFunction {
    type Error = u32;
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            PCIE_VF_FUNCTION_ID_START..=PCIE_PF_FUNCTION_ID => {
                Ok(unsafe { core::mem::transmute::<u8, PcieFunction>(value) })
            }
            _ => Err(u32::MAX),
        }
    }
}

// Constants for Memory Location Ids
const PCIE_PF_MEMORY_LOCATION_ID: u8 = 0x10;
const PCIE_VF_MEMORY_LOCATION_ID_START: u8 = 0x20;

impl TryFrom<MemoryLocation> for PcieFunction {
    type Error = u32;

    fn try_from(value: MemoryLocation) -> Result<Self, Self::Error> {
        match value.into() {
            PCIE_PF_MEMORY_LOCATION_ID => Ok(PcieFunction::Pf),
            MemoryLocation::MIN_VF_LOC..=MemoryLocation::MAX_VF_LOC => {
                PcieFunction::try_from(value as u8 - MemoryLocation::MIN_VF_LOC)
            }
            _ => Err(u32::MAX)?,
        }
    }
}

// Memory Location Ids for Pcie Function Ids
seq_macro::seq! {
    N in 0..64 {
        /// Memory Location Ids for Pcie Function Ids
        #[repr(u8)]
        #[derive(Clone, Copy, Eq, PartialEq)]
        pub enum MemoryLocation {
            Soc = 0,
            Pf = PCIE_PF_MEMORY_LOCATION_ID,
            #(
                #[allow(clippy::identity_op)]
                Vf~N = PCIE_VF_MEMORY_LOCATION_ID_START + N,
            )*
        }
    }
}

impl MemoryLocation {
    const MIN_VF_LOC: u8 = Self::loc(PcieFunction::Vf0);
    const MAX_VF_LOC: u8 = Self::loc(PcieFunction::Vf63);
    const fn loc(pfn: PcieFunction) -> u8 {
        match pfn {
            PcieFunction::Pf => PCIE_PF_MEMORY_LOCATION_ID,
            _ => PCIE_VF_MEMORY_LOCATION_ID_START + pfn.0,
        }
    }
}

impl TryFrom<u8> for MemoryLocation {
    type Error = u32;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        let val = match value {
            0u8 => MemoryLocation::Soc,
            PCIE_PF_MEMORY_LOCATION_ID => MemoryLocation::Pf,
            Self::MIN_VF_LOC..=Self::MAX_VF_LOC => unsafe {
                core::mem::transmute::<u8, MemoryLocation>(value)
            },
            _ => Err(u32::MAX)?,
        };

        Ok(val)
    }
}

impl From<MemoryLocation> for u32 {
    fn from(value: MemoryLocation) -> Self {
        value as Self
    }
}

impl From<MemoryLocation> for u8 {
    fn from(value: MemoryLocation) -> Self {
        value as Self
    }
}

impl From<PcieFunction> for MemoryLocation {
    fn from(value: PcieFunction) -> Self {
        Self::loc(value).try_into().unwrap()
    }
}

#[cfg(test)]
mod tests {
    // Import the symbols from the main code module
    use super::*;

    #[test]
    fn test_iterator() {
        for _pcie_fn in PcieFunction::iter() {}
    }

    #[test]
    fn test_pcie_function_from_pcie_function_for_u8() {
        assert_eq!(u8::from(PcieFunction::Pf), 64u8);
    }

    #[test]
    fn test_pcie_function_from_pcie_function_for_usize() {
        assert_eq!(usize::from(PcieFunction::Pf), 64usize);
    }

    #[test]
    fn test_pcie_function_try_from_u8() {
        for i in 0..65u8 {
            let pfn = PcieFunction::try_from(i);
            assert!(pfn.is_ok());
            let pfn = pfn.unwrap();
            if i == 64 {
                assert_eq!(pfn.0, PcieFunction::Pf.0);
            } else {
                assert_eq!(pfn.0, PcieFunction::try_from(i).unwrap().0);
            }
        }
        assert!(matches!(PcieFunction::try_from(65u8), Err(u32::MAX)));
        assert!(matches!(PcieFunction::try_from(100u8), Err(u32::MAX)));
    }

    #[test]
    fn test_memory_location_try_from_u8() {
        assert!(matches!(
            MemoryLocation::try_from(0u8).unwrap(),
            MemoryLocation::Soc
        ));
        assert!(matches!(
            MemoryLocation::try_from(PCIE_PF_MEMORY_LOCATION_ID).unwrap(),
            MemoryLocation::Pf
        ));

        assert!(matches!(MemoryLocation::try_from(100u8), Err(u32::MAX)));
    }

    #[test]
    fn test_memory_location_from_mem_location_for_u32() {
        assert_eq!(u32::from(MemoryLocation::Soc), 0u32);
    }

    #[test]
    fn test_memory_location_from_mem_location_for_u8() {
        assert_eq!(u8::from(MemoryLocation::Soc), 0u8);
    }

    #[test]
    fn test_memory_location_from_pcie_function_for_mem_location() {
        assert!(matches!(
            MemoryLocation::from(PcieFunction::Pf),
            MemoryLocation::Pf
        ));
        assert!(matches!(
            MemoryLocation::from(PcieFunction::Vf0),
            MemoryLocation::Vf0
        ));
    }

    #[test]
    fn test_memory_location_tryfrom_mem_location_for_pcie_function() {
        let memvalue: MemoryLocation = unsafe { core::mem::transmute(200u8) };

        assert!(matches!(
            PcieFunction::try_from(MemoryLocation::Pf),
            Ok(PcieFunction::Pf)
        ));
        assert!(matches!(
            PcieFunction::try_from(MemoryLocation::Vf0),
            Ok(PcieFunction::Vf0)
        ));
        assert!(matches!(PcieFunction::try_from(memvalue), Err(u32::MAX)));
    }
}
