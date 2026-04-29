// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_queue_controller::*;
use mcr_types::*;

use crate::error::AdminErr;
use crate::function::CntrlStateChangeAction;
use crate::function::FunctionMgr;
use crate::function::FunctionMgrTrait;
use crate::function::FunctionTrait;
use crate::function::MAX_FUNCTION_RESOURCES;
use crate::mock::*;

static mut CRC_VOL: VolatileCell<u32> = VolatileCell::new(0);

#[test]
fn test_query_state_change_enable() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_enabled().once().returning(|| true);
            qc.expect_id().times(1).returning(move || cntrl_id);
            qc.expect_ready().times(2).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        assert_eq!(func.query_state_change(), CntrlStateChangeAction::Enable);
    }
}

#[test]
fn test_query_state_change_disable() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_enabled().once().returning(|| false);
            qc.expect_ready().times(2).returning(|| true);
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        assert_eq!(func.query_state_change(), CntrlStateChangeAction::Disable);
    }
}

#[test]
fn test_query_state_change_invalid() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_enabled().once().returning(|| true);
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(2).returning(|| true);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        assert_eq!(func.query_state_change(), CntrlStateChangeAction::Invalid);
    }
}

#[test]
fn test_cntrl_id() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(2).returning(move || cntrl_id);
            qc.expect_ready().once().returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        assert!(pfn == func.cntrl_id().into());
    }
}

#[test]
fn test_enable_function() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }
}

#[test]
fn test_ready_function() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(2).returning(|| true);
            qc.expect_id().times(5).returning(move || cntrl_id);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
        assert!(func.ready());
    }
}

#[test]
fn test_enable_function_create_asq_fails() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq()
                .once()
                .returning(|_, _| Err(u32::MAX));
            qc.expect_delete_cq().once().returning(|_, _| ());
            qc.expect_id().times(4).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }
}

#[test]
fn test_disable_function() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_delete_cq().once().returning(|_, _| ());
            qc.expect_delete_sq().once().returning(|_, _| ());
            qc.expect_disable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        func.disable();
    }
}

#[test]
fn test_reset_function() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_delete_cq().once().returning(|_, _| ());
            qc.expect_delete_sq().once().returning(|_, _| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_reset().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    fnmgr.reset();
}

#[test]
fn test_set_res_cnt() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(4).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        assert_eq!(
            fnmgr.function(pfn).res_mask(),
            (1u128 << res_id).to_le_bytes()
        );
    }
}

#[test]
fn test_set_all_res_cnt_to_pf() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 65);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 65);
}

#[test]
fn test_reallocate_set_res_cnt_to_pf_and_vf0() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id == QueueCntrlId::Vf0 {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 65);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 65);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 32);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 65);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 32);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf0, 33);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf0).res_cnt(), 33);
}

#[test]
fn test_set_res_cnt_across_5_functions() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id == QueueCntrlId::Vf0
                || cntrl_id == QueueCntrlId::Vf1
                || cntrl_id == QueueCntrlId::Vf2
                || cntrl_id == QueueCntrlId::Vf3
            {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 25);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 25);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf0, 10);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf0).res_cnt(), 10);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf1, 10);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf1).res_cnt(), 10);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf2, 10);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf2).res_cnt(), 10);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf3, 10);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf3).res_cnt(), 10);
}

#[test]
fn test_set_res_cnt_fails_expcept_pf() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 65);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 65);

    for pfn in PcieFunction::iter() {
        if pfn != PcieFunction::Pf {
            let cnt = fnmgr.set_res_cnt(pfn, 2);
            assert_eq!(cnt.err(), Some(AdminErr::SetResCountLimitExceeded));
        }
    }
}

#[test]
fn test_create_cq_with_no_resource() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cq = func.create_cq(HostCqId(1), mem, None);
        assert_eq!(cq.err(), Some(AdminErr::QueueIdOutOfRangeForFunction));
    }
}

#[test]
fn test_create_cq_for_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
    }
}

#[test]
fn test_create_cq_for_hsm_already_allocated() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert_eq!(cq.err(), Some(AdminErr::CqAlreadyAllocated));
    }
}

#[test]
fn test_create_cq_for_invalid_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(66), mem, None);
        assert_eq!(cq.err(), Some(AdminErr::InvalidQueueId));
    }
}

