#[doc = "++

Copyright (c) Microsoft Corporation. All rights reserved.

Abstract:

    Regenerate the template by building hsm-src-x509-build with the generate-templates flag.

--"]
use mcr_types::SecureByteArray;

pub struct AzihsmLeafCertTbsParams<'a> {
    pub public_key: &'a [u8; 97usize],
    pub issuer_sn: &'a [u8; 64usize],
    pub subject_sn: &'a [u8; 32usize],
    pub serial_number: &'a [u8; 20usize],
    pub subject_key_id: &'a [u8; 20usize],
    pub authority_key_id: &'a [u8; 20usize],
    pub not_before: &'a [u8; 15usize],
    pub not_after: &'a [u8; 15usize],
}
impl AzihsmLeafCertTbsParams<'_> {
    pub const PUBLIC_KEY_LEN: usize = 97usize;
    pub const ISSUER_SN_LEN: usize = 64usize;
    pub const SUBJECT_SN_LEN: usize = 32usize;
    pub const SERIAL_NUMBER_LEN: usize = 20usize;
    pub const SUBJECT_KEY_ID_LEN: usize = 20usize;
    pub const AUTHORITY_KEY_ID_LEN: usize = 20usize;
    pub const NOT_BEFORE_LEN: usize = 15usize;
    pub const NOT_AFTER_LEN: usize = 15usize;
}
pub struct AzihsmLeafCertTbs {
    tbs: SecureByteArray<{ Self::TBS_TEMPLATE_LEN }>,
}

impl Default for AzihsmLeafCertTbs {
    fn default() -> Self {
        let params = AzihsmLeafCertTbsParams {
            public_key: &[0u8; AzihsmLeafCertTbsParams::PUBLIC_KEY_LEN],
            subject_sn: &[0u8; AzihsmLeafCertTbsParams::SUBJECT_SN_LEN],
            issuer_sn: &[0u8; AzihsmLeafCertTbsParams::ISSUER_SN_LEN],
            serial_number: &[0u8; AzihsmLeafCertTbsParams::SERIAL_NUMBER_LEN],
            subject_key_id: &[0u8; AzihsmLeafCertTbsParams::SUBJECT_KEY_ID_LEN],
            authority_key_id: &[0u8; AzihsmLeafCertTbsParams::AUTHORITY_KEY_ID_LEN],
            not_before: &[0u8; AzihsmLeafCertTbsParams::NOT_BEFORE_LEN],
            not_after: &[0u8; AzihsmLeafCertTbsParams::NOT_AFTER_LEN],
        };
        Self::new(&params)
    }
}

