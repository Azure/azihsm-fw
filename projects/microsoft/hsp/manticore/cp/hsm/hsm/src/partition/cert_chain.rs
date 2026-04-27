// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::ShaMode;
use mcr_ipc_controller::*;
use mcr_ipc_message::GetCertChainLengthsPayload;
use mcr_ipc_message::GetCertPayload;
use mcr_ipc_message::IpcMessageGetCert;
use mcr_ipc_message::IpcMessageGetCertChainLengths;
use mcr_ipc_message::*;

use super::*;
use crate::error::HsmErr;

const ALIAS_AND_PART_CERT_CNT: usize = 2;
// Maximum number of certificates in the chain including Alias and Partition certs
pub(crate) const MAX_CERTS: usize = MAX_DEVICE_ID_CERTS + ALIAS_AND_PART_CERT_CNT;

#[derive(Copy, Clone, PartialEq)]
/// Certificate Chain Lengths Information
pub(crate) struct GetCertChainLengthsInfo {
    /// Hash of cert chain
    pub(crate) hash: [u8; 32],

    /// Number of certs in the chain
    pub(crate) num_certs: u8,

    /// Cert ids and lengths
    pub(crate) cert_lengths: [u16; MAX_CERTS],
}

/// Context for certificate chain lengths
pub(crate) struct GetCertLengthsContext<E: HsmEnvTrait + 'static> {
    /// Certificate chain lengths information
    pub(crate) cert_info: Option<GetCertChainLengthsInfo>,

    /// HSP IPC channel reference
    pub(crate) channel_ref: Option<HspIpcChannelRef<E>>,
}

/// Context for certificate signing operation
pub(crate) struct CertSignContext<E: HsmEnvTrait + 'static> {
    /// Certificate TBS (To Be Signed) data
    pub(crate) tbs: AzihsmLeafCertTbs,

    /// TBS digest buffer
    pub(crate) _tbs_digest_buf: DmaBuffer<E>,

    /// Signature buffer
    pub(crate) signature_buf: DmaBuffer<E>,

    /// UPKA Engine reference
    pub(crate) engine_ref: PkaEngineRef<E>,
}

/// Context for getting a certificate from the chain
pub(crate) struct GetCertContext<E: HsmEnvTrait + 'static> {
    /// Cert ID
    pub(crate) cert_id: u8,

    /// Cert length
    pub(crate) cert_len: Option<u16>,

    /// Cert buffer
    pub(crate) cert_buf: Option<IoMemRange>,

    /// HSP IPC channel reference
    pub(crate) channel_ref: Option<HspIpcChannelRef<E>>,
}

impl<E: HsmEnvTrait> Partition<E> {
    /// Send IPC request to get the lengths of individual certificates in the chain.
    pub(crate) fn begin_get_dev_id_cert_chain_inner(
        &self,
        tag: TagId,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<E>,
    ) -> HsmResult<()> {
        let channel_ref = self
            .state
            .env()
            .hsp_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        if let Some(cert_chain_info_from_part) = self.state.get_cert_chain_lengths_info() {
            get_cert_chain_lengths_ctx.cert_info = Some(cert_chain_info_from_part);

            return Ok(());
        }

        let msg = IpcMessageGetCertChainLengths {
            info: GetCertChainLengthsPayload {
                hash: Default::default(),
                num_certs: Default::default(),
                cert_lengths: Default::default(),
            },
            ..Default::default()
        };

        channel_ref
            .map(|c| c.send_request(tag, msg.encode()))
            .map_err(|_err| HsmErr::IpcSendFailure)?;

        get_cert_chain_lengths_ctx.channel_ref = Some(channel_ref);

        Ok(())
    }