#[test]
fn test_create_cq_failed_form_queue_controller() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_cq()
                .times(1)
                .returning(|_, _, _, _| Err(u32::MAX));
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert_eq!(cq.err(), Some(AdminErr::CreateCqFailedByQueueController));
    }
}

#[test]
fn test_create_cq_for_fp() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
    }
}

#[test]
fn test_create_cq_for_fp_odd_queue_id() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());
    }
}

#[test]
fn test_create_cq_for_invalid_fp_queue_id() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(511), mem, None);
        assert_eq!(cq.err(), Some(AdminErr::InvalidQueueId));
    }
}

#[test]
fn test_create_all_cq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let prev_cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(prev_cnt.is_ok());
        assert_eq!(prev_cnt.unwrap(), 0);

        assert_eq!(func.res_cnt(), 1);

        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());
    }
}

#[test]
fn test_delete_cq_for_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_delete_cq().once().returning(|_, _| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        assert!(func.delete_cq(HostCqId(1)).is_ok());
    }
}

#[test]
fn test_delete_invalid_cq_for_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let result = func.delete_cq(HostCqId(70));
        assert_eq!(result.err(), Some(AdminErr::InvalidQueueId));
    }
}

#[test]
fn test_delete_out_of_range_cq_for_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let result = func.delete_cq(HostCqId(3));
        assert_eq!(result.err(), Some(AdminErr::QueueIdOutOfRangeForFunction));
    }
}

#[test]
fn test_delete_cq_for_hsm_that_was_not_created() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        assert_eq!(
            func.delete_cq(HostCqId(1)).err(),
            Some(AdminErr::InvalidCompletionQueueIdInDelete)
        );
    }
}

#[test]
fn test_delete_cq_with_valid_host_sq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        assert!(func.dev_sq(HostSqId::Id1) == Ok(DevSqId(65 + res_id)));
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        assert_eq!(
            func.delete_cq(HostCqId::Id1).err(),
            Some(AdminErr::InvalidQueueDelete)
        );
        assert_eq!(
            func.delete_cq(HostCqId::Id256).err(),
            Some(AdminErr::InvalidQueueDelete)
        );
        assert_eq!(
            func.delete_cq(HostCqId::Id257).err(),
            Some(AdminErr::InvalidQueueDelete)
        );
    }
}

#[test]
fn test_create_out_of_range_sq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        assert_eq!(
            func.create_sq(HostSqId(3), HostCqId(1), mem).err(),
            Some(AdminErr::QueueIdOutOfRangeForFunction)
        );
    }
}

#[test]
fn test_create_invalid_sq_id() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        assert_eq!(
            func.create_sq(HostSqId(88), HostCqId(1), mem).err(),
            Some(AdminErr::InvalidQueueId)
        );
    }
}

#[test]
fn test_create_sq_with_no_cq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        assert_eq!(
            func.create_sq(HostSqId(1), HostCqId(1), mem).err(),
            Some(AdminErr::CqNotAvailable)
        );
    }
}

#[test]
fn test_create_cq_and_sq_for_hsm() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().once().returning(|_, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }
}

#[test]
fn test_create_all_cq_and_sq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }
}

#[test]
fn test_create_cq_and_sq_already_created() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().once().returning(|_, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(1).returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let func = fnmgr.function(pfn);
        let res_id = res_id as u8;
        let _ = func.enable();

        let prev_cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(prev_cnt.is_ok());
        assert_eq!(prev_cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert_eq!(result.err(), Some(AdminErr::SqAlreadyAllocated));
    }
}

#[test]
fn test_create_sq_fails_at_controller() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq().once().returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq()
                .once()
                .returning(|_, _, _| Err(u32::MAX));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert_eq!(
            result.err(),
            Some(AdminErr::CreateSqFailedByQueueController)
        );
    }
}

#[test]
fn test_delete_sq_invalid_sq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let result = func.delete_sq(HostSqId::Id78);
        assert_eq!(result.err(), Some(AdminErr::InvalidQueueId));
    }
}

#[test]
fn test_delete_sq_out_of_range() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let result = func.delete_sq(HostSqId::Id2);
        assert_eq!(result.err(), Some(AdminErr::QueueIdOutOfRangeForFunction));
    }
}

#[test]
fn test_delete_all_cq_and_sq() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_delete_sq().times(3).returning(|_, _| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        let result = func.delete_sq(HostSqId::Id1);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        let result = func.delete_sq(HostSqId::Id256);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));

        let result = func.delete_sq(HostSqId::Id257);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }
}

