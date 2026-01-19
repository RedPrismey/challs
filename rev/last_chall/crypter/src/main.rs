use aes::cipher::KeyInit;
use aes::cipher::{generic_array::GenericArray, BlockEncrypt};
use aes::Aes128;
use rand::{rngs::StdRng, Rng, SeedableRng};
use std::fs::read;
use std::fs::File;
use std::io::Write;

// Instpiré de https://github.com/Amaop/Rust-Crypter/ et de https://youtu.be/h2j3Y05puE4

const BLOCK_SIZE: usize = 16; // AES128 block size

fn main() -> std::io::Result<()> {
    let args: Vec<String> = std::env::args().collect();

    if args.len() != 2 {
        println!("Usage : {} <file.rs>", args.first().unwrap());
        return Err(std::io::Error::new(
            std::io::ErrorKind::NotFound,
            "wrong number of argument",
        ));
    }

    let file_name = args.get(1).unwrap();
    let mut plain_file = read(file_name)?;

    let mut crypt_file = File::create("encrypted_out.bin")?;
    let mut key_file = File::create("key")?;

    // Calculate padding size
    let pad_size = BLOCK_SIZE - (plain_file.len() % BLOCK_SIZE);
    // pad according to PKCS#7
    plain_file.extend(vec![pad_size as u8; pad_size]);

    let mut rng = StdRng::from_entropy();
    let key = rng.gen::<[u8; 16]>();
    let key = GenericArray::from_slice(&key);

    let cipher = Aes128::new(key);

    let mut enc_bytes = Vec::new();
    for block in plain_file.chunks(BLOCK_SIZE) {
        let mut block_array = GenericArray::clone_from_slice(block);
        cipher.encrypt_block(&mut block_array);
        enc_bytes.extend_from_slice(&block_array);
    }

    crypt_file.write_all(&enc_bytes)?;
    key_file.write_all(key)?;

    Ok(())
}
