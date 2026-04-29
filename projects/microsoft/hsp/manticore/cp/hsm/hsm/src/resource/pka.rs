// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::boxed::Box;
use alloc::vec;
use alloc::vec::Vec;
use core::ops::Deref;

use mcr_crypto_pka::*;

use super::HsmFsmResourceId;
use crate::cmd_scheduler::CmdResourceInfo;
use crate::HsmFsmEvent;

/// Self test function
type SelfTestFn = Box<dyn Fn(&dyn PkaTrait)>;

pub(crate) struct PkaResource<T: PkaTrait> {
    /// Resource list
    inner: Vec<T>,

    /// List of free resource IDs
    free_list: Vec<usize>,

    /// Resouce context list to check the resource usage
    ctx_list: Vec<<PkaResource<T> as CmdResourceInfo>::Context>,

    /// Check if self test is pending for this instance
    pending_self_test: Vec<bool>,

    /// Closure if self test is pending
    fn_test: Option<SelfTestFn>,

    /// Resource override range
    #[cfg(feature = "fips_validation_hooks")]
    fixed_instance_id: Option<usize>,
}

impl<T: PkaTrait> PkaResource<T> {
    pub fn new(inner: Vec<T>) -> Self {
        Self {
            free_list: (0..inner.len()).collect(),
            ctx_list: vec![None; inner.len()],
            pending_self_test: vec![false; inner.len()],
            inner,
            fn_test: None,
            #[cfg(feature = "fips_validation_hooks")]
            fixed_instance_id: None,
        }
    }
}

impl<T: PkaTrait> Deref for PkaResource<T> {
    type Target = Vec<T>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<T: PkaTrait> CmdResourceInfo for PkaResource<T> {
    type Id = HsmFsmResourceId;
    type Resource = T;
    type Event = HsmFsmEvent;
    type Context = Option<u16>;

    /// Resource ID
    fn id(&self) -> Self::Id {
        HsmFsmResourceId::Pka
    }

    /// Get the current resource count
    fn count(&self) -> usize {
        #[cfg(feature = "fips_validation_hooks")]
        match self.fixed_instance_id {
            Some(instance) if self.free_list.contains(&instance) => 1,
            Some(_) => 0,
            None => self.free_list.len(),
        }

        #[cfg(not(feature = "fips_validation_hooks"))]
        self.free_list.len()
    }

    /// Acquire the resource
    fn set(&mut self, ctx: Self::Context) -> Option<usize> {
        #[cfg(feature = "fips_validation_hooks")]
        let index = match self.fixed_instance_id {
            Some(instance) if self.free_list.contains(&instance) => {
                self.free_list.retain(|&i| i != instance);
                Some(instance)
            }
            Some(_) => None,
            None => self.free_list.pop(),
        }?;

        #[cfg(not(feature = "fips_validation_hooks"))]
        let index = self.free_list.pop()?;

        self.ctx_list[index] = ctx;

        Some(index)
    }

    /// Release the resource
    fn clear(&mut self, instance_id: usize) {
        if self.pending_self_test[instance_id] {
            let self_test = self.fn_test.take();
            if let Some(fn_test) = self_test {
                fn_test(&self.inner[instance_id]);
            }
            self.fn_test = None;
            self.pending_self_test[instance_id] = false;
        }

        self.free_list.push(instance_id);
    }

    /// Get the resource
    fn resource(&self, instance_id: usize) -> &Self::Resource {
        &self.inner[instance_id]
    }

    /// Find the context
    fn find_ctx<F>(&self, predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        for ctx_entry in self.ctx_list.iter() {
            if predicate(ctx_entry) {
                return Some(*ctx_entry);
            }
        }

        None
    }

    /// Self test if the resource is operating as expected
    fn self_test<FnTest>(&mut self, instance_id: usize, test: FnTest)
    where
        FnTest: Fn(&dyn PkaTrait) + 'static,
    {
        if self.free_list.contains(&instance_id) {
            test(&self.inner[instance_id]);
        } else {
            self.pending_self_test[instance_id] = true;
            self.fn_test = Some(Box::new(test));
        }
    }

    /// Cleanup the resource
    fn cleanup_event(&self, instance_id: usize) -> Self::Event {
        HsmFsmEvent::ResourceCleanup(self.id(), instance_id)
    }

    /// Cleanup the context of the resource
    fn cleanup_ctx(&mut self, instance_id: usize) {
        self.ctx_list[instance_id] = None;
    }

    /// Override the resource range
    #[cfg(feature = "fips_validation_hooks")]
    fn operate_on_fixed_resource(&mut self, instance: Option<usize>) {
        self.fixed_instance_id = instance.filter(|&i| i < self.inner.len());
    }
}

#[cfg(test)]
mod tests {
    use mcr_types::IoMemRange;

