pub mod db_engine;
pub mod db_collection;
pub mod vars;
pub mod fd;

use crate::ffi::{zval};
use crate::ffi::types::uv_cb_type;

#[repr(C)]
#[allow(dead_code)]
enum DB_TYPE {
    MysqlDb,
    PgsqlDb,
}

#[repr(C)]
#[derive(Eq, Hash)]
pub struct cb_item {
    cb: uv_cb_type,
    cb_read: uv_cb_type,
    read: bool,
    written: bool,
    db_type: DB_TYPE,
    db_handle: *mut zval,
}
