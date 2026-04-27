// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec;
use core::ops::Range;

use mcr_error::McrResult;
use mcr_types::SecureByteVec;

use crate::engine::SoftAes;
use crate::SoftAesErr;
use crate::SyncAesEcb;

pub(crate) fn aes(
    encrypt: bool,
    engine: &SoftAes,
    k: &[u8],
    a: u64,
    b: u64,
) -> McrResult<(u64, u64)> {
    let mut block = SecureByteVec::from([a.to_le_bytes(), b.to_le_bytes()].concat());

    if encrypt {
        engine.encrypt(k, &mut block)?;
    } else {
        engine.decrypt(k, &mut block)?;
    }

    let x = u64::from_le_bytes(block[0..8].try_into().unwrap());
    let y = u64::from_le_bytes(block[8..16].try_into().unwrap());

    Ok((x, y))
}

// 5649 section 3
const PADDED_UPPER_AIV: u64 = 0xA65959A600000000;
// RFC 3394 section 2.2.3
const UNPADDED_AIV: u64 = 0xA6A6A6A6A6A6A6A6;

/// As specified by RFC 5649 section 4.1
pub fn key_wrap(engine: &SoftAes, kek: &[u8], input: &[u8], output: &mut [u8]) -> McrResult<()> {
    if kek.len() != 16 && kek.len() != 24 && kek.len() != 32 {
        Err(SoftAesErr::InvalidKekLength)?
    }

    if output.len() < 16 {
        Err(SoftAesErr::InsufficientOutputBufferLength)?
    }

    // compute aiv according to RFC 5649 section 3
    let m = input.len();
    let mli = m as u64;
    let aiv = (PADDED_UPPER_AIV | mli).swap_bytes();

    if input.len() % 8 == 0 {
        // no padding
        return base_key_wrap(engine, kek, input, output, aiv);
    }

    // append padding
    let r = input.len().next_multiple_of(8);
    let mut p = SecureByteVec::from(vec![0u8; r]);

    p[0..input.len()].copy_from_slice(input);

    // special case
    if p.len() == 8 {
        let p64 = u64::from_le_bytes(p[0..8].try_into().unwrap());
        let (c0, c1) = aes(true, engine, kek, aiv, p64)?;
        output[0..8].copy_from_slice(&c0.to_le_bytes());
        output[8..16].copy_from_slice(&c1.to_le_bytes());
        Ok(())
    } else {
        base_key_wrap(engine, kek, &p, output, aiv)
    }
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub(crate) enum AivPadding {
    None,

    Length(usize),
}

fn check_aiv(output: &[u8], aiv: u64) -> McrResult<AivPadding> {
    if output.len() % 8 != 0 {
        Err(SoftAesErr::UnalignedOutputBufferLength)?
    }
    // check AIV
    if aiv == UNPADDED_AIV {
        return Ok(AivPadding::None);
    }
    if aiv & 0xffffffffu64 != PADDED_UPPER_AIV >> 32 {
        Err(SoftAesErr::InvalidAivPadding)?
    }
    // check data size
    let n = output.len() / 8;
    let mli = ((aiv >> 32) as u32).swap_bytes() as usize;
    if mli <= 8 * (n - 1) || mli > 8 * n {
        Err(SoftAesErr::InvalidMli)?
    }

    // check zero padding
    // safely check rightmost bytes are 0 to avoid potential padding oracle attacks
    let mut acc = 0;
    for x in &output[mli..] {
        acc |= *x;
    }
    if acc == 0 {
        Ok(AivPadding::Length(mli))
    } else {
        Err(SoftAesErr::PaddingOracleAttachDetected)?
    }
}

/// In-placement implementation of AES key unwrapping using a software implementation of AES,
/// as specified by RFC 5649 section 4.2.
///
/// # Arguments
///
/// * `kek` - the AES key that will be used to unwrap the key
/// * `input` - a u8 slice of the wrapped key that is at least 16 bytes long, which be used to contain the output
///
/// # Returns
///
/// * A result indicating the range of the input that contains the unwrapped key.
pub fn key_unwrap_inplace(
    engine: &SoftAes,
    kek: &[u8],
    input: &mut [u8],
) -> McrResult<Range<usize>> {
    if input.len() < 16 {
        Err(SoftAesErr::InsufficientInputLength)?
    }

    if input.len() == 16 {
        // special case
        let c0 = u64::from_le_bytes(input[0..8].try_into().unwrap());
        let c1 = u64::from_le_bytes(input[8..16].try_into().unwrap());
        let (a, p1) = aes(false, engine, kek, c0, c1)?;
        let _ = &input[0..8].copy_from_slice(&p1.to_le_bytes());
        let plen = match check_aiv(&input[0..8], a)? {
            AivPadding::None => Err(SoftAesErr::UnpaddedAiv)?,
            AivPadding::Length(size) => size,
        };
        return Ok(0..plen);
    }

    match base_key_unwrap_inplace(engine, kek, input)? {
        (AivPadding::None, _) => Err(SoftAesErr::UnpaddedAiv)?,
        // remove padding bytes
        (AivPadding::Length(l), Range { start, end: _ }) => Ok(start..start + l),
    }
}

/// In-placement implementation of AES key unwrapping using a software implementation of AES,
/// as specified by RFC 5649 section 4.2.
///
/// # Arguments
///
/// * `kek` - the AES key that will be used to unwrap the key
/// * `input` - a u8 slice of the wrapped key that is at least 16 bytes long
/// * `output` - a mutable u8 slice that is at least 8 bytes long to contain the unwrapped key
///
/// # Returns
///
/// * A result indicating the size of the output that is valid.
pub fn key_unwrap(
    engine: &SoftAes,
    kek: &[u8],
    input: &[u8],
    output: &mut [u8],
) -> McrResult<usize> {
    if input.len() < 16 {
        Err(SoftAesErr::InsufficientInputLength)?
    }

    if output.len() < 8 {
        Err(SoftAesErr::InsufficientOutputBufferLength)?
    }

    if input.len() == 16 {
        // special case
        let c0 = u64::from_le_bytes(input[0..8].try_into().unwrap());
        let c1 = u64::from_le_bytes(input[8..16].try_into().unwrap());
        let (a, p1) = aes(false, engine, kek, c0, c1)?;
        output[0..8].copy_from_slice(&p1.to_le_bytes());
        let plen = match check_aiv(output, a)? {
            AivPadding::None => Err(SoftAesErr::UnpaddedAiv)?,
            AivPadding::Length(size) => size,
        };
        return Ok(plen);
    }

    match base_key_unwrap(engine, kek, input, &mut output[..input.len() - 8])? {
        AivPadding::None => Err(SoftAesErr::UnpaddedAiv)?,
        AivPadding::Length(plen) => Ok(plen),
    }
}

/// As specified by RFC 3394 section 2.2.1
///
/// Optimized to use the output as the intermediate storage rather than having an additional allocation.
pub(crate) fn base_key_wrap(
    engine: &SoftAes,
    kek: &[u8],
    input: &[u8],
    output: &mut [u8],
    aiv: u64,
) -> McrResult<()> {
    if input.len() % 8 != 0 {
        Err(SoftAesErr::UnalignedInputBufferLength)?
    }

    if output.len() < input.len() + 8 {
        Err(SoftAesErr::IncompatibleInputAndOutputLength)?
    }

    // initialize
    let n = input.len() / 8;
    let mut a = aiv;
    output[8..(n + 1) * 8].copy_from_slice(&input[..n * 8]);

    // intermediate calculation
    for j in 0..6 {
        for i in 0..n {
            let b = u64::from_le_bytes(output[(i + 1) * 8..(i + 2) * 8].try_into().unwrap());
            let (msb, lsb) = aes(true, engine, kek, a, b)?;
            output[(i + 1) * 8..(i + 2) * 8].copy_from_slice(&lsb.to_le_bytes());
            a = msb ^ (((n * j) + (i + 1)) as u64).swap_bytes();
        }
    }
    // output
    output[0..8].copy_from_slice(&a.to_le_bytes());
    Ok(())
}

/// As specified by RFC 3394 section 2.2.2
///
/// Optimized to use the output as the intermediate storage rather than having an additional allocation.
pub(crate) fn base_key_unwrap(
    engine: &SoftAes,
    kek: &[u8],
    input: &[u8],
    output: &mut [u8],
) -> McrResult<AivPadding> {
    if input.len() % 8 != 0 {
        Err(SoftAesErr::UnalignedInputBufferLength)?
    }

    if input.len() != output.len() + 8 {
        Err(SoftAesErr::IncompatibleInputAndOutputLength)?
    }

    // initialize
    let n = input.len() / 8 - 1;
    let mut a = u64::from_le_bytes(input[0..8].try_into().unwrap());
    output[0..n * 8].copy_from_slice(&input[8..(n + 1) * 8]);

    // intermediate calculation
    for j in (0..6).rev() {
        for i in (0..n).rev() {
            let b = u64::from_le_bytes(output[i * 8..(i + 1) * 8].try_into().unwrap());
            let (msb, lsb) = aes(
                false,
                engine,
                kek,
                a ^ (((n * j) + (i + 1)) as u64).swap_bytes(),
                b,
            )?;
            a = msb;
            output[i * 8..(i + 1) * 8].copy_from_slice(&lsb.to_le_bytes());
        }
    }
    check_aiv(output, a)
}

/// In-place imeplementation of AES key unwrapping.
/// As specified by RFC 3394 section 2.2.2
pub(crate) fn base_key_unwrap_inplace(
    engine: &SoftAes,
    kek: &[u8],
    input: &mut [u8],
) -> McrResult<(AivPadding, Range<usize>)> {
    if input.len() % 8 != 0 {
        Err(SoftAesErr::UnalignedInputBufferLength)?
    }

    // initialize
    let n = input.len() / 8 - 1;
    let mut a = u64::from_le_bytes(input[0..8].try_into().unwrap());

    // intermediate calculation
    for j in (0..6).rev() {
        for i in (0..n).rev() {
            let b = u64::from_le_bytes(input[(i + 1) * 8..(i + 2) * 8].try_into().unwrap());
            let (msb, lsb) = aes(
                false,
                engine,
                kek,
                a ^ (((n * j) + (i + 1)) as u64).swap_bytes(),
                b,
            )?;
            a = msb;
            input[(i + 1) * 8..(i + 2) * 8].copy_from_slice(&lsb.to_le_bytes());
        }
    }
    match check_aiv(&input[8..], a) {
        Ok(padding) => Ok((padding, 8..input.len())),
        Err(err) => Err(err),
    }
}

#[cfg(test)]
mod test {
    use alloc::vec;

    use hex_literal::hex;
    use keywrap::UNPADDED_AIV;

    use crate::*;

    // TODO: test failure cases

    fn check_wrap_unpadded(engine: &SoftAes, kek: &[u8], k: &[u8], known_output: &[u8]) {
        let mut output = vec![0u8; k.len() + 8];
        assert_eq!(
            Ok(()),
            base_key_wrap(engine, kek, k, &mut output, UNPADDED_AIV)
        );
        assert_eq!(*known_output, output,);

        let mut input = vec![0u8; k.len()];
        assert!(Ok(AivPadding::None) == base_key_unwrap(engine, kek, known_output, &mut input));
        assert_eq!(*k, input);

        let mut output_copy = vec![0u8; known_output.len()];
        output_copy.copy_from_slice(known_output);

        let result = base_key_unwrap_inplace(engine, kek, &mut output_copy);
        assert!(result.is_ok());
        assert!(result.as_ref().unwrap().0 == AivPadding::None);
        let r = result.unwrap().1;
        assert_eq!(*k, output_copy[r]);
    }

    fn check_wrap(engine: &SoftAes, kek: &[u8], k: &[u8], known_output: &[u8]) {
        let mut output = vec![0u8; k.len().next_multiple_of(8) + 8];
        assert_eq!(Ok(()), key_wrap(engine, kek, k, &mut output));
        assert_eq!(*known_output, output,);

        let mut input = vec![0u8; k.len().next_multiple_of(8)];
        assert_eq!(
            Ok(k.len()),
            key_unwrap(engine, kek, known_output, &mut input)
        );
        assert_eq!(k, &input[..k.len()]);

        let mut output_copy = vec![0u8; known_output.len()];
        output_copy.copy_from_slice(known_output);
        // assert_eq!(Ok(8..k.len()), key_unwrap_inplace(kek, &mut output_copy));
        // assert_eq!(k, &output_copy[8..k.len()]);
        let result = key_unwrap_inplace(engine, kek, &mut output_copy);
        assert!(result.is_ok());
        let r = result.unwrap();
        assert_eq!(*k, output_copy[r]);
    }

    #[test]
    fn test_key_wrap_unwrap() {
        let engine = SoftAes::new();
        check_wrap(
            &engine,
            &hex!("5840df6e29b02af1 ab493b705bf16ea1 ae8338f4dcc176a8"),
            &hex!("c37b7e6492584340 bed1220780894115 5068f738"),
            &hex!("138bdeaa9b8fa7fc 61f97742e72248ee 5ae6ae5360d1ae6a 5f54f373fa543b6a"),
        );
        check_wrap(
            &engine,
            &hex!("5840df6e29b02af1 ab493b705bf16ea1 ae8338f4dcc176a8"),
            &hex!("466f7250617369"),
            &hex!("afbeb0f07dfbf541 9200f2ccb50bb24f"),
        );
    }

    #[test]
    fn test_rsa_key_wrap() {
        let engine = SoftAes::new();
        // key generated by softhsm2
        // wrap generated by p11wrap, then verified to decode with openssl rsa -inform DER
        check_wrap(
            &engine,
            &hex!("0fef74496281613fe3cb6ddc11946a97308f32b03da5d357034455a62bd7d913"),
            // DER-encoded
            &hex!("30820942020100300d06092a864886f70d01010105000482092c308209280201000282020100c16ce7efe07d4f839789ed894c22bf99837fa34152c09a02e8351f9ab898977b9ab55d939b7f047491a5666fcee1581648dc59fb8741b210f45616e9c11cbed63a44740a8c99dd774eb83ed242afa477f7be51813d4fed10300a21d63f3f1720b230423bf715de706ecf5db408b7ff5c6c6817faa466eb51849c4452caf0e4d256856b3258170c535ccf59e367275999bf4998b217212a6b723b13ad1a08a6084d56b714f082d1f7e305b1cde0e08a68fec2e50782af010ed2faf7302537c9578835943bcf6a430227729c651e94c9b49a57622dc497c1b90629500bc08fa05ca6f80fb1f421a8a31391afde2a4f6450b5a19ff3b66fe59fc2fd8c31e27fc8714905495401d1e97cc443873d0d58871690169487c1754cd37c7926222ad4ac84833015012146df2c3db2e3b521c3e3d9e7f249b4e05406d3b7c67d47544b4167160e00eb98e6e483bcf6da7638f67185d3ae6cb957ff2ed67ad73b4698dfa006e911e63e61d1dc03ebad7650392d59e463612e85be8ebf0c1a93f7dcd6e0d9967beb45d9acb99868397449292446a7bf1f5baccc3d4106fcaf0c36692c359d22644eb4347c05275008881b400950f370672500a2853d985e2805b1705322a2a9824e471fef0af34d1f1bab0d69af4e5f02ba886b46e180372c3d63c729bb48d78a8eacbc2e3e4876853b03eaac55d8e8b0fa6a0328bd96659e531e814cabff05020301000102820200538cabc8f1529f888b6a202928b26fc9ab43f46c5086f25fa416e24e6e433757c4556814ccdb7a3cb1f6dc7f1a413b4ff0ec019044f5eb2928a9911f2a73b4b90b952afcad8bfede3caa1834b16a7623fde6ff04d0cb97ee6099d7d0d823f3e50323165119a32020b7b4a9d88cd5919fc611d69dc4ead1e5b5dedc225917e1f73b39493b0752f9577ee4f0026ab9d419b5c76006393871bed4aa510efa0a575189ba95bd9d401cdb32fc7037aa55c363a3fbe27b32874e712d500b7b07f0876f605e286807697285592dc80163cac82fd365407add8ae1ba7ebf549ca6f2434612375f6c323461c80339705a8a331df540e78e14732ef56463912ac01782b3bcfeeda4f81928491752cf129ec356273e72de6012db1a356b6979ab5e4880a7ec1b3e09e5e266115d34963bf215f8e1378094a0408b74d0071857cefe1cda1235a0071f361331f5cd40a0db4c0a45b0e8332d18c4f4d60c9943609dd5e4163feec2cce3a0a7bfd18dabde4c106634dbfdd3b2aa08cd0653905ad2e1b0754bfd6d0e61b8d0e3a60bab96684331abc611b4860336ca82d80ad4603d296d8748b0698c53688af930319c019436c9ac5ccc0420a1a3c218e1e0d126e3a0c0efe03b01c873f96dc7397599baaf1f0bd7aa35fc72045ad997550d4cb2bc2156e19b3f18dd3d6005c31aab6f848914a777d208e41b6b1798bffb69a4b943d2f72180af270282010100d500cd62b11d520f3f5f67bbbe3b0f384d4127458c0589c718c84d470fe2063460c4380e21b9e3e2effec2df52c5ac91c7668dd6ce7f24a8bd1db85d1e61b74f06845ed896967fb5439bedd198ebcc001c392e018614d4de85c1e9f575970085d120ed6e2f550ff1d4a47db0fcc6c6dc78b35cb5a9672edbe457a86365a159ffd0c30d7b17759f92af4133310ef7576865a14917f6f3b5ada17c3ff3b310e5478798e8bf9c59cd0752722dd041d815868d9245718d6faa859ee59dab18be252df767713ba085f6efac366191dc1fb4d9dd4152aa467f723b8c31913532c05e212db43915524c1edc733bcc85143248ca82521f71f2a973175854fe7c4472ce070282010100e87866ace129b2efb1511dfb99d4b94dd4ae08f8c464267474899fa0547c633b897b288e45ad424cdf1c3034ad5dba8ddff2ee9ca87d075abcd04456ceb9125d4fc0555863b465f3feb81573979a28058170adbc95b5cc1f73065713d102ffc73e8388122fcc2045ad803e29a05cb285b2788eb4121b17826a6e5097b945f2df00a509967f63fb6c7f63c60a8a6e506da96284ada2bb778c7df37ed4341c2baa2f532a82ca04b396aaa97dcee2a804f0af6b990179eb21e28101a8b880cc093dac7b3b36dc632a6df33ab6347f9c7d4b77fc51dd810b4d43dddbaf4f808a3910fb5566a8f3970e64c3eab9059d430a5e0d3daac412f9d6ced400a0c2649687930282010043a93794cc123648e5a696bb0a9894231c573ff455a044bddbdf74bcc80cd24fbd15578115b188f443ac3796dd2231c935001cba496a15e90bf9eee0959010bca7a350c59840425d0016fe1a806b16a84a4ed790605929ad6debc537d59c9bebe61c818f68b5aa94f529334c0f5fcd37a797c2316a987e481766c8f49a0a011ec3dfd1de71fb492f3d4086d4649ea9435e553774c75b6f30b2855783403473c09f10318efc4982597b150dfc838df278856c1b6710b892005413385c6b45ab65c89b315af188dc8211c04c8a1ad46f8c6d3e1cd63e02a9f4c479d25885c099a60c709d596a8507a09b72396b6498fc8a678f52b1cd958a1bb651796d5c08e3ed02820100765e68197b04da81c5cdc1f08ca18e411d1c08a1728e742e33f0a780e1c3fc5b2263bd80c3e5b6aea1a41ebf93cef7e0a9b96eda01c8b7e7f1e0320be972bef1185c9c98471c62155d0baa90930f0175dc34dda1fab0f8e0c296e5dbc73f39b1e018e53e2d6c48ba71dfacc7514a21f485ad712c2b53f42289c2e5f27b7a2c727447949c36fd57d624ff4ea5ab5b0444f24ca1b62ff838a526b4c74d91c30e50b78c2747354a338f72129f66aa6e57259e8cb0f9d9746a6a84b9ad8954bb8fb1019435d077635f056125f204c53d8f66d00f8037c32f5806487f45c2bb76a61c097db0aa326e25fc14edfd8be8496c8bfb3dfd279d69ed809ede3afd006ecd5502820101009108a9ce7c8dabc2c0da4b2f121f450f92d29aa876cb4a9780c8b425cec5993f46fc3f03595964a3292fa9e45020d9665039e8af64515d3d77c403a4ac3d9f07e76adb82dcd27549bc2484cc6ace26df2d4ce74df940264bc3da6088c1078c3da77fda2c4362e8f79326b94a7150973b7f652a1962a385124d1fc664146bbcf8ef1d37c3ac06089a6f93cbe3bb3fad09f27be4719eedec76ac9a75ffd5348d99aa472ed0c116e34a23bbc84d32998943d80266ddb4b65cd8a27ba748cee2921a2948b838b4ee6adc46943b9129cb289f54668a8cdf0fd276786973bb7a713f94c3ab0f4fa44fdffde16912706704473afa29d71c162a2b025d9065e9e31fe5340000"),
            &hex!("47507ea3cf5f23180fe91e6f7968f438c216dd607fef105eeddabf51b52f006575e52879c126cb4591180e92a5bcc60598b57f4f6d791661bb6c0edb7a58e852564135a0894b6c664566301cf40aea2fad130dedfef505db36ab347757932c859b7e46bded4db9ec4023d7c68538787b17674a2fd847c66f3b6ac8b3edcba910e69b50914d819233e4513e3d9c3fec2c6f07f7b1f04bb1614b7dead7a9dfbccc6f8a0a8a1c6ef4bd7ca3e795e75c56f148fe61e2e450aab6f8d209a2490837a267ff9a188d451670fb9163bbf96cdc9d295a7440b17458903c264679799cb800d9ad3daf834d13ba97258b95b52ca982241a069a0bf520c7e00d6dd49c4ed4e5771b8a781c6108af7bbfc760ed02870ebcc9dd4696886d688de3e5b37df27f0f45f0984c2175370fbeffc95f5538d0f18106bdc6c4e45dedf7e439930a34d9fc3e7377c9074757fc73271faa6bf4e3a9d0cb6c5efa8ea091ac51abe1c7dec4ffce48a7c01a0ab5db1a3f3014515cb769124a8bec55e5952ea4a638cdc887ebda4d353010f68e168650ccca707275f9cd373481c6f1c382429181c470d2bf3e8724699037a372e1efb631fb26359bdddb033f12800d2159c4c355877343a7947b00ee2558ce8fdfde7e347e2063ec0f3578ea63d4539378ae234ca8a987c05e1477deafb1a73de833b2e7bdcefcbf88d33aacf54fbd9f46d97954d124e6745a0ce2214e57822f5496f8ca9aa091d2a10c8e59a631b2cd5d68a472fc5666884926cf5fbcd2609d6a6a8bc193d5932964f1e6fceeb06320ce8722b74f801135cb5c6d25ff34d5f4e7ed84d91985a366497cb9dd12698f13a806b743e282e7a58be5764362e2d5855442ef47abbed93e55e051d22fb21fb034f65821ccf95188f9b9675137bd3dbcedb9f0027133802a2998133d42f5cdc1418382b47f8ced17878082d94a83bc37af163964039fa3a1faa4b9ba75df8df47c31e58295e35de3d27c6a452aa1b0396c10a8eb65a05f06c47f706b1aa91ee265d803c8b12cc690bc2cff7d6ad5d21979a3e0c3aac8c74d098fc79cc68607ba7a881a9d5b756cd16862de7be844bdd7936aa8e476048e1c0bee43510d0e8ac4276fcb78c37992f9c53885c76f06e6401b5a59512e513a06a7b506e3405d1d44c879065a02d02d003b4a2759dc8d4a5b2fdad497dac50246590389db3faaa26bf93a562ac8f757029c355f826afacb49c1e6623829f104b9c08b3960bb70748891e8e3ea42c172a058498678243de940c6598dad7ec7524779ee2e2878ee16cea7d0cdf80754f6255c80177b5e144089d99bd6de6714938a47d4dadc030d97d7db195471e94822ec73063236feb80cc106abc86e21d69690b40b96ef2397f1a9b029435e58a294272a23991e37ac5a3980abfb487849b0ed79da37c832a957ac6eaa41e6abe063dd38cdbd9004b5507528be9d17dcaec0196f234a73f96f70845a80471dd2b09cab44eb72584fe0f82ae3a14a9620d400101f92d6d424477b78c382ddb9834c31c475be8c26e0d96fa49610f14b0ad3c6c5dc26669d63923d94874c08677c8c20356987a9be2216312f62a0e13f74f2de751ddda14a7e3e4bddad36d4c12f3c1474d9c825d287639454fec05cc8b037d844a209e5edd4b4267c1a949eec502d83873b84259a6b102031522263509c6427c15122f54ad5c91a173a4ca00b3b33f7507e1b5d8b6f6d94d172ebaf59dbc3b7675527f59b23b35e9caf6fed87f79c3d3a82fe0d9faefaf458c44e274bc55081d678232e1b764952e533b7dcdb1c3f732bc68b08505094edfcaa17913d13147dfdf3464bb29224946cbc9e16a2a54a0b02360cac2d92116c1a2cc7761830dc0f7ca928c7ca220bc8514ed616c7a29284435c93f5bb581c90e4c800c9dcc0e10e735a8a9832e261dd3f7dec277499123719440c75961cadbf29f2400bd3ae2a9e87f67e0e152b5f9b614cd4e4aa3e3532efb299b11cb5124cc725b79e1cc53a9b10d7cfbe50d0b82feea0cfbe9361abdad5bce6704308e3731ca39d79e604ce2cf6ac5442f879837662a432acbfbbf3a720b9dd5731a6f03fce9c00a68a368ff4015efd9a9ceaf5ec0ca51d9d7e3b8d4350f7c37153ec3f3aa6598cf521a9bb5184b26aba209d7222f1308e5b23f6d22606bb7dfafe5ccbf87f193eeb6e4d09f0763d005a320c6fd7f1ff22f656b8bc1e08596d013bb178ee22bdcb3c14b7a7217fc89f60b0359ac18e39a97736dd26a84b3d0858c4b3c91c2a1a85b0736d7e77be2193eb5a168519dad52eca2cb079eace0ce45451280ae864b4fbd75906d2d1f2f6860b2b99e6790a701c53df1fb264420d71b24b02104de89915f29c68676f32dd46a128a84e7f8098cfe0eabb6f1bf488e1d87a5eae51fd3e3e6b7d40d0d4584023736518c12f8c931216e88f4d0d11f8b95ecf78e231f1de5aca5d6ac1698b73c0a13bfecacd038c62a8195b028e825a10ae2ed3b1d86785fabf8ede27971781a5c223d5f48876e71c60f19c37bb2bc815d7cbbccf4d9dcaf0442deb322529504ae2c86cf4fca6c67316a425d083c4076cc76824c34b7b90b30af23a672969e4870c2c9212a8fbddea936be9fdf925dc1e2062c5e9dbe0f7e7bb4f4d7546f9cc942f7c0082e71ab2280e789b68056a5401975a5bdc34d49fcba543a2fcac1de5227367bedf639e18405a75d70743f32ea9be1ad07f20ccd21a41639364a9f86e2f4dafa2c6feef3031112f4f4b09b7a36fc2b2536eb4495c0a51ba4fe90f3f72699bdcda05871d445afa544701f2a510ddbfe6cad75f78c90ae94015c1bbfd711077db7f52122648d9fcb4c43b213efbf6d55be8bb0173959a358a075c4328767f4d158616e97e31198c8902b533e4f839e79ee61c7bcb55e135fc5ed2ea1cda3f570fbc1a55b5d33558344477dfb5fef43759c080a059d52e1aa9f74f7756f826f13b18134d806f5fe2945c634f312aa99e87cf394499bd7555dae28ff693ac84bae47f32823f66150585b74f07bce47c8055ee78c715b3565cc4f54d855770c0b8889102b417a5339beb1089e97ac5c3aa51a509b11c5e16baa2909ee9a6c5dc612751eafa8a76a482fe1960ef491f618469ec4d7ef1dfd77570d9acac63ffa282baf20b50599757740756078779c0d06ffd99905830950e011dd03b666c2c042dd176d4784ffdce42037146d81877580984ad25b98b07731c20abafe9ecda7c59f56e7960d6d6c37e3a01e0a13c093510e0a4b95b0bc766d3920334bcca2060c87cbe6c196ed6df45eb119ecee2dd920e8fb5d0851834518e6817516d15e5e5c276b2b10044e156cad328c7b305a9d85ef12b48590e3b02e099a06f7e1e7131"),
        );
    }

    #[test]
    fn test_base_unpadded_key_wrap_unwrap() {
        let engine = SoftAes::new();
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F"),
            &hex!("00112233445566778899AABBCCDDEEFF"),
            &hex!("1FA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"),
        );
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F1011121314151617"),
            &hex!("00112233445566778899AABBCCDDEEFF"),
            &hex!("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5D"),
        );
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"),
            &hex!("00112233445566778899AABBCCDDEEFF"),
            &hex!("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE7"),
        );
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F1011121314151617"),
            &hex!("00112233445566778899AABBCCDDEEFF0001020304050607"),
            &hex!("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725A93 6BA814915C6762D2"),
        );
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"),
            &hex!("00112233445566778899AABBCCDDEEFF0001020304050607"),
            &hex!("A8F9BC1612C68B3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"),
        );
        check_wrap_unpadded(
            &engine,
            &hex!("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"),
            &hex!("00112233445566778899AABBCCDDEEFF000102030405060708090A0B0C0D0E0F"),
            &hex!("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"));
    }
}