    /// IPC response to get the lengths of individual certificates in the chain.
    pub(crate) fn end_get_dev_id_cert_chain_info_inner(
        &self,
        get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<E>,
    ) -> HsmResult<()> {
        let channel_ref = get_cert_chain_lengths_ctx
            .channel_ref
            .as_ref()
            .ok_or(HsmErr::IpcResponseError)?;

        let msg = channel_ref
            .map(|c| c.receive_message())
            .ok_or(HsmErr::IpcResponseError)?;

        let response_message = IpcMessageDecoder::decode::<IpcMessageGetCertChainLengths>(msg)
            .map_err(|_| HsmErr::IpcResponseError)?;

        let header = response_message.header;
        if header.status() != 0 {
            Err(HsmErr::IpcResponseError)?
        }

        let cert_info = response_message.info;
        if cert_info.num_certs > MAX_DEVICE_ID_CERTS as u8 {
            Err(HsmErr::IpcResponseError)?
        }

        let mut cert_lengths = [0u16; MAX_CERTS];
        let cert_lengths_from_ipc = cert_info.cert_lengths;
        cert_lengths[..(cert_info.num_certs as usize)]
            .copy_from_slice(&cert_lengths_from_ipc[..(cert_info.num_certs as usize)]);

        // Account for Alias and PID certs
        let num_certs_with_alias_and_pid = cert_info.num_certs + ALIAS_AND_PART_CERT_CNT as u8;

        get_cert_chain_lengths_ctx.cert_info = Some(GetCertChainLengthsInfo {
            hash: cert_info.hash,
            num_certs: num_certs_with_alias_and_pid,
            cert_lengths,
        });

        Ok(())
    }

    /// Update Cert Chain Lengths Info
    pub(crate) fn update_cert_chain_lengths_info_inner(
        &self,
        cert_info: &mut GetCertChainLengthsInfo,
    ) -> HsmResult<()> {
        // Update the cert lengths array with the Alias and PID cert lengths
        cert_info.cert_lengths[cert_info.num_certs as usize - ALIAS_AND_PART_CERT_CNT] =
            self.get_alias_cert_len() as u16;

        cert_info.cert_lengths[cert_info.num_certs as usize - 1] =
            self.partition_cert_length() as u16;

        // Update the hash of the cert chain to include the Alias and PID certs
        let sha256_digest_len = ShaMode::Sha256.get_digest_size_hw();
        let working_buf_len = sha256_digest_len * 4;
        let mut working_buf = self.dma_alloc(working_buf_len)?;

        let mut input_buf_len = 0;

        // Copy dev id cert chain hash into the input buffer; working_buf[0..32]
        working_buf.as_ref_mut()[input_buf_len..input_buf_len + sha256_digest_len]
            .copy_from_slice(&cert_info.hash);

        // Increment input_buf_len to be 32; this is the start of the alias cert hash.
        input_buf_len += sha256_digest_len;

        // Get the IoMemRange for the alias cert hash output buffer
        let mut alias_cert_hash_buf = IoMemRange::from(
            &working_buf.as_ref()[input_buf_len..input_buf_len + sha256_digest_len],
        );

        // Get hash of alias cert; working_buf[32..64]
        let alias_cert = self.get_alias_cert();
        self.sha_single_block_zc_internal(ShaMode::Sha256, &alias_cert, &mut alias_cert_hash_buf)?;

        // Increment input_buf_len to be 64; this is the start of the PID cert hash.
        input_buf_len += sha256_digest_len;

        // Get the IoMemRange for the PID cert hash output buffer
        let mut pid_cert_hash_buf = IoMemRange::from(
            &working_buf.as_ref()[input_buf_len..input_buf_len + sha256_digest_len],
        );

        // Increment input_buf_len to be 96; this is the end of the input buffer.
        input_buf_len += sha256_digest_len;

        // Get hash of PID cert; working_buf[64..96]
        self.sha_single_block_zc_internal(
            ShaMode::Sha256,
            &self.state.get_partition_cert(),
            &mut pid_cert_hash_buf,
        )?;

        // Get the thumbprint of the entire cert chain
        // Thumbprint is the SHA-256 Hash(Hash(dev_id_cert_chain) || Hash(alias_cert) || Hash(pid_cert))

        // input_buf = working_buf[0..96] with contains
        // dev id cert chain hash, alias cert hash, and pid cert hash
        let input_buf = IoMemRange::from(&working_buf.as_ref()[..input_buf_len]);

        // working_buf[96..128] is used as the output buffer for the thumbprint
        let mut thumbprint_buf =
            IoMemRange::from(&working_buf.as_ref()[input_buf_len..working_buf_len]);
        self.sha_single_block_zc_internal(ShaMode::Sha256, &input_buf, &mut thumbprint_buf)?;

        // Update the cert_info with the thumbprint
        cert_info
            .hash
            .copy_from_slice(&thumbprint_buf.slice()[..sha256_digest_len]);

        Ok(())
    }