#[test]
fn test_delete_sq_already_deleted() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_delete_sq().times(3).returning(|_, _| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        let result = func.delete_sq(HostSqId::Id1);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        let result = func.delete_sq(HostSqId::Id256);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));

        let result = func.delete_sq(HostSqId::Id257);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));

        assert_eq!(
            func.delete_sq(HostSqId::Id1).err(),
            Some(AdminErr::InvalidSqId)
        );
        assert_eq!(
            func.delete_sq(HostSqId::Id256).err(),
            Some(AdminErr::InvalidSqId)
        );
        assert_eq!(
            func.delete_sq(HostSqId::Id257).err(),
            Some(AdminErr::InvalidSqId)
        );
    }
}

#[test]
fn test_set_reset_on_pf_with_all_queues_enabled() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(195)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(195).returning(|_, _, _| Ok(()));
            qc.expect_delete_sq().times(260).returning(|_, _| ());
            qc.expect_delete_cq().times(260).returning(|_, _| ());
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(133).returning(move || cntrl_id);
            qc.expect_reset().times(65).returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        assert!(func.enable().is_ok());

        let cnt = fnmgr.set_res_cnt(pfn, 65);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 65);

        let mem = QueueMem {
            addr: Default::default(),
            len: Default::default(),
        };

        for res_id in 0..MAX_FUNCTION_RESOURCES {
            let hsm_host_queue_id = 1 + res_id as u16;
            let fp_even_host_queue_id = 256 + (res_id as u16 * 2);
            let fp_odd_host_queue_id = 257 + (res_id as u16 * 2);

            assert!(func
                .create_cq(HostCqId(hsm_host_queue_id), mem, None)
                .is_ok());
            assert!(func
                .create_cq(HostCqId(fp_even_host_queue_id), mem, None)
                .is_ok());
            assert!(func
                .create_cq(HostCqId(fp_odd_host_queue_id), mem, None)
                .is_ok());

            assert!(func
                .create_sq(
                    HostSqId(hsm_host_queue_id),
                    HostCqId(hsm_host_queue_id),
                    mem
                )
                .is_ok());
            assert!(func
                .create_sq(
                    HostSqId(fp_even_host_queue_id),
                    HostCqId(fp_even_host_queue_id),
                    mem
                )
                .is_ok());
            assert!(func
                .create_sq(
                    HostSqId(fp_odd_host_queue_id),
                    HostCqId(fp_odd_host_queue_id),
                    mem
                )
                .is_ok());
        }

        fnmgr.reset();
    }
}

#[test]
fn test_admin_queue_cntrl_not_ready() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |_| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().times(2).returning(|_, _| Ok(()));
            qc.expect_create_asq().times(2).returning(|_, _| Ok(()));
            qc.expect_enable().times(2).returning(|| ());
            qc.expect_id().times(7).returning(|| QueueCntrlId::Pf);
            qc.expect_disable().times(1).returning(|| ());
            qc.expect_delete_cq().times(1).returning(|_, _| ());
            qc.expect_delete_sq().times(1).returning(|_, _| ());
            qc.expect_ready().times(1).returning(|| false);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let admin_queue_reference_1 = func.admin_queue();
        assert!(admin_queue_reference_1.as_ref().unwrap().valid());
        assert!(admin_queue_reference_1.as_ref().unwrap().sq_id() == QueueCntrlId::Pf.into());
        assert!(admin_queue_reference_1.as_ref().unwrap().cq_id() == QueueCntrlId::Pf.into());

        func.disable();
        assert!(!admin_queue_reference_1.as_ref().unwrap().valid());

        let _ = func.enable();
        let admin_queue_reference_2 = func.admin_queue();
        assert!(admin_queue_reference_2.as_ref().unwrap().valid());
    }
}

#[test]
fn test_admin_queue_cntrl_ready() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |_| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().times(2).returning(|_, _| Ok(()));
            qc.expect_create_asq().times(2).returning(|_, _| Ok(()));
            qc.expect_enable().times(2).returning(|| ());
            qc.expect_id().times(9).returning(|| QueueCntrlId::Pf);
            qc.expect_disable().times(1).returning(|| ());
            qc.expect_delete_cq().times(1).returning(|_, _| ());
            qc.expect_delete_sq().times(1).returning(|_, _| ());
            qc.expect_ready().times(1).returning(|| true);
            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let admin_queue_reference_1 = func.admin_queue();
        assert!(admin_queue_reference_1.as_ref().unwrap().valid());
        assert!(admin_queue_reference_1.as_ref().unwrap().sq_id() == QueueCntrlId::Pf.into());
        assert!(admin_queue_reference_1.as_ref().unwrap().cq_id() == QueueCntrlId::Pf.into());

        func.disable();
        assert!(!admin_queue_reference_1.as_ref().unwrap().valid());

        let _ = func.enable();
        let admin_queue_reference_2 = func.admin_queue();
        assert!(admin_queue_reference_2.as_ref().unwrap().valid());
    }
}

