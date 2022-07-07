use std::borrow::BorrowMut;
use std::collections::HashSet;
use crate::db_helpers::vars::FD_SET;

pub enum DbFd {
    No,
    Hash(HashSet<u16>),
}


impl DbFd {
    fn unwrap(&'static mut self) -> &mut HashSet<u16> {
        match self {
            DbFd::No => {
                panic!("No data present");
            }
            DbFd::Hash(val) => { val }
        }
    }
 pub fn get_storage() -> &'static mut HashSet<u16> {
        unsafe {
            match FD_SET.borrow_mut() {
                DbFd::No => {
                    FD_SET = DbFd::Hash(HashSet::new());
                    FD_SET.unwrap()
                }
                DbFd::Hash(obj) => obj
            }
        }
    }
}