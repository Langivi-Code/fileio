use std::collections::HashMap;
use crate::db_helpers::db_engine::DbEngine;

pub enum DbFunctions {
    No,
    Hash(HashMap<u16, DbEngine>),
}

impl DbFunctions {
    fn unwrap(&'static mut self) -> &mut HashMap<u16, DbEngine> {
        match self {
            DbFunctions::No => {
                panic!("Engine is not initialized");
            }
            DbFunctions::Hash(val) => { val }
        }
    }
    pub fn get_named_engine(&'static mut self , fd:u16)->&'static mut DbEngine {
        let map = self.unwrap();
        if !map.contains_key(&fd) {
            map.insert(fd, DbEngine::Hash(HashMap::new()));
        }
        map.get_mut(&fd).unwrap()
    }

}