#[test]
fn test_set_res_cnt_on_enabled_vfs() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(2).returning(|| true);
                qc.expect_clear_enable().times(2).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let res = fnmgr.set_res_cnt(pfn, 65);
        assert!(res.is_ok());
        assert_eq!(res.unwrap(), 0);
        assert_eq!(fnmgr.function(pfn).res_cnt(), 65);

        let res = fnmgr.set_res_cnt(pfn, 0);
        assert!(res.is_ok());
        assert_eq!(fnmgr.function(pfn).res_cnt(), 0);
    }
}

#[test]
fn test_set_res_cnt_across_5_functions_and_disable() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            if cntrl_id == QueueCntrlId::Vf3 {
                qc.expect_create_acq().once().returning(|_, _| Ok(()));
                qc.expect_create_asq().once().returning(|_, _| Ok(()));
                qc.expect_enable().once().returning(|| ());
                qc.expect_id().times(4).returning(move || cntrl_id);
                qc.expect_create_cq()
                    .times(2)
                    .returning(|_, _, _, _| Ok(()));
                qc.expect_disable().times(1).returning(|| ());
                qc.expect_delete_cq().times(2).returning(|_, _| ());
                qc.expect_delete_sq().times(1).returning(|_, _| ());
            }
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id == QueueCntrlId::Vf0
                || cntrl_id == QueueCntrlId::Vf1
                || cntrl_id == QueueCntrlId::Vf2
                || cntrl_id == QueueCntrlId::Vf3
            {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);
        let _ = func.enable();
    }

    let cnt = fnmgr.set_res_cnt(PcieFunction::Pf, 1);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Pf).res_cnt(), 1);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf0, 1);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf0).res_cnt(), 1);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf1, 1);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf1).res_cnt(), 1);

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf2, 1);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf2).res_cnt(), 1);

    let mem = QueueMem {
        addr: Default::default(),
        len: Default::default(),
    };

    let cnt = fnmgr.set_res_cnt(PcieFunction::Vf3, 1);
    assert!(cnt.is_ok());
    assert_eq!(cnt.unwrap(), 0);
    assert_eq!(fnmgr.function(PcieFunction::Vf3).res_cnt(), 1);

    // check if the completion queue create can be successful
    let func = fnmgr.function(PcieFunction::Vf3);
    let cq = func.create_cq(HostCqId(1), mem, None);
    assert!(cq.is_ok());

    func.disable();

    let _ = func.enable();
    let cq = func.create_cq(HostCqId(1), mem, None);
    assert!(cq.is_ok());
}

#[test]
fn test_save_lm_context() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(5).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        let major = lm_context.version.major;
        let minor = lm_context.version.minor;
        assert_eq!(major, 1);
        assert_eq!(minor, 0);
    }
}

#[test]
fn test_restore_lm_context_with_invalid_crc() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(5).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().once().returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        lm_context.crc = 0;

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::InvalidLmContextToRestore)
        );
    }
}

#[test]
fn test_restore_lm_context_with_invalid_resource_count() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(5).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(2).returning(|| true);
                qc.expect_clear_enable().times(2).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::ResourceLimitReached)
        );
    }
}

#[test]
fn test_restore_lm_context_restore_admin_completion_queue_failed() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(7).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            qc.expect_restore_host_register_info()
                .times(1)
                .returning(|_| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Err(u32::MAX));
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(3).returning(|| true);
                qc.expect_clear_enable().times(3).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());
        assert!(fnmgr.set_res_cnt(pfn, 1).is_ok());

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::CreateCqFailedByQueueController)
        );
    }
}

#[test]
fn test_restore_lm_context_restore_admin_submission_queue_failed() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(7).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            qc.expect_restore_host_register_info()
                .times(1)
                .returning(|_| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(1)
                .returning(|_, _| Err(u32::MAX));
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(3).returning(|| true);
                qc.expect_clear_enable().times(3).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());
        assert!(fnmgr.set_res_cnt(pfn, 1).is_ok());

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::CreateSqFailedByQueueController)
        );
    }
}

