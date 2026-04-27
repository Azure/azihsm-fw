// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::error::HsmErr;
use crate::partition::CredentialMgr;
use crate::partition::UserCredential;

#[test]
fn test_authorize_user_default() {
    let cred_mgr = CredentialMgr::default();
    let user_cred = UserCredential::default();

    assert!(!cred_mgr.authorize_user(&user_cred.id, &user_cred.pin));
}

#[test]
fn test_authorize_user_unauthorized() {
    let cred_mgr = CredentialMgr::default();
    let user_cred = UserCredential::default();

    assert!(!cred_mgr.authorize_user(&[1u8; 16usize], &user_cred.pin));
    assert!(!cred_mgr.authorize_user(&user_cred.id, &[2u8; 16usize]));
}

#[test]
fn test_change_user_cred() {
    let mut cred_mgr = CredentialMgr::default();

    assert!(cred_mgr
        .change_user_cred(&[1u8; 16usize], &[2u8; 16usize])
        .is_ok());

    assert!(cred_mgr.authorize_user(&[1u8; 16usize], &[2u8; 16usize]));
}

#[test]
fn test_change_user_cred_default() {
    let mut cred_mgr = CredentialMgr::default();
    let user_cred = UserCredential::default();

    let result = cred_mgr.change_user_cred(&user_cred.id, &user_cred.pin);
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), HsmErr::InvalidUserCredential);
}

#[test]
fn test_user_cred() {
    let cred_mgr = CredentialMgr::default();
    let user_cred = UserCredential::default();

    let cred = cred_mgr.user_cred();
    assert!(cred == user_cred);
}

#[test]
fn test_change_user_to_default_id() {
    let mut cred_mgr = CredentialMgr::default();
    assert!(cred_mgr.change_user_cred(&[0x56; 16], &[0x57; 16]).is_ok());

    let result = cred_mgr.change_user_cred(&[0; 16], &[2u8; 16usize]);
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), HsmErr::InvalidUserCredential);

    let result = cred_mgr.change_user_cred(&[2u8; 16usize], &[0; 16]);
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), HsmErr::InvalidUserCredential);
}

#[test]
fn test_app_vault_id() {
    let mut cred_mgr = CredentialMgr::default();
    assert!(cred_mgr.change_user_cred(&[0x45; 16], &[0x67; 16]).is_ok());

    let result = cred_mgr.get_user_vault_id();
    assert_eq!(result, 0);
}

#[test]
fn test_authorize_user() {
    let mut cred_mgr = CredentialMgr::default();
    assert!(cred_mgr.change_user_cred(&[0x45; 16], &[0x67; 16]).is_ok());

    let result = cred_mgr.authorize_user(&[0x45; 16usize], &[0x67; 16usize]);
    assert!(result);
}

#[test]
fn test_authorize_user_unauth() {
    let cred_mgr = CredentialMgr::default();

    assert!(!cred_mgr.authorize_user(&[1u8; 16usize], &[2u8; 16usize]));
    assert!(!cred_mgr.authorize_user(&[0u8; 16usize], &[0u8; 16usize]));
}

#[test]
fn test_change_user_cred_smaller_size() {
    let mut cred_mgr = CredentialMgr::default();

    let result = cred_mgr.change_user_cred(&[1; 15], &[2; 16]);
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), HsmErr::InvalidUserCredential);

    let result = cred_mgr.change_user_cred(&[1; 16], &[2; 15]);
    assert!(result.is_err());
    assert_eq!(result.unwrap_err(), HsmErr::InvalidUserCredential);
}

#[test]
fn test_verify_user_cred_is_set() {
    let mut cred_mgr = CredentialMgr::default();

    assert!(!cred_mgr.verify_user_cred_is_set());

    let result = cred_mgr.change_user_cred(&[1; 16], &[2; 16]);
    assert!(result.is_ok());

    assert!(cred_mgr.verify_user_cred_is_set());
}

#[test]
fn test_restore_user_cred() {
    let mut cred_mgr = CredentialMgr::default();

    let default_cred = UserCredential::default();
    let restore_cred = UserCredential {
        id: [1; 16],
        pin: [2; 16],
        vault_id: 3,
    };

    assert!(!cred_mgr.verify_user_cred_is_set());
    cred_mgr.restore_user_cred(default_cred);
    assert!(!cred_mgr.verify_user_cred_is_set());
    assert!(cred_mgr.user_cred() == default_cred);

    cred_mgr.restore_user_cred(restore_cred);
    assert!(cred_mgr.verify_user_cred_is_set());
    assert!(cred_mgr.user_cred() == restore_cred);
}

#[test]
fn test_clear() {
    let mut cred_mgr = CredentialMgr::default();

    assert!(!cred_mgr.verify_user_cred_is_set());

    let result = cred_mgr.change_user_cred(&[1; 16], &[2; 16]);
    assert!(result.is_ok());

    assert!(cred_mgr.verify_user_cred_is_set());

    cred_mgr.clear();
    assert!(!cred_mgr.verify_user_cred_is_set());
    assert!(cred_mgr.user_cred() == UserCredential::default());
}
