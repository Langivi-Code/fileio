pub mod db_engine;
pub mod db_collection;
pub mod vars;
pub mod fd;
pub mod db_functions;
pub mod seq;

use std::ffi::c_void;
use std::hash::{Hash, Hasher};
use crate::ffi::{false_, zval};
use crate::ffi::types::uv_cb_type;

/// cbindgen:ignore
#[repr(C)]
pub(crate) enum DB_TYPE {
    MysqlDb,
    PgsqlDb,
}

/// cbindgen:ignore
#[repr(C)]
pub struct cb_item {
    cb: uv_cb_type,
    cb_read: uv_cb_type,
    read: bool,
    written: bool,
    db_type: DB_TYPE,
    db_handle: zval,
    conn: *mut c_void,
}

// impl Eq for cb_item {
//
// }
// impl Hash for cb_item{
//     fn hash<H: Hasher>(&self, state: &mut H) {
//         todo!()
//     }
//
//     fn hash_slice<H: Hasher>(data: &[Self], state: &mut H) where Self: Sized {
//         todo!()
//     }
// }
// impl PartialEq<Self> for cb_item {
//     fn eq(&self, other: &Self) -> bool {
//         let mut eq = false;
//             let a = &self.cb as *const uv_cb_type;
//             let b = &other.cb as *const uv_cb_type;
//         if  a != b {
//             eq = false;
//         }
//         let a = &self.cb_read as *const uv_cb_type;
//         let b = &other.cb_read as *const uv_cb_type;
//         if a != b  {
//             eq = false;
//         }
//         eq
//     }
// }

