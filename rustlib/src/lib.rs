pub mod ffi;
mod db_helpers;
mod export;

use crate::ffi::{php_printf};
use std::os::raw::c_char;

// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }


#[no_mangle]
pub extern "C" fn f() {
    let str = String::from("hello");
    unsafe {
        php_printf(str.as_ptr() as *const c_char);
    }
}

//
// /// cbindgen:derive-eq
// #[repr(C)]
// pub struct FdMap {
//     map: HashMap<u16, bool>,
// }
//
