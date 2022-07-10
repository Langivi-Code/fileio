extern crate core;

mod db_helpers;
mod export;
pub mod ffi;
mod promise_helpers;

use crate::ffi::php_printf;
use std::os::raw::c_char;

// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }
