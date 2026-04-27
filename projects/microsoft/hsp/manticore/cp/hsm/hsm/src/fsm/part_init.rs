// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;
use mcr_ipc_controller::IpcMessageChannelTrait;
use mcr_ipc_message::IpcMessageEncoderTrait;
use mcr_ipc_message::IpcMessageSetRes;
use mcr_ipc_message::IpcMessageStatusCode;
use mcr_ipc_message::SetResInfo;

use mcr_crypto_pka::PkaEccPublicKey;

use super::*;

/// Partition Initialization FSM States
#[derive(Clone, Copy, PartialEq, Eq)]
pub(crate) enum HsmPartInitFsmState {
    /// Initial state of the partition initialization FSM
    Init,

    /// Waiting for a resource to be ready for PIDK generation
    WaitingForResourceForPIDK,

    /// Generating partition key
    GeneratingPartitionKey,

    /// Waiting for a resource to be ready for PCT validation
    WaitingForResourceForPct,

    /// Continue PCT operation
    ContinuePctOperation,
}

/// Partition Init FSM context.
pub(crate) struct HsmPartInitFsm<E: HsmEnvTrait + 'static> {
    /// Environment for the HSM operations
    env: Rc<RefCell<E>>,

    /// Partition being initialized
    part: E::Partition,

    /// Current state of the FSM
    state: HsmPartInitFsmState,

    /// Information from the IPC message
    ipc_message_info: Option<SetResInfo>,

    /// Context for the partition identifier generation
    ctx: Option<GetPartitionIdCtx>,

    /// ephemeral key material
    staged_km: Option<PartitionIdGenResult>,

    /// PCT Validation Operation (Sign, Verify, ECDH)
    pct_op: Option<EccKeyPct<<<E as env::HsmEnvTrait>::Partition as partition::HsmPartition>::Env>>,
}

impl<E: HsmEnvTrait + 'static> CmdFsm for HsmPartInitFsm<E> {
    type Error = HsmErr;
    type ResourceId = HsmFsmResourceId;
    type Event = HsmFsmEvent;
    type Recorder = HsmFsmEventRecorder;

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (HsmPartInitFsmState::Init, HsmFsmEvent::InitPartition(message)) => {
                self.start(tag, message)
            }
            (
                HsmPartInitFsmState::WaitingForResourceForPIDK,
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            ) => self.begin_generate_partition_identifiers(tag),

            (HsmPartInitFsmState::GeneratingPartitionKey, HsmFsmEvent::PkaDone(_)) => {
                self.continue_get_partition_identifiers(tag)
            }
            (
                HsmPartInitFsmState::WaitingForResourceForPct,
                HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            ) => self.handle_begin_pct_validation(tag),
            (HsmPartInitFsmState::ContinuePctOperation, HsmFsmEvent::PkaDone(_)) => {
                self.handle_continue_pct_validation(tag)
            }
            (_, HsmFsmEvent::CheckAlive) => Err(HsmErr::Pending),
            (_, _) => {
                error!("[part_init] Unexpected event {} ", u32::from(event));

                Err(HsmErr::InvalidEvent)
            }
        }
    }

    fn acquire_resource(&mut self, _tag: TagId, res_id: ResId) -> HsmFsmEvent {
        match res_id {
            HsmFsmResourceId::Pka => HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            _ => HsmFsmEvent::Unknown,
        }
    }
}

