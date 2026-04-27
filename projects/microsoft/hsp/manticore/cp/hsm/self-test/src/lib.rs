// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;
use bitfield_struct::bitfield;

use mcr_types::CoreId;

#[macro_use]
mod self_test;

// Exhaustive list of tests expressed via an enum.
define_self_tests! {
    SelfTest {
        AesXtsNegEnc,
        AesXtsNegDec,
        AesGcmAlignedAndUnalignedData,
        AesGcmAlignedData,
        AesGcmAadNoAlignedData,
        AesCbc,
        AesEcb,
        AesUnwrapWithPadding,
        Rng,
        EcdhEngineInstance0,
        EcdsaEngineInstance0,
        Rsa2KModExpEngineInstance0,
        Rsa2KModExpCrtEngineInstance0,
        EcdhEngineInstance1,
        EcdsaEngineInstance1,
        Rsa2KModExpEngineInstance1,
        Rsa2KModExpCrtEngineInstance1,
        EcdhEngineInstance2,
        EcdsaEngineInstance2,
        Rsa2KModExpEngineInstance2,
        Rsa2KModExpCrtEngineInstance2,
        EcdhEngineInstance3,
        EcdsaEngineInstance3,
        Rsa2KModExpEngineInstance3,
        Rsa2KModExpCrtEngineInstance3,
        EcdhEngineInstance4,
        EcdsaEngineInstance4,
        Rsa2KModExpEngineInstance4,
        Rsa2KModExpCrtEngineInstance4,
        EcdhEngineInstance5,
        EcdsaEngineInstance5,
        Rsa2KModExpEngineInstance5,
        Rsa2KModExpCrtEngineInstance5,
        EcdhEngineInstance6,
        EcdsaEngineInstance6,
        Rsa2KModExpEngineInstance6,
        Rsa2KModExpCrtEngineInstance6,
        EcdhEngineInstance7,
        EcdsaEngineInstance7,
        Rsa2KModExpEngineInstance7,
        Rsa2KModExpCrtEngineInstance7,
        EcdhEngineInstance8,
        EcdsaEngineInstance8,
        Rsa2KModExpEngineInstance8,
        Rsa2KModExpCrtEngineInstance8,
        EcdhEngineInstance9,
        EcdsaEngineInstance9,
        Rsa2KModExpEngineInstance9,
        Rsa2KModExpCrtEngineInstance9,
        EcdhEngineInstance10,
        EcdsaEngineInstance10,
        Rsa2KModExpEngineInstance10,
        Rsa2KModExpCrtEngineInstance10,
        EcdhEngineInstance11,
        EcdsaEngineInstance11,
        Rsa2KModExpEngineInstance11,
        Rsa2KModExpCrtEngineInstance11,
        EcdhEngineInstance12,
        EcdsaEngineInstance12,
        Rsa2KModExpEngineInstance12,
        Rsa2KModExpCrtEngineInstance12,
        EcdhEngineInstance13,
        EcdsaEngineInstance13,
        Rsa2KModExpEngineInstance13,
        Rsa2KModExpCrtEngineInstance13,
        EcdhEngineInstance14,
        EcdsaEngineInstance14,
        Rsa2KModExpEngineInstance14,
        Rsa2KModExpCrtEngineInstance14,
        EcdhEngineInstance15,
        EcdsaEngineInstance15,
        Rsa2KModExpEngineInstance15,
        Rsa2KModExpCrtEngineInstance15,
        Hkdf,
        Kbkdf,
        SelfTestCompleted,
    }
}

#[derive(Clone)]
pub struct SelfTestTracker {
    /// list of all self tests
    tests: Vec<SelfTest>,

    /// current test index
    current_index: usize,
}

/// Implement iterator for SelfTest
impl SelfTestTracker {
    /// Create a new self test tracker
    pub fn new() -> SelfTestTracker {
        SelfTestTracker {
            tests: SelfTest::all().to_vec(),
            current_index: SelfTest::all().len() - 1,
        }
    }

    /// Set the next self test
    pub fn set_next_test(&mut self, next_test: SelfTest) {
        let len = SelfTest::all().len();
        self.current_index = (next_test as usize + len - 1) % len;
    }

    /// Get the current self test
    pub fn get_current_test(&self) -> SelfTest {
        self.tests[self.current_index]
    }

    /// Get the next self test
    pub fn next_test(&mut self) -> SelfTest {
        self.current_index = (self.current_index + 1) % self.tests.len();
        self.tests[self.current_index]
    }
}

/// Implement Default for SelfTestTracker
impl Default for SelfTestTracker {
    fn default() -> Self {
        Self::new()
    }
}

/// Self Test Request from Admin to HSM
#[repr(C)]
#[derive(Clone, Copy)]
pub struct SelfTestReqPacket {
    /// Test ID
    pub test_id: SelfTest,
}

/// Self Test Response from HSM to Admin
#[repr(C)]
#[derive(Clone, Copy)]
pub struct SelfTestRespPacket {
    /// Test Result
    pub result: Result<(), u32>,
}

/// Negative test kind used for fips health test
#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum NegKind {
    /// No negative test (default)
    None = 0,
    /// Trigger RNG repetition count (RCT) failure
    RngRct = 1,
    /// Trigger RNG adaptive proportion (APT) failure
    RngApt = 2,
}

impl From<u8> for NegKind {
    fn from(v: u8) -> Self {
        match v {
            1 => NegKind::RngRct,
            2 => NegKind::RngApt,
            _ => NegKind::None,
        }
    }
}