    /// Send IPC request to get the certificate.
    pub(crate) fn begin_get_cert_inner(
        &self,
        tag: TagId,
        get_cert_ctx: &mut GetCertContext<E>,
    ) -> HsmResult<()> {
        let channel_ref = self
            .state
            .env()
            .hsp_ipc_channel()
            .acquire(tag, ())
            .ok_or(HsmErr::Pending)?;

        let cert_chain_info_from_part = self
            .state
            .get_cert_chain_lengths_info()
            .ok_or(HsmErr::InvalidCertificate)?;

        if cert_chain_info_from_part.num_certs == 0
            || get_cert_ctx.cert_id >= cert_chain_info_from_part.num_certs
        {
            return Err(HsmErr::InvalidCertificate);
        }

        if get_cert_ctx.cert_id == cert_chain_info_from_part.num_certs - 1 {
            // PID Cert is requested
            get_cert_ctx
                .cert_buf
                .ok_or(HsmErr::InvalidCertificate)?
                .slice_mut()
                .copy_from_slice(self.state.get_partition_cert().slice());
        } else if get_cert_ctx.cert_id == cert_chain_info_from_part.num_certs - 2 {
            // Alias Cert is requested
            get_cert_ctx
                .cert_buf
                .ok_or(HsmErr::InvalidCertificate)?
                .slice_mut()
                .copy_from_slice(self.get_alias_cert().slice());
        } else {
            let addr = get_cert_ctx
                .cert_buf
                .ok_or(HsmErr::InvalidCertificate)?
                .addr() as u64;
            let buf_size = get_cert_ctx.cert_len.ok_or(HsmErr::InvalidCertificate)?;

            let msg = IpcMessageGetCert {
                info: GetCertPayload {
                    cert_id: get_cert_ctx.cert_id,
                    cert_len: 0,
                    addr,
                    buf_size,
                },
                ..Default::default()
            };
            channel_ref
                .map(|c| c.send_request(tag, msg.encode()))
                .map_err(|_err| HsmErr::IpcSendFailure)?;

            get_cert_ctx.channel_ref = Some(channel_ref);
        }

        Ok(())
    }

    /// IPC response to get the certificate.
    pub(crate) fn end_get_cert_inner(&self, get_cert_ctx: &mut GetCertContext<E>) -> HsmResult<()> {
        let channel_ref = get_cert_ctx
            .channel_ref
            .as_ref()
            .ok_or(HsmErr::IpcResponseError)?;

        let msg = channel_ref
            .map(|c| c.receive_message())
            .ok_or(HsmErr::IpcResponseError)?;

        let response_message = IpcMessageDecoder::decode::<IpcMessageGetCert>(msg)
            .map_err(|_| HsmErr::IpcResponseError)?;

        let header = response_message.header;
        if header.status() != 0 {
            Err(HsmErr::IpcResponseError)?
        }

        let cert = response_message.info;
        let initial_cert_len = get_cert_ctx.cert_len.ok_or(HsmErr::InvalidCertificate)?;
        let initial_cert = get_cert_ctx.cert_buf.ok_or(HsmErr::InvalidCertificate)?;

        if cert.cert_id != get_cert_ctx.cert_id
            || cert.cert_len != initial_cert_len
            || cert.addr != initial_cert.addr() as u64
        {
            Err(HsmErr::IpcResponseError)?
        }

        Ok(())
    }
}