    use super::*;

    #[derive(Clone, PartialEq)]
    struct TestResource {}

    impl PkaTrait for TestResource {
        fn peek_tag(&self) -> Option<u16> {
            None
        }

        fn begin_ecc_gen_key(&self, _tag: u16, _curve: PkaEccCurve) -> McrResult<PkaEccCmd> {
            Err(u32::MAX)?
        }

        fn end_ecc_gen_key(&self, _tag: u16, _op: PkaEccCmd) -> McrResult<PkaEccKeyPair> {
            Err(u32::MAX)?
        }

        fn begin_ecc_sign_zc(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
            _priv_key: &[u8],
            _digest: &IoMemRange,
            _signature: &IoMemRange,
        ) -> McrResult<PkaEccCmd> {
            Err(u32::MAX)?
        }

        fn end_ecc_sign_zc(&self, _tag: u16) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_ecc_verify_zc(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
            _pub_key: &IoMemRange,
            _digest: &IoMemRange,
            _signature: &IoMemRange,
        ) -> McrResult<()> {
            Err(u32::MAX)
        }

        fn end_ecc_verify_zc(&self, _tag: u16) -> McrResult<bool> {
            Err(u32::MAX)
        }

        fn begin_ecc_point_validation_zc(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
            _public_key: &IoMemRange,
        ) -> McrResult<()> {
            Err(u32::MAX)
        }

        fn end_ecc_point_validation_zc(&self, _tag: u16) -> McrResult<bool> {
            Err(u32::MAX)
        }

        fn begin_ecc_gen_pub_key_zc(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
            _private_key: &[u8],
            _pub_key: &IoMemRange,
        ) -> McrResult<PkaEccCmd> {
            Err(u32::MAX)?
        }

        fn end_ecc_gen_pub_key_zc(&self, _tag: u16, _op: PkaEccCmd) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_montgomery_constant_calculation(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
        ) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn end_montgomery_constant_calculation(&self, _tag: u16) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_ecdh_compute_zc(
            &self,
            _tag: u16,
            _curve: PkaEccCurve,
            _private_key: &[u8],
            _public_key: &IoMemRange,
        ) -> McrResult<PkaEccCmd> {
            Err(u32::MAX)?
        }

        fn end_ecdh_compute(&self, _tag: u16, _op: PkaEccCmd) -> McrResult<PkaEccSecretValue> {
            Err(u32::MAX)?
        }

