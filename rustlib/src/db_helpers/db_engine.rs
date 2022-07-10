use crate::db_helpers::cb_item;
use crate::ffi::{php_var_dump, zval};
use std::borrow::{Borrow, BorrowMut};
use std::collections::{HashMap, VecDeque};

pub enum DbEngine {
    No,
    Hash(HashMap<String, VecDeque<cb_item>>),
}

impl DbEngine {
    fn unwrap(&'static mut self) -> &mut HashMap<String, VecDeque<cb_item>> {
        match self {
            DbEngine::No => {
                panic!("Engine is not initialized");
            }
            DbEngine::Hash(val) => val,
        }
    }

    pub fn db_map_get(&'static mut self, fn_name: &str) -> &'static cb_item {
        let map = self.unwrap();
        match map.get(&fn_name.to_string()) {
            None => {
                println!("None::db_map_get: FD {} queue size is", fn_name);
                panic!("No item present")
            }
            Some(mut cb) => cb.get(0).unwrap(),
        }
    }
    pub fn db_map_get_next(&'static mut self, fn_name: &str) -> &'static cb_item {
        match self.unwrap().get(&fn_name.to_string()) {
            None => {
                panic!("No next item present")
            }
            Some(mut cb) => cb.get(1).unwrap(),
        }
    }
    pub fn db_map_get_and_remove(&'static mut self, fn_name: &str) -> cb_item {
        let v = self.unwrap().get_mut(&fn_name.to_string());
        match v {
            None => {
                println!(
                    "in no item db_map_get_and_remove:FD {} queue size is {}",
                    fn_name,
                    v.unwrap().len()
                );
                panic!("No item fff present {}", fn_name);
            }
            Some(cb) => {
                for (numb, item) in cb.iter().enumerate() {
                    println!("Element number #{}", numb);
                    unsafe {
                        let zv = &item.borrow().db_handle as *const zval;
                        php_var_dump(zv, 1);
                    }
                }
                let cb_item_ = cb.pop_front().unwrap();
                println!(
                    "db_map_get_and_remove:FD {} queue size is {}",
                    fn_name,
                    cb.len()
                );
                cb_item_
            }
        }
    }

    pub fn db_map_has(&'static mut self, fn_name: &str) -> bool {
        let map = self.unwrap();
        let str_k = fn_name.to_string();
        let fd = map.contains_key(&str_k);
        if !fd {
            return false;
        }
        let vec = map.get(&str_k);
        let mut result = true;
        if let None = vec {
            result = false;
        } else if let Some(val) = vec {
            result = if val.len() == 0 { false } else { true };
        }
        result
    }

    pub fn db_map_add(&'static mut self, fn_name: &str, function_item: cb_item) {
        let hash = self.unwrap();
        let str_k = fn_name.to_string();
        if !hash.contains_key(&str_k) {
            hash.insert(str_k.clone(), VecDeque::with_capacity(10));
        }
        let vec = hash.get_mut(&str_k).unwrap();
        unsafe {
            println!("FROM ADD RUST");
            php_var_dump(function_item.borrow().db_handle.borrow() as *const zval, 1);
        }
        vec.push_back(function_item);
        println!("db_map_add:FD {} queue size is {}", fn_name, vec.len());
    }
}
