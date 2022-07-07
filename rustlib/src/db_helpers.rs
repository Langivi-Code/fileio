pub mod db_engine;
pub mod db_collection;
pub mod vars;
pub mod fd;

use crate::ffi::{false_, zval};
use crate::ffi::types::uv_cb_type;

#[repr(C)]
#[allow(dead_code)]
enum DB_TYPE {
    MysqlDb,
    PgsqlDb,
}

#[repr(C)]
pub struct cb_item {
    cb: uv_cb_type,
    cb_read: uv_cb_type,
    read: bool,
    written: bool,
    db_type: DB_TYPE,
    db_handle: *mut zval,
}

impl PartialEq<Self> for cb_item {
    fn eq(&self, other: &Self) -> bool {
        let mut eq = false;
        if self.cb != other.cb {
            eq = false;
        }
        if self.cb_read!=other.cb_read {
            eq = false;
        }
        eq
    }
}