impl AzihsmLeafCertTbs {
    const PUBLIC_KEY_OFFSET: usize = 224usize;
    const ISSUER_SN_OFFSET: usize = 56usize;
    const SUBJECT_SN_OFFSET: usize = 169usize;
    const SERIAL_NUMBER_OFFSET: usize = 11usize;
    const SUBJECT_KEY_ID_OFFSET: usize = 369usize;
    const AUTHORITY_KEY_ID_OFFSET: usize = 402usize;
    const NOT_BEFORE_OFFSET: usize = 124usize;
    const NOT_AFTER_OFFSET: usize = 141usize;
    const PUBLIC_KEY_LEN: usize = 97usize;
    const ISSUER_SN_LEN: usize = 64usize;
    const SUBJECT_SN_LEN: usize = 32usize;
    const SERIAL_NUMBER_LEN: usize = 20usize;
    const SUBJECT_KEY_ID_LEN: usize = 20usize;
    const AUTHORITY_KEY_ID_LEN: usize = 20usize;
    const NOT_BEFORE_LEN: usize = 15usize;
    const NOT_AFTER_LEN: usize = 15usize;
    pub const TBS_TEMPLATE_LEN: usize = 422usize;
    const TBS_TEMPLATE: [u8; Self::TBS_TEMPLATE_LEN] = [
        48u8, 130u8, 1u8, 162u8, 160u8, 3u8, 2u8, 1u8, 2u8, 2u8, 20u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 48u8, 10u8, 6u8, 8u8, 42u8, 134u8, 72u8, 206u8, 61u8, 4u8, 3u8, 3u8, 48u8, 75u8,
        49u8, 73u8, 48u8, 71u8, 6u8, 3u8, 85u8, 4u8, 3u8, 12u8, 64u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 48u8,
        34u8, 24u8, 15u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 24u8, 15u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 48u8, 43u8, 49u8, 41u8, 48u8, 39u8, 6u8, 3u8, 85u8, 4u8, 3u8,
        12u8, 32u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 48u8, 118u8, 48u8, 16u8, 6u8, 7u8, 42u8, 134u8, 72u8, 206u8, 61u8,
        2u8, 1u8, 6u8, 5u8, 43u8, 129u8, 4u8, 0u8, 34u8, 3u8, 98u8, 0u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 163u8, 99u8, 48u8, 97u8, 48u8, 15u8, 6u8, 3u8, 85u8, 29u8, 19u8, 1u8,
        1u8, 255u8, 4u8, 5u8, 48u8, 3u8, 2u8, 1u8, 0u8, 48u8, 14u8, 6u8, 3u8, 85u8, 29u8, 15u8,
        1u8, 1u8, 255u8, 4u8, 4u8, 3u8, 2u8, 7u8, 128u8, 48u8, 29u8, 6u8, 3u8, 85u8, 29u8, 14u8,
        4u8, 22u8, 4u8, 20u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 48u8, 31u8, 6u8, 3u8, 85u8, 29u8,
        35u8, 4u8, 24u8, 48u8, 22u8, 128u8, 20u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
    ];
    pub fn new(params: &AzihsmLeafCertTbsParams) -> Self {
        let mut template = Self {
            tbs: Self::TBS_TEMPLATE.into(),
        };
        template.apply(params);
        template
    }
    pub fn sign<Sig, Error>(
        &self,
        sign_fn: impl Fn(&[u8]) -> Result<Sig, Error>,
    ) -> Result<Sig, Error> {
        sign_fn(self.tbs.as_slice())
    }
    pub fn tbs(&self) -> &[u8] {
        self.tbs.as_slice()
    }
    fn apply(&mut self, params: &AzihsmLeafCertTbsParams) {
        #[inline(always)]
        fn apply_slice<const OFFSET: usize, const LEN: usize>(
            buf: &mut [u8; 422usize],
            val: &[u8; LEN],
        ) {
            buf[OFFSET..OFFSET + LEN].copy_from_slice(val);
        }
        apply_slice::<{ Self::PUBLIC_KEY_OFFSET }, { Self::PUBLIC_KEY_LEN }>(
            &mut self.tbs,
            params.public_key,
        );
        apply_slice::<{ Self::ISSUER_SN_OFFSET }, { Self::ISSUER_SN_LEN }>(
            &mut self.tbs,
            params.issuer_sn,
        );
        apply_slice::<{ Self::SUBJECT_SN_OFFSET }, { Self::SUBJECT_SN_LEN }>(
            &mut self.tbs,
            params.subject_sn,
        );
        apply_slice::<{ Self::SERIAL_NUMBER_OFFSET }, { Self::SERIAL_NUMBER_LEN }>(
            &mut self.tbs,
            params.serial_number,
        );
        apply_slice::<{ Self::SUBJECT_KEY_ID_OFFSET }, { Self::SUBJECT_KEY_ID_LEN }>(
            &mut self.tbs,
            params.subject_key_id,
        );
        apply_slice::<{ Self::AUTHORITY_KEY_ID_OFFSET }, { Self::AUTHORITY_KEY_ID_LEN }>(
            &mut self.tbs,
            params.authority_key_id,
        );
        apply_slice::<{ Self::NOT_BEFORE_OFFSET }, { Self::NOT_BEFORE_LEN }>(
            &mut self.tbs,
            params.not_before,
        );
        apply_slice::<{ Self::NOT_AFTER_OFFSET }, { Self::NOT_AFTER_LEN }>(
            &mut self.tbs,
            params.not_after,
        );
    }
}
