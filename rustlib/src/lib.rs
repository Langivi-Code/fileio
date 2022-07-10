mod ffi;

use std::collections::{HashMap, HashSet};
use std::os::raw::c_char;
use lazy_static::lazy_static;
use crate::ffi::*;


pub struct uv_cb {
    fci: zend_fcall_info,
    fcc: zend_fcall_info_cache,
}

pub struct db_type {
    handle: zval,
    cb: uv_cb,
    cb_read: uv_cb,
}
// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }
// #[repr(C)]
// pub struct DbQueue{
//     fd:u16,
//     method_map:HashMap<str,HashSet<i8>>
// // }
// lazy_static! {
//     static ref RB: FdMap = {
//         let mut maps = HashMap::new();
//         FdMap { map: maps }
//     };
// }
// static mut RB: FdMap = FdMap { map: Default::default() };

#[no_mangle]
pub extern fn f() {
    let str = String::from("hello");
    unsafe {
        php_printf(str.as_ptr() as *const c_char);
    }
}
// #[no_mangle]
// pub extern  fn fd_map_add(k: u16) {
//    unsafe {
//        RB.add(k);
//    }
// }
// //
// #[no_mangle]
// pub extern fn fd_map_has(k: u16)->bool {
//     unsafe {
//         RB.has(k)
//     }
// }
//
// /// cbindgen:derive-eq
// #[repr(C)]
// pub struct FdMap {
//     map: HashMap<u16, bool>,
// }
//
