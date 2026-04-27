// Copyright (c) Microsoft Corporation. All rights reserved.

// Note: All the necessary code is auto generated
#[cfg(feature = "generate_templates")]
include!(concat!(env!("OUT_DIR"), "/azihsm_csr_tbs.rs"));
#[cfg(not(feature = "generate_templates"))]
include! {"../../build/azihsm_csr_tbs.rs"}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::x509::cert_builder::test_utils::tests::*;
    use openssl::sha::Sha384;
    use openssl::{ecdsa::EcdsaSig, x509::X509Req};

    use x509_parser::cri_attributes::ParsedCriAttribute;
    use x509_parser::extensions::ParsedExtension;
    use x509_parser::oid_registry::asn1_rs::oid;
    use x509_parser::prelude::{FromDer, X509CertificationRequest};

    use crate::x509::cert_builder::builder::*;

    fn make_test_csr(subject_key: &Ecc384AsymKey) -> AzihsmCsrTbs {
        let params = AzihsmCsrTbsParams {
            public_key: &subject_key.pub_key().try_into().unwrap(),
            subject_sn: &subject_key.hex_str().into_bytes()[..AzihsmCsrTbsParams::SUBJECT_SN_LEN]
                .try_into()
                .unwrap(),
        };

        AzihsmCsrTbs::new(&params)
    }

    #[test]
    fn test_csr_signing() {
        let key = Ecc384AsymKey::default();
        let ec_key = key.priv_key().ec_key().unwrap();
        let csr = make_test_csr(&key);

        let sig: EcdsaSig = csr
            .sign(|b| {
                let mut sha = Sha384::new();
                sha.update(b);
                EcdsaSig::sign(&sha.finish(), &ec_key)
            })
            .unwrap();

        assert_ne!(csr.tbs(), AzihsmCsrTbs::TBS_TEMPLATE);
        assert_eq!(
            &csr.tbs()[AzihsmCsrTbs::PUBLIC_KEY_OFFSET
                ..AzihsmCsrTbs::PUBLIC_KEY_OFFSET + AzihsmCsrTbs::PUBLIC_KEY_LEN],
            key.pub_key(),
        );
        assert_eq!(
            &csr.tbs()[AzihsmCsrTbs::SUBJECT_SN_OFFSET
                ..AzihsmCsrTbs::SUBJECT_SN_OFFSET + AzihsmCsrTbs::SUBJECT_SN_LEN],
            &key.hex_str().into_bytes()[..AzihsmCsrTbsParams::SUBJECT_SN_LEN],
        );

        let ecdsa_sig = Ecdsa384Signature {
            r: sig.r().to_vec_padded(48).unwrap().try_into().unwrap(),
            s: sig.s().to_vec_padded(48).unwrap().try_into().unwrap(),
        };

        let builder = Ecdsa384CsrBuilder::new(csr.tbs(), &ecdsa_sig).unwrap();
        let mut buf = vec![0u8; builder.len()];
        builder.build(&mut buf).unwrap();

        let req: X509Req = X509Req::from_der(&buf).unwrap();
        assert!(req.verify(&req.public_key().unwrap()).unwrap());
        assert!(req.verify(key.priv_key()).unwrap());
    }

    #[test]
    fn test_extensions() {
        let key = Ecc384AsymKey::default();
        let ec_key = key.priv_key().ec_key().unwrap();
        let csr = make_test_csr(&key);

        let sig: EcdsaSig = csr
            .sign(|b| {
                let mut sha = Sha384::new();
                sha.update(b);
                EcdsaSig::sign(&sha.finish(), &ec_key)
            })
            .unwrap();

        let ecdsa_sig = Ecdsa384Signature {
            r: sig.r().to_vec_padded(48).unwrap().try_into().unwrap(),
            s: sig.s().to_vec_padded(48).unwrap().try_into().unwrap(),
        };

        let builder = Ecdsa384CsrBuilder::new(csr.tbs(), &ecdsa_sig).unwrap();
        let mut buf = vec![0u8; builder.len()];
        builder.build(&mut buf).unwrap();

        let (_, parsed_csr) = X509CertificationRequest::from_der(&buf).unwrap();

        let requested_extensions = parsed_csr
            .certification_request_info
            .iter_attributes()
            .find_map(|attr| {
                if let ParsedCriAttribute::ExtensionRequest(requested) = attr.parsed_attribute() {
                    Some(&requested.extensions)
                } else {
                    None
                }
            })
            .unwrap();

        // BasicConstraints
        let bc_ext = requested_extensions
            .iter()
            .find(|ext| matches!(ext.parsed_extension(), ParsedExtension::BasicConstraints(_)))
            .unwrap();
        let ParsedExtension::BasicConstraints(bc) = bc_ext.parsed_extension() else {
            panic!("Extension is not BasicConstraints");
        };

        assert!(bc_ext.critical);
        assert!(bc.ca);

        // KeyUsage
        let ku_ext = requested_extensions
            .iter()
            .find(|ext| matches!(ext.parsed_extension(), ParsedExtension::KeyUsage(_)))
            .unwrap();

        assert!(ku_ext.critical);
    }

    #[test]
    #[cfg(feature = "generate_templates")]
    fn test_csr_template() {
        let manual_template =
            std::fs::read(std::path::Path::new("./src/x509/build/azihsm_csr_tbs.rs")).unwrap();
        let auto_generated_template = std::fs::read(std::path::Path::new(concat!(
            env!("OUT_DIR"),
            "/azihsm_csr_tbs.rs"
        )))
        .unwrap();
        if auto_generated_template != manual_template {
            panic!("Auto-generated AZIHSM CSR template is not equal to the manual template.")
        }
    }
}
