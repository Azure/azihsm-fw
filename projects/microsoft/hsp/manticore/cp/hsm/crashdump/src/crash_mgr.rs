// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::{
    CpuRegisterContext, CrashDumpBlock, CrashDumpBody, CrashDumpHeader, RegisterBlockContext,
};
use core::fmt;
use core::mem::size_of;
use log::*;
use zerocopy::IntoBytes;

const DUMP_HEADER_VERSION: u16 = 0x1;
const DUMP_HEADER_MAGIC_COMITTED: u32 = 0x4D446D70u32; // MDmp
const DUMP_HEADER_MAGIC_DIRTY: u32 = 0x2BB2928F; // ~MDmp

pub(crate) trait CrashDump {
    fn create_dump(
        &mut self,
        failure_code: u32,
        cpu_reg_context: &CpuRegisterContext,
        register_block: &RegisterBlockContext,
        cpu_id: u8,
        additional_info: &str,
    );
}

pub(crate) struct CrashDumpManager<'a> {
    base: &'a mut [u8],
}

impl<'a> CrashDumpManager<'a> {
    pub(crate) fn new(base: &'a mut [u8]) -> Self {
        Self { base }
    }

    pub(crate) fn commit_crash_dump(&mut self) {
        self.base[0..size_of::<u32>()].copy_from_slice(DUMP_HEADER_MAGIC_COMITTED.as_bytes());
    }

    #[allow(dead_code)]
    pub(crate) fn dirty_crash_dump(&mut self) {
        self.base[0..size_of::<u32>()].copy_from_slice(DUMP_HEADER_MAGIC_DIRTY.as_bytes());
    }

    #[allow(dead_code)]
    pub(crate) fn get_crashdump(&self) -> &CrashDumpBlock {
        unsafe { &*(self.base.as_ptr() as *const CrashDumpBlock) }
    }
}

impl CrashDump for CrashDumpManager<'_> {
    fn create_dump(
        &mut self,
        failure_code: u32,
        cpu_reg_context: &CpuRegisterContext,
        register_block: &RegisterBlockContext,
        cpu_id: u8,
        additional_info: &str,
    ) {
        let payload_size = size_of::<CrashDumpBody>() + additional_info.len();
        let mut block = CrashDumpBlock {
            header: CrashDumpHeader {
                magic: DUMP_HEADER_MAGIC_DIRTY,
                failure_code,
                crashdump_version: DUMP_HEADER_VERSION,
                core_type: cpu_id,
                dump_type: CrashDumpBlock::get_dump_type(),
                crash_type: 0,
                _rsvd: 0u8,
                payload_size: payload_size.try_into().unwrap(),
            },

            body: CrashDumpBody {
                stack_ptr: cpu_reg_context.sp,
                xpsr: 0,
                r0: cpu_reg_context.r0,
                r1: cpu_reg_context.r1,
                r2: cpu_reg_context.r2,
                r3: cpu_reg_context.r3,
                r12: cpu_reg_context.r12,
                lr: cpu_reg_context.lr,
                return_address: cpu_reg_context.pc,
                xpsr_pre_exception: 0,
                hfsr: 0,
                cfsr: register_block.cfsr,
                mmfar: register_block.mmfar,
                bfar: register_block.bfar,
                afsr: 0,
            },
        };

        self.base[0..size_of::<CrashDumpBlock>()].copy_from_slice(block.as_bytes());
        if !additional_info.is_empty() {
            self.base
                [size_of::<CrashDumpBlock>()..size_of::<CrashDumpBlock>() + additional_info.len()]
                .copy_from_slice(additional_info.as_bytes());
        }

        self.commit_crash_dump();
        block.header.magic = DUMP_HEADER_MAGIC_COMITTED;
        trace!("CrashDump Collected : {:?}", block);
    }
}

struct Hex(u32);
impl fmt::Debug for Hex {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "0x{:08x}", self.0)
    }
}

impl fmt::Debug for CrashDumpHeader {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("CrashDumpHeader")
            .field("magic", &Hex(self.magic))
            .field("failure_code", &Hex(self.failure_code))
            .field("crashdump_version", &Hex(self.crashdump_version.into()))
            .field("core_type", &Hex(self.core_type.into()))
            .field("dump_type", &Hex(self.dump_type.into()))
            .field("crash_type", &Hex(self.crash_type.into()))
            .field("_rsvd", &Hex(self._rsvd.into()))
            .field("payload_size", &Hex(self.payload_size.into()))
            .finish()
    }
}

