// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::mock::MockDmaHeap;
use crate::mock::MockEnv;
use crate::mock::MockHal;
use crate::mock::MockPka;
use crate::mock::MockRng;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::set_ipc_expectations;
use crate::partition::PartEnv;

#[test]
fn test_rng() {
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_rng().once().return_const(MockRng::new());
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let _ = env.rng();
}

#[test]
fn test_dma_heap() {
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_dma_heap()
        .once()
        .return_const(MockDmaHeap::new());
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    let _ = env.dma_heap();
}

#[test]
fn test_vault_addr() {
    let mut hal = MockHal::new();
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);
    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_vault_addr().once().returning(|| 0);
    set_ipc_expectations(&mut hal);

    let env = PartEnv::<MockEnv>::new(hal, cmd_scheduler());
    assert_eq!(env.vault_addr(), 0);
}
