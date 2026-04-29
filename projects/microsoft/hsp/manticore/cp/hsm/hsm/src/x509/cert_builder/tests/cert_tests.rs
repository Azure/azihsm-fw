// Copyright (c) Microsoft Corporation. All rights reserved.

// Note: All the necessary code is auto generated
#[cfg(feature = "generate_templates")]
include!(concat!(env!("OUT_DIR"), "/azihsm_leaf_cert_tbs.rs"));
#[cfg(not(feature = "generate_templates"))]
include! {"../../build/azihsm_leaf_cert_tbs.rs"}

#[cfg(test)]
mod tests {
    use openssl::ecdsa::EcdsaSig;
    use openssl::sha::Sha384;
    use openssl::x509::X509;

    use x509_parser::nom::Parser;
    use x509_parser::oid_registry::asn1_rs::oid;
    use x509_parser::oid_registry::Oid;
    use x509_parser::prelude::X509CertificateParser;
    use x509_parser::x509::X509Version;

    use super::*;
    use crate::x509::cert_builder::builder::*;
    use crate::x509::cert_builder::test_utils::tests::*;
    use crate::x509::cert_builder::NotAfter;
    use crate::x509::cert_builder::NotBefore;

    fn make_test_cert(
        subject_key: &Ecc384AsymKey,
        issuer_key: &Ecc384AsymKey,
    ) -> AzihsmLeafCertTbs {
        let params = AzihsmLeafCertTbsParams {
            serial_number: &[0xABu8; AzihsmLeafCertTbsParams::SERIAL_NUMBER_LEN],
            public_key: &subject_key.pub_key().try_into().unwrap(),
            subject_sn: &subject_key.hex_str().into_bytes()
                [..AzihsmLeafCertTbsParams::SUBJECT_SN_LEN]
                .try_into()
                .unwrap(),
            issuer_sn: &issuer_key.hex_str().into_bytes().try_into().unwrap(),
            subject_key_id: &subject_key.sha1(),
            authority_key_id: &issuer_key.sha1(),
            not_before: &NotBefore::default().value,
            not_after: &NotAfter::default().value,
        };

        AzihsmLeafCertTbs::new(&params)
    }

    #[test]
    fn test_cert_signing() {
        let subject_key = Ecc384AsymKey::default();
        let issuer_key = Ecc384AsymKey::default();
        let ec_key = issuer_key.priv_key().ec_key().unwrap();
        let cert = make_test_cert(&subject_key, &issuer_key);

        let sig = cert
            .sign(|b| {
                let mut sha = Sha384::new();
                sha.update(b);
                EcdsaSig::sign(&sha.finish(), &ec_key)
            })
            .unwrap();

        assert_ne!(cert.tbs(), AzihsmLeafCertTbs::TBS_TEMPLATE);
        assert_eq!(
            &cert.tbs()[AzihsmLeafCertTbs::PUBLIC_KEY_OFFSET
                ..AzihsmLeafCertTbs::PUBLIC_KEY_OFFSET + AzihsmLeafCertTbs::PUBLIC_KEY_LEN],
            subject_key.pub_key(),
        );
        assert_eq!(
            &cert.tbs()[AzihsmLeafCertTbs::SUBJECT_SN_OFFSET
                ..AzihsmLeafCertTbs::SUBJECT_SN_OFFSET + AzihsmLeafCertTbs::SUBJECT_SN_LEN],
            &subject_key.hex_str().into_bytes()[..AzihsmLeafCertTbs::SUBJECT_SN_LEN],
        );
        assert_eq!(
            &cert.tbs()[AzihsmLeafCertTbs::ISSUER_SN_OFFSET
                ..AzihsmLeafCertTbs::ISSUER_SN_OFFSET + AzihsmLeafCertTbs::ISSUER_SN_LEN],
            issuer_key.hex_str().into_bytes(),
        );
        assert_eq!(
            &cert.tbs()[AzihsmLeafCertTbs::SUBJECT_KEY_ID_OFFSET
                ..AzihsmLeafCertTbs::SUBJECT_KEY_ID_OFFSET + AzihsmLeafCertTbs::SUBJECT_KEY_ID_LEN],
            subject_key.sha1(),
        );
        assert_eq!(
            &cert.tbs()[AzihsmLeafCertTbs::AUTHORITY_KEY_ID_OFFSET
                ..AzihsmLeafCertTbs::AUTHORITY_KEY_ID_OFFSET
                    + AzihsmLeafCertTbs::AUTHORITY_KEY_ID_LEN],
            issuer_key.sha1(),
        );
        let ecdsa_sig = Ecdsa384Signature {
            r: sig.r().to_vec_padded(48).unwrap().try_into().unwrap(),
            s: sig.s().to_vec_padded(48).unwrap().try_into().unwrap(),
        };

        let builder = Ecdsa384CertBuilder::new(cert.tbs(), &ecdsa_sig).unwrap();
        let mut buf = vec![0u8; builder.len()];
        builder.build(&mut buf).unwrap();

        let cert: X509 = X509::from_der(&buf).unwrap();
        assert!(cert.verify(issuer_key.priv_key()).unwrap());
    }

    #[test]
    fn test_extensions() {
        let subject_key = Ecc384AsymKey::default();
        let issuer_key = Ecc384AsymKey::default();
        let ec_key = issuer_key.priv_key().ec_key().unwrap();
        let cert = make_test_cert(&subject_key, &issuer_key);

        let sig = cert
            .sign(|b| {
                let mut sha = Sha384::new();
                sha.update(b);
                EcdsaSig::sign(&sha.finish(), &ec_key)
            })
            .unwrap();

        let ecdsa_sig = Ecdsa384Signature {
            r: TryInto::<[u8; 48]>::try_into(sig.r().to_vec_padded(48).unwrap()).unwrap(),
            s: TryInto::<[u8; 48]>::try_into(sig.s().to_vec_padded(48).unwrap()).unwrap(),
        };

        let builder = Ecdsa384CertBuilder::new(cert.tbs(), &ecdsa_sig).unwrap();
        let mut buf = vec![0u8; builder.len()];
        builder.build(&mut buf).unwrap();

        let mut parser = X509CertificateParser::new().with_deep_parse_extensions(true);
        let parsed_cert = match parser.parse(&buf) {
            Ok((_, parsed_cert)) => parsed_cert,
            Err(e) => panic!("x509 parsing failed: {:?}", e),
        };

        assert_eq!(parsed_cert.version(), X509Version::V3);

        // Basic checks on standard extensions
        let basic_constraints = parsed_cert.basic_constraints().unwrap().unwrap();
        assert!(basic_constraints.critical);

        let key_usage = parsed_cert.key_usage().unwrap().unwrap();
        assert!(key_usage.critical);

        // Check that TCG extensions are marked critical
        let ext_map = parsed_cert.extensions_map().unwrap();
    }

    #[test]
    #[cfg(feature = "generate_templates")]
    fn test_cert_template() {
        let manual_template = std::fs::read(std::path::Path::new(
            "./src/x509/build/azihsm_leaf_cert_tbs.rs",
        ))
        .unwrap();
        let auto_generated_template = std::fs::read(std::path::Path::new(concat!(
            env!("OUT_DIR"),
            "/azihsm_leaf_cert_tbs.rs"
        )))
        .unwrap();
        if auto_generated_template != manual_template {
            panic!(
                "Auto-generated AZIHSM  Leaf Certificate template is not equal to the manual template."
            )
        }
    }
}
