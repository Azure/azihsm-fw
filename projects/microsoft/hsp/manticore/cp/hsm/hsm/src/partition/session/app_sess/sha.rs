// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_sha::*;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_types::IoMemRange;

use crate::error::HsmErr;
use crate::error::HsmResult;
use crate::partition::UserSession;
use crate::HsmEnvTrait;

#[derive(Copy, Clone)]
pub(crate) enum ShaType {
    /// SHA_1
    Sha1 = 20,

    /// SHA_256
    Sha256 = 32,

    /// SHA_384
    Sha384 = 48,

    /// SHA_512
    Sha512 = 64,
}

/// Returns the size of the block size for the given SHA type.
impl From<ShaType> for usize {
    /// Converts to this type from the input type.
    fn from(mode: ShaType) -> Self {
        match mode {
            ShaType::Sha1 => 64,
            ShaType::Sha256 => 64,
            ShaType::Sha384 => 128,
            ShaType::Sha512 => 128,
        }
    }
}

impl ShaType {
    /// This returns the digest size as required by the SHA HW.
    pub fn get_digest_size_hw(&self) -> usize {
        match self {
            ShaType::Sha1 => 20,
            ShaType::Sha256 => 32,
            // HS SHA requires the initial digest for SHA-384 to be 64 bytes.
            ShaType::Sha384 => 64,
            ShaType::Sha512 => 64,
        }
    }

    /// This returns the digest size as per the SHA algorithm.
    #[cfg(feature = "fips_validation_hooks")]
    pub fn get_digest_size(&self) -> usize {
        match self {
            ShaType::Sha1 => 20,
            ShaType::Sha256 => 32,
            ShaType::Sha384 => 48,
            ShaType::Sha512 => 64,
        }
    }
}

impl From<ShaType> for ShaMode {
    /// Converts to this type from the input type.
    fn from(value: ShaType) -> Self {
        match value {
            ShaType::Sha1 => ShaMode::Sha1,
            ShaType::Sha256 => ShaMode::Sha256,
            ShaType::Sha384 => ShaMode::Sha384,
            ShaType::Sha512 => ShaMode::Sha512,
        }
    }
}

impl From<ShaMode> for ShaType {
    /// Converts to this type from the input type.
    fn from(value: ShaMode) -> Self {
        match value {
            ShaMode::Sha1 => ShaType::Sha1,
            ShaMode::Sha256 => ShaType::Sha256,
            ShaMode::Sha384 => ShaType::Sha384,
            ShaMode::Sha512 => ShaType::Sha512,
        }
    }
}

impl TryFrom<DdiHashAlgorithm> for ShaType {
    type Error = HsmErr;

    fn try_from(value: DdiHashAlgorithm) -> Result<Self, Self::Error> {
        match value {
            DdiHashAlgorithm::Sha1 => Ok(ShaType::Sha1),
            DdiHashAlgorithm::Sha256 => Ok(ShaType::Sha256),
            DdiHashAlgorithm::Sha384 => Ok(ShaType::Sha384),
            DdiHashAlgorithm::Sha512 => Ok(ShaType::Sha512),
            _ => Err(HsmErr::InvalidArgument),
        }
    }
}

impl<E: HsmEnvTrait> UserSession<E> {
    #[allow(unused)]
    pub fn sha_single_block_inner_zc(
        &self,
        mode: ShaType,
        buffer: &IoMemRange,
        output_buffer: &mut IoMemRange,
    ) -> HsmResult<()> {
        // Prepare the command packet.
        let cmd_info = ShaDigestCmdInfoZc {
            buffer,
            init_digest: None,
            mode: mode.into(),
            last: true,
            len: buffer.len() as u32,
            total_len: buffer.len() as u32,
            output_buffer,
        };

        self.execute_sha_op_zc(&cmd_info)
    }

    fn execute_sha_op_zc(&self, cmd_info: &ShaDigestCmdInfoZc) -> HsmResult<()> {
        // Compute the SHA digest
        self.state
            .env()
            .sha()
            .digest_zc(cmd_info)
            .map_err(|_| HsmErr::ShaCmdFailed)
    }
}