#[bitfield(u32)]
pub struct PreopsNegativeTest {
    /// Self test identifier (bits 15-0)
    pub test_id: u16,

    /// Negative test kind (bits 18-16)
    #[bits(3)]
    pub neg_kind: u8,

    /// Reserved (bits 23-19)
    #[bits(5)]
    pub _reserved: u8,

    /// Core identifier (bits 26-24)
    #[bits(3)]
    pub core_id: u8,

    /// Core identifier (bits 31-27)
    #[bits(5)]
    pub _reserved1: u8,
}

impl PreopsNegativeTest {
    #[inline]
    fn core_matches(&self) -> bool {
        // Get current CPU ID and convert to CoreId
        let current: CoreId = mcr_cpu::cpu_id().into();
        current == self.core_id().into()
    }

    /// Validates CPU ID and returns appropriate SelfTest from GSRAM value
    pub fn get_preops_negative_self_test(&self) -> Option<SelfTest> {
        if self.core_matches() {
            SelfTest::try_from(self.test_id()).ok()
        } else {
            None
        }
    }

    /// Validates CPU ID and returns appropriate NegKind from GSRAM value
    pub fn get_preops_negative_kind(&self) -> NegKind {
        if self.core_matches() {
            NegKind::from(self.neg_kind())
        } else {
            NegKind::None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_self_test_tracker() {
        let mut tracker = SelfTestTracker::new();
        let mut tests = Vec::new();
        for _ in 0..SelfTest::all().len() {
            tests.push(tracker.next_test());
        }

        assert_eq!(tests.len(), SelfTest::all().len());
        assert!(tests == SelfTest::all().to_vec());
    }

    #[test]
    fn test_self_test_current_test() {
        let mut tracker = SelfTestTracker::new();
        let _ = tracker.next_test();
        let mut tests = Vec::new();
        for _ in 0..SelfTest::all().len() {
            tests.push(tracker.get_current_test());
            tracker.next_test();
        }

        assert_eq!(tests.len(), SelfTest::all().len());
        assert!(tests == SelfTest::all().to_vec());
    }

    #[test]
    fn test_self_test_wrap_around() {
        let mut tracker = SelfTestTracker::new();
        let mut tests = Vec::new();
        for _ in 0..SelfTest::all().len() {
            tests.push(tracker.next_test());
        }

        assert_eq!(tests.len(), SelfTest::all().len());
        assert!(tests == SelfTest::all().to_vec());

        for _ in 0..SelfTest::all().len() {
            tracker.next_test();
        }

        assert!(tracker.next_test() == SelfTest::all().to_vec()[0]);
        tracker.next_test();
        assert!(tracker.next_test() == SelfTest::all().to_vec()[2]);
    }

    #[test]
    fn test_from() {
        let test = SelfTest::from(SelfTest::EcdhEngineInstance0 as usize);
        assert!(test == SelfTest::EcdhEngineInstance0);
    }

    #[test]
    fn test_try_from() {
        let test = SelfTest::try_from(SelfTest::EcdhEngineInstance0 as u32);
        assert!(test == Ok(SelfTest::EcdhEngineInstance0));
    }

    #[test]
    fn test_try_into() {
        let test: u32 = SelfTest::EcdhEngineInstance15.try_into().unwrap();
        assert!(test == SelfTest::EcdhEngineInstance15 as u32);
    }

    #[test]
    fn test_get_instance() {
        let test = SelfTest::EcdhEngineInstance0;
        assert!(test.get_engine_instance() == Some(0));

        let test = SelfTest::EcdsaEngineInstance14;
        assert!(test.get_engine_instance() == Some(14));

        let test = SelfTest::Kbkdf;
        assert!(test.get_engine_instance().is_none());
    }

    #[test]
    fn out_of_range() {
        let mut test_index = SelfTest::SelfTestCompleted as usize;
        assert!(SelfTest::from(test_index) == SelfTest::SelfTestCompleted);
        test_index += 1;
        assert!(SelfTest::from(test_index) == SelfTest::SelfTestCompleted);
    }

    #[test]
    fn test_is_matching_ecdsa() {
        let test = SelfTest::EcdsaEngineInstance1;
        let base = SelfTest::EcdsaEngineInstance0;

        let result = test.is_matching_test(base, 1);
        assert!(result);

        let result = test.is_matching_test(base, 10);
        assert!(!result);
    }

    #[test]
    fn test_is_matching_test_ecdh() {
        let test = SelfTest::EcdhEngineInstance1;
        let base = SelfTest::EcdhEngineInstance0;

        let result = test.is_matching_test(base, 1);
        assert!(result);

        let result = test.is_matching_test(base, 10);
        assert!(!result);
    }

    #[test]
    fn test_is_matching_test_rsa_mod_exp() {
        let test = SelfTest::Rsa2KModExpEngineInstance1;
        let base = SelfTest::Rsa2KModExpEngineInstance0;

        let result = test.is_matching_test(base, 1);
        assert!(result);

        let result = test.is_matching_test(base, 10);
        assert!(!result);
    }

    #[test]
    fn test_is_matching_test_rsa_mod_exp_crt() {
        let test = SelfTest::Rsa2KModExpCrtEngineInstance1;
        let base = SelfTest::Rsa2KModExpCrtEngineInstance0;

        let result = test.is_matching_test(base, 1);
        assert!(result);

        let result = test.is_matching_test(base, 10);
        assert!(!result);
    }
}
