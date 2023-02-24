use crate::db_helpers::vars::FD_SET;
use std::borrow::BorrowMut;
use std::collections::{HashMap, HashSet, LinkedList};
use crate::ffi::zend_object;
use crate::promise_helpers::vars::PROMISE_LIST;

pub enum PromiseList {
    No,
    Hash(HashMap<u64, LinkedList<*mut zend_object>>),
}

impl PromiseList {
    fn unwrap(&'static mut self) -> &mut HashMap<u64, LinkedList<*mut zend_object>> {
        match self {
            PromiseList::No => {
                panic!("No data present");
            }
            PromiseList::Hash(val) => val,
        }
    }
    pub fn get_storage() -> &'static mut HashMap<u64, LinkedList<*mut zend_object>> {
        unsafe {
            match PROMISE_LIST.borrow_mut() {
                PromiseList::No => {
                    PROMISE_LIST = PromiseList::Hash(HashMap::new());
                    PROMISE_LIST.unwrap()
                }
                PromiseList::Hash(_) =>  PROMISE_LIST.unwrap(),
            }
        }
    }
}
