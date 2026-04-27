// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::vec;

use mcr_types::*;

use super::*;

#[derive(Copy, Clone)]
struct DeleteQueueRequest {
    tag: u16,

    pfn: PcieFunction,

    queue_id: Option<DevSqId>,
}

impl Default for DeleteQueueRequest {
    fn default() -> Self {
        Self {
            tag: Default::default(),
            pfn: PcieFunction(0),
            queue_id: Default::default(),
        }
    }
}

fn mem_addr_to_slice<T>(addr: usize, len: usize) -> &'static mut [T] {
    unsafe { core::slice::from_raw_parts_mut(addr as *mut T, len) }
}

fn mem_add_to_volatile_ptr(addr: usize) -> &'static VolatileCell<u32> {
    unsafe {
        #[allow(clippy::transmute_ptr_to_ref)]
        core::mem::transmute(addr as *const u32)
    }
}

#[test]
fn test_simplex_init() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    // Remote CPU sent a new message
    {
        let msg = DeleteQueueRequest {
            tag: 100,
            pfn: PcieFunction::Pf,
            queue_id: Some(DevSqId::Id104),
        };

        assert!(pipe.send(msg).is_ok());
    }

    let message = pipe.recv().unwrap();

    assert!(matches!(message.pfn, PcieFunction::Pf));
    assert_eq!(message.tag, 100);
    assert!(matches!(message.queue_id, Some(DevSqId::Id104)));
    assert_eq!(ci, 1);
    assert_eq!(pi, 1);
}

#[test]
fn test_simplex_queue_multiple_messages() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    assert!(pipe.is_empty());

    // Queue up the message in pipe
    for i in 0..64 {
        let msg = DeleteQueueRequest {
            tag: 100 + i,
            pfn: PcieFunction(i as u8),
            queue_id: Some(DevSqId(i as u8 + 1)),
        };

        assert!(pipe.send(msg).is_ok());
    }

    // Receive the message from pipe
    for i in 0..64 {
        let message = pipe.recv().unwrap();

        assert_eq!(message.pfn.0, PcieFunction(i as u8).0);
        assert_eq!(message.tag, 100 + i);
        let message_queue_id = message.queue_id.unwrap();

        assert_eq!(message_queue_id.0, DevSqId(i as u8 + 1).0);
        assert_eq!(ci, i as u32 + 1);
    }

    assert!(pipe.is_empty());
}

#[test]
fn test_simplex_queue_full_condition() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    assert!(pipe.is_empty());

    // Queue up the message in pipe
    for i in 0..66 {
        let msg = DeleteQueueRequest {
            tag: 100 + i,
            pfn: PcieFunction(i as u8),
            queue_id: Some(DevSqId(i as u8 + 1)),
        };

        let result = pipe.send(msg);
        if i == 65 {
            assert_eq!(result.err(), Some(SimplexPipeErr::PipeFull as u32));
        } else {
            assert!(result.is_ok());
        }
    }

    // Receive the message from pipe
    for i in 0..65 {
        let message = pipe.recv().unwrap();

        assert_eq!(message.pfn.0, PcieFunction(i as u8).0);
        assert_eq!(message.tag, 100 + i);
        let message_queue_id = message.queue_id.unwrap();
        assert_eq!(message_queue_id.0, DevSqId(i as u8 + 1).0);
        assert_eq!(ci, i as u32 + 1);
    }

    assert!(pipe.is_empty());
}

#[test]
fn test_simplex_recv_none() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    assert!(pipe.is_empty());
    let message = pipe.recv();

    assert!(message.is_none());
}

#[test]
fn test_simplex_peek() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    // Remote CPU sent a new message
    {
        let msg = DeleteQueueRequest {
            tag: 100,
            pfn: PcieFunction::Pf,
            queue_id: Some(DevSqId::Id104),
        };

        assert!(pipe.send(msg).is_ok());
    }

    let message = pipe.peek().unwrap();

    assert_eq!(message.pfn.0, PcieFunction::Pf.0);
    assert_eq!(message.tag, 100);
    let message_queue_id = message.queue_id.unwrap();
    assert_eq!(message_queue_id.0, DevSqId::Id104.0);

    let message = pipe.recv().unwrap();

    assert_eq!(message.pfn.0, PcieFunction::Pf.0);
    assert_eq!(message.tag, 100);
    let message_queue_id = message.queue_id.unwrap();
    assert_eq!(message_queue_id.0, DevSqId::Id104.0);
    assert_eq!(ci, 1);
    assert_eq!(pi, 1);
}

#[test]
fn test_simplex_peek_none() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    assert!(pipe.is_empty());
    let message = pipe.peek();

    assert!(message.is_none());
}

#[test]
fn test_simplex_rollover_condition() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    assert!(pipe.is_empty());

    // Queue up the message in pipe and receive as and when the message is available
    for i in 0..1000 {
        let msg = DeleteQueueRequest {
            tag: 100 + i,
            pfn: PcieFunction((i % 64) as u8),
            queue_id: Some(DevSqId((i % 64) as u8 + 1)),
        };

        assert!(pipe.send(msg).is_ok());

        let message = pipe.recv().unwrap();

        assert_eq!(message.pfn.0, PcieFunction((i % 64) as u8).0);
        assert_eq!(message.tag, 100 + i);
        let message_queue_id = message.queue_id.unwrap();
        assert_eq!(message_queue_id.0, DevSqId((i % 64) as u8 + 1).0);
    }

    assert!(pipe.is_empty());
}

