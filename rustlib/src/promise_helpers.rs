use std::ffi::{CStr, CString};
use std::os::raw::c_void;
use std::ptr;
use crate::ffi::{php_var_dump, size_t, zend_object, zend_read_property, zend_refcounted_h, zend_update_property, zval};

pub mod copy;
mod vars;
pub mod promise_list;

pub fn zend_gc_try_delref(p: &mut zend_refcounted_h) {
    if (p.refcount > 0) {
        p.refcount -= 1;
    }
}

pub  fn update_zval (s:&mut zend_object, t: &mut zend_object) ->bool{
    let _then = "_list";
    let mut bts = Vec::from(_then);
    // bts.push(0);
    // dbg!(&bts);
    let c_s = unsafe{CStr::from_bytes_with_nul_unchecked(bts.as_slice())};
    let ptr: *mut zval = ptr::null_mut();
    let zv = unsafe {zend_read_property(s.ce, s, c_s.as_ptr(), (bts.len()) as size_t, false,  ptr )};
    unsafe {
        php_var_dump(zv, 1);
    }
    unsafe {zend_update_property(t.ce, t, c_s.as_ptr(), (bts.len()) as size_t, zv)};
    return  true;
}