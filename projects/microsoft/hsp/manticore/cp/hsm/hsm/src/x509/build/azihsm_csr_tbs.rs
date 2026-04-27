#[doc = "++

Copyright (c) Microsoft Corporation. All rights reserved.

Abstract:

    Regenerate the template by building hsm-src-x509-build with the generate-templates flag.

--"]
pub struct AzihsmCsrTbsParams<'a> {
    pub public_key: &'a [u8; 97usize],
    pub subject_sn: &'a [u8; 32usize],
}
impl AzihsmCsrTbsParams<'_> {
    pub const PUBLIC_KEY_LEN: usize = 97usize;
    pub const SUBJECT_SN_LEN: usize = 32usize;
}
pub struct AzihsmCsrTbs {
    tbs: [u8; Self::TBS_TEMPLATE_LEN],
}
impl AzihsmCsrTbs {
    const PUBLIC_KEY_OFFSET: usize = 74usize;
    const SUBJECT_SN_OFFSET: usize = 19usize;
    const PUBLIC_KEY_LEN: usize = 97usize;
    const SUBJECT_SN_LEN: usize = 32usize;
    pub const TBS_TEMPLATE_LEN: usize = 226usize;
    const TBS_TEMPLATE: [u8; Self::TBS_TEMPLATE_LEN] = [
        48u8, 129u8, 223u8, 2u8, 1u8, 0u8, 48u8, 43u8, 49u8, 41u8, 48u8, 39u8, 6u8, 3u8, 85u8, 4u8,
        3u8, 12u8, 32u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 48u8, 118u8, 48u8, 16u8, 6u8, 7u8, 42u8, 134u8, 72u8, 206u8,
        61u8, 2u8, 1u8, 6u8, 5u8, 43u8, 129u8, 4u8, 0u8, 34u8, 3u8, 98u8, 0u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8, 95u8,
        95u8, 95u8, 95u8, 95u8, 160u8, 53u8, 48u8, 51u8, 6u8, 9u8, 42u8, 134u8, 72u8, 134u8, 247u8,
        13u8, 1u8, 9u8, 14u8, 49u8, 38u8, 48u8, 36u8, 48u8, 18u8, 6u8, 3u8, 85u8, 29u8, 19u8, 1u8,
        1u8, 255u8, 4u8, 8u8, 48u8, 6u8, 1u8, 1u8, 255u8, 2u8, 1u8, 1u8, 48u8, 14u8, 6u8, 3u8,
        85u8, 29u8, 15u8, 1u8, 1u8, 255u8, 4u8, 4u8, 3u8, 2u8, 2u8, 4u8,
    ];
    pub fn new(params: &AzihsmCsrTbsParams) -> Self {
        let mut template = Self {
            tbs: Self::TBS_TEMPLATE,
        };
        template.apply(params);
        template
    }
    pub fn sign<Sig, Error>(
        &self,
        sign_fn: impl Fn(&[u8]) -> Result<Sig, Error>,
    ) -> Result<Sig, Error> {
        sign_fn(&self.tbs)
    }
    pub fn tbs(&self) -> &[u8] {
        &self.tbs
    }
    fn apply(&mut self, params: &AzihsmCsrTbsParams) {
        #[inline(always)]
        fn apply_slice<const OFFSET: usize, const LEN: usize>(
            buf: &mut [u8; 226usize],
            val: &[u8; LEN],
        ) {
            buf[OFFSET..OFFSET + LEN].copy_from_slice(val);
        }
        apply_slice::<{ Self::PUBLIC_KEY_OFFSET }, { Self::PUBLIC_KEY_LEN }>(
            &mut self.tbs,
            params.public_key,
        );
        apply_slice::<{ Self::SUBJECT_SN_OFFSET }, { Self::SUBJECT_SN_LEN }>(
            &mut self.tbs,
            params.subject_sn,
        );
    }
}
