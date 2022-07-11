use crate::ffi::zend_refcounted_h;

pub mod copy;
mod vars;
pub mod promise_list;

pub fn zend_gc_try_delref(p: &mut zend_refcounted_h) {
    if (p.refcount > 0) {
        p.refcount -= 1;
    }
}