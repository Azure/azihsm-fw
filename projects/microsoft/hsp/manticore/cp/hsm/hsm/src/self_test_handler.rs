// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::error;
use crate::error::HsmErr;
use crate::heap::HsmDmaAllocTrait;
use crate::heap::HsmDmaHeapTrait;
use crate::HsmEnvTrait;
use crate::HsmEventHandler;
use crate::HsmHalTrait;

use mcr_crypto_aes::AesTrait;
use mcr_crypto_aes::AES_SELF_TEST_INPUT_BUF_MAX_SIZE_BYTES;
use mcr_crypto_aes::AES_SELF_TEST_IV_BUF_MAX_SIZE_BYTES;
use mcr_crypto_aes::AES_SELF_TEST_OUTPUT_BUF_MAX_SIZE_BYTES;
use mcr_crypto_pka::PkaTrait;
use mcr_crypto_rng::RngTrait;
use mcr_crypto_sha::HashAlgorithm;
use mcr_crypto_sha::ShaTrait;
use mcr_logging::*;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_simplex::SimplexPipeTrait;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;

impl<E: HsmEnvTrait> HsmEventHandler<E> {
    pub(crate) fn execute_self_test(&mut self, self_test_packet: &SelfTestReqPacket) {
        let self_test = self_test_packet.test_id;

        match self_test {
            SelfTest::Hkdf => {
                let result = self.env().borrow().hal().sha().hkdf_self_test_256();

                if result.is_err() {
                    // Handle the error case
                    self.env()
                        .borrow()
                        .hal()
                        .notify_self_test_failure(self_test);
                }

                self.send_self_test_response(result);
            }
            SelfTest::Kbkdf => {
                let result = self.env().borrow().hal().sha().kbkdf_self_test_512();

                if result.is_err() {
                    // Handle the error case
                    self.env()
                        .borrow()
                        .hal()
                        .notify_self_test_failure(self_test);
                }

                self.send_self_test_response(result);
            }
            SelfTest::Rsa2KModExpEngineInstance0
            | SelfTest::Rsa2KModExpEngineInstance1
            | SelfTest::Rsa2KModExpEngineInstance2
            | SelfTest::Rsa2KModExpEngineInstance3
            | SelfTest::Rsa2KModExpEngineInstance4
            | SelfTest::Rsa2KModExpEngineInstance5
            | SelfTest::Rsa2KModExpEngineInstance6
            | SelfTest::Rsa2KModExpEngineInstance7
            | SelfTest::Rsa2KModExpEngineInstance8
            | SelfTest::Rsa2KModExpEngineInstance9
            | SelfTest::Rsa2KModExpEngineInstance10
            | SelfTest::Rsa2KModExpEngineInstance11
            | SelfTest::Rsa2KModExpEngineInstance12
            | SelfTest::Rsa2KModExpEngineInstance13
            | SelfTest::Rsa2KModExpEngineInstance14
            | SelfTest::Rsa2KModExpEngineInstance15 => {
                if let Some(instance) = self_test.get_engine_instance() {
                    let self_test_env = self.env().borrow().clone();

                    self.env().borrow().pka_engine().self_test(
                        instance,
                        move |engine: &dyn PkaTrait| {
                            // Get the padded key from RSA operation
                            let response_result = match engine.rsa_mod_exp_self_test() {
                                Ok(padded_key) => {
                                    // If RSA operation succeeded, try OAEP decode
                                    self_test_env.hal().sha().decode_oaep_kek_self_test(
                                        padded_key.as_ref(),
                                        HashAlgorithm::Sha256,
                                    )
                                }
                                Err(e) => Err(e), // Forward the RSA error
                            };

                            if response_result.is_err() {
                                // Handle the error case
                                self_test_env.hal().notify_self_test_failure(self_test);
                            }

                            if self_test_env
                                .hal()
                                .self_test_resp()
                                .send(SelfTestRespPacket {
                                    result: response_result,
                                })
                                .is_err()
                            {
                                error!("[rsa2k_mod] Failed to send self test response");
                            }
                        },
                    );
                } else {
                    self.send_self_test_response(Err(HsmErr::MissingSelfTestEngineInstance.into()));
                }
            }
            SelfTest::Rsa2KModExpCrtEngineInstance0
            | SelfTest::Rsa2KModExpCrtEngineInstance1
            | SelfTest::Rsa2KModExpCrtEngineInstance2
            | SelfTest::Rsa2KModExpCrtEngineInstance3
            | SelfTest::Rsa2KModExpCrtEngineInstance4
            | SelfTest::Rsa2KModExpCrtEngineInstance5
            | SelfTest::Rsa2KModExpCrtEngineInstance6
            | SelfTest::Rsa2KModExpCrtEngineInstance7
            | SelfTest::Rsa2KModExpCrtEngineInstance8
            | SelfTest::Rsa2KModExpCrtEngineInstance9
            | SelfTest::Rsa2KModExpCrtEngineInstance10
            | SelfTest::Rsa2KModExpCrtEngineInstance11
            | SelfTest::Rsa2KModExpCrtEngineInstance12
            | SelfTest::Rsa2KModExpCrtEngineInstance13
            | SelfTest::Rsa2KModExpCrtEngineInstance14
            | SelfTest::Rsa2KModExpCrtEngineInstance15 => {
                if let Some(instance) = self_test.get_engine_instance() {
                    let self_test_env = self.env().borrow().clone();

                    self.env().borrow().pka_engine().self_test(
                        instance,
                        move |engine: &dyn PkaTrait| {
                            let result = engine.rsa_mod_exp_crt_self_test();

                            if result.is_err() {
                                // Handle the error case
                                self_test_env.hal().notify_self_test_failure(self_test);
                            }

                            if self_test_env
                                .hal()
                                .self_test_resp()
                                .send(SelfTestRespPacket { result })
                                .is_err()
                            {
                                error!("[rsa2k_crt] Failed to send self test response");
                            }
                        },
                    );
                } else {
                    self.send_self_test_response(Err(HsmErr::MissingSelfTestEngineInstance.into()));
                }
            }
            SelfTest::EcdsaEngineInstance0
            | SelfTest::EcdsaEngineInstance1
            | SelfTest::EcdsaEngineInstance2
            | SelfTest::EcdsaEngineInstance3
            | SelfTest::EcdsaEngineInstance4
            | SelfTest::EcdsaEngineInstance5
            | SelfTest::EcdsaEngineInstance6
            | SelfTest::EcdsaEngineInstance7
            | SelfTest::EcdsaEngineInstance8
            | SelfTest::EcdsaEngineInstance9
            | SelfTest::EcdsaEngineInstance10
            | SelfTest::EcdsaEngineInstance11
            | SelfTest::EcdsaEngineInstance12
            | SelfTest::EcdsaEngineInstance13
            | SelfTest::EcdsaEngineInstance14
            | SelfTest::EcdsaEngineInstance15 => {
                if let Some(instance) = self_test.get_engine_instance() {
                    let self_test_env = self.env().borrow().clone();

                    self.env().borrow().pka_engine().self_test(
                        instance,
                        move |engine: &dyn PkaTrait| {
                            let result = engine.ecdsa_self_test();

                            if result.is_err() {
                                // Handle the error case
                                self_test_env.hal().notify_self_test_failure(self_test);
                            }

                            if self_test_env
                                .hal()
                                .self_test_resp()
                                .send(SelfTestRespPacket { result })
                                .is_err()
                            {
                                error!("[ecdsa_engine] Failed to send self test response");
                            }
                        },
                    );
                } else {
                    self.send_self_test_response(Err(HsmErr::MissingSelfTestEngineInstance.into()));
                }
            }
            SelfTest::EcdhEngineInstance0
            | SelfTest::EcdhEngineInstance1
            | SelfTest::EcdhEngineInstance2
            | SelfTest::EcdhEngineInstance3
            | SelfTest::EcdhEngineInstance4
            | SelfTest::EcdhEngineInstance5
            | SelfTest::EcdhEngineInstance6
            | SelfTest::EcdhEngineInstance7
            | SelfTest::EcdhEngineInstance8
            | SelfTest::EcdhEngineInstance9
            | SelfTest::EcdhEngineInstance10
            | SelfTest::EcdhEngineInstance11
            | SelfTest::EcdhEngineInstance12
            | SelfTest::EcdhEngineInstance13
            | SelfTest::EcdhEngineInstance14
            | SelfTest::EcdhEngineInstance15 => {
                if let Some(instance) = self_test.get_engine_instance() {
                    let self_test_env = self.env().borrow().clone();

                    self.env().borrow().pka_engine().self_test(
                        instance,
                        move |engine: &dyn PkaTrait| {
                            let result = engine.ecdh_self_test();

                            if result.is_err() {
                                // Handle the error case
                                self_test_env.hal().notify_self_test_failure(self_test);
                            }

                            if self_test_env
                                .hal()
                                .self_test_resp()
                                .send(SelfTestRespPacket { result })
                                .is_err()
                            {
                                error!("[ecdh_engine] Failed to send self test response");
                            }
                        },
                    );
                } else {
                    self.send_self_test_response(Err(HsmErr::MissingSelfTestEngineInstance.into()));
                }
            }
            SelfTest::AesCbc => {
                let result = self.execute_aes_cbc_self_test();

                if result.is_err() {
                    // Handle the error case
                    self.env()
                        .borrow()
                        .clone()
                        .hal()
                        .notify_self_test_failure(self_test);
                }

                self.send_self_test_response(result);
            }
            SelfTest::Rng => {
                let self_test_env = self.env().borrow().clone();

                let self_test_drain_closure = move || {
                    let result = self_test_env.hal().rng().self_test();

                    if result.is_err() {
                        self_test_env.hal().notify_self_test_failure(self_test);
                    }

                    if self_test_env
                        .hal()
                        .self_test_resp()
                        .send(SelfTestRespPacket { result })
                        .is_err()
                    {
                        error!("[rng] Failed to send self test response");
                    }
                };

                if self.scheduler().drain(self_test_drain_closure).is_err() {
                    self.send_self_test_response(Err(HsmErr::DrainBusy.into()));
                }
            }
            _ => {
                unreachable!(
                    "Test Id: {:?} is unexpected in the HSM handler",
                    self_test as u32
                )
            }
        }
    }