impl<E: HsmEnvTrait + 'static> HsmPartInitFsm<E> {
    /// Create a new instance of the partition initialization FSM
    pub fn new(env: Rc<RefCell<E>>, part: E::Partition) -> Self {
        Self {
            env,
            part,
            state: HsmPartInitFsmState::Init,
            ctx: None,
            ipc_message_info: None,
            staged_km: None,
            pct_op: None,
        }
    }

    /// Start the partition initialization process
    fn start(&mut self, tag: TagId, message: IpcMessageSetRes) -> Result<(), HsmErr> {
        // First handle set resource count. If we fail, this function sends the response and returns an error.
        self.handle_set_res_cnt(message)?;

        self.begin_generate_partition_identifiers(tag)
    }

    /// Handle the Set Resource Count message.
    fn handle_set_res_cnt(&mut self, message: IpcMessageSetRes) -> Result<(), HsmErr> {
        let mask = u128::from_le_bytes(message.info.mask);
        self.part.set_resource_mask(mask);

        let guid = message.info.vm_launch_guid;
        self.part.set_vm_launch_guid(&guid);

        self.ipc_message_info = Some(message.info);

        Ok(())
    }

    /// Begin the partition identifier generation process.
    fn begin_generate_partition_identifiers(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let ctx = self.part.begin_generate_partition_identifiers(tag);
        match ctx {
            Err(HsmErr::Pending) => {
                if self.state == HsmPartInitFsmState::WaitingForResourceForPIDK {
                    // We don't expect to wait for the PKA engine here, we should have already acquired it.
                    error!("[part_init] Gen ID phase - expect to find UPKA engine on res ready");
                    self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                    return Err(HsmErr::InvalidState);
                }

                self.state = HsmPartInitFsmState::WaitingForResourceForPIDK;

                Err(HsmErr::Pending)
            }
            Ok(ctx) => {
                if ctx.identifiers_present {
                    self.prepare_and_send_response(IpcMessageStatusCode::Success);

                    Ok(())
                } else {
                    self.state = HsmPartInitFsmState::GeneratingPartitionKey;
                    self.ctx = Some(ctx);

                    Err(HsmErr::Pending)
                }
            }

            Err(err) => {
                error!("[part_init] Failed begin part id generation {}", err as u32);
                self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                Err(HsmErr::PartitionIdGenerationFailed)
            }
        }
    }

    /// Continue the partition identifier generation process.
    fn continue_get_partition_identifiers(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let Some(ctx) = self.ctx.take() else {
            error!("[part_init] No context available for ending partition identifier generation.");
            self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

            return Err(HsmErr::InvalidState);
        };

        match self.part.continue_generate_partition_identifiers(tag, ctx) {
            Ok(km) => {
                self.staged_km = Some(km);
                self.handle_begin_pct_validation(tag)
            }
            Err(err) => {
                error!(
                    "[part_init] Failed Continue part id generation: {}",
                    err as u32
                );
                self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                Err(err)
            }
        }
    }

    /// Handle the beginning of PCT validation.
    fn handle_begin_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let km = self.staged_km.as_ref().ok_or(HsmErr::InvalidState)?;

        // Build PKA public key from native XY (no reversals here)
        let pub_key = PkaEccPublicKey::from_bytes(km.curve, &km.pub_xy)
            .map_err(|_| HsmErr::InvalidArgument)?;

        match self.part.begin_ecc_pct_validation_raw(
            tag,
            EccKeyUsage::SignVerify,
            &pub_key,
            &km.priv_d,
        ) {
            Ok(pct) => {
                self.pct_op = Some(pct);
                self.state = HsmPartInitFsmState::ContinuePctOperation;

                Err(HsmErr::Pending)
            }
            Err(HsmErr::Pending) => {
                self.state = HsmPartInitFsmState::WaitingForResourceForPct;

                Err(HsmErr::Pending)
            }
            Err(e) => {
                // Drop the staged key material now. its Drop impl will zeroize.
                let _ = self.staged_km.take();
                error!("[part_init] Failed begin PCT validation: {}", e as u32);
                self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                Err(e)
            }
        }
    }

    /// Handle continuation of PCT validation.
    fn handle_continue_pct_validation(&mut self, tag: TagId) -> Result<(), HsmErr> {
        let mut pct_op = self.pct_op.take().ok_or(HsmErr::InvalidState)?;

        if self.part.is_pct_final_state(&pct_op) {
            match self.part.end_ecc_pct_validation(tag, &mut pct_op) {
                Ok(true) => {
                    // PCT passed — commit staged keys (will be zeroized on drop)
                    if let Some(km) = self.staged_km.take() {
                        self.part.end_generate_partition_identifiers(km)?;
                    } else {
                        self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                        return Err(HsmErr::InvalidState);
                    }
                    self.prepare_and_send_response(IpcMessageStatusCode::Success);

                    Ok(())
                }
                Ok(false) => {
                    error!("[part_init] Failed PCT final validation");
                    self.part.notify_pct_validation_failure(
                        HsmErr::PartitionIdKeyGenerationPctFailed as u32,
                    );
                    let _ = self.staged_km.take();
                    self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                    Err(HsmErr::PartitionIdKeyGenerationPctFailed)
                }
                Err(err) => {
                    error!("[part_init] Failed end PCT validation: {}", err as u32);
                    let _ = self.staged_km.take();
                    self.prepare_and_send_response(IpcMessageStatusCode::OperationFailed);

                    Err(err)
                }
            }
        } else {
            match self.part.continue_ecc_pct_validation(tag, &mut pct_op) {
                Ok(()) => {
                    self.pct_op = Some(pct_op);
                    self.state = HsmPartInitFsmState::ContinuePctOperation;

                    Err(HsmErr::Pending)
                }
                Err(err) => {
                    error!("[part_init] Failed continue PCT validation: {}", err as u32);
                    self.pct_op = Some(pct_op);

                    Err(err)
                }
            }
        }
    }

    /// Prepare and send the response to the Admin core.
    fn prepare_and_send_response(&self, status: IpcMessageStatusCode) {
        let mut response = IpcMessageSetRes {
            ..Default::default()
        };

        response.header.set_response(true);
        response.header.set_status(status.into());

        if let Some(ref info) = self.ipc_message_info {
            response.info = *info;
        } else {
            response.info = SetResInfo::default();
        }

        let _ = self
            .env
            .borrow()
            .hal()
            .admin_ipc_channel()
            .send_response(response.encode());
    }
}