#[test]
fn test_restore_lm_context_restore_io_completion_queue_failed() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(7).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            qc.expect_restore_host_register_info()
                .times(1)
                .returning(|_| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_enable_sq().times(1).returning(|_, _| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Err(u32::MAX));
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(3).returning(|| true);
                qc.expect_clear_enable().times(3).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());
        assert!(fnmgr.set_res_cnt(pfn, 1).is_ok());

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::CreateCqFailedByQueueController)
        );
    }
}

#[test]
fn test_restore_lm_context_restore_io_submission_queue_failed() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(7).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            qc.expect_restore_host_register_info()
                .times(1)
                .returning(|_| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_enable_sq().times(1).returning(|_, _| ());
            qc.expect_restore_cq_info()
                .times(3)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(1)
                .returning(|_, _| Err(u32::MAX));
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(3).returning(|| true);
                qc.expect_clear_enable().times(3).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());
        assert!(fnmgr.set_res_cnt(pfn, 1).is_ok());

        assert_eq!(
            func.restore_lm_context(&mut lm_context),
            Err(AdminErr::CreateSqFailedByQueueController)
        );
    }
}

#[test]
fn test_restore_lm_context() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(7).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            qc.expect_cq_info().times(4).returning(get_cq_info);
            qc.expect_host_register_info()
                .times(1)
                .returning(get_controller_info);
            qc.expect_sq_info().times(4).returning(get_sq_info);
            qc.expect_restore_host_register_info()
                .times(1)
                .returning(|_| ());
            qc.expect_restore_cq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(1)
                .returning(|_, _| Ok(()));
            qc.expect_enable_sq().times(1).returning(|_, _| ());
            qc.expect_restore_cq_info()
                .times(3)
                .returning(|_, _| Ok(()));
            qc.expect_restore_sq_info()
                .times(3)
                .returning(|_, _| Ok(()));
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(3).returning(|| true);
                qc.expect_clear_enable().times(3).returning(|| ());
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    let masked_bk_boot = MaskedBkBoot {
        len: MASKED_BK_BOOT_SIZE as u32,
        data: [1u8; MASKED_BK_BOOT_SIZE],
    };

    let sealed_bk3 = SealedBk3 {
        len: SEALED_BK3_SIZE as u32,
        data: [1u8; SEALED_BK3_SIZE],
    };

    for pfn in PcieFunction::iter() {
        let func = fnmgr.function(pfn);

        let mut lm_context = default_lm_context();

        func.save_lm_context(&mut lm_context, 0, &masked_bk_boot, &sealed_bk3);

        assert!(fnmgr.set_res_cnt(pfn, 0).is_ok());
        assert!(fnmgr.set_res_cnt(pfn, 1).is_ok());

        assert!(func.restore_lm_context(&mut lm_context).is_ok());
    }
}