    /// Helper to execute the AES CBC self test
    fn execute_aes_cbc_self_test(&self) -> Result<(), u32> {
        let env = self.env().borrow();
        let dma_heap = env.hal().dma_heap();

        let mut aes_self_test_input = dma_heap
            .allocate(AES_SELF_TEST_INPUT_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let mut aes_self_test_output = dma_heap
            .allocate(AES_SELF_TEST_OUTPUT_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let mut aes_self_test_iv = dma_heap
            .allocate(AES_SELF_TEST_IV_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;

        self.env().borrow().hal().aes().aes_cbc_self_test(
            aes_self_test_input.as_ref_mut(),
            aes_self_test_output.as_ref_mut(),
            aes_self_test_iv.as_ref_mut(),
        )
    }

    /// Helper function to send a response to self test request
    fn send_self_test_response(&self, result: Result<(), u32>) {
        let self_test_resp_packet = SelfTestRespPacket { result };

        if self
            .env()
            .borrow()
            .hal()
            .self_test_resp()
            .send(self_test_resp_packet)
            .is_err()
        {
            error!("Failed to send self test response");
        }
    }
}

#[cfg(test)]
mod tests {
    use mcr_self_test::SelfTestReqPacket;
    use mcr_self_test::SelfTestRespPacket;

    use super::*;
    use crate::handler::HsmEventHandler;
    use crate::mock::*;
    use crate::recorder::HsmFsmEventRecorder;
    use crate::resource::PkaResource;
    use crate::CmdResource;
    use crate::CmdScheduler;
    use crate::HsmFsmEvent;

    /// Configuration for the self test handler unit testing
    #[derive(Default, Copy, Clone)]
    struct TestConfig {
        /// If the intended test execution completed successfully or not
        test_fail: bool,
    }

    /// Set up the test environment for the self test
    ///
    /// # Arguments
    ///
    /// * `test_id` - The self test to be executed
    ///
    /// # Returns
    ///
    /// The handler with the test environment set up
    fn set_test_env(test_id: SelfTest, config: TestConfig) -> HsmEventHandler<MockEnv> {
        let mut env = MockEnv::new();
        let mut hal = MockHal::new();
        let mut self_test_req = MockSimplexPipe::<SelfTestReqPacket>::new();
        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }

        self_test_req
            .expect_recv()
            .once()
            .returning(move || Some(SelfTestReqPacket { test_id }));

        hal.expect_self_test_req()
            .once()
            .return_const(self_test_req);

        match test_id {
            SelfTest::Hkdf | SelfTest::Kbkdf => {
                let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                self_test_resp.expect_send().once().return_const(Ok(()));
                hal.expect_self_test_resp()
                    .once()
                    .return_const(self_test_resp);
                env.expect_pka_engine()
                    .times(1)
                    .return_const(CmdResource::new(
                        PkaResource::new(pka),
                        scheduler.clone(),
                        16,
                    ));

                if test_id == SelfTest::Hkdf {
                    let mut sha = MockSha::new();
                    sha.expect_hkdf_self_test_256().times(1).returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });
                    hal.expect_sha().times(1).return_const(sha);
                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    let hal_called_times = if config.test_fail { 4 } else { 3 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);
                } else if test_id == SelfTest::Kbkdf {
                    let mut sha = MockSha::new();
                    sha.expect_kbkdf_self_test_512()
                        .times(1)
                        .returning(move || {
                            if config.test_fail {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                    hal.expect_sha().times(1).return_const(sha);
                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    let hal_called_times = if config.test_fail { 4 } else { 3 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);
                }
            }
            SelfTest::EcdhEngineInstance0
            | SelfTest::EcdhEngineInstance1
            | SelfTest::EcdhEngineInstance2
            | SelfTest::EcdhEngineInstance3
            | SelfTest::EcdhEngineInstance4
            | SelfTest::EcdhEngineInstance5
            | SelfTest::EcdhEngineInstance6
            | SelfTest::EcdhEngineInstance7
            | SelfTest::EcdhEngineInstance8
            | SelfTest::EcdhEngineInstance9
            | SelfTest::EcdhEngineInstance10
            | SelfTest::EcdhEngineInstance11
            | SelfTest::EcdhEngineInstance12
            | SelfTest::EcdhEngineInstance13
            | SelfTest::EcdhEngineInstance14
            | SelfTest::EcdhEngineInstance15 => {
                let instance = test_id.get_engine_instance().unwrap();
                pka[instance]
                    .expect_ecdh_self_test()
                    .times(1)
                    .returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                let pka_rsrc = PkaResource::new(pka);
                let pka_engine = CmdResource::new(pka_rsrc, scheduler, 16);
                env.expect_pka_engine().times(2).return_const(pka_engine);

                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();
                    let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                    self_test_resp.expect_send().once().return_const(Ok(()));
                    hal.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);
                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    let hal_called_times = if config.test_fail { 2 } else { 1 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);

                    env
                });
                env.expect_hal().times(1).return_const(hal);
            }
            SelfTest::EcdsaEngineInstance0
            | SelfTest::EcdsaEngineInstance1
            | SelfTest::EcdsaEngineInstance2
            | SelfTest::EcdsaEngineInstance3
            | SelfTest::EcdsaEngineInstance4
            | SelfTest::EcdsaEngineInstance5
            | SelfTest::EcdsaEngineInstance6
            | SelfTest::EcdsaEngineInstance7
            | SelfTest::EcdsaEngineInstance8
            | SelfTest::EcdsaEngineInstance9
            | SelfTest::EcdsaEngineInstance10
            | SelfTest::EcdsaEngineInstance11
            | SelfTest::EcdsaEngineInstance12
            | SelfTest::EcdsaEngineInstance13
            | SelfTest::EcdsaEngineInstance14
            | SelfTest::EcdsaEngineInstance15 => {
                let instance = test_id.get_engine_instance().unwrap();
                pka[instance]
                    .expect_ecdsa_self_test()
                    .times(1)
                    .returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                let pka_rsrc = PkaResource::new(pka);
                let pka_engine = CmdResource::new(pka_rsrc, scheduler, 16);
                env.expect_pka_engine().times(2).return_const(pka_engine);

                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();
                    let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                    self_test_resp.expect_send().once().return_const(Ok(()));
                    hal.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);

                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    let hal_called_times = if config.test_fail { 2 } else { 1 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);

                    env
                });

