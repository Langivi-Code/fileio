use crate::db_helpers::vars::{FD_SET, TIME_ENTRIES};
use std::borrow::BorrowMut;
use std::collections::{HashMap, LinkedList};
use std::ffi::CStr;
use std::time::{SystemTime, UNIX_EPOCH};

#[derive(Debug)]
pub struct EntryToPop {
    pub func_name: String,
    time: u128,
}

impl EntryToPop {
    pub fn new(func_name: &CStr) -> EntryToPop {
        let string = func_name.to_str().unwrap();
        let func = String::from(string);
        let now = SystemTime::now();

        EntryToPop {
            func_name: func,
            time: now.duration_since(UNIX_EPOCH).unwrap().as_millis(),
        }
    }
}
pub enum DbTime {
    No,
    Hash(HashMap<u16, LinkedList<EntryToPop>>),
}

impl DbTime {
    fn unwrap(&'static mut self) -> &mut HashMap<u16, LinkedList<EntryToPop>> {
        match self {
            DbTime::No => {
                panic!("No data present");
            }
            DbTime::Hash(val) => val,
        }
    }

    pub fn get_storage(fd: u16) -> &'static mut LinkedList<EntryToPop> {
        unsafe {
            match TIME_ENTRIES.borrow_mut() {
                DbTime::No => {
                    TIME_ENTRIES = DbTime::Hash(HashMap::from([(fd, LinkedList::new())]));
                    TIME_ENTRIES.unwrap().get_mut(&fd).unwrap()
                }
                DbTime::Hash(obj) => {
                    if !obj.contains_key(&fd) {
                        obj.insert(fd, LinkedList::new());
                    }
                    obj.get_mut(&fd).unwrap()
                }
            }
        }
    }
}
