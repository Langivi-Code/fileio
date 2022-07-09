use crate::db_helpers::fd::DbFd;
use super::db_helpers::{ cb_item, vars};
use super::db_helpers::db_collection::DbCollection as Col;

#[no_mangle]
pub extern "C" fn pg_get_item(k: u16) -> &'static cb_item {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[0]).db_map_get(k)
    }
}
#[no_mangle]
pub extern "C" fn pg_get_and_remove_item(k: u16)-> cb_item  {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[0]).db_map_get_and_remove(k)
    }
}

#[no_mangle]
pub extern "C" fn pg_get_next_item(k: u16) -> &'static cb_item {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[0]).db_map_get_next(k)
    }
}

#[no_mangle]
pub extern "C" fn pg_has_item(k: u16) -> bool {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[0]).db_map_has(k)
    }
}

#[no_mangle]
pub extern "C" fn pg_add_item(k: u16, function_item: cb_item) {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[0]).db_map_add(k, function_item);
    }
}

#[no_mangle]
pub extern "C" fn my_get_item(k: u16) -> &'static cb_item {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[1]).db_map_get(k)
    }
}

#[no_mangle]
pub extern "C" fn my_has_item(k: u16) -> bool {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[1]).db_map_has(k)
    }
}

#[no_mangle]
pub extern "C" fn my_add_item(k: u16, function_item: cb_item) {
    unsafe {
        Col::get_engine(&mut vars::ENGINES[1]).db_map_add(k, function_item);
    }
}

#[no_mangle]
pub extern "C" fn fd_map_add(k: u16) {
    DbFd::get_storage().insert(k);
}

#[no_mangle]
pub extern "C" fn fd_map_has(k: u16) -> bool {
    DbFd::get_storage().contains(&k)
}