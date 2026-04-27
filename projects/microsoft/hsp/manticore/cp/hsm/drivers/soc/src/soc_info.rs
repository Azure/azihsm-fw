// Copyright (c) Microsoft Corporation. All rights reserved.

#[cfg(feature = "fips_validation_hooks")]
use mcr_cpu::CpuId;
use mcr_mem_map::GsRamMemMap;
#[cfg(feature = "fips_validation_hooks")]
use mcr_mem_map::HsmDtcmMemMap;
use mcr_mem_map::PsRamMemMap;
#[cfg(feature = "fips_validation_hooks")]
use mcr_mem_map::SocMemMap;
use mcr_registers::por;

use crate::*;

/// SoC information
#[derive(Clone, Default)]
pub struct SocInfo {}

impl SocInfoTrait for SocInfo {
    /// Get SoC firmware package version
    fn fw_version(&self) -> [u8; 32] {
        let mut fw_version: [u8; 32] = [0; 32];
        fw_version.copy_from_slice(GsRamMemMap::fw_package_version());

        fw_version
    }

    /// Get SoC ID
    fn id(&self) -> [u8; 32] {
        let mut soc_id: [u8; 32] = [0; 32];
        soc_id.copy_from_slice(GsRamMemMap::soc_id());

        soc_id
    }

    /// Get Firmware SVN
    fn svn(&self) -> [u8; 8] {
        let mut svn: [u8; 8] = [0; 8];
        // BKS table entry 0 corresponds to current firmware SVN
        svn.copy_from_slice(&GsRamMemMap::bks_table()[0].svn[..]);

        svn
    }

    /// Get reset type
    fn reset_type(&self) -> SocResetType {
        PsRamMemMap::reset_type().get().into()
    }

    /// Reset GDMA engine
    fn reset_gdma(&self) {
        let reg = por::RegisterBlock::block();

        reg.reset_control().read_and_modify(|_, w| w.rst_gdma(true));
        reg.reset_control()
            .read_and_modify(|_, w| w.rst_gdma(false));
    }

    /// Reset NVMe interface
    fn reset_nvme(&self) {
        let reg = por::RegisterBlock::block();

        reg.reset_control().read_and_modify(|_, w| w.rst_nqm(true));
        reg.reset_control().read_and_modify(|_, w| w.rst_nqm(false));
    }

    /// Set negative cast indicator
    ///
    /// This function can only be called from Admin core
    #[cfg(feature = "fips_validation_hooks")]
    fn set_negative_cast_hooks(&self, id: SelfTest) {
        if let Some(negative_self_test_id) = SocMemMap::negative_self_test_id().first_mut() {
            *negative_self_test_id = Some(id)
        }
    }
}

impl SocInfo {
    /// Get the indicator to induce cryptographic algorithm self test failure
    ///
    /// This function is used to induce a failure in the self test for FIPS validation.
    /// It is used in the self test process to check if the self test is working correctly.
    ///
    /// # Arguments
    ///
    /// `id` - The self test ID to check if a failure is required to be induced.
    /// `inst` - The instance of the self test to check if a failure is required to be induced.
    ///
    /// # Returns
    ///
    /// `true` if the self test should fail, `false` otherwise.
    #[cfg(feature = "fips_validation_hooks")]
    pub fn induce_cast_failure(&self, matching_id: SelfTest, inst: Option<usize>) -> bool {
        let negative_self_test_id_location = match mcr_cpu::cpu_id() {
            CpuId::Admin => SocMemMap::negative_self_test_id().first_mut(),
            CpuId::Hsm => HsmDtcmMemMap::negative_self_test_id().first_mut(),
            _ => return false,
        };

        let negative_self_test_id = match negative_self_test_id_location {
            Some(negative_self_test_id) => negative_self_test_id,
            None => return false,
        };

        let result = match negative_self_test_id {
            Some(negative_self_test_id) => {
                if let Some(inst) = inst {
                    negative_self_test_id.is_matching_test(matching_id, inst)
                } else if *negative_self_test_id == matching_id {
                    true
                } else {
                    false
                }
            }
            None => false,
        };

        // Reset the test hook to fail the negagtive self test if the given test is matches
        if result {
            negative_self_test_id.take();
        }

        result
    }

    /// Get the indicator to induce health test failure
    ///
    /// This function is used to induce a failure in the health test for FIPS validation.
    /// It is used in the health test process to check if the health test is working correctly.
    //// # Arguments
    ///
    /// `matching_kind` - The kind of health test to check if a failure is required to be induced.
    /// # Returns
    ///
    /// `true` if the health test should fail, `false` otherwise.
    #[cfg(feature = "fips_validation_hooks")]
    pub fn induce_health_failure(&self, matching_kind: NegKind) -> bool {
        use mcr_mem_map::HsmDtcmMemMap;

        let slot = match mcr_cpu::cpu_id() {
            CpuId::Admin => SocMemMap::negative_kind().first_mut(),
            CpuId::Hsm => HsmDtcmMemMap::negative_kind().first_mut(),
            _ => return false,
        };

        let Some(kind) = slot else {
            return false;
        };

        let result = *kind == matching_kind;

        if result {
            // One-shot: clear after match
            *kind = NegKind::None;
        }

        result
    }
}
