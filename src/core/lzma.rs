use std::{ffi::c_void, ptr};

extern "C" {
    pub fn runa_compress(
        input: *mut i8,
        input_size: i32,
        output: *mut *mut i8,
        output_size: *mut i32,
    ) -> bool;

    pub fn runa_decompress(
        input: *mut i8,
        input_size: i32,
        output_size: *mut i32,
    ) -> *mut i8;
}

pub fn compress(input: &[u8]) -> Option<Vec<u8>> {
    let mut output_ptr: *mut i8 = ptr::null_mut();
    let mut output_size: i32 = 0;

    let success = unsafe {
        runa_compress(
            input.as_ptr() as *mut i8,
            input.len() as i32,
            &mut output_ptr,
            &mut output_size,
        )
    };

    if !success || output_ptr.is_null() {
        return None;
    }

    let compressed = unsafe {
        std::slice::from_raw_parts(output_ptr as *const u8, output_size as usize)
    }.to_vec();

    unsafe { free(output_ptr as *mut c_void); }
    Some(compressed)
}

pub fn decompress(input: &[u8]) -> Option<Vec<u8>> {
    let mut output_size: i32 = 0;

    let output_ptr = unsafe {
        runa_decompress(
            input.as_ptr() as *mut i8,
            input.len() as i32,
            &mut output_size,
        )
    };

    if output_ptr.is_null() {
        return None;
    }

    let decompressed = unsafe {
        std::slice::from_raw_parts(output_ptr as *const u8, output_size as usize)
    }.to_vec();

    unsafe { free(output_ptr as *mut c_void); }
    Some(decompressed)
}

extern "C" {
    fn free(ptr: *mut c_void);
}
