
#[repr(C)]
pub struct uv_cb_type {
    fci: crate::ffi::zend_fcall_info,
    fcc: crate::ffi::zend_fcall_info_cache,
}
