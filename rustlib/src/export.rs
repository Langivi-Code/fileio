use std::ffi::{CStr, CString};
use libc::c_char;
use crate::db_helpers::fd::DbFd;
use crate::db_helpers::seq::{DbTime, EntryToPop};
use super::db_helpers::{ cb_item, vars};
use super::db_helpers::db_collection::DbCollection as Col;
extern crate libc;
#[no_mangle]
pub extern "C" fn pg_get_item(k: u16) -> &'static cb_item {
    unsafe {
        let storage = DbTime::get_storage(k);
        let time_entry = storage.back().unwrap();
        Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_get(time_entry.func_name.as_str())
    }
}
#[no_mangle]
pub extern "C" fn pg_get_and_remove_item(k: u16)-> cb_item  {
    unsafe {
        let storage = DbTime::get_storage(k);
        let time_entry = storage.pop_back().unwrap();
        Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_get_and_remove(time_entry.func_name.as_str())
    }
}

#[no_mangle]

pub extern "C" fn pg_get_func_name(k: u16) -> *const c_char {
    unsafe {
        let storage = DbTime::get_storage(k);
        let time_entry = storage.back().unwrap();
        let mut bts= time_entry.func_name.clone().as_bytes().to_vec();
        bts.push(0);
        // dbg!(&bts);
        let c_s = CStr::from_bytes_with_nul(bts.as_slice()).unwrap();
        // println!("{:?}",c_s.to_bytes());
        c_s.as_ptr()
    }
}

#[no_mangle]
pub extern "C" fn pg_has_item( k: u16) -> bool {
    unsafe {
        let storage = DbTime::get_storage(k);
        let time_entry = storage.back();
        if time_entry.is_none() {
            return  false
        }

        let v = Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_has(time_entry.unwrap().func_name.as_str());
        // println!("has {}", v);
        v
    }
}

#[no_mangle]
pub extern "C" fn pg_add_item(func_name:*const c_char, k: u16, function_item: cb_item) {
    unsafe {
        let str = CStr::from_ptr(func_name);
        println!("{}",str.to_str().unwrap());
        let storage = DbTime::get_storage(k);
        storage.push_front(EntryToPop::new(str));
        Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_add(str.to_str().unwrap(), function_item);
    }
}
//
// #[no_mangle]
// pub extern "C" fn my_get_item(k: u16) -> &'static cb_item {
//     unsafe {
//         Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_get(func_name)
//     }
// }
//
// #[no_mangle]
// pub extern "C" fn my_has_item(k: u16) -> bool {
//     unsafe {
//         Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_has(func_name)
//     }
// }
//
// #[no_mangle]
// pub extern "C" fn my_add_item(k: u16, function_item: cb_item) {
//     unsafe {
//         Col::get_engine(&mut vars::ENGINES[0], k).get_named_engine(k).db_map_add(func_name, function_item);
//     }
// }

#[no_mangle]
pub extern "C" fn fd_map_add(k: u16) {
    DbFd::get_storage().insert(k);
}

#[no_mangle]
pub extern "C" fn fd_map_has(k: u16) -> bool {
    DbFd::get_storage().contains(&k)
}