impl fmt::Debug for CrashDumpBody {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_struct("CrashDumpBody")
            .field("stack_ptr", &Hex(self.stack_ptr))
            .field("xpsr", &Hex(self.xpsr))
            .field("lr", &Hex(self.lr))
            .field("r0", &Hex(self.r0))
            .field("r1", &Hex(self.r1))
            .field("r2", &Hex(self.r2))
            .field("r3", &Hex(self.r3))
            .field("r12", &Hex(self.r12))
            .field("lr", &Hex(self.lr))
            .field("return_address", &Hex(self.return_address))
            .field("xpsr_pre_exception", &Hex(self.xpsr_pre_exception))
            .field("hfsr", &Hex(self.hfsr))
            .field("cfsr", &Hex(self.cfsr))
            .field("mmfar", &Hex(self.mmfar))
            .field("bfar", &Hex(self.bfar))
            .field("afsr", &Hex(self.afsr))
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_crashdump_size() {
        let mut crashdump_memory = [0u8; 0x1000];
        let mut mgr: CrashDumpManager = CrashDumpManager::new(&mut crashdump_memory);
        mgr.create_dump(
            1,
            &CpuRegisterContext {
                lr: 0x12345678,
                sp: 0x11223344,
                pc: 0x87654321,
                r0: 0x55667788,
                r1: 0x66778899,
                r2: 0x77889900,
                r3: 0x88990011,
                r12: 0x99001122,
            },
            &RegisterBlockContext::new(0x12345678, 0x87654321, 0x11223344),
            0,
            "",
        );

        let crashdump = mgr.get_crashdump();
        assert!(crashdump.header.payload_size > 0);
        assert!(crashdump.header.payload_size <= size_of::<CrashDumpBlock>() as u16);
        assert_eq!(crashdump.header.failure_code, 1);
    }

    #[test]
    fn test_crashdump_values() {
        let mut crashdump_memory = [0u8; 0x1000];
        let mut mgr: CrashDumpManager = CrashDumpManager::new(&mut crashdump_memory);
        mgr.create_dump(
            1,
            &CpuRegisterContext {
                lr: 0x12345678,
                sp: 0x11223344,
                pc: 0x87654321,
                r0: 0x55667788,
                r1: 0x66778899,
                r2: 0x77889900,
                r3: 0x88990011,
                r12: 0x99001122,
            },
            &RegisterBlockContext::new(0x12345678, 0x87654321, 0x11223344),
            0,
            "",
        );
        let crashdump = mgr.get_crashdump();
        assert_eq!(crashdump.header.magic, DUMP_HEADER_MAGIC_COMITTED);
        assert_eq!(crashdump.header.crashdump_version, DUMP_HEADER_VERSION);
        assert_eq!(crashdump.body.lr, 0x12345678);
        assert_eq!(crashdump.body.stack_ptr, 0x11223344);
        assert_eq!(crashdump.body.return_address, 0x87654321);
        assert_eq!(crashdump.body.r0, 0x55667788);
        assert_eq!(crashdump.body.r1, 0x66778899);
        assert_eq!(crashdump.body.r2, 0x77889900);
        assert_eq!(crashdump.body.r3, 0x88990011);
        assert_eq!(crashdump.body.r12, 0x99001122);
        assert_eq!(crashdump.body.cfsr, 0x12345678);
        assert_eq!(crashdump.body.mmfar, 0x87654321);
        assert_eq!(crashdump.body.bfar, 0x11223344);
    }

    #[test]
    fn test_crashdump_additional_info() {
        let mut crashdump_memory = [0u8; 0x1000];
        let mut mgr: CrashDumpManager = CrashDumpManager::new(&mut crashdump_memory);
        let additional_info = "Crash and Dump";
        mgr.create_dump(
            1,
            &CpuRegisterContext {
                lr: 0x12345678,
                sp: 0x11223344,
                pc: 0x87654321,
                r0: 0x55667788,
                r1: 0x66778899,
                r2: 0x77889900,
                r3: 0x88990011,
                r12: 0x99001122,
            },
            &RegisterBlockContext::new(0x12345678, 0x87654321, 0x11223344),
            0,
            additional_info,
        );

        let info = &mgr.base
            [size_of::<CrashDumpBlock>()..size_of::<CrashDumpBlock>() + additional_info.len()];
        assert!(info == additional_info.as_bytes());
    }
}
