use std::collections::{HashMap, HashSet};
use crate::db_helpers::{cb_item};

pub enum DbEngine {
    No,
    Hash(HashMap<u16, HashSet<cb_item>>),
}

impl DbEngine {
    fn unwrap(&'static mut self) -> &mut HashMap<u16, HashSet<cb_item>> {
        match self {
            DbEngine::No => {
                panic!("No data present");
            }
            DbEngine::Hash(val) => { val }
        }
    }

    pub fn db_map_get(&'static mut self, k: u16) -> &'static cb_item {
        match self.unwrap().get(&k) {
            None => { panic!("No item present") }
            Some(mut cb) => {
               let (numb, item) = &cb.iter().enumerate().next().unwrap();
                cb.remove(&item);
                *item
            }
        }
    }

    pub fn db_map_has(&'static mut self, k: u16) -> bool {
        self.unwrap().contains_key(&k)
    }

    pub fn db_map_add(&'static mut self, fd: u16, function_item: cb_item) {
        if !self.unwrap().contains_key(&fd) {
            self.unwrap().insert(fd, HashSet::new());
        }
        self.unwrap().get(&fd).unwrap().insert( function_item);
    }
}