#[test]
fn test_get_enabled_sq_info() {
    let resource: Vec<Resource> = vec![Resource::default(); MAX_FUNCTION_RESOURCES as usize];
    let resource_table = unsafe {
        &mut *(resource.as_ptr() as usize as *mut [Resource; MAX_FUNCTION_RESOURCES as usize])
    };
    let crc_ptr = std::ptr::addr_of!(CRC_VOL);
    let crc = unsafe { &*crc_ptr };

    let fnmgr = FunctionMgr::new(
        |cntrl_id| {
            let mut qc = MockQueueController::new();
            qc.expect_create_cq()
                .times(3)
                .returning(|_, _, _, _| Ok(()));
            qc.expect_create_sq().times(3).returning(|_, _, _| Ok(()));
            qc.expect_id().times(3).returning(move || cntrl_id);
            qc.expect_create_acq().once().returning(|_, _| Ok(()));
            qc.expect_create_asq().once().returning(|_, _| Ok(()));
            qc.expect_enable().once().returning(|| ());
            qc.expect_ready().times(1).returning(|| false);
            if cntrl_id != QueueCntrlId::Pf {
                qc.expect_enabled().times(1).returning(|| false);
            }

            qc
        },
        resource_table,
        crc,
    );

    assert!(fnmgr.is_ok());
    let fnmgr = fnmgr.unwrap();

    let mem = QueueMem {
        addr: MemoryAddr {
            lo: 0x40000000,
            hi: 0x01,
        },
        len: Default::default(),
    };

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let res_id = res_id as u8;
        let func = fnmgr.function(pfn);
        let _ = func.enable();

        let cnt = fnmgr.set_res_cnt(pfn, 1);
        assert!(cnt.is_ok());
        assert_eq!(cnt.unwrap(), 0);
        assert_eq!(func.res_cnt(), 1);
        let cq = func.create_cq(HostCqId(1), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(256), mem, None);
        assert!(cq.is_ok());
        let cq = func.create_cq(HostCqId(257), mem, None);
        assert!(cq.is_ok());

        let result = func.create_sq(HostSqId::Id1, HostCqId::Id1, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
        let result = func.create_sq(HostSqId::Id256, HostCqId::Id256, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(res_id));
        assert!(dev_cq == DevCqId(res_id));
        let result = func.create_sq(HostSqId::Id257, HostCqId::Id257, mem);
        assert!(result.is_ok());
        let (dev_sq, dev_cq) = result.unwrap();
        assert!(dev_sq == DevSqId(65 + res_id));
        assert!(dev_cq == DevCqId(65 + res_id));
    }

    for (res_id, pfn) in PcieFunction::iter().enumerate() {
        let func = fnmgr.function(pfn);

        let enabled_sq_info = func.get_enabled_sq_info();
        assert_eq!(enabled_sq_info.len(), 3);

        for (i, sq_info) in enabled_sq_info.iter().enumerate() {
            let (expected_host_sq, expected_dev_sq, expected_dev_cq) = match i {
                0 => (
                    HostSqId::Id1,
                    DevSqId(65 + res_id as u8),
                    DevCqId(65 + res_id as u8),
                ),
                1 => (
                    HostSqId::Id256,
                    DevSqId(res_id as u8),
                    DevCqId(res_id as u8),
                ),
                2 => (
                    HostSqId::Id257,
                    DevSqId(65 + res_id as u8),
                    DevCqId(65 + res_id as u8),
                ),
                _ => unreachable!(),
            };

            assert!(sq_info.0 == expected_host_sq);
            assert!(sq_info.1 == expected_dev_sq);
            assert!(sq_info.2 == expected_dev_cq);
        }
    }
}

fn default_lm_context() -> VmLiveMigrationInfo {
    VmLiveMigrationInfo {
        version: LmVersionInfo { minor: 1, major: 1 },
        resource_cnt: 1,
        sq_cnt: 3,
        cq_cnt: 3,
        cntrl_info: get_controller_info(),
        admin_cq_info: get_cq_info(DevCqId(0), HostCqId(0)),
        admin_sq_info: get_sq_info(DevSqId(0), HostSqId(0)),
        io_cq_info: [get_cq_info(DevCqId(0), HostCqId(0)); 195],
        io_sq_info: [get_sq_info(DevSqId(0), HostSqId(0)); 195],
        masked_bk_boot: MaskedBkBoot {
            len: MASKED_BK_BOOT_SIZE as u32,
            data: [1u8; MASKED_BK_BOOT_SIZE],
        },
        sealed_bk3: SealedBk3 {
            len: SEALED_BK3_SIZE as u32,
            data: [1u8; SEALED_BK3_SIZE],
        },
        session_allocation_mask: 0,
        reserved: [0u8; 3],
        crc: 0,
    }
}

fn get_cq_info(_cq_info: DevCqId, host_cq: HostCqId) -> LmCqInfo {
    LmCqInfo {
        id: host_cq.0,
        len: 1,
        addr: MemoryAddr { lo: 0, hi: 0 },
        attr: LmCqAttributes::new(),
        iv: 0,
        head: 0,
        tail: 0,
    }
}

fn get_sq_info(_dev_sq: DevSqId, host_sq: HostSqId) -> LmSqInfo {
    LmSqInfo {
        id: host_sq.0,
        len: 1,
        cq_id: host_sq.0,
        rsvd: 0,
        addr: MemoryAddr { lo: 0, hi: 0 },
        head: 0,
        tail: 0,
    }
}

fn get_controller_info() -> ControllerLmInfo {
    ControllerLmInfo {
        version: 1,
        ivms: 0,
        configuration: 0,
        aq_attr: 0,
        asq_addr_lo: 0,
        asq_addr_hi: 0,
        acq_addr_lo: 0,
        acq_addr_hi: 0,
        memory_buffer_location: 0,
        memory_buffer_size: 0,
    }
}