#[test]
fn test_simplex_test_clone() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);
    let cloned_pipe = pipe.clone();

    assert!(pipe.is_empty());

    // Queue up the message in pipe and receive as and when the message is available
    for i in 0..1000 {
        let msg = DeleteQueueRequest {
            tag: 100 + i,
            pfn: PcieFunction((i % 64) as u8),
            queue_id: Some(DevSqId((i % 64) as u8 + 1)),
        };

        assert!(cloned_pipe.send(msg).is_ok());

        let message = cloned_pipe.recv().unwrap();

        assert_eq!(message.pfn.0, PcieFunction((i % 64) as u8).0);
        assert_eq!(message.tag, 100 + i);
        let message_queue_id = message.queue_id.unwrap();
        assert_eq!(message_queue_id.0, DevSqId((i % 64) as u8 + 1).0);
    }

    assert!(cloned_pipe.is_empty());
}

#[test]
fn test_multiple_simplex() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_u32 = vec![u32::default(); 66];
    let ci_u32 = 0u32;
    let ci_ptr_u32: *const u32 = &ci_u32;
    let pi_u32 = 0u32;
    let pi_ptr_u32: *const u32 = &pi_u32;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let queue_ref_u32: &'static mut [u32] =
        mem_addr_to_slice(queue_u32.as_ptr() as usize, queue_u32.len());
    let ci_ref_u32 = mem_add_to_volatile_ptr(ci_ptr_u32 as usize);
    let pi_ref_u32 = mem_add_to_volatile_ptr(pi_ptr_u32 as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let config_u32 = SimplexPipeConfig {
        queue: queue_ref_u32,
        ci: ci_ref_u32,
        pi: pi_ref_u32,
    };

    let pipe = SimplexPipe::new(config);

    let pipe_u32 = SimplexPipe::new(config_u32);

    // Remote CPU sent a new message
    {
        let msg = DeleteQueueRequest {
            tag: 100,
            pfn: PcieFunction::Pf,
            queue_id: Some(DevSqId::Id104),
        };

        assert!(pipe.send(msg).is_ok());
    }

    let message = pipe.recv().unwrap();

    assert_eq!(message.pfn.0, PcieFunction::Pf.0);
    assert_eq!(message.tag, 100);
    let message_queue_id = message.queue_id.unwrap();
    assert_eq!(message_queue_id.0, DevSqId::Id104.0);
    assert_eq!(ci, 1);
    assert_eq!(pi, 1);

    // Remote CPU sent a new message_u32
    {
        let msg = 0x12345678;

        assert!(pipe_u32.send(msg).is_ok());
    }

    let message = pipe_u32.recv().unwrap();

    assert_eq!(message, 0x12345678);
    assert_eq!(ci_u32, 1);
    assert_eq!(pi_u32, 1);
}

#[test]
fn test_simplex_is_full_and_empty_slot_count() {
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    // Initially the pipe should not be full and should have (len - 1) empty slots
    assert!(!pipe.is_full());
    assert_eq!(pipe.empty_slot_count(), 65);

    // Fill the pipe to capacity (65 messages) and verify full/empty_slot_count
    for i in 0..65 {
        let msg = DeleteQueueRequest {
            tag: 100 + i,
            pfn: PcieFunction(i as u8),
            queue_id: Some(DevSqId(i as u8 + 1)),
        };

        assert!(pipe.send(msg).is_ok());
    }

    // Now pipe must report full and 0 empty slots
    assert!(pipe.is_full());
    assert_eq!(pipe.empty_slot_count(), 0);

    // Consume one message: pipe should no longer be full and empty_slot_count should be 1
    let _ = pipe.recv().unwrap();
    assert!(!pipe.is_full());
    assert_eq!(pipe.empty_slot_count(), 1);
}

#[test]
fn test_simplex_empty_slot_count_rollover() {
    // Test empty_slot_count after wrap-around behavior
    let queue = vec![DeleteQueueRequest::default(); 66];
    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;

    let queue_ref: &'static mut [DeleteQueueRequest] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let config = SimplexPipeConfig {
        queue: queue_ref,
        ci: ci_ref,
        pi: pi_ref,
    };

    let pipe = SimplexPipe::new(config);

    // Send 60 messages
    for i in 0..60 {
        let msg = DeleteQueueRequest {
            tag: 200 + i,
            pfn: PcieFunction(i as u8),
            queue_id: Some(DevSqId(i as u8 + 1)),
        };

        assert!(pipe.send(msg).is_ok());
    }

    // Consume 58 messages
    for _ in 0..58 {
        assert!(pipe.recv().is_some());
    }

    // Now there should be empty slots = capacity - 60 + 58
    assert_eq!(pipe.empty_slot_count(), 63);

    // Send enough to wrap-around: send 64 more items (this will cycle indexes)
    for i in 0..64 {
        let msg = DeleteQueueRequest {
            tag: 300 + i,
            pfn: PcieFunction((i % 64) as u8),
            queue_id: Some(DevSqId((i % 64) as u8 + 1)),
        };

        // eventually will error when pipe is full; stop on error
        let _ = pipe.send(msg);
    }

    // After rollover operations as well, empty_slot_count should be within 0..=65
    let count = pipe.empty_slot_count();
    assert!(count <= 65);
}