                env.expect_hal().times(1).return_const(hal);
            }
            SelfTest::Rsa2KModExpEngineInstance0
            | SelfTest::Rsa2KModExpEngineInstance1
            | SelfTest::Rsa2KModExpEngineInstance2
            | SelfTest::Rsa2KModExpEngineInstance3
            | SelfTest::Rsa2KModExpEngineInstance4
            | SelfTest::Rsa2KModExpEngineInstance5
            | SelfTest::Rsa2KModExpEngineInstance6
            | SelfTest::Rsa2KModExpEngineInstance7
            | SelfTest::Rsa2KModExpEngineInstance8
            | SelfTest::Rsa2KModExpEngineInstance9
            | SelfTest::Rsa2KModExpEngineInstance10
            | SelfTest::Rsa2KModExpEngineInstance11
            | SelfTest::Rsa2KModExpEngineInstance12
            | SelfTest::Rsa2KModExpEngineInstance13
            | SelfTest::Rsa2KModExpEngineInstance14
            | SelfTest::Rsa2KModExpEngineInstance15 => {
                let instance = test_id.get_engine_instance().unwrap();
                pka[instance]
                    .expect_rsa_mod_exp_self_test()
                    .times(1)
                    .returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(Vec::new())
                        }
                    });

                let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                let pka_rsrc = PkaResource::new(pka);
                let pka_engine = CmdResource::new(pka_rsrc, scheduler, 16);
                env.expect_pka_engine().times(2).return_const(pka_engine);

                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();
                    let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                    self_test_resp.expect_send().once().return_const(Ok(()));
                    hal.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);

                    // Set up SHA mock with OAEP expectations only if PKA succeeds (for successful tests)
                    // If config.test_fail is true, PKA fails first so OAEP is never called
                    if !config.test_fail {
                        let mut sha = MockSha::new();
                        sha.expect_decode_oaep_kek_self_test()
                            .times(1)
                            .returning(move |_, _| Ok(()));
                        hal.expect_sha().times(1).return_const(sha);
                    }

                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }

                    env.expect_hal().times(2).return_const(hal);

                    env
                });

                env.expect_hal().times(1).return_const(hal);
            }
            SelfTest::Rsa2KModExpCrtEngineInstance0
            | SelfTest::Rsa2KModExpCrtEngineInstance1
            | SelfTest::Rsa2KModExpCrtEngineInstance2
            | SelfTest::Rsa2KModExpCrtEngineInstance3
            | SelfTest::Rsa2KModExpCrtEngineInstance4
            | SelfTest::Rsa2KModExpCrtEngineInstance5
            | SelfTest::Rsa2KModExpCrtEngineInstance6
            | SelfTest::Rsa2KModExpCrtEngineInstance7
            | SelfTest::Rsa2KModExpCrtEngineInstance8
            | SelfTest::Rsa2KModExpCrtEngineInstance9
            | SelfTest::Rsa2KModExpCrtEngineInstance10
            | SelfTest::Rsa2KModExpCrtEngineInstance11
            | SelfTest::Rsa2KModExpCrtEngineInstance12
            | SelfTest::Rsa2KModExpCrtEngineInstance13
            | SelfTest::Rsa2KModExpCrtEngineInstance14
            | SelfTest::Rsa2KModExpCrtEngineInstance15 => {
                let instance = test_id.get_engine_instance().unwrap();
                let mut pka = Vec::new();
                for _ in 0..16 {
                    pka.push(MockPka::new());
                }
                pka[instance]
                    .expect_rsa_mod_exp_self_test()
                    .times(1)
                    .returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(Vec::new())
                        }
                    });

                let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
                let pka_rsrc = PkaResource::new(pka);
                let pka_engine = CmdResource::new(pka_rsrc, scheduler, 16);
                env.expect_pka_engine().times(1).return_const(pka_engine);

                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();
                    let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                    self_test_resp.expect_send().once().return_const(Ok(()));
                    hal.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);

                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    let hal_called_times = if config.test_fail { 2 } else { 1 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);

                    env
                });

                env.expect_hal().times(1).return_const(hal);
            }
            SelfTest::AesCbc => {
                env.expect_pka_engine()
                    .times(1)
                    .return_const(CmdResource::new(
                        PkaResource::new(pka),
                        scheduler.clone(),
                        16,
                    ));
                let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                self_test_resp.expect_send().once().return_const(Ok(()));
                hal.expect_self_test_resp()
                    .once()
                    .return_const(self_test_resp);
                let mut dma_heap = MockDmaHeap::new();
                dma_heap
                    .expect_allocate()
                    .times(3)
                    .returning(|s| Some(MockDmaAlloc::new(s)));

                let mut aes = MockAes::new();
                aes.expect_aes_cbc_self_test()
                    .times(1)
                    .returning(move |_, _, _| {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });
                hal.expect_aes().times(1).return_const(aes);
                hal.expect_dma_heap().times(1).return_const(dma_heap);

                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();

                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }
                    env.expect_hal().times(1).return_const(hal);

                    env
                });

                env.expect_hal().times(4).return_const(hal);
            }
            SelfTest::Rng => {
                env.expect_pka_engine()
                    .times(1)
                    .return_const(CmdResource::new(
                        PkaResource::new(pka),
                        scheduler.clone(),
                        16,
                    ));
                env.expect_clone().once().returning(move || {
                    let mut hal = MockHal::new();
                    let mut env = MockEnv::new();
                    let mut self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
                    self_test_resp.expect_send().once().return_const(Ok(()));
                    hal.expect_self_test_resp()
                        .once()
                        .return_const(self_test_resp);

                    let mut rng = MockRng::new();
                    rng.expect_self_test().times(1).returning(move || {
                        if config.test_fail {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });
                    hal.expect_rng().times(1).return_const(rng);

                    if config.test_fail {
                        hal.expect_notify_self_test_failure()
                            .times(1)
                            .returning(move |_| {
                                print!(
                                    "Simulating self-test failure for test ID: {}",
                                    test_id as u32
                                );
                            });
                    }

                    let hal_called_times = if config.test_fail { 3 } else { 2 };

                    env.expect_hal()
                        .times(hal_called_times as usize)
                        .return_const(hal);

                    env
                });
                env.expect_hal().times(1).return_const(hal);
            }
            _ => (),
        }

        HsmEventHandler::new(env, scheduler)
    }

    #[test]
    fn handle_hsm_self_test_ecdh0_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::EcdhEngineInstance0, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_exdh9_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::EcdhEngineInstance9, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_ecdh14_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::EcdhEngineInstance14, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_ecdsa0_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::EcdsaEngineInstance0, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_ecdsa9_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::EcdsaEngineInstance9, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_ecdsa14_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::EcdsaEngineInstance14, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_rsa_mod_exp0_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::Rsa2KModExpEngineInstance0, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_rsa_mod_exp9_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::Rsa2KModExpEngineInstance9, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_rsa_mod_exp14_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::Rsa2KModExpEngineInstance14, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_hkdf_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::Hkdf, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_hkdf_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::Hkdf, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_kbkdf_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::Kbkdf, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_kbkdf_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::Kbkdf, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_aes_cbc_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::AesCbc, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_aes_cbc_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::AesCbc, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_rng_pass() {
        let test_config = TestConfig::default();
        let mut handler = set_test_env(SelfTest::Rng, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_hsm_self_test_rng_fail() {
        let test_config = TestConfig { test_fail: true };
        let mut handler = set_test_env(SelfTest::Rng, test_config);

        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    #[test]
    fn handle_self_test_completed() {
        let mut env = MockEnv::new();
        let mut hal = MockHal::new();
        let mut self_test_req = MockSimplexPipe::<SelfTestReqPacket>::new();
        let self_test_resp = MockSimplexPipe::<SelfTestRespPacket>::new();
        let mut pka = Vec::new();
        for _ in 0..16 {
            pka.push(MockPka::new());
        }

        self_test_req.expect_recv().once().returning(move || {
            Some(SelfTestReqPacket {
                test_id: SelfTest::SelfTestCompleted,
            })
        });

        hal.expect_self_test_req()
            .once()
            .return_const(self_test_req);

        hal.expect_self_test_resp()
            .once()
            .return_const(self_test_resp);

        env.expect_clone().once().returning(MockEnv::new);

        env.expect_hal().times(1).return_const(hal);

        let scheduler = CmdScheduler::new(65, 1, HsmFsmEventRecorder::default());
        env.expect_pka_engine()
            .times(1)
            .return_const(CmdResource::new(
                PkaResource::new(pka),
                scheduler.clone(),
                16,
            ));

        let mut handler = HsmEventHandler::new(env, scheduler);

        // Capture the panic gracefully to verify the panic message
        let result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(move || {
            handler.on_event(HsmFsmEvent::SelfTestRequest)
        }));

        assert!(result.is_err());

        let panic_message = result.unwrap_err();
        let panic_str = if let Some(s) = panic_message.downcast_ref::<&str>() {
            *s
        } else if let Some(s) = panic_message.downcast_ref::<String>() {
            s.as_str()
        } else {
            "Unknown panic"
        };

        let expected = format!(
            "entered unreachable code: Test Id: {} is unexpected in the HSM handler",
            SelfTest::SelfTestCompleted as u32
        );
        assert!(panic_str.contains(&expected));
    }
}
