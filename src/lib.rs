mod core;
mod ffi;
use core::*;
use std::ffi::CString;
use std::{ffi::c_void, fs::File, io::BufReader};
use std::os::raw::c_char;

macro_rules! runa_api_function {
    (
        fn $name:ident($runa:ident $(, $arg:ident : $arg_ty:ty )* ) $(-> $ret:ty)? {
            $($body:tt)*
        }
    ) => {
        #[no_mangle]
        pub extern "C" fn $name(ptr: *mut Runa, $($arg: $arg_ty),*) $(-> $ret)? {
            let $runa = unsafe { &mut *ptr };
            $($body)*
        }
    };
}


#[no_mangle]
pub extern "C" fn runa_start() -> *mut Runa {
    Box::into_raw(Box::new(Runa {
        ..Default::default()
    }))
}

#[no_mangle]
pub extern "C" fn runa_free(ptr: *mut Runa) {
    if ptr.is_null() { return; }
    unsafe { _ = Box::from_raw(ptr); }
}

runa_api_function!(
fn runa_loadfile(runa, filename: *const i8) {
    runa.filename = Some(
        unsafe {
            std::ffi::CStr::from_ptr(filename)
        }.to_string_lossy().into_owned()
    );
    let path = runa.filename.as_ref().unwrap();
    let file = File::open(path).unwrap();
    let reader = BufReader::new(file);
    runa.file = Some(reader);
    core::parse(runa);
});

runa_api_function!(
fn runa_push_function(runa, name: *const i8, callback: *const c_void, argc: i32) {
    let function_name = unsafe {
        std::ffi::CStr::from_ptr(name)
    }.to_string_lossy().into_owned();
    let callback = unsafe { std::mem::transmute::<*const c_void, core::RunaCallback>(callback) };
    core::runa_push_local(runa, Local::Function(Function {
        name: function_name,
        std: true,
        argv: None,
        argc: argc as usize,
        body: None,
        callback: Some(callback),
    }));
});

runa_api_function!(
fn runa_peek_arg(runa, index: i32) -> ffi::RunaValueFFI {
    let args = match runa.args.last() {
        Some(a) => a,
        None => return RunaValue::Nil.to_ffi()
    };

    let value = match args.get(index as usize) {
        Some(v) => v,
        None => return RunaValue::Nil.to_ffi()
    };

    value.to_ffi()
});

#[no_mangle]
pub extern "C" fn runa_value_free(val: ffi::RunaValueFFI) {
    unsafe {
        match val.tag {
            ffi::RunaValueTag::String => {
                let ptr = val.data.string as *mut c_char;
                if! ptr.is_null()
                { _ = CString::from_raw(ptr); }
            }
            _ => {}
        }
    }
}


#[no_mangle]
#[allow(unreachable_patterns)]
pub extern "C" fn runa_value_to_string(val: ffi::RunaValueFFI) -> *mut c_char {
    unsafe {
        let c_string = match val.tag {
            ffi::RunaValueTag::String  => return val.data.string as *mut c_char,
            ffi::RunaValueTag::Integer => CString::new(val.data.integer.to_string()).unwrap(),
            ffi::RunaValueTag::Float   => CString::new(val.data.float.to_string()).unwrap(),
            ffi::RunaValueTag::Boolean => CString::new(val.data.boolean.to_string()).unwrap(),
            ffi::RunaValueTag::Nil     => CString::new("nil").unwrap(),
            _ => unreachable!(),
        };

        c_string.into_raw()
    }
}

#[no_mangle]
pub extern "C" fn runa_str_free(ptr: *mut c_char) {
    if ptr.is_null() { return; }
    unsafe { let _ = CString::from_raw(ptr); }
}