        fn begin_rsa_private_key_op_zc(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _priv_key: &[u8],
            _data: &IoMemRange,
            _result: &IoMemRange,
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_private_key_op_zc(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_rsa_public_key_op_zc(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _public_key: &IoMemRange,
            _data: &IoMemRange,
            _result: &IoMemRange,
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_public_key_op_zc(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_rsa_private_key_op_crt_zc(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _crt_param1: &[u8],
            _crt_param2: &[u8],
            _data: &IoMemRange,
            _result: &IoMemRange,
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_private_key_op_crt_zc(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_rsa_montgomery_constant_calculation(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _modulus_be: &[u8],
        ) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn end_rsa_montgomery_constant_calculation(&self, _tag: u16) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_rsa_montgomery_in(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _data_be: &[u8],
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_montgomery_in(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
            Err(u32::MAX)?
        }

        fn begin_rsa_modular_inverse(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _data_be: &[u8],
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_modular_inverse(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<PkaRsaMontData> {
            Err(u32::MAX)?
        }

        fn begin_rsa_montgomery_out(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _data_be: &[u8],
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_montgomery_out(&self, _tag: u16, _op: PkaRsaCmd) -> McrResult<PkaRsaData> {
            Err(u32::MAX)?
        }

        fn begin_rsa_modular_multiplication(
            &self,
            _tag: u16,
            _rsa_type: PkaRsaSize,
            _value1: &[u8],
            _value2: &[u8],
        ) -> McrResult<PkaRsaCmd> {
            Err(u32::MAX)?
        }

        fn end_rsa_modular_multiplication(
            &self,
            _tag: u16,
            _op: PkaRsaCmd,
        ) -> McrResult<PkaRsaMontData> {
            Err(u32::MAX)?
        }

        fn ecdsa_self_test(&self) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn ecdh_self_test(&self) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn rsa_mod_exp_self_test(&self) -> McrResult<Vec<u8>> {
            Err(u32::MAX)?
        }

        fn rsa_mod_exp_crt_self_test(&self) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn begin_memory_wipe(&self, _tag: u16) -> McrResult<()> {
            Err(u32::MAX)?
        }

        fn end_memory_wipe(&self, _tag: u16) -> McrResult<()> {
            Err(u32::MAX)?
        }
    }

    #[test]
    fn test_pka_new() {
        let mut test_resource = super::PkaResource::new(vec![TestResource {}; 16]);
        assert!(test_resource.id() == HsmFsmResourceId::Pka);
        assert_eq!(test_resource.count(), 16);
        assert_eq!(test_resource.set(Some(100)), Some(15));
        assert_eq!(test_resource.set(Some(101)), Some(14));
        assert_eq!(test_resource.set(Some(102)), Some(13));
        assert_eq!(test_resource.set(Some(103)), Some(12));
        assert_eq!(test_resource.set(Some(104)), Some(11));
        assert_eq!(test_resource.set(Some(105)), Some(10));
        assert_eq!(test_resource.set(Some(106)), Some(9));
        assert_eq!(test_resource.set(Some(107)), Some(8));
        assert_eq!(test_resource.set(Some(108)), Some(7));
        assert_eq!(test_resource.set(Some(109)), Some(6));
        assert_eq!(test_resource.set(Some(110)), Some(5));
        assert_eq!(test_resource.set(Some(111)), Some(4));
        assert_eq!(test_resource.set(Some(112)), Some(3));
        assert_eq!(test_resource.set(Some(113)), Some(2));
        assert_eq!(test_resource.set(Some(114)), Some(1));
        assert_eq!(test_resource.set(Some(115)), Some(0));
        assert_eq!(test_resource.set(Some(116)), None);
        assert_eq!(
            test_resource.find_ctx(|id| *id == Some(100)),
            Some(Some(100))
        );

        test_resource.self_test(15, move |resource| {
            let _ = resource.rsa_mod_exp_self_test();
        });

        test_resource.cleanup_ctx(15);
        test_resource.clear(15);

        test_resource.self_test(15, move |resource| {
            let _ = resource.rsa_mod_exp_self_test();
        });

        assert_eq!(test_resource.find_ctx(|id| *id == Some(100)), None);
        assert_eq!(test_resource.set(Some(117)), Some(15));
        test_resource.clear(14);
        assert_eq!(test_resource.set(Some(118)), Some(14));
        test_resource.clear(4);
        assert_eq!(test_resource.set(Some(119)), Some(4));

        assert_eq!(test_resource.resource(0).peek_tag(), None);

        let test_vec = test_resource.deref();
        let test_res = TestResource {};
        for item in test_vec.iter().take(15 + 1) {
            assert!(*item == test_res);
        }
    }
}
