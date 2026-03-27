use std::ffi::CString;
use std::os::raw::c_char;

#[repr(C)]
#[derive(Debug)]
pub enum RunaValueTag {
    String,
    Integer,
    Float,
    Boolean,
    Table,
    Nil,
}

#[repr(C)]
pub union RunaValueData {
    pub string: *const c_char,
    pub identifier: *const u8,
    pub integer: isize,
    pub float: f64,
    pub boolean: bool,
}

#[repr(C)]
pub struct RunaValueFFI {
    pub tag: RunaValueTag,
    pub data: RunaValueData,
}

impl crate::core::RunaValue {
    pub fn to_ffi(&self) -> RunaValueFFI {
        match self {
            crate::core::RunaValue::String(s) => {
                let cstr = CString::new(s.clone()).unwrap();
                let ptr = cstr.into_raw();

                RunaValueFFI {
                    tag: RunaValueTag::String,
                    data: RunaValueData { string: ptr },
                }
            }

            crate::core::RunaValue::Integer(i) => RunaValueFFI {
                tag: RunaValueTag::Integer,
                data: RunaValueData { integer: *i },
            },

            crate::core::RunaValue::Float(f) => RunaValueFFI {
                tag: RunaValueTag::Float,
                data: RunaValueData { float: *f },
            },

            crate::core::RunaValue::Boolean(b) => RunaValueFFI {
                tag: RunaValueTag::Boolean,
                data: RunaValueData { boolean: *b },
            },

            crate::core::RunaValue::Nil => RunaValueFFI {
                tag: RunaValueTag::Nil,
                data: RunaValueData { integer: 0 }
            },

            crate::core::RunaValue::Table(internal_identifier, _) => RunaValueFFI {
                tag: RunaValueTag::Table,
                data: RunaValueData { identifier: internal_identifier.as_ptr() }
            },
        }
    }
}
