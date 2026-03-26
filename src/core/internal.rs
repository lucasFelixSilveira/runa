static mut SEED: u64 = 0x123456789ABCDEF0;

pub fn gen_id() -> String {
    let bytes = generate_random_bytes();
    bytes_to_hex(&bytes)
}

fn generate_random_bytes() -> [u8; 16] {
    let mut bytes = [0u8; 16];
    let mut state = unsafe { SEED };

    for byte in bytes.iter_mut() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        *byte = (state >> 56) as u8;
    }

    unsafe { SEED = state; }
    bytes
}

fn bytes_to_hex(bytes: &[u8; 16]) -> String {
    const HEX: &[u8; 16] = b"0123456789abcdef";
    let mut s = String::with_capacity(32);
    for &b in bytes {
        s.push(HEX[(b >> 4) as usize] as char);
        s.push(HEX[(b & 0x0F) as usize] as char);
    }
    s